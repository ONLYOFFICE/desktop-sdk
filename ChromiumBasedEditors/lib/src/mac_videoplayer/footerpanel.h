#ifndef FOOTER_PANEL_H_
#define FOOTER_PANEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSFooterPanel : NSView<StyledView>
- (instancetype)initWithFrame:(NSRect)frame_rect superview:(NSView*)parent;
- (void)dealloc;

- (void)applySkin:(CFooterSkin::Type)type;
@end

#endif	// FOOTER_PANEL_H_
