#ifndef ICON_PUSH_BUTTON_H_
#define ICON_PUSH_BUTTON_H_

#import <Cocoa/Cocoa.h>

@interface NSIconPushButton : NSButton
- (instancetype)initWithIconName:(NSString*)icon_name size:(NSSize)size;
- (void)dealloc;

- (void)setIcon:(NSString*)icon_name;

// events
- (void)mouseEntered:(NSEvent*)event;
- (void)mouseExited:(NSEvent*)event;
- (void)mouseDown:(NSEvent*)event;
- (void)mouseUp:(NSEvent*)event;
@end

#endif	// ICON_PUSH_BUTTON_H_
