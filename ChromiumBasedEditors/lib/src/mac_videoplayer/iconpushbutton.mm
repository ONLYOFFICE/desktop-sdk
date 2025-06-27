#import "iconpushbutton.h"

#import "utils.h"

@implementation NSIconPushButton

- (instancetype)initWithIconName:(NSString*)icon_name size:(NSSize)size {
	NSRect init_frame_rect = NSMakeRect(0, 0, size.width, size.height);
	self = [super initWithFrame:init_frame_rect];
	if (self) {
		[self setWantsLayer:YES];
		self.layer.cornerRadius = 3.0;
		self.bordered = NO;
		self.layer.backgroundColor = [NSColorFromHex(0x313437) CGColor];
		[self setIcon:icon_name];
		// add mouse tracking for background color or/and icon changing
		[self addTrackingRect:init_frame_rect owner:self userData:nil assumeInside:NO];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: icon button deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

- (void)setIcon:(NSString*)icon_name {
	// TODO: rework with skins
	NSString* icon_full_name = [NSString stringWithFormat:@"%@%@", icon_name, @"-dark-2x"];
	NSString* icon_path = [[NSBundle mainBundle] pathForResource:icon_full_name ofType:@"png"];
	if (icon_path == nil) {
		NSLog(@"Error: could not load icon %@.png", icon_full_name);
		return;
	}
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:icon_path];
	self.image = image;
	self.imageScaling = NSImageScaleProportionallyDown;
	[image release];
}

- (void)mouseEntered:(NSEvent*)event {
	self.layer.backgroundColor = [NSColorFromHex(0x46494B) CGColor];
}

- (void)mouseExited:(NSEvent*)event {
	self.layer.backgroundColor = [NSColorFromHex(0x313437) CGColor];
}

- (void)mouseDown:(NSEvent*)event {
	NSLog(@"debug: button pressed");
}

- (void)mouseUp:(NSEvent*)event {
	NSLog(@"debug: button released");
}

@end
