#ifndef ICON_PUSH_BUTTON_H_
#define ICON_PUSH_BUTTON_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"

@interface NSIconPushButton : NSButton
- (instancetype)initWithIconName:(NSString*)icon_name size:(NSSize)size skin:(CFooterSkin*)skin;
- (void)dealloc;

// button appearance
- (void)updateStyle;
- (void)setIcon:(NSString*)icon_name;

// events
- (void)mouseEntered:(NSEvent*)event;
- (void)mouseExited:(NSEvent*)event;
- (void)mouseDown:(NSEvent*)event;
- (void)mouseUp:(NSEvent*)event;
@end

#endif	// ICON_PUSH_BUTTON_H_
