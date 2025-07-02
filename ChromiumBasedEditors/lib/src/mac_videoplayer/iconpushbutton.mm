#import "iconpushbutton.h"

#import "utils.h"

@interface NSIconPushButton ()
{
	CButtonStyle* m_style;
	bool m_hovered;
	NSString* m_icon_name;
}
@end

@implementation NSIconPushButton

- (instancetype)initWithIconName:(NSString*)icon_name size:(NSSize)size style:(CButtonStyle*)style {
	NSRect init_frame_rect = NSMakeRect(0, 0, size.width, size.height);
	self = [super initWithFrame:init_frame_rect];
	if (self) {
		[self setWantsLayer:YES];
		// apply skin
		m_style = style;
		m_icon_name = icon_name;
		[self updateStyle];
		// add mouse tracking for background color or/and icon changing
		[self addTrackingRect:init_frame_rect owner:self userData:nil assumeInside:NO];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: button deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

- (void)updateStyle {
	self.layer.cornerRadius = m_style->border_radius;
	self.bordered = NO;
	self.layer.backgroundColor = [NSColorFromHex(m_style->bg_color_regular) CGColor];
	[self setIcon:m_icon_name];
}

- (void)setIcon:(NSString*)icon_name {
	m_icon_name = icon_name;
	// TODO: are -2x png icons always suitable on mac platforms ???
	NSString* icon_full_name = [NSString stringWithFormat:@"%@%@%@", icon_name, m_style->icon_postfix, @"-2x"];
	NSString* icon_path = [[NSBundle mainBundle] pathForResource:icon_full_name ofType:@"png"];
	if (icon_path == nil) {
		NSLog(@"Error: could not load icon %@.png", icon_full_name);
		return;
	}
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:icon_path];
	self.image = image;
	self.imageScaling = NSImageScaleProportionallyDown;
#if !__has_feature(objc_arc)
	[image release];
#endif
}

- (void)mouseEntered:(NSEvent*)event {
	m_hovered = true;
	self.layer.backgroundColor = [NSColorFromHex(m_style->bg_color_hovered) CGColor];
}

- (void)mouseExited:(NSEvent*)event {
	m_hovered = false;
	self.layer.backgroundColor = [NSColorFromHex(m_style->bg_color_regular) CGColor];
}

- (void)mouseDown:(NSEvent*)event {
	self.layer.backgroundColor = [NSColorFromHex(m_style->bg_color_pressed) CGColor];
	NSLog(@"debug: button pressed");
}

- (void)mouseUp:(NSEvent*)event {
	CGColor* bg_color = nil;
	if (m_hovered) {
		bg_color = [NSColorFromHex(m_style->bg_color_hovered) CGColor];
	} else {
		bg_color = [NSColorFromHex(m_style->bg_color_regular) CGColor];
	}
	self.layer.backgroundColor = bg_color;
	NSLog(@"debug: button released");
}

@end
