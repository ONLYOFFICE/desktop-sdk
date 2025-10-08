#ifndef TIME_LABEL_H_
#define TIME_LABEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSTimeLabel : NSTextField<StyledView>
- (instancetype)initWithStyle:(CTimeLabelStyle*)style;

- (void)setTime:(double)time_sec;
// get bounding box size for longest possible time
- (NSSize)getBoundingBoxSize;
@end

#endif	// TIME_LABEL_H_
