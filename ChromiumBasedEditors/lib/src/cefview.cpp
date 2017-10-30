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

#include "include/cef_browser.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "tests/cefclient/browser/client_handler.h"
#include "tests/shared/common/client_switches.h"
#include "tests/cefclient/renderer/client_renderer.h"

#include "include/cef_menu_model.h"

#include "include/wrapper/cef_stream_resource_handler.h"
#include "include/wrapper/cef_byte_read_handler.h"

#include "window_common.h"

#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "./fileconverter.h"
#include "./fileprinter.h"

#include "../../../../core/PdfReader/PdfReader.h"
#include "../../../../core/DjVuFile/DjVu.h"
#include "../../../../core/XpsFile/XpsFile.h"

#include "cefwrapper/monitor_info.h"

class CAscNativePrintDocument : public IAscNativePrintDocument
{
protected:
    IOfficeDrawingFile* m_pReader;

public:
    CAscNativePrintDocument() : IAscNativePrintDocument()
    {
        m_pReader = NULL;
    }
    virtual ~CAscNativePrintDocument()
    {
        RELEASEOBJECT(m_pReader);
        NSDirectory::DeleteDirectory(m_sTempFolder);
    }
public:
    virtual void Draw(IRenderer* pRenderer, int nPageIndex)
    {
        if (m_pReader)
            m_pReader->DrawPageOnRenderer(pRenderer, nPageIndex, NULL);
    }

    virtual void PreOpen(int nFileType)
    {
        m_pReader = NULL;
        switch (nFileType)
        {
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF:
        {
            m_pReader = new PdfReader::CPdfReader(m_pApplicationFonts);
            break;
        }
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS:
        {
            m_pReader = new CXpsFile(m_pApplicationFonts);
            break;
        }
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU:
        {
            m_pReader = new CDjVuFile(m_pApplicationFonts);
            break;
        }
        default:
            break;
        };
    }

    virtual void Open(const std::wstring& sPath, const std::wstring& sRecoveryDir)
    {
        m_sFilePath = sPath;
        m_sTempFolder = sRecoveryDir + L"/PrintTemp";
        NSDirectory::CreateDirectory(m_sTempFolder);

        m_pReader->SetTempDirectory(m_sTempFolder.c_str());
        m_pReader->LoadFromFile(m_sFilePath);
    }

    virtual void Close()
    {
        m_pReader->Close();
    }

    virtual void Check(CArray<CPagePrintData>& arPages)
    {
        int nPagesCount = m_pReader->GetPagesCount();
        arPages.SetCount(nPagesCount);
        for (int i = 0; i < nPagesCount; ++i)
        {
            double dPageDpiX, dPageDpiY;
            double dWidth, dHeight;
            m_pReader->GetPageInfo(i, &dWidth, &dHeight, &dPageDpiX, &dPageDpiY);

            dWidth  *= 25.4 / dPageDpiX;
            dHeight *= 25.4 / dPageDpiY;

            arPages[i].Width = dWidth;
            arPages[i].Height = dHeight;
        }
    }
};

#ifdef WIN32
LRESULT CALLBACK MyMouseHook(int nCode, WPARAM wp, LPARAM lp)
{
    MOUSEHOOKSTRUCT *pmh = (MOUSEHOOKSTRUCT *) lp;

    HWND h1 = GetParent(pmh->hwnd); // cefbrowser
    HWND h2 = GetParent(h1);        // cefview

    if (nCode == HC_ACTION && wp == WM_MBUTTONUP)
    {
        SendMessageA(h2, WM_USER + 1, 0, 0);
    }

    return CallNextHookEx(NULL, nCode, wp, lp);
}
#endif

#if defined(_LINUX) && !defined(_MAC)
#include <X11/Xlib.h>
#endif

#ifdef _MAC
#include "mac_common.h"
#endif

class CCefBinaryFileReaderCounter : public CefBaseRefCounted
{
public:
    BYTE* data;

    CCefBinaryFileReaderCounter(BYTE* _data = NULL)
    {
        this->AddRef();
        data = _data;
    }

    virtual ~CCefBinaryFileReaderCounter()
    {
        RELEASEARRAYOBJECTS(data);
    }

public:
    IMPLEMENT_REFCOUNTING(CCefBinaryFileReaderCounter);
};

class CNativeFileViewerInfo
{
public:
    std::wstring m_sFilePath;

    int m_nCounter;
    bool m_bEnabled;

public:
    CNativeFileViewerInfo()
    {
        m_nCounter = 0;
        m_bEnabled = false;
    }
};

class CAscClientHandler;
class CCefView_Private : public NSEditorApi::IMenuEventDataBase, public IASCFileConverterEvents
{
public:
    CefRefPtr<CAscClientHandler>        m_handler;
    CAscApplicationManager*             m_pManager;
    CCefViewWidgetImpl*                 m_pWidgetImpl;

    std::map<std::string, std::string>  m_cookies;

    int m_nParentId;

    bool m_bIsNativeSave;

    CPrintData m_oPrintData;

    bool m_bIsMouseHook;
    DWORD m_dwTimeMouseWheelUp;

    std::wstring m_strUrl;

    CefRefPtr<CefBeforeDownloadCallback> m_before_callback;

    CAscLocalFileInfoCS m_oLocalInfo;
    CASCFileConverterToEditor m_oConverterToEditor;
    CASCFileConverterFromEditor m_oConverterFromEditor;

    int m_nLocalFileOpenError;

    int m_nEditorType;

    CCefView* m_pCefView;

    bool m_bIsRemoveRecoveryOnClose;
    bool m_bIsClosing;
    bool m_bIsSavingDialog;

    CNativeFileViewerInfo m_oNativeViewer;

    int m_nDeviceScale;
    bool m_bIsWindowsCheckZoom = false;

    bool m_bIsReporter;
    int m_nReporterParentId;
    int m_nReporterChildId;

    bool m_bIsSSO;

#if defined(_LINUX) && !defined(_MAC)
    WindowHandleId m_lNaturalParent;
#endif

public:
    CCefView_Private()
    {
        m_pManager = NULL;
        m_pWidgetImpl = NULL;
        m_nParentId = -1;
        m_bIsNativeSave = false;
        m_dwTimeMouseWheelUp = (DWORD)-1;

        m_bIsMouseHook = false;
        m_strUrl = L"";

        m_before_callback = NULL;

        m_oConverterToEditor.m_pEvents = this;
        m_oConverterFromEditor.m_pEvents = this;

        m_nEditorType = 0;
        m_pCefView = NULL;
        m_nLocalFileOpenError = 0;

        m_bIsRemoveRecoveryOnClose = false;

        m_bIsClosing = false;
        m_bIsSavingDialog = false;

#if defined(_LINUX) && !defined(_MAC)
        m_lNaturalParent = 0;
#endif

        m_nDeviceScale = 1;
        m_bIsWindowsCheckZoom = false;

        m_bIsReporter = false;
        m_nReporterParentId = -1;
        m_nReporterChildId = -1;

        m_bIsSSO = false;
    }

    void Destroy()
    {
        m_oConverterToEditor.Stop();
        m_oConverterFromEditor.Stop();

        // смотрим, есть ли несохраненные данные
        m_oLocalInfo.m_oCS.Enter();

        bool bIsNeedRemove = false;
        if (!m_bIsSavingDialog && (!m_oLocalInfo.m_oInfo.m_bIsModified || m_bIsRemoveRecoveryOnClose))
            bIsNeedRemove = true;

        std::wstring sChangesFile = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/changes/changes0.json";
        if (!NSFile::CFileBinary::Exists(sChangesFile))
        {
            // нет изменений - нечего рековерить. актуально для НОВОГО файла
            bIsNeedRemove = true;
        }

        if (m_bIsReporter)
            bIsNeedRemove = false;

        if (bIsNeedRemove)
        {
            if (NSDirectory::Exists(m_oLocalInfo.m_oInfo.m_sRecoveryDir))
                NSDirectory::DeleteDirectory(m_oLocalInfo.m_oInfo.m_sRecoveryDir);
        }

        m_oLocalInfo.m_oCS.Leave();

        if (m_bIsReporter)
        {
            CCefView* pViewParent = m_pManager->GetViewById(m_nReporterParentId);
            if (pViewParent)
            {
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM);
                NSEditorApi::CAscReporterMessage* pData = new NSEditorApi::CAscReporterMessage();
                pData->put_ReceiverId(m_nReporterParentId);
                pData->put_Message(L"{ \"reporter_command\" : \"end\" }");
                pEvent->m_pData = pData;
                pViewParent->Apply(pEvent);
            }
        }
    }

    ~CCefView_Private()
    {
        Destroy();
    }

    void CheckZoom();

    void CloseBrowser(bool _force_close);

    void LocalFile_Start()
    {
        m_oLocalInfo.SetupOptions(m_oConverterToEditor.m_oInfo);

        // и теперь убираем, если новый
        std::wstring sNewPath = m_pManager->m_oSettings.file_converter_path + L"/empty/";
        if (m_oLocalInfo.m_oInfo.m_sFileSrc.find(sNewPath) == 0)
            m_oLocalInfo.m_oInfo.m_sFileSrc = m_oLocalInfo.m_oInfo.m_sFileSrc.substr(sNewPath.length());

        m_oConverterToEditor.Stop();
        m_oConverterToEditor.Start(0);
    }

    void LocalFile_SaveStart(std::wstring sPath = L"", int nType = -1)
    {
        m_oLocalInfo.SetupOptions(m_oConverterFromEditor.m_oInfo);
        m_oConverterFromEditor.m_nTypeEditorFormat = m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat;

        if (!sPath.empty())
            m_oConverterFromEditor.m_oInfo.m_sFileSrc = sPath;

        if (nType != -1)
            m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat = nType;

        m_oConverterFromEditor.m_bIsRetina = m_oLocalInfo.m_bIsRetina;

        m_oConverterFromEditor.Stop();
        m_oConverterFromEditor.Start(0);
    }

    void LocalFile_End();
    void LocalFile_SaveEnd(int nError, const std::wstring& sPass = L"");

    void LocalFile_IncrementCounter();

    virtual void OnFileConvertToEditor(const int& nError);

    virtual void OnFileConvertFromEditor(const int& nError, const std::wstring& sPass = L"")
    {
        LocalFile_SaveEnd(nError, sPass);
    }

    void LocalFile_GetSupportSaveFormats(std::vector<int>& arFormats)
    {
        if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_DOCUMENT)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT);
            //arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);
        }
    }

    bool LocalFile_IsSupportSaveCurrentFormat()
    {
        std::vector<int> arFormats;
        LocalFile_GetSupportSaveFormats(arFormats);

        for (std::vector<int>::iterator iter = arFormats.begin(); iter != arFormats.end(); iter++)
        {
            if (*iter == m_oLocalInfo.m_oInfo.m_nCurrentFileFormat)
                return true;
        }
        return false;
    }

    bool LocalFile_IsSupportEditCurrentFormat()
    {
        return LocalFile_IsSupportEditFormat(m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);
    }
    bool LocalFile_IsSupportEditFormat(int nFormat)
    {
        if ((nFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM) != 0)
            return false;

        return true;
    }

    void SendProcessMessage(CefProcessId target_process, CefRefPtr<CefProcessMessage> message);
};

class CCefFileDownloader_Private : public client::ClientHandler, public client::ClientHandler::Delegate
{
public:
    IFileDownloaderEvents* m_pEvents;
    std::wstring m_sFileSrc;
    std::wstring m_sFileDst;

    CefRefPtr<CefBrowser> browser_;
    int browser_id_;

    CCefFileDownloader_Private() : client::ClientHandler(this, false, "https://onlyoffice.com/")
    {
        m_pEvents = NULL;
    }
    virtual ~CCefFileDownloader_Private()
    {
    }

public:
    CefRefPtr<CefBrowser> GetBrowser() const
    {
        return browser_;
    }

    int GetBrowserId() const
    {
        return browser_id_;
    }

    virtual bool OnBeforePopup(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        const CefString& target_frame_name,
        CefLifeSpanHandler::WindowOpenDisposition target_disposition,
        bool user_gesture,
        const CefPopupFeatures& popupFeatures,
        CefWindowInfo& windowInfo,
        CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings,
        bool* no_javascript_access) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();

        return true;
    }

    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
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

    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback)
    {
        CEF_REQUIRE_UI_THREAD();

        callback->Continue(m_sFileDst, false);
    }

    void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefDownloadItem> download_item,
                                          CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        if (NULL == m_pEvents)
            return;

        m_pEvents->OnProgress(download_item->GetPercentComplete());

        if (download_item->IsComplete())
        {
            m_pEvents->OnDownload(true);

            CloseBrowser(true);
        }
        if (download_item->IsCanceled())
        {
            m_pEvents->OnDownload(false);

            CloseBrowser(true);
        }
    }

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        client::ClientHandler::OnAfterCreated(browser);
        if (!GetBrowser())
        {
            // We need to keep the main child window, but not popup windows
            browser->GetHost()->StartDownload(m_sFileSrc);

            browser_ = browser;
            browser_id_ = browser->GetIdentifier();
        }
    }

    void CloseBrowser(bool _force_close)
    {
        if (!CefCurrentlyOn(TID_UI))
        {
            CefPostTask(TID_UI,
                base::Bind(&CCefFileDownloader_Private::CloseBrowser, this, _force_close));
            return;
        }

        if (GetBrowser() && GetBrowser()->GetHost())
            GetBrowser()->GetHost()->CloseBrowser(_force_close);
    }

    // ClientHandlerDelegate
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {}

    // Called when the browser is closing.
    virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE {}

    // Called when the browser has been closed.
    virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser) OVERRIDE { }

    // Set the window URL address.
    virtual void OnSetAddress(const std::string& url) OVERRIDE {}

    // Set the window title.
    virtual void OnSetTitle(const std::string& title) OVERRIDE {}

    // Set fullscreen mode.
    virtual void OnSetFullscreen(bool fullscreen) OVERRIDE {}

    // Set the loading state.
    virtual void OnSetLoadingState(bool isLoading,
                                     bool canGoBack,
                                     bool canGoForward) OVERRIDE {}

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions) OVERRIDE {}

public:
    IMPLEMENT_REFCOUNTING(CCefFileDownloader_Private);
};

CCefFileDownloader::CCefFileDownloader(IFileDownloaderEvents* pEvents)
{
    m_pInternal = new CCefFileDownloader_Private();
    m_pInternal->m_pEvents = pEvents;
}

CCefFileDownloader::~CCefFileDownloader()
{
    m_pInternal->Release();
    m_pInternal = NULL;
}

void CCefFileDownloader::DownloadFile(CAscApplicationManager* pManager, const std::wstring& sUrl, const std::wstring& sDstFile)
{
    m_pInternal->m_sFileSrc = sUrl;
    m_pInternal->m_sFileDst = sDstFile;

    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(pManager->m_oSettings.cookie_path, true, NULL);

    CefWindowHandle hWnd = (CefWindowHandle)0;

    CefWindowInfo info;
    CefBrowserSettings _settings;
    _settings.file_access_from_file_urls = STATE_ENABLED;
    _settings.universal_access_from_file_urls = STATE_ENABLED;
    _settings.javascript_access_clipboard = STATE_ENABLED;

    _settings.plugins = STATE_DISABLED;

    // Initialize window info to the defaults for a child window.

#ifdef WIN32
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = 10;
    rect.bottom = 10;
    info.SetAsChild(hWnd, rect);
#endif

#if defined(_LINUX) && !defined(_MAC)
    CefRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = 10;
    rect.height = 10;
    info.SetAsChild(hWnd, rect);
#endif

#ifdef _MAC
    info.SetAsChild(hWnd, 0, 0, 10, 10);
#endif

    // Creat the new child browser window
    CefBrowserHost::CreateBrowser(info, m_pInternal, "ascdesktop://emptydownload.html", _settings, NULL);
}

class CAscClientHandler : public client::ClientHandler, public CCookieFoundCallback, public client::ClientHandler::Delegate
{
public:
    CCefView* m_pParent;
    bool m_bIsLoaded;

    bool m_bIsEditorTypeSet;
    int m_nBeforeBrowserCounter;

    CefRefPtr<CefBrowser> browser_;
    int browser_id_;

    CefRefPtr<CefJSDialogHandler> m_pCefJSDialogHandler;

    enum client_menu_ids
    {
        CLIENT_ID_SHOW_DEVTOOLS   = MENU_ID_USER_FIRST,
        CLIENT_ID_CLOSE_DEVTOOLS,
        CLIENT_ID_INSPECT_ELEMENT
    };

public:
    class CAscCefJSDialogHandler : public CefJSDialogHandler
    {
    private:
        CAscApplicationManager* m_pManager;
    public:
        CAscCefJSDialogHandler() : CefJSDialogHandler()
        {
            m_pManager = NULL;
        }
        virtual ~CAscCefJSDialogHandler()
        {
        }
        virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
                                const CefString& origin_url,
                                JSDialogType dialog_type,
                                const CefString& message_text,
                                const CefString& default_prompt_text,
                                CefRefPtr<CefJSDialogCallback> callback,
                                bool& suppress_message) OVERRIDE
        {
            //suppress_message = true;
            return true;
        }
        virtual bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
                                          const CefString& message_text,
                                          bool is_reload,
                                          CefRefPtr<CefJSDialogCallback> callback) OVERRIDE
        {
            if (m_pManager && m_pManager->GetDebugInfoSupport())
                callback->Continue(true, "");

            return true;
        }

        void SetAppManager(CAscApplicationManager* pManager)
        {
            m_pManager = pManager;
        }

    public:
        IMPLEMENT_REFCOUNTING(CAscCefJSDialogHandler);
    };

public:
    CAscClientHandler() : client::ClientHandler(this, false, "https://onlyoffice.com/")
    {
        m_pParent = NULL;
        m_bIsLoaded = false;

        m_bIsEditorTypeSet = false;
        m_nBeforeBrowserCounter = 0;

        browser_id_ = 0;

        m_pCefJSDialogHandler = new CAscCefJSDialogHandler();
    }

    virtual ~CAscClientHandler()
    {
    }

    CefRefPtr<CefBrowser> GetBrowser() const
    {
        return browser_;
    }

    int GetBrowserId() const
    {
        return browser_id_;
    }

    bool CheckPopup(std::wstring sUrl, bool bIsBeforeBrowse = false, bool bIsBackground = false, bool bIsNotOpenLinks = false)
    {
        NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
        if (NULL != m_pParent && NULL != m_pParent->GetAppManager())
            pListener = m_pParent->GetAppManager()->GetEventListener();

        bool bIsEditor      = (sUrl.find(L"files/doceditor.aspx?fileid") == std::wstring::npos) ? false : true;

        bool bIsDownload    = false;
        if (sUrl.find(L"filehandler.ashx?action=download") != std::wstring::npos)
            bIsDownload     = true;
        else if (sUrl.find(L"filehandler.ashx?action=view") != std::wstring::npos)
            bIsDownload     = true;

        if (!bIsBeforeBrowse && !bIsEditor && !bIsDownload && !bIsNotOpenLinks)
        {
            if (m_pParent && (m_pParent->GetType() == cvwtEditor || m_pParent->GetType() == cvwtSimple))
            {
                NSEditorApi::CAscOnOpenExternalLink* pData = new NSEditorApi::CAscOnOpenExternalLink();
                pData->put_Id(m_pParent->GetId());
                pData->put_Url(sUrl);

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK);
                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
            }
            return true;
        }

        if (bIsDownload)
        {
            if (NULL != m_pParent && NULL != pListener)
            {
                GetBrowser()->GetHost()->StartDownload(sUrl);
                
#if 0
                NSEditorApi::CAscDownloadFileInfo* pData = new NSEditorApi::CAscDownloadFileInfo();
                pData->put_Url(sUrl);
                pData->put_FilePath(L"");
                pData->put_Percent(0);
                pData->put_IsComplete(false);
                pData->put_Id(m_pParent->GetId());

                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START;
                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
#endif
            }

            return true;
        }

        if (bIsEditor)
        {
            sUrl += L"&placement=desktop";

            if (NULL != pListener)
            {
                NSEditorApi::CAscCreateTab* pData = new NSEditorApi::CAscCreateTab();
                pData->put_Url(sUrl);

#if 1
                if (bIsBackground)
                {
                    pData->put_Active(false);
                }
#endif

                if (true)
                {
                    CCefView* pEqual = m_pParent->GetAppManager()->GetViewByUrl(sUrl);
                    if (NULL != pEqual)
                        pData->put_IdEqual(pEqual->GetId());
                }

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_CREATETAB);
                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
            }

            return true;
        }

        return false;
    }

    virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() OVERRIDE
    {
        return m_pCefJSDialogHandler;
    }

    virtual bool OnBeforePopup(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        const CefString& target_frame_name,
        CefLifeSpanHandler::WindowOpenDisposition target_disposition,
        bool user_gesture,
        const CefPopupFeatures& popupFeatures,
        CefWindowInfo& windowInfo,
        CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings,
        bool* no_javascript_access) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();

        std::wstring sUrl = target_url.ToWString();

        CheckPopup(sUrl, false, (WOD_NEW_BACKGROUND_TAB == target_disposition) ? true : false);

        return true;
    }

    virtual bool OnOpenURLFromTab(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        CefRequestHandler::WindowOpenDisposition target_disposition,
        bool user_gesture) OVERRIDE {
        CEF_REQUIRE_IO_THREAD();

        std::wstring sUrl = target_url.ToWString();

        CheckPopup(sUrl, false, (WOD_NEW_BACKGROUND_TAB == target_disposition) ? true : false, true);

        return true;
    }

    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool is_redirect)
    {
        std::wstring sUrl = request->GetURL().ToWString();

        if (m_pParent->m_pInternal->m_bIsSSO)
        {
            std::wstring::size_type pos1 = sUrl.find(L"?token=");
            if (pos1 != std::wstring::npos)
            {
                std::wstring sToken = sUrl.substr(pos1 + 7);
                std::wstring sUrlPortal = sUrl.substr(0, pos1);

                NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
                if (NULL != m_pParent && NULL != m_pParent->GetAppManager())
                    pListener = m_pParent->GetAppManager()->GetEventListener();

                if (pListener)
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_SSO_TOKEN);
                    NSEditorApi::CAscSSOToken* pData = new NSEditorApi::CAscSSOToken();
                    pData->put_Token(sToken);
                    pData->put_Url(sUrlPortal);
                    pEvent->m_pData = pData;

                    pListener->OnEvent(pEvent);
                }

                sUrlPortal += L"products/files/?desktop=true";
                m_pParent->load(sUrlPortal);
                return true;
            }
        }

        std::wstring sTest1 = sUrl;
        std::wstring sTest2 = m_pParent->m_pInternal->m_strUrl;
        NSCommon::url_correct(sTest1);
        NSCommon::url_correct(sTest2);

        NSCommon::string_replace(sTest1, L" ", L"%20");
        NSCommon::string_replace(sTest2, L" ", L"%20");
        NSCommon::string_replace(sTest1, L"\\", L"/");
        NSCommon::string_replace(sTest2, L"\\", L"/");

        if (frame->IsMain() && m_nBeforeBrowserCounter != 0 && sUrl.find(L"file://") == 0 && sTest1 != sTest2)
            return true;

        m_nBeforeBrowserCounter++;

        CefViewWrapperType eType = this->m_pParent->GetType();

        if (m_pParent && m_pParent->GetType() == cvwtSimple)
        {
            bool bHandled = CheckPopup(sUrl, true);
            if (bHandled)
                return true;

#if 0
            // блокируем, если не первый раз
            if (m_nBeforeBrowserCounter > 1)
                return true;
#else
            // если делать, как выше, то множественные логины отрубятся.
            // и дебаггер тоже
            if (sUrl.find(L"files/#") != std::wstring::npos)
                return true;
#endif
        }

        bool ret = client::ClientHandler::OnBeforeBrowse(browser, frame, request, is_redirect);
        if (NULL != m_pParent)
        {            
            m_pParent->resizeEvent();
            m_pParent->focus();

            if (!m_bIsLoaded)
            {
                m_pParent->GetWidgetImpl()->child_loaded();
                m_bIsLoaded = true;
            }
        }
        return ret;
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message)
    {
        CEF_REQUIRE_UI_THREAD();

        NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
        if (NULL != m_pParent && NULL != m_pParent->GetAppManager())
            pListener = m_pParent->GetAppManager()->GetEventListener();

        // Check for messages from the client renderer.
        std::string message_name = message->GetName();
        if (message_name == "EditorType" && !m_bIsEditorTypeSet)
        {
            m_bIsEditorTypeSet = true;

            int nType = message->GetArgumentList()->GetInt(0);
            m_pParent->m_pInternal->m_nEditorType = nType;

            if (NULL != pListener)
            {
                NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
                pData->put_Id(m_pParent->GetId());
                pData->put_Type(nType);

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE);
                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "spell_check_task")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->SpellCheck(message->GetArgumentList()->GetInt(0),
                                                       message->GetArgumentList()->GetString(1).ToString(),
                                                       message->GetArgumentList()->GetInt(2));
            }

            return true;
        }
        else if (message_name == "create_editor_api")
        {
            if (m_pParent)
            {
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_CONTROL_ID;
                m_pParent->Apply(pEvent);

                CAscApplicationManager* pManager = m_pParent->GetAppManager();
                if (pManager)
                    pManager->SendKeyboardAttack();

                NSEditorApi::CAscMenuEvent* pEvent2 = new NSEditorApi::CAscMenuEvent();
                pEvent2->m_nType = ASC_MENU_EVENT_TYPE_CEF_SYNC_COMMAND;
                m_pParent->Apply(pEvent2);

                // в обычных случаях - просто увеличится счетчик и все
                m_pParent->m_pInternal->m_oConverterToEditor.NativeViewerOpen();
            }

            return true;
        }
        else if (message_name == "is_cookie_present")
        {
            CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
            pVisitor->m_pCallback       = this;
            pVisitor->m_sDomain         = message->GetArgumentList()->GetString(0).ToString();
            pVisitor->m_sCookieSearch   = message->GetArgumentList()->GetString(1).ToString();

            pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));

            return true;
        }
        else if (message_name == "on_check_auth")
        {
            CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
            pVisitor->m_pCallback       = this;

            pVisitor->m_sCookieSearch   = "asc_auth_key";

            int nCount = message->GetArgumentList()->GetInt(0);
            for (int i = 0; i < nCount; i++)
            {
                pVisitor->m_arDomains.push_back(message->GetArgumentList()->GetString(i + 1));
            }

            pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));

            return true;
        }
        else if (message_name == "set_cookie")
        {
            CCefCookieSetter* pVisitor = new CCefCookieSetter();
            pVisitor->m_pCallback       = this;

            pVisitor->m_sUrl            = message->GetArgumentList()->GetString(0).ToString();
            pVisitor->m_sDomain         = message->GetArgumentList()->GetString(1).ToString();
            pVisitor->m_sPath           = message->GetArgumentList()->GetString(2).ToString();
            pVisitor->m_sCookieKey      = message->GetArgumentList()->GetString(3).ToString();
            pVisitor->m_sCookieValue    = message->GetArgumentList()->GetString(4).ToString();

            pVisitor->SetCookie(CefCookieManager::GetGlobalManager(NULL));

            return true;
        }
        else if (message_name == "onDocumentModifiedChanged")
        {
            if (m_pParent && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = new NSEditorApi::CAscCefMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED;
                pEvent->put_SenderId(m_pParent->GetId());

                bool bValue = message->GetArgumentList()->GetBool(0);
                /*
                if (!m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
                {
                    // в будущем переделать!!!
                    std::wstring sChangesFile = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/changes/changes0.json";
                    if (!NSFile::CFileBinary::Exists(sChangesFile))
                    {
                        bValue = false;
                    }
                }
                */

                NSEditorApi::CAscDocumentModifyChanged* pData = new NSEditorApi::CAscDocumentModifyChanged();
                pData->put_Id(m_pParent->GetId());
                pData->put_Changed(bValue);
                pEvent->m_pData = pData;

                m_pParent->SetModified(bValue);
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "set_document_name")
        {
            if (m_pParent && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME);

                NSEditorApi::CAscDocumentName* pData = new NSEditorApi::CAscDocumentName();
                pData->put_Id(m_pParent->GetId());
                pData->put_Name(message->GetArgumentList()->GetString(0).ToWString());

                pEvent->m_pData = pData;

                if (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
                {
                    CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
                    if (_frame)
                    {
                        std::wstring sUrl = _frame->GetURL().ToWString();
                        NSCommon::url_correct(sUrl);
                        std::wstring sPath = L"";

                        std::wstring::size_type pos = 0;
                        std::wstring::size_type pos1 = sUrl.find(L"//");
                        if (pos1 != std::wstring::npos)
                            pos = pos1 + 3;

                        pos1 = sUrl.find(L"/", pos);
                        if (0 < pos1)
                            sPath = sUrl.substr(0, pos1);

                        int nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                        if  (2 == m_pParent->m_pInternal->m_nEditorType)
                            nType = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
                        else if  (1 ==m_pParent->m_pInternal->m_nEditorType)
                            nType = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;

                        m_pParent->GetAppManager()->m_pInternal->Recents_Add(sPath + L"/" + pData->get_Name(),
                                                                             nType, sUrl);

                        pData->put_Path(sPath);
                        pData->put_Url(sUrl);
                    }
                }
                else if (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved)
                {
                    std::wstring sPath = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
#ifdef WIN32
                    NSCommon::string_replace(sPath, L"/", L"\\");
#endif
                    pData->put_Path(sPath);
                }

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "on_document_save")
        {
            // для локального файла посылаем когда прошла конвертация
            if (m_pParent &&
                    m_pParent->GetAppManager()->GetEventListener() &&
                    m_pParent->m_pInternal->m_bIsNativeSave &&
                    m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
            {
                m_pParent->m_pInternal->m_bIsNativeSave = false;

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);

                NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
                pData->put_Id(m_pParent->GetId());

                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "js_message")
        {
            if (m_pParent && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE);

                NSEditorApi::CAscJSMessage* pData = new NSEditorApi::CAscJSMessage();
                pData->put_Id(m_pParent->GetId());
                pData->put_Name(message->GetArgumentList()->GetString(0).ToWString());
                pData->put_Value(message->GetArgumentList()->GetString(1).ToWString());

                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "on_exec_command")
        {
            if (m_pParent && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);

                NSEditorApi::CAscExecCommand* pData = new NSEditorApi::CAscExecCommand();
                pData->put_Command(message->GetArgumentList()->GetString(0).ToWString());

                if (message->GetArgumentList()->GetSize() == 2)
                    pData->put_Param(message->GetArgumentList()->GetString(1).ToWString());

                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "print_start")
        {
            if (m_pParent)
            {
                m_pParent->m_pInternal->m_oPrintData.m_sDocumentUrl = message->GetArgumentList()->GetString(0).ToWString();
                m_pParent->m_pInternal->m_oPrintData.m_sFrameUrl = message->GetArgumentList()->GetString(2).ToWString();
                m_pParent->m_pInternal->m_oPrintData.m_arPages.SetCount(message->GetArgumentList()->GetInt(1));
                m_pParent->m_pInternal->m_oPrintData.m_sThemesUrl = message->GetArgumentList()->GetString(3).ToWString();
                m_pParent->m_pInternal->m_oPrintData.m_nCurrentPage = message->GetArgumentList()->GetInt(4);
                
                if (!m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
                {
                    m_pParent->m_pInternal->m_oPrintData.m_sDocumentUrl = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/";
                }

                m_pParent->m_pInternal->m_oPrintData.CalculateImagePaths();

                if (m_pParent->GetAppManager()->GetEventListener())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_START);

                    NSEditorApi::CAscTypeId* pData = new NSEditorApi::CAscTypeId();
                    pData->put_Id(m_pParent->GetId());
                    pEvent->m_pData = pData;

                    m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
                }
            }
            return true;
        }
        else if (message_name == "print_page")
        {
            if (m_pParent)
            {
                int nIndex = message->GetArgumentList()->GetInt(1);
                int nProgress = 100 * (nIndex + 1) / m_pParent->m_pInternal->m_oPrintData.m_arPages.GetCount();

                if (nProgress >= 0 && nProgress <= 100)
                {
                    m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Base64 = message->GetArgumentList()->GetString(0).ToString();
                    m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Width  = message->GetArgumentList()->GetDouble(2);
                    m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Height = message->GetArgumentList()->GetDouble(3);
                }

                if (m_pParent->GetAppManager()->GetEventListener())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS);

                    NSEditorApi::CAscPrintProgress* pData = new NSEditorApi::CAscPrintProgress();
                    pData->put_Id(m_pParent->GetId());
                    pData->put_Progress(nProgress);

                    pEvent->m_pData = pData;

                    m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
                }
            }
            return true;
        }
        else if (message_name == "print_end")
        {
            if (m_pParent)
            {
                if (m_pParent->GetAppManager()->GetEventListener())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END);

                    NSEditorApi::CAscPrintEnd* pData = new NSEditorApi::CAscPrintEnd();
                    pData->put_Id(m_pParent->GetId());
                    pData->put_PagesCount(m_pParent->m_pInternal->m_oPrintData.m_arPages.GetCount());
                    pData->put_CurrentPage(m_pParent->m_pInternal->m_oPrintData.m_nCurrentPage);
                    pEvent->m_pData = pData;

                    m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);

#if 0
                    // TEST
                    m_pParent->m_pInternal->m_oPrintData.Print_Start();

                    for (int i = 0; i < m_pParent->m_pInternal->m_oPrintData.m_arPages.GetCount(); ++i)
                        m_pParent->m_pInternal->m_oPrintData.TestSaveToRasterFile(L"D:\\test_page" + std::to_wstring(i) + L".png", 793, 1122, i);

                    m_pParent->m_pInternal->m_oPrintData.Print_End();
#endif
                }
            }
            return true;
        }
        else if (message_name == "print")
        {
            if (m_pParent)
            {
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_START;
                m_pParent->Apply(pEvent);
            }
            return true;
        }
        else if (message_name == "load_js")
        {
            if (m_pParent)
            {
                NSEditorApi::CAscEditorScript* pData = new NSEditorApi::CAscEditorScript();
                pData->put_Url(message->GetArgumentList()->GetString(0).ToWString());
                pData->put_Destination(message->GetArgumentList()->GetString(1).ToWString());
                pData->put_Id(m_pParent->GetId());
                pData->put_FrameId(message->GetArgumentList()->GetInt(2));

                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION;
                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->Apply(pEvent);
            }
            return true;
        }
        else if (message_name == "onfullscreenenter")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER);
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "onfullscreenleave")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE);
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "onlocaldocument_loadstart")
        {
            if (m_pParent)
            {
                m_pParent->m_pInternal->LocalFile_IncrementCounter();
            }

            return true;
        }
        else if (message_name == "onlocaldocument_open")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN);
                NSEditorApi::CAscLocalFileOpen* pData = new NSEditorApi::CAscLocalFileOpen();
                if (message->GetArgumentList()->GetSize() == 1)
                    pData->put_Directory(message->GetArgumentList()->GetString(0).ToWString());
                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "onlocaldocument_create")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE);
                NSEditorApi::CAscLocalFileCreate* pData = new NSEditorApi::CAscLocalFileCreate();
                pData->put_Type(message->GetArgumentList()->GetInt(0));
                pEvent->m_pData = pData;
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "onlocaldocument_sendrecents")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recents_Dump();
            }
            return true;
        }
        else if (message_name == "onlocaldocument_openrecents")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = new NSEditorApi::CAscLocalOpenFileRecent_Recover();
                pData->put_Id(message->GetArgumentList()->GetInt(0));
                pData->put_IsRecover(false);
                pData->put_Path(m_pParent->GetAppManager()->m_pInternal->m_arRecents[pData->get_Id()].m_sPath);

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN);
                pEvent->m_pData = pData;
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "onlocaldocument_removerecents")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recents_Remove(message->GetArgumentList()->GetInt(0));
            }
            return true;
        }
        else if (message_name == "onlocaldocument_removeallrecents")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recents_RemoveAll();
            }
            return true;
        }
        else if (message_name == "onlocaldocument_sendrecovers")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recovers_Dump();
            }
            return true;
        }
        else if (message_name == "onlocaldocument_openrecovers")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = new NSEditorApi::CAscLocalOpenFileRecent_Recover();
                pData->put_Id(message->GetArgumentList()->GetInt(0));
                pData->put_IsRecover(true);
                pData->put_Path(m_pParent->GetAppManager()->m_pInternal->m_arRecovers[pData->get_Id()].m_sPath);

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN);
                pEvent->m_pData = pData;
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "onlocaldocument_removerecovers")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recovers_Remove(message->GetArgumentList()->GetInt(0), true);
            }
            return true;
        }
        else if (message_name == "onlocaldocument_removeallrecovers")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                m_pParent->GetAppManager()->m_pInternal->Recovers_RemoveAll();
            }
            return true;
        }
        else if (message_name == "onlocaldocument_onsavestart")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                bool bIsNeedSave = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved ? false : true;
                std::string sParams = message->GetArgumentList()->GetString(0).ToString();
                std::wstring sPassword = message->GetArgumentList()->GetString(1).ToWString();

                bool bIsSaveAs = (sParams.find("saveas=true") != std::string::npos) ? true : false;
                bool bIsNeedSaveDialog = bIsNeedSave || bIsSaveAs || (!m_pParent->m_pInternal->LocalFile_IsSupportSaveCurrentFormat());

                CApplicationManagerAdditionalBase* pAdditional = m_pParent->GetAppManager()->m_pInternal->m_pAdditional;
                if (pAdditional)
                {
                    if (pAdditional->Local_Save_End(bIsNeedSaveDialog, m_pParent->GetId(), m_pParent->m_pInternal->m_handler->GetBrowser()))
                        return true;
                }

                m_pParent->m_pInternal->m_oLocalInfo.m_bIsRetina = (sParams.find("retina=true") != std::string::npos) ? true : false;
                m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sPassword = sPassword;

                if (bIsNeedSaveDialog)
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE);
                    NSEditorApi::CAscLocalSaveFileDialog* pData = new NSEditorApi::CAscLocalSaveFileDialog();
                    pData->put_Id(m_pParent->GetId());

                    if (!bIsNeedSave)
                        pData->put_Path(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc);
                    else
                        pData->put_Path(NSCommon::GetFileName(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc));

                    pData->put_FileType(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);
                    m_pParent->m_pInternal->LocalFile_GetSupportSaveFormats(pData->get_SupportFormats());
                    pEvent->m_pData = pData;

                    m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
                    m_pParent->m_pInternal->m_bIsSavingDialog = true;
                }
                else
                {
                    m_pParent->m_pInternal->LocalFile_SaveStart();
                }
            }
            return true;
        }
        else if (message_name == "onlocaldocument_onaddimage")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
                NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
                pData->put_Id(m_pParent->GetId());
                pEvent->m_pData = pData;
                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "on_logout")
        {
            if (m_pParent && m_pParent->GetAppManager())
            {
                std::wstring sPortal = message->GetArgumentList()->GetString(0).ToWString();
                m_pParent->GetAppManager()->Logout(sPortal);
            }
            return true;
        }
        else if (message_name == "on_setadvancedoptions")
        {
            if (m_pParent)
            {
                int nCount = (int)message->GetArgumentList()->GetSize();
                m_pParent->m_pInternal->m_oConverterToEditor.m_sAdditionalConvertation = message->GetArgumentList()->GetString(0).ToWString();

                m_pParent->m_pInternal->LocalFile_Start();
            }
            return true;
        }
        else if (message_name == "DropOfficeFiles")
        {
            if (m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscLocalOpenFiles* pData = new NSEditorApi::CAscLocalOpenFiles();

                int nCount = (int)message->GetArgumentList()->GetSize();

                std::vector<std::wstring>& arFolder = pData->get_Files();

                for (int i = 0; i < nCount; i++)
                {
                    std::wstring sValue = message->GetArgumentList()->GetString(i).ToWString();

                    COfficeFileFormatChecker oChecker;
                    if (oChecker.isOfficeFile(sValue))
                    {
                        if (AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT == oChecker.nFileType)
                        {
                            std::wstring sExt = NSCommon::GetFileExtention(sValue);
                            if (sExt == L"txt" || sExt == L"xml")
                                arFolder.push_back(sValue);
                        }
                        else
                            arFolder.push_back(sValue);
                    }
                }

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN);
                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
            return true;
        }        
        else if (message_name == "on_init_js_context")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                std::vector<NSEditorApi::CAscMenuEvent*>* pArr = NULL;
                CApplicationManagerAdditionalBase* pAdditional = m_pParent->GetAppManager()->m_pInternal->m_pAdditional;

                if (pAdditional && pAdditional->m_arApplyEvents)
                {
                    for (std::vector<NSEditorApi::CAscMenuEvent*>::iterator i = pAdditional->m_arApplyEvents->begin();
                         i != pAdditional->m_arApplyEvents->end(); i++)
                    {
                        m_pParent->Apply(*i);
                    }

                    pAdditional->m_arApplyEvents->clear();
                    pAdditional->m_arApplyEvents = NULL;
                }                
            }
            return true;
        }
        else if (message_name == "native_viewer_onopened")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                std::string sBase64 = message->GetArgumentList()->GetString(0).ToString();
                m_pParent->m_pInternal->m_oConverterToEditor.NativeViewerOpenEnd(sBase64);
            }
            return true;
        }
        else if (message_name == "on_signature_sign")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                std::string sId = message->GetArgumentList()->GetString(0).ToString();
                std::wstring sGuid = message->GetArgumentList()->GetString(1).ToWString();
                std::wstring sUrl = message->GetArgumentList()->GetString(2).ToWString();
                std::wstring sUrl2 = message->GetArgumentList()->GetString(3).ToWString();

                std::wstring sFile = m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_sFileSrc;
                if (sFile.empty())
                    sFile = m_pParent->m_pInternal->m_oConverterToEditor.m_oInfo.m_sFileSrc;

                std::wstring sUnzipDir = NSDirectory::CreateDirectoryWithUniqueName(NSDirectory::GetTempPath());
                NSDirectory::CreateDirectory(sUnzipDir);

                COfficeUtils oCOfficeUtils(NULL);
                oCOfficeUtils.ExtractToDirectory(sFile, sUnzipDir, NULL, 0);

                ICertificate* pCertificate = ICertificate::GetById(sId);
                if (pCertificate)
                {
                    COOXMLSigner oOOXMLSigner(sUnzipDir, pCertificate);

                    oOOXMLSigner.SetGuid(sGuid);
                    oOOXMLSigner.SetImageValid(sUrl);
                    oOOXMLSigner.SetImageInvalid(sUrl2);

                    oOOXMLSigner.Sign();

                    CASCFileConverterToEditor* pConverter = &m_pParent->m_pInternal->m_oConverterToEditor;
                    pConverter->CheckSignaturesByDir(sUnzipDir);

                    std::wstring sSignatures = pConverter->GetSignaturesJSON();

                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_update_signatures");
                    message->GetArgumentList()->SetString(0, sSignatures);
                    browser->SendProcessMessage(PID_RENDERER, message);
                }

                RELEASEOBJECT(pCertificate);

                oCOfficeUtils.CompressFileOrDirectory(sUnzipDir, sFile, true);

                NSDirectory::DeleteDirectory(sUnzipDir);
            }
            return true;
        }
        else if (message_name == "on_signature_viewcertificate")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                int nIndex = message->GetArgumentList()->GetInt(0);
                COOXMLVerifier* pVerifier = m_pParent->m_pInternal->m_oConverterToEditor.m_pVerifier;
                if (NULL != pVerifier)
                {
                    COOXMLSignature* pSign = pVerifier->GetSignature(nIndex);
                    int nRet = pSign ? pSign->GetCertificate()->ShowCertificate() : 0;

                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_viewcertificate_ret");
                    message->GetArgumentList()->SetString(0, std::to_string(nRet));
                    browser->SendProcessMessage(PID_RENDERER, message);
                }
            }
            return true;
        }
        else if (message_name == "on_signature_defaultcertificate")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                CCertificateInfo info = ICertificate::GetDefault();

                CJSONSimple serializer;
                serializer.Start();
                serializer.Write(L"name", info.GetName());
                serializer.Next();
                serializer.Write(L"id", info.GetId());
                serializer.Next();
                serializer.Write(L"date", info.GetDate());
                serializer.End();

                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_defaultcertificate_ret");
                message->GetArgumentList()->SetString(0, serializer.GetData());
                browser->SendProcessMessage(PID_RENDERER, message);
            }
            return true;
        }
        else if (message_name == "on_signature_selectsertificate")
        {
            if (m_pParent && m_pParent->m_pInternal)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_selectsertificate_ret");

                ICertificate* pCert = ICertificate::CreateInstance();
                if (pCert->ShowSelectDialog())
                {
                    CCertificateInfo info = pCert->GetInfo();

                    CJSONSimple serializer;
                    serializer.Start();
                    serializer.Write(L"name", info.GetName());
                    serializer.Next();
                    serializer.Write(L"id", info.GetId());
                    serializer.Next();
                    serializer.Write(L"date", info.GetDate());
                    serializer.End();

                    message->GetArgumentList()->SetString(0, serializer.GetData());
                }

                RELEASEOBJECT(pCert);
                browser->SendProcessMessage(PID_RENDERER, message);
            }
            return true;
        }
        else if (message_name == "on_open_filename_dialog")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_Filter(message->GetArgumentList()->GetString(0).ToWString());
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_file_save_question")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
            NSEditorApi::CAscEditorSaveQuestion* pData = new NSEditorApi::CAscEditorSaveQuestion();
            pData->put_Id(m_pParent->GetId());
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_start_reporter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_REPORTER_CREATE);
            NSEditorApi::CAscReporterCreate* pData = new NSEditorApi::CAscReporterCreate();
            m_pParent->m_pInternal->m_nReporterChildId = m_pParent->GetAppManager()->GenerateNextViewId();

            CAscReporterData* pCreate = new CAscReporterData();
            pCreate->Id = m_pParent->m_pInternal->m_nReporterChildId;
            pCreate->ParentId = m_pParent->GetId();
            pCreate->Url = message->GetArgumentList()->GetString(0).ToWString();
            pCreate->LocalRecoverFolder = message->GetArgumentList()->GetString(1).ToWString();
            pData->put_Data(pCreate);

            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_end_reporter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_REPORTER_END);
            NSEditorApi::CAscTypeId* pData = new NSEditorApi::CAscTypeId();
            pData->put_Id(m_pParent->m_pInternal->m_nReporterChildId);
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "send_to_reporter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO);
            NSEditorApi::CAscReporterMessage* pData = new NSEditorApi::CAscReporterMessage();
            pData->put_ReceiverId(m_pParent->m_pInternal->m_nReporterChildId);
            pData->put_Message(message->GetArgumentList()->GetString(0).ToWString());
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "send_from_reporter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM);
            NSEditorApi::CAscReporterMessage* pData = new NSEditorApi::CAscReporterMessage();
            pData->put_ReceiverId(m_pParent->m_pInternal->m_nReporterParentId);
            pData->put_Message(message->GetArgumentList()->GetString(0).ToWString());
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            return true;
        }

        CAscApplicationManager_Private* pInternalMan = m_pParent->GetAppManager()->m_pInternal;
        if (pInternalMan->m_pAdditional && pInternalMan->m_pAdditional->OnProcessMessageReceived(browser, source_process, message))
            return true;

        if (message_router_->OnProcessMessageReceived(browser, source_process, message))
        {
            return true;
        }

        return false;
    }

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        client::ClientHandler::OnAfterCreated(browser);
        if (!GetBrowser())
        {
            // We need to keep the main child window, but not popup windows
            browser_ = browser;
            browser_id_ = browser->GetIdentifier();
        }        
    }

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        if (--browser_count_ == 0)
        {
            // Remove and delete message router handlers.
            MessageHandlerSet::const_iterator it =
                message_handler_set_.begin();
            for (; it != message_handler_set_.end(); ++it)
            {
                message_router_->RemoveHandler(*(it));
                delete *(it);
            }
            message_handler_set_.clear();
            message_router_ = NULL;
        }

        NotifyBrowserClosed(browser);
    }

    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        NotifyBrowserClosing(browser);

        SetParentNULL(browser->GetHost()->GetWindowHandle());

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

#ifdef WIN32
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                             bool isLoading,
                                             bool canGoBack,
                                             bool canGoForward) {
      client::ClientHandler::OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);

      if (!isLoading)
      {
          // вот тут уже можно делать зум!!!
          m_pParent->m_pInternal->m_bIsWindowsCheckZoom = true;
          m_pParent->resizeEvent();
      }
    }
#endif

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) OVERRIDE
    {
        if (m_pParent && errorCode != ERR_ABORTED)
        {
            m_pParent->load(L"ascdesktop://loaderror.html");
        }
    }

    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();
        
        if (m_pParent && !m_pParent->GetAppManager()->GetDebugInfoSupport())
        {
            model->Clear();
            return;
        }

        int _remove_commands[] = {MENU_ID_BACK,
                                  MENU_ID_FORWARD,
                                  MENU_ID_RELOAD,
                                  MENU_ID_RELOAD_NOCACHE,
                                  MENU_ID_STOPLOAD,

                                  // Editing.
                                  MENU_ID_UNDO,
                                  MENU_ID_REDO,
                                  //MENU_ID_CUT,
                                  //MENU_ID_COPY,
                                  //MENU_ID_PASTE,
                                  //MENU_ID_DELETE,
                                  //MENU_ID_SELECT_ALL,

                                  // Miscellaneous.
                                  MENU_ID_FIND,
                                  MENU_ID_PRINT,
                                  MENU_ID_VIEW_SOURCE,

                                  // Spell checking word correction suggestions.
                                  MENU_ID_SPELLCHECK_SUGGESTION_0,
                                  MENU_ID_SPELLCHECK_SUGGESTION_1,
                                  MENU_ID_SPELLCHECK_SUGGESTION_2,
                                  MENU_ID_SPELLCHECK_SUGGESTION_3,
                                  MENU_ID_SPELLCHECK_SUGGESTION_4,
                                  MENU_ID_SPELLCHECK_SUGGESTION_LAST,
                                  MENU_ID_NO_SPELLING_SUGGESTIONS,
                                  MENU_ID_ADD_TO_DICTIONARY
                                 };

        int _remove_count = 23 - 5;
        for (int i = 0; i < _remove_count; ++i)
        {
            if (-1 != model->GetIndexOf(_remove_commands[i]))
            {
                model->Remove(_remove_commands[i]);
            }
        }

        bool bIsExist = false;
        int nNaturalCount = model->GetCount();
        for (int i = 0; i < nNaturalCount; ++i)
        {
             CefMenuModel::MenuItemType type = model->GetTypeAt(i);
             if (type != MENUITEMTYPE_SEPARATOR)
             {
                 bIsExist = true;
                 break;
             }
        }

        if (bIsExist)
        {
            // удаляем все первые и последние сепараторы
            int nNaturalCount2 = model->GetCount();
            for (int i = 0; i < nNaturalCount2; ++i)
            {
                 CefMenuModel::MenuItemType type = model->GetTypeAt(i);
                 if (type != MENUITEMTYPE_SEPARATOR)
                 {
                     break;
                 }
                 model->RemoveAt(i);
                 --nNaturalCount2;
            }

            for (int i = nNaturalCount2 - 1; i >= 0; --i)
            {
                 CefMenuModel::MenuItemType type = model->GetTypeAt(i);
                 if (type != MENUITEMTYPE_SEPARATOR)
                 {
                     break;
                 }
                 model->RemoveAt(i);
                 --nNaturalCount2;
            }
        }

        if (!bIsExist)
            model->Clear();

        if (m_pParent && m_pParent->GetAppManager()->GetDebugInfoSupport())
        {
            if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0)
            {
                // Add a separator if the menu already has items.
                if (model->GetCount() > 0)
                    model->AddSeparator();

                // Add DevTools items to all context menus.
                model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
                model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, "Close DevTools");
                model->AddSeparator();
                model->AddItem(CLIENT_ID_INSPECT_ELEMENT, "Inspect Element");
            }
        }

        if (delegate_)
          delegate_->OnBeforeContextMenu(model);
    }

    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
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
        if (event.type == KEYEVENT_RAWKEYDOWN && m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
        {
            if ((112 == event.windows_key_code) && m_pParent->GetAppManager()->GetDebugInfoSupport())
            {
                m_pParent->m_pInternal->m_handler->ShowDevTools(m_pParent->m_pInternal->m_handler->GetBrowser(), CefPoint());
                return false;
            }

            NSEditorApi::CAscKeyboardDown* pData = new NSEditorApi::CAscKeyboardDown();
            pData->put_KeyCode(event.windows_key_code);
            int nMods = event.modifiers;

            if ((nMods & EVENTFLAG_CONTROL_DOWN) != 0)
                pData->put_IsCtrl(true);
            if ((nMods & EVENTFLAG_SHIFT_DOWN) != 0)
                pData->put_IsShift(true);
            if ((nMods & EVENTFLAG_ALT_DOWN) != 0)
                pData->put_IsAlt(true);
            
#ifdef _MAC
            if ((nMods & EVENTFLAG_COMMAND_DOWN) != 0)
                pData->put_IsCommandMac(true);
#endif

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN);
            pEvent->m_pData = pData;
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            
#if 0
            if (!pData->get_IsAlt() &&
                !pData->get_IsCtrl() &&
                !pData->get_IsShift() &&
                pData->get_IsCommandMac() &&
                event.focus_on_editable_field)
            {
            }
#endif

            if (((nMods & EVENTFLAG_CONTROL_DOWN) != 0) && event.windows_key_code == 9)
                return true; // tab!!!
        }

        return false;
    }

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) OVERRIDE
    {
        std::wstring url = request->GetURL().ToWString();
        if (url.find(L"require.js") != std::wstring::npos)
        {
            const std::wstring& sAppDataPath = m_pParent->GetAppManager()->m_oSettings.app_data_path;
            std::string sAppData = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(sAppDataPath.c_str(),
                                                                                     (LONG)sAppDataPath.length());

            const std::wstring& sFontsDataPath = m_pParent->GetAppManager()->m_oSettings.fonts_cache_info_path;
            std::string sFontsData = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(sFontsDataPath.c_str(),
                                                                                     (LONG)sFontsDataPath.length());

            sAppData = "window[\"AscDesktopEditor_AppData\"] = function(){return \"" + sAppData + "\";}\n";
            sAppData += ("window[\"AscDesktopEditor_FontsData\"] = function(){return \"" + sFontsData + "\";}\n");

            sAppData += "window.local_load = {};\n\
window.local_correct_url = function(url)\n\
{\n\
    return url.substring(url.lastIndexOf('/')+1);\n\
};\n\
window.local_load_add = function(context, moduleName, url)\n\
{\n\
    var _url = window.local_correct_url(url);\n\
    window.local_load[_url] = { context: context, name: moduleName };\n\
};\n\
window.local_load_remove = function(url)\n\
{\n\
    var _url = window.local_correct_url(url);\n\
    delete window.local_load[_url];\n\
};\n\
window.asc_desktop_on_load_js=function(url)\n\
{\n\
    var _url = window.local_correct_url(url);\n\
    var _data = window.local_load[_url];\n\
    if (_data)\n\
    {\n\
        _data.context.completeLoad(_data.name);\n\
        delete window.local_load[_url];\n\
    }\n\
    else if (window.asc_desktop_context)\n\
    {\n\
        window.asc_desktop_context.completeLoad();\n\
    }\n\
};\n\n";


            std::string sFooter = "\n\n\
require.load2 = require.load;\n\
require.load = function (context, moduleName, url) {\n\
    if (window.AscDesktopEditor && window.AscDesktopEditor.LoadJS)\n\
    {\n\
        window.local_load_add(context, moduleName, url);\n\
        var _ret = window.AscDesktopEditor.LoadJS(url);\n\
        if (2 != _ret)\n\
            window.local_load_remove(url);\n\
\n\
        if (1 == _ret)\n\
        {\n\
            setTimeout(function() { context.completeLoad(moduleName); }, 1);\n\
            return;\n\
        }\n\
        else if (2 == _ret)\n\
            return;\n\
    }\n\
    return this.load2(context, moduleName, url);\n\
};\n\n";

            std::wstring sPathToEditors = NSCommon::GetDirectoryName(m_pParent->GetAppManager()->m_oSettings.local_editors_path);
            sPathToEditors += L"/../../../vendor/requirejs/require.js";
            return GetLocalFileRequest(sPathToEditors, sAppData, sFooter);
        }

        if (true)
        {
            int nPos = url.find(L"apps/documenteditor/main/");

            if (nPos != std::wstring::npos)
            {
                const std::wstring& sAppDataPath = m_pParent->GetAppManager()->m_oSettings.app_data_path;

                std::wstring urlFind = url.substr(nPos);
                urlFind = sAppDataPath + L"/webdata/cloud/" + urlFind;

                return GetLocalFileRequest(urlFind);
            }
        }
        if (true)
        {
            int nPos = url.find(L"apps/presentationeditor/main/");

            if (nPos != std::wstring::npos)
            {
                const std::wstring& sAppDataPath = m_pParent->GetAppManager()->m_oSettings.app_data_path;

                std::wstring urlFind = url.substr(nPos);
                urlFind = sAppDataPath + L"/webdata/cloud/" + urlFind;

                return GetLocalFileRequest(urlFind);
            }
        }
        if (true)
        {
            int nPos = url.find(L"apps/spreadsheeteditor/main/");

            if (nPos != std::wstring::npos)
            {
                const std::wstring& sAppDataPath = m_pParent->GetAppManager()->m_oSettings.app_data_path;

                std::wstring urlFind = url.substr(nPos);
                urlFind = sAppDataPath + L"/webdata/cloud/" + urlFind;

                return GetLocalFileRequest(urlFind);
            }
        }

#if 0

        // TEST
        std::wstring sPathVersion = m_pParent->GetAppManager()->m_oSettings.app_data_path + L"/webdata/cloud/3.0b.1439/word/";
        if (true)
        {
            int nPos = url.find(L"sdk-all.js");

            if (nPos != std::wstring::npos)
            {
                std::wstring urlFind = sPathVersion + L"sdk-all.js";
                return GetLocalFileRequest(urlFind);
            }
        }
        if (true)
        {
            int nPos = url.find(L"app.js");

            if (nPos != std::wstring::npos)
            {
                std::wstring urlFind = sPathVersion + L"app.js";
                return GetLocalFileRequest(urlFind);
            }
        }

#endif

        if (false)
        {
            int nPos = url.find(L"OfficeWeb/sdk/Fonts/");

            if (nPos != std::wstring::npos)
            {
                std::wstring urlFind = url.substr(nPos + 20); // 20 = len(OfficeWeb/sdk/Fonts/)
                //CefRequest::HeaderMap _map;
                //request->GetHeaderMap(_map);
                //_map.insert(std::pair<CefString, CefString>("Access-Control-Allow-Origin", "*"));
                //_map.insert(std::pair<CefString, CefString>("Access-Control-Allow-Methods", "POST, GET, OPTIONS, HEAD"));
                //request->SetHeaderMap(_map);
                return GetLocalFileRequest(urlFind);
            }
        }

        return NULL;
    }

    CefRefPtr<CefResourceHandler> GetLocalFileRequest(const std::wstring& strFileName, const std::string& sHeaderScript = "", const std::string& sFooter = "")
    {
        NSFile::CFileBinary oFileBinary;
        if (!oFileBinary.OpenFile(strFileName))
            return NULL;

        DWORD dwSize = (DWORD)oFileBinary.GetFileSize();
        DWORD dwOffset = 0;
        DWORD dwFooterSize = (DWORD)sFooter.length();

        if (!sHeaderScript.empty())
        {
            dwOffset = (DWORD)sHeaderScript.length();
        }

        BYTE* pBytes = new BYTE[dwOffset + dwSize + dwFooterSize];
        if (dwOffset != 0)
        {
            memcpy(pBytes, sHeaderScript.c_str(), dwOffset);
        }

        DWORD dwSize2 = 0;
        oFileBinary.ReadFile(pBytes + dwOffset, dwSize, dwSize2);
        oFileBinary.CloseFile();

        if (dwSize2 != dwSize)
        {
            RELEASEARRAYOBJECTS(pBytes);
            return NULL;
        }

        if (!sFooter.empty())
        {
            memcpy(pBytes + dwOffset + dwSize, sFooter.c_str(), dwFooterSize);
        }

        std::string mime_type = GetMimeType(strFileName);
        if (mime_type.empty())
        {
            RELEASEARRAYOBJECTS(pBytes);
            return NULL;
        }

        CCefBinaryFileReaderCounter* pCounter = new CCefBinaryFileReaderCounter(pBytes);
        CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForHandler(new CefByteReadHandler(pBytes, dwOffset + dwSize + dwFooterSize, pCounter));
        if (stream.get())
        {
#if 0
            FILE* f = fopen("D:/request_test.log", "a+");
            fprintf(f, mime_type.c_str());
            fprintf(f, ": ");
            std::string sFileA = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strFileName.c_str(), (LONG)strFileName.length());
            fprintf(f, sFileA.c_str());
            fprintf(f, "\n");
            fclose(f);
#endif

            return new CefStreamResourceHandler(mime_type, stream);
        }

        pCounter->data = NULL;
        RELEASEARRAYOBJECTS(pBytes);
        return NULL;
    }

    std::string GetMimeType(const std::wstring& sFile)
    {
        int nPos = sFile.find_last_of(wchar_t('.'));
        if (nPos == std::wstring::npos)
            return "";

        std::wstring sExt = sFile.substr(nPos + 1);
        std::transform(sExt.begin(), sExt.end(), sExt.begin(), ::tolower);
        if (sExt == L"js")
            return "application/javascript";
        else if (sExt == L"css")
            return "text/css";
        else if (sExt == L"png")
            return "image/png";
        else if (sExt == L"jpg" || sExt == L"jpeg")
            return "image/jpeg";
        else if (sExt == L"gif")
            return "image/gif";
        else if (sExt == L"tiff")
            return "image/tiff";
        else if (sExt == L"ico")
            return "image/x-icon";
        else if (sExt == L"htm" || sExt == L"html")
            return "text/html";
        else if (sExt == L"json")
            return "application/json";
        //else if (sExt == L"ttf")
        //    return "application/octet-stream";
        //else if (sExt == L"otf")
        //    return "application/octet-stream";

        return "";
    }

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           TerminationStatus status) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        message_router_->OnRenderProcessTerminated(browser);

        // Load the startup URL if that's not the website that we terminated on.
        CefRefPtr<CefFrame> frame = browser->GetMainFrame();
        frame->LoadURL("ascdesktop://crash.html");
    }

    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback)
    {
        CEF_REQUIRE_UI_THREAD();
        
        std::wstring s1 = download_item->GetURL().ToWString();
        std::wstring s2 = m_pParent->GetAppManager()->m_pInternal->GetPrivateDownloadUrl();
        
        if (s1 == s2)
        {
            callback->Continue(m_pParent->GetAppManager()->m_pInternal->GetPrivateDownloadPath(), false);
            return;
        }
        
        m_pParent->m_pInternal->m_before_callback = callback;
        callback->AddRef();

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG);
        NSEditorApi::CAscSaveDialog* pData = new NSEditorApi::CAscSaveDialog();
        pData->put_Id(m_pParent->GetId());
        pData->put_FilePath(suggested_name.ToWString());
        pData->put_IdDownload((unsigned int)download_item->GetId());
        pEvent->m_pData = pData;

        m_pParent->GetAppManager()->Apply(pEvent);
    }

    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefDownloadItem> download_item,
                                          CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        if (NULL == m_pParent)
            return;
        
        std::wstring sUrl = download_item->GetURL().ToWString();
        std::wstring sPath = download_item->GetFullPath().ToWString();
        
        unsigned int uId = (unsigned int)download_item->GetId();

        NSEditorApi::CAscDownloadFileInfo* pData = new NSEditorApi::CAscDownloadFileInfo();
        pData->put_Url(sUrl);
        pData->put_FilePath(sPath);
        pData->put_Percent(download_item->GetPercentComplete());
        pData->put_IsComplete(false);
        pData->put_Id(m_pParent->GetId());
        pData->put_Speed(download_item->GetCurrentSpeed() / 1024.0);
        pData->put_IdDownload(uId);

        long long l1 = download_item->GetReceivedBytes();
        long long l2 = download_item->GetTotalBytes();

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD;
        pEvent->m_pData = pData;

        if (m_pParent->GetAppManager()->m_pInternal->IsCanceled(uId))
        {
            pData->put_IsCanceled(true);
            pData->put_IsComplete(true);
            
            if (callback)
                callback->Cancel();
        }

        if (download_item->IsComplete())
        {
            //SetLastDownloadFile(download_item->GetFullPath());
            //SendNotification(NOTIFY_DOWNLOAD_COMPLETE);

            if (NULL != m_pParent)
            {
                pData->put_IsComplete(true);
            }
        }

        m_pParent->GetAppManager()->Apply(pEvent);
    }

    bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefDragData> dragData,
                     CefDragHandler::DragOperationsMask mask) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

#if 0
        bool bIsFile = dragData->IsFile();
        std::wstring strFileName = dragData->GetFileName();
        std::wstring strFile2 = dragData->GetLinkURL();
#endif
        std::vector<CefString> arFiles;
        dragData->GetFileNames(arFiles);

        if (1 == arFiles.size() && m_pParent)
        {
            std::wstring sFile = arFiles.begin()->ToWString();
            if (NSCommon::GetFileExtention(sFile) == L"plugin")
            {
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
                NSEditorApi::CAscAddPlugin* pData = new NSEditorApi::CAscAddPlugin();
                pData->put_Path(sFile);
                pEvent->m_pData = pData;
                m_pParent->GetAppManager()->Apply(pEvent);
                return true;
            }
        }

        if (0 != arFiles.size())
        {
            std::string sCode = "[";
            for (std::vector<CefString>::iterator i = arFiles.begin(); i != arFiles.end(); i++)
            {
                std::string sFile = (*i).ToString();
                NSCommon::string_replaceA(sFile, "\\", "/");
                sCode += "\"";
                sCode += sFile;
                sCode += "\",";
            }
            ((char*)sCode.c_str())[sCode.length() - 1] = ']';

            std::vector<int64> arFramesIds;
            browser->GetFrameIdentifiers(arFramesIds);

            for (std::vector<int64>::iterator i = arFramesIds.begin(); i != arFramesIds.end(); i++)
            {
                CefRefPtr<CefFrame> _frame = browser->GetFrame(*i);
                if (_frame)
                    _frame->ExecuteJavaScript("window.AscDesktopEditor.SetDropFiles(" + sCode + ");", _frame->GetURL(), 0);
            }
        }

        // разруливаем на стороне js
        return false;
    }

    virtual void OnFoundCookie(bool bIsPresent, std::string sValue);
    virtual void OnSetCookie();
    virtual void OnDeleteCookie(bool bIsPresent);
    virtual void OnFoundCookies(std::map<std::string, std::string>& mapValues);

    // ClientHandlerDelegate
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {}

    // Called when the browser is closing.
    virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        if (m_pParent && m_pParent->m_pInternal)
            m_pParent->m_pInternal->m_bIsClosing = true;
    }

    // Called when the browser has been closed.
    virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        m_pParent->m_pInternal->Destroy();
        m_pParent->m_pInternal->m_pManager->OnDestroyWindow();
    }

    // Set the window URL address.
    virtual void OnSetAddress(const std::string& url) OVERRIDE {}

    // Set the window title.
    virtual void OnSetTitle(const std::string& title) OVERRIDE {}

    // Set fullscreen mode.
    virtual void OnSetFullscreen(bool fullscreen) OVERRIDE {}

    // Set the loading state.
    virtual void OnSetLoadingState(bool isLoading,
                                     bool canGoBack,
                                     bool canGoForward) OVERRIDE {}

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions) OVERRIDE {}

public:
    IMPLEMENT_REFCOUNTING(CAscClientHandler);
};

void CCefView_Private::CloseBrowser(bool _force_close)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        CefPostTask(TID_UI,
            base::Bind(&CCefView_Private::CloseBrowser, this, _force_close));
        return;
    }

    if (m_handler && m_handler->GetBrowser() && m_handler->GetBrowser()->GetHost())
        m_handler->GetBrowser()->GetHost()->CloseBrowser(_force_close);
}
void CCefView_Private::LocalFile_End()
{
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_loadend");
    message->GetArgumentList()->SetString(0, m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    if (!m_oConverterToEditor.m_sName.empty())
    {
        m_oLocalInfo.m_oInfo.m_sFileSrc = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/" + m_oConverterToEditor.m_sName;
    }

    message->GetArgumentList()->SetString(1, m_oLocalInfo.m_oInfo.m_sFileSrc);
    message->GetArgumentList()->SetBool(2, m_oLocalInfo.m_oInfo.m_bIsSaved);
    message->GetArgumentList()->SetString(3, m_oConverterToEditor.GetSignaturesJSON());
    m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);

    m_oLocalInfo.m_oInfo.m_nCounterConvertion = 1; // for reload enable
}
void CCefView_Private::LocalFile_SaveEnd(int nError, const std::wstring& sPass)
{
    m_oConverterFromEditor.m_nTypeEditorFormat = -1;
    m_oLocalInfo.m_bIsRetina = false;

    if (!LocalFile_IsSupportEditFormat(m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat))
    {
        if (0 == nError)
            m_pManager->m_pInternal->Recents_Add(m_oConverterFromEditor.m_oInfo.m_sFileSrc, m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat);
        nError = ASC_CONSTANT_CANCEL_SAVE;
    }

    if (ASC_CONSTANT_CANCEL_SAVE == nError)
    {
        if (m_pManager->GetEventListener() && m_pCefView != NULL)
        {
            m_bIsNativeSave = false;

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);

            NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
            pData->put_Id(m_pCefView->GetId());
            pData->put_IsCancel(true);

            pEvent->m_pData = pData;

            m_pManager->GetEventListener()->OnEvent(pEvent);
        }

        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onsaveend");
        message->GetArgumentList()->SetString(0, "");
        message->GetArgumentList()->SetInt(1, 1);
        m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
        return;
    }

    bool bIsSaved = ((0 == nError) && NSFile::CFileBinary::Exists(m_oConverterFromEditor.m_oInfo.m_sFileSrc)) ? true : false;

    if (bIsSaved && !m_oLocalInfo.m_oInfo.m_bIsSaved)
    {
        m_oLocalInfo.m_oInfo.m_bIsSaved = true;
    }

    if (bIsSaved)
    {
        m_oLocalInfo.m_oInfo.m_sFileSrc = m_oConverterFromEditor.m_oInfo.m_sFileSrc;
        m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat;

        std::wstring sNameInfo = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/asc_name.info";
        if (NSFile::CFileBinary::Exists(sNameInfo))
            NSFile::CFileBinary::Remove(sNameInfo);

        NSStringUtils::CStringBuilder oBuilderInfo;
        oBuilderInfo.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><info type=\"" + std::to_wstring(m_oLocalInfo.m_oInfo.m_nCurrentFileFormat) + L"\" name=\"");
        oBuilderInfo.WriteEncodeXmlString(NSCommon::GetFileName(m_oLocalInfo.m_oInfo.m_sFileSrc));
        oBuilderInfo.WriteString(L"\" />");
        NSFile::CFileBinary::SaveToFile(sNameInfo, oBuilderInfo.GetData(), true);
    }

    if (bIsSaved && m_pManager && m_pManager->m_pInternal)
    {
        m_pManager->m_pInternal->Recents_Add(m_oLocalInfo.m_oInfo.m_sFileSrc, m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);

        if (m_pManager->GetEventListener() && m_pCefView != NULL)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME);

            NSEditorApi::CAscDocumentName* pData = new NSEditorApi::CAscDocumentName();
            pData->put_Id(m_pCefView->GetId());
            pData->put_Name(NSCommon::GetFileName(m_oLocalInfo.m_oInfo.m_sFileSrc));
            if (m_oLocalInfo.m_oInfo.m_bIsSaved)
            {
                std::wstring sPath = m_oLocalInfo.m_oInfo.m_sFileSrc;
#ifdef WIN32
                NSCommon::string_replace(sPath, L"/", L"\\");
#endif
                pData->put_Path(sPath);
            }

            pEvent->m_pData = pData;

            m_pManager->GetEventListener()->OnEvent(pEvent);
        }

        if (m_pManager->GetEventListener() && m_pCefView != NULL)
        {
            m_bIsNativeSave = false;

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);

            NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
            pData->put_Id(m_pCefView->GetId());

            pEvent->m_pData = pData;

            m_pManager->GetEventListener()->OnEvent(pEvent);
        }
    }

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onsaveend");
    message->GetArgumentList()->SetString(0, (0 == nError) ? m_oLocalInfo.m_oInfo.m_sFileSrc : L"");
    message->GetArgumentList()->SetInt(1, (0 == nError) ? 0 : 2);

    if (!sPass.empty() && (0 == nError))
    {
        message->GetArgumentList()->SetString(2, sPass);

        ICertificate* pCert = ICertificate::CreateInstance();
        std::string sHash = pCert->GetHash(m_oLocalInfo.m_oInfo.m_sFileSrc, OOXML_HASH_ALG_SHA1);
        delete pCert;

        message->GetArgumentList()->SetString(3, sHash);
    }
    m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

void CCefView_Private::LocalFile_IncrementCounter()
{
    CTemporaryCS oCS(&m_oLocalInfo.m_oCS);
    ++m_oLocalInfo.m_oInfo.m_nCounterConvertion;

    if (2 == m_oLocalInfo.m_oInfo.m_nCounterConvertion)
    {
        if (89 == m_nLocalFileOpenError ||
            90 == m_nLocalFileOpenError ||
            91 == m_nLocalFileOpenError)
        {
            --m_oLocalInfo.m_oInfo.m_nCounterConvertion;

            CefRefPtr<CefBrowser> browser;
            if (m_handler.get())
            {
                browser = m_handler->GetBrowser();
            }
            if (browser)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_additionalparams");
                message->GetArgumentList()->SetInt(0, m_nLocalFileOpenError);

                if (90 == m_nLocalFileOpenError || 91 == m_nLocalFileOpenError)
                {
                    ICertificate* pCert = ICertificate::CreateInstance();
                    std::string sHash = pCert->GetHash(m_oLocalInfo.m_oInfo.m_sFileSrc, OOXML_HASH_ALG_SHA1);
                    delete pCert;

                    message->GetArgumentList()->SetString(1, sHash);
                }

                browser->SendProcessMessage(PID_RENDERER, message);
            }
            return;
        }
        else
        {
            LocalFile_End();
        }
    }
}
void CCefView_Private::OnFileConvertToEditor(const int& nError)
{
    m_nLocalFileOpenError = nError;
    LocalFile_IncrementCounter();
}
void CCefView_Private::CheckZoom()
{
#ifdef WIN32
    if (!m_bIsWindowsCheckZoom)
        return;

    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::Bind(&CCefView_Private::CheckZoom, this));
        return;
    }

    if (!m_handler || !m_handler->GetBrowser() || !m_handler->GetBrowser()->GetHost())
        return;

    if (!m_pWidgetImpl || m_bIsClosing)
        return;

    CefRefPtr<CefBrowserHost> _host = m_handler->GetBrowser()->GetHost();
    CefWindowHandle hwnd = _host->GetWindowHandle();

    int nDeviceScale = NSMonitor::GetRawMonitorDpi(hwnd);

    if (-1 != m_pManager->m_pInternal->m_nForceDisplayScale && m_pManager->m_pInternal->m_nForceDisplayScale > 0)
        nDeviceScale = m_pManager->m_pInternal->m_nForceDisplayScale;

    if (nDeviceScale != m_nDeviceScale)
    {
        m_nDeviceScale = nDeviceScale;

        if (m_nDeviceScale >= 1)
        {
            double dScale = (m_nDeviceScale == 2) ? (log(2) / log(1.2)) : 0;

            _host->SetZoomLevel(dScale);
            _host->NotifyScreenInfoChanged();
            _host->WasResized();
        }
    }
#endif
}

void CCefView_Private::SendProcessMessage(CefProcessId target_process, CefRefPtr<CefProcessMessage> message)
{
    if (m_handler && m_handler->GetBrowser())
    {
        m_handler->GetBrowser()->SendProcessMessage(target_process, message);
    }
}

void CAscClientHandler::OnFoundCookie(bool bIsPresent, std::string sValue)
{
    if (!m_pParent || !m_pParent->m_pInternal || !m_pParent->m_pInternal->m_handler || !m_pParent->m_pInternal->m_handler->GetBrowser())
        return;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_is_cookie_present");
    message->GetArgumentList()->SetBool(0, bIsPresent);
    message->GetArgumentList()->SetString(1, sValue);
    m_pParent->m_pInternal->m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}
void CAscClientHandler::OnFoundCookies(std::map<std::string, std::string>& mapValues)
{
    if (!m_pParent || !m_pParent->m_pInternal || !m_pParent->m_pInternal->m_handler || !m_pParent->m_pInternal->m_handler->GetBrowser())
        return;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_check_auth");
    message->GetArgumentList()->SetInt(0, (int)mapValues.size());

    int nCurrent = 1;
    for (std::map<std::string, std::string>::iterator i = mapValues.begin(); i != mapValues.end(); i++)
    {
        message->GetArgumentList()->SetString(nCurrent++, i->first);
        message->GetArgumentList()->SetString(nCurrent++, i->second);
    }

    m_pParent->m_pInternal->m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

void CAscClientHandler::OnSetCookie()
{
    if (!m_pParent || !m_pParent->m_pInternal || !m_pParent->m_pInternal->m_handler || !m_pParent->m_pInternal->m_handler->GetBrowser())
        return;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_set_cookie");
    m_pParent->m_pInternal->m_handler->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

void CAscClientHandler::OnDeleteCookie(bool bIsPresent)
{
    // not used
}

CCefView::CCefView(CCefViewWidgetImpl* parent, int nId)
{
    //setAttribute(Qt::WA_NativeWindow);
    //setAttribute(Qt::WA_DontCreateNativeAncestors);
    //RegisterDragDrop((HWND)this->winId(), this);

    //setAcceptDrops(true);
    m_pInternal = new CCefView_Private();
    m_pInternal->m_pWidgetImpl = parent;
    m_nId = nId;
    m_eWrapperType = cvwtSimple;
    m_pInternal->m_pCefView = this;
}

CCefView::~CCefView()
{
    RELEASEOBJECT(m_pInternal);
}

void CCefView::load(const std::wstring& urlInput)
{
    std::wstring url = urlInput;
    std::wstring::size_type posQ = url.find_first_of('?');
    if (std::wstring::npos != posQ)
    {
        if (std::wstring::npos == url.find(L"desktop=true", posQ))
            url = (url + L"&desktop=true");
    }
    else
    {
        url = (url + L"?desktop=true");
    }

    if (!m_pInternal || !m_pInternal->m_pManager)
        return;

    m_pInternal->m_bIsSSO = false;
    if (0 == url.find(L"sso:"))
    {
        url = url.substr(4);
        m_pInternal->m_bIsSSO = true;
    }

    m_pInternal->m_oConverterToEditor.m_pManager = this->GetAppManager();
    m_pInternal->m_oConverterToEditor.m_pView = this;
    m_pInternal->m_oConverterFromEditor.m_pManager = this->GetAppManager();

    m_pInternal->m_strUrl = url;
    NSCommon::url_correct(m_pInternal->m_strUrl);
    m_pInternal->m_oPrintData.m_pApplicationFonts = GetAppManager()->GetApplicationFonts();

    if (m_pInternal->m_handler)
    {
        CefRefPtr<CefFrame> _frame;
        if (m_pInternal->m_handler->GetBrowser())
            _frame = m_pInternal->m_handler->GetBrowser()->GetMainFrame();

        if (_frame)
        {
            CefString sUrl;
            sUrl.FromWString(url);
            _frame->LoadURL(sUrl);
        }
        return;
    }

    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(m_pInternal->m_pManager->m_oSettings.cookie_path, true, NULL);

    //m_pInternal->SetCookie(manager);

#if 0
    CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
    pVisitor->m_sDomain         = "";
    pVisitor->m_sCookieSearch   = "";
    pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager());
#endif

    // Create the single static handler class instance
    CAscClientHandler* pClientHandler = new CAscClientHandler();
    pClientHandler->m_pParent = this;
    m_pInternal->m_handler = pClientHandler;

    CAscClientHandler::CAscCefJSDialogHandler* pAscCEfJSDialogHandler = static_cast<CAscClientHandler::CAscCefJSDialogHandler*>(pClientHandler->m_pCefJSDialogHandler.get());
    if (pAscCEfJSDialogHandler)
        pAscCEfJSDialogHandler->SetAppManager(m_pInternal->m_pManager);

    CefWindowHandle hWnd = (CefWindowHandle)m_pInternal->m_pWidgetImpl->parent_wid();
    //m_pInternal->m_handler->SetMainWindowHandle(hWnd);

    CefWindowInfo info;
    CefBrowserSettings _settings;
    _settings.file_access_from_file_urls = STATE_ENABLED;
    _settings.universal_access_from_file_urls = STATE_ENABLED;
    _settings.javascript_access_clipboard = STATE_ENABLED;

    _settings.plugins = STATE_DISABLED;

    if (m_eWrapperType == cvwtEditor)
    {
        _settings.background_color = 0xFFF4F4F4;
    }
    else
    {
        _settings.background_color = 0xFFFFFFFF;
    }

    // Initialize window info to the defaults for a child window.

#ifdef WIN32
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = m_pInternal->m_pWidgetImpl->parent_width() - 1;
    rect.bottom = m_pInternal->m_pWidgetImpl->parent_height() - 1;
    info.SetAsChild(hWnd, rect);
#endif

#if defined(_LINUX) && !defined(_MAC)
    CefRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = m_pInternal->m_pWidgetImpl->parent_width();
    rect.height = m_pInternal->m_pWidgetImpl->parent_height();

    Display* display = cef_get_xdisplay();
    Window x11root = XDefaultRootWindow(display);
    Window x11w = XCreateSimpleWindow(display, x11root, 0, 0, rect.width, rect.height, 0, 0, _settings.background_color);

    XReparentWindow(display, x11w, hWnd, 0, 0);
    XMapWindow(display, x11w);
    m_pInternal->m_lNaturalParent = x11w;

    info.SetAsChild(m_pInternal->m_lNaturalParent, rect);
#endif

#ifdef _MAC
    info.SetAsChild(hWnd, 0, 0,
                    m_pInternal->m_pWidgetImpl->parent_width(),
                    m_pInternal->m_pWidgetImpl->parent_height());
#endif

#if 0
    std::string sLog = "load: " + std::to_string(m_pInternal->m_pWidgetImpl->parent_width()) +
            ", " + std::to_string(m_pInternal->m_pWidgetImpl->parent_height()) + "\n";
    FILE* fLog = fopen("D:\\resize.log", "a+");
    fprintf(fLog, sLog.c_str());
    fclose(fLog);
#endif

    CefString sUrl = url;

    // Creat the new child browser window
    CefBrowserHost::CreateBrowser(info, m_pInternal->m_handler.get(), sUrl, _settings, NULL);

    focus();
}

std::wstring CCefView::GetUrl()
{
    if (!m_pInternal)
        return L"";

    return m_pInternal->m_strUrl;
}

void CCefView::focus(bool value)
{
    if (!m_pInternal || !m_pInternal->m_pWidgetImpl || !m_pInternal->m_handler.get())
        return;

    CefRefPtr<CefBrowser> browser = m_pInternal->m_handler->GetBrowser();
    if (browser)
    {
        // Give focus to the browser.
        browser->GetHost()->SetFocus(value);

        if (!m_pInternal->m_bIsMouseHook)
        {
            if (this->GetType() == cvwtSimple)
            {
#ifdef WIN32
#if 0
                DWORD threadId = GetWindowThreadProcessId(browser->GetHost()->GetWindowHandle(), NULL);
                HHOOK hook = SetWindowsHookEx(WH_MOUSE, MyMouseHook, NULL, threadId);
#endif
#endif
            }

            m_pInternal->m_bIsMouseHook = true;
        }
    }
}

bool CCefView::nativeEvent(const char* data, const int& datalen, void *message, long *result)
{
#ifdef WIN32
    *result = 0;
    MSG *msg = static_cast<MSG *>(message);

    if (msg->message == WM_ERASEBKGND)
    {
        if (m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser())
        {
            // Don't erase the background.
            return true;
        }
    }
    else if (msg->message == WM_MOVE || msg->message == WM_MOVING)
    {
        if (m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser() && m_pInternal->m_handler->GetBrowser()->GetHost())
        {
            m_pInternal->m_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
        }
        return true;
    }
    else if (msg->message == (WM_USER + 1))
    {
#if 0
        if (this->GetType() == cvwtSimple)
        {
            m_pInternal->m_dwTimeMouseWheelUp = NSTimers::GetTickCount();
        }
#endif
        return true;
    }
#endif

    return false;
}

void CCefView::resizeEvent(int width, int height)
{
    if (!m_pInternal->m_handler || !m_pInternal->m_handler->GetBrowser() || !m_pInternal->m_handler->GetBrowser()->GetHost())
        return;
    
    if (!m_pInternal->m_pWidgetImpl || m_pInternal->m_bIsClosing)
        return;

    CefWindowHandle hwnd = m_pInternal->m_handler->GetBrowser()->GetHost()->GetWindowHandle();

#ifdef WIN32

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (0 >= width) ? (m_pInternal->m_pWidgetImpl->parent_width() - 1) : width;
    rect.bottom = (0 >= height) ? (m_pInternal->m_pWidgetImpl->parent_height() - 1) : height;

#if 0
    SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left + 1,
                 rect.bottom - rect.top + 1, SWP_NOZORDER);
#else
    HDWP hdwp = BeginDeferWindowPos(1);
    hdwp = DeferWindowPos(hdwp, hwnd, NULL,
        rect.left, rect.top, rect.right - rect.left + 1,
        rect.bottom - rect.top + 1, SWP_NOZORDER);
    EndDeferWindowPos(hdwp);
#endif
#endif

#if defined(_LINUX) && !defined(_MAC)
    ::Display* xdisplay = cef_get_xdisplay();

    XWindowChanges changes = {0};
    changes.width = (0 == width) ? (m_pInternal->m_pWidgetImpl->parent_width()) : width;
    changes.height = (0 == height) ? (m_pInternal->m_pWidgetImpl->parent_height()) : height;
    changes.y = 0;
    changes.y = 0;

    XConfigureWindow(xdisplay, m_pInternal->m_lNaturalParent, CWHeight | CWWidth | CWY, &changes);
    XConfigureWindow(xdisplay, hwnd, CWHeight | CWWidth | CWY, &changes);
#endif

#ifdef _MAC
    MAC_COMMON_set_window_handle_sizes(hwnd, 0, 0,
        (0 == width) ? (m_pInternal->m_pWidgetImpl->parent_width()) : width,
        (0 == height) ? (m_pInternal->m_pWidgetImpl->parent_height()) : height);
#endif

#if 0
    std::string sLog = "resize: " + std::to_string(m_pInternal->m_pWidgetImpl->parent_width()) +
            ", " + std::to_string(m_pInternal->m_pWidgetImpl->parent_height()) + "\n";
    FILE* fLog = fopen("D:\\resize.log", "a+");
    fprintf(fLog, sLog.c_str());
    fclose(fLog);
#endif
    focus();

    m_pInternal->CheckZoom();
}

void CCefView::moveEvent()
{
#if defined(_LINUX) && !defined(_MAC)
    if (m_pInternal && m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser() && m_pInternal->m_handler->GetBrowser()->GetHost())
    {
        m_pInternal->m_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
    }
#endif

#ifdef WIN32

    m_pInternal->CheckZoom();

#endif
}

void CCefView::Apply(NSEditorApi::CAscMenuEvent* pEvent)
{
    if (NULL == pEvent)
        return;

    if (!m_pInternal)
        return;

    CefRefPtr<CefBrowser> browser;
    if (m_pInternal->m_handler.get())
    {
        browser = m_pInternal->m_handler->GetBrowser();
    }
    if (!browser)
    {
        if (pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY ||
            pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE)
        {
            if (m_pInternal->m_pManager)
                m_pInternal->m_pManager->OnDestroyWindow();
        }

        CApplicationManagerAdditionalBase* pAdditional = GetAppManager()->m_pInternal->m_pAdditional;
        if (pAdditional)
            pAdditional->ApplyView(this, pEvent, NULL);

        RELEASEINTERFACE(pEvent);
        return;
    }

    switch (pEvent->m_nType)
    {
        case ASC_MENU_EVENT_TYPE_CEF_KEYBOARDLAYOUT:
        {
            NSEditorApi::CAscKeyboardLayout* pData = (NSEditorApi::CAscKeyboardLayout*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("keyboard_layout");
            message->GetArgumentList()->SetInt(0, pData->get_Language());
            browser->SendProcessMessage(PID_RENDERER, message);

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK:
        {
            NSEditorApi::CAscSpellCheckType* pData = (NSEditorApi::CAscSpellCheckType*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("spell_check_response");
            message->GetArgumentList()->SetString(0, pData->get_Result());
            message->GetArgumentList()->SetInt(1, pData->get_FrameId());
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_CONTROL_ID:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cef_control_id");
            message->GetArgumentList()->SetInt(0, m_nId);
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SYNC_COMMAND:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("sync_command_end");
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_CLOSE:
        {
            if (!this->GetModified())
            {
                // здесь потом делать запрос на то, сохранен ли файл. пока просто считаем что все сохранено
                NSEditorApi::CAscCefMenuEvent* pData = this->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONCLOSE);

                NSEditorApi::CAscTypeId* pDataId = new NSEditorApi::CAscTypeId();
                pDataId->put_Id(m_nId);
                pData->m_pData = pDataId;

                if (NULL != this->GetAppManager()->GetEventListener())
                    this->GetAppManager()->GetEventListener()->OnEvent(pData);
            }
            else
            {
                NSEditorApi::CAscCefMenuEvent* pData = this->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE);

                NSEditorApi::CAscTypeId* pDataId = new NSEditorApi::CAscTypeId();
                pDataId->put_Id(m_nId);
                pData->m_pData = pDataId;

                if (NULL != this->GetAppManager()->GetEventListener())
                    this->GetAppManager()->GetEventListener()->OnEvent(pData);
            }

            RELEASEINTERFACE(pEvent);
            return;

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_DESTROY:
        case ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE:
        {
            if (m_pInternal && pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE)
                m_pInternal->m_bIsRemoveRecoveryOnClose = true;
            
            m_pInternal->m_oConverterFromEditor.Stop();
            m_pInternal->m_oConverterToEditor.Stop();

            m_pInternal->m_pWidgetImpl = NULL;
            if (m_pInternal && m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser())
            {
                m_pInternal->CloseBrowser(true);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SAVE:
        {
            m_pInternal->m_bIsNativeSave = true;

            if (NULL != m_pInternal->m_pManager->m_pInternal->m_pAdditional)
                m_pInternal->m_pManager->m_pInternal->m_pAdditional->Local_Save_Start();

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("document_save");
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_PRINT_START:
        {
            if (m_pInternal)
            {
                if (this->GetType() == cvwtEditor)
                {
                    m_pInternal->m_oPrintData.m_eEditorType = ((CCefViewEditor*)this)->GetEditorType();
                }

                m_pInternal->m_oPrintData.Print_Start();
            }

            bool bIsNativePrint = false;
            if (!m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc.empty())
            {
                // локальные файлы
                switch (m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat)
                {
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU:
                {
                    m_pInternal->m_oPrintData.m_pNativePrinter = new CAscNativePrintDocument();
                    break;
                }
                default:
                    break;
                };

                if (NULL != m_pInternal->m_oPrintData.m_pNativePrinter)
                {                    
                    m_pInternal->m_oPrintData.m_pNativePrinter->m_pApplicationFonts = m_pInternal->m_oPrintData.m_pApplicationFonts;
                    m_pInternal->m_oPrintData.m_pNativePrinter->PreOpen(m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);

                    m_pInternal->m_oPrintData.m_pNativePrinter->Open(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc,
                                                                     m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);
                    m_pInternal->m_oPrintData.m_pNativePrinter->Check(m_pInternal->m_oPrintData.m_arPages);
                    bIsNativePrint = true;
                }
            }

            if (!bIsNativePrint)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print");
                browser->SendProcessMessage(PID_RENDERER, message);
            }
            else
            {
                // analog message "print_end"
                if (m_pInternal->m_pManager->GetEventListener())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = this->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END);

                    NSEditorApi::CAscPrintEnd* pData = new NSEditorApi::CAscPrintEnd();
                    pData->put_Id(GetId());
                    pData->put_PagesCount(m_pInternal->m_oPrintData.m_arPages.GetCount());
                    pData->put_CurrentPage(0);
                    pEvent->m_pData = pData;

                    m_pInternal->m_pManager->GetEventListener()->OnEvent(pEvent);
                }
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_PRINT_END:
        {
            if (m_pInternal)
            {
                m_pInternal->m_oPrintData.Print_End();
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE:
        {
            if (m_pInternal)
            {
                NSEditorApi::CAscPrintPage* pData = (NSEditorApi::CAscPrintPage*)pEvent->m_pData;
                m_pInternal->m_oPrintData.Print(pData->get_Context(), GetAppManager()->m_oPrintSettings, pData->get_Page());
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE_CHECK:
        {
            pEvent->AddRef();
            if (m_pInternal)
            {
                NSEditorApi::CAscPrintPage* pData = (NSEditorApi::CAscPrintPage*)pEvent->m_pData;
                bool bIsRotate = m_pInternal->m_oPrintData.CheckPrintRotate(pData->get_Context(), GetAppManager()->m_oPrintSettings, pData->get_Page());
                pData->put_IsRotate(bIsRotate);
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION:
        {
            NSEditorApi::CAscEditorScript* pData = (NSEditorApi::CAscEditorScript*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_load_js");
            message->GetArgumentList()->SetString(0, pData->get_Destination());
            message->GetArgumentList()->SetString(1, pData->get_Url());
            message->GetArgumentList()->SetInt(2, pData->get_FrameId());
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG:
        {
            NSEditorApi::CAscSaveDialog* pData = (NSEditorApi::CAscSaveDialog*)pEvent->m_pData;

            std::wstring strPath = pData->get_FilePath();
            if (strPath.empty())
            {
                GetAppManager()->CancelDownload(pData->get_IdDownload());
            }
            else
            {
                CefString sPath;
                sPath.FromWString(pData->get_FilePath());
                m_pInternal->m_before_callback->Continue(sPath, false);
            }

            m_pInternal->m_before_callback->Release();
            m_pInternal->m_before_callback = NULL;

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS:
        {
            NSEditorApi::CAscLocalRecentsAll* pData = (NSEditorApi::CAscLocalRecentsAll*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecents");
            message->GetArgumentList()->SetString(0, pData->get_JSON());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS:
        {
            NSEditorApi::CAscLocalRecentsAll* pData = (NSEditorApi::CAscLocalRecentsAll*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecovers");
            message->GetArgumentList()->SetString(0, pData->get_JSON());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH:
        {
            NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;
            std::wstring sPath = pData->get_Path();
            m_pInternal->m_bIsSavingDialog = false;

            if (sPath == L"")
            {
                m_pInternal->LocalFile_SaveEnd(ASC_CONSTANT_CANCEL_SAVE);
            }
            else
            {
                int nFileType = pData->get_FileType();

                m_pInternal->m_oConverterFromEditor.m_sOriginalFileNameCrossPlatform = L"";
                if (m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
                    m_pInternal->m_oConverterFromEditor.m_sOriginalFileNameCrossPlatform = m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;

                m_pInternal->LocalFile_SaveStart(sPath, nFileType);
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE:
        {
            NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onaddimage");
            message->GetArgumentList()->SetString(0, pData->get_Path());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS:
        {
            NSEditorApi::CAscExecCommandJS* pData = (NSEditorApi::CAscExecCommandJS*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_native_message");
            message->GetArgumentList()->SetString(0, pData->get_Command());
            message->GetArgumentList()->SetString(1, pData->get_Param());
            message->GetArgumentList()->SetString(2, pData->get_FrameName());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND:
        {
            NSEditorApi::CAscEditorExecuteCommand* pData = (NSEditorApi::CAscEditorExecuteCommand*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_editor_native_message");
            message->GetArgumentList()->SetString(0, pData->get_Command());
            message->GetArgumentList()->SetString(1, pData->get_Params());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }        
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("update_install_plugins");
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG:
        {
            NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_open_filename_dialog");
            message->GetArgumentList()->SetString(0, pData->get_Path());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO:
        {
            NSEditorApi::CAscEditorSaveQuestion* pData = (NSEditorApi::CAscEditorSaveQuestion*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_file_save_question");
            message->GetArgumentList()->SetBool(0, pData->get_Value());

            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM:
        {
            NSEditorApi::CAscReporterMessage* pData = (NSEditorApi::CAscReporterMessage*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("from_reporter_message");
            message->GetArgumentList()->SetString(0, pData->get_Message());
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
        {
            NSEditorApi::CAscReporterMessage* pData = (NSEditorApi::CAscReporterMessage*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("to_reporter_message");
            message->GetArgumentList()->SetString(0, pData->get_Message());
            browser->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        default:
        {
            CApplicationManagerAdditionalBase* pAdditional = GetAppManager()->m_pInternal->m_pAdditional;
            if (pAdditional)
                pAdditional->ApplyView(this, pEvent, browser);

            break;
        }
    }

    RELEASEINTERFACE(pEvent);
}

NSEditorApi::CAscMenuEvent* CCefView::ApplySync(NSEditorApi::CAscMenuEvent* pEvent)
{
    return NULL;
}

NSEditorApi::CAscCefMenuEvent* CCefView::CreateCefEvent(int nType)
{
    NSEditorApi::CAscCefMenuEvent* pEvent = new NSEditorApi::CAscCefMenuEvent(nType);
    pEvent->put_SenderId(m_nId);
    return pEvent;
}

bool CCefView::StartDownload(const std::wstring& sUrl)
{
    if (!m_pInternal || !m_pInternal->m_handler || !m_pInternal->m_handler->GetBrowser())
        return false;
    
    if (!m_pInternal->m_handler->GetBrowser()->GetHost())
        return false;
    
    m_pInternal->m_handler->GetBrowser()->GetHost()->StartDownload(sUrl);
    return true;
}

#if 0
void CCefView::dragEnterEvent(QDragEnterEvent *)
{
    OutputDebugStringA( "dragEnterEvent" );
}

void CCefView::dragMoveEvent(QDragMoveEvent *)
{
    OutputDebugStringA( "dragMoveEvent" );
}

void CCefView::dragLeaveEvent(QDragLeaveEvent *)
{
    OutputDebugStringA( "dragLeaveEvent" );
}

void CCefView::dropEvent(QDropEvent *)
{
    OutputDebugStringA( "dropEvent" );
}
#endif

CAscApplicationManager* CCefView::GetAppManager()
{
    return m_pInternal->m_pManager;
}

void CCefView::SetAppManager(CAscApplicationManager* pManager)
{
    if (!m_pInternal)
        return;
    m_pInternal->m_pManager = pManager;
    m_pInternal->m_oPrintData.m_pAdditional = pManager->m_pInternal->m_pAdditional;
}

CCefViewWidgetImpl* CCefView::GetWidgetImpl()
{
    return m_pInternal->m_pWidgetImpl;
}
void CCefView::OnDestroyWidgetImpl()
{
    m_pInternal->m_pWidgetImpl = NULL;
}

int CCefView::GetId()
{
    return m_nId;
}

CefViewWrapperType CCefView::GetType()
{
    return m_eWrapperType;
}

void CCefView::SetParentCef(int nId)
{
    m_pInternal->m_nParentId = nId;
}
int CCefView::GetParentCef()
{
    return m_pInternal->m_nParentId;
}

void CCefView::SetModified(bool bIsModified)
{
    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsModified = bIsModified;
}

bool CCefView::GetModified()
{
    return m_pInternal->m_oLocalInfo.m_oInfo.m_bIsModified;
}

bool CCefView::IsPresentationReporter()
{
    return m_pInternal->m_bIsReporter;
}

void CCefView::LoadReporter(int nParentId, std::wstring url)
{
    m_pInternal->m_bIsReporter = true;
    m_pInternal->m_nReporterParentId = nParentId;

    // url - parent url
    std::wstring urlReporter = url;
    std::wstring::size_type pos = url.rfind(L"/index.html");
    if (std::wstring::npos != pos)
    {
        urlReporter = url.substr(0, pos);
        urlReporter += L"/index.reporter.html";
    }

    this->load(urlReporter);
}

/////////////////////////////////////////////////////////////
CCefViewEditor::CCefViewEditor(CCefViewWidgetImpl* parent, int nId) : CCefView(parent, nId)
{
    m_eType = etUndefined;
    m_eWrapperType = cvwtEditor;
}

CCefViewEditor::~CCefViewEditor()
{
}

void CCefViewEditor::SetEditorType(AscEditorType eType)
{
    m_eType = eType;
}

AscEditorType CCefViewEditor::GetEditorType()
{
    return m_eType;
}

int CCefViewEditor::GetFileFormat(const std::wstring& sFilePath)
{
    COfficeFileFormatChecker oChecker;
    oChecker.isOfficeFile(sFilePath);

    if (AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT == oChecker.nFileType)
    {
        std::wstring sExt = NSCommon::GetFileExtention(sFilePath);
        if (sExt != L"txt" && sExt != L"xml")
            return 0;
    }
    else if (AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO == oChecker.nFileType)
    {
        std::wstring sExt = NSCommon::GetFileExtention(sFilePath);
        return COfficeFileFormatChecker::GetFormatByExtension(L"." + sExt);
    }

    return oChecker.nFileType;
}

void CCefViewEditor::OpenLocalFile(const std::wstring& sFilePath, const int& nFileFormat_)
{
    int nFileFormat = nFileFormat_;
    if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_DOCY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    else if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_PPTY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
    else if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_XLSY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;

    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = nFileFormat;

    std::wstring sParams = L"placement=desktop";
    if (nFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
    {
        sParams = L"placement=desktop&doctype=presentation";
    }
    else if (nFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
    {
        sParams = L"placement=desktop&doctype=spreadsheet";
    }
    else if (nFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
    {
        sParams = L"placement=desktop&mode=view";
    }
    
    if (!GetAppManager()->m_pInternal->GetEditorPermission() && sParams.find(L"mode=view") == std::wstring::npos)
        sParams += L"&mode=view";
    
    std::wstring sAdditionalParams = GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
    if (!sAdditionalParams.empty())
        sParams += (L"&" + sAdditionalParams);

    this->GetAppManager()->m_pInternal->Recents_Add(sFilePath, nFileFormat);

    std::wstring sUrl = this->GetAppManager()->m_oSettings.local_editors_path;
    if (0 == sUrl.find('/'))
        sUrl = L"file://" + sUrl;
    else
        sUrl = L"file:///" + sUrl;

    if (!sParams.empty())
        sUrl += L"?";

    m_pInternal->m_oConverterToEditor.m_pManager = this->GetAppManager();
    m_pInternal->m_oConverterToEditor.m_pView = this;
    m_pInternal->m_oConverterFromEditor.m_pManager = this->GetAppManager();

    m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = sFilePath;
    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pInternal->m_pManager->m_oSettings.recover_path, L"DE_");
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    std::wstring::size_type nPosPoint = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.rfind('.');
    if (nPosPoint != std::wstring::npos && nPosPoint > m_pInternal->m_pManager->m_oSettings.recover_path.length())
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.substr(0, nPosPoint);
    }

    NSDirectory::CreateDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    m_pInternal->LocalFile_Start();

    // start convert file
    this->load(sUrl + sParams);
}
void CCefViewEditor::CreateLocalFile(const int& nFileFormat, const std::wstring& sName)
{
    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    std::string sCountry = this->GetAppManager()->m_oSettings.country;
    NSCommon::makeUpper(sCountry);
    std::wstring sPrefix = L"mm_";
    if ("US" == sCountry ||
        "CA" == sCountry)
    {
        sPrefix = L"in_";
    }

    std::wstring sFilePath = this->GetAppManager()->m_oSettings.file_converter_path + L"/empty/" + sPrefix + L"new.";
    std::wstring sExtension = L"docx";

    std::wstring sParams = L"placement=desktop";
    int nType = nFileFormat;
    if (nFileFormat == etPresentation)
    {
        sParams = L"placement=desktop&doctype=presentation";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
        sFilePath += L"pptx";
        sExtension = L"pptx";
    }
    else if (nFileFormat == etSpreadsheet)
    {
        sParams = L"placement=desktop&doctype=spreadsheet";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
        sFilePath += L"xlsx";
        sExtension = L"xlsx";
    }
    else
        sFilePath += L"docx";

    if (!NSFile::CFileBinary::Exists(sFilePath))
    {
        std::wstring sTestName = this->GetAppManager()->m_oSettings.file_converter_path + L"/empty/" + L"new." + sExtension;
        if (NSFile::CFileBinary::Exists(sTestName))
            sFilePath = sTestName;
    }
    
    if (!GetAppManager()->m_pInternal->GetEditorPermission())
        sParams += L"&mode=view";
    
    std::wstring sAdditionalParams = GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
    if (!sAdditionalParams.empty())
        sParams += (L"&" + sAdditionalParams);

    std::wstring sUrl = this->GetAppManager()->m_oSettings.local_editors_path;
    if (0 == sUrl.find('/'))
        sUrl = L"file://" + sUrl;
    else
        sUrl = L"file:///" + sUrl;

    if (!sParams.empty())
        sUrl += L"?";

    m_pInternal->m_oConverterToEditor.m_pManager = this->GetAppManager();
    m_pInternal->m_oConverterToEditor.m_pView = this;
    m_pInternal->m_oConverterFromEditor.m_pManager = this->GetAppManager();
    m_pInternal->m_oConverterToEditor.m_sName = sName;

    m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = sFilePath;
    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pInternal->m_pManager->m_oSettings.recover_path, L"DE_");
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    std::wstring::size_type nPosPoint = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.rfind('.');
    if (nPosPoint != std::wstring::npos && nPosPoint > m_pInternal->m_pManager->m_oSettings.recover_path.length())
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.substr(0, nPosPoint);
    }

    NSDirectory::CreateDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    m_pInternal->LocalFile_Start();

    // start convert file
    this->load(sUrl + sParams);
}

bool CCefViewEditor::OpenRecoverFile(const int& nId)
{
    if (!GetAppManager()->m_pInternal->GetEditorPermission())
        return false;
    
    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    std::wstring sUrl = this->GetAppManager()->m_oSettings.local_editors_path;
    if (0 == sUrl.find('/'))
        sUrl = L"file://" + sUrl;
    else
        sUrl = L"file:///" + sUrl;

    sUrl += L"?";

    CAscEditorFileInfo oInfo;
    for (std::vector<CAscEditorFileInfo>::iterator i = GetAppManager()->m_pInternal->m_arRecovers.begin();
         i != GetAppManager()->m_pInternal->m_arRecovers.end(); i++)
    {
        if (i->m_nId == nId)
        {
            oInfo = *i;
            break;
        }
    }

    if (-1 == oInfo.m_nId)
        return false;

    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    
    std::wstring sParams = L"";
    if (oInfo.m_nFileType & AVS_OFFICESTUDIO_FILE_PRESENTATION)
    {
        sParams = L"placement=desktop&doctype=presentation";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
    }
    else if (oInfo.m_nFileType & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
    {
        sParams = L"placement=desktop&doctype=spreadsheet";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
    }
    else if (oInfo.m_nFileType & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM || oInfo.m_bIsViewer)
    {
        sParams = L"placement=desktop&mode=view";
    }
    else
    {
        sParams = L"placement=desktop";
    }
    std::wstring sAdditionalParams = GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
    if (!sAdditionalParams.empty())
        sParams += (L"&" + sAdditionalParams);

    m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = oInfo.m_sPath;
    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSCommon::GetDirectoryName(oInfo.m_sPath);
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    m_pInternal->LocalFile_Start();

    // start convert file
    this->load(sUrl + sParams);

    this->GetAppManager()->m_pInternal->Recovers_Remove(nId);
    return true;
}

bool CCefViewEditor::OpenReporter(const std::wstring& sFolderInput)
{
    std::wstring sFolder = sFolderInput;
    if (0 == sFolder.find(L"file:///"))
    {
        sFolder = sFolder.substr(7);
        if (!NSDirectory::Exists(sFolder))
            sFolder = sFolder.substr(1);
    }

    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;

    m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = sFolder + L"/Editor.bin";
    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = sFolder;
    NSCommon::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    m_pInternal->LocalFile_Start();
    return true;
}

bool CCefViewEditor::OpenRecentFile(const int& nId)
{
    CAscEditorFileInfo oInfo;
    for (std::vector<CAscEditorFileInfo>::iterator i = GetAppManager()->m_pInternal->m_arRecents.begin();
         i != GetAppManager()->m_pInternal->m_arRecents.end(); i++)
    {
        if (i->m_nId == nId)
        {
            oInfo = *i;
            break;
        }
    }

    if (-1 == oInfo.m_nId)
        return false;

    if (oInfo.m_sUrl.empty())
    {
        int nFormat = GetFileFormat(oInfo.m_sPath);
        if (nFormat == 0)
            return false;

        this->OpenLocalFile(oInfo.m_sPath, nFormat);
        return true;
    }

    this->load(oInfo.m_sUrl);
    return true;
}

// NATIVE file converter
void CASCFileConverterToEditor::NativeViewerOpen(bool bIsEnabled)
{
    if (bIsEnabled)
    {
        m_pView->m_pInternal->m_oNativeViewer.m_bEnabled = true;
        m_bIsNativeOpening = true;
        m_pView->m_pInternal->m_oNativeViewer.m_nCounter++;
        m_pView->m_pInternal->m_oNativeViewer.m_sFilePath = m_pView->m_pInternal->m_oConverterToEditor.m_sNativeSrcPath;

        if (m_pView->m_pInternal->m_oNativeViewer.m_nCounter < 2)
        {
            while (m_bIsNativeOpening)
                NSThreads::Sleep(100);
        }
        else
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("native_viewer_onopened");
            message->GetArgumentList()->SetString(0, m_pView->m_pInternal->m_oNativeViewer.m_sFilePath);
            message->GetArgumentList()->SetString(1, m_pView->GetAppManager()->m_oSettings.fonts_cache_info_path);
            message->GetArgumentList()->SetString(2, m_pView->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

            this->m_pView->m_pInternal->SendProcessMessage(PID_RENDERER, message);

            while (m_bIsNativeOpening)
                NSThreads::Sleep(100);
        }
        return;
    }

    m_pView->m_pInternal->m_oNativeViewer.m_nCounter++;
    if (m_pView->m_pInternal->m_oNativeViewer.m_bEnabled)
    {
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("native_viewer_onopened");
        message->GetArgumentList()->SetString(0, m_pView->m_pInternal->m_oNativeViewer.m_sFilePath);
        message->GetArgumentList()->SetString(1, m_pView->GetAppManager()->m_oSettings.fonts_cache_info_path);
        message->GetArgumentList()->SetString(2, m_pView->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

        this->m_pView->m_pInternal->SendProcessMessage(PID_RENDERER, message);
    }
}

void CASCFileConverterToEditor::NativeViewerOpenEnd(const std::string& sBase64)
{
    std::wstring sFileDst = m_oInfo.m_sRecoveryDir + L"/Editor.bin";
    NSFile::CFileBinary oFile;
    oFile.CreateFileW(sFileDst);
    oFile.WriteFile((BYTE*)sBase64.c_str(), (DWORD)sBase64.length());
    oFile.CloseFile();

    m_bIsNativeOpening = false;
}
