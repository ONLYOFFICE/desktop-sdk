#ifndef VIDEOPLAYER_SLIDER_H_
#define VIDEOPLAYER_SLIDER_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSStyledSlider : NSSlider<StyledView>
- (instancetype)initWithStyle:(CSliderStyle*)style vertical:(BOOL)vertical;
@end

#endif	// VIDEOPLAYER_SLIDER_H_
