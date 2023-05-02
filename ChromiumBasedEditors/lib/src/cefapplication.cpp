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

#if defined(_LINUX) && !defined(_MAC)

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <X11/Xlib.h>

#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h
#undef RootWindow
#undef None
#undef Always

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

#include <memory>

#include "../../../../core/DesktopEditor/common/File.h"
#include "../src/applicationmanager_p.h"

#include "cefwrapper/client_app.h"
#include "cefwrapper/client_scheme.h"

#ifdef CEF_2623
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/main_message_loop_std.h"
namespace client {
    typedef MainMessageLoopMultithreadedWin MainMessageLoopMultithreaded;
}
#else
#include "tests/cefclient/browser/main_context_impl.h"
#include "tests/shared/browser/main_message_loop_std.h"
#include "tests/shared/browser/main_message_loop_external_pump.h"

class CMainMessageLoopExternalPumpBase : public client::MainMessageLoopExternalPump
{
private:
    bool timer_pending_;
    IExternalMessageLoop* message_loop_;

public:
    CMainMessageLoopExternalPumpBase(IExternalMessageLoop* native_loop) : client::MainMessageLoopExternalPump()
    {
        timer_pending_ = false;
        message_loop_ = native_loop;
    }
    ~CMainMessageLoopExternalPumpBase()
    {
        message_loop_->KillTimer();
    }

    virtual void Quit() OVERRIDE
    {
        message_loop_->Exit();
    }
    virtual int Run() OVERRIDE
    {
        message_loop_->Run();

        for (int i = 0; i < 10; ++i)
        {
            // Do some work.
            CefDoMessageLoopWork();

            // Sleep to allow the CEF proc to do work.
            NSThreads::Sleep(50);
        }

        return 0;
    }

    void OnScheduleMessagePumpWork(int64 delay_ms) OVERRIDE
    {
        message_loop_->Execute(new int64(delay_ms));
    }

#ifdef CEF_VERSION_ABOVE_102
	static std::unique_ptr<client::MainMessageLoopExternalPump> Create(IExternalMessageLoop* native_loop)
    {
		return std::make_unique<CMainMessageLoopExternalPumpBase>(native_loop);
    }
#else
	static scoped_ptr<client::MainMessageLoopExternalPump> Create(IExternalMessageLoop* native_loop)
	{
		return scoped_ptr<client::MainMessageLoopExternalPump>(new CMainMessageLoopExternalPumpBase(native_loop));
	}
#endif

public:
    void OnExecute(void* message)
    {
        int64* addr = (int64*)message;
        const int64 delay_ms = *addr;
        delete addr;
        OnScheduleWork(delay_ms);

    }
    void OnTimer()
    {
        OnTimerTimeout();
    }

protected:
    virtual bool IsTimerPending() OVERRIDE
    {
        return timer_pending_;
    }

    virtual void SetTimer(int64 delay_ms) OVERRIDE
    {
        timer_pending_ = true;
        message_loop_->SetTimer(delay_ms);
    }
    virtual void KillTimer() OVERRIDE
    {
        message_loop_->KillTimer();
        timer_pending_ = false;
    }
};

#ifdef WIN32
#include "tests/cefclient/browser/main_message_loop_multithreaded_win.h"
namespace client {
    typedef MainMessageLoopMultithreadedWin MainMessageLoopMultithreaded;
}
#endif
#if defined(_LINUX) && !defined(_MAC)
#include "tests/cefclient/browser/main_message_loop_multithreaded_gtk.h"
namespace client {
    typedef MainMessageLoopMultithreadedGtk MainMessageLoopMultithreaded;
}
#endif
#if defined(_MAC)
namespace client {
    typedef MainMessageLoopStd MainMessageLoopMultithreaded;
}

#include "include/wrapper/cef_library_loader.h"

#endif
#endif

#include "./plugins.h"

class CApplicationCEF_Private
{
public:
    CefRefPtr<client::ClientApp> m_app;

	bool m_bIsMessageLoopRunned;
#ifdef CEF_VERSION_ABOVE_102
	std::unique_ptr<client::MainContextImpl> context;
	std::unique_ptr<client::MainMessageLoop> message_loop;
#else
	scoped_ptr<client::MainContextImpl> context;
	scoped_ptr<client::MainMessageLoop> message_loop;
#endif

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
        m_bIsMessageLoopRunned = false;
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
    LOGGER_STRING("CApplicationCEF::Init_CEF::start");

#ifdef _MAC
    bool bIsHelper = false;
    for (int arg_index = 0; arg_index < argc; ++arg_index)
    {
        std::string sHelperTest(argv[arg_index]);
        if (0 == sHelperTest.find("--type="))
        {
            bIsHelper = true;
            break;
        }
    }

    CefScopedLibraryLoader library_loader;
    if (bIsHelper)
        library_loader.LoadInHelper();
    else
        library_loader.LoadInMain();
#endif

    if (NULL == pManager->m_pInternal->m_pDpiChecker)
        pManager->m_pInternal->m_pDpiChecker = pManager->InitDpiChecker();

#if 0
    for (int i = 0; i < argc; ++i)
    {
        LOGGER_STRING2(argv[i]);
    }
#endif

    m_pInternal->m_pManager = pManager;

#ifdef WIN32
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();
#endif

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

#if defined(_WIN32) && !defined(_DEBUG)
        NSSystem::SetEnvValue("PATH", NSFile::GetProcessDirectory() + L"\\converter;%PATH%");
#endif
    }

    if (pManager->m_pInternal->m_bExperimentalFeatures)
    {
        pManager->m_oSettings.pass_support = true;
        pManager->m_oSettings.sign_support = true;
        pManager->m_oSettings.protect_support = true;
    }

    switch (process_type)
    {
        case client::ClientApp::BrowserProcess:
        {
			m_pInternal->m_app = new CAscClientAppBrowser(pManager->m_pInternal->m_mapSettings, pManager);
            break;
        }
        case client::ClientApp::RendererProcess:
        case client::ClientApp::ZygoteProcess:
        {
            m_pInternal->m_app = new CAscClientAppRenderer(pManager->m_pInternal->m_mapSettings);
            break;
        }
        case client::ClientApp::OtherProcess:
        default:
        {
            m_pInternal->m_app = new CAscClientAppOther(pManager->m_pInternal->m_mapSettings);
            break;
        }
    }

    // Execute the secondary process, if any.
    m_pInternal->m_nReturnCodeInitCef = CefExecuteProcess(main_args, m_pInternal->m_app.get(), NULL);
    if (m_pInternal->m_nReturnCodeInitCef >= 0)
    {        
        return m_pInternal->m_nReturnCodeInitCef;
    }

    LOGGER_STRING("CApplicationCEF::Init_CEF::main");

    CefSettings settings;

    std::wstring sHelperPath = NSFile::GetProcessDirectory();
#ifdef _MAC
    sHelperPath += L"/../Frameworks/editors_helper.app/Contents/MacOS";
#endif
    sHelperPath += L"/editors_helper";
#ifdef _WIN32
    sHelperPath += L".exe";
#endif

    cef_string_t _subprocess;
    memset(&_subprocess, 0, sizeof(_subprocess));
    cef_string_from_wide(sHelperPath.c_str(), sHelperPath.length(), &_subprocess);
    settings.browser_subprocess_path = _subprocess;

#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    // Populate the settings based on command line arguments.
#if defined(CEF_VERSION_ABOVE_102)
	m_pInternal->context = std::make_unique<client::MainContextImpl>(command_line, false);
#elif defined(CEF_2623)
	m_pInternal->context.reset(new client::MainContextImpl(command_line, false));
#else
	m_pInternal->context = new client::MainContextImpl(command_line, false);
#endif

    m_pInternal->context->PopulateSettings(&settings);

    bool isMultithreaded = false;
#ifdef WIN32
    isMultithreaded = true;
#endif
#if defined(_LINUX) && !defined(_MAC)
    //isMultithreaded = true;
#endif

    pManager->SetApplication(this);

    if (m_pInternal->m_pManager->IsExternalEventLoop())
    {
#ifndef CEF_2623
        settings.external_message_pump = 1;
#endif
    }
    else if (isMultithreaded)
    {
        settings.multi_threaded_message_loop = 1;
#if defined(CEF_VERSION_ABOVE_102)
		m_pInternal->message_loop = std::make_unique<client::MainMessageLoopMultithreaded>();
#elif defined(CEF_2623)
		m_pInternal->message_loop.reset(new client::MainMessageLoopMultithreaded);
#else
		m_pInternal->message_loop = new client::MainMessageLoopMultithreaded();
#endif
    }
    else
    {
#if defined(CEF_VERSION_ABOVE_102)
		m_pInternal->message_loop = std::make_unique<client::MainMessageLoopStd>();
#elif defined(CEF_2623)
		m_pInternal->message_loop.reset(new client::MainMessageLoopStd);
#else
		m_pInternal->message_loop = new client::MainMessageLoopStd();
#endif
    }

    std::wstring sCachePath = pManager->m_oSettings.cache_path;

    cef_string_t _cache;
    memset(&_cache, 0, sizeof(_cache));
    cef_string_from_wide(sCachePath.c_str(), sCachePath.length(), &_cache);
    settings.cache_path = _cache;

    std::wstring sCachePathUser = sCachePath;
    cef_string_t _cache_user;
    memset(&_cache_user, 0, sizeof(_cache_user));
    cef_string_from_wide(sCachePathUser.c_str(), sCachePathUser.length(), &_cache_user);
    settings.user_data_path = _cache_user;

    std::wstring sCachePathLog = sCachePath + L"/log.log";
    cef_string_t _cache_log;
    memset(&_cache_log, 0, sizeof(_cache_log));
    cef_string_from_wide(sCachePathLog.c_str(), sCachePathLog.length(), &_cache_log);
    settings.log_file = _cache_log;
    settings.log_severity = LOGSEVERITY_DISABLE;

    settings.persist_session_cookies = true;

    // Initialize CEF.
    m_pInternal->context->Initialize(main_args, settings, m_pInternal->m_app.get(), NULL);
    asc_scheme::InitScheme(pManager);

    LOGGER_STRING("CApplicationCEF::Init_CEF::initialize");

#if defined(_LINUX) && !defined(_MAC)
    // The Chromium sandbox requires that there only be a single thread during
    // initialization. Therefore initialize GTK after CEF.
    gtk_init(&argc, &argv);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors. Must be done after initializing GTK.
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

	Display* pDisplay = (Display*)CefGetXDisplay();
	XSynchronize(pDisplay, True);
#endif

    CPluginsManager oPlugins;
    oPlugins.m_strDirectory = pManager->m_oSettings.system_plugins_path;
    oPlugins.m_strUserDirectory = pManager->m_oSettings.user_plugins_path;
    oPlugins.GetInstalledPlugins();

    bool bIsCurrentCryptoPresent = false;
    for (std::map<int, std::string>::iterator iterCrypto = oPlugins.m_arCryptoModes.begin(); iterCrypto != oPlugins.m_arCryptoModes.end(); iterCrypto++)
    {
        NSAscCrypto::CAscCryptoJsonValue value;
        value.m_eType = (NSAscCrypto::AscCryptoType)iterCrypto->first;
        value.m_sGuid = iterCrypto->second;
        pManager->m_pInternal->m_mapCrypto.insert(std::pair<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>(value.m_eType, value));

        if (!bIsCurrentCryptoPresent && (pManager->m_pInternal->m_nCurrentCryptoMode == value.m_eType))
            bIsCurrentCryptoPresent = 0;
    }
    // смотрим, есть ли плагин для выставленного режима криптования
    if (!bIsCurrentCryptoPresent || oPlugins.m_bCryptoDisabledOnStart)
    {
        pManager->m_pInternal->m_nCurrentCryptoMode = NSAscCrypto::None;
        pManager->m_pInternal->CheckSetting("--crypto-mode", "default");
        pManager->m_pInternal->SaveSettings();
    }

    pManager->m_pInternal->m_bCryptoDisableForLocal = oPlugins.m_bCryptoDisableForLocal;
    pManager->m_pInternal->m_bCryptoDisableForInternalCloud = oPlugins.m_bCryptoDisableForInternalCloud;
    pManager->m_pInternal->m_bCryptoDisableForExternalCloud = oPlugins.m_bCryptoDisableForExternalCloud;

    for (std::vector<CExternalPluginInfo>::iterator iterExt = oPlugins.m_arExternals.begin(); iterExt != oPlugins.m_arExternals.end(); iterExt++)
    {
        pManager->m_pInternal->m_arExternalPlugins.push_back(*iterExt);
    }

    pManager->m_pInternal->LoadCryptoData();

    CExternalClouds oClouds;
#ifdef _MAC
    oClouds.m_sSystemDirectory = NSFile::GetProcessDirectory() + L"/../Resources/providers";
#else
    oClouds.m_sSystemDirectory = NSFile::GetProcessDirectory() + L"/providers";
#endif
    oClouds.Load(pManager->m_pInternal->m_arExternalClouds);

    NSSystem::SetEnvValueA("APPLICATION_NAME", pManager->m_oSettings.converter_application_name);
    NSSystem::SetEnvValueA("COMPANY_NAME", pManager->m_oSettings.converter_application_company);

    for (std::map<std::string, std::string>::iterator iterMap = pManager->m_oSettings.converter_environments.begin();
         iterMap != pManager->m_oSettings.converter_environments.end(); iterMap++)
    {
        // TODO: on linux undate environments in converter
        NSSystem::SetEnvValueA(iterMap->first, iterMap->second);
    }

    pManager->m_pInternal->LocalFiles_Init();

    // local files resolver
    pManager->m_pInternal->m_oLocalFilesResolver.Init(pManager->m_oSettings.fonts_cache_info_path);
    pManager->m_pInternal->m_oLocalFilesResolver.AddDir(pManager->m_oSettings.recover_path);
    pManager->m_pInternal->m_oLocalFilesResolver.AddDir(pManager->m_oSettings.user_plugins_path,
        {L"cloud_crypto.xml", L"addons", L"advanced_crypto_data.docx"});
    pManager->m_pInternal->m_oLocalFilesResolver.AddDir(pManager->m_oSettings.system_plugins_path);
    std::wstring sTempDirectory = pManager->m_pInternal->StartTmpDirectory();

    pManager->m_pInternal->m_oLocalFilesResolver.AddDir(sTempDirectory);
    NSFile::CFileBinary::SetTempPath(sTempDirectory);

    // user plugins
    if (true)
    {
        std::wstring sSystemPlugins = pManager->m_oSettings.system_plugins_path;
        std::wstring sUserPlugins = pManager->m_oSettings.user_plugins_path;

        if (!NSDirectory::Exists(sUserPlugins))
            NSDirectory::CreateDirectories(sUserPlugins);

        std::wstring strBase = sUserPlugins + L"/v1";
        if (NSDirectory::Exists(strBase))
            NSFile::CFileBinary::Remove(strBase);

        NSDirectory::CopyDirectory(sSystemPlugins + L"/v1", strBase);

        if (NSFile::CFileBinary::Exists(sUserPlugins + L"/pluginBase.js"))
            NSFile::CFileBinary::Remove(sUserPlugins + L"/pluginBase.js");
        if (NSFile::CFileBinary::Exists(sUserPlugins + L"/plugins.css"))
            NSFile::CFileBinary::Remove(sUserPlugins + L"/plugins.css");

        NSFile::CFileBinary::Copy(sSystemPlugins + L"/pluginBase.js", sUserPlugins + L"/pluginBase.js");
        NSFile::CFileBinary::Copy(sSystemPlugins + L"/plugins.css", sUserPlugins + L"/plugins.css");
    }

    LOGGER_STRING("CApplicationCEF::Init_CEF::end");

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
        if (m_pInternal->m_bIsMessageLoopRunned)
        {
#ifdef _WIN32
            // https://bitbucket.org/chromiumembedded/cef/issues/2703/crash-in-createnetworkcontext-on-shutdown
            // TODO: fix in new cef builds (next release)
            ExitMessageLoop();
            return;
#endif
        }

        if (m_pInternal->context)
            m_pInternal->context->Shutdown();
        RELEASEOBJECT(m_pInternal);
    }
}

int CApplicationCEF::RunMessageLoop(bool& is_runned)
{
    is_runned = true;
    m_pInternal->m_bIsMessageLoopRunned = true;
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
    m_pInternal->message_loop->Quit();
    m_pInternal->m_bIsMessageLoopRunned = false;
    return true;
}

bool CApplicationCEF::IsChromiumSubprocess()
{
    return (m_pInternal->m_nReturnCodeInitCef >= 0) ? true : false;
}

void CApplicationCEF::Prepare(int argc, char* argv[])
{
#if defined(_LINUX) && !defined(_MAC)
    NSSystem::SetEnvValueA("GDK_BACKEND", "x11");
#endif
}

// External message loop
bool CAscApplicationManager::IsExternalEventLoop()
{
#if defined(CEF_2623) || defined(_MAC)
    m_pInternal->m_bIsUseExternalMessageLoop = false;
#else
    if (m_pInternal->m_bIsUseExternalMessageLoop)
    {
        IExternalMessageLoop* external_message_loop = GetExternalMessageLoop();
        if (NULL != external_message_loop)
        {
            m_pInternal->m_pExternalMessageLoop = external_message_loop;
            m_pInternal->m_pApplication->m_pInternal->message_loop = CMainMessageLoopExternalPumpBase::Create(external_message_loop);
            return true;
        }
    }
#endif

    return false;
}
IExternalMessageLoop* CAscApplicationManager::GetExternalMessageLoop()
{
    return NULL;
}
void CAscApplicationManager::ExternalMessageLoop_OnExecute(void* message)
{
#ifndef CEF_2623
    CMainMessageLoopExternalPumpBase* loop = (CMainMessageLoopExternalPumpBase*)(m_pInternal->m_pApplication->m_pInternal->message_loop.get());
    loop->OnExecute(message);
#endif
}
void CAscApplicationManager::ExternalMessageLoop_OnTimeout()
{
#ifndef CEF_2623
    CMainMessageLoopExternalPumpBase* loop = (CMainMessageLoopExternalPumpBase*)(m_pInternal->m_pApplication->m_pInternal->message_loop.get());
    loop->OnTimer();
#endif
}
//
