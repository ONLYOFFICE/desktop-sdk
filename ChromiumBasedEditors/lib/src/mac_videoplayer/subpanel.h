#ifndef FOOTER_SUB_PANEL_H_
#define FOOTER_SUB_PANEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSSubPanel : NSView<StyledView>
- (instancetype)initWithStyle:(CFooterStyle*)style;
@end

#endif	// FOOTER_SUB_PANEL_H_
