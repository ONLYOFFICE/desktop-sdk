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

#include "../../../core/DesktopEditor/common/File.h"
#include "../../../core/DesktopEditor/common/Directory.h"
#include "../../../core/DesktopEditor/xml/include/xmlutils.h"

#include "./src/clienthandler.h"
#include "./src/client_app.h"

#ifdef GetTempPath
#undef GetTempPath
#endif

#ifdef CreateDirectory
#undef CreateDirectory
#endif

CGlobalHtmlFileParams* g_globalParams;

#ifdef _LINUX
void TerminationSignalHandler(int signatl) {
  LOG(ERROR) << "Received termination signal: " << signatl;
  ((CHtmlClientHandler*)g_globalParams->m_client.get())->Exit();
}

#include <sys/prctl.h>

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
#else

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    std::wstring sXml(lpCmdLine);

#ifdef DEBUG_WINDOW_SHOW
    // fix for qt debugger
    if (sXml.length() > 2)
    {
        if ('\"' == sXml.c_str()[0])
            sXml = sXml.substr(1);
        if ('\"' == sXml.c_str()[sXml.length() - 1])
            sXml = sXml.substr(0, sXml.length() - 1);
    }
#endif

#endif

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

#ifdef _LINUX
    CefMainArgs main_args(argc, argv);
#else
    CefMainArgs main_args(hInstance);
#endif

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();

#ifdef _LINUX
    command_line->InitFromArgv(argc, argv);
#else
    command_line->InitFromString(::GetCommandLineW());
#endif

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
    settings.windowless_rendering_enabled = true;

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

    // Initialize CEF.
    context->Initialize(main_args, settings, app, NULL);

#ifdef CEF_2623
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(sCachePath, true, NULL);    
    manager = NULL;
#endif

#ifdef _LINUX
    // Install a signal handler so we clean up after ourselves.
    signal(SIGINT, TerminationSignalHandler);
    signal(SIGTERM, TerminationSignalHandler);
#endif

    CefRefPtr<CHtmlClientHandler> client_handler = new CHtmlClientHandler(g_globalParams);
    client_handler->Init();

    CefWindowInfo window_info;
#ifdef DEBUG_WINDOW_SHOW
    window_info.SetAsPopup(NULL, "HTML");
#else
    window_info.SetAsWindowless(NULL);
#endif

    CefBrowserSettings browser_settings;
    browser_settings.plugins = STATE_DISABLED;
    CefBrowserHost::CreateBrowser(window_info, client_handler.get(), client_handler->GetUrl(), browser_settings, NULL
                                              #ifndef CEF_2623
                                                  , NULL
                                              #endif
                                  );

    scoped_ptr<client::MainMessageLoop> message_loop;
    message_loop.reset(new client::MainMessageLoopStd);
    message_loop->Run();

    // Shut down CEF.
    context->Shutdown();
    context.reset();

    if (!sCachePath.empty())
        NSDirectory::DeleteDirectory(sCachePath);

    RELEASEOBJECT(g_globalParams);

    return 0;
}
