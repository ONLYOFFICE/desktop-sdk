#ifndef VIDEOPLAYER_SLIDER_H_
#define VIDEOPLAYER_SLIDER_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSStyledSlider : NSSlider<StyledView>
- (instancetype)initWithStyle:(CSliderStyle*)style vertical:(BOOL)vertical;
// actions
- (void)setActionPress:(SEL)action;
- (void)setActionRelease:(SEL)action;
@end

#endif	// VIDEOPLAYER_SLIDER_H_
