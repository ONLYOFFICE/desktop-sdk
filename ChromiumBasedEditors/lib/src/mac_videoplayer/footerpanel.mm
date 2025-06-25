#import "footerpanel.h"

@implementation NSFooterPanel

- (instancetype)initWithFrame:(NSRect)frame_rect {
	self = [super initWithFrame:frame_rect];
	if (self) {
		[self setWantsLayer:YES];
		// TODO: rework
		self.layer.cornerRadius = 8.0;
		self.layer.masksToBounds = YES;
		CGColorRef bg_color = [[NSColor redColor] CGColor];
		[self.layer setBackgroundColor:bg_color];
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
