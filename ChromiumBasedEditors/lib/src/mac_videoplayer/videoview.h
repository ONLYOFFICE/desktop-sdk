#ifndef VIDEO_VIEW_H_
#define VIDEO_VIEW_H_

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface NSVideoView : NSView
{
	AVPlayerLayer* player_layer;
}
- (instancetype)initWithFrame:(NSRect)frame_rect player:(AVPlayer*)player;
- (void)dealloc;
@end

#endif	// VIDEO_VIEW_H_
