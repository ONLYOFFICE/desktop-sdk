#include <Cocoa/Cocoa.h>

#include "include/cef_app.h"
#import "include/cef_application_mac.h"
#include "cefclient/browser/client_app_browser.h"
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_std.h"

#include "../../../graphics/BaseThread.h"

namespace
{
    CAscApplicationManager* g_application_manager = NULL;
}

@interface NSAscApplication ()
{
@private
    BOOL handlingSendEvent_;
    CAscApplicationManager* m_pManager;
    bool m_bIsRunOwner;
}
@end

@implementation NSAscApplication

- (id)init
{
    if (self = [super init])
    {
        m_pManager = [NSAscApplicationWorker getAppManager];
        
        m_bIsRunOwner = false;
    }
    return self;
}

- (BOOL)isHandlingSendEvent
{
    return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent
{
    handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event
{
    CefScopedSendingEvent sendingEventScoper;
    [super sendEvent:event];
    
#if 0
    id _firstResponder = [[NSApp keyWindow] firstResponder];
    //id _firstResponder = nil;
    if ([event type] == NSKeyDown)
    {
        if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
        {
            if ([[event charactersIgnoringModifiers] isEqualToString:@"x"])
            {
                [self sendAction:@selector(cut:) to:_firstResponder from:self];
            }
            else if ([[event charactersIgnoringModifiers] isEqualToString:@"c"])
            {
                [self sendAction:@selector(copy:) to:_firstResponder from:self];
            }
            else if ([[event charactersIgnoringModifiers] isEqualToString:@"v"])
            {
                [self sendAction:@selector(paste:) to:_firstResponder from:self];
            }
            else if ([[event charactersIgnoringModifiers] isEqualToString:@"z"])
            {
                [self sendAction:@selector(undo:) to:_firstResponder from:self];
            }
            else if ([[event charactersIgnoringModifiers] isEqualToString:@"a"])
            {
                [self sendAction:@selector(selectAll:) to:_firstResponder from:self];
            }
        }
        else if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == (NSCommandKeyMask | NSShiftKeyMask))
        {
            if ([[event charactersIgnoringModifiers] isEqualToString:@"Z"])
            {
                [self sendAction:@selector(redo:) to:_firstResponder from:self];
            }
        }
    }
#endif
}

- (void)terminate:(id)sender
{
    id<NSApplicationDelegate> __delegate = [[NSApplication sharedApplication] delegate];
    if (__delegate != nil && ([__delegate respondsToSelector:@selector(applicationShouldTerminate:)]))
    {
        NSApplicationTerminateReply bIsTerminate = [__delegate applicationShouldTerminate:self];
        if (bIsTerminate == NSTerminateCancel)
            return;
    }
    m_pManager->DestroyCefView(-1);
}

- (void) run
{
    if (!m_bIsRunOwner)
    {
        bool _is_runned = false;
        m_bIsRunOwner = true;
        m_pManager->GetApplication()->RunMessageLoop(_is_runned);
        
        if (_is_runned)
            return;
    }
    
    [super run];
}

- (CAscApplicationManager*) getAppManager
{
    return m_pManager;
}

- (void) initAppManager
{
    // init app manager
}

@end

@interface NSAscApplicationWorker ()
{
@private
    NSAutoreleasePool* pool;
}
@end

@implementation NSAscApplicationWorker

- (id)init
{
    if (self = [super init])
    {
        pool = [[NSAutoreleasePool alloc] init];
        
        if (NULL == g_application_manager)
            g_application_manager = new CAscApplicationManager();
        
        // default document path
        NSString* path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject];
        
        NSStringEncoding pEncode    =   CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE );
        NSData* pSData              =   [ path dataUsingEncoding : pEncode ];
        std::wstring sUserPath( (wchar_t*) [ pSData bytes ], [ pSData length] / sizeof ( wchar_t ) );
        
        g_application_manager->m_oSettings.SetUserDataPath(sUserPath);
        
        [NSAscApplication sharedApplication];
    }
    return self;
}

- (id)initWithCreator:(ASC_CreateAppManagerFunc)creator
{
    if (self = [super init])
    {
        pool = [[NSAutoreleasePool alloc] init];
        
        if (NULL == g_application_manager)
            g_application_manager = creator();
        
        // default document path
        NSString* path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject];
        
        NSStringEncoding pEncode    =   CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE );
        NSData* pSData              =   [ path dataUsingEncoding : pEncode ];
        std::wstring sUserPath( (wchar_t*) [ pSData bytes ], [ pSData length] / sizeof ( wchar_t ) );
        
        g_application_manager->m_oSettings.SetUserDataPath(sUserPath);
        
        [NSAscApplication sharedApplication];
    }
    return self;
}

- (void) Start:(int)argc: (const char* [])argv;
{
    CApplicationCEF* application_cef = new CApplicationCEF();
    
    char** argv_copy = NULL;
    if (0 != argc)
    {
        argv_copy = new char*[argc];
        for (int i = 0; i < argc; ++i)
        {
            int len = (int)strlen(argv[i]) + 1;
            argv_copy[i] = new char[len];
            memcpy(argv_copy[i], argv[i], len);
        }
    }
    
    application_cef->Init_CEF(g_application_manager, argc, argv_copy);
    
    if (NULL != argv_copy)
    {
        for (int i = 0; i < argc; ++i)
        {
            delete [] argv_copy[i];
        }
        delete [] argv_copy;
    }
    g_application_manager->CheckFonts();
    g_application_manager->StartSpellChecker();
    g_application_manager->StartKeyboardChecker();
}
- (void) End
{
    if (g_application_manager)
    {
        g_application_manager->CloseApplication();
    
        delete g_application_manager->GetApplication();
        delete g_application_manager;
    
        g_application_manager = NULL;
    }
    [pool release];
}

+ (CAscApplicationManager*) getAppManager
{
    return g_application_manager;
}

@end
