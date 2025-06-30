#ifndef TIME_LABEL_H_
#define TIME_LABEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSTimeLabel : NSTextField
- (instancetype)initWithSkin:(CFooterSkin*)skin;
- (void)dealloc;

// time label appearance and content
- (void)updateStyle;
- (void)setText:(NSString*)text;
- (NSSize)getBoundingBoxSize;
@end

#endif	// TIME_LABEL_H_
