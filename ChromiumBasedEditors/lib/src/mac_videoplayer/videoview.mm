#import "videoview.h"

@interface NSVideoView () {
	AVPlayerLayer* player_layer;
}
@end


@implementation NSVideoView

- (instancetype)initWithFrame:(NSRect)frame_rect player:(AVPlayer*)player {
	self = [super initWithFrame:frame_rect];
	if (self) {
		player_layer = [AVPlayerLayer playerLayerWithPlayer:player];
		// set black background (transparent by default)
		CGColorRef bg_color = [[NSColor blackColor] CGColor];
		[player_layer setBackgroundColor:bg_color];
		[self setWantsLayer:YES];
		[self setLayer:player_layer];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: video view deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
