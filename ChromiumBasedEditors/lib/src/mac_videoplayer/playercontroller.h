#ifndef PLAYER_CONTROLLER_BRIDGE_H_
#define PLAYER_CONTROLLER_BRIDGE_H_

#import <AVFoundation/AVFoundation.h>

#import "footerpanel.h"

/*
 * This class is designed to connect UI events (button pressed, slider moved, etc.) with player,
 * as well as player events (playback rage changed, seek changed) with UI.
 */
@interface NSPlayerControllerBridge : NSObject
- (instancetype)initWithPlayer:(AVPlayer*)player footer:(NSFooterPanel*)footer;
- (void)removeObservers;
// media
- (BOOL)setMedia:(NSString*)media_path;
// playback functions
- (void)play;
- (void)pause;
- (void)togglePause;
- (void)setVolume:(CGFloat)value;
- (void)toggleMute;
- (void)stop;
@end

#endif	// PLAYER_CONTROLLER_BRIDGE_H_
