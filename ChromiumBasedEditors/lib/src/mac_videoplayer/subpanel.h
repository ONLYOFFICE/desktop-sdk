#ifndef FOOTER_SUB_PANEL_H_
#define FOOTER_SUB_PANEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSSubPanel : NSView
- (instancetype)initWithStyle:(CFooterStyle*)style;
// appearance
- (void)updateStyle;
@end

#endif	// FOOTER_SUB_PANEL_H_
