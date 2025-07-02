#import "slider.h"

#import "utils.h"

@interface NSStyledSliderCell : NSSliderCell
{
	CSliderStyle* m_style;
	NSImage* m_knob_image;
	NSRect m_track_rect;
}
- (instancetype)initWithStyle:(CSliderStyle*)style;
- (void)updateStyle;
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
	m_track_rect = NSInsetRect(rect, 0, (rect.size.height - track_thickness) / 2);
	NSBezierPath* track_path = [NSBezierPath bezierPathWithRoundedRect:m_track_rect xRadius:track_border_radius yRadius:track_border_radius];
	[NSColorFromHex(m_style->track.color) setFill];
	[track_path fill];
	// draw the filled portion of track (left of knob)
	double value = (self.doubleValue - self.minValue) / (self.maxValue - self.minValue);
	NSRect filled_rect = m_track_rect;
	filled_rect.size.width *= value;
	[NSColorFromHex(m_style->track.fill_color) setFill];
	// TODO: when knob is visible, the right part of the filled rect does not have to be rounded
	NSBezierPath* filled_path = [NSBezierPath bezierPathWithRoundedRect:filled_rect xRadius:track_border_radius yRadius:track_border_radius];
	[filled_path fill];
}

- (NSRect)knobRectFlipped:(BOOL)flipped {
	// calculate knob center offset relative to slider current value
	// by default we stick knob edge to the edge of the slider track
	CGFloat knob_center_max_offset = m_knob_image.size.width / 2;
	if (knob_center_max_offset >= m_track_rect.origin.x) {
		// if the knob is big enough, then stick it to the control rect edge
		knob_center_max_offset -= m_track_rect.origin.x;
	}
	// this formula essentially means the following:
	//  - near the edges the knob will have corresponding offset from slider exact value, to be able to fit into control rect
	//  - at the middle the offset is zero, meaning the knob positioned exactly where slider is
	//  - the offset changes lineary and flips its sign when knob passes the middle point
	CGFloat knob_center_offset = -(knob_center_max_offset * self.doubleValue) / ((self.maxValue - self.minValue) / 2) + knob_center_max_offset;
	// get current slider value
	double value = (self.doubleValue - self.minValue) / (self.maxValue - self.minValue);
	CGFloat slider_pos = m_track_rect.size.width * value + m_track_rect.origin.x;
	// create knob rect from the center point
	CGFloat knob_center_pos = slider_pos + knob_center_offset;
	NSRect knob_rect = NSMakeRect(knob_center_pos - m_knob_image.size.width / 2, NSMidY(m_track_rect) - m_knob_image.size.height / 2, m_knob_image.size.width, m_knob_image.size.height);
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
		const CGFloat knob_thickness = m_style->knob.thickness;
		m_knob_image = [[NSImage imageWithSize:NSMakeSize(knob_thickness, knob_thickness) flipped:NO drawingHandler:^BOOL(NSRect dst_rect) {
			// TODO: add border
			const CGFloat border_radius = m_style->knob.border_radius;
			NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:dst_rect xRadius:border_radius yRadius:border_radius];
			[NSColorFromHex(m_style->knob.color) setFill];
			[path fill];
			return YES;
		}] retain];
	} else {
		const CGFloat knob_thickness = m_style->track.thickness;
		m_knob_image = [[NSImage alloc] initWithSize:NSMakeSize(knob_thickness, knob_thickness)];
	}
}

- (void)dealloc {
	NSLog(@"debug: slider cell deallocated");
#if !__has_feature(objc_arc)
	[m_knob_image release];
	[super dealloc];
#endif
}

@end


@interface NSStyledSlider ()
{
	NSStyledSliderCell* m_cell;
}
@end

@implementation NSStyledSlider

- (void)setNeedsDisplayInRect:(NSRect)invalid_rect {
	[super setNeedsDisplayInRect:[self bounds]];
}

- (instancetype)initWithStyle:(CSliderStyle*)style {
	self = [super init];
	if (self) {
		[self setWantsLayer:YES];
		// set cell
		m_cell = [[NSStyledSliderCell alloc] initWithStyle:style];
		[self setCell:m_cell];
		// set min and max values
		self.minValue = 0.0;
		self.maxValue = 100.0;
		// set action
		[self setTarget:self];
		[self setAction:@selector(sliderValueChanged:)];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: slider deallocated");
#if !__has_feature(objc_arc)
	[m_cell release];
	[super dealloc];
#endif
}

- (void)updateStyle {
	[m_cell updateStyle];
}

- (void)sliderValueChanged:(NSSlider*)sender {
	NSLog(@"debug: slider value changed: %.2f", sender.doubleValue);
}

@end
