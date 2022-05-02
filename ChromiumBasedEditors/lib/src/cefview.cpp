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

#include "include/cef_browser.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/cef_parser.h"

#ifdef CEF_2623
#include "cefclient/browser/client_handler.h"
#include "cefclient/common/client_switches.h"
#include "cefclient/renderer/client_renderer.h"
#else
#include "tests/cefclient/browser/client_handler.h"
#include "tests/shared/common/client_switches.h"
#include "tests/cefclient/renderer/client_renderer.h"
#endif

#include "include/cef_menu_model.h"

#include "include/wrapper/cef_stream_resource_handler.h"
#include "include/wrapper/cef_byte_read_handler.h"

#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "./fileconverter.h"
#include "./fileprinter.h"

#include "../../../../core/PdfReader/PdfReader.h"
#include "../../../../core/DjVuFile/DjVu.h"
#include "../../../../core/XpsFile/XpsFile.h"

#include "cefwrapper/monitor_info.h"

#include "./plugins.h"

#include "../../../../core/DesktopEditor/graphics/BaseThread.h"
#include "../../../../core/DesktopEditor/graphics/Matrix.h"

#include "./cefwrapper/client_scheme.h"
#include "./mailto.h"

#include <boost/regex.hpp>

#if defined (_LINUX) && !defined(_MAC)
#define DONT_USE_NATIVE_FILE_DIALOGS
#endif

#ifdef _WIN32
void CCefViewWidgetImpl::SetParentNull(WindowHandleId handle)
{
    SetParent(handle, NULL);
}
#endif
#if defined(_LINUX) && !defined(_MAC)
void CCefViewWidgetImpl::SetParentNull(WindowHandleId handle)
{
}
#endif

std::wstring GetUrlWithoutProtocol(const std::wstring& url)
{
    if (0 == url.find(L"http://"))
        return url.substr(7);
    if (0 == url.find(L"https://"))
        return url.substr(8);
    return url;
}

#define CRYPTO_CLOUD_SUPPORT 5020300

#include <math.h>

#ifdef CEF_2623
typedef CefStreamResourceHandler CefStreamResourceHandlerCORS;
#else
#define USE_STREAM_RESOURCE_RECOVER_CORS
class CefStreamResourceHandlerCORS : public CefStreamResourceHandler
{
public:
    CefStreamResourceHandlerCORS(const CefString& mime_type,
                             CefRefPtr<CefStreamReader> stream) : CefStreamResourceHandler(mime_type, stream)
    {
    }

    virtual void GetResponseHeaders(
        CefRefPtr<CefResponse> response,
        int64& response_length,
        CefString& redirectUrl) OVERRIDE
    {
      CefStreamResourceHandler::GetResponseHeaders(response, response_length, redirectUrl);

      CefResponse::HeaderMap headers;
      response->GetHeaderMap(headers);
      headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
      response->SetHeaderMap(headers);
    }
};
#endif

// иногда (при неправильной ссылке) - не приходят сообщения OnDownloadUpdate
// поэтому мы запускаем таймер. и если за его время не было сообщений - то считаем,
// что при скачке произошла ошибка
class CDownloadFilesAborted : public NSTimers::CTimer
{
protected:
    class CefDownloadItemAborted : public CefDownloadItem
    {
    private:
        std::wstring m_sUrl;

    public:
        CefDownloadItemAborted(const std::wstring& sUrl) : CefDownloadItem()
        {
            m_sUrl = sUrl;
            this->AddRef();
        }

    public:
        virtual bool IsValid() { return true; }
        virtual bool IsInProgress() { return false; }
        virtual bool IsComplete() { return false; }
        virtual bool IsCanceled() { return true; }
        virtual int64 GetCurrentSpeed() { return 0; }
        virtual int GetPercentComplete() { return 0; }
        virtual int64 GetTotalBytes() { return 0; }
        virtual int64 GetReceivedBytes() { return 0; }
        virtual CefTime GetStartTime() { return CefTime(); }
        virtual CefTime GetEndTime() { return CefTime(); }
        virtual CefString GetFullPath() { return ""; }
        virtual uint32 GetId() { return 0; }
        virtual CefString GetURL() { return m_sUrl; }
        virtual CefString GetOriginalUrl() { return m_sUrl; }
        virtual CefString GetSuggestedFileName() { return ""; }
        virtual CefString GetContentDisposition() { return ""; }
        virtual CefString GetMimeType() { return "download_aborted_timer"; }

        IMPLEMENT_REFCOUNTING(CefDownloadItemAborted);
    };
    class CUrlInfo
    {
    public:
        std::wstring Url;
        DWORD Time;
    public:
        CUrlInfo(const std::wstring& _url = L"", const DWORD& _time = 0)
        {
            Url = _url;
            Time = _time;
        }
        CUrlInfo(const CUrlInfo& src)
        {
            Url = src.Url;
            Time = src.Time;
        }
        CUrlInfo& operator=(const CUrlInfo& src)
        {
            Url = src.Url;
            Time = src.Time;
            return *this;
        }
    };

public:
    CDownloadFilesAborted() : NSTimers::CTimer()
    {
        m_oCS.InitializeCriticalSection();
        m_bIsFromTimer = false;
        m_dwIntervalCheck = 5000;
        m_bIsSkipNextIteration = false;
        SetInterval(m_dwIntervalCheck);
    }
    virtual ~CDownloadFilesAborted()
    {
        m_oCS.DeleteCriticalSection();
    }

public:
    NSCriticalSection::CRITICAL_SECTION m_oCS;
    CefDownloadHandler* m_pHandler;

    bool m_bIsFromTimer;
    DWORD m_dwIntervalCheck;
    bool m_bIsSkipNextIteration;

    std::map<std::wstring, CUrlInfo> m_mapUrls;

public:
    void StartDownload(const std::wstring& sUrl)
    {
        CTemporaryCS oCS(&m_oCS);
        m_mapUrls.insert(std::pair<std::wstring, CUrlInfo>(GetUrlWithoutProtocol(sUrl), CUrlInfo(sUrl, NSTimers::GetTickCount())));
        if (!IsRunned())
        {
            Start(0);
            return;
        }
        else
        {
            m_bIsSkipNextIteration = true;
        }
    }
    void EndDownload(const std::wstring& sUrl)
    {
        if (m_bIsFromTimer)
            return;

        CTemporaryCS oCS(&m_oCS);

        std::map<std::wstring, CUrlInfo>::iterator find = m_mapUrls.find(GetUrlWithoutProtocol(sUrl));
        if (find != m_mapUrls.end())
            m_mapUrls.erase(find);

        if (m_mapUrls.empty() && IsRunned())
            Stop();
    }

    virtual void OnTimer()
    {
        if (!IsRunned())
            return;

        CTemporaryCS oCS(&m_oCS);

        if (m_bIsSkipNextIteration)
        {
            m_bIsSkipNextIteration = false;
            return;
        }

        std::vector<std::wstring> arRemoved;
        for (std::map<std::wstring, CUrlInfo>::iterator iter = m_mapUrls.begin(); iter != m_mapUrls.end(); iter++)
        {
            if (NSTimers::GetTickCount() > (iter->second.Time + m_dwIntervalCheck))
                arRemoved.push_back(iter->second.Url);
        }

        for (std::vector<std::wstring>::iterator iter = arRemoved.begin(); iter != arRemoved.end(); iter++)
        {
            m_mapUrls.erase(*iter);
            if (m_pHandler)
            {
                m_bIsFromTimer = true;
                CefDownloadItemAborted* item = new CefDownloadItemAborted(*iter);
                m_pHandler->OnDownloadUpdated(NULL, item, NULL);
                item->Release();
                m_bIsFromTimer = false;
            }
        }

        if (m_mapUrls.empty())
        {
            StopNoJoin();
        }
    }
};

class CDownloadFileItem
{
public:
    std::wstring Url;
    std::wstring Path;

public:
    CDownloadFileItem(const std::wstring& _url = L"", const std::wstring& _path = L"")
    {
        Url = _url;
        Path = _path;
    }
    CDownloadFileItem(const CDownloadFileItem& src)
    {
        Url = src.Url;
        Path = src.Path;
    }
    CDownloadFileItem& operator=(const CDownloadFileItem& src)
    {
        Url = src.Url;
        Path = src.Path;
        return *this;
    }
};

class CAscNativePrintDocument : public IAscNativePrintDocument
{
protected:
    IOfficeDrawingFile* m_pReader;
    std::wstring m_sPassword;

public:
    CAscNativePrintDocument(const std::wstring& sPassword) : IAscNativePrintDocument()
    {
        m_pReader = NULL;
        m_sPassword = sPassword;
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
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA:
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
        m_pReader->LoadFromFile(m_sFilePath, L"", m_sPassword, m_sPassword);
    }

    virtual void Close()
    {
        m_pReader->Close();
    }

    virtual void Check(std::vector<CPagePrintData>& arPages)
    {
        arPages.clear();
        int nPagesCount = m_pReader->GetPagesCount();
        for (int i = 0; i < nPagesCount; ++i)
        {
            double dPageDpiX, dPageDpiY;
            double dWidth, dHeight;
            m_pReader->GetPageInfo(i, &dWidth, &dHeight, &dPageDpiX, &dPageDpiY);

            dWidth  *= 25.4 / dPageDpiX;
            dHeight *= 25.4 / dPageDpiY;

            CPagePrintData data;
            data.Width = dWidth;
            data.Height = dHeight;

            arPages.push_back(data);
        }
    }
};

#if defined(_LINUX) && !defined(_MAC)
#include <X11/Xlib.h>
#endif

#ifdef CEF_2623
#define CefBase_Class CefBase
#else
#define CefBase_Class CefBaseRefCounted
#endif
class CCefBinaryFileReaderCounter : public CefBase_Class
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

class CTemporaryDocumentInfo
{
public:
    std::wstring PathSrc;
    int Type;
    std::wstring Url;
    std::wstring ExternalCloud;
    std::wstring ParentUrl;

public:
    CTemporaryDocumentInfo(const std::wstring& sPathSrc, const int& nType,
                           const std::wstring& sUrl = L"",
                           const std::wstring& sExternalCloudId = L"",
                           const std::wstring& sParentUrl = L"")
    {
        PathSrc = sPathSrc;
        Type = nType;
        Url = sUrl;
        ExternalCloud = sExternalCloudId;
        ParentUrl = sParentUrl;
    }
};

class CAscClientHandler;
class CCefView_Private : public NSEditorApi::IMenuEventDataBase, public IASCFileConverterEvents, public CTextDocxConverterCallback
{
public:
    class CSystemMessage
    {
    public:
        int ViewID;
        std::string FrameID;
        std::string Message;

        CSystemMessage()
        {
            ViewID = -1;
            FrameID = "";
            Message = "";
        }

        CSystemMessage(const CSystemMessage& oSrc)
        {
            ViewID = oSrc.ViewID;
            FrameID = oSrc.FrameID;
            Message = oSrc.Message;
        }

        CSystemMessage& operator=(const CSystemMessage& oSrc)
        {
            ViewID = oSrc.ViewID;
            FrameID = oSrc.FrameID;
            Message = oSrc.Message;
            return *this;
        }
    };
    class CCloudCryptoUpload
    {
    public:
        CCefView_Private* View;
        CAscApplicationManager* Manager;

        std::list<std::wstring> Files;
        std::list<std::wstring> FilesDst;
        int_64_type FrameID;
        bool IsRemove;

        CCloudCryptoUpload()
        {
            IsRemove = false;
            FrameID = -1;
        }

        ~CCloudCryptoUpload()
        {
            if (IsRemove)
            {
                for (std::list<std::wstring>::iterator i = Files.begin(); i != Files.end(); i++)
                {
                    NSFile::CFileBinary::Remove(*i);
                }
            }
            for (std::list<std::wstring>::iterator i = FilesDst.begin(); i != FilesDst.end(); i++)
            {
                NSFile::CFileBinary::Remove(*i);
            }
        }

    public:
        void Init()
        {
            for (std::list<std::wstring>::iterator iter = Files.begin(); iter != Files.end(); iter++)
            {
                std::wstring sFile = *iter;

                COfficeFileFormatChecker oChecker;
                bool bIsOfficeFile = oChecker.isOfficeFile(sFile);

                if (bIsOfficeFile)
                {
                    // зашифрованные - итак зашифрованы
                    if (oChecker.nFileType == AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO)
                        bIsOfficeFile = false;
                }
                if (bIsOfficeFile)
                {
                    if (!IsFormatSupportCrypto(oChecker.nFileType))
                        bIsOfficeFile = false;
                }

                if (!bIsOfficeFile)
                {
                    // не нужна конвертация
                    FilesDst.push_back(L"");
                }
                else
                {
                    std::wstring sTmpFolder = NSDirectory::GetTempPath();
                    std::wstring sFileDst = NSFile::CFileBinary::CreateTempFileWithUniqueName(sTmpFolder, L"CC_");
                    if (NSFile::CFileBinary::Exists(sFileDst))
                        NSFile::CFileBinary::Remove(sFileDst);

                    std::wstring::size_type nPosPoint = sFileDst.rfind('.');
                    if (nPosPoint != std::wstring::npos && nPosPoint > sTmpFolder.length())
                        sFileDst = sFileDst.substr(0, nPosPoint);

                    NSDirectory::CreateDirectory(sFileDst);

                    sFileDst += (L"/" + NSFile::GetFileName(sFile));
                    FilesDst.push_back(sFileDst);
                }
            }

            NextFile();
        }
        CefRefPtr<CefFrame> GetFrame();
        void NextFile()
        {
            if (0 >= Files.size())
            {
                OnComplete();
                return;
            }
            std::wstring sFileSrc = *Files.begin();
            std::wstring sFileDst = *FilesDst.begin();

            if (sFileDst.empty())
            {
                OnCompleteFile(sFileSrc);
                return;
            }

            // запрос пароля
            CefRefPtr<CefFrame> pFrame = GetFrame();
            if (!pFrame)
            {
                OnCompleteFile(sFileSrc);
                return;
            }
            // нужно криптовать... запрашиваем пароль
            pFrame->ExecuteJavaScript("window.AscDesktopEditor.sendSystemMessage({ type : \"generatePassword\" });", pFrame->GetURL(), 0);
        }
        void OnPassword(const std::wstring& sPass, const std::wstring& sInfo)
        {
            CefRefPtr<CefFrame> pFrame = GetFrame();
            if (pFrame)
                pFrame->ExecuteJavaScript("window.onSystemMessage && window.onSystemMessage({ type : \"operation\", block : true, opType : 1 });", pFrame->GetURL(), 0);

            // пришел пароль из плагина. делаем конвертацию и отсылаем файл
            if (0 >= Files.size())
            {
                OnComplete();
                return;
            }
            std::wstring sFileSrc = *Files.begin();
            std::wstring sFileDst = *FilesDst.begin();

            // конвертируем
            NSStringUtils::CStringBuilder oBuilder;
            oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
            oBuilder.WriteEncodeXmlString(sFileSrc);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
            oBuilder.WriteEncodeXmlString(sFileDst);
            oBuilder.WriteString(L"</m_sFileTo>");

            COfficeFileFormatChecker oChecker;
            oChecker.isOfficeFile(sFileSrc);

            oBuilder.WriteString(L"<m_nFormatTo>");
            oBuilder.WriteString(std::to_wstring(oChecker.nFileType));
            oBuilder.WriteString(L"</m_nFormatTo>");
            oBuilder.WriteString(L"<m_sSavePassword>");
            oBuilder.WriteEncodeXmlString(sPass);
            oBuilder.WriteString(L"</m_sSavePassword>");
            oBuilder.WriteString(L"<m_sDocumentID>");
            oBuilder.WriteEncodeXmlString(sInfo);
            oBuilder.WriteString(L"</m_sDocumentID>");

            oBuilder.WriteString(L"<m_sFontDir>");
            oBuilder.WriteEncodeXmlString(Manager->m_oSettings.fonts_cache_info_path);
            oBuilder.WriteString(L"</m_sFontDir>");

            oBuilder.WriteString(L"</TaskQueueDataConvert>");

            std::wstring sXmlConvert = oBuilder.GetData();
            std::wstring sTempFileForParams = NSDirectory::GetTempPath() + L"/params_to_crypto.xml";
            NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);

            int nReturnCode = NSX2T::Convert(Manager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, Manager);
            NSFile::CFileBinary::Remove(sTempFileForParams);

            if (nReturnCode)
            {
                OnCompleteFile(sFileSrc);
                return;
            }

            // запрос сохранения пароля
            if (!pFrame)
            {
                OnCompleteFile(sFileSrc);
                return;
            }

            ICertificate* pCert = ICertificate::CreateInstance();
            std::string sHash = pCert->GetHash(sFileDst, OOXML_HASH_ALG_SHA256);
            delete pCert;

            std::string sPassA = U_TO_UTF8(sPass);
            std::string sInfoA = U_TO_UTF8(sInfo);

            if (pFrame)
            {
                pFrame->ExecuteJavaScript("window.AscDesktopEditor.sendSystemMessage({ type : \"setPasswordByFile\", isNeedMessage : true, hash : \"" + sHash + "\", password : \"" + sPassA + "\", docinfo : \"" + sInfoA + "\" });", pFrame->GetURL(), 0);
            }
        }
        void OnSavePassword()
        {
            // пароли сохранены
            if (0 >= Files.size())
            {
                OnComplete();
                return;
            }
            std::wstring sFileSrc = *Files.begin();
            std::wstring sFileDst = *FilesDst.begin();
            OnCompleteFile(sFileDst, true);
        }
        void OnSend()
        {
            // файл отослался
            if (0 >= Files.size())
                return; // error

            std::wstring sFile = *Files.begin();
            std::wstring sFileDst = *FilesDst.begin();
            Files.erase(Files.begin());
            FilesDst.erase(FilesDst.begin());

            if (IsRemove)
                NSFile::CFileBinary::Remove(sFile);
            if (!sFileDst.empty())
            {
                NSFile::CFileBinary::Remove(sFileDst);
                std::wstring sTmpDir = NSFile::GetDirectoryName(sFileDst);
                NSDirectory::DeleteDirectory(sTmpDir);
            }

            if (0 >= Files.size())
            {
                OnComplete();
                return;
            }

            NextFile();
        }

    private:
        void OnCompleteFile(const std::wstring& sFile, const bool& isCrypto = false)
        {
            // файл готов. отправляем его в облако. удалим все ненужное на деструкторе
            CefRefPtr<CefFrame> pFrame = GetFrame();
            if (!pFrame)
                return;

            std::wstring sFileCorrect = sFile;
#ifdef _WIN32
            NSStringUtils::string_replace(sFileCorrect, L"\\", L"/");
#endif
            pFrame->ExecuteJavaScript("window.on_cloud_crypto_upload && window.on_cloud_crypto_upload(\"" + U_TO_UTF8(sFileCorrect) + (isCrypto ? "\", true);" : "\", false);"), pFrame->GetURL(), 0);
        }
        void OnComplete()
        {
            // вся работа закончена
            CefRefPtr<CefFrame> pFrame = GetFrame();
            if (pFrame)
                pFrame->ExecuteJavaScript("window.on_cloud_crypto_upload = undefined;window.onSystemMessage({ type : \"upload_end\" });", pFrame->GetURL(), 0);
            View->m_pUploadFiles = NULL;
            delete this;
        }
    };

public:
    CefRefPtr<CAscClientHandler> m_handler;
    CAscApplicationManager* m_pManager;
    CCefView* m_pCefView;

    // окно
    CCefViewWidgetImpl* m_pWidgetImpl;

    // данные для печати
    CPrintData m_oPrintData;
    std::string m_sPrintParameters;

    // ссылка для view
    std::wstring m_strUrl;
    std::wstring m_sOriginalUrl;
    std::wstring m_sOpenAsLocalUrl;

    // вызывается для скачки файлов, после того, как покажем диалог для выбора куда качать
    CefRefPtr<CefBeforeDownloadCallback> m_before_callback;
    NSEditorApi::CAscDownloadFileInfo* m_before_callback_info; // мы можем отменить скачку (а евенты наверх уше ушли - надо удалить из приложения)

    // информация для локальных файлов
    CAscLocalFileInfoCS m_oLocalInfo;

    // конвертеры
    CASCFileConverterToEditor m_oConverterToEditor;
    CASCFileConverterFromEditor m_oConverterFromEditor;

    // криптованные данные храним в docx
    CTextDocxConverter m_oTxtToDocx;

    // идет сохранение с помощью x2t
    bool m_bIsSaving;
    // идет сохранение и криптование с помощью x2t
    bool m_bIsSavingCrypto;
    // нужно ли удалить папку recover
    bool m_bIsRemoveRecoveryOnClose;
    // идет закрытие веб-страницы
    bool m_bIsClosing;
    // идет удаление веб-страницы
    bool m_bIsDestroying;
    // показывается диалог для сохранения
    bool m_bIsSavingDialog;
    // идет сборка криптованного облачного файла
    bool m_bIsBuilding;
    // удален ли view
    bool m_bIsDestroy;
    // упал ли процесс рендерера
    bool m_bIsCrashed;
    // ошибка загрузки
    bool m_bIsLoadingError;

    // поддерживается ли криптование
    bool m_bIsOnlyPassSupport;

    // типо редактора (0 - word, 1 - slide, 2 - cell)
    int m_nEditorType;

    // ошибка при открытии файла (0 - все хорошо)
    int m_nLocalFileOpenError;

    // настройки для открытия pdf, djvu, xps
    CNativeFileViewerInfo m_oNativeViewer;
    std::wstring m_sNativeFilePassword;

    // текущий devicePixelRatio
    double m_dDeviceScale;
    // нужно ли при move/resize проверять deviceScale
    bool m_bIsWindowsCheckZoom;

    // настройки для репортера
    bool m_bIsReporter; // репортер
    int m_nReporterParentId; // репортер
    int m_nReporterChildId; // id репортера у parent
    std::string m_sCloudCryptoReporter;

    // sso
    bool m_bIsSSO;
    bool m_bIsFirstLoadSSO;
    std::wstring m_strSSOFirstDomain;
    std::map<std::wstring, bool> m_arSSOSecondDomain;

    // id фрейма, из которого пришел евент (для коллбэка)
    std::string m_sIFrameIDMethod;

    // криптование облачных файлов
    bool m_bIsCloudCryptFile;
    std::wstring m_sCloudCryptSrc;
    std::wstring m_sCloudCryptName;
    int m_nCloudCryptFileType;

    // скачка криптованного файла (в принципе можно просто качать что угодно)
    CCefView* m_pDownloadViewCallback;
    std::wstring m_sDownloadViewPath;

    // external id у recent
    std::wstring m_sRecentOpenExternalId;

    // hash info (GetHash js function)
    std::string m_sGetHashAlg;
    std::string m_sGetHashFrame;

    // картинки для скачки, в криптованном режиме
    std::map<std::wstring, std::wstring> m_arCryptoImages;
    std::map<std::wstring, int_64_type> m_arCryptoImagesFrameId;

    // файлы с ссылками для метода AscDesktopEditor.DownloadFiles
    std::map<std::wstring, CDownloadFileItem> m_arDownloadedFiles;
    std::map<std::wstring, std::wstring> m_arDownloadedFilesComplete;
    int64 m_nDownloadedFilesFrameId;

    // приходил ли хоть раз евент onDocumentModifiedChanged
    bool m_bIsReceiveOnce_OnDocumentModified;

    // url страницы, откуда был открыт текущий редактор (нужно для recents)
    std::wstring m_sParentUrl;

#if defined(_LINUX) && !defined(_MAC)
    WindowHandleId m_lNaturalParent;
#endif

    // прерывание скачивания у проблемных ссылок
    CDownloadFilesAborted m_oDownloaderAbortChecker;

    // настройки внешних (не onlyoffice) облаков
    bool m_bIsExternalCloud;
    CExternalCloudRegister m_oExternalCloud;

    // облачный криптованный файл => downloadAs
    int m_nCryptoDownloadAsFormat;
    std::string m_sCryptoDownloadAsParams;
    CSimpleConverter m_oSimpleX2tConverter;

    // версия облака и поддерживаемый им функционал
    std::string m_sCloudVersion;
    int m_nCloudVersion;

    // системные сообщения
    std::vector<CSystemMessage> m_arSystemMessages;

    // создание/аплоад криптованных файлов в облаке
    CCloudCryptoUpload* m_pUploadFiles;

    // путь к файлу для сравнения (только для локальных файлов)
    // после открытия нужно прокинуть в редактор и удалить файл (если он временный (по урлу))
    std::wstring m_sComparingFile;
    int m_nComparingFileType; // 0 - file, 1 - url

    bool m_bIsLocalFileLocked; // залочен ли файл?
    NSSystem::CLocalFileLocker* m_pLocalFileLocker; // класс для лока открытых файлов

    NSSystem::CLocalFileLocker* m_pLockRecover; // для корректнрой работы рековеров в не singleapplication mode

    std::string m_sVersionForReporter;

    CTemporaryDocumentInfo* m_pTemporaryCloudFileInfo;

public:
    CCefView_Private()
    {
        m_pManager = NULL;
        m_pWidgetImpl = NULL;
        m_pCefView = NULL;

        m_sPrintParameters = "";

        m_bIsSaving = false;
        m_bIsSavingCrypto = false;
        m_bIsRemoveRecoveryOnClose = false;
        m_bIsClosing = false;
        m_bIsDestroying = false;
        m_bIsSavingDialog = false;
        m_bIsBuilding = false;
        m_bIsDestroy = false;
        m_bIsCrashed = false;
        m_bIsLoadingError = false;

        m_strUrl = L"";

        m_before_callback = NULL;
        m_before_callback_info = NULL;

        m_oConverterToEditor.m_pEvents = this;
        m_oConverterFromEditor.m_pEvents = this;

        m_nEditorType = 0;
        m_nLocalFileOpenError = 0;

#if defined(_LINUX) && !defined(_MAC)
        m_lNaturalParent = 0;
#endif

        m_dDeviceScale = 1.0;
        m_bIsWindowsCheckZoom = false;

        m_bIsReporter = false;
        m_nReporterParentId = -1;
        m_nReporterChildId = -1;

        m_bIsSSO = false;
        m_bIsFirstLoadSSO = true;
        m_strSSOFirstDomain = L"";

        m_pDownloadViewCallback = NULL;

        m_bIsOnlyPassSupport = false;

        m_bIsReceiveOnce_OnDocumentModified = false;

        m_bIsCloudCryptFile = false;
        m_bIsExternalCloud = false;
        m_nCloudCryptFileType = 0;

        m_nCryptoDownloadAsFormat = -1;

        m_nCloudVersion = CRYPTO_CLOUD_SUPPORT;

        m_pUploadFiles = NULL;

        m_nComparingFileType = -1;

        m_bIsLocalFileLocked = false;
        m_pLocalFileLocker = NULL;

        m_nDownloadedFilesFrameId = -1;

        m_pLockRecover = NULL;
        m_pTemporaryCloudFileInfo = NULL;
    }

    void Destroy()
    {
        if (m_bIsDestroy)
            return;

        RELEASEOBJECT(m_pLockRecover);

        m_oConverterToEditor.Stop();
        m_oConverterFromEditor.Stop();
        m_oTxtToDocx.Stop();

        // разлочиваем файл
        RELEASEOBJECT(m_pLocalFileLocker);

        // смотрим, есть ли несохраненные данные
        m_oLocalInfo.m_oCS.Enter();

        bool bIsNeedRemove = false;
        if (!m_bIsSavingDialog && ((!m_oLocalInfo.m_oInfo.m_bIsModified && m_bIsReceiveOnce_OnDocumentModified) || m_bIsRemoveRecoveryOnClose))
            bIsNeedRemove = true;

        bool bIsChangesExist = false;
        std::wstring sChangesFile = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/changes/changes0.json";
        if (!NSFile::CFileBinary::Exists(sChangesFile))
        {
            // нет изменений - нечего рековерить. актуально для НОВОГО файла
            bIsNeedRemove = true;            
        }
        else
        {
            bIsChangesExist = true;
        }

        if (m_bIsReporter)
            bIsNeedRemove = false;

        if (m_bIsCrashed && bIsChangesExist)
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

        NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW);
        m_pCefView->GetAppManager()->GetEventListener()->OnEvent(pEvent);
        m_bIsDestroy = true;
    }

    ~CCefView_Private()
    {
        Destroy();
    }

    void CheckZoom();

    void CloseBrowser(bool _force_close);

    CefRefPtr<CefBrowser> GetBrowser() const;

    void CheckLockLocalFile()
    {
        if (!m_oLocalInfo.m_oInfo.m_bIsSaved)
            return;

        if (!NSSystem::CLocalFileLocker::IsSupportFunctionality())
            return;

        if (NSFile::CFileBinary::Exists(m_oLocalInfo.m_oInfo.m_sFileSrc))
        {
            if (m_pLocalFileLocker && m_pLocalFileLocker->GetFileLocked() != m_oLocalInfo.m_oInfo.m_sFileSrc)
                RELEASEOBJECT(m_pLocalFileLocker);

            if (!m_pLocalFileLocker)
                m_pLocalFileLocker = new NSSystem::CLocalFileLocker(m_oLocalInfo.m_oInfo.m_sFileSrc);
        }
    }
    void CheckLockRecoveryFile()
    {
        if (m_pManager->m_pInternal->m_bIsOnlyEditorWindowMode)
        {
            std::wstring sRecoveryLockerFile = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/rec.lock";
            NSFile::CFileBinary::SaveToFile(sRecoveryLockerFile, L"lock");
            m_pLockRecover = new NSSystem::CLocalFileLocker(sRecoveryLockerFile);
        }
    }

    int GetViewCryptoMode()
    {
        // общий режим
        int nCryptoMode = m_pManager->m_pInternal->m_nCurrentCryptoMode;

        // чужое облако, не поддерживающее шифрование
        if (m_bIsExternalCloud && !m_oExternalCloud.crypto_support)
            nCryptoMode = 0;

        // шифровальщик сам не поддерживает чужие облака
        if (m_bIsExternalCloud && m_pManager->m_pInternal->m_bCryptoDisableForExternalCloud)
            nCryptoMode = 0;

        // локальный файл и шифровальщик не поддерживает их
        if (m_pManager->m_pInternal->m_bCryptoDisableForLocal && !m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty() && m_sCloudCryptSrc.empty())
            nCryptoMode = 0;

        return nCryptoMode;
    }

    std::wstring GetFileDocInfo(const std::wstring& sFile)
    {
        // используется для криптования.
        // незашифрованная информация в зашифрованном файле
        std::wstring sDocInfo = L"";
        COfficeFileFormatChecker oChecker;
        bool bIsStorage = oChecker.isMS_OFFCRYPTOFormatFile(sFile, sDocInfo);
        if (!bIsStorage)
        {
            sDocInfo = L"";
            bIsStorage = oChecker.isOpenOfficeFormatFile(sFile, sDocInfo);
        }

        if (bIsStorage && !sDocInfo.empty())
        {
            // correct sDocInfo. only < 255 codes
            size_t sDocInfoLen = sDocInfo.length();
            wchar_t* sDocInfoData = (wchar_t*)sDocInfo.c_str();
            for (size_t i = 0; i < sDocInfoLen; ++i)
            {
                int nSymbol = sDocInfoData[i];
                if (nSymbol > 0xFF || nSymbol < 0x20)
                {
                    if (nSymbol != '\n')
                        sDocInfoData[i] = ' ';
                }
            }
        }
        return sDocInfo;
    }

    std::wstring GetViewDownloadPath()
    {
        return m_sDownloadViewPath;
    }
    void OnViewDownloadFile()
    {
        std::wstring::size_type posHash = m_sDownloadViewPath.rfind(L".asc_file_get_hash");
        if (std::wstring::npos != posHash)
        {
            std::wstring sTest = m_sDownloadViewPath.substr(posHash);
            if (sTest == L".asc_file_get_hash")
            {
                ICertificate* pCert = ICertificate::CreateInstance();
                std::string sHash = pCert->GetHash(m_sDownloadViewPath, OOXML_HASH_ALG_SHA256);
                delete pCert;

                std::wstring sDocInfo = GetFileDocInfo(m_sDownloadViewPath);
                std::string sDocInfoA = U_TO_UTF8(sDocInfo);

                NSFile::CFileBinary::Remove(m_sDownloadViewPath);

                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("file_get_hash_callback");
                message->GetArgumentList()->SetString(0, sHash);
                message->GetArgumentList()->SetString(1, m_sGetHashFrame);

                std::string sJSON = "{hash:\"" + sHash + "\",docinfo:\"" + sDocInfoA + "\"}";
                message->GetArgumentList()->SetString(2, sJSON);

                SEND_MESSAGE_TO_RENDERER_PROCESS(GetBrowser(), message);

                return;
            }
        }

        if (!m_sCloudCryptSrc.empty())
        {
            CCefViewEditor* pEditorThis = (CCefViewEditor*)m_pCefView;
            pEditorThis->OpenLocalFile(m_sDownloadViewPath, CCefViewEditor::GetFileFormat(m_sDownloadViewPath));
        }
    }

    void SendSystemMessage(CSystemMessage& sysMessage)
    {
        CTemporaryCS oCS(&m_pCefView->GetAppManager()->m_pInternal->m_oCS_SystemMessages);

        CCefView* pMainView = m_pCefView->GetAppManager()->m_pInternal->GetViewForSystemMessages();

        if (pMainView != m_pCefView)
        {
            if (pMainView->m_pInternal->m_arSystemMessages.size() == 0)
            {
                pMainView->m_pInternal->m_arSystemMessages.push_back(sysMessage);

                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_system_message");
                message->GetArgumentList()->SetString(0, sysMessage.Message);
                SEND_MESSAGE_TO_RENDERER_PROCESS(pMainView->m_pInternal->GetBrowser(), message);
            }
            else
            {
                pMainView->m_pInternal->m_arSystemMessages.push_back(sysMessage);
            }
        }
        else
        {
            if (m_arSystemMessages.empty())
                return; // error!!!

            CSystemMessage messageSrc = m_arSystemMessages[0];
            m_arSystemMessages.erase(m_arSystemMessages.begin());

            CCefView* pSrcSender = m_pCefView->GetAppManager()->GetViewById(messageSrc.ViewID);
            if (pSrcSender)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_system_message");
                message->GetArgumentList()->SetString(0, sysMessage.Message);
                message->GetArgumentList()->SetString(1, messageSrc.FrameID);
                SEND_MESSAGE_TO_RENDERER_PROCESS(pSrcSender->m_pInternal->GetBrowser(), message);
            }

            if (0 != m_arSystemMessages.size())
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_system_message");
                message->GetArgumentList()->SetString(0, m_arSystemMessages[0].Message);
                SEND_MESSAGE_TO_RENDERER_PROCESS(pMainView->m_pInternal->GetBrowser(), message);
            }
        }
    }

    void LocalFile_Start()
    {
        if (!m_sComparingFile.empty())
        {
            if (m_oConverterToEditor.m_oInfo.m_sRecoveryDir.empty())
                m_oLocalInfo.SetupOptions(m_oConverterToEditor.m_oInfo);

            m_oConverterToEditor.Stop();
            m_oConverterToEditor.Start(0);
            return;
        }

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
        m_oLocalInfo.m_oInfo.m_sDocumentInfo = L"";

        m_oConverterFromEditor.m_nTypeEditorFormat = m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat;

        if (!sPath.empty())
            m_oConverterFromEditor.m_oInfo.m_sFileSrc = sPath;

        if (m_pLocalFileLocker)
        {
            if (m_pLocalFileLocker->GetFileLocked() != m_oConverterFromEditor.m_oInfo.m_sFileSrc)
            {
                // сохраняем не в тот же файл
                RELEASEOBJECT(m_pLocalFileLocker);
            }
            else if (m_sCloudCryptSrc.empty())
            {
                // сохраняем обычный локальный файл по тому же пути
                m_oConverterFromEditor.m_pLocker = m_pLocalFileLocker;
            }
        }

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
        if (m_nCryptoDownloadAsFormat != -1)
        {
            m_nCryptoDownloadAsFormat = -1;
            m_sCryptoDownloadAsParams = "";

            if (this->GetBrowser())
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("crypto_download_as_end");
                SEND_MESSAGE_TO_RENDERER_PROCESS(GetBrowser(), message);
            }
            return;
        }

        LocalFile_SaveEnd(nError, sPass);
    }

    void LocalFile_GetSupportSaveFormats(std::vector<int>& arFormats)
    {
        // в какие форматы можно сохранить текущий документ
        bool bEncryption = (GetViewCryptoMode() != 0) ? true : false;

        // важен порядок (для красоты) - поэтому такие странные if'ы

        if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_DOCUMENT)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX);

#ifndef DISABLE_OFORM_SUPPORT
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF);
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM);
#endif

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA);
            }
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA);                
            }
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA);

                arFormats.push_back(AVS_OFFICESTUDIO_FILE_IMAGE_PNG);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_IMAGE_JPG);
            }
        }
        else if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
        {
            arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF);

            if (!bEncryption)
            {
                if (m_oLocalInfo.m_oInfo.m_nCurrentFileFormat != AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF)
                    arFormats.push_back(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX);

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX);
            }

            arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT);            

            if (!bEncryption)
            {
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2);
                arFormats.push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB);
            }
        }
    }

    bool LocalFile_IsSupportSaveCurrentFormat()
    {
        // открыли файл. А можно ли его сохранить? (без изменения формата)
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
        if ((nFormat & AVS_OFFICESTUDIO_FILE_IMAGE) != 0)
            return false;

        return true;
    }
    bool LocalFile_IsSupportOpenFormat(int nFormat)
    {
        if ((nFormat & AVS_OFFICESTUDIO_FILE_IMAGE) != 0)
            return false;

        return true;
    }

    virtual void CTextDocxConverterCallback_OnConvert(std::wstring sData)
    {
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(m_oTxtToDocx.m_bIsToDocx ? "set_advanced_encrypted_data" : "get_advanced_encrypted_data");
        NSArgumentList::SetInt64(message->GetArgumentList(), 0, m_oTxtToDocx.m_nFrameId);
        message->GetArgumentList()->SetString(1, m_oTxtToDocx.m_sData);
        SEND_MESSAGE_TO_RENDERER_PROCESS(GetBrowser(), message);
    }

    void SendProcessMessage(CefProcessId target_process, CefRefPtr<CefProcessMessage> message);
};

class CAscClientHandler : public client::ClientHandler, public CCookieFoundCallback, public client::ClientHandler::Delegate, public CefDialogHandler
{
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
                        #ifdef CEF_2623
                                const CefString& accept_lang,
                        #endif
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
    CCefView* m_pParent;
    bool m_bIsLoaded;

    bool m_bIsEditorTypeSet;
    int m_nBeforeBrowserCounter;

    CefRefPtr<CefBrowser> browser_;
    int browser_id_;

    bool m_bIsCrashed;

    CefRefPtr<CefJSDialogHandler> m_pCefJSDialogHandler;

    CefRefPtr<CefFileDialogCallback> m_pFileDialogCallback;
    CefRefPtr<CefFileDialogCallback> m_pDirectoryDialogCallback;

    enum client_menu_ids
    {
        CLIENT_ID_SHOW_DEVTOOLS   = MENU_ID_USER_FIRST,
        CLIENT_ID_CLOSE_DEVTOOLS,
        CLIENT_ID_INSPECT_ELEMENT
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
        m_pFileDialogCallback = NULL;
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

    CefRefPtr<CefDialogHandler> GetDialogHandler() OVERRIDE
    {
        return this;
    }

    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefDialogHandler::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              int selected_accept_filter,
                              CefRefPtr<CefFileDialogCallback> callback)
    {
#ifndef _MAC

        // BUG with crash renderer process after upload files to cloud

        NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
        if (NULL != m_pParent && NULL != m_pParent->GetAppManager())
            pListener = m_pParent->GetAppManager()->GetEventListener();

        int nMode = (mode & 0xFF);
        if ((nMode == FILE_DIALOG_OPEN || nMode == FILE_DIALOG_OPEN_MULTIPLE) && accept_filters.empty() && pListener)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_IsMultiselect((nMode == 1) ? true : false);
            pData->put_Filter(L"any");
            pEvent->m_pData = pData;

            m_pFileDialogCallback = callback;

            pListener->OnEvent(pEvent);
            return true;
        }

#ifdef _LINUX
        std::string sFilterCheck = "";
        if (1 == accept_filters.size())
            sFilterCheck = accept_filters[0].ToString();
        if ((nMode == FILE_DIALOG_OPEN || nMode == FILE_DIALOG_OPEN_MULTIPLE) && ("image/*" == sFilterCheck) && pListener)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_IsMultiselect((nMode == 1) ? true : false);
            pData->put_Filter(L"images");
            pEvent->m_pData = pData;

            m_pFileDialogCallback = callback;

            pListener->OnEvent(pEvent);
            return true;
        }
#endif

#endif

#ifdef DONT_USE_NATIVE_FILE_DIALOGS
        if (nMode == FILE_DIALOG_OPEN || nMode == FILE_DIALOG_OPEN_MULTIPLE)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_IsMultiselect((nMode == FILE_DIALOG_OPEN_MULTIPLE) ? true : false);

            std::wstring sFilterInput;
            bool isFirst = true;
            for (std::vector<CefString>::const_iterator iter = accept_filters.begin(); iter != accept_filters.end(); iter++)
            {
                if (!isFirst)
                {
                    sFilterInput += L";;";
                    isFirst = false;
                }

                sFilterInput += (iter->ToWString());
            }
            if (sFilterInput.empty())
                sFilterInput = L"any";
            else
                sFilterInput += L";;*.*";

            // пока не учитываем фильтры (переводы)
            std::wstring sFilter = L"any";
            if (L".docx.doc.docm.dot.dotm.dotx.epub.fodt.odt.ott.rtf;;*.*" == sFilterInput)
                sFilter = L"word";

            pData->put_Filter(sFilter);
            pEvent->m_pData = pData;

            // save callback
            m_pFileDialogCallback = callback;

            pListener->OnEvent(pEvent);
            return true;
        }

        if (nMode == FILE_DIALOG_OPEN_FOLDER)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG);
            NSEditorApi::CAscLocalOpenDirectoryDialog* pData = new NSEditorApi::CAscLocalOpenDirectoryDialog();
            pEvent->m_pData = pData;

            m_pDirectoryDialogCallback = callback;
            pListener->OnEvent(pEvent);
            return true;
        }
#endif

        return false;
    }

    std::wstring makeLowerUrl(const std::wstring& url)
    {
        std::wstring::size_type nLen = url.length();
        const wchar_t* pStr = (wchar_t*)url.c_str();

        std::wstring sRet;
        sRet.reserve(nLen);

        for (int i = 0; i < nLen; ++i)
        {
            sRet.append(1, (pStr[i] >= 'A' && pStr[i] <= 'Z') ? (pStr[i] + 'a' - 'A') : pStr[i]);
        }
        return sRet;
    }

    bool IsExternalCloudEditor(const std::wstring& sUrl, const std::wstring& sTest)
    {
        if (0 == sTest.find(L"regex:"))
        {
            std::wstring sTestRegex = sTest.substr(6);
            boost::wregex oRegEx(sTestRegex);
            if (boost::regex_search(sUrl, oRegEx))
                return true;
        }
        else
        {
            if (std::wstring::npos != sUrl.find(m_pParent->m_pInternal->m_oExternalCloud.test_editor))
                return true;
        }
        return false;
    }

    bool CheckPopup(std::wstring sUrl, bool bIsBeforeBrowse = false, bool bIsBackground = false, bool bIsNotOpenLinks = false)
    {
        NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
        if (NULL != m_pParent && NULL != m_pParent->GetAppManager())
            pListener = m_pParent->GetAppManager()->GetEventListener();

        std::wstring sUrlLower = makeLowerUrl(sUrl);
        if (!m_pParent->m_pInternal->m_bIsExternalCloud)
        {
            NSStringUtils::string_replace(sUrlLower, L".aspx?", L"?");
            NSStringUtils::string_replace(sUrlLower, L".ashx?", L"?");
        }

        // определяем, редактор ли это
        bool bIsEditor = (sUrlLower.find(L"files/doceditor?fileid") == std::wstring::npos) ? false : true;
        if (!bIsEditor)
        {
            // есть еще actions...
            bIsEditor = (sUrlLower.find(L"files/httphandlers/filehandler?action=") == std::wstring::npos) ? false : true;
        }

        if (!bIsEditor && m_pParent->m_pInternal->m_bIsExternalCloud)
        {
            bIsEditor = IsExternalCloudEditor(sUrlLower, m_pParent->m_pInternal->m_oExternalCloud.test_editor);

            if (bIsEditor)
            {
                // прокидываем во view информацию
                sUrl += (L"desktop_editor_cloud_type_external=" + m_pParent->m_pInternal->m_oExternalCloud.id + L"_ext_id");
            }
        }

        // определяем
        bool bIsDownload    = false;
        if (sUrlLower.find(L"filehandler?action=download") != std::wstring::npos)
            bIsDownload     = true;
        else if (sUrlLower.find(L"filehandler?action=view") != std::wstring::npos)
            bIsDownload     = true;

        if (!bIsBeforeBrowse && !bIsEditor && !bIsDownload && !bIsNotOpenLinks)
        {
            if (m_pParent && (m_pParent->GetType() == cvwtEditor || m_pParent->GetType() == cvwtSimple))
            {
                if (0 == sUrl.find(L"about:blank"))
                    return true;

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
            if (NULL != m_pParent && NULL != pListener && NULL == m_pParent->m_pInternal->m_before_callback)
            {
                // есть parent и ничего не качается в данный момент
                m_pParent->StartDownload(sUrl);
            }

            return true;
        }

        if (bIsEditor)
        {
            if (std::wstring::npos == sUrl.find(L"?"))
                sUrl += L"?placement=desktop";
            else
                sUrl += L"&placement=desktop";

            if (NULL != pListener)
            {
                NSEditorApi::CAscCreateTab* pData = new NSEditorApi::CAscCreateTab();
                pData->put_Url(sUrl);

                CRecentParent oRecentParent;
                oRecentParent.Url = sUrl;
                oRecentParent.Parent = m_pParent->m_pInternal->GetBrowser()->GetMainFrame()->GetURL().ToWString();

                m_pParent->GetAppManager()->m_pInternal->m_arRecentParents.push_back(oRecentParent);

                if (bIsBackground)
                    pData->put_Active(false);

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
        #ifndef MESSAGE_IN_BROWSER
        CefRefPtr<CefDictionaryValue>& extra_info,
        #endif
        bool* no_javascript_access) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        CheckPopup(target_url.ToWString(), false, (WOD_NEW_BACKGROUND_TAB == target_disposition) ? true : false);
        return true;
    }

    virtual bool OnOpenURLFromTab(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        CefRequestHandler::WindowOpenDisposition target_disposition,
        bool user_gesture) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        CheckPopup(target_url.ToWString(), false, (WOD_NEW_BACKGROUND_TAB == target_disposition) ? true : false, true);
        return true;
    }

    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                            #ifndef CEF_2623
                                bool user_gesture,
                            #endif
                                bool is_redirect) OVERRIDE
    {
        std::wstring sUrl = request->GetURL().ToWString();

        if (0 == sUrl.find(L"mailto"))
        {
            // disable navigation
            std::wstring sCode = L"window.open(\"" + sUrl + L"\");";
            if (frame)
                frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
            return true;
        }

        if (m_pParent->m_pInternal->m_bIsSSO)
        {
            if (m_pParent->m_pInternal->m_bIsFirstLoadSSO)
            {
                m_pParent->m_pInternal->m_strSSOFirstDomain = NSCommon::GetBaseDomain(sUrl);
                m_pParent->m_pInternal->m_bIsFirstLoadSSO = false;
            }
            else
            {
                if (!m_pParent->m_pInternal->m_strSSOFirstDomain.empty())
                {
                    std::wstring sPositionAuth = m_pParent->m_pInternal->m_strSSOFirstDomain + L"/auth.aspx";
                    std::wstring::size_type nPosition = sUrl.find(sPositionAuth);

                    if (std::string::npos != nPosition)
                    {
                        nPosition += sPositionAuth.length();
                        if (nPosition >= sUrl.length())
                        {
                            // add ?desktop=true
                            this->m_pParent->load(L"sso:" + sUrl + L"?desktop=true");
                            return true;
                        }
                    }

                    std::wstring sCurrentBaseDomain = NSCommon::GetBaseDomain(sUrl);

                    if (!sCurrentBaseDomain.empty() && sCurrentBaseDomain != m_pParent->m_pInternal->m_strSSOFirstDomain)
                    {
                        if (m_pParent->m_pInternal->m_arSSOSecondDomain.find(sCurrentBaseDomain) ==
                                m_pParent->m_pInternal->m_arSSOSecondDomain.end())
                        {
                            m_pParent->m_pInternal->m_arSSOSecondDomain.insert(std::pair<std::wstring, bool>(sCurrentBaseDomain, true));
                        }
                    }
                }
            }

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

                sUrlPortal += L"Products/Files/?desktop=true";

                if (!m_pParent->m_pInternal->m_strSSOFirstDomain.empty())
                {
                    for (std::map<std::wstring, bool>::iterator i = m_pParent->m_pInternal->m_arSSOSecondDomain.begin();
                         i != m_pParent->m_pInternal->m_arSSOSecondDomain.end(); i++)
                    {
                        m_pParent->GetAppManager()->Logout(i->first);
                    }
                }

                m_pParent->load(sUrlPortal);
                return true;
            }

            std::wstring sTestSUP = m_pParent->m_pInternal->m_strSSOFirstDomain + L"/auth.aspx?sup=";
            if (std::wstring::npos != sUrl.find(sTestSUP) && std::wstring::npos == sUrl.find(L"desktop=true"))
            {
                if (sUrl.rfind('&') == (sUrl.length() - 1))
                    sUrl += L"desktop=true";
                else
                    sUrl += L"&desktop=true";

                sUrl = L"sso:" + sUrl;

                if (!m_pParent->m_pInternal->m_strSSOFirstDomain.empty())
                {
                    for (std::map<std::wstring, bool>::iterator i = m_pParent->m_pInternal->m_arSSOSecondDomain.begin();
                         i != m_pParent->m_pInternal->m_arSSOSecondDomain.end(); i++)
                    {
                        std::wstring sSSO_2 = i->first;
                        m_pParent->GetAppManager()->Logout(sSSO_2);

                        // check base domain
                        std::wstring::size_type posSSO_2_1 = sSSO_2.rfind('.');
                        if (std::wstring::npos != posSSO_2_1)
                        {
                            std::wstring::size_type posSSO_2_2 = sSSO_2.rfind('.', posSSO_2_1 - 1);
                            if (std::wstring::npos != posSSO_2_2)
                            {
                                std::wstring sSSO_2_2 = sSSO_2.substr(posSSO_2_2 + 1);
                                if (std::wstring::npos == m_pParent->m_pInternal->m_strSSOFirstDomain.find(sSSO_2_2))
                                    m_pParent->GetAppManager()->Logout(sSSO_2_2);
                            }
                        }
                    }
                }

                m_pParent->load(sUrl);
                return true;
            }

            std::wstring sTestSUP_Simple = m_pParent->m_pInternal->m_strSSOFirstDomain + L"/";
            std::wstring sTestUrl_Simple = sUrl;
            std::wstring::size_type nFindSS = sTestUrl_Simple.find(L"//");
            if (std::wstring::npos != nFindSS)
                sTestUrl_Simple = sTestUrl_Simple.substr(nFindSS + 2);

            if (0 == sTestSUP_Simple.find(sTestUrl_Simple) && std::wstring::npos == sUrl.find(L"desktop=true"))
            {
                if (sUrl.rfind(L"/") != (sUrl.length() - 1))
                    sUrl += L"/";

                m_pParent->load(sUrl + L"Products/Files/?desktop=true");
                return true;
            }

            if (!m_pParent->m_pInternal->m_bIsExternalCloud)
            {
                std::wstring::size_type nPosWithoutD = NSCommon::FindLowerCaseR(sUrl, L"/products/files/");
                if (nPosWithoutD != std::wstring::npos)
                {
                    if ((nPosWithoutD + 16) == sUrl.length())
                    {
                        m_pParent->load(sUrl + L"?desktop=true");
                        return true;
                    }
                }
            }
        }

        std::wstring sTest1 = sUrl;
        std::wstring sTest2 = m_pParent->m_pInternal->m_strUrl;
        NSCommon::url_correct(sTest1);
        NSCommon::url_correct(sTest2);

        NSStringUtils::string_replace(sTest1, L" ", L"%20");
        NSStringUtils::string_replace(sTest2, L" ", L"%20");
        NSStringUtils::string_replace(sTest1, L"\\", L"/");
        NSStringUtils::string_replace(sTest2, L"\\", L"/");

        if (frame->IsMain() && m_nBeforeBrowserCounter != 0 && sUrl.find(L"file://") == 0 && sTest1 != sTest2)
            return true;

        m_nBeforeBrowserCounter++;

        if (m_pParent && m_pParent->GetType() == cvwtSimple)
        {
            bool bHandled = CheckPopup(sUrl, true);
            if (bHandled)
                return true;
        }

        bool ret = client::ClientHandler::OnBeforeBrowse(browser, frame, request,
                                                 #ifndef CEF_2623
                                                         user_gesture,
                                                 #endif
                                                         is_redirect);
        if (NULL != m_pParent)
        {            
            m_pParent->resizeEvent();
            m_pParent->focus();

            if (!m_bIsLoaded)
            {
                m_pParent->GetWidgetImpl()->OnLoaded();
                m_bIsLoaded = true;
            }
        }
        return ret;
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                      #ifndef MESSAGE_IN_BROWSER
                                          CefRefPtr<CefFrame> frame,
                                      #endif
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        CAscApplicationManager* pManager = m_pParent->GetAppManager();
        NSEditorApi::CAscCefMenuEventListener* pListener = NULL;
        if (NULL != m_pParent && NULL != pManager)
            pListener = pManager->GetEventListener();

        if (!pListener)
            return false;

        CefRefPtr<CefListValue> args = message->GetArgumentList();

        // Check for messages from the client renderer.
        std::string message_name = message->GetName();
        if (message_name == "EditorType" && !m_bIsEditorTypeSet)
        {
            // прокидываем наверх тип редактора
            m_bIsEditorTypeSet = true;
            m_pParent->m_pInternal->m_nEditorType = args->GetInt(0);

            NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
            pData->put_Id(m_pParent->GetId());
            pData->put_Type(m_pParent->m_pInternal->m_nEditorType);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE);
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "spell_check_task")
        {
            // задача для проверки орфографии
            m_pParent->GetAppManager()->SpellCheck(args->GetInt(0),
                                                   args->GetString(1).ToString(),
                                                   NSArgumentList::GetInt64(args, 2));
            return true;
        }
        else if (message_name == "create_editor_api")
        {
            // евент после создания api (sdkjs)
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
            return true;
        }
        else if (message_name == "on_js_context_created")
        {
            // вызывается, после создания js контекста
            bool bIsOnlyPassSupport = m_pParent->GetAppManager()->m_oSettings.pass_support;
            if (bIsOnlyPassSupport)
            {
                if (0 == m_pParent->GetAppManager()->m_pInternal->m_mapCrypto.size())
                    bIsOnlyPassSupport = false;
            }
            if (m_pParent->m_pInternal->m_bIsExternalCloud && !m_pParent->m_pInternal->m_oExternalCloud.crypto_support)
                bIsOnlyPassSupport = false;

            int nFlags = 0;
            if (bIsOnlyPassSupport)
                nFlags |= 0x01;

            if (m_pParent->GetAppManager()->m_oSettings.protect_support)
                nFlags |= 0x02;

            m_pParent->m_pInternal->m_bIsOnlyPassSupport = bIsOnlyPassSupport;

            int nCryptoMode = m_pParent->m_pInternal->GetViewCryptoMode();

            CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_js_context_created_callback");
            messageOut->GetArgumentList()->SetInt(0, nFlags);
            messageOut->GetArgumentList()->SetInt(1, nCryptoMode);
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            return true;
        }
        else if (message_name == "on_init_js_context")
        {
            // вызывается, после создания js контекста
            std::vector<NSEditorApi::CAscMenuEvent*>* pArr = NULL;
            CApplicationManagerAdditionalBase* pAdditional = pManager->m_pInternal->m_pAdditional;

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

            // прокидываем cloudCrypto для репортера
            if (m_pParent->m_pInternal->m_bIsReporter && !m_pParent->m_pInternal->m_sCloudCryptoReporter.empty())
            {
                CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
                if (_frame)
                {
                    std::string sCloudCryptoJson = m_pParent->m_pInternal->m_sCloudCryptoReporter;
                    NSStringUtils::string_replaceA(sCloudCryptoJson, "\\", "\\\\");
                    NSStringUtils::string_replaceA(sCloudCryptoJson, "\"", "\\\"");
                    std::string sCodeReporter = "window.AscDesktopEditor.execCommand(\"portal:cryptoinfo\", \"" + sCloudCryptoJson + "\");";
                    _frame->ExecuteJavaScript(sCodeReporter, _frame->GetURL(), 0);
                }
            }
            return true;
        }
        else if (message_name == "is_cookie_present")
        {
            CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
            pVisitor->m_pCallback       = this;
            pVisitor->m_sDomain         = args->GetString(0).ToString();
            pVisitor->m_sCookieSearch   = args->GetString(1).ToString();

            pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));
            return true;
        }
        else if (message_name == "on_check_auth")
        {
            CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
            pVisitor->m_pCallback       = this;
            pVisitor->m_sCookieSearch   = "asc_auth_key";

            int nCount = args->GetInt(0);
            for (int i = 0; i < nCount; i++)
                pVisitor->m_arDomains.push_back(args->GetString(i + 1));

            pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));
            return true;
        }
        else if (message_name == "set_cookie")
        {
            CCefCookieSetter* pVisitor = new CCefCookieSetter();
            pVisitor->m_pCallback       = this;

            pVisitor->m_sUrl            = args->GetString(0).ToString();
            pVisitor->m_sDomain         = args->GetString(1).ToString();
            pVisitor->m_sPath           = args->GetString(2).ToString();
            pVisitor->m_sCookieKey      = args->GetString(3).ToString();
            pVisitor->m_sCookieValue    = args->GetString(4).ToString();
            
            pVisitor->Correct();
            pVisitor->SetCookie(CefCookieManager::GetGlobalManager(NULL));
            return true;
        }
        else if (message_name == "onDocumentModifiedChanged")
        {
            bool bValue = args->GetBool(0);
            m_pParent->m_pInternal->m_bIsReceiveOnce_OnDocumentModified = true;
            m_pParent->SetModified(bValue);

            NSEditorApi::CAscCefMenuEvent* pEvent = new NSEditorApi::CAscCefMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED;
            pEvent->put_SenderId(m_pParent->GetId());

            NSEditorApi::CAscDocumentModifyChanged* pData = new NSEditorApi::CAscDocumentModifyChanged();
            pData->put_Id(m_pParent->GetId());
            pData->put_Changed(bValue);
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "set_document_name")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME);

            NSEditorApi::CAscDocumentName* pData = new NSEditorApi::CAscDocumentName();
            pData->put_Id(m_pParent->GetId());
            pData->put_Name(args->GetString(0).ToWString());

            pEvent->m_pData = pData;

            if (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
            {
                CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
                if (_frame)
                {
                    std::wstring sUrl = _frame->GetURL().ToWString();
                    NSCommon::url_correct(sUrl);

                    if (sUrl.find(L"/index.reporter.html") != std::wstring::npos)
                        return true;

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

                    if (true)
                    {
                        // уточняем формат по расширению
                        COfficeFileFormatChecker oChecker;
                        std::wstring sFileExt = NSFile::GetFileExtention(pData->get_Name());
                        NSCommon::makeLowerW(sFileExt);
                        int nTypeByExt = oChecker.GetFormatByExtension(L"." + sFileExt);

                        if (nType == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX && (((nTypeByExt & AVS_OFFICESTUDIO_FILE_DOCUMENT) != 0) || ((nTypeByExt & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM) != 0)))
                            nType = nTypeByExt;
                        else if (nType == AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX && ((nTypeByExt & AVS_OFFICESTUDIO_FILE_SPREADSHEET) != 0))
                            nType = nTypeByExt;
                        else if (nType == AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX && ((nTypeByExt & AVS_OFFICESTUDIO_FILE_PRESENTATION) != 0))
                            nType = nTypeByExt;
                    }

                    std::wstring sExtId = L"";
                    if (m_pParent->m_pInternal->m_bIsExternalCloud)
                        sExtId = m_pParent->m_pInternal->m_oExternalCloud.id;

                    CTemporaryDocumentInfo* pTempInfo = new CTemporaryDocumentInfo(sPath + L"/" + pData->get_Name(),
                                                                                   nType, sUrl, sExtId, m_pParent->m_pInternal->m_sParentUrl);
                    m_pParent->m_pInternal->m_pTemporaryCloudFileInfo = pTempInfo;

                    pManager->m_pInternal->Recents_Add(pTempInfo->PathSrc, pTempInfo->Type, pTempInfo->Url, pTempInfo->ExternalCloud, pTempInfo->ParentUrl);
                    pData->put_Path(sPath);
                    pData->put_Url(sUrl);
                }
            }
            else if (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved)
            {
                std::wstring sPath = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
#ifdef WIN32
                NSStringUtils::string_replace(sPath, L"/", L"\\");
#endif
                pData->put_Path(sPath);
            }

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_document_save")
        {
            // для локального файла посылаем когда прошла конвертация
            if (m_pParent->m_pInternal->m_bIsSaving &&
                (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty() ||
                !m_pParent->m_pInternal->m_sCloudCryptSrc.empty()))
            {
                m_pParent->m_pInternal->m_bIsSaving = false;

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);

                NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
                pData->put_Id(m_pParent->GetId());

                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
            }
            return true;
        }
        else if (message_name == "js_message")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE);

            NSEditorApi::CAscJSMessage* pData = new NSEditorApi::CAscJSMessage();
            pData->put_Id(m_pParent->GetId());
            pData->put_Name(args->GetString(0).ToWString());
            pData->put_Value(args->GetString(1).ToWString());

            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_exec_command")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);

            NSEditorApi::CAscExecCommand* pData = new NSEditorApi::CAscExecCommand();
            pData->put_Command(args->GetString(0).ToWString());

            if (args->GetSize() == 2)
                pData->put_Param(args->GetString(1).ToWString());

            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "print_start")
        {
            // начало печати
            m_pParent->m_pInternal->m_oPrintData.m_sDocumentUrl = args->GetString(0).ToWString();
            m_pParent->m_pInternal->m_oPrintData.m_sFrameUrl = args->GetString(2).ToWString();

            m_pParent->m_pInternal->m_oPrintData.m_arPages.resize(args->GetInt(1));
            m_pParent->m_pInternal->m_oPrintData.m_sThemesUrl = args->GetString(3).ToWString();
            m_pParent->m_pInternal->m_oPrintData.m_nCurrentPage = args->GetInt(4);

            if (!m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.empty())
            {
                m_pParent->m_pInternal->m_oPrintData.m_sDocumentUrl = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/";
            }

            m_pParent->m_pInternal->m_oPrintData.CalculateImagePaths(!m_pParent->m_pInternal->m_sCloudCryptSrc.empty());

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_START);
            NSEditorApi::CAscTypeId* pData = new NSEditorApi::CAscTypeId();
            pData->put_Id(m_pParent->GetId());
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "print_page")
        {
            // запоминаем пдф-команды для каждой страницы
            int nIndex = args->GetInt(1);
            int nProgress = 100 * (nIndex + 1) / (int)m_pParent->m_pInternal->m_oPrintData.m_arPages.size();

            if (nProgress >= 0 && nProgress <= 100)
            {
                m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Base64 = args->GetString(0).ToString();
                m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Width  = args->GetDouble(2);
                m_pParent->m_pInternal->m_oPrintData.m_arPages[nIndex].Height = args->GetDouble(3);
            }

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS);

            NSEditorApi::CAscPrintProgress* pData = new NSEditorApi::CAscPrintProgress();
            pData->put_Id(m_pParent->GetId());
            pData->put_Progress(nProgress);

            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "print_end")
        {
            // окончание печати на js - стороне. отправляем евент наверх, что готовы к печати
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END);

            NSEditorApi::CAscPrintEnd* pData = new NSEditorApi::CAscPrintEnd();
            pData->put_Id(m_pParent->GetId());
            pData->put_PagesCount((int)m_pParent->m_pInternal->m_oPrintData.m_arPages.size());
            pData->put_CurrentPage(m_pParent->m_pInternal->m_oPrintData.m_nCurrentPage);
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);

            int nParam = 0;
            if (args->GetSize() > 0)
                nParam = args->GetInt(0);

            pManager->m_oPrintSettings.Mode = (nParam == 1) ? CAscPrintSettings::pmFit : CAscPrintSettings::pm100;

#if 0
            m_pParent->m_pInternal->m_oPrintData.Print_Start();

            for (int i = 0; i < m_pParent->m_pInternal->m_oPrintData.m_arPages.GetCount(); ++i)
                m_pParent->m_pInternal->m_oPrintData.TestSaveToRasterFile(L"D:\\test_page" + std::to_wstring(i) + L".png", 793, 1122, i);

            m_pParent->m_pInternal->m_oPrintData.Print_End();
#endif
            return true;
        }
        else if (message_name == "print")
        {
            // метод для печати (AscDesktopEditor.Print)
            // для редакторов - вызывает asc_nativePrint
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_START;
            m_pParent->m_pInternal->m_sPrintParameters = "";
            if (args->GetSize() == 1)
                m_pParent->m_pInternal->m_sPrintParameters = args->GetString(0);
            m_pParent->Apply(pEvent);
            return true;
        }
        else if (message_name == "load_js")
        {
            // загрузка скриптов
            NSEditorApi::CAscEditorScript* pData = new NSEditorApi::CAscEditorScript();
            pData->put_Url(args->GetString(0).ToWString());
            pData->put_Destination(args->GetString(1).ToWString());
            pData->put_Id(m_pParent->GetId());
            pData->put_FrameId(NSArgumentList::GetInt64(args, 2));

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION;
            pEvent->m_pData = pData;

            pManager->Apply(pEvent);
            return true;
        }
        else if (message_name == "onfullscreenenter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER);
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onfullscreenleave")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE);
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onlocaldocument_loadstart")
        {
            m_pParent->m_pInternal->LocalFile_IncrementCounter();
            return true;
        }
        else if (message_name == "onlocaldocument_open")
        {
            // этот метод не используется. для старой стартовой страницы
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN);
            NSEditorApi::CAscLocalFileOpen* pData = new NSEditorApi::CAscLocalFileOpen();
            if (args->GetSize() == 1)
                pData->put_Directory(args->GetString(0).ToWString());
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onlocaldocument_create")
        {
            // этот метод не используется. для старой стартовой страницы
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE);
            NSEditorApi::CAscLocalFileCreate* pData = new NSEditorApi::CAscLocalFileCreate();
            pData->put_Type(args->GetInt(0));
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onlocaldocument_sendrecents")
        {
            pManager->m_pInternal->Recents_Dump();
            return true;
        }
        else if (message_name == "onlocaldocument_openrecents")
        {
            NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = new NSEditorApi::CAscLocalOpenFileRecent_Recover();
            pData->put_Id(args->GetInt(0));
            pData->put_IsRecover(false);
            pData->put_Path(pManager->m_pInternal->m_arRecents[pData->get_Id()].m_sPath);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN);
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onlocaldocument_removerecents")
        {
            pManager->m_pInternal->Recents_Remove(args->GetInt(0));
            return true;
        }
        else if (message_name == "onlocaldocument_removeallrecents")
        {
            pManager->m_pInternal->Recents_RemoveAll();
            return true;
        }
        else if (message_name == "onlocaldocument_sendrecovers")
        {
            pManager->m_pInternal->Recovers_Dump();
            return true;
        }
        else if (message_name == "onlocaldocument_openrecovers")
        {
            NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = new NSEditorApi::CAscLocalOpenFileRecent_Recover();
            pData->put_Id(args->GetInt(0));
            pData->put_IsRecover(true);
            pData->put_Path(pManager->m_pInternal->m_arRecovers[pData->get_Id()].m_sPath);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN);
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "onlocaldocument_removerecovers")
        {
            pManager->m_pInternal->Recovers_Remove(args->GetInt(0), true);
            return true;
        }
        else if (message_name == "onlocaldocument_removeallrecovers")
        {
            pManager->m_pInternal->Recovers_RemoveAll();
            return true;
        }
        else if (message_name == "onlocaldocument_onsavestart")
        {
            bool bIsNeedSave = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved ? false : true;
            std::string sParams     = args->GetString(0).ToString();
            std::wstring sPassword  = args->GetString(1).ToWString();
            std::wstring sDocInfo   = args->GetString(2).ToWString();
            int nSaveFileType       = args->GetInt(3);

            bool bIsSaveAs = (sParams.find("saveas=true") != std::string::npos) ? true : false;

            // нужно ли показывать диалог?
            // 1) файл новый/восстановленный, т.е. некуда пока сохранять
            // 2) saveAs
            // 3) текущий формат не поддерживает сохранение
            bool bIsNeedSaveDialog = bIsNeedSave || bIsSaveAs || (!m_pParent->m_pInternal->LocalFile_IsSupportSaveCurrentFormat());

            CApplicationManagerAdditionalBase* pAdditional = pManager->m_pInternal->m_pAdditional;
            if (pAdditional)
            {
                if (pAdditional->Local_Save_End(bIsNeedSaveDialog, m_pParent->GetId(), m_pParent->m_pInternal->m_handler->GetBrowser()))
                    return true;
            }

            m_pParent->m_pInternal->m_oLocalInfo.m_bIsRetina = (sParams.find("retina=true") != std::string::npos) ? true : false;
            m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sPassword = sPassword;
            m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sDocumentInfo = sDocInfo;

            if (bIsNeedSaveDialog)
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE);
                NSEditorApi::CAscLocalSaveFileDialog* pData = new NSEditorApi::CAscLocalSaveFileDialog();
                pData->put_Id(m_pParent->GetId());

                if (!bIsNeedSave)
                    pData->put_Path(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc);
                else
                    pData->put_Path(NSFile::GetFileName(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc));

                pData->put_FileType(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);
                if (nSaveFileType == 0)
                    m_pParent->m_pInternal->LocalFile_GetSupportSaveFormats(pData->get_SupportFormats());
                else
                    pData->get_SupportFormats().push_back(nSaveFileType);
                pEvent->m_pData = pData;

                pListener->OnEvent(pEvent);
                m_pParent->m_pInternal->m_bIsSavingDialog = true;
            }
            else
            {
                m_pParent->m_pInternal->LocalFile_SaveStart();
            }
            return true;
        }
        else if (message_name == "onlocaldocument_onaddimage")
        {
            // добавление картинки (с показом диалога)
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_logout")
        {
            // AscDesktopEditor.Logout
            m_pParent->GetAppManager()->Logout(args->GetString(0).ToWString());
            return true;
        }
        else if (message_name == "on_setadvancedoptions")
        {
            // открытие с настройками
            m_pParent->m_pInternal->m_oConverterToEditor.m_sAdditionalConvertation = args->GetString(0).ToWString();

            // detect password
            std::wstring sWrap = L"<info>" + m_pParent->m_pInternal->m_oConverterToEditor.m_sAdditionalConvertation + L"</info>";
            XmlUtils::CXmlNode node;
            if (node.FromXmlString(sWrap))
            {
                XmlUtils::CXmlNode nodePass = node.ReadNode(L"m_sPassword");

                if (nodePass.IsValid())
                    m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sPassword = nodePass.GetTextExt();
            }

            m_pParent->m_pInternal->LocalFile_Start();
            return true;
        }
        else if (message_name == "DropOfficeFiles")
        {
            // js (ondrop) => AscDesktopEditor.DropOfficeFiles
            NSEditorApi::CAscLocalOpenFiles* pData = new NSEditorApi::CAscLocalOpenFiles();

            int nCount = (int)args->GetSize();
            std::vector<std::wstring>& arFolder = pData->get_Files();

            for (int i = 0; i < nCount; i++)
            {
                std::wstring sValue = args->GetString(i).ToWString();
                COfficeFileFormatCheckerWrapper oChecker;
                if (oChecker.isOfficeFile2(sValue, true))
                    arFolder.push_back(sValue);
            }

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN);
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "native_viewer_onopened")
        {
            // открыт вьюер
            std::string sBase64 = args->GetString(0).ToString();
            m_pParent->m_pInternal->m_sNativeFilePassword = args->GetString(1).ToWString();
            m_pParent->m_pInternal->m_oConverterToEditor.NativeViewerOpenEnd(sBase64);
            return true;
        }
        else if (message_name == "on_signature_sign")
        {
            // подписать
            std::string sId     = args->GetString(0).ToString();
            std::wstring sGuid  = args->GetString(1).ToWString();
            std::wstring sUrl   = args->GetString(2).ToWString();
            std::wstring sUrl2  = args->GetString(3).ToWString();

            std::wstring sFile = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
            int nCurrentFormat = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat;
            std::wstring sPassword = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sPassword;

            switch (nCurrentFormat)
            {
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF:
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM:
                case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:
                {
                    break;
                }
                default:
                {
                    CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_editor_warning");
                    messageOut->GetArgumentList()->SetString(0, "This feature only for docx/pptx/xlsx formats");
                    SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
                    return true;
                }
            }

            NSOOXMLPassword::COOXMLZipDirectory oZIP(m_pParent->GetAppManager());
            oZIP.Open(sFile, sPassword);

            std::wstring sUnzipDir = oZIP.GetDirectory();

            ICertificate* pCertificate = ICertificate::GetById(sId);
            std::wstring sSignatures = L"";
            int nSignError = 0;
            if (pCertificate)
            {
                COOXMLSigner oOOXMLSigner(sUnzipDir, pCertificate);

                oOOXMLSigner.SetGuid(sGuid);
                oOOXMLSigner.SetImageValid(sUrl);
                oOOXMLSigner.SetImageInvalid(sUrl2);

                nSignError = oOOXMLSigner.Sign();

                CASCFileConverterToEditor* pConverter = &m_pParent->m_pInternal->m_oConverterToEditor;
                pConverter->CheckSignaturesByDir(sUnzipDir);

                sSignatures = pConverter->GetSignaturesJSON();
            }

            RELEASEOBJECT(pCertificate);

            if (m_pParent->m_pInternal->m_pLocalFileLocker)
            {
                if (m_pParent->m_pInternal->m_pLocalFileLocker->GetFileLocked() != sFile)
                {
                    RELEASEOBJECT(m_pParent->m_pInternal->m_pLocalFileLocker);
                }
                else
                {
                    oZIP.m_pLocker = m_pParent->m_pInternal->m_pLocalFileLocker;
                }
            }
            int nRetValue = oZIP.Close((0 == nSignError) ? false : true);

            if (0 == nRetValue && 0 == nSignError)
            {
                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_signature_update_signatures");
                messageOut->GetArgumentList()->SetString(0, sSignatures);
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }

            if (1 == nSignError)
            {
                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_editor_warning");
                messageOut->GetArgumentList()->SetString(0, "This certificate is not supported");
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }

            m_pParent->m_pInternal->CheckLockLocalFile();
            return true;
        }
        else if (message_name == "on_signature_remove")
        {
            // удалить подпись
            std::string sGuid;
            if (args->GetSize() > 0)
                sGuid = args->GetString(0).ToString();

            std::wstring sFile = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
            std::wstring sPassword = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sPassword;

            NSOOXMLPassword::COOXMLZipDirectory oZIP(m_pParent->GetAppManager());
            oZIP.Open(sFile, sPassword);

            std::wstring sUnzipDir = oZIP.GetDirectory();

            CASCFileConverterToEditor* pConverter = &m_pParent->m_pInternal->m_oConverterToEditor;
            pConverter->CheckSignaturesByDir(sUnzipDir);
            pConverter->m_pVerifier->RemoveSignature(sGuid);

            if (m_pParent->m_pInternal->m_pLocalFileLocker)
            {
                if (m_pParent->m_pInternal->m_pLocalFileLocker->GetFileLocked() != sFile)
                {
                    RELEASEOBJECT(m_pParent->m_pInternal->m_pLocalFileLocker);
                }
                else
                {
                    oZIP.m_pLocker = m_pParent->m_pInternal->m_pLocalFileLocker;
                }
            }

            int nRetValue = oZIP.Close();

            if (0 == nRetValue)
            {
                std::wstring sSignatures = pConverter->GetSignaturesJSON();

                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_signature_update_signatures");
                messageOut->GetArgumentList()->SetString(0, sSignatures);
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }

            m_pParent->m_pInternal->CheckLockLocalFile();
            return true;
        }
        else if (message_name == "on_signature_viewcertificate")
        {
            // посмотреть сертификат
            int nIndex = args->GetInt(0);
            COOXMLVerifier* pVerifier = m_pParent->m_pInternal->m_oConverterToEditor.m_pVerifier;
            if (NULL != pVerifier)
            {
                COOXMLSignature* pSign = pVerifier->GetSignature(nIndex);
                WindowHandleId _handle = m_pParent->GetWidgetImpl()->cef_handle;
                int nRet = pSign ? pSign->GetCertificate()->ShowCertificate(&_handle) : 0;

                if (-1 == nRet)
                {
                    std::string sData = pSign->GetCertificate()->Print();
                    if (!sData.empty())
                    {
                        NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE);
                        NSEditorApi::CAscX509CertificateData* pData = new NSEditorApi::CAscX509CertificateData();
                        pData->put_Data(UTF8_TO_U(sData));
#ifdef _MAC
                        std::string sCertificateContent = pSign->GetCertificate()->GetCertificateBase64();
                        BYTE* pDataCert = NULL;
                        int nDataCertLen = 0;
                        NSFile::CBase64Converter::Decode(sCertificateContent.c_str(), (int)sCertificateContent.length(), pDataCert, nDataCertLen);

                        if (0 < nDataCertLen)
                        {
                            std::wstring sFilePath = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pParent->m_pInternal->m_pManager->m_pInternal->StartTmpDirectory(), L"OL");
                            if (NSFile::CFileBinary::Exists(sFilePath))
                                NSFile::CFileBinary::Remove(sFilePath);
                            sFilePath += L".cer";
                            NSFile::CFileBinary oFile;
                            if (oFile.CreateFileW(sFilePath))
                            {
                                oFile.WriteFile(pDataCert, (DWORD)nDataCertLen);
                            }
                            oFile.CloseFile();
                            pData->put_FilePath(sFilePath);
                        }

                        RELEASEARRAYOBJECTS(pDataCert);
#endif

                        pEvent->m_pData = pData;
                        pListener->OnEvent(pEvent);
                    }
                }

                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_signature_viewcertificate_ret");
                messageOut->GetArgumentList()->SetString(0, std::to_string(nRet));
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }
            return true;
        }
        else if (message_name == "on_signature_defaultcertificate")
        {
            // данные сертификата по умолчанию
            CCertificateInfo info = ICertificate::GetDefault();

            CJSONSimple serializer;
            serializer.Start();
            serializer.Write(L"name", info.GetName());
            serializer.Next();
            serializer.Write(L"id", info.GetId());
            serializer.Next();
            serializer.Write(L"date", info.GetDate());
            serializer.End();

            CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_signature_defaultcertificate_ret");
            messageOut->GetArgumentList()->SetString(0, serializer.GetData());
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            return true;
        }
        else if (message_name == "on_signature_selectsertificate")
        {
            // диалог выбора сертификата
            ICertificate* pCert = ICertificate::CreateInstance();

            WindowHandleId _handle = m_pParent->GetWidgetImpl()->cef_handle;
            int nShowDialogResult = pCert->ShowSelectDialog(&_handle);

            if (-1 == nShowDialogResult)
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE);
                pListener->OnEvent(pEvent);
                RELEASEOBJECT(pCert);
                return true;
            }

            CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_signature_selectsertificate_ret");
            if (1 == nShowDialogResult)
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

                messageOut->GetArgumentList()->SetString(0, serializer.GetData());
            }
            else
            {
                messageOut->GetArgumentList()->SetString(0, "{}");
            }
            RELEASEOBJECT(pCert);
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            return true;
        }
        else if (message_name == "on_open_filename_dialog")
        {
            // показать окно выбора файлов. по окончании вызовется коллбек js
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            NSEditorApi::CAscLocalOpenFileDialog* pData = new NSEditorApi::CAscLocalOpenFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_Filter(args->GetString(0).ToWString());
            m_pParent->m_pInternal->m_sIFrameIDMethod = args->GetString(1);

            if (message->GetArgumentList()->GetSize() > 2)
                pData->put_IsMultiselect(args->GetBool(2));

            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_save_filename_dialog")
        {
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG);
            NSEditorApi::CAscSaveDialog* pData = new NSEditorApi::CAscSaveDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_FilePath(args->GetString(0).ToWString());
            pData->put_IdDownload(0);
            pEvent->m_pData = pData;
            m_pParent->m_pInternal->m_sIFrameIDMethod = args->GetString(1);
            m_pParent->GetAppManager()->Apply(pEvent);
            return true;
        }
        else if (message_name == "on_file_save_question")
        {
            // при некоторых действиях необходимо сохранение (например подпись)
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
            NSEditorApi::CAscEditorSaveQuestion* pData = new NSEditorApi::CAscEditorSaveQuestion();
            pData->put_Id(m_pParent->GetId());
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
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
            pCreate->Url = args->GetString(0).ToWString();
            pCreate->LocalRecoverFolder = args->GetString(1).ToWString();
            pCreate->Version = args->GetString(2).ToString();

            if (args->GetSize() > 3)
                pCreate->CloudCryptoInfo = args->GetString(3).ToString();

            pData->put_Data(pCreate);

            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "on_end_reporter")
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_REPORTER_END);
            NSEditorApi::CAscTypeId* pData = new NSEditorApi::CAscTypeId();
            pData->put_Id(m_pParent->m_pInternal->m_nReporterChildId);
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            return true;
        }
        else if (message_name == "send_to_reporter")
        {
            // сообщение докладчику
            CCefView* pViewSend = m_pParent->GetAppManager()->GetViewById(m_pParent->m_pInternal->m_nReporterChildId);
            if (!pViewSend)
                return true;

            std::wstring sMessage = args->GetString(0).ToWString();

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO);
            NSEditorApi::CAscReporterMessage* pData = new NSEditorApi::CAscReporterMessage();
            pData->put_ReceiverId(m_pParent->m_pInternal->m_nReporterChildId);
            pData->put_Message(sMessage);
            pEvent->m_pData = pData;

            pViewSend->Apply(pEvent);
            return true;
        }
        else if (message_name == "send_from_reporter")
        {
            // сообщение от докладчика
            CCefView* pViewSend = m_pParent->GetAppManager()->GetViewById(m_pParent->m_pInternal->m_nReporterParentId);
            if (!pViewSend)
                return true;

            std::wstring sMessage = args->GetString(0).ToWString();

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM);
            NSEditorApi::CAscReporterMessage* pData = new NSEditorApi::CAscReporterMessage();
            pData->put_ReceiverId(m_pParent->m_pInternal->m_nReporterParentId);
            pData->put_Message(sMessage);
            pEvent->m_pData = pData;

            pViewSend->Apply(pEvent);
            return true;
        }        
        else if (message_name == "file_get_hash")
        {
            // нужно посчитать хэш облачного файла.
            // качаем, считаем, отдаем
            m_pParent->m_pInternal->m_pDownloadViewCallback = m_pParent;
            m_pParent->m_pInternal->m_sDownloadViewPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
            if (NSFile::CFileBinary::Exists(m_pParent->m_pInternal->m_sDownloadViewPath))
                NSFile::CFileBinary::Remove(m_pParent->m_pInternal->m_sDownloadViewPath);

            m_pParent->m_pInternal->m_sDownloadViewPath += (L".asc_file_get_hash");
            m_pParent->m_pInternal->m_sGetHashAlg = message->GetArgumentList()->GetString(1).ToString();
            m_pParent->m_pInternal->m_sGetHashFrame = message->GetArgumentList()->GetString(2).ToString();

            std::wstring sBaseUrl = m_pParent->GetUrl();
            std::wstring::size_type pos = NSCommon::FindLowerCase(sBaseUrl, L"/products/files");
            if (pos != std::wstring::npos)
                sBaseUrl = sBaseUrl.substr(0, pos);

            std::wstring sHashFileUrl = args->GetString(0);
            if ((0 != sHashFileUrl.find(L"www")) && (0 != sHashFileUrl.find(L"http")) && (0 != sHashFileUrl.find(L"ftp")))
                sHashFileUrl = sBaseUrl + sHashFileUrl;

            m_pParent->StartDownload(sHashFileUrl);
            return true;
        }
        else if (message_name == "send_system_message")
        {
            // системное сообщение (для external plugins)
            CCefView_Private::CSystemMessage sysMessage;
            sysMessage.ViewID = m_pParent->GetId();
            sysMessage.FrameID = args->GetString(1).ToString();
            sysMessage.Message = args->GetString(0).ToString();

            m_pParent->m_pInternal->SendSystemMessage(sysMessage);
            return true;
        }
        else if (message_name == "call_in_all_windows")
        {
            CAscApplicationManager_Private* pPrivate = m_pParent->GetAppManager()->m_pInternal;

            for (std::map<int, CCefView*>::iterator iterView = pPrivate->m_mapViews.begin(); iterView != pPrivate->m_mapViews.end(); iterView++)
            {
                CCefView* pTmp = iterView->second;
                CefRefPtr<CefBrowser> pBrowser = pTmp->m_pInternal->GetBrowser();
                if (pBrowser)
                {
                    // may be send to all frames?!
                    CefRefPtr<CefFrame> pFrame = pBrowser->GetMainFrame();
                    if (pFrame)
                    {
                        pFrame->ExecuteJavaScript(args->GetString(0), pFrame->GetURL(), 0);
                    }
                }
            }

            return true;
        }
        else if (message_name == "open_file_crypt")
        {
            // открытие облачного зашифрованного файла
            // 1) корректируем ссылку
            // 2) помечаем ссылку параметрами, чтобы созданный view понял, о чем идет речь (имя)
            std::wstring sName = args->GetString(0).ToWString();
            std::wstring sDownloadLink = args->GetString(1).ToWString();

            std::wstring sBaseUrl = m_pParent->GetUrl();
            std::wstring::size_type pos = NSCommon::FindLowerCase(sBaseUrl, L"/products/files");
            if (pos != std::wstring::npos)
                sBaseUrl = sBaseUrl.substr(0, pos);

            if (!m_pParent->m_pInternal->m_bIsExternalCloud)
            {
                if (0 != sDownloadLink.find(sBaseUrl) && !NSFileDownloader::IsNeedDownload(sDownloadLink))
                    sDownloadLink = sBaseUrl + sDownloadLink;

                std::wstring::size_type posCheckEnter = NSCommon::FindLowerCase(sDownloadLink, L"/products/files");
                if (posCheckEnter != std::wstring::npos)
                {
                    std::wstring sBaseDownloadLink = sDownloadLink.substr(0, posCheckEnter);
                    if (sBaseDownloadLink != sBaseUrl)
                    {
                        sDownloadLink = sBaseUrl + sDownloadLink.substr(posCheckEnter);
                    }
                }
            }
            else
            {
                // change base domains
                sBaseUrl = NSCommon::GetBaseDomain(sBaseUrl, true);
                std::wstring sBaseDownloadLink = NSCommon::GetBaseDomain(sDownloadLink, true);

                if (!sBaseUrl.empty() && sBaseUrl != sBaseDownloadLink)
                {
                    sDownloadLink = sBaseUrl + sDownloadLink.substr(sBaseDownloadLink.length());
                }
            }

            std::wstring sOpenUrl = sDownloadLink + L"<openaslocal>" + sName + L"</openaslocal>";

            ((CCefViewEditor*)m_pParent)->load(sOpenUrl);
            return true;
        }
        else if (message_name == "build_crypted")
        {
            // собрать зашифтованный файл
            m_pParent->m_pInternal->m_bIsBuilding = true;
            if (0 == args->GetSize())
                return true;

            std::wstring sPass = args->GetString(0).ToWString();
            std::wstring sDocInfo = args->GetString(1).ToWString();

            int nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
            if  (2 == m_pParent->m_pInternal->m_nEditorType)
                nType = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
            else if  (1 ==m_pParent->m_pInternal->m_nEditorType)
                nType = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;

            if (nType == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX && 0 != m_pParent->m_pInternal->m_nCloudCryptFileType)
                nType = m_pParent->m_pInternal->m_nCloudCryptFileType;

            m_pParent->m_pInternal->m_oLocalInfo.SetupOptions(m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo);
            m_pParent->m_pInternal->m_oConverterFromEditor.m_bIsEditorWithChanges = true;
            m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat = nType;
            m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_sFileSrc = m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_sRecoveryDir + L"/EditorCrypted.bin";
            m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_sPassword = sPass;
            m_pParent->m_pInternal->m_oConverterFromEditor.m_oInfo.m_sDocumentInfo = sDocInfo;
            m_pParent->m_pInternal->m_oConverterFromEditor.Start(0);
            return true;
        }
        else if (message_name == "build_crypted_end")
        {
            // окончание сборки. файл уже отправлен.
            // просто отправляем наверх
            m_pParent->m_pInternal->m_bIsBuilding = false;

            bool bIsNoError = args->GetBool(0);
            if (m_pParent->m_pInternal->m_bIsSavingCrypto)
            {
                m_pParent->m_pInternal->m_bIsSavingCrypto = false;
                m_pParent->m_pInternal->m_bIsSaving = false;

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);
                NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
                pData->put_Id(m_pParent->GetId());
                pEvent->m_pData = pData;
                pListener->OnEvent(pEvent);
            }
            else
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(bIsNoError ?
                    ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END : ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR);
                pListener->OnEvent(pEvent);
            }

            return true;
        }
        else if (message_name == "preload_crypto_image")
        {
            std::wstring sUrl1 = args->GetString(0).ToWString();
            std::wstring sUrl2 = args->GetString(1).ToWString();
            int_64_type nFrameID = NSArgumentList::GetInt64(args, 2);

            if (sUrl2.empty() && ((0 == sUrl1.find(L"image")) || (0 == sUrl1.find(L"display"))))
            {
                sUrl2 = L"media/" + sUrl1;
            }

            if (0 == sUrl2.find(L"media/"))
            {
                std::wstring sNum = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/" + sUrl2;
                if (NSFile::CFileBinary::Exists(sNum))
                {
                    CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_preload_crypto_image");
                    messageOut->GetArgumentList()->SetInt(0, 0);
                    NSArgumentList::SetInt64(messageOut->GetArgumentList(), 1, nFrameID);
                    messageOut->GetArgumentList()->SetString(2, sNum);
                    messageOut->GetArgumentList()->SetString(3, sUrl1);
                    SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
                }
                else
                {
                    if (m_pParent->m_pInternal->m_arCryptoImages.find(sUrl1) == m_pParent->m_pInternal->m_arCryptoImages.end())
                    {
                        m_pParent->m_pInternal->m_arCryptoImages.insert(std::pair<std::wstring, std::wstring>(sUrl1, sNum));
                        m_pParent->m_pInternal->m_arCryptoImagesFrameId.insert(std::pair<std::wstring, int_64_type>(sUrl1, nFrameID));
                        m_pParent->StartDownload(sUrl1);
                    }
                }
            }
            else
            {
                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("on_preload_crypto_image");
                messageOut->GetArgumentList()->SetInt(0, 1);
                NSArgumentList::SetInt64(messageOut->GetArgumentList(), 1, nFrameID);
                messageOut->GetArgumentList()->SetString(2, sUrl1);
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }

            return true;
        }
        else if (message_name == "download_files")
        {
            // скачать файлы.
            int nCount = args->GetSize();

            if (nCount == 0 || ((nCount & 0x01) == 0x01))
                return true;

            int nIndex = 0;
            int nParams = args->GetInt(nIndex++);
            int_64_type nFrameID = NSArgumentList::GetInt64(args, nIndex++);
            m_pParent->m_pInternal->m_nDownloadedFilesFrameId = nFrameID;

            while (nIndex < nCount)
            {
                std::wstring sUrl  = args->GetString(nIndex++);
                std::wstring sFile = args->GetString(nIndex++);

#ifdef WIN32
                NSStringUtils::string_replace(sFile, L"/", L"\\");
#endif

                m_pParent->m_pInternal->m_arDownloadedFiles.insert(std::pair<std::wstring, CDownloadFileItem>(GetUrlWithoutProtocol(sUrl), CDownloadFileItem(sUrl, sFile)));
            }

            if (nParams == 1)
            {
                CefRefPtr<CefFrame> frame = browser->GetFrame(nFrameID);
                if (frame)
                    frame->ExecuteJavaScript("window.onSystemMessage && window.onSystemMessage({ type : \"operation\", block : true, opType : 0 });", frame->GetURL(), 0);
            }

            for (std::map<std::wstring, CDownloadFileItem>::iterator iter = m_pParent->m_pInternal->m_arDownloadedFiles.begin();
                 iter != m_pParent->m_pInternal->m_arDownloadedFiles.end(); /*nothing*/)
            {
                // могут прийти локальные ссылки, или base64 (наприменр при вставке картинок в зашифрованный файл).
                // нужно просто имитировать скачку - просто сохранить файлы во временные
                bool isEmulate = false;
                std::wstring sEmulateLocal = iter->second.Url;
                if (0 == sEmulateLocal.find(L"file://"))
                {
                    std::wstring sFileLocal = sEmulateLocal.substr(7);
                    if (!NSFile::CFileBinary::Exists(sFileLocal))
                        sFileLocal = sFileLocal.substr(1);

                    NSFile::CFileBinary::Copy(sFileLocal, iter->second.Path);
                    isEmulate = true;
                }
                else if (0 == sEmulateLocal.find(L"data:"))
                {
                    std::wstring::size_type nBase64Start = sEmulateLocal.find(L"base64,");
                    if (nBase64Start != std::wstring::npos)
                    {
                        int nStartIndex = (int)nBase64Start + 7;
                        int nCount = (int)sEmulateLocal.length() - nStartIndex;

                        char* pData = new char[nCount];
                        const wchar_t* pDataSrc = sEmulateLocal.c_str();
                        for (int i = 0; i < nCount; ++i)
                            pData[i] = (char)pDataSrc[i + nStartIndex];

                        BYTE* pDataDecode = NULL;
                        int nLenDecode = 0;
                        NSFile::CBase64Converter::Decode(pData, nCount, pDataDecode, nLenDecode);

                        RELEASEARRAYOBJECTS(pData);

                        NSFile::CFileBinary oFile;
                        if (oFile.CreateFileW(iter->second.Path))
                        {
                            oFile.WriteFile(pDataDecode, (DWORD)nLenDecode);
                            oFile.CloseFile();
                        }

                        RELEASEARRAYOBJECTS(pDataDecode);
                    }
                    isEmulate = true;
                }

                if (isEmulate)
                {
                    m_pParent->m_pInternal->m_arDownloadedFilesComplete.insert(std::pair<std::wstring, std::wstring>(iter->second.Url, iter->second.Path));
                    m_pParent->m_pInternal->m_arDownloadedFiles.erase(iter++);

                    if (m_pParent->m_pInternal->m_arDownloadedFiles.empty())
                    {
                        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_download_files");

                        int nIndex = 0;
                        NSArgumentList::SetInt64(message->GetArgumentList(), nIndex++, m_pParent->m_pInternal->m_nDownloadedFilesFrameId);
                        m_pParent->m_pInternal->m_nDownloadedFilesFrameId = -1;

                        for (std::map<std::wstring, std::wstring>::iterator iterComplete = m_pParent->m_pInternal->m_arDownloadedFilesComplete.begin();
                             iterComplete != m_pParent->m_pInternal->m_arDownloadedFilesComplete.end(); iterComplete++)
                        {
                            message->GetArgumentList()->SetString(nIndex++, iterComplete->first);
                            message->GetArgumentList()->SetString(nIndex++, iterComplete->second);
                        }

                        if (!browser)
                            browser = m_pParent->m_pInternal->GetBrowser();
                        SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);

                        m_pParent->m_pInternal->m_arDownloadedFilesComplete.clear();
                        m_pParent->m_pInternal->m_arDownloadedFiles.clear();
                        break;
                    }
                    continue;
                }

                m_pParent->StartDownload(iter->second.Url);
                iter++;
            }            

            return true;
        }
        else if (message_name == "set_crypto_mode")
        {
            // выставить режим криптования.
            // если открыт хоть один редактор - не выставлять
            std::string sPass = args->GetString(0).ToString();
            int nMode = args->GetInt(1);
            bool bIsCallback = args->GetBool(2);
            int_64_type nFrameId = NSArgumentList::GetInt64(args, 3);

            if (!bIsCallback)
            {
                m_pParent->GetAppManager()->SetCryptoMode(sPass, nMode);
            }
            else
            {
                std::map<int, CCefView*>& mapViews = m_pParent->GetAppManager()->m_pInternal->m_mapViews;
                bool bIsEditorPresent = false;
                for (std::map<int, CCefView*>::iterator iter = mapViews.begin(); iter != mapViews.end(); iter++)
                {
                    CCefView* tmp = iter->second;
                    if (tmp->GetType() == cvwtEditor)
                    {
                        bIsEditorPresent = true;
                        break;
                    }
                    else
                    {
                        if (tmp->m_pInternal->m_pUploadFiles)
                        {
                            bIsEditorPresent = true;
                            break;
                        }
                    }
                }

                if (!bIsEditorPresent)
                    m_pParent->GetAppManager()->SetCryptoMode(sPass, nMode);

                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("set_crypto_mode");
                messageOut->GetArgumentList()->SetInt(0, bIsEditorPresent ? 1 : 0);
                NSArgumentList::SetInt64(messageOut->GetArgumentList(), 1, nFrameId);
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }

            return true;
        }
        else if (message_name == "get_advanced_encrypted_data")
        {
            m_pParent->m_pInternal->m_oTxtToDocx.m_nFrameId = NSArgumentList::GetInt64(args, 0);
            m_pParent->m_pInternal->m_oTxtToDocx.m_sPassword = args->GetString(1);
            m_pParent->m_pInternal->m_oTxtToDocx.ToData();

            return true;
        }
        else if (message_name == "set_advanced_encrypted_data")
        {
            m_pParent->m_pInternal->m_oTxtToDocx.m_nFrameId = NSArgumentList::GetInt64(args, 0);
            m_pParent->m_pInternal->m_oTxtToDocx.m_sPassword = args->GetString(1);
            m_pParent->m_pInternal->m_oTxtToDocx.m_sData = args->GetString(2);
            m_pParent->m_pInternal->m_oTxtToDocx.ToDocx();

            return true;
        }
        else if (message_name == "crypto_download_as")
        {
            int nFormat = args->GetInt(0);
            std::string sParams = args->GetString(1);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE);
            NSEditorApi::CAscLocalSaveFileDialog* pData = new NSEditorApi::CAscLocalSaveFileDialog();
            pData->put_Id(m_pParent->GetId());
            pData->put_Path(NSFile::GetFileName(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc));
            pData->put_FileType(m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat);
            if (nFormat == AVS_OFFICESTUDIO_FILE_OTHER_HTMLZIP)
                pData->get_SupportFormats().push_back(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML);
            else
                pData->get_SupportFormats().push_back(nFormat);
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
            m_pParent->m_pInternal->m_bIsSavingDialog = true;
            m_pParent->m_pInternal->m_nCryptoDownloadAsFormat = nFormat;
            m_pParent->m_pInternal->m_sCryptoDownloadAsParams = sParams;

            return true;
        }
        else if (message_name == "set_editors_version")
        {
            std::string sCloudVersion = args->GetString(0);
            m_pParent->m_pInternal->m_sCloudVersion = sCloudVersion;
            if ("undefined" == sCloudVersion)
                return true;

            int nCount = 3;
            int nValue = 0;
            std::string::size_type pos = sCloudVersion.find(".");
            while (nCount > 0 && pos != std::string::npos)
            {
                std::string sTmp = sCloudVersion.substr(0, pos);
                int nTmp = 0;

                try
                {
                    nTmp = std::stoi(sTmp);
                }
                catch (...)
                {
                    nTmp = 0;
                }

                for (int i = 0; i < nCount; i++)
                    nTmp *= 100;

                nValue += nTmp;

                --nCount;

                if (sCloudVersion.length() == pos)
                    break;

                sCloudVersion = sCloudVersion.substr(pos + 1);
                pos = sCloudVersion.find(".");
                if (pos == std::string::npos && sCloudVersion.length() != 0)
                    pos = sCloudVersion.length();
            }

            m_pParent->m_pInternal->m_nCloudVersion = nValue;

            // debug version
            if ("{{PRODUCT_VERSION}}" == sCloudVersion)
                m_pParent->m_pInternal->m_nCloudVersion = CRYPTO_CLOUD_SUPPORT;

            std::wstring sMainFrameUrl = L"file://";
            if (browser->GetMainFrame())
                sMainFrameUrl = browser->GetMainFrame()->GetURL();

            bool bIsCryptoSupport = true;
            if (m_pParent->m_pInternal->m_bIsExternalCloud)
            {
                bIsCryptoSupport = m_pParent->m_pInternal->m_oExternalCloud.crypto_support;
            }
            if (bIsCryptoSupport && NSFileDownloader::IsNeedDownload(sMainFrameUrl) && m_pParent->m_pInternal->m_nCloudVersion < CRYPTO_CLOUD_SUPPORT)
            {
                CefRefPtr<CefProcessMessage> messageOut = CefProcessMessage::Create("set_editors_version_no_crypto");
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, messageOut);
            }
            return true;
        }
        else if ("media_start" == message_name)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START);
            NSEditorApi::CAscExternalMedia* pData = new NSEditorApi::CAscExternalMedia();
            pEvent->m_pData = pData;

            Aggplus::CMatrix oTransform;

            std::wstring sPath = (message->GetArgumentList()->GetString(0).ToWString());
            if (!NSFile::CFileBinary::Exists(sPath))
                sPath = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/media/" + sPath;

            pData->put_Url(sPath);

            int nX = args->GetInt(1);
            int nY = args->GetInt(2);
            double dExtW = args->GetDouble(3);
            double dExtH = args->GetDouble(4);

            double dScale = args->GetDouble(5);

            if (args->GetSize() > 6)
            {
                oTransform.SetElements(
                            args->GetDouble(6),
                            args->GetDouble(7),
                            args->GetDouble(8),
                            args->GetDouble(9),
                            args->GetDouble(10),
                            args->GetDouble(11));
            }

            double x1 = 0; double y1 = 0;
            double x2 = dExtW; double y2 = 0;
            double x3 = dExtW; double y3 = dExtH;
            double x4 = 0; double y4 = dExtH;

            oTransform.TransformPoint(x1, y1);
            oTransform.TransformPoint(x2, y2);
            oTransform.TransformPoint(x3, y3);
            oTransform.TransformPoint(x4, y4);

            double boundsX = x1;
            double boundsY = y1;
            double boundsR = x1;
            double boundsB = y1;

            if (x2 < boundsX) boundsX = x2;
            if (x3 < boundsX) boundsX = x3;
            if (x4 < boundsX) boundsX = x4;

            if (x2 > boundsR) boundsR = x2;
            if (x3 > boundsR) boundsR = x3;
            if (x4 > boundsR) boundsR = x4;

            if (y2 < boundsY) boundsY = y2;
            if (y3 < boundsY) boundsY = y3;
            if (y4 < boundsY) boundsY = y4;

            if (y2 > boundsB) boundsB = y2;
            if (y3 > boundsB) boundsB = y3;
            if (y4 > boundsB) boundsB = y4;

            pData->put_X(nX);
            pData->put_Y(nY);
            pData->put_W(dExtW);
            pData->put_H(dExtH);

            double dKoef = (double)m_pParent->GetDeviceScale();

            int nBoundsX = (int)(boundsX * dScale);
            int nBoundsY = (int)(boundsY * dScale);
            int nBoundsR = (int)(boundsR * dScale + 0.9);
            int nBoundsB = (int)(boundsB * dScale + 0.9);

            pData->put_BoundsX((int)(dKoef * (nX + nBoundsX)));
            pData->put_BoundsY((int)(dKoef * (nY + nBoundsY)));
            pData->put_BoundsW((int)(dKoef * (nBoundsR - nBoundsX + 1)));
            pData->put_BoundsH((int)(dKoef * (nBoundsB - nBoundsY + 1)));

            pListener->OnEvent(pEvent);
            return true;
        }
        else if ("media_end" == message_name)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_END);
            pListener->OnEvent(pEvent);
            return true;
        }
        else if ("send_by_mail" == message_name)
        {
            CefRefPtr<CefFrame> frameMain = browser->GetMainFrame();
            if (!frameMain)
                return true;

            std::wstring sUrl = m_pParent->GetUrl();
            std::wstring sMailApp = L"";
            std::wstring sMailAppType = L"";
            if (NSFileDownloader::IsNeedDownload(sUrl))
            {
                std::string sCode = "window.open(\"mailto:?subject=";
                sCode += "File";
                sCode += "&body=";

                std::wstring::size_type posQ = sUrl.find(L"?");
                if (std::wstring::npos != posQ)
                    sUrl = sUrl.substr(0, posQ);

                sCode += U_TO_UTF8(sUrl);
                sCode += "\");";

                frameMain->ExecuteJavaScript(sCode, frameMain->GetURL(), 0);
                return true;
            }
            else
            {
                std::wstring sUrlFile = L"";
                if (m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved)
                    sUrlFile = m_pParent->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;

                sMailApp = CMailToCommandLine::GetMailApp(sMailAppType);

                if (!sMailApp.empty())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PROCESS);
                    NSEditorApi::CAscExternalProcess* pData = new NSEditorApi::CAscExternalProcess();
                    pEvent->m_pData = pData;

                    pData->put_Program(sMailApp);
                    pData->put_Detached(true);

                    CMailToCommandLine::GetArguments(pData->get_Arguments(), sMailAppType, sUrlFile);

                    pListener->OnEvent(pEvent);
                }
                else
                {
#ifdef _MAC
                    std::string sCodeApp = "open -a Mail";
                    if (!sUrlFile.empty())
                    {
                        sCodeApp += " ";

                        std::string sUrlFileA = U_TO_UTF8(sUrlFile);
                        NSStringUtils::string_replaceA(sUrlFileA, "\\", "\\\\");
                        NSStringUtils::string_replaceA(sUrlFileA, " ", "\\ ");
                        NSStringUtils::string_replaceA(sUrlFileA, "<", "\\<");
                        NSStringUtils::string_replaceA(sUrlFileA, ">", "\\>");
                        NSStringUtils::string_replaceA(sUrlFileA, "(", "\\(");
                        NSStringUtils::string_replaceA(sUrlFileA, ")", "\\)");
                        NSStringUtils::string_replaceA(sUrlFileA, "[", "\\[");
                        NSStringUtils::string_replaceA(sUrlFileA, "]", "\\]");
                        NSStringUtils::string_replaceA(sUrlFileA, "?", "\\?");
                        NSStringUtils::string_replaceA(sUrlFileA, "!", "\\!");
                        NSStringUtils::string_replaceA(sUrlFileA, "'", "\\'");
                        NSStringUtils::string_replaceA(sUrlFileA, "\"", "\\\"");
                        NSStringUtils::string_replaceA(sUrlFileA, "|", "\\|");
                        NSStringUtils::string_replaceA(sUrlFileA, "&", "\\&");

                        sCodeApp += sUrlFileA;
                        sCodeApp += "";
                    }
                    system(sCodeApp.c_str());
                    return true;
#endif
                    CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
                    if (_frame)
                    {
                        std::wstring sCode = L"(function(){ \n\
var _e = undefined;\n\
if (window[\"Asc\"] && window[\"Asc\"][\"editor\"]) \n\
{\n\
_e = window[\"Asc\"][\"editor\"];\n\
}\n\
else if (window[\"editor\"])\n\
{\n\
_e = window[\"editor\"];\n\
}\n\
if (!_e) return;\n\
_e.sendEvent(\"asc_onError\", -452, 0);\n\
})();";

                        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
                    }
                }
            }

            return true;
        }
        else if ("cloud_crypto_upload" == message_name)
        {
            if (m_pParent->m_pInternal->m_pUploadFiles)
                return true;

            m_pParent->m_pInternal->m_pUploadFiles = new CCefView_Private::CCloudCryptoUpload();
            m_pParent->m_pInternal->m_pUploadFiles->IsRemove = args->GetBool(0);
            m_pParent->m_pInternal->m_pUploadFiles->FrameID = NSArgumentList::GetInt64(args, 1);
            m_pParent->m_pInternal->m_pUploadFiles->View = m_pParent->m_pInternal;
            m_pParent->m_pInternal->m_pUploadFiles->Manager = m_pParent->GetAppManager();
            int nCount = args->GetInt(2);
            for (int i = 0; i < nCount; ++i)
                m_pParent->m_pInternal->m_pUploadFiles->Files.push_back(args->GetString(3 + i));

            m_pParent->m_pInternal->m_pUploadFiles->Init();
            return true;
        }
        else if ("cloud_crypto_upload_pass" == message_name)
        {
            if (m_pParent->m_pInternal->m_pUploadFiles)
                m_pParent->m_pInternal->m_pUploadFiles->OnPassword(args->GetString(0).ToWString(), args->GetString(1).ToWString());
            return true;
        }
        else if ("cloud_crypto_upload_save" == message_name)
        {
            if (m_pParent->m_pInternal->m_pUploadFiles)
                m_pParent->m_pInternal->m_pUploadFiles->OnSavePassword();
            return true;
        }
        else if ("cloud_crypto_upload_end" == message_name)
        {
            if (m_pParent->m_pInternal->m_pUploadFiles)
                m_pParent->m_pInternal->m_pUploadFiles->OnSend();
            return true;
        }
        else if ("on_compare_document" == message_name)
        {
            std::wstring sType = args->GetString(0).ToWString();
            std::wstring sFile_Url = args->GetString(1).ToWString();
            std::wstring sUrl = L"ascdesktop://compare/" + std::to_wstring(m_pParent->GetId()) + L"/" + sType + L"/" + sFile_Url;

            NSEditorApi::CAscCreateTab* pData = new NSEditorApi::CAscCreateTab();
            pData->put_Url(sUrl);

            CRecentParent oRecentParent;
            oRecentParent.Url = sUrl;
            oRecentParent.Parent = m_pParent->m_pInternal->GetBrowser()->GetMainFrame()->GetURL().ToWString();

            m_pParent->GetAppManager()->m_pInternal->m_arRecentParents.push_back(oRecentParent);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_CREATETAB);
            pEvent->m_pData = pData;

            pListener->OnEvent(pEvent);
            return true;
        }
        else if ("on_document_content_ready" == message_name)
        {
            if (!m_pParent->m_pInternal->m_sComparingFile.empty())
            {
                m_pParent->m_pInternal->m_oConverterToEditor.m_sComparingFile = m_pParent->m_pInternal->m_sComparingFile;
                m_pParent->m_pInternal->LocalFile_Start();
            }

            return true;
        }
        else if ("on_set_is_readonly" == message_name)
        {
            return true;
        }
        else if ("on_need_update_app" == message_name)
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_ON_NEED_UPDATE_APP);
            pEvent->m_pData = new NSEditorApi::CAscUpdateApp();
            pListener->OnEvent(pEvent);
            return true;
        }
        else if ("get_current_window_info" == message_name)
        {
            CefRefPtr<CefBrowserHost> _host = m_pParent->m_pInternal->GetBrowser()->GetHost();
            CefWindowHandle hwnd = _host->GetWindowHandle();

            unsigned int _dx = 0;
            unsigned int _dy = 0;
            double dDeviceScale = 1;
        #ifdef WIN32
            Core_GetMonitorRawDpi(hwnd, &_dx, &_dy);
            dDeviceScale = Core_GetMonitorScale(_dx, _dy);
        #else
            int nDeviceScaleTmp = CAscApplicationManager::GetDpiChecker()->GetWidgetImplDpi(m_pParent->GetWidgetImpl(), &_dx, &_dy);
            dDeviceScale = CAscApplicationManager::GetDpiChecker()->GetScale(_dx, _dy);
        #endif

            std::string sCode = "console.log(\"window [" + std::to_string(_dx) + ", " + std::to_string(_dx) + "]: " + std::to_string(dDeviceScale) + "\");";

            CefRefPtr<CefFrame> frame = m_pParent->m_pInternal->GetBrowser()->GetMainFrame();
            if (frame)
                frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);

            return true;
        }

        CAscApplicationManager_Private* pInternalMan = m_pParent->GetAppManager()->m_pInternal;
        if (pInternalMan->m_pAdditional && pInternalMan->m_pAdditional->OnProcessMessageReceived(browser, source_process, message, m_pParent))
            return true;

        if (message_router_->OnProcessMessageReceived(browser,
                                              #ifndef MESSAGE_IN_BROWSER
                                                      frame,
                                              #endif
                                                      source_process, message))
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

        CCefViewWidgetImpl::SetParentNull(browser->GetHost()->GetWindowHandle());

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }


    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                             bool isLoading,
                                             bool canGoBack,
                                             bool canGoForward)
    {
        client::ClientHandler::OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);

        if (!isLoading)
        {
            // вот тут уже можно делать зум!!!
            m_pParent->m_pInternal->m_bIsWindowsCheckZoom = true;
            m_pParent->m_pInternal->m_dDeviceScale = -1;
            m_pParent->resizeEvent();
        }
    }

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame
                         #ifndef CEF_2623
                             , TransitionType transition_type
                         #endif
                             )
    {
        // вот тут уже можно делать зум!!!
        m_pParent->m_pInternal->m_bIsWindowsCheckZoom = true;
        m_pParent->m_pInternal->m_dDeviceScale = -1;
        m_pParent->resizeEvent();
    }


    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode)
    {
        bool bIsCryptoSupport = true;
        if (m_pParent->m_pInternal->m_bIsExternalCloud)
        {
            bIsCryptoSupport = m_pParent->m_pInternal->m_oExternalCloud.crypto_support;
        }

        std::wstring sMainFrameUrl = L"file://";
        if (browser && browser->GetMainFrame())
            sMainFrameUrl = browser->GetMainFrame()->GetURL();

        if (frame && bIsCryptoSupport)
        {
            if (!NSFileDownloader::IsNeedDownload(sMainFrameUrl) || (m_pParent->m_pInternal->m_nCloudVersion >= CRYPTO_CLOUD_SUPPORT))
                m_pParent->GetAppManager()->m_pInternal->SendCryptoData(frame);
        }

        if (frame->IsMain())
        {
            std::wstring sUrl = m_pParent->GetUrl();
            if ((0 != sUrl.find(L"file:///")) || !m_pParent->m_pInternal->m_bIsOnlyPassSupport || m_pParent->GetType() == cvwtEditor)
                return;

            // это стартовая страница. отправляем external плагины
            std::vector<CExternalPluginInfo>& arPluginsExternal = m_pParent->m_pInternal->m_pManager->m_pInternal->m_arExternalPlugins;
            std::wstring sSystemPluginsPath = m_pParent->GetAppManager()->m_oSettings.system_plugins_path;
            std::wstring sUserPluginsPath = m_pParent->GetAppManager()->m_oSettings.user_plugins_path;

            if (arPluginsExternal.size() != 0 && NULL != m_pParent->GetAppManager()->GetEventListener())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS);
                NSEditorApi::CAscSystemExternalPlugins* pEventData = new NSEditorApi::CAscSystemExternalPlugins();
                pEvent->m_pData = pEventData;

                for (std::vector<CExternalPluginInfo>::iterator iterExt = arPluginsExternal.begin(); iterExt != arPluginsExternal.end(); iterExt++)
                {
                    std::string sGuidA = iterExt->sGuid;
                    if (0 == sGuidA.find("asc."))
                        sGuidA = sGuidA.substr(4);
                    std::wstring sGuid = UTF8_TO_U(sGuidA);

                    std::wstring sSrc = (iterExt->isUser ? sUserPluginsPath : sSystemPluginsPath) + L"/" + sGuid + L"/index.html" + m_pParent->m_pInternal->m_pManager->m_pInternal->m_mainPostFix;
                    NSCommon::url_correct(sSrc);

                    std::wstring sNameG = UTF8_TO_U((iterExt->sName));
                    std::wstring sNameLocal = UTF8_TO_U((iterExt->sNameObject));
                    pEventData->addItem(sNameG, L"system_asc." + sGuid, sSrc, sNameLocal);
                }

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
            }
        }
    }

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) OVERRIDE
    {
        if (m_pParent && errorCode != ERR_ABORTED)
        {
            m_pParent->load(L"ascdesktop://loaderror.html");
            m_pParent->m_pInternal->m_bIsLoadingError = true;
        }
    }

    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();
        
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

#ifndef CEF_2623
        if (delegate_)
          delegate_->OnBeforeContextMenu(model);
#endif
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
            
            if (((nMods & EVENTFLAG_CONTROL_DOWN) != 0) && event.windows_key_code == 9)
                return true; // tab!!!
            if (((nMods & EVENTFLAG_ALT_DOWN) != 0) && event.windows_key_code == 115)
                return true; // alt + f4!!!
        }

        return false;
    }

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) OVERRIDE
    {
        std::wstring url = request->GetURL().ToWString();

#ifdef USE_STREAM_RESOURCE_RECOVER_CORS
        if (0 == url.find(L"ascdesktop://fonts"))
        {
            std::wstring sUrlFrame = frame->GetURL().ToWString();
            if (NSFileDownloader::IsNeedDownload(sUrlFrame))
            {
                int nFlag = UU_SPACES | UU_REPLACE_PLUS_WITH_SPACE;
#if defined (_LINUX) && !defined(_MAC)
                nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#else
#ifndef CEF_2623
                nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#endif
#endif

                CefString cefUrl = request->GetURL();
                CefString cefFile = CefURIDecode(cefUrl, false, static_cast<cef_uri_unescape_rule_t>(nFlag));

                std::wstring sBinaryFile = cefFile.ToWString().substr(19);
                bool bIsCheck = true;
                if (0 == sBinaryFile.find(L"fonts_thumbnail"))
                {
                    sBinaryFile = (m_pParent->GetAppManager()->m_oSettings.fonts_cache_info_path + L"/" + NSFile::GetFileName(sBinaryFile));
                    while (!NSFile::CFileBinary::Exists(sBinaryFile) && m_pParent->GetAppManager()->m_pInternal->IsRunned())
                        NSThreads::Sleep(100);
                    bIsCheck = false;
                }

                if (bIsCheck && !m_pParent->GetAppManager()->IsResolveLocalFile(sBinaryFile))
                    return NULL;

                // check on recovery folder!!!
                return GetLocalFileRequest2(sBinaryFile);
            }
        }
#endif

#ifdef NO_CACHE_WEB_CLOUD_SCRIPTS
        if (std::wstring::npos != url.find(L"sdk/Common/AllFonts.js") ||
            std::wstring::npos != url.find(L"sdkjs/common/AllFonts.js"))
        {
            while (!m_pParent->GetAppManager()->IsInitFonts() && m_pParent->GetAppManager()->m_pInternal->IsRunned())
                NSThreads::Sleep( 10 );

            std::wstring sPathFonts = m_pParent->GetAppManager()->m_oSettings.fonts_cache_info_path + L"/AllFonts.js";

            if (true)
            {
                int nMajorVersion = NSVersion::GetMajorVersion(m_pParent->m_pInternal->m_sVersionForReporter);
                if (nMajorVersion != 0 && nMajorVersion < 6)
                {
                    sPathFonts += L".1";
                }
            }

            return GetLocalFileRequest(sPathFonts, "", "");
        }
#endif

#ifdef DEBUG_LOCAL_SERVER

        // local files check
        if (std::string::npos == frame->GetURL().ToString().find("file://"))
        {
            std::wstring sResourcePath = L"";

            std::wstring::size_type pos = url.find(L"/sdkjs/");
            if (std::wstring::npos != pos)
            {
                sResourcePath = NSFile::GetProcessDirectory() + L"/web" + url.substr(pos);
            }
            else if (std::wstring::npos != (pos = url.find(L"/web-apps/")))
            {
                sResourcePath = NSFile::GetProcessDirectory() + L"/web" + url.substr(pos);
            }

            if (!sResourcePath.empty() &&
                std::wstring::npos == sResourcePath.find(L"AllFonts.js") &&
                std::wstring::npos == sResourcePath.find(L"require.js") &&
                NSFile::CFileBinary::Exists(sResourcePath))
            {
                return GetLocalFileRequest(sResourcePath, "window.compareVersions=true;", "");
            }
        }

#endif

        if (url.find(L"require.js") != std::wstring::npos)
        {
            std::string sAppData = ("(function() { \"file:\"==window.location.protocol&&(window.fetch=function(r){return new Promise(function(e,t){var o=new XMLHttpRequest;o.open(\"GET\",r,!0),o.overrideMimeType?o.overrideMimeType(\"text/plain; charset=utf-8\"):o.setRequestHeader(\"Accept-Charset\",\"utf-8\"),o.onload=function(){4!=o.readyState||200!=o.status&&0!=location.href.indexOf(\"file:\")||e({status:200,statusText:o.statusText,value:o.response,ok:!0,json:function(){return Promise.resolve(JSON.parse(this.value))},text:function(){return Promise.resolve(this.value)}})},o.onerror=function(){t(new TypeError(\"Network request failed\"))},o.send(null)})}); })();\n");

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

            std::wstring sPathToEditors = NSFile::GetDirectoryName(m_pParent->GetAppManager()->m_oSettings.local_editors_path);
            sPathToEditors += L"/../../../vendor/requirejs/require.js";
            return GetLocalFileRequest(sPathToEditors, sAppData, sFooter);
        }

#ifndef NO_CACHE_WEB_CLOUD_SCRIPTS
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
#endif

        return NULL;
    }

    CefRefPtr<CefResourceHandler> GetLocalFileRequest2(std::wstring& strFileName)
    {
        BYTE* fileData = NULL;
        DWORD fileSize = (DWORD)asc_scheme::read_file_with_urls(strFileName, fileData);

        if (!fileData)
            return NULL;

        std::string  mime_type = asc_scheme::GetMimeTypeFromExt(strFileName);

        CCefBinaryFileReaderCounter* pCounter = new CCefBinaryFileReaderCounter(fileData);
        CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForHandler(new CefByteReadHandler(fileData, fileSize, pCounter));
        if (stream.get())
            return new CefStreamResourceHandlerCORS(mime_type, stream);

        pCounter->data = NULL;
        RELEASEARRAYOBJECTS(fileData);
        return NULL;

    }
    CefRefPtr<CefResourceHandler> GetLocalFileRequest(const std::wstring& strFileName, const std::string& sHeaderScript = "", const std::string& sFooter = "", const bool& isSchemeRequest = false)
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
        if (isSchemeRequest)
            mime_type = asc_scheme::GetMimeTypeFromExt(strFileName);

        if (mime_type.empty())
        {
            RELEASEARRAYOBJECTS(pBytes);
            return NULL;
        }

        CCefBinaryFileReaderCounter* pCounter = new CCefBinaryFileReaderCounter(pBytes);
        CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForHandler(new CefByteReadHandler(pBytes, dwOffset + dwSize + dwFooterSize, pCounter));
        if (stream.get())
        {
            if (isSchemeRequest)
                return new CefStreamResourceHandlerCORS(mime_type, stream);

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

        if (L"1" == sExt && std::wstring::npos != sFile.find(L"AllFonts.js.1"))
            return "application/javascript";
        else if (L"2" == sExt && std::wstring::npos != sFile.find(L"AllFonts.js.2"))
            return "application/javascript";

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

        if (m_pParent && m_pParent->m_pInternal)
            m_pParent->m_pInternal->m_bIsCrashed = true;
    }

    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback)
    {
        CEF_REQUIRE_UI_THREAD();

        std::wstring sUrlOriginal = download_item->GetOriginalUrl().ToWString();
        std::wstring sUrlOriginalWithoutProtocol = GetUrlWithoutProtocol(sUrlOriginal);

        std::wstring sUrl = download_item->GetURL().ToWString();
        std::wstring sUrlWithoutProtocol = GetUrlWithoutProtocol(sUrl);
        //m_pParent->m_pInternal->m_oDownloaderAbortChecker.EndDownload(sUrl);

        std::wstring sDestPath = L"";

        // если указан m_pDownloadViewCallback - то уже все готово. просто продолжаем
        if (NULL != m_pParent->m_pInternal->m_pDownloadViewCallback)
        {
            sDestPath = m_pParent->m_pInternal->m_pDownloadViewCallback->m_pInternal->GetViewDownloadPath();
        }

        if (sDestPath.empty())
        {
            // если ссылка приватная (внутренняя) - то уже все готово. просто продолжаем
            std::wstring sPrivateDownloadUrl = m_pParent->GetAppManager()->m_pInternal->GetPrivateDownloadUrl();
            if (sUrlOriginalWithoutProtocol == GetUrlWithoutProtocol(sPrivateDownloadUrl))
            {
                sDestPath = m_pParent->GetAppManager()->m_pInternal->GetPrivateDownloadPath();
            }
        }

        if (sDestPath.empty())
        {
            // проверяем на зашифрованные картинки
            std::map<std::wstring, std::wstring>::iterator findCryptoImage = m_pParent->m_pInternal->m_arCryptoImages.find(sUrlOriginal);
            if (findCryptoImage != m_pParent->m_pInternal->m_arCryptoImages.end())
            {
                sDestPath = findCryptoImage->second;
            }
        }

        if (sDestPath.empty())
        {
            // проверяем на файлы метода AscDesktopEditor.DownloadFiles
            std::map<std::wstring, CDownloadFileItem>::iterator findDownloadFile = m_pParent->m_pInternal->m_arDownloadedFiles.find(sUrlOriginalWithoutProtocol);
            if (findDownloadFile != m_pParent->m_pInternal->m_arDownloadedFiles.end())
            {
                sDestPath = findDownloadFile->second.Path;
            }
        }

        if (!sDestPath.empty())
        {
#ifdef _WIN32
            NSStringUtils::string_replace(sDestPath, L"/", L"\\");
#endif
            callback->Continue(sDestPath, false);
            return;
        }
        
        // скачивание, которое надо отправлять наверх
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

#if 0
        FILE* f = fopen("...", "a+");

        fprintf(f, "-------------------------------\n");
        fprintf(f, "IsValid: %d\n", download_item->IsValid() ? 1 : 0);
        fprintf(f, "IsInProgress: %d\n", download_item->IsInProgress() ? 1 : 0);
        fprintf(f, "IsComplete: %d\n", download_item->IsComplete() ? 1 : 0);
        fprintf(f, "IsCanceled: %d\n", download_item->IsCanceled() ? 1 : 0);

        fprintf(f, "GetCurrentSpeed: %d\n", (int)download_item->GetCurrentSpeed());
        fprintf(f, "GetPercentComplete: %d\n", (int)download_item->GetPercentComplete());
        fprintf(f, "GetTotalBytes: %d\n", (int)download_item->GetTotalBytes());
        fprintf(f, "GetReceivedBytes: %d\n", (int)download_item->GetReceivedBytes());
        fprintf(f, "GetId: %d\n", (int)download_item->GetId());

        if (!download_item->GetFullPath().empty())
        {
            std::string s = download_item->GetFullPath().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetFullPath: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }
        if (!download_item->GetOriginalUrl().empty())
        {
            std::string s = download_item->GetOriginalUrl().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetOriginalUrl: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }
        if (!download_item->GetURL().empty())
        {
            std::string s = download_item->GetURL().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetURL: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }
        if (!download_item->GetSuggestedFileName().empty())
        {
            std::string s = download_item->GetSuggestedFileName().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetSuggestedFileName: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }
        if (!download_item->GetContentDisposition().empty())
        {
            std::string s = download_item->GetContentDisposition().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetContentDisposition: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }
        if (!download_item->GetMimeType().empty())
        {
            std::string s = download_item->GetMimeType().ToString();
            NSStringUtils::string_replaceA(s, "%", "%%");
            fprintf(f, "GetMimeType: ");
            fprintf(f, s.c_str());
            fprintf(f, "\n");
        }

        fclose(f);
#endif

        if (!download_item->IsValid())
            return;

        std::wstring sUrlOriginal = download_item->GetOriginalUrl().ToWString();
        std::wstring sUrlOriginalWithoutProtocol = GetUrlWithoutProtocol(sUrlOriginal);

        std::wstring sUrl = download_item->GetURL().ToWString();
        std::wstring sUrlWithoutProtocol = GetUrlWithoutProtocol(sUrl);

        std::wstring sPath = download_item->GetFullPath().ToWString();

        if (download_item->IsInProgress() || download_item->IsCanceled() || download_item->IsComplete() || (0 != download_item->GetReceivedBytes()))
            m_pParent->m_pInternal->m_oDownloaderAbortChecker.EndDownload(sUrl);

        // проверяем на m_pDownloadViewCallback
        if (NULL != m_pParent->m_pInternal->m_pDownloadViewCallback)
        {
            if (download_item->IsComplete())
            {
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE);
                NSEditorApi::CAscUIThreadMessage* pData = new NSEditorApi::CAscUIThreadMessage();
                pData->put_Type(0);
                pEvent->m_pData = pData;

                m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);

                // OpenLocalFile нужно запускать в главном потоке
                //m_pParent->m_pInternal->m_pDownloadViewCallback->m_pInternal->OnViewDownloadFile();
                //m_pParent->m_pInternal->m_pDownloadViewCallback = NULL;
            }
            return;
        }

        // проверяем на зашифрованные картинки
        if (!m_pParent->m_pInternal->m_arCryptoImages.empty())
        {
            std::map<std::wstring, std::wstring>::iterator findCryptoImage = m_pParent->m_pInternal->m_arCryptoImages.find(sUrlOriginal);
            if (findCryptoImage != m_pParent->m_pInternal->m_arCryptoImages.end())
            {
                if (download_item->IsComplete() || download_item->IsCanceled())
                {
                    int_64_type nFrameID = 0;
                    std::map<std::wstring, int_64_type>::iterator findCryptoImageFrameId = m_pParent->m_pInternal->m_arCryptoImagesFrameId.find(sUrlOriginal);
                    if (findCryptoImageFrameId !=  m_pParent->m_pInternal->m_arCryptoImagesFrameId.end())
                    {
                        nFrameID = findCryptoImageFrameId->second;
                        m_pParent->m_pInternal->m_arCryptoImagesFrameId.erase(findCryptoImageFrameId);
                    }

                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_preload_crypto_image");
                    message->GetArgumentList()->SetInt(0, 0);
                    NSArgumentList::SetInt64(message->GetArgumentList(), 1, nFrameID);
                    message->GetArgumentList()->SetString(2, findCryptoImage->second);
                    message->GetArgumentList()->SetString(3, findCryptoImage->first);

                    if (!browser)
                        browser = m_pParent->m_pInternal->GetBrowser();
                    SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);

                    m_pParent->m_pInternal->m_arCryptoImages.erase(findCryptoImage);
                }
                return;
            }
        }

        // проверяем на файлы метода AscDesktopEditor.DownloadFiles
        if (!m_pParent->m_pInternal->m_arDownloadedFiles.empty())
        {
            std::map<std::wstring, CDownloadFileItem>::iterator findDownloadFile = m_pParent->m_pInternal->m_arDownloadedFiles.find(sUrlOriginalWithoutProtocol);
            if (findDownloadFile != m_pParent->m_pInternal->m_arDownloadedFiles.end())
            {
                if (download_item->IsComplete() || download_item->IsCanceled())
                {
                    m_pParent->m_pInternal->m_arDownloadedFilesComplete.insert(
                                std::pair<std::wstring, std::wstring>(findDownloadFile->second.Url, findDownloadFile->second.Path));

                    m_pParent->m_pInternal->m_arDownloadedFiles.erase(findDownloadFile);

                    if (m_pParent->m_pInternal->m_arDownloadedFiles.empty())
                    {
                        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_download_files");

                        int nIndex = 0;
                        NSArgumentList::SetInt64(message->GetArgumentList(), nIndex++, m_pParent->m_pInternal->m_nDownloadedFilesFrameId);
                        m_pParent->m_pInternal->m_nDownloadedFilesFrameId = -1;

                        for (std::map<std::wstring, std::wstring>::iterator iter = m_pParent->m_pInternal->m_arDownloadedFilesComplete.begin();
                             iter != m_pParent->m_pInternal->m_arDownloadedFilesComplete.end(); iter++)
                        {
                            message->GetArgumentList()->SetString(nIndex++, iter->first);
                            message->GetArgumentList()->SetString(nIndex++, iter->second);
                        }

                        if (!browser)
                            browser = m_pParent->m_pInternal->GetBrowser();
                        SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);

                        m_pParent->m_pInternal->m_arDownloadedFilesComplete.clear();
                        m_pParent->m_pInternal->m_arDownloadedFiles.clear();
                    }
                }
                return;
            }
        }

        // интерфейсные скачивания
        unsigned int uId = (unsigned int)download_item->GetId();

        NSEditorApi::CAscDownloadFileInfo* pData = new NSEditorApi::CAscDownloadFileInfo();
        pData->put_Url(sUrl);
        pData->put_FilePath(sPath);
        pData->put_Percent(download_item->GetPercentComplete());
        pData->put_IsComplete(false);
        pData->put_Id(m_pParent->GetId());
        pData->put_Speed(download_item->GetCurrentSpeed() / 1024.0);
        pData->put_IdDownload(uId);

        if (m_pParent->m_pInternal->m_before_callback)
        {
            if (m_pParent->m_pInternal->m_before_callback_info)
                m_pParent->m_pInternal->m_before_callback_info->Release();
            m_pParent->m_pInternal->m_before_callback_info = pData->Copy();
            m_pParent->m_pInternal->m_before_callback_info->put_IsCanceled(true);
            m_pParent->m_pInternal->m_before_callback_info->put_IsComplete(true);
        }

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
            pData->put_IsComplete(true);

        m_pParent->GetAppManager()->Apply(pEvent);
    }

#ifndef CEF_2623
    void OnGotFocus(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_PAGE_GOT_FOCUS);
        m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
    }
#endif

    bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefDragData> dragData,
                     CefDragHandler::DragOperationsMask mask) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        std::vector<CefString> arFiles;
        dragData->GetFileNames(arFiles);

        if (0 != arFiles.size())
        {
            // нельзя напрямую - нужно скрыть добавление в resolver
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_drop_files");
            int nIndex = 0;
            for (std::vector<CefString>::iterator i = arFiles.begin(); i != arFiles.end(); i++)
            {
                std::wstring sFile = (*i).ToWString();
                NSStringUtils::string_replace(sFile, L"\\", L"/");
                m_pParent->GetAppManager()->AddFileToLocalResolver(sFile);
                message->GetArgumentList()->SetString(nIndex++, sFile);
            }
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
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
                                     bool canGoForward) OVERRIDE
    {
        if (!isLoading && m_pParent && m_pParent->GetAppManager() && m_pParent->GetAppManager()->GetEventListener())
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = m_pParent->CreateCefEvent(ASC_MENU_EVENT_TYPE_PAGE_LOAD_END);
            m_pParent->GetAppManager()->GetEventListener()->OnEvent(pEvent);
        }
    }

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions) OVERRIDE {}

#ifndef CEF_2623
    virtual void OnAutoResize(const CefSize& new_size) OVERRIDE {}
#endif

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
CefRefPtr<CefBrowser> CCefView_Private::GetBrowser() const
{
    if (!m_handler)
        return NULL;
    return m_handler->GetBrowser();
}
void CCefView_Private::LocalFile_End()
{
    if (!m_oConverterToEditor.m_sName.empty())
    {
        m_oLocalInfo.m_oInfo.m_sFileSrc = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/" + m_oConverterToEditor.m_sName;
    }

    if (m_bIsCloudCryptFile)
    {
        CefRefPtr<CefProcessMessage> messageCrypt = CefProcessMessage::Create("onload_crypt_document");
        messageCrypt->GetArgumentList()->SetString(0, m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/Editor.bin");

        SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), messageCrypt);
        m_oLocalInfo.m_oInfo.m_nCounterConvertion = 1; // for reload enable
        return;
    }

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_loadend");
    message->GetArgumentList()->SetString(0, m_oLocalInfo.m_oInfo.m_sRecoveryDir);
    message->GetArgumentList()->SetString(1, m_oLocalInfo.m_oInfo.m_sFileSrc);
    message->GetArgumentList()->SetBool(2, m_oLocalInfo.m_oInfo.m_bIsSaved);
    message->GetArgumentList()->SetString(3, m_oConverterToEditor.GetSignaturesJSON());

    bool isLocked = false;
    if (m_oLocalInfo.m_oInfo.m_bIsSaved)
        isLocked = NSSystem::CLocalFileLocker::IsLocked(m_oLocalInfo.m_oInfo.m_sFileSrc);

    if (!isLocked)
        CheckLockLocalFile();

    message->GetArgumentList()->SetBool(4, isLocked);

    if (isLocked)
        m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), message);

    m_oLocalInfo.m_oInfo.m_nCounterConvertion = 1; // for reload enable
}
void CCefView_Private::LocalFile_SaveEnd(int nError, const std::wstring& sPass)
{
    m_oConverterFromEditor.m_nTypeEditorFormat = -1;
    m_oLocalInfo.m_bIsRetina = false;

    std::wstring sNotEditableLocal;
    int nError_Old = nError;

    bool bIsSavedFileCurrent = true;
    int nOldFormat = m_oLocalInfo.m_oInfo.m_nCurrentFileFormat;
    if (nOldFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
    {
        bIsSavedFileCurrent = false;
    }
    if (!LocalFile_IsSupportEditFormat(m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat))
    {
        bIsSavedFileCurrent = false;
    }
    if (m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM &&
            nOldFormat != AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM)
    {
        // значит был saveAs. мы не можем переходить из интерфейса редактора в интерфейс заполнения
        bIsSavedFileCurrent = false;
    }

    if (!bIsSavedFileCurrent)
    {
        if (0 == nError && LocalFile_IsSupportOpenFormat(m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat))
            m_pManager->m_pInternal->Recents_Add(m_oConverterFromEditor.m_oInfo.m_sFileSrc, m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat, L"", L"", m_sParentUrl);
        nError = ASC_CONSTANT_CANCEL_SAVE;

        sNotEditableLocal = m_oConverterFromEditor.m_oInfo.m_sFileSrc;
    }

    if (ASC_CONSTANT_CANCEL_SAVE == nError)
    {
        if (m_pManager->GetEventListener() && m_pCefView != NULL)
        {
            m_bIsSaving = false;

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

        if (0 == nError_Old && !sNotEditableLocal.empty())
        {
            bool bIsPassAdd = false;
            if (!sPass.empty() && (0 == nError_Old))
            {
                bIsPassAdd = true;
                message->GetArgumentList()->SetString(2, sPass);

                ICertificate* pCert = ICertificate::CreateInstance();
                std::string sHash = pCert->GetHash(sNotEditableLocal, OOXML_HASH_ALG_SHA256);
                delete pCert;

                message->GetArgumentList()->SetString(3, sHash);
            }
        }

        SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), message);
        return;
    }

    if (m_sCloudCryptSrc.empty())
    {
        bool bIsSaved = ((0 == nError) && NSFile::CFileBinary::Exists(m_oConverterFromEditor.m_oInfo.m_sFileSrc)) ? true : false;

        if (bIsSaved && !m_oLocalInfo.m_oInfo.m_bIsSaved)
        {
            m_oLocalInfo.m_oInfo.m_bIsSaved = true;
        }

        if (bIsSaved)
        {
            // обновим информацию о файле
            m_oLocalInfo.m_oInfo.m_sFileSrc = m_oConverterFromEditor.m_oInfo.m_sFileSrc;
            m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = m_oConverterFromEditor.m_oInfo.m_nCurrentFileFormat;

            CheckLockLocalFile();

            // информация для recover
            std::wstring sNameInfo = m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/asc_name.info";
            if (NSFile::CFileBinary::Exists(sNameInfo))
                NSFile::CFileBinary::Remove(sNameInfo);

            NSStringUtils::CStringBuilder oBuilderInfo;
            oBuilderInfo.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><info type=\"" + std::to_wstring(m_oLocalInfo.m_oInfo.m_nCurrentFileFormat) + L"\" name=\"");
            oBuilderInfo.WriteEncodeXmlString(NSFile::GetFileName(m_oLocalInfo.m_oInfo.m_sFileSrc));
            oBuilderInfo.WriteString(L"\" />");
            NSFile::CFileBinary::SaveToFile(sNameInfo, oBuilderInfo.GetData(), true);

            // добавляем в recent
            m_pManager->m_pInternal->Recents_Add(m_oLocalInfo.m_oInfo.m_sFileSrc, m_oLocalInfo.m_oInfo.m_nCurrentFileFormat, L"", L"", m_sParentUrl);

            if (m_pManager->GetEventListener())
            {
                // событие наверх
                NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME);

                NSEditorApi::CAscDocumentName* pData = new NSEditorApi::CAscDocumentName();
                pData->put_Id(m_pCefView->GetId());
                pData->put_Name(NSFile::GetFileName(m_oLocalInfo.m_oInfo.m_sFileSrc));
                if (m_oLocalInfo.m_oInfo.m_bIsSaved)
                {
                    std::wstring sPath = m_oLocalInfo.m_oInfo.m_sFileSrc;
    #ifdef WIN32
                    NSStringUtils::string_replace(sPath, L"/", L"\\");
    #endif
                    pData->put_Path(sPath);
                }

                pEvent->m_pData = pData;
                m_pManager->GetEventListener()->OnEvent(pEvent);
            }

            if (m_pManager->GetEventListener())
            {
                // событие onsave
                // в случае криптования - после записи паролей
                if (GetViewCryptoMode() == 0) // только после конца записи в блокчейн
                {
                    m_bIsSaving = false;

                    NSEditorApi::CAscCefMenuEvent* pEvent = m_pCefView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONSAVE);
                    NSEditorApi::CAscDocumentOnSaveData* pData = new NSEditorApi::CAscDocumentOnSaveData();
                    pData->put_Id(m_pCefView->GetId());
                    pEvent->m_pData = pData;
                    m_pManager->GetEventListener()->OnEvent(pEvent);
                }
                else
                {
                    m_bIsSavingCrypto = true;
                }
            }
        }
    }

    std::string sMessageName = "onlocaldocument_onsaveend";
    std::wstring sFileSrc = m_oLocalInfo.m_oInfo.m_sFileSrc;
    if (!m_sCloudCryptSrc.empty())
    {
        sMessageName = "build_crypted_file_end";
        sFileSrc = m_oConverterFromEditor.m_oInfo.m_sFileSrc;
    }

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(sMessageName);
    message->GetArgumentList()->SetString(0, (0 == nError) ? sFileSrc : L"");
    message->GetArgumentList()->SetInt(1, (0 == nError) ? 0 : 2);

    bool bIsPassAdd = false;
    if (!sPass.empty() && (0 == nError))
    {
        bIsPassAdd = true;
        message->GetArgumentList()->SetString(2, sPass);

        ICertificate* pCert = ICertificate::CreateInstance();
        std::string sHash = pCert->GetHash(sFileSrc, OOXML_HASH_ALG_SHA256);
        delete pCert;

        message->GetArgumentList()->SetString(3, sHash);
    }

    SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), message);
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
                browser = m_handler->GetBrowser();

            if (browser)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_additionalparams");
                message->GetArgumentList()->SetInt(0, m_nLocalFileOpenError);

                if (90 == m_nLocalFileOpenError || 91 == m_nLocalFileOpenError)
                {
                    ICertificate* pCert = ICertificate::CreateInstance();
                    std::string sHash = pCert->GetHash(m_oLocalInfo.m_oInfo.m_sFileSrc, OOXML_HASH_ALG_SHA256);
                    delete pCert;

                    message->GetArgumentList()->SetString(1, sHash);

                    std::wstring sDocInfo = GetFileDocInfo(m_oLocalInfo.m_oInfo.m_sFileSrc);
                    if (!sDocInfo.empty())
                        message->GetArgumentList()->SetString(2, sDocInfo);
                }

                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
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
    if (m_bIsClosing || m_bIsDestroying)
        return;

    if (!m_oConverterToEditor.m_sComparingFile.empty())
    {
        if (89 == nError || 90 == nError || 91 == nError)
        {
            CefRefPtr<CefBrowser> browser;
            if (m_handler.get())
                browser = m_handler->GetBrowser();

            if (browser)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_additionalparams");
                message->GetArgumentList()->SetInt(0, nError);

                if (90 == nError || 91 == nError)
                {
                    ICertificate* pCert = ICertificate::CreateInstance();
                    std::string sHash = pCert->GetHash(m_oConverterToEditor.m_sComparingFile, OOXML_HASH_ALG_SHA256);
                    delete pCert;

                    message->GetArgumentList()->SetString(1, sHash);

                    std::wstring sDocInfo = GetFileDocInfo(m_oConverterToEditor.m_sComparingFile);
                    if (!sDocInfo.empty())
                        message->GetArgumentList()->SetString(2, sDocInfo);
                }

                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            }
            return;
        }
        else
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("oncompare_loadend");
            message->GetArgumentList()->SetInt(0, nError);
            message->GetArgumentList()->SetString(1, m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/compare");
            SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), message);

            m_oConverterToEditor.m_sComparingFile = L"";

            // если по урлу - то файл временный
            if (m_nComparingFileType == 1)
                NSFile::CFileBinary::Remove(m_sComparingFile);

            m_sComparingFile = L"";
            m_nComparingFileType = -1;
        }
        return;
    }

    m_nLocalFileOpenError = nError;
    LocalFile_IncrementCounter();
}
void CCefView_Private::CheckZoom()
{
    if (CAscApplicationManager::IsUseSystemScaling())
        return;

    if (NULL == CAscApplicationManager::GetDpiChecker())
        return;

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

#ifdef WIN32
    double dDeviceScale = NSMonitor::GetRawMonitorDpi(hwnd);
#else
    unsigned int _dx = 0;
    unsigned int _dy = 0;
    int nDeviceScaleTmp = CAscApplicationManager::GetDpiChecker()->GetWidgetImplDpi(this->m_pCefView->GetWidgetImpl(), &_dx, &_dy);
    double dDeviceScale = CAscApplicationManager::GetDpiChecker()->GetScale(_dx, _dy);
#endif

    if (m_pManager->m_pInternal->m_dForceDisplayScale > 0)
        dDeviceScale = m_pManager->m_pInternal->m_dForceDisplayScale;

    if (fabs(dDeviceScale - m_dDeviceScale) > 0.0001)
    {
        m_dDeviceScale = dDeviceScale;

        if (m_dDeviceScale >= 1)
        {
            double dScale = (m_dDeviceScale > 1.1) ? (log(m_dDeviceScale) / log(1.2)) : 0;

            _host->SetZoomLevel(dScale);
            _host->NotifyScreenInfoChanged();
            _host->WasResized();
        }
    }
}

void CCefView_Private::SendProcessMessage(CefProcessId target_process, CefRefPtr<CefProcessMessage> message)
{
    if (m_handler && m_handler->GetBrowser())
    {
        SEND_MESSAGE_TO_RENDERER_PROCESS(m_handler->GetBrowser(), message);
    }
}

void CAscClientHandler::OnFoundCookie(bool bIsPresent, std::string sValue)
{
    if (!m_pParent || !m_pParent->m_pInternal || !m_pParent->m_pInternal->m_handler || !m_pParent->m_pInternal->m_handler->GetBrowser())
        return;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_is_cookie_present");
    message->GetArgumentList()->SetBool(0, bIsPresent);
    message->GetArgumentList()->SetString(1, sValue);
    SEND_MESSAGE_TO_RENDERER_PROCESS(m_pParent->m_pInternal->m_handler->GetBrowser(), message);
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

    SEND_MESSAGE_TO_RENDERER_PROCESS(m_pParent->m_pInternal->m_handler->GetBrowser(), message);
}

void CAscClientHandler::OnSetCookie()
{
    if (!m_pParent || !m_pParent->m_pInternal || !m_pParent->m_pInternal->m_handler || !m_pParent->m_pInternal->m_handler->GetBrowser())
        return;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_set_cookie");
    SEND_MESSAGE_TO_RENDERER_PROCESS(m_pParent->m_pInternal->m_handler->GetBrowser(), message);
}

void CAscClientHandler::OnDeleteCookie(bool bIsPresent)
{
    // not used
}

// CefView --------------------------------------------------------------------------------
CCefView::CCefView(CCefViewWidgetImpl* parent, int nId)
{    
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

void CCefView::load(const std::wstring& urlInputSrc)
{
    std::wstring urlInput = urlInputSrc;

    // заглушка, чтобы не проверять выше
    NSStringUtils::string_replace(urlInput, L"//?desktop=true", L"/?desktop=true");

    std::wstring sExternalSchemeName = m_pInternal->m_pManager->GetExternalSchemeName() + L":";

    bool bIsScheme = false;
    if (0 == urlInput.find(sExternalSchemeName))
    {
        bIsScheme = true;
        if (0 == urlInput.find(sExternalSchemeName + L"//"))
            urlInput = urlInput.substr(12);
        else
            urlInput = urlInput.substr(10);
    }
    if (!bIsScheme)
    {
        if (0 == urlInput.find(L"open|"))
            bIsScheme = true;
    }
    if (bIsScheme)
    {
        std::vector<std::wstring> arParams;
        std::wstring::size_type posOld = 0;
        std::wstring::size_type pos = urlInput.find('|');
        while (pos != std::wstring::npos)
        {
            arParams.push_back(urlInput.substr(posOld, pos - posOld));
            posOld = pos + 1;
            pos = urlInput.find('|', posOld);
        }
        if (posOld < urlInput.length())
            arParams.push_back(urlInput.substr(posOld));

        size_t nSizeParams = (size_t)arParams.size();
        if (1 == nSizeParams)
        {
            urlInput = arParams[0];
        }
        else if (1 < nSizeParams && L"open" == arParams[0])
        {
            urlInput = arParams[nSizeParams - 1];
        }
    }

    if (NSFileDownloader::IsNeedDownload(urlInput))
    {
        // проверяем на recent parent
        std::wstring sRecentParent = L"";

        std::wstring sUrlRecent = urlInput;
        std::wstring::size_type posQ = sUrlRecent.find(L"?");
        if (std::wstring::npos != posQ)
            sUrlRecent = sUrlRecent.substr(0, posQ);

        std::vector<CRecentParent>* parent = &m_pInternal->m_pManager->m_pInternal->m_arRecentParents;
        for (std::vector<CRecentParent>::iterator iter = parent->begin(); iter != parent->end(); iter++)
        {
            std::wstring sIterUrl = iter->Url;
            posQ = sIterUrl.find(L"?");
            if (std::wstring::npos != posQ)
                sIterUrl = sIterUrl.substr(0, posQ);

            if (sUrlRecent == sIterUrl)
            {
                sRecentParent = iter->Parent;
                parent->erase(iter);
                break;
            }
        }

        m_pInternal->m_sParentUrl = sRecentParent;
    }

    if (true)
    {
        if (m_pInternal->m_bIsExternalCloud && GetType() == cvwtSimple)
        {
            //NSStringUtils::string_replace(urlInput, L"/products/files/", L"/");
            // теперь смотрим за этим выше
        }
        else if (GetType() == cvwtEditor)
        {
            std::wstring sExternalCloudFind = L"desktop_editor_cloud_type_external=";
            std::wstring::size_type posExternal = urlInput.find(sExternalCloudFind);
            if (std::wstring::npos != posExternal)
            {
                std::wstring::size_type posExternal2 = urlInput.find(L"_ext_id", posExternal);

                if (std::wstring::npos != posExternal2)
                {
                    m_pInternal->m_bIsExternalCloud = true;

                    std::wstring::size_type posExternal1 = posExternal + sExternalCloudFind.length();
                    std::wstring sExtId = urlInput.substr(posExternal1, posExternal2 - posExternal1);

                    m_pInternal->m_pManager->m_pInternal->TestExternal(sExtId, m_pInternal->m_oExternalCloud);

                    std::wstring sReplaceExt = sExternalCloudFind + sExtId + L"_ext_id";

                    NSStringUtils::string_replace(urlInput, sReplaceExt, L"");
                }
            }
            else if (!m_pInternal->m_sRecentOpenExternalId.empty())
            {
                m_pInternal->m_bIsExternalCloud = true;
                m_pInternal->m_pManager->m_pInternal->TestExternal(m_pInternal->m_sRecentOpenExternalId, m_pInternal->m_oExternalCloud);
            }
        }
    }

    m_pInternal->m_sRecentOpenExternalId = L"";

    m_pInternal->m_oTxtToDocx.m_pManager = this->GetAppManager();
    m_pInternal->m_oTxtToDocx.m_pCallback = m_pInternal;

    m_pInternal->m_sOriginalUrl = urlInput;

    // check compare
    if (0 == urlInput.find(L"ascdesktop://compare/"))
    {
        std::wstring::size_type pos1 = urlInput.find('/', 21);
        std::wstring::size_type pos2 = urlInput.find('/', pos1 + 1);

        std::wstring strId = urlInput.substr(21, pos1 - 21);
        std::wstring strType = urlInput.substr(pos1 + 1, pos2 - pos1 - 1);
        std::wstring strFile = urlInput.substr(pos2 + 1);

        m_pInternal->m_sComparingFile = strFile;
        m_pInternal->m_nComparingFileType = (strType == L"file") ? 0 : 1;
        ((CCefViewEditor*)this)->OpenCopyAsRecoverFile(std::stoi(strId));
        return;
    }

    // check openaslocal
    std::wstring::size_type pos1 = urlInput.find(L"<openaslocal>");
    std::wstring::size_type pos2 = urlInput.find(L"</openaslocal>");

    if (pos1 != std::wstring::npos && pos2 != std::wstring::npos)
    {
        m_pInternal->m_sOpenAsLocalUrl = urlInput;
        if (true)
        {
            std::map<std::wstring, int>& mapOP = GetAppManager()->m_pInternal->m_mapOnlyPass;
            std::map<std::wstring, int>::iterator findOP = mapOP.find(urlInput);

            if (mapOP.end() != findOP)
            {
                findOP->second = GetId();
            }
            else
            {
                mapOP.insert(std::pair<std::wstring, int>(urlInput, GetId()));
            }
        }

        m_pInternal->m_sCloudCryptSrc = urlInput.substr(0, pos1);

        if (pos1 != std::wstring::npos && pos2 != std::wstring::npos)
        {
            m_pInternal->m_sCloudCryptName = urlInput.substr(pos1 + 13, pos2 - pos1 - 13);
        }

        if (m_pInternal->m_sCloudCryptName.empty())
            m_pInternal->m_sCloudCryptName = L"Document";

        CCefViewEditor* pEditor = ((CCefViewEditor*)this);
        pEditor->OpenLocalFile(L"", 0);
        return;
    }

    std::wstring url = urlInput;
    std::wstring::size_type posQ = url.find_first_of('?');
    if (std::wstring::npos != posQ)
    {
        if (std::wstring::npos == url.find(L"desktop=true", posQ))
            url = (url + L"&desktop=true");

        if (1 == this->GetId())
        {
            m_pInternal->m_pManager->m_pInternal->m_mainPostFix = url.substr(posQ);
            m_pInternal->m_pManager->m_pInternal->m_mainLang = L"default";

            std::wstring sLang = NSUrlParse::GetUrlValue(m_pInternal->m_pManager->m_pInternal->m_mainPostFix, L"lang");
            if (!sLang.empty())
                m_pInternal->m_pManager->m_pInternal->m_mainLang = sLang;
        }
    }
    else
    {
        url = (url + L"?desktop=true");
    }

    if (!m_pInternal || !m_pInternal->m_pManager)
        return;

    if (m_pInternal->m_handler)
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
            m_pInternal->m_handler->m_nBeforeBrowserCounter = 0;
            CefString sUrl;
            sUrl.FromWString(url);
            _frame->LoadURL(sUrl);
        }
        return;
    }

#ifdef MESSAGE_IN_BROWSER
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->SetStoragePath(m_pInternal->m_pManager->m_oSettings.cookie_path, true, NULL);
#endif

    // Create the single static handler class instance
    CAscClientHandler* pClientHandler = new CAscClientHandler();
    pClientHandler->m_pParent = this;
    m_pInternal->m_handler = pClientHandler;
    m_pInternal->m_oDownloaderAbortChecker.m_pHandler = pClientHandler;

    CAscClientHandler::CAscCefJSDialogHandler* pAscCEfJSDialogHandler = static_cast<CAscClientHandler::CAscCefJSDialogHandler*>(pClientHandler->m_pCefJSDialogHandler.get());
    if (pAscCEfJSDialogHandler)
        pAscCEfJSDialogHandler->SetAppManager(m_pInternal->m_pManager);

    CefWindowHandle hWnd = (CefWindowHandle)m_pInternal->m_pWidgetImpl->cef_handle;
    //m_pInternal->m_handler->SetMainWindowHandle(hWnd);

    CefBrowserSettings _settings;
    _settings.file_access_from_file_urls = STATE_ENABLED;
    _settings.universal_access_from_file_urls = STATE_ENABLED;
    _settings.javascript_access_clipboard = STATE_ENABLED;
    _settings.plugins = STATE_DISABLED;
    _settings.background_color = (m_eWrapperType == cvwtEditor) ? 0xFFF4F4F4 : 0xFFFFFFFF;

    CefWindowInfo info;
    CefWindowHandle _handle = (CefWindowHandle)m_pInternal->m_pWidgetImpl->cef_handle;
    int _w = m_pInternal->m_pWidgetImpl->cef_width;
    int _h = m_pInternal->m_pWidgetImpl->cef_height;

#ifdef WIN32
    info.SetAsChild(_handle, RECT { 0, 0, _w, _h });
#endif
#if defined(_LINUX) && !defined(_MAC)
    info.SetAsChild(_handle, CefRect(0, 0, _w, _h));
#endif
#ifdef _MAC
    info.SetAsChild(_handle, 0, 0, _w, _h);
#endif

    CefString sUrl = url;

    m_pInternal->m_bIsFirstLoadSSO = true;
    m_pInternal->m_strSSOFirstDomain = L"";
    m_pInternal->m_arSSOSecondDomain.clear();

#ifdef CEF_VERSION_ABOVE_86
    CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
    size_t nCount = 0;
    std::vector<std::string> props = m_pInternal->m_pManager->GetRendererStartupProperties();
    for (std::vector<std::string>::iterator iter = props.begin(); iter != props.end(); iter++)
    {
        extra_info->SetString(std::to_string(++nCount), *iter);
    }
#endif

    // Creat the new child browser window
#ifdef _WIN32
    // need after window->show
    CefBrowserHost::CreateBrowser(
#else
    CefBrowserHost::CreateBrowserSync(
#endif
            #ifdef CEF_2623
                info, m_pInternal->m_handler.get(), sUrl, _settings, NULL);
            #else
            #ifdef CEF_VERSION_ABOVE_86
                info, m_pInternal->m_handler.get(), sUrl, _settings, extra_info, NULL);
            #else
                info, m_pInternal->m_handler.get(), sUrl, _settings, NULL, NULL);
            #endif
            #endif

    GetWidgetImpl()->AfterCreate();

    focus();
}
void CCefView::reload()
{
    this->load(this->GetUrl());
}
std::wstring CCefView::GetUrl()
{
    return m_pInternal ? m_pInternal->m_strUrl : L"";
}
std::wstring CCefView::GetOriginalUrl()
{
    return m_pInternal ? m_pInternal->m_sOriginalUrl : L"";
}
std::wstring CCefView::GetUrlAsLocal()
{
    return m_pInternal ? m_pInternal->m_sOpenAsLocalUrl : L"";
}

void CCefView::focus(bool value)
{
    if (!m_pInternal || !m_pInternal->m_pWidgetImpl || !m_pInternal->m_handler.get())
        return;

    CefRefPtr<CefBrowser> browser = m_pInternal->m_handler->GetBrowser();
    if (browser)
        browser->GetHost()->SetFocus(value);
}

void CCefView::resizeEvent()
{
    this->moveEvent();
}
void CCefView::moveEvent()
{
    if (!m_pInternal->m_pWidgetImpl || m_pInternal->m_bIsClosing)
        return;

    this->GetWidgetImpl()->UpdateSize();
    m_pInternal->CheckZoom();

    if (m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser() && m_pInternal->m_handler->GetBrowser()->GetHost())
        m_pInternal->m_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
}

void CCefView::Apply(NSEditorApi::CAscMenuEvent* pEvent)
{
    if (!m_pInternal || NULL == pEvent)
        return;

    CefRefPtr<CefBrowser> browser;
    if (m_pInternal->m_handler.get())
        browser = m_pInternal->m_handler->GetBrowser();

    if (!browser)
    {
        if (pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY ||
            pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE)
        {
            m_pInternal->m_bIsDestroying = true;
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
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK:
        {
            NSEditorApi::CAscSpellCheckType* pData = (NSEditorApi::CAscSpellCheckType*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("spell_check_response");
            message->GetArgumentList()->SetString(0, pData->get_Result());
            NSArgumentList::SetInt64(message->GetArgumentList(), 1, pData->get_FrameId());
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_CONTROL_ID:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cef_control_id");
            message->GetArgumentList()->SetInt(0, m_nId);
            message->GetArgumentList()->SetBool(1, GetAppManager()->m_oSettings.sign_support);
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SYNC_COMMAND:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("sync_command_end");
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
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
            if (m_pInternal && pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE && m_pInternal->m_bIsReceiveOnce_OnDocumentModified)
                m_pInternal->m_bIsRemoveRecoveryOnClose = true;

            m_pInternal->m_bIsDestroying = true;
            
            m_pInternal->m_oConverterFromEditor.Stop();
            m_pInternal->m_oConverterToEditor.Stop();

            m_pInternal->m_pWidgetImpl->OnRelease();
            m_pInternal->m_pWidgetImpl = NULL;
            if (m_pInternal && m_pInternal->m_handler && m_pInternal->m_handler->GetBrowser())
            {
                m_pInternal->CloseBrowser(true);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SAVE:
        {
            m_pInternal->m_bIsSaving = true;

            if (NULL != m_pInternal->m_pManager->m_pInternal->m_pAdditional)
                m_pInternal->m_pManager->m_pInternal->m_pAdditional->Local_Save_Start();

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("document_save");
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_PRINT_START:
        {
            std::string sPrintParameters = m_pInternal->m_sPrintParameters;
            m_pInternal->m_sPrintParameters = "";
            if (m_pInternal)
            {
                if (this->GetType() == cvwtEditor)
                {
                    m_pInternal->m_oPrintData.m_eEditorType = ((CCefViewEditor*)this)->GetEditorType();
                }

                m_pInternal->m_oPrintData.Print_Start(m_pInternal->m_pManager->m_pInternal->m_pApplicationFonts);
            }

            bool bIsNativePrint = false;
            if (!m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc.empty())
            {
                // локальные файлы
                switch (m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat)
                {
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU:
                {
                    m_pInternal->m_oPrintData.m_pNativePrinter = new CAscNativePrintDocument(m_pInternal->m_sNativeFilePassword);
                    break;
                }
                default:
                    break;
                };

                if (NULL != m_pInternal->m_oPrintData.m_pNativePrinter)
                {                    
                    m_pInternal->m_oPrintData.m_pNativePrinter->m_pApplicationFonts = m_pInternal->m_oPrintData.m_pApplicationFonts;

                    if (NULL == m_pInternal->m_oPrintData.m_pNativePrinter->m_pApplicationFonts)
                        m_pInternal->m_oPrintData.m_pNativePrinter->m_pApplicationFonts = m_pInternal->m_pManager->m_pInternal->m_pApplicationFonts;


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
                if ("" != sPrintParameters)
                    message->GetArgumentList()->SetString(0, sPrintParameters);
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            }
            else
            {
                // analog message "print_end"
                if (m_pInternal->m_pManager->GetEventListener())
                {
                    NSEditorApi::CAscCefMenuEvent* pEvent = this->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END);

                    NSEditorApi::CAscPrintEnd* pData = new NSEditorApi::CAscPrintEnd();
                    pData->put_Id(GetId());
                    pData->put_PagesCount((int)m_pInternal->m_oPrintData.m_arPages.size());
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
            NSArgumentList::SetInt64(message->GetArgumentList(), 2, pData->get_FrameId());
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG:
        {
            NSEditorApi::CAscSaveDialog* pData = (NSEditorApi::CAscSaveDialog*)pEvent->m_pData;

            if (m_pInternal->m_sIFrameIDMethod.empty())
            {
                // download
                std::wstring strPath = pData->get_FilePath();
                if (strPath.empty())
                {
                    GetAppManager()->CancelDownload(pData->get_IdDownload());

                    if (m_pInternal->m_before_callback_info)
                    {
                        NSEditorApi::CAscCefMenuEvent* pCefEvent = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD);
                        pCefEvent->put_SenderId(m_pInternal->m_before_callback_info->get_Id());
                        pCefEvent->m_pData = m_pInternal->m_before_callback_info;
                        m_pInternal->m_before_callback_info->AddRef();
                        GetAppManager()->GetEventListener()->OnEvent(pCefEvent);
                    }
                }
                else
                {
                    CefString sPath;
                    sPath.FromWString(pData->get_FilePath());
                    m_pInternal->m_before_callback->Continue(sPath, false);
                }

                if (m_pInternal->m_before_callback_info)
                {
                    m_pInternal->m_before_callback_info->Release();
                    m_pInternal->m_before_callback_info = NULL;
                }
                m_pInternal->m_before_callback->Release();
                m_pInternal->m_before_callback = NULL;
            }
            else
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_save_filename_dialog");
                message->GetArgumentList()->SetString(0, m_pInternal->m_sIFrameIDMethod);
                message->GetArgumentList()->SetString(1, pData->get_FilePath());
                SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
                m_pInternal->m_sIFrameIDMethod = "";
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS:
        {
            NSEditorApi::CAscLocalRecentsAll* pData = (NSEditorApi::CAscLocalRecentsAll*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecents");
            message->GetArgumentList()->SetString(0, pData->get_JSON());

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS:
        {
            NSEditorApi::CAscLocalRecentsAll* pData = (NSEditorApi::CAscLocalRecentsAll*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecovers");
            message->GetArgumentList()->SetString(0, pData->get_JSON());

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH:
        {
            NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;
            std::wstring sPath = pData->get_Path();
            m_pInternal->m_bIsSavingDialog = false;

            if (m_pInternal->m_nCryptoDownloadAsFormat > 0)
            {
                if (sPath == L"")
                {
                    m_pInternal->OnFileConvertFromEditor(0);
                }
                else
                {
                    m_pInternal->m_oSimpleX2tConverter.m_nOutputFormat = m_pInternal->m_nCryptoDownloadAsFormat;
                    m_pInternal->m_oSimpleX2tConverter.m_sOutputPath = sPath;
                    m_pInternal->m_oSimpleX2tConverter.m_sOutputParams = m_pInternal->m_sCryptoDownloadAsParams;
                    m_pInternal->m_oSimpleX2tConverter.m_sRecoverFolder = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir;
                    m_pInternal->m_oSimpleX2tConverter.m_pEvents = m_pInternal;
                    m_pInternal->m_oSimpleX2tConverter.m_pManager = m_pInternal->m_pManager;
                    m_pInternal->m_oSimpleX2tConverter.Convert(m_pInternal->m_pManager->m_pInternal->m_bIsEnableConvertLogs);
                }
                break;
            }

            if (sPath == L"")
            {
                m_pInternal->LocalFile_SaveEnd(ASC_CONSTANT_CANCEL_SAVE);
                // cancel build file
                m_pInternal->m_bIsBuilding = false;
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

            std::wstring sPath = pData->get_Path();
            if (pData->get_IsMultiselect() || sPath.empty())
            {
                std::vector<std::wstring>& arPaths = pData->get_Files();
                if (arPaths.size() > 0)
                    sPath = arPaths[0];
            }

            m_pInternal->m_pManager->AddFileToLocalResolver(sPath);
            message->GetArgumentList()->SetString(0, sPath);
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS:
        {
            NSEditorApi::CAscExecCommandJS* pData = (NSEditorApi::CAscExecCommandJS*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_native_message");
            message->GetArgumentList()->SetString(0, pData->get_Command());
            std::wstring sParam = pData->get_Param();
            NSStringUtils::string_replace(sParam, L"\\", L"\\\\");
            NSStringUtils::string_replace(sParam, L"\"", L"\\\"");
            message->GetArgumentList()->SetString(1, sParam);
            message->GetArgumentList()->SetString(2, pData->get_FrameName());

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND:
        {
            NSEditorApi::CAscEditorExecuteCommand* pData = (NSEditorApi::CAscEditorExecuteCommand*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_editor_native_message");
            message->GetArgumentList()->SetString(0, pData->get_Command());
            message->GetArgumentList()->SetString(1, pData->get_Params());

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }        
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("update_install_plugins");
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG:
        {
            NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

            if (m_pInternal->m_handler->m_pFileDialogCallback)
            {
                std::vector<CefString> file_paths;

                if (!pData->get_IsMultiselect())
                {
                    std::wstring sPathRet = pData->get_Path();
                    if (!sPathRet.empty())
                    {
                        m_pInternal->m_pManager->AddFileToLocalResolver(sPathRet);
                        file_paths.push_back(sPathRet);
                    }
                }
                else
                {
                    std::vector<std::wstring>& arPaths = pData->get_Files();
                    if (arPaths.size() > 0)
                    {
                        for (std::vector<std::wstring>::iterator i = arPaths.begin(); i != arPaths.end(); i++)
                        {
                            std::wstring sPathRet = *i;
                            if (!sPathRet.empty())
                            {
                                m_pInternal->m_pManager->AddFileToLocalResolver(sPathRet);
                                file_paths.push_back(sPathRet);
                            }
                        }
                    }
                    else
                    {
                        std::wstring sPathRet = pData->get_Path();
                        if (!sPathRet.empty())
                        {
                            m_pInternal->m_pManager->AddFileToLocalResolver(sPathRet);
                            file_paths.push_back(sPathRet);
                        }
                    }
                }

                m_pInternal->m_handler->m_pFileDialogCallback->Continue(0, file_paths);
                m_pInternal->m_handler->m_pFileDialogCallback = NULL;

                break;
            }


            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_open_filename_dialog");
            message->GetArgumentList()->SetString(0, m_pInternal->m_sIFrameIDMethod);
            message->GetArgumentList()->SetBool(1, pData->get_IsMultiselect());
            m_pInternal->m_sIFrameIDMethod = "";

            if (!pData->get_IsMultiselect())
            {
                m_pInternal->m_pManager->AddFileToLocalResolver(pData->get_Path());
                message->GetArgumentList()->SetString(2, pData->get_Path());
            }
            else
            {
                int nIndex = 2;
                std::vector<std::wstring>& arPaths = pData->get_Files();

                if (arPaths.size() > 0)
                {
                    for (std::vector<std::wstring>::iterator i = arPaths.begin(); i != arPaths.end(); i++)
                    {
                        m_pInternal->m_pManager->AddFileToLocalResolver(*i);
                        message->GetArgumentList()->SetString(nIndex++, *i);
                    }
                }
                else
                {
                    std::wstring sPath1 = pData->get_Path();
                    if (!sPath1.empty())
                    {
                        m_pInternal->m_pManager->AddFileToLocalResolver(sPath1);
                        message->GetArgumentList()->SetString(nIndex++, sPath1);
                    }
                }
            }

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG:
        {
            NSEditorApi::CAscLocalOpenDirectoryDialog* pData = (NSEditorApi::CAscLocalOpenDirectoryDialog*)pEvent->m_pData;
            if (m_pInternal->m_handler->m_pDirectoryDialogCallback)
            {
                std::wstring sPath = pData->get_Path();
                if (sPath.empty())
                    m_pInternal->m_handler->m_pDirectoryDialogCallback->Cancel();
                else
                {
                    std::vector<CefString> file_paths;
                    file_paths.push_back(sPath);
                    m_pInternal->m_handler->m_pDirectoryDialogCallback->Continue(0, file_paths);
                }
                m_pInternal->m_handler->m_pDirectoryDialogCallback = NULL;
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO:
        {
            NSEditorApi::CAscEditorSaveQuestion* pData = (NSEditorApi::CAscEditorSaveQuestion*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_file_save_question");
            message->GetArgumentList()->SetBool(0, pData->get_Value());

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM:
        {
            NSEditorApi::CAscReporterMessage* pData = (NSEditorApi::CAscReporterMessage*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("from_reporter_message");
            message->GetArgumentList()->SetString(0, pData->get_Message());
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
        {
            NSEditorApi::CAscReporterMessage* pData = (NSEditorApi::CAscReporterMessage*)pEvent->m_pData;
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("to_reporter_message");
            message->GetArgumentList()->SetString(0, pData->get_Message());
            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE:
        {
            NSEditorApi::CAscUIThreadMessage* pData = (NSEditorApi::CAscUIThreadMessage*)pEvent->m_pData;
            int nType = pData->get_Type();

            switch (nType)
            {
            case 0:
            {
                m_pInternal->m_pDownloadViewCallback->m_pInternal->OnViewDownloadFile();
                m_pInternal->m_pDownloadViewCallback = NULL;
                break;
            }
            default:
                break;
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE:
        {
            NSEditorApi::CAscOpenSslData* pData = (NSEditorApi::CAscOpenSslData*)pEvent->m_pData;

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_selectsertificate_ret");

            if (pData)
            {
                ICertificate* pCert = ICertificate::CreateInstance();
                std::wstring sCertPass = pData->get_CertPassword();
                std::wstring sKeyPass = pData->get_KeyPassword();
                std::string sCertPassA = U_TO_UTF8(sCertPass);
                std::string sKeyPassA = U_TO_UTF8(sKeyPass);
                bool bFromFiles = pCert->FromFiles(pData->get_KeyPath(), sKeyPassA, pData->get_CertPath(), sCertPassA);

                if (bFromFiles)
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
                else
                {
                    message->GetArgumentList()->SetString(0, "{}");
                }

                RELEASEOBJECT(pCert);
            }

            SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_BINARY_FROM_RENDERER:
        {
            NSEditorApi::CAscBinaryMessage* pData = (NSEditorApi::CAscBinaryMessage*)pEvent->m_pData;

            BYTE* pBinaryData = pData->get_Data();
            unsigned int nBinaryLen = pData->get_DataSize();
            std::string sMessage = pData->get_MessageName();

            RELEASEARRAYOBJECTS(pBinaryData);
            break;
        }
        case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD:
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted_file");
            message->GetArgumentList()->SetString(0, m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);
            m_pInternal->SendProcessMessage(PID_RENDERER, message);
            break;
        }
        case ASC_MENU_EVENT_TYPE_EXECUTE_JS_CODE:
        {
            NSEditorApi::CAscJSMessage* pData = (NSEditorApi::CAscJSMessage*)pEvent->m_pData;
            CefRefPtr<CefBrowser> pBrowser = m_pInternal->GetBrowser();
            if (pBrowser)
            {
                std::wstring sCode = pData->get_Value();

                std::vector<int64> arIds;
                pBrowser->GetFrameIdentifiers(arIds);

                for (std::vector<int64>::iterator iter = arIds.begin(); iter != arIds.end(); iter++)
                {
                    CefRefPtr<CefFrame> frame = pBrowser->GetFrame(*iter);
                    if (frame)
                        frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
                }
            }
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
    if (!m_pInternal || !m_pInternal->m_handler || !m_pInternal->m_handler->GetBrowser() || !m_pInternal->m_handler->GetBrowser()->GetHost())
        return false;
    
    m_pInternal->m_oDownloaderAbortChecker.StartDownload(sUrl);
    m_pInternal->m_handler->GetBrowser()->GetHost()->StartDownload(sUrl);    
    return true;
}

void CCefView::SetExternalCloud(const std::wstring& sProviderId)
{
    if (L"asc" == sProviderId || L"onlyoffice" == sProviderId)
    {
        m_pInternal->m_bIsSSO = true;
        return;
    }
    m_pInternal->m_bIsExternalCloud = GetAppManager()->m_pInternal->TestExternal(sProviderId, m_pInternal->m_oExternalCloud);
}

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
    if (m_pInternal->m_bIsClosing)
        return NULL;
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
void CCefView::LoadReporter(void* reporter_data)
{
    CAscReporterData* pReporterData = (CAscReporterData*)reporter_data;
    int nParentId = pReporterData->ParentId;
    std::wstring url = pReporterData->Url;

    m_pInternal->m_bIsReporter = true;
    m_pInternal->m_nReporterParentId = nParentId;
    m_pInternal->m_sCloudCryptoReporter = pReporterData->CloudCryptoInfo;
    m_pInternal->m_sVersionForReporter = pReporterData->Version;

    // url - parent url
    std::wstring urlReporter = url;

    // TODO: check .html? ???
    std::wstring urlParentCheck = L"index.html";
    std::wstring::size_type pos = url.rfind(L"/" + urlParentCheck);
    if (std::wstring::npos == pos)
    {
        pos = url.rfind(L"/index_loader.html");
        if (std::wstring::npos != pos)
            urlParentCheck = L"index_loader.html";
    }

    if (std::wstring::npos != pos)
    {
        urlReporter = url.substr(0, pos);
        urlReporter += L"/index.reporter.html";
    }

    std::wstring sUrlParams = L"";
    std::wstring::size_type posParams = url.find(urlParentCheck + L"?");
    if (posParams != std::wstring::npos)
    {
        std::wstring::size_type posLang = url.find(L"lang=", posParams);
        if (posLang != std::wstring::npos)
        {
            std::wstring::size_type posLang2 = url.find(L"&", posLang);
            if (posLang2 == std::wstring::npos)
                posLang2 = url.length();
            sUrlParams += url.substr(posLang, posLang2 - posLang);
        }
    }

    if (!sUrlParams.empty())
    {
        urlReporter += (L"?" + sUrlParams);
    }
    this->load(urlReporter);
}

double CCefView::GetDeviceScale()
{
    if (m_pInternal->m_dDeviceScale >= 1)
        return m_pInternal->m_dDeviceScale;

    if (!m_pInternal->GetBrowser() || !m_pInternal->GetBrowser()->GetHost())
        return 1;

    CefWindowHandle hwnd = m_pInternal->GetBrowser()->GetHost()->GetWindowHandle();

    unsigned int _dx = 0;
    unsigned int _dy = 0;
    double dDeviceScale = 1;
#ifdef WIN32
    Core_GetMonitorRawDpi(hwnd, &_dx, &_dy);
    dDeviceScale = Core_GetMonitorScale(_dx, _dy);
#else
    int nDeviceScaleTmp = CAscApplicationManager::GetDpiChecker()->GetWidgetImplDpi(GetWidgetImpl(), &_dx, &_dy);
    dDeviceScale = CAscApplicationManager::GetDpiChecker()->GetScale(_dx, _dy);
#endif

    return dDeviceScale;
}

CefRefPtr<CefFrame> CCefView_Private::CCloudCryptoUpload::GetFrame()
{
    if (!View->m_handler || !View->m_handler->GetBrowser())
        return NULL;
    return View->m_handler->GetBrowser()->GetFrame(FrameID);
}

// CefViewEditor --------------------------------------------------------------------------
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

void CCefViewEditor::OpenLocalFile(const std::wstring& sFilePath, const int& nFileFormat_, const std::wstring& params)
{
    if (sFilePath.empty())
    {
        if (!m_pInternal->m_sCloudCryptSrc.empty())
        {
            m_pInternal->m_bIsCloudCryptFile = true;

            if (m_pInternal->m_bIsReporter)
            {
                CCefViewEditor* pParentView = (CCefViewEditor*)GetAppManager()->GetViewById(m_pInternal->m_nReporterParentId);
                m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
                m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = pParentView->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir;
                m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = pParentView->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
                m_pInternal->LocalFile_IncrementCounter();
                m_pInternal->LocalFile_IncrementCounter();
                return;
            }

            m_pInternal->m_pDownloadViewCallback = this;
            m_pInternal->m_sDownloadViewPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pInternal->m_pManager->m_pInternal->StartTmpDirectory(), L"OL");
            if (NSFile::CFileBinary::Exists(m_pInternal->m_sDownloadViewPath))
                NSFile::CFileBinary::Remove(m_pInternal->m_sDownloadViewPath);

#ifdef WIN32
            NSStringUtils::string_replace(m_pInternal->m_sDownloadViewPath, L"/", L"\\");
#endif

            std::wstring sExtBase = m_pInternal->m_sCloudCryptName;

            // в истории версий вид ссылки такой: document.docx (data)
            std::wstring::size_type pos1 = sExtBase.rfind(L"(");
            std::wstring::size_type pos2 = sExtBase.rfind(L")");
            std::wstring::size_type posDot = sExtBase.rfind(L".");

            if (std::wstring::npos != pos1 && std::wstring::npos != pos2 && std::wstring::npos != posDot)
            {
                if (pos2 > posDot)
                    sExtBase = sExtBase.substr(0, pos1);
            }

            NSStringUtils::string_replace(sExtBase, L" ", L"");
            std::wstring sExt = NSFile::GetFileExtention(sExtBase);
            m_pInternal->m_sDownloadViewPath += (L"." + sExt);

            m_pInternal->m_nCloudCryptFileType = 0;
            if (sExt == L"docxf")
                m_pInternal->m_nCloudCryptFileType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF;
            else if (sExt == L"oform")
                m_pInternal->m_nCloudCryptFileType = AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM;

            // load preview...
            if (m_pInternal->m_oLocalInfo.m_oInfo.m_nCounterConvertion != 0)
            {
                // RELOAD!!!
                m_pInternal->m_oLocalInfo.m_oInfo.m_nCounterConvertion = 0;
                if (NSDirectory::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
                    NSDirectory::DeleteDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);
            }
            StartDownload(m_pInternal->m_sCloudCryptSrc);
            return;
        }
    }

    int nFileFormat = nFileFormat_;
    if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_DOCY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    else if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_PPTY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
    else if (nFileFormat == AVS_OFFICESTUDIO_FILE_TEAMLAB_XLSY)
        nFileFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;

    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = nFileFormat;

    std::wstring sParams = L"";

    if (!m_pInternal->m_bIsCloudCryptFile)
    {
        sParams += L"placement=desktop";
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
            sParams = L"placement=desktop&filetype=pdf&mode=view";
        }

        if (nFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM)
        {
            sParams += L"&filetype=oform";
        }
        else if (nFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF)
        {
            sParams += L"&filetype=docxf";
        }

        if (!GetAppManager()->m_pInternal->GetEditorPermission() && sParams.find(L"mode=view") == std::wstring::npos)
            sParams += L"&mode=view";

        std::wstring sAdditionalParams = !params.empty() ? params : GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
        if (!sAdditionalParams.empty())
            sParams += (L"&" + sAdditionalParams);

        std::wstring sLocalFileName = L"";
        if (NSFile::CFileBinary::Exists(sFilePath))
            sLocalFileName = NSFile::GetFileName(sFilePath);

        if (!sLocalFileName.empty())
        {
            CefString sTmp = sLocalFileName;
            CefString sEncode = CefURIEncode(sTmp, false);
            sParams += (L"&title=" + sEncode.ToWString());
        }
    }

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pInternal->m_pManager->m_oSettings.recover_path, L"DE_");
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    std::wstring::size_type nPosPoint = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.rfind('.');
    if (nPosPoint != std::wstring::npos && nPosPoint > m_pInternal->m_pManager->m_oSettings.recover_path.length())
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.substr(0, nPosPoint);
    }

    NSDirectory::CreateDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);
    m_pInternal->CheckLockRecoveryFile();

    if (!m_pInternal->m_sCloudCryptSrc.empty())
    {
        NSDirectory::CreateDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/openaslocal");

        std::wstring sNameBase = m_pInternal->m_sCloudCryptName;
        if (true)
        {
            // в истории версий вид ссылки такой: document.docx (data)
            std::wstring::size_type pos1 = sNameBase.rfind(L"(");
            std::wstring::size_type pos2 = sNameBase.rfind(L")");
            std::wstring::size_type posDot = sNameBase.rfind(L".");

            if (std::wstring::npos != pos1 && std::wstring::npos != pos2 && std::wstring::npos != posDot)
            {
                if (pos2 > posDot)
                    sNameBase = sNameBase.substr(0, pos1);
            }

            // нужно удалить все пробелы в конце файла
            posDot = sNameBase.rfind(L".");
            if (std::wstring::npos != posDot)
            {
                std::wstring::size_type posRemove = sNameBase.find(' ', posDot);
                if (std::wstring::npos != posRemove)
                    sNameBase = sNameBase.substr(0, posRemove);
            }
        }

        std::wstring sFilePathSrc = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir + L"/openaslocal/" + sNameBase;
        NSStringUtils::string_replace(sFilePathSrc, L"\\", L"/");

        NSFile::CFileBinary::Copy(sFilePath, sFilePathSrc);
        m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = sFilePathSrc;

        NSFile::CFileBinary::Remove(sFilePath);

        m_pInternal->LocalFile_IncrementCounter();

        // Send crypto flag to recents
        CTemporaryDocumentInfo* pTempInfo = m_pInternal->m_pTemporaryCloudFileInfo;
        if (pTempInfo)
            m_pInternal->m_pManager->m_pInternal->Recents_Add(pTempInfo->PathSrc, pTempInfo->Type, pTempInfo->Url, pTempInfo->ExternalCloud, pTempInfo->ParentUrl, true);
    }
    else
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = sFilePath;
#ifdef _WIN32
        NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");
#endif

        this->GetAppManager()->m_pInternal->Recents_Add(sFilePath, nFileFormat, L"", L"", m_pInternal->m_sParentUrl);
    }

    if (m_pInternal->m_pTemporaryCloudFileInfo)
    {
        delete m_pInternal->m_pTemporaryCloudFileInfo;
        m_pInternal->m_pTemporaryCloudFileInfo = NULL;
    }

    m_pInternal->m_oConverterToEditor.m_pManager = this->GetAppManager();
    m_pInternal->m_oConverterToEditor.m_pView = this;
    m_pInternal->m_oConverterFromEditor.m_pManager = this->GetAppManager();

    m_pInternal->LocalFile_Start();

    if (m_pInternal->m_bIsCloudCryptFile)
        return;

    std::wstring sUrl = this->GetAppManager()->m_oSettings.local_editors_path;
    if (0 == sUrl.find('/'))
        sUrl = L"file://" + sUrl;
    else
        sUrl = L"file:///" + sUrl;

    if (!sParams.empty())
        sUrl += L"?";

    // start convert file
    this->load(sUrl + sParams);
}
void CCefViewEditor::CreateLocalFile(const int& nFileFormat, const std::wstring& sName)
{
    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    std::wstring sFilePath = this->GetAppManager()->GetNewFilePath(nFileFormat);

    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    std::wstring sParams = L"placement=desktop";
    if (nFileFormat == etPresentation)
    {
        sParams = L"placement=desktop&doctype=presentation";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
    }
    else if (nFileFormat == etSpreadsheet)
    {
        sParams = L"placement=desktop&doctype=spreadsheet";
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
    }
    else if (nFileFormat == etDocumentMasterForm)
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF;
        sParams += L"&filetype=docxf";
    }
    
    if (!GetAppManager()->m_pInternal->GetEditorPermission())
        sParams += L"&mode=view";
    
    std::wstring sAdditionalParams = GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
    if (!sAdditionalParams.empty())
        sParams += (L"&" + sAdditionalParams);

    if (!sName.empty())
    {
        CefString sTmp = sName;
        CefString sEncode = CefURIEncode(sTmp, false);
        sParams += (L"&title=" + sEncode.ToWString());
    }

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
    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_pInternal->m_pManager->m_oSettings.recover_path, L"DE_");
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

    std::wstring::size_type nPosPoint = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.rfind('.');
    if (nPosPoint != std::wstring::npos && nPosPoint > m_pInternal->m_pManager->m_oSettings.recover_path.length())
    {
        m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir.substr(0, nPosPoint);
    }

    NSDirectory::CreateDirectory(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);
    m_pInternal->CheckLockRecoveryFile();

    m_pInternal->LocalFile_Start();

    // start convert file
    this->load(sUrl + sParams);
}
bool CCefViewEditor::OpenCopyAsRecoverFile(const int& nIdSrc)
{
    std::wstring sNewRecoveryDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(GetAppManager()->m_oSettings.recover_path, L"DE_");
    if (NSFile::CFileBinary::Exists(sNewRecoveryDir))
        NSFile::CFileBinary::Remove(sNewRecoveryDir);
    NSStringUtils::string_replace(sNewRecoveryDir, L"\\", L"/");
    std::wstring::size_type nPosPoint = sNewRecoveryDir.rfind('.');
    if (nPosPoint != std::wstring::npos && nPosPoint > GetAppManager()->m_oSettings.recover_path.length())
        sNewRecoveryDir = sNewRecoveryDir.substr(0, nPosPoint);

    CCefView* pViewSrc = m_pInternal->m_pManager->GetViewById(nIdSrc);
    if (!pViewSrc)
        return false;

    NSDirectory::CopyDirectory(pViewSrc->m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, sNewRecoveryDir);

    // check on cloud crypto
    if (NSFile::CFileBinary::Exists(sNewRecoveryDir + L"/EditorForCompare.bin"))
    {
        if (NSFile::CFileBinary::Exists(sNewRecoveryDir + L"/Editor.bin"))
            NSFile::CFileBinary::Remove(sNewRecoveryDir + L"/Editor.bin");
        NSFile::CFileBinary::Copy(sNewRecoveryDir + L"/EditorForCompare.bin", sNewRecoveryDir + L"/Editor.bin");
        if (NSFile::CFileBinary::Exists(sNewRecoveryDir + L"/EditorForCompare.bin"))
            NSFile::CFileBinary::Remove(sNewRecoveryDir + L"/EditorForCompare.bin");

        if (NSDirectory::Exists(sNewRecoveryDir + L"/openaslocal"))
            NSDirectory::DeleteDirectory(sNewRecoveryDir + L"/openaslocal");
    }

    m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved = false;

    std::wstring sUrl = GetAppManager()->m_oSettings.local_editors_path;
    if (0 == sUrl.find('/'))
        sUrl = L"file://" + sUrl;
    else
        sUrl = L"file:///" + sUrl;

    sUrl += L"?";

    int nFileType = pViewSrc->m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat;
    m_pInternal->m_oLocalInfo.m_oInfo.m_nCurrentFileFormat = nFileType;

    std::wstring sParams = L"";
    if (nFileType & AVS_OFFICESTUDIO_FILE_PRESENTATION)
        sParams = L"placement=desktop&doctype=presentation";
    else if (nFileType & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
        sParams = L"placement=desktop&doctype=spreadsheet";
    else if (nFileType & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
        sParams = L"placement=desktop&mode=view";
    else
        sParams = L"placement=desktop";

    std::wstring sAdditionalParams = GetAppManager()->m_pInternal->m_sAdditionalUrlParams;
    if (!sAdditionalParams.empty())
        sParams += (L"&" + sAdditionalParams);

    m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc = NSFile::GetFileName(pViewSrc->m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc);
    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = sNewRecoveryDir;

    m_pInternal->LocalFile_Start();

    // start convert file
    this->load(sUrl + sParams);
    return true;
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
    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = NSFile::GetDirectoryName(oInfo.m_sPath);
    if (NSFile::CFileBinary::Exists(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir))
        NSFile::CFileBinary::Remove(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir);

    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

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
    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc, L"\\", L"/");

    m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir = sFolder;
    NSStringUtils::string_replace(m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir, L"\\", L"/");

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

    m_pInternal->m_sRecentOpenExternalId = oInfo.m_sExternalCloudId;

    CRecentParent oRecentParent;
    oRecentParent.Url = oInfo.m_sUrl;
    oRecentParent.Parent = oInfo.m_sParentUrl;
    m_pInternal->m_pManager->m_pInternal->m_arRecentParents.push_back(oRecentParent);

    this->load(oInfo.m_sUrl);
    return true;
}

bool CCefViewEditor::CheckCloudCryptoNeedBuild()
{
    if (!m_pInternal->m_bIsCloudCryptFile)
        return false;
    if (m_pInternal->m_bIsCrashed || m_pInternal->m_bIsLoadingError)
        return false;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("is_need_build_crypted_file");
    m_pInternal->SendProcessMessage(PID_RENDERER, message);
    return true;
}
bool CCefViewEditor::IsBuilding()
{
    if (m_pInternal->m_bIsBuilding)
        return true;

    // вставляем сюда и сохранение
    if (m_pInternal->m_oConverterFromEditor.IsWorking())
        return true;

    return false;
}

std::wstring CCefViewEditor::GetLocalFilePath()
{
    if (!m_pInternal->m_oLocalInfo.m_oInfo.m_bIsSaved)
        return L"";
    return m_pInternal->m_oLocalInfo.m_oInfo.m_sFileSrc;
}
std::wstring CCefViewEditor::GetRecoveryDir()
{
    return m_pInternal->m_oLocalInfo.m_oInfo.m_sRecoveryDir;
}

int CCefViewEditor::GetFileFormat(const std::wstring& sFilePath)
{
    if (!NSFile::CFileBinary::Exists(sFilePath))
    {
        if (NSSystem::CLocalFileLocker::IsLocked(sFilePath))
        {
            std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"TMP");
            if (NSFile::CFileBinary::Exists(sTmpFile))
                NSFile::CFileBinary::Remove(sTmpFile);
            sTmpFile += (L"." + NSFile::GetFileExtention(sFilePath));
            NSFile::CFileBinary::Copy(sFilePath, sTmpFile);
            int nFormat = GetFileFormat(sTmpFile);
            NSFile::CFileBinary::Remove(sTmpFile);
            return nFormat;
        }
    }

    // формат файла по файлу
    // если файл зашифрован (MS_OFFCRYPTO) - то определяем по расширению
    COfficeFileFormatCheckerWrapper oChecker;
    if (oChecker.isOfficeFile2(sFilePath))
        return oChecker.nFileType2;
    return 0;
}

// NATIVE file converter
void CASCFileConverterToEditor::NativeViewerOpen(bool bIsEnabled)
{
    // вызывается из конвертера и из создания апи.
    // такой же счетчик, как и в файлах для редактора
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

    if (!m_pView->m_pInternal->m_bIsCloudCryptFile)
    {
        oFile.WriteFile((BYTE*)sBase64.c_str(), (DWORD)sBase64.length());
    }
    else
    {
        int nCount = (int)sBase64.length();
        int nPos = 0;
        std::string::size_type stPos = sBase64.find(";");
        if (stPos != std::string::npos)
            nPos = (int)stPos + 1;
        nCount -= nPos;

        BYTE* data = NULL;
        int data_len = 0;
        NSFile::CBase64Converter::Decode(sBase64.c_str() + nPos, nCount, data, data_len);
        oFile.WriteFile(data, (DWORD)data_len);
        RELEASEARRAYOBJECTS(data);
    }

    oFile.CloseFile();

    m_bIsNativeOpening = false;
}

// CAscApplicationManager_Private
void CAscApplicationManager_Private::ExecuteInAllFrames(const std::string& sCode)
{
    // вызвать код во всех view и во всех фреймах
    CCefView* pView = GetViewForSystemMessages();
    if (!pView || !pView->m_pInternal || !pView->m_pInternal->GetBrowser())
        return;

    CefRefPtr<CefFrame> pFrame = pView->m_pInternal->GetBrowser()->GetMainFrame();
    if (pFrame)
    {
        std::string sCodeAll = "window.AscDesktopEditor.CallInAllWindows(\"function(){" + sCode + "}\");";
        pFrame->ExecuteJavaScript(sCodeAll, pFrame->GetURL(), 0);
    }
}

void CAscApplicationManager_Private::ChangeEditorViewsCount()
{
    // событие, которое посылается во все view - когда есть подозрение на смену количества редакторов
    // чтобы порталы узнавали о режиме криптования
    bool bIsEditorPresent = false;
    for (std::map<int, CCefView*>::iterator iter = m_mapViews.begin(); iter != m_mapViews.end(); iter++)
    {
        CCefView* tmp = iter->second;
        if (tmp->GetType() == cvwtEditor)
        {
            bIsEditorPresent = true;
            break;
        }
    }

    CCefView* pView = GetViewForSystemMessages();
    if (!pView || !pView->m_pInternal || !pView->m_pInternal->GetBrowser())
        return;

    std::string sCodeValue = bIsEditorPresent ? "true" : "false";
    std::string sCode = "(function(){if (window.onChangeEditorsCount) window.onChangeEditorsCount(" + sCodeValue + ");})();";

    std::vector<int64> identifiers;
    pView->m_pInternal->GetBrowser()->GetFrameIdentifiers(identifiers);

    for (std::vector<int64>::iterator iter = identifiers.begin(); iter != identifiers.end(); iter++)
    {
        CefRefPtr<CefFrame> pFrame = pView->m_pInternal->GetBrowser()->GetFrame(*iter);
        if (pFrame)
        {
            pFrame->ExecuteJavaScript(sCode, pFrame->GetURL(), 0);
        }
    }
}

#if defined(_LINUX) && !defined(_MAC)
void* CefGetXDisplay(void)
{
    return (void*)cef_get_xdisplay();
}

#endif
