/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#include <Cocoa/Cocoa.h>

#include "include/cef_app.h"
#import "include/cef_application_mac.h"
#include "tests/shared/browser/client_app_browser.h"
#include "tests/cefclient/browser/main_context_impl.h"
#include "tests/shared/browser/main_message_loop_std.h"

#include "../../../../core/DesktopEditor/graphics/BaseThread.h"

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

- (void) Start:(int)argc argv:(const char* [])argv
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
