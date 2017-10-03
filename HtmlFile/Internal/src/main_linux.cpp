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

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#undef Success     // Definition conflicts with cef_message_router.h
#undef RootWindow  // Definition conflicts with root_window.h

#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "../../../core/DesktopEditor/common/File.h"
#include "../../../core/DesktopEditor/common/Directory.h"
#include "./src/clienthandler.h"
#include "./src/client_app.h"

#include "../../../core/DesktopEditor/xml/include/xmlutils.h"

//#define ASC_HTML_FILE_INTERNAL_LOG

#include "src/client_app.h"
#include "src/clienthandler.h"
#include "tests/shared/common/client_switches.h"
#include "tests/cefclient/browser/main_context_impl.h"
#include "tests/shared/browser/main_message_loop_std.h"

#include <sys/prctl.h>

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
#include "tests/cefclient/browser/browser_window_std_gtk.h"
#include "tests/cefclient/browser/client_handler_std.h"
// подменяем функции окна

namespace client {

namespace {

::Window GetXWindowForWidget(GtkWidget* widget) {
  // The GTK window must be visible before we can retrieve the XID.
  ::Window xwindow = GDK_WINDOW_XID(gtk_widget_get_window(widget));
  DCHECK(xwindow);
  return xwindow;
}

void SetXWindowVisible(::Window xwindow, bool visible) {
  ::Display* xdisplay = cef_get_xdisplay();

  // Retrieve the atoms required by the below XChangeProperty call.
  const char* kAtoms[] = {
    "_NET_WM_STATE",
    "ATOM",
    "_NET_WM_STATE_HIDDEN"
  };
  Atom atoms[3];
  int result = XInternAtoms(xdisplay, const_cast<char**>(kAtoms), 3, false,
                            atoms);
  if (!result)
    NOTREACHED();

  if (!visible) {
    // Set the hidden property state value.
    scoped_ptr<Atom[]> data(new Atom[1]);
    data[0] = atoms[2];

    XChangeProperty(xdisplay,
                    xwindow,
                    atoms[0],  // name
                    atoms[1],  // type
                    32,  // size in bits of items in 'value'
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(data.get()),
                    1);  // num items
  } else {
    // Set an empty array of property state values.
    XChangeProperty(xdisplay,
                    xwindow,
                    atoms[0],  // name
                    atoms[1],  // type
                    32,  // size in bits of items in 'value'
                    PropModeReplace,
                    NULL,
                    0);  // num items
  }
}

void SetXWindowBounds(::Window xwindow,
                      int x, int y, size_t width, size_t height) {
  ::Display* xdisplay = cef_get_xdisplay();
  XWindowChanges changes = {0};
  changes.x = x;
  changes.y = y;
  changes.width = static_cast<int>(width);
  changes.height = static_cast<int>(height);
  XConfigureWindow(xdisplay, xwindow,
                   CWX | CWY | CWHeight | CWWidth, &changes);
}

}  // namespace

BrowserWindowStdGtk::BrowserWindowStdGtk(Delegate* delegate,
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

void BrowserWindowStdGtk::CreateBrowser(
    ClientWindowHandle parent_handle,
    const CefRect& rect,
    const CefBrowserSettings& settings,
    CefRefPtr<CefRequestContext> request_context) {
  REQUIRE_MAIN_THREAD();

  CefWindowInfo window_info;
  window_info.SetAsChild(GetXWindowForWidget(parent_handle), rect);

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

void BrowserWindowStdGtk::GetPopupConfig(CefWindowHandle temp_handle,
                                         CefWindowInfo& windowInfo,
                                         CefRefPtr<CefClient>& client,
                                         CefBrowserSettings& settings) {
  // Note: This method may be called on any thread.
  // The window will be properly sized after the browser is created.
  windowInfo.SetAsChild(temp_handle, CefRect());
  client = client_handler_;
}

void BrowserWindowStdGtk::ShowPopup(ClientWindowHandle parent_handle,
                                    int x, int y, size_t width, size_t height) {
  REQUIRE_MAIN_THREAD();

  if (browser_) {
    ::Window parent_xwindow = GetXWindowForWidget(parent_handle);
    ::Display* xdisplay = cef_get_xdisplay();
    ::Window xwindow = browser_->GetHost()->GetWindowHandle();
    DCHECK(xwindow);

    XReparentWindow(xdisplay, xwindow, parent_xwindow, x, y);

    SetXWindowBounds(xwindow, x, y, width, height);
    SetXWindowVisible(xwindow, true);
  }
}

void BrowserWindowStdGtk::Show() {
  REQUIRE_MAIN_THREAD();

  if (browser_) {
    ::Window xwindow = browser_->GetHost()->GetWindowHandle();
    DCHECK(xwindow);
#if 0
    SetXWindowVisible(xwindow, true);
#endif
  }
}

void BrowserWindowStdGtk::Hide() {
  REQUIRE_MAIN_THREAD();

  if (browser_) {
    ::Window xwindow = browser_->GetHost()->GetWindowHandle();
    DCHECK(xwindow);
    SetXWindowVisible(xwindow, false);
  }
}

void BrowserWindowStdGtk::SetBounds(int x, int y, size_t width, size_t height) {
  REQUIRE_MAIN_THREAD();

  if (browser_) {
    ::Window xwindow = browser_->GetHost()->GetWindowHandle();
    DCHECK(xwindow);
    SetXWindowBounds(xwindow, x, y, width, height);
  }
}

void BrowserWindowStdGtk::SetFocus(bool focus) {
  REQUIRE_MAIN_THREAD();

  if (browser_)
    browser_->GetHost()->SetFocus(focus);
}

ClientWindowHandle BrowserWindowStdGtk::GetWindowHandle() const {
  REQUIRE_MAIN_THREAD();

  // There is no GtkWidget* representation of this object.
  NOTREACHED();
  return NULL;
}

}  // namespace client
#endif

int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
  LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display *display) {
  return 0;
}

void TerminationSignalHandler(int signatl) {
  LOG(ERROR) << "Received termination signal: " << signatl;
  client::MainContext::Get()->GetRootWindowManager()->CloseAllWindows(true);
}

int main(int argc, char* argv[])
{
    ::prctl(PR_SET_PDEATHSIG, SIGHUP);
    std::wstring sXml;

    for (int i = 0; i < argc && i < 2; ++i)
    {
        std::string sXmlA(argv[i]);

        if (0 == sXmlA.find("<html>"))
        {
            sXml = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sXmlA.c_str(), (LONG)sXmlA.length());
        }
    }

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

    // Create a copy of |argv| on Linux because Chromium mangles the value
    // internally (see issue #620).
    CefScopedArgArray scoped_arg_array(argc, argv);
    char** argv_copy = scoped_arg_array.array();

    CefMainArgs main_args(argc, argv);

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv);

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
    int exit_code = CefExecuteProcess(main_args, app, NULL);
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
    scoped_ptr<client::MainMessageLoop> message_loop(new client::MainMessageLoopStd);

    // Initialize CEF.
    context->Initialize(main_args, settings, app, NULL);

    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(sCachePath, true, NULL);
    manager = NULL;

    g_globalParams->m_pManager = context->GetRootWindowManager();

    // The Chromium sandbox requires that there only be a single thread during
    // initialization. Therefore initialize GTK after CEF.
    gtk_init(&argc, &argv_copy);

    // Perform gtkglext initialization required by the OSR example.
    gtk_gl_init(&argc, &argv_copy);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors. Must be done after initializing GTK.
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    // Install a signal handler so we clean up after ourselves.
    signal(SIGINT, TerminationSignalHandler);
    signal(SIGTERM, TerminationSignalHandler);

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
