/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#if defined(_LINUX) && !defined(_MAC)

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <X11/Xlib.h>

#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h
#undef RootWindow
#undef None

#include "include/wrapper/cef_helpers.h"

int XErrorHandlerImpl(Display *display, XErrorEvent *event)
{
    LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
    return 0;
}

int XIOErrorHandlerImpl(Display *display)
{
    return 0;
}

#endif

#include "../../../../core/DesktopEditor/common/File.h"
#include "../src/applicationmanager_p.h"

#include "cefwrapper/client_app.h"
#include "cefwrapper/client_scheme.h"

#ifdef CEF_2623
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/main_message_loop_std.h"
#else
#include "tests/cefclient/browser/main_context_impl.h"

#ifdef WIN32
#include "tests/cefclient/browser/main_message_loop_multithreaded_win.h"
#endif
#include "tests/shared/browser/main_message_loop_std.h"
#endif

#include "./plugins.h"

class CApplicationCEF_Private
{
public:
    CefRefPtr<client::ClientApp> m_app;
    scoped_ptr<client::MainContextImpl> context;
    scoped_ptr<client::MainMessageLoop> message_loop;

    CAscApplicationManager* m_pManager;

    int m_nReturnCodeInitCef;

#if defined(_LINUX) && !defined(_MAC)
    CefScopedArgArray* argc_copy;
#endif

    CApplicationCEF_Private()
    {
        m_nReturnCodeInitCef = -1;
#if defined(_LINUX) && !defined(_MAC)
        argc_copy = NULL;
#endif

        m_pManager = NULL;
    }
    ~CApplicationCEF_Private()
    {
        message_loop.reset();
        context.reset();

#if defined(_LINUX) && !defined(_MAC)
        RELEASEOBJECT(argc_copy);
#endif
    }
};

CApplicationCEF::CApplicationCEF()
{
    m_pInternal = new CApplicationCEF_Private();
}

#if defined(_LINUX) && !defined(_MAC)
//#define CEF_USE_SANDBOX
#endif

int CApplicationCEF::Init_CEF(CAscApplicationManager* pManager, int argc, char* argv[])
{
#if 0
    FILE* f = fopen("D:\\logloglog.txt", "a+");
    fprintf(f, "-----------------------------------------------\n");
    for (int i = 0; i < argc; ++i)
    {
        fprintf(f, argv[i]);
        fprintf(f, "\n");
    }
    fprintf(f, "-----------------------------------------------\n");
    fclose(f);
#endif
    m_pInternal->m_pManager = pManager;

    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

#if defined(_LINUX) && !defined(_MAC)
    // Create a copy of |argv| on Linux because Chromium mangles the value
    // internally (see issue #620).
    m_pInternal->argc_copy = new CefScopedArgArray(argc, argv);
    char** argv_copy = m_pInternal->argc_copy->array();
#endif

#ifdef WIN32
    CefMainArgs main_args((HINSTANCE)GetModuleHandle(NULL));
    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());
#endif

#if defined(_LINUX) && !defined(_MAC)
    CefMainArgs main_args(argc, argv_copy);

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv_copy);
#endif

#ifdef _MAC
    CefMainArgs main_args(argc, argv);

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv);
#endif

    client::ClientApp::ProcessType process_type = client::ClientApp::GetProcessType(command_line);

    if (process_type == client::ClientApp::BrowserProcess)
    {
        // ASC command line props
        pManager->m_pInternal->LoadSettings();
        for (int i = 0; i < argc; ++i)
        {
            std::string sCommandLine(argv[i]);
            
            std::string::size_type posSeparate = sCommandLine.find('=');
            
            std::string sName = (std::string::npos == posSeparate) ? sCommandLine : sCommandLine.substr(0, posSeparate);
            std::string sValue = (std::string::npos == posSeparate) ? "" : sCommandLine.substr(posSeparate + 1);
            
            pManager->m_pInternal->CheckSetting(sName, sValue);
        }
        pManager->m_pInternal->SaveSettings();
    }

#ifdef WIN32
    // Create a ClientApp of the correct type.
    if (process_type == client::ClientApp::BrowserProcess)
        m_pInternal->m_app = new CAscClientAppBrowser(pManager->m_pInternal->m_mapSettings);
    else if (process_type == client::ClientApp::RendererProcess ||
             process_type == client::ClientApp::ZygoteProcess)
        m_pInternal->m_app = new CAscClientAppRenderer(pManager->m_pInternal->m_mapSettings);
    else if (process_type == client::ClientApp::OtherProcess)
        m_pInternal->m_app = new CAscClientAppOther(pManager->m_pInternal->m_mapSettings);
#endif

#if defined(_LINUX) && !defined(_MAC)
    // Create a ClientApp of the correct type.    
    if (process_type == client::ClientApp::BrowserProcess)
        m_pInternal->m_app = new CAscClientAppBrowser(pManager->m_pInternal->m_mapSettings);
    else if (process_type == client::ClientApp::RendererProcess ||
             process_type == client::ClientApp::ZygoteProcess)
        m_pInternal->m_app = new CAscClientAppRenderer(pManager->m_pInternal->m_mapSettings);
    else if (process_type == client::ClientApp::OtherProcess)
        m_pInternal->m_app = new CAscClientAppOther(pManager->m_pInternal->m_mapSettings);
#endif

#ifdef _MAC
    m_pInternal->m_app = new CAscClientAppBrowser(pManager->m_pInternal->m_mapSettings);
#endif

#if 1
    // Execute the secondary process, if any.
    m_pInternal->m_nReturnCodeInitCef = CefExecuteProcess(main_args, m_pInternal->m_app.get(), NULL);
    if (m_pInternal->m_nReturnCodeInitCef >= 0)
    {        
        return m_pInternal->m_nReturnCodeInitCef;
    }
#endif

    CefSettings settings;
    
#ifdef _MAC
#if 1
    std::wstring sSubprocessPath = NSFile::GetProcessDirectory();
    std::wstring sName = NSCommon::GetFileName(NSFile::GetProcessPath());
    sSubprocessPath += L"/../Frameworks/ONLYOFFICE Helper.app/Contents/MacOS/ONLYOFFICE Helper";
    
    cef_string_t _subprocess;
    memset(&_subprocess, 0, sizeof(_subprocess));
    cef_string_from_wide(sSubprocessPath.c_str(), sSubprocessPath.length(), &_subprocess);
    settings.browser_subprocess_path = _subprocess;
    
    // second variant
    //CefString(&settings.browser_subprocess_path).FromWString(sSubprocessPath);
#endif
#endif

#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

#if 0
    settings.single_process = 1;
#endif

    // Populate the settings based on command line arguments.
    m_pInternal->context.reset(new client::MainContextImpl(command_line, false));
    m_pInternal->context->PopulateSettings(&settings);

#ifdef WIN32
    if (!m_pInternal->m_pManager->IsExternalEventLoop())
        settings.multi_threaded_message_loop = 1;
    //settings.windowless_rendering_enabled = 1;
#endif

    if (/*!m_pInternal->m_pManager->IsExternalEventLoop()*/true)
    {
#ifdef WIN32
        if (settings.multi_threaded_message_loop)
            m_pInternal->message_loop.reset(new client::MainMessageLoopMultithreadedWin);
        else
            m_pInternal->message_loop.reset(new client::MainMessageLoopStd);
#else
        m_pInternal->message_loop.reset(new client::MainMessageLoopStd);
#endif
    }

    std::wstring sCachePath = pManager->m_oSettings.cache_path;

    cef_string_t _cache;
    memset(&_cache, 0, sizeof(_cache));
    cef_string_from_wide(sCachePath.c_str(), sCachePath.length(), &_cache);
    settings.cache_path = _cache;

    settings.persist_session_cookies = true;

    // Initialize CEF.
    bool bInit = m_pInternal->context->Initialize(main_args, settings, m_pInternal->m_app.get(), NULL);
    bool bIsInitScheme = asc_scheme::InitScheme(pManager);

#if defined(_LINUX) && !defined(_MAC)
    // The Chromium sandbox requires that there only be a single thread during
    // initialization. Therefore initialize GTK after CEF.
    gtk_init(&argc, &argv);

    // Perform gtkglext initialization required by the OSR example.
    gtk_gl_init(&argc, &argv);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors. Must be done after initializing GTK.
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

    pManager->SetApplication(this);

    CPluginsManager oPlugins;
    oPlugins.m_strDirectory = pManager->m_oSettings.system_plugins_path;
    oPlugins.m_strUserDirectory = pManager->m_oSettings.user_plugins_path;
    oPlugins.GetInstalledPlugins();

    for (std::map<int, std::string>::iterator iterCrypto = oPlugins.m_arCryptoModes.begin(); iterCrypto != oPlugins.m_arCryptoModes.end(); iterCrypto++)
    {
        NSAscCrypto::CAscCryptoJsonValue value;
        value.m_eType = (NSAscCrypto::AscCryptoType)iterCrypto->first;
        value.m_sGuid = iterCrypto->second;
        pManager->m_pInternal->m_mapCrypto.insert(std::pair<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>(value.m_eType, value));
    }

    for (std::vector<std::string>::iterator iterExt = oPlugins.m_arExternals.begin(); iterExt != oPlugins.m_arExternals.end(); iterExt++)
    {
        pManager->m_pInternal->m_arExternalPlugins.push_back(*iterExt);
    }

    pManager->m_pInternal->LoadCryptoData();

#ifdef WIN32
    SetEnvironmentVariableA("APPLICATION_NAME", pManager->m_oSettings.converter_application_name.c_str());
    SetEnvironmentVariableA("COMPANY_NAME", pManager->m_oSettings.converter_application_company.c_str());
#else
    std::string s1 = "APPLICATION_NAME=" + pManager->m_oSettings.converter_application_name;
    std::string s2 = "COMPANY_NAME=" + pManager->m_oSettings.converter_application_company;
    putenv((char*)s1.c_str());
    putenv((char*)s2.c_str());
#endif

    return 0;
}

CApplicationCEF::~CApplicationCEF()
{
    Close();
}

void CApplicationCEF::Close()
{
    if (NULL != m_pInternal)
    {
        if (m_pInternal->context)
            m_pInternal->context->Shutdown();
        RELEASEOBJECT(m_pInternal);
    }
}

int CApplicationCEF::RunMessageLoop(bool& is_runned)
{
    if (m_pInternal->m_pManager->IsExternalEventLoop())
        return 0;

    is_runned = true;
#ifdef LINUX
    CLinuxData::Check(m_pInternal->m_pManager);
#endif
    return m_pInternal->message_loop->Run();
}

void CApplicationCEF::DoMessageLoopEvent()
{
    CefDoMessageLoopWork();
}

bool CApplicationCEF::ExitMessageLoop()
{
    if (!this->m_pInternal->m_pManager->IsExternalEventLoop())
        m_pInternal->message_loop->Quit();
    else
        m_pInternal->m_pManager->ExitExternalEventLoop();
    return true;
}

bool CApplicationCEF::IsChromiumSubprocess()
{
    return (m_pInternal->m_nReturnCodeInitCef >= 0) ? true : false;
}

bool CApplicationCEF::IsMainProcess(int argc, char* argv[])
{
    bool bIsMain = true;
#ifdef WIN32
    std::string sCommandLine = GetCommandLineA();
    if (sCommandLine.find("--type") != std::string::npos)
        bIsMain = false;
#else
    for (int i = 0; i < argc; ++i)
    {
        std::string sCommandLine(argv[i]);
        if (sCommandLine.find("--type") != std::string::npos)
        {
            bIsMain = false;
            break;
        }
    }
#endif
    return bIsMain;
}
