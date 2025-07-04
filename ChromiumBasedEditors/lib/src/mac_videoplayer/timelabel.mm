#import "timelabel.h"

#import "utils.h"

@interface NSTimeLabel ()
{
	CTimeLabelStyle* m_style;
	NSDictionary* m_attributes;
	NSSize m_bounding_box_size;
}
- (void)setText:(NSString*)text;
@end

@implementation NSTimeLabel

- (instancetype)initWithStyle:(CTimeLabelStyle*)style {
	self = [super init];
	if (self) {
		[self setWantsLayer:YES];
		// apply skin
		m_style = style;
		[self updateStyle];
		// set initial time
		[self setText:@"00:00:00"];
		// set text properties
		[self setEditable:NO];
		[self setBezeled:NO];
		[self setSelectable:NO];
		[self setDrawsBackground:NO];
		[self setBordered:NO];
	}
	return self;
}

- (void)setText:(NSString*)text {
	NSLog(@"debug: set text: %@", text);
	NSAttributedString* attr_string = [[NSAttributedString alloc] initWithString:NSLocalizedString(text, nil) attributes:m_attributes];
	[self setAttributedStringValue:attr_string];
#if !__has_feature(objc_arc)
	[attr_string release];
#endif
}

- (void)updateStyle {
	// get font
	CGFloat font_size = m_style->font_size;
	NSFont* font = nil;
	if (m_style->font_name.length == 0) {
		font = [NSFont systemFontOfSize:font_size];
	} else {
		font = [NSFont fontWithName:m_style->font_name size:font_size];
		if (!font) {
			font = [NSFont systemFontOfSize:font_size];
		}
	}
	// get color
	NSColor* color = NSColorFromHex(m_style->color);
	// update attributes
	m_attributes = @{
		NSFontAttributeName: font,
		NSForegroundColorAttributeName: color
	};
	// update text bounding box size
	m_bounding_box_size = [@"00:00:00" sizeWithAttributes:m_attributes];
	m_bounding_box_size.width += 2;		// add extra couple of pixels
	// update current text
	[self setText:self.stringValue];
}

- (void)setTime:(double)time_sec {
	// calculate hours
	unsigned hours = (unsigned)(time_sec / 3600);
	time_sec -= hours * 3600;
	// calculate minutes
	unsigned minutes = (unsigned)(time_sec / 60);
	time_sec -= (minutes * 60);
	// calculate seconds
	unsigned seconds = (unsigned)(time_sec);
	// set label text
	NSString* time_str = [NSString stringWithFormat:@"%02u:%02u:%02u", hours, minutes, seconds];
	[self setText:time_str];
}

- (NSSize)getBoundingBoxSize {
	return m_bounding_box_size;
}

- (void)dealloc {
	NSLog(@"debug: time label deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
