#include <string>
#include <stdio.h>
#include "../../../core/DesktopEditor/common/File.h"
#include "../../../core/DesktopEditor/common/Directory.h"
#include "./src/clienthandler.h"
#include "./src/client_app.h"

#include "../../../core/DesktopEditor/xml/include/xmlutils.h"

//#define ASC_HTML_FILE_INTERNAL_LOG

#include "src/client_app.h"
#include "src/clienthandler.h"
#include "cefclient/common/client_switches.h"
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/main_message_loop_std.h"

class CGlobalHtmlFileParams
{
public:
    std::vector<std::wstring> sSdkPath;
    std::wstring sDstPath;
    std::vector<std::wstring> arFiles;

    client::RootWindowManager* m_pManager;

public:
    CGlobalHtmlFileParams()
    {
    }
};

CGlobalHtmlFileParams* g_globalParams;

#if 1
#include "cefclient/browser/browser_window_std_win.h"
#include "cefclient/browser/client_handler_std.h"
// подменяем функции окна

namespace client {

BrowserWindowStdWin::BrowserWindowStdWin(Delegate* delegate,
                                         const std::string& startup_url)
    : BrowserWindow(delegate) {

    if (startup_url == "asc_html_file_internal")
    {
        client_handler_ = new CHtmlClientHandler(this, startup_url, g_globalParams->m_pManager);
        ((CHtmlClientHandler*)(client_handler_.operator ->()))->Init(g_globalParams->sSdkPath, g_globalParams->arFiles, g_globalParams->sDstPath);
    }
    else
    {
        client_handler_ = new ClientHandlerStd(this, startup_url);
    }
}

void BrowserWindowStdWin::CreateBrowser(
    ClientWindowHandle parent_handle,
    const CefRect& rect,
    const CefBrowserSettings& settings,
    CefRefPtr<CefRequestContext> request_context) {
  REQUIRE_MAIN_THREAD();

  CefWindowInfo window_info;
  RECT wnd_rect = {rect.x, rect.y, rect.x + rect.width, rect.y + rect.height};
  window_info.SetAsChild(parent_handle, wnd_rect);

  if ("asc_html_file_internal" == client_handler_->startup_url())
  {
    CefBrowserHost::CreateBrowser(window_info, client_handler_,
                                ((CHtmlClientHandler*)(client_handler_.operator ->()))->GetUrl(),
                                settings, request_context);
  }
  else
  {
    CefBrowserHost::CreateBrowser(window_info, client_handler_,
                                  client_handler_->startup_url(),
                                  settings, request_context);
  }
}

void BrowserWindowStdWin::GetPopupConfig(CefWindowHandle temp_handle,
                                         CefWindowInfo& windowInfo,
                                         CefRefPtr<CefClient>& client,
                                         CefBrowserSettings& settings) {
  // Note: This method may be called on any thread.
  // The window will be properly sized after the browser is created.
  windowInfo.SetAsChild(temp_handle, RECT());
  client = client_handler_;
}

void BrowserWindowStdWin::ShowPopup(ClientWindowHandle parent_handle,
                                    int x, int y, size_t width, size_t height) {
  REQUIRE_MAIN_THREAD();

  HWND hwnd = GetWindowHandle();
  if (hwnd) {
    SetParent(hwnd, parent_handle);
    SetWindowPos(hwnd, NULL, x, y,
                 static_cast<int>(width), static_cast<int>(height),
                 SWP_NOZORDER);
    ShowWindow(hwnd, SW_SHOW);
  }
}

void BrowserWindowStdWin::Show() {
  REQUIRE_MAIN_THREAD();

#if 0
  HWND hwnd = GetWindowHandle();
  if (hwnd && !::IsWindowVisible(hwnd))
    ShowWindow(hwnd, SW_SHOW);
#endif
}

void BrowserWindowStdWin::Hide() {
  REQUIRE_MAIN_THREAD();

  HWND hwnd = GetWindowHandle();
  if (hwnd) {
    // When the frame window is minimized set the browser window size to 0x0 to
    // reduce resource usage.
    SetWindowPos(hwnd, NULL,
        0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
  }
}

void BrowserWindowStdWin::SetBounds(int x, int y, size_t width, size_t height) {
  REQUIRE_MAIN_THREAD();

  HWND hwnd = GetWindowHandle();
  if (hwnd) {
    // Set the browser window bounds.
    SetWindowPos(hwnd, NULL, x, y,
                 static_cast<int>(width), static_cast<int>(height),
                 SWP_NOZORDER);
  }
}

void BrowserWindowStdWin::SetFocus(bool focus) {
  REQUIRE_MAIN_THREAD();

  if (browser_)
    browser_->GetHost()->SetFocus(focus);
}

ClientWindowHandle BrowserWindowStdWin::GetWindowHandle() const {
  REQUIRE_MAIN_THREAD();

  if (browser_)
    return browser_->GetHost()->GetWindowHandle();
  return NULL;
}
}  // namespace client
#endif

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    std::wstring sXml(lpCmdLine);

    bool bIsChromiumSubprocess = true;
    if (0 == sXml.find(L"<html>"))
        bIsChromiumSubprocess = false;

    g_globalParams = new CGlobalHtmlFileParams();

    if (!bIsChromiumSubprocess)
    {
        XmlUtils::CXmlNode oNode;
        if (!oNode.FromXmlFile(sXml.substr(6)))
            return 1;

        XmlUtils::CXmlNodes oNodesSdkPath;
        if (!oNode.GetNodes(L"sdk", oNodesSdkPath))
            return 1;

        XmlUtils::CXmlNode oNodeDstPath;
        if (!oNode.GetNode(L"destination", oNodeDstPath))
            return 1;

        int nCountSdks = oNodesSdkPath.GetCount();
        for (int i = 0; i < nCountSdks; ++i)
        {
            XmlUtils::CXmlNode _node;
            oNodesSdkPath.GetAt(i, _node);

            g_globalParams->sSdkPath.push_back(_node.GetText());
        }

        g_globalParams->sDstPath = oNodeDstPath.GetText();

        XmlUtils::CXmlNodes oNodesFiles;
        if (!oNode.GetNodes(L"file", oNodesFiles))
            return 1;

        int nCountFiles = oNodesFiles.GetCount();
        for (int i = 0; i < nCountFiles; ++i)
        {
            XmlUtils::CXmlNode oNodeF;
            if (oNodesFiles.GetAt(i, oNodeF))
            {
                g_globalParams->arFiles.push_back(oNodeF.GetText());
            }
        }

        if (0 == g_globalParams->arFiles.size())
            return 1;
    }

    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    CefMainArgs main_args(hInstance);

    void* sandbox_info = NULL;

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());

    // Create a ClientApp of the correct type.
    CefRefPtr<CefApp> app;
    client::ClientApp::ProcessType process_type = client::ClientApp::GetProcessType(command_line);
    if (process_type == client::ClientApp::BrowserProcess)
        app = new CAscClientAppBrowser();
    else if (process_type == client::ClientApp::RendererProcess || process_type == client::ClientApp::ZygoteProcess)
        app = new CAscClientAppRenderer();
    else if (process_type == client::ClientApp::OtherProcess)
        app = new CAscClientAppOther();

    // Execute the secondary process, if any.
    int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
    if (exit_code >= 0)
    return exit_code;

    // Create the main context object.
    scoped_ptr<client::MainContextImpl> context(new client::MainContextImpl(command_line, true));

    CefSettings settings;

    settings.no_sandbox = true;

    std::wstring sCachePath = g_globalParams->sDstPath;
    if (NSDirectory::Exists(sCachePath))
    {
        sCachePath += L"data";
        NSDirectory::CreateDirectory(sCachePath);

        cef_string_t _cache;
        memset(&_cache, 0, sizeof(_cache));
        cef_string_from_wide(sCachePath.c_str(), sCachePath.length(), &_cache);
        settings.cache_path = _cache;
    }
    else
        sCachePath = L"";

    // Populate the settings based on command line arguments.
    context->PopulateSettings(&settings);

    // Create the main message loop object.
    scoped_ptr<client::MainMessageLoop> message_loop;
    if (settings.multi_threaded_message_loop)
        message_loop.reset(new client::MainMessageLoopMultithreadedWin);
    else
        message_loop.reset(new client::MainMessageLoopStd);

    // Initialize CEF.
    context->Initialize(main_args, settings, app, sandbox_info);

    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(sCachePath, true, NULL);
    manager = NULL;

    g_globalParams->m_pManager = context->GetRootWindowManager();

    // Create the first window.
    context->GetRootWindowManager()->CreateRootWindow(
        false,             // Show controls.
        settings.windowless_rendering_enabled ? true : false,
        CefRect(),        // Use default system size.
        "asc_html_file_internal");   // Use default URL.

    // Run the message loop. This will block until Quit() is called by the
    // RootWindowManager after all windows have been destroyed.
    int result = message_loop->Run();

    // Shut down CEF.
    context->Shutdown();

    // Release objects in reverse order of creation.
    message_loop.reset();
    context.reset();

    if (!sCachePath.empty())
        NSDirectory::DeleteDirectory(sCachePath);

    RELEASEOBJECT(g_globalParams);

    return result;
}
