#ifndef VIDEOPLAYER_SLIDER_H_
#define VIDEOPLAYER_SLIDER_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSStyledSlider : NSSlider
- (instancetype)initWithStyle:(CSliderStyle*)style vertical:(BOOL)vertical;
- (void)dealloc;

// slider appearance
- (void)updateStyle;

// test action function
- (void)sliderValueChanged:(NSSlider*)sender;
@end

#endif	// VIDEOPLAYER_SLIDER_H_
