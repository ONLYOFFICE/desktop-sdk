#import "slider.h"

#import "utils.h"

@interface NSStyledSliderCell : NSSliderCell<StyledView>
{
	CSliderStyle* m_style;
	NSImage* m_knob_image;
	NSRect m_track_rect;
}
- (instancetype)initWithStyle:(CSliderStyle*)style;
@end

@implementation NSStyledSliderCell

- (instancetype)initWithStyle:(CSliderStyle*)style {
	self = [super init];
	if (self) {
		// set skin
		m_style = style;
		[self updateStyle];
	}
	return self;
}

- (void)drawBarInside:(NSRect)rect flipped:(BOOL)flipped {
	// draw the whole track
	const CGFloat track_thickness = m_style->track.thickness;
	const CGFloat track_border_radius = m_style->track.border_radius;
	if (![self isVertical]) {
		m_track_rect = NSInsetRect(rect, 0, (rect.size.height - track_thickness) / 2);
	} else {
		m_track_rect = NSInsetRect(rect, (rect.size.width - track_thickness) / 2, 0);
	}
	NSBezierPath* track_path = [NSBezierPath bezierPathWithRoundedRect:m_track_rect xRadius:track_border_radius yRadius:track_border_radius];
	[NSColorFromHex(m_style->track.color) setFill];
	[track_path fill];

	// draw the filled portion of track (left of knob)
	double value = (self.doubleValue - self.minValue) / (self.maxValue - self.minValue);
	NSRect filled_rect = m_track_rect;
	if (![self isVertical]) {
		filled_rect.size.width *= value;
	} else {
		filled_rect.origin.y += filled_rect.size.height * (1 - value);
		filled_rect.size.height *= value;
	}
	[NSColorFromHex(m_style->track.fill_color) setFill];
	NSBezierPath* filled_path = [NSBezierPath bezierPathWithRoundedRect:filled_rect xRadius:track_border_radius yRadius:track_border_radius];
	[filled_path fill];
}

- (NSRect)knobRectFlipped:(BOOL)flipped {
	// calculate knob center offset relative to slider current value
	CGFloat knob_center_max_offset = 0;
	if (![self isVertical]) {
		// by default we stick knob edge to the edge of the slider track
		knob_center_max_offset = m_knob_image.size.width / 2;
		if (knob_center_max_offset >= m_track_rect.origin.x) {
			// if the knob is big enough, then stick it to the control rect edge
			knob_center_max_offset -= m_track_rect.origin.x;
		}
	} else {
		knob_center_max_offset = m_knob_image.size.height / 2;
		if (knob_center_max_offset >= m_track_rect.origin.y) {
			knob_center_max_offset -= m_track_rect.origin.y;
		}
	}
	// this formula essentially means the following:
	//  - near the edges the knob will have corresponding offset from slider exact value, to be able to fit into control rect
	//  - at the middle the offset is zero, meaning the knob positioned exactly where slider is
	//  - the offset changes lineary and flips its sign when knob passes the middle point
	CGFloat knob_center_offset = -(knob_center_max_offset * self.doubleValue) / ((self.maxValue - self.minValue) / 2) + knob_center_max_offset;
	// get current slider value
	double value = (self.doubleValue - self.minValue) / (self.maxValue - self.minValue);
	CGFloat slider_pos = 0;
	if (![self isVertical]) {
		slider_pos = m_track_rect.size.width * value + m_track_rect.origin.x;
	} else {
		slider_pos = m_track_rect.size.height * (1 - value) + m_track_rect.origin.y;
	}
	// create knob rect from the center point
	NSRect knob_rect;
	if (![self isVertical]) {
		CGFloat knob_center_pos = slider_pos + knob_center_offset;
		knob_rect = NSMakeRect(knob_center_pos - m_knob_image.size.width / 2, NSMidY(m_track_rect) - m_knob_image.size.height / 2, m_knob_image.size.width, m_knob_image.size.height);
	} else {
		CGFloat knob_center_pos = slider_pos - knob_center_offset;
		knob_rect = NSMakeRect(NSMidX(m_track_rect) - m_knob_image.size.width / 2, knob_center_pos - m_knob_image.size.height / 2, m_knob_image.size.width, m_knob_image.size.height);
	}
	return knob_rect;
}

- (void)drawKnob:(NSRect)knob_rect {
	if (m_style->is_knob_visible) {
		[m_knob_image drawInRect:knob_rect];
	}
}

- (void)updateStyle {
	// set knob image
	if (m_style->is_knob_visible) {
		const CGFloat knob_thickness = m_style->knob.thickness + m_style->knob.border_width;
		m_knob_image = [[NSImage imageWithSize:NSMakeSize(knob_thickness, knob_thickness) flipped:NO drawingHandler:^BOOL(NSRect dst_rect) {
			// draw knob
			const CGFloat border_radius = m_style->knob.border_radius;
			const CGFloat border_width = m_style->knob.border_width;
			NSRect knob_rect = NSMakeRect(dst_rect.origin.x + border_width / 2, dst_rect.origin.y + border_width / 2,
										  dst_rect.size.width - border_width, dst_rect.size.height - border_width);
			NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:knob_rect xRadius:border_radius yRadius:border_radius];
			[NSColorFromHex(m_style->knob.color) setFill];
			[path fill];
			// draw border
			[path setLineWidth:border_width];
			[NSColorFromHex(m_style->knob.border_color) setStroke];
			[path stroke];
			return YES;
		}] retain];
	} else {
		const CGFloat knob_thickness = m_style->track.thickness;
		m_knob_image = [[NSImage alloc] initWithSize:NSMakeSize(knob_thickness, knob_thickness)];
	}
}

- (void)dealloc {
#if !__has_feature(objc_arc)
	[m_knob_image release];
	[super dealloc];
#endif
}

@end


@interface NSStyledSlider ()
{
	NSStyledSliderCell* m_cell;
	// actions
	SEL m_action_press;
	SEL m_action_release;
}
@end

@implementation NSStyledSlider

// needed for correct redrawing when moving knob quickly or shrinking footer panel
- (void)setNeedsDisplayInRect:(NSRect)invalid_rect {
	[super setNeedsDisplayInRect:NSUnionRect(self.bounds, invalid_rect)];
}

- (instancetype)initWithStyle:(CSliderStyle*)style vertical:(BOOL)vertical {
	self = [super init];
	if (self) {
		// set cell
		m_cell = [[NSStyledSliderCell alloc] initWithStyle:style];
		[self setCell:m_cell];
		// set slider orientation
		[self setVertical:vertical];
		// set min and max values
		self.minValue = 0.0;
		self.maxValue = 100.0;
	}
	return self;
}

- (void)updateStyle {
	[m_cell updateStyle];
}

- (void)setActionPress:(SEL)action {
	m_action_press = action;
}

- (void)setActionRelease:(SEL)action {
	m_action_release = action;
}

- (void)mouseDown:(NSEvent*)event {
	if (self.target && self->m_action_press) {
		[self.target performSelector:self->m_action_press withObject:self];
	}
	// enters dragging loop
	[super mouseDown:event];
	// call release when dragging ends
	if (self.target && self->m_action_release) {
		[self.target performSelector:self->m_action_release withObject:self];
	}
}

- (void)dealloc {
#if !__has_feature(objc_arc)
	[m_cell release];
	// TODO: vertical slider cell is not deallocated ???
	[super dealloc];
#endif
}

@end
