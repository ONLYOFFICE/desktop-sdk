#ifndef FOOTER_PANEL_H_
#define FOOTER_PANEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSFooterPanel : NSView
- (instancetype)initWithFrame:(NSRect)frame_rect;
- (void)dealloc;

- (void)applySkin:(CFooterSkin::Type)type;
@end

#endif	// FOOTER_PANEL_H_
