#import "footerpanel.h"

#import "iconpushbutton.h"
#import "utils.h"

@implementation NSFooterPanel

- (instancetype)initWithFrame:(NSRect)frame_rect {
	self = [super initWithFrame:frame_rect];
	if (self) {
		[self setWantsLayer:YES];
		// TODO: rework with skins
		// bg color
		CGColorRef bg_color = [NSColorFromHex(0x313437) CGColor];
		[self.layer setBackgroundColor:bg_color];
		// border radius
		self.layer.cornerRadius = 5.0;
		self.layer.masksToBounds = YES;

		// place buttons
		NSIconPushButton* btn_play = [[NSIconPushButton alloc] initWithFrame:NSMakeRect(8, 5, 30, 30) iconName:@"btn-play"];
		[self addSubview:btn_play];
		[btn_play release];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: footer panel deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
