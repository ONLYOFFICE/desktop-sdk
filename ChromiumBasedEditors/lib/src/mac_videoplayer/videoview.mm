#import "videoview.h"

@interface NSVideoView ()
{
	AVPlayerLayer* m_player_layer;
}
@end

@implementation NSVideoView

- (instancetype)initWithFrame:(NSRect)frame_rect player:(AVPlayer*)player superview:(NSView*)parent {
	self = [super initWithFrame:frame_rect];
	if (self) {
		// set parent
		[parent addSubview:self positioned:NSWindowAbove relativeTo:nil];
		// setup layer
		m_player_layer = [AVPlayerLayer playerLayerWithPlayer:player];
		// set black background (transparent by default)
		CGColorRef bg_color = [[NSColor blackColor] CGColor];
		[m_player_layer setBackgroundColor:bg_color];
		[self setWantsLayer:YES];
		[self setLayer:m_player_layer];
	}
	return self;
}

- (void)dealloc {
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
