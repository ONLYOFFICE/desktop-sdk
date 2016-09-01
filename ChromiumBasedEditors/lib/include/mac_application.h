#ifndef CEF_MAC_APPLICATION
#define CEF_MAC_APPLICATION

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include "applicationmanager.h"

@protocol CrAppProtocol
- (BOOL)isHandlingSendEvent;
@end

@protocol CrAppControlProtocol<CrAppProtocol>
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;
@end

@protocol CefAppProtocol<CrAppControlProtocol>
@end

Q_DECL_EXPORT @interface NSAscApplication : NSApplication<CefAppProtocol>

- (void) initAppManager;
- (CAscApplicationManager*) getAppManager;

@end

typedef CAscApplicationManager* (*ASC_CreateAppManagerFunc)();

Q_DECL_EXPORT @interface NSAscApplicationWorker : NSObject

- (id)init;
- (id)initWithCreator:(ASC_CreateAppManagerFunc)creator;
- (void) Start:(int)argc: (const char* [])argv;
- (void) End;
+ (CAscApplicationManager*) getAppManager;

@end

#endif // CEF_MAC_APPLICATION
