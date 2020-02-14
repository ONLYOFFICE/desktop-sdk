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

#ifndef _HTMLFILE_CLIENT_HANDLER_
#define _HTMLFILE_CLIENT_HANDLER_

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include "include/base/cef_bind.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_closure_task.h"
#include "tests/cefclient/browser/client_handler.h"
#include "tests/shared/common/client_switches.h"
#include "include/cef_app.h"

#include "tests/cefclient/renderer/client_renderer.h"
#include "tests/shared/browser/main_message_loop.h"

#include "../../../core/DesktopEditor/common/File.h"
#include <string>
#include <vector>

#include "tests/cefclient/browser/root_window_manager.h"
#include <iostream>

#ifdef CEF_2623
#define MESSAGE_IN_BROWSER
#else
// с версии выше 74 - убрать определение
//#define MESSAGE_IN_BROWSER
#endif

#ifdef MESSAGE_IN_BROWSER
#define SEND_MESSAGE_TO_BROWSER_PROCESS(message) CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, message)
#define SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message) browser->SendProcessMessage(PID_RENDERER, message)
#else
#define SEND_MESSAGE_TO_BROWSER_PROCESS(message) CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, message)
#define SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message) browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message)
#endif

class CGlobalHtmlFileParams
{
public:
    std::vector<std::wstring>   sSdkPath;
    std::wstring                sDstPath;
    std::vector<std::wstring>   arFiles;

    CefRefPtr<client::ClientHandler> m_client;

public:
    CGlobalHtmlFileParams()
    {        
    }
};

class CHtmlRenderHandler : public CefRenderHandler
{
public:
    CHtmlRenderHandler() {}

public:
#ifdef CEF_2623
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        rect = CefRect(0, 0, 1000, 1000);
        return true;
    }
#else
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        rect = CefRect(0, 0, 1000, 1000);
    }
#endif
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                         PaintElementType type,
                         const RectList& dirtyRects,
                         const void* buffer,
                         int width,
                         int height)
    {
        return;
    }

public:
    IMPLEMENT_REFCOUNTING(CHtmlRenderHandler);

};

class CHtmlClientHandler : public client::ClientHandler, public client::ClientHandler::Delegate
{
private:
    std::wstring m_sTempFile;
    std::wstring m_sCachePath;
    bool m_bIsLoadedFirst;

public:
    CGlobalHtmlFileParams* m_global;

    CefRefPtr<CHtmlRenderHandler> m_render;
    CefRefPtr<CefBrowser> m_browser;

public:
    CHtmlClientHandler(CGlobalHtmlFileParams* params) : client::ClientHandler(this, true, "html_file")
    {
        m_global = params;
        m_render = new CHtmlRenderHandler();
        m_bIsLoadedFirst = false;
    }

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
    {
        return m_render;
    }

#ifndef CEF_2623
    virtual void OnAutoResize(const CefSize& new_size) OVERRIDE {}
#endif

    void Init()
    {
        std::vector<std::wstring>& arSdks = m_global->sSdkPath;
        std::vector<std::wstring>& arFiles = m_global->arFiles;
        std::wstring& sDestinationFile = m_global->sDstPath;

        m_sCachePath = sDestinationFile;
        std::wstring sUniquePath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"HTML");

        // под линуксом предыдущая функция создает файл!!!
        if (NSFile::CFileBinary::Exists(sUniquePath))
            NSFile::CFileBinary::Remove(sUniquePath);

        m_sTempFile = sUniquePath + L".html";
        NSFile::CFileBinary oFileBinary;
        oFileBinary.CreateFileW(m_sTempFile);

        std::wstring sFilesHTML = L"";
        for (std::vector<std::wstring>::const_iterator iter = arFiles.begin(); iter != arFiles.end(); )
        {
            sFilesHTML += L"\"";
            sFilesHTML += *iter;
            sFilesHTML += L"\"";

            iter++;
            if (iter != arFiles.end())
                sFilesHTML += L",";
        }

        std::wstring sSdkPathHTML;
        for (std::vector<std::wstring>::const_iterator i = arSdks.begin(); i != arSdks.end(); i++)
        {
            sSdkPathHTML += (L"<script src=\"" + *i + L"\" type=\"text/javascript\"></script>");
        }

        std::wstring sHtmlContent = L"\
<!DOCTYPE html>\
<html>\
<head>\
<title>OnlyOffice: HtmlFile</title>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\
<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=IE8\" />\
</head>\
<body>\
<div id=\"editor_sdk\" style=\"width:100%;height:100%;\">\
</div>" + sSdkPathHTML + L"\
<script>\
window.onload = function ()\
{\
    window.Native.InitSDK(\"editor_sdk\");\
    window.Native.OpenEmptyDocument();\
    window.Native.SetDestinationDocumentPath(\"" + sDestinationFile + L"\");\
    \
    window.iframes_convert = [" + sFilesHTML + L"];\
    \
    var _current_iframe_convert = 0;\
    window.htmlOnLoadTimerID = -1;\
    \
    window.on_load_iframe = function ()\
    {\
        if (-1 != window.htmlOnLoadTimerID)\
        {\
            clearTimeout(window.htmlOnLoadTimerID);\
            window.htmlOnLoadTimerID = -1;\
        }\
        window.Native.AddHtml(\"pasteFrame\");\
        \
        ++_current_iframe_convert;\
        if (_current_iframe_convert < window.iframes_convert.length)\
        {\
            window.add_iframe_convert();\
        }\
        else\
        {\
            window.Native.SaveDocument();\
            window.Native.Exit();\
        }\
    };\
    \
    window.add_iframe_convert = function()\
    {\
        if (_current_iframe_convert >= window.iframes_convert.length)\
            return;\
        \
        var ifr = document.createElement(\"iframe\");\
        ifr.name = \"pasteFrame\";\
        ifr.id = \"pasteFrame\";\
        ifr.style.position = 'absolute';\
        ifr.style.top = '-100px';\
        ifr.style.left = '0px';\
        ifr.style.width = '10000px';\
        ifr.style.height = '100px';\
        ifr.style.overflow = 'hidden';\
        ifr.style.zIndex = -1000;\
        ifr.onload = window.on_load_iframe;\
        window.htmlOnLoadTimerID = setTimeout(window.on_load_iframe, 20000);\
        ifr.src = window.iframes_convert[_current_iframe_convert];\
        \
        document.body.appendChild(ifr);\
    };\
    \
    window.add_iframe_convert();\
};\
</script>\
</body>\
</html>";

                // for debug:
                // setTimeout(window.add_iframe_convert, 40000);\
                // for release:
                // window.add_iframe_convert();\

        oFileBinary.WriteStringUTF8(sHtmlContent, true);
        oFileBinary.CloseFile();

#if 0
        NSFile::CFileBinary oFileBinaryTest;
        oFileBinaryTest.CreateFileW(L"D://123.html");
        oFileBinaryTest.WriteStringUTF8(sHtmlContent, true);
        oFileBinaryTest.CloseFile();
#endif
    }

    void RemoveTemp()
    {
        if (!m_sTempFile.empty())
            NSFile::CFileBinary::Remove(m_sTempFile);
        m_sTempFile = L"";
    }

    virtual ~CHtmlClientHandler()
    {
        RemoveTemp();
    }

    std::wstring GetUrl()
    {
        if (0 == m_sTempFile.find(L"/"))
            return L"file://" + m_sTempFile;
        else
            return L"file:///" + m_sTempFile;

        return m_sTempFile;
    }

    std::wstring GetCachePath()
    {
        return m_sCachePath;
    }

    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      const CefString& target_url,
                                      const CefString& target_frame_name,
                                      const CefPopupFeatures& popupFeatures,
                                      CefWindowInfo& windowInfo,
                                      CefRefPtr<CefClient>& client,
                                      CefBrowserSettings& settings,
                                      bool* no_javascript_access)
    {
        CEF_REQUIRE_IO_THREAD();
        return true;
    }

    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool is_redirect)
    {
        //std::wstring sUrl = request->GetURL().ToWString();
        bool bIsMain = frame->IsMain();
        if (bIsMain)
        {
            if (m_bIsLoadedFirst)
                return true;
            m_bIsLoadedFirst = true;
        }
        return false;
    }

    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback)
    {
        CEF_REQUIRE_UI_THREAD();
        callback->Continue("", false);
    }

    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefDownloadItem> download_item,
                                          CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();
        callback->Cancel();
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message)
    {
        CEF_REQUIRE_UI_THREAD();

        // Check for messages from the client renderer.
        std::string message_name = message->GetName();
        if (message_name == "Exit")
        {
#ifndef DEBUG_WINDOW_SHOW
            this->Exit();
#endif
            return true;
        }

        return false;
    }    

    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  #ifndef CEF_2623
                                  cef_log_severity_t level,
                                  #endif
                                  const CefString& message,
                                  const CefString& source,
                                  int line) OVERRIDE
    {
        return false;
    }

    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                               const CefKeyEvent& event,
                               CefEventHandle os_event,
                               bool* is_keyboard_shortcut) OVERRIDE
    {
        return false;
    }

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           TerminationStatus status) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        this->Exit();
    }

    void Exit()
    {
        if (m_browser && m_browser->GetHost())
            m_browser->GetHost()->CloseBrowser(true);
        client::MainMessageLoop::Get()->Quit();
    }

    /////////////////////////////////////////////////////
    // Called when the browser is created.
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser)
      {
          m_browser = browser;
      }

    // Called when the browser is closing.
    virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser)
      {
      }

    // Called when the browser has been closed.
    virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser)
      {
      }

    // Set the window URL address.
    virtual void OnSetAddress(const std::string& url)
      {
      }

    // Set the window title.
    virtual void OnSetTitle(const std::string& title)
      {
      }

    // Set the Favicon image.
    virtual void OnSetFavicon(CefRefPtr<CefImage> image){};

    // Set fullscreen mode.
    virtual void OnSetFullscreen(bool fullscreen)
      {
      }

    // Set the loading state.
    virtual void OnSetLoadingState(bool isLoading,
                                   bool canGoBack,
                                   bool canGoForward)
      {
      }

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(
        const std::vector<CefDraggableRegion>& regions)
      {
      }

    // Set focus to the next/previous control.
    virtual void OnTakeFocus(bool next) {}

    // Called on the UI thread before a context menu is displayed.
    virtual void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) {}

#ifdef CEF_3202
    virtual void OnAutoResize(const CefSize& new_size) OVERRIDE {}
#endif

public:
    IMPLEMENT_REFCOUNTING(CHtmlClientHandler);
    DISALLOW_COPY_AND_ASSIGN(CHtmlClientHandler);
};


#endif // _HTMLFILE_CLIENT_HANDLER_
