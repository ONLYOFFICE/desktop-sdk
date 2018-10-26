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

#include "./client_renderer.h"

#include <sstream>
#include <string>

#include "include/cef_dom.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_message_router.h"

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../../core/DesktopEditor/raster/ImageFileFormatChecker.h"
#include "../../src/applicationmanager_p.h"

#include "../../../../../core/DesktopEditor/raster/BgraFrame.h"
#include "../../../../../core/DesktopEditor/graphics/pro/Image.h"

#include "../../../../../core/HtmlRenderer/include/ASCSVGWriter.h"

#include "../../src/nativeviewer.h"
#include "../../src/plugins.h"

#include "../../src/additional/renderer.h"
#include "../crypto_mode.h"

namespace NSCommon
{
    template <typename Type>
    class smart_ptr
    {
    protected:
        Type*			m_pData;
        mutable LONG*	m_pCountRef;

    public:
        smart_ptr()
        {
            m_pData		= NULL;
            m_pCountRef = NULL;
        }
        smart_ptr(Type* pPointer)
        {
            m_pData		= pPointer;
            m_pCountRef = new LONG(1);
        }
        smart_ptr(const smart_ptr<Type>& pPointer)
        {
            m_pData		= NULL;
            m_pCountRef	= NULL;
            *this = pPointer;
        }
        ~smart_ptr()
        {
            Release();
        }

        inline void Release()
        {
            if (!IsInit() || (NULL == m_pCountRef))
                return;

            *m_pCountRef -= 1;
            if (0 >= *m_pCountRef)
            {
                delete m_pData;
                delete m_pCountRef;
            }
            m_pData		= NULL;
            m_pCountRef	= NULL;
        }
        inline void AddRef()
        {
            if (!IsInit() || (NULL == m_pCountRef))
                return;
            *m_pCountRef += 1;
        }

    public:
        smart_ptr<Type>& operator=(const Type& oSrc)
        {
            Release();

            m_pData		= new Type(oSrc);
            m_pCountRef = new LONG(1);

            return *this;
        }
        smart_ptr<Type>& operator=(Type* pType)
        {
            Release();

            m_pData		= pType;
            m_pCountRef = new LONG(1);

            return *this;
        }
        smart_ptr<Type>& operator=(const smart_ptr<Type>& oSrc)
        {
            Release();

            if ((NULL == oSrc.m_pData) || (NULL == oSrc.m_pCountRef))
                return *this;

            *oSrc.m_pCountRef += 1;
            Attach(oSrc.m_pData, oSrc.m_pCountRef);
            return *this;
        }

    public:

        inline bool IsInit() const
        {
            return (NULL != m_pData);
        }
        inline bool is_init() const
        {
            return IsInit();
        }

        template<class T> inline const bool is()const
        {
            if (!IsInit())
                return false;
            T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
            return (NULL != pResult);
        }
        template<class T> inline const T& as()const
        {
            T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
            return *pResult;
        }
        template<class T> inline T& as()
        {
            T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
            return *pResult;
        }

        template <typename T>
        inline void Attach(T* pCast, const LONG* pCountRef)
        {
            m_pData		= pCast;
            m_pCountRef	= const_cast<LONG*>(pCountRef);
        }

        template<typename T>
        inline smart_ptr<T> smart_dynamic_cast()const
        {
            smart_ptr<T> new_type;

            if ((NULL == m_pData) || (NULL == m_pCountRef))
                return new_type;

            T* pCast = dynamic_cast<T*>(m_pData);

            if (NULL == pCast)
                return new_type;

            *m_pCountRef += 1;

            new_type.Attach(pCast, m_pCountRef);

            return new_type;
        }

        inline Type& operator*()  { return *m_pData; }
        inline Type* operator->() { return  m_pData; }

        inline const Type& operator*()  const { return *m_pData; }
        inline const Type* operator->() const { return  m_pData; }

        inline const Type& get() { return  *m_pData; } const

        inline void reset(Type* pPointer = NULL)
        {
            *this = pPointer;
        }
    };
}

namespace asc_client_renderer
{
class CAscEditorNativeV8Handler : public CefV8Handler, public INativeViewer_Events
{
    enum EditorType
    {
        Document        = 0,
        Presentation    = 1,
        Spreadsheet     = 2
    };

    class CSavedPageInfo
    {
    public:
        std::wstring Url;
        int Page;
        int W;
        int H;

    public:
        CSavedPageInfo()
        {
            Page = -1;
            W = 0;
            H = 0;
        }
    };
    class CSavedPageTextInfo
    {
    public:
        int Page;
        std::string Info;
        int Paragraphs;
        int Words;
        int Spaces;
        int Symbols;

    public:
        CSavedPageTextInfo(const int& page, const int& paragraphs, const int& words, const int& symbols, const int& spaces, const std::string& sBase64Data)
        {
            Page = page;
            Info = sBase64Data;
            Paragraphs = paragraphs;
            Words = words;
            Spaces = spaces;
            Symbols = symbols;
        }
    };

public:
    EditorType  m_etType;
    int         m_nEditorId;
    bool*       sync_command_check;

    NSFile::CFileBinary m_oCurrentFileBinary;
    BYTE*               m_pCurrentFileData;
    int                 m_nCurrentIndex;

    int                 m_nCurrentPrintIndex;
    std::string         m_sVersion;
    std::wstring        m_sAppData;
    std::wstring        m_sFontsData;

    std::wstring        m_sSystemPlugins;
    std::wstring        m_sUserPlugins;

    int                 m_nSkipMouseWhellMax;
    int                 m_nSkipMouseWhellCounter;

    std::wstring        m_sLocalFileFolderWithoutFile;
    std::wstring        m_sLocalFileFolder;
    std::wstring        m_sLocalFileChanges;
    std::wstring        m_sLocalFileSrc;
    bool                m_bLocalIsSaved;
    int                 m_nCurrentChangesIndex; // текущий индекс изменения, чтобы не парсить файл. а понять, надо ли удалять или нет быстро.
    int                 m_nOpenChangesIndex; // количество изменений, при открытии
    bool                m_bIsPrinting;

    int                 m_nLocalImagesNextIndex;
    std::map<std::wstring, std::wstring> m_mapLocalAddImages;

    NSFonts::IApplicationFonts* m_pLocalApplicationFonts;

    std::string        m_sScrollStyle;

    std::vector<std::wstring> m_arDropFiles;

    CNativeViewer m_oNativeViewer;
    int m_nNativeOpenFileTimerID;
    int m_bIsNativeViewerMode;

    bool m_bIsSupportSigs;
    bool m_bIsSupportOnlyPass;
    bool m_bIsSupportProtect;
    int m_nCryptoMode;

    std::list<CSavedPageInfo> m_arCompleteTasks;

    std::list<CSavedPageTextInfo> m_arCompleteTextTasks;

    bool m_bIsDebugMode;

    std::wstring m_sCryptDocumentFolder; // recover

    std::wstring m_sCookiesPath;

    NSCriticalSection::CRITICAL_SECTION m_oCompleteTasksCS;

    CAscEditorNativeV8Handler()
    {
        m_etType = Document;
        m_nEditorId = -1;
        sync_command_check = NULL;

        m_pCurrentFileData  = NULL;
        m_nCurrentIndex     = 0;
        m_nCurrentPrintIndex = 0;
        m_sVersion = "";
        m_sAppData = L"";
        m_sFontsData = L"";

        m_nSkipMouseWhellMax = 2;
        m_nSkipMouseWhellCounter = 0;
        m_nCurrentChangesIndex = 0;
        m_nOpenChangesIndex = 0;

        m_bLocalIsSaved = false;

        m_nLocalImagesNextIndex = 0;
        m_pLocalApplicationFonts = NULL;

        m_nNativeOpenFileTimerID = -1;
        m_bIsNativeViewerMode = false;

        m_bIsPrinting = false;

        m_bIsSupportSigs = true;

        m_bIsDebugMode = false;
        m_bIsSupportOnlyPass = true;
        m_bIsSupportProtect = true;

        m_nCryptoMode = 0;

        m_oCompleteTasksCS.InitializeCriticalSection();

#if 0
        m_sScrollStyle = "::-webkit-scrollbar { width: 12px; height: 12px; } \
::-webkit-scrollbar-track { -webkit-box-shadow: inset 0 0 6px rgba(0,0,0,0.3); border-radius: 10px; } \
::-webkit-scrollbar-thumb { border-radius: 10px; -webkit-box-shadow: inset 0 0 6px rgba(0,0,0,0.5); }";
#endif

#if 1
        m_sScrollStyle = "\
::-webkit-scrollbar { background: transparent; width: 16px; height: 16px; } \
::-webkit-scrollbar-button { width: 5px; height:5px; } \
::-webkit-scrollbar-track {	background:#F5F5F5; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-thumb { background:#BFBFBF; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-thumb:hover { background:#A7A7A7; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-corner { background:inherit; }";
#endif
    }

    virtual ~CAscEditorNativeV8Handler()
    {
        NSBase::Release(m_pLocalApplicationFonts);
        m_oCompleteTasksCS.DeleteCriticalSection();
    }

    virtual void OnDocumentOpened(const std::string& sBase64)
    {        
    }

    virtual void OnPageSaved(const std::wstring& sUrl, const int& nPageNum, const int& nPageW, const int& nPageH)
    {
        CTemporaryCS oCS(&m_oCompleteTasksCS);
        CSavedPageInfo oInfo;
        oInfo.Url = sUrl;
        oInfo.Page = nPageNum;
        oInfo.W = nPageW;
        oInfo.H = nPageH;
        m_arCompleteTasks.push_back(oInfo);
    }
    virtual void OnPageText(const int& page, const int& paragraphs, const int& words, const int& symbols, const int& spaces, const std::string& sBase64Data)
    {
        CTemporaryCS oCS(&m_oCompleteTasksCS);
        CSavedPageTextInfo oInfo(page, paragraphs, words, symbols, spaces, sBase64Data);
        m_arCompleteTextTasks.push_back(oInfo);
    }

    virtual bool Execute(const CefString& sMessageName,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) OVERRIDE
    {
        std::string name = sMessageName.ToString();
        if (name == "Copy")
        {
            CefV8Context::GetCurrentContext()->GetFrame()->Copy();
            return  true;
        }
        else if (name == "Paste")
        {
            CefV8Context::GetCurrentContext()->GetFrame()->Paste();
            return  true;
        }
        else if (name == "Cut")
        {
            CefV8Context::GetCurrentContext()->GetFrame()->Cut();
            return  true;
        }
        else if (name == "SetEditorType")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            if (val->IsValid() && val->IsString())
            {
                CefString editorType = val->GetStringValue();
                std::string strEditorType = editorType.ToString();

                if (strEditorType == "document")
                    m_etType = Document;
                else if (strEditorType == "presentation")
                    m_etType = Presentation;
                else if (strEditorType == "spreadsheet")
                    m_etType = Spreadsheet;
            }

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
            message->GetArgumentList()->SetInt(0, (int)m_etType);
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "SetEditorId")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            if (val->IsValid() && val->IsInt())
            {
                m_nEditorId = val->GetIntValue();
            }
            return true;
        }
        else if (name == "GetEditorId")
        {
            retval = CefV8Value::CreateInt(m_nEditorId);
            return true;
        }
        else if (name == "LoadJS")
        {
            bool bIsLocal = false;
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

            if (0 == browser->GetMainFrame()->GetURL().ToString().find("file://"))
            {
                bIsLocal = true;
            }

            if (m_sVersion.empty())
            {                
                CefRefPtr<CefV8Value> retval3;
                CefRefPtr<CefV8Exception> exception;

                bool bIsVersion = browser->GetMainFrame()->GetV8Context()->Eval(
"(function() { \n\
if (window.DocsAPI && window.DocsAPI.DocEditor) \n\
    return window.DocsAPI.DocEditor.version(); \n\
else \n\
    return undefined; \n\
})();",
#ifndef CEF_2623
            "", 0,
#endif
            retval3, exception);
                if (bIsVersion)
                {
                    if (retval3->IsString())
                        m_sVersion = retval3->GetStringValue().ToString();
                }

                if (m_sVersion.empty())
                    m_sVersion = "undefined";

                CefRefPtr<CefV8Value> retval2;
                CefRefPtr<CefV8Exception> exception2;

                bool bIsAppData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_AppData\"]();",
                                                                          #ifndef CEF_2623
                                                                                      "", 0,
                                                                          #endif
                                                                          retval2, exception2);
                if (bIsAppData)
                {
                    if (retval2->IsString())
                    {
                        std::string sAppDataA = retval2->GetStringValue().ToString();
                        m_sAppData = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sAppDataA.c_str(), (LONG)sAppDataA.length());
                    }
                }
                retval2 = NULL;
                exception2 = NULL;
                bool bIsFontsData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_FontsData\"]();",
                                                                            #ifndef CEF_2623
                                                                                        "", 0,
                                                                            #endif
                                                                            retval2, exception2);
                if (bIsAppData)
                {
                    if (retval2->IsString())
                    {
                        std::string sFontsDataA = retval2->GetStringValue().ToString();
                        m_sFontsData = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sFontsDataA.c_str(), (LONG)sFontsDataA.length());
                    }
                }

                retval2 = NULL;
                exception2 = NULL;
                bool bIsSPData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_SP\"]();",
                                                                         #ifndef CEF_2623
                                                                                     "", 0,
                                                                         #endif
                                                                         retval2, exception2);
                if (bIsSPData)
                {
                    if (retval2->IsString())
                    {
                        std::string sDataA = retval2->GetStringValue().ToString();
                        m_sSystemPlugins = UTF8_TO_U(sDataA);
                    }
                }

                retval2 = NULL;
                exception2 = NULL;
                bool bIsUPData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_UP\"]();",
                                                                         #ifndef CEF_2623
                                                                                     "", 0,
                                                                         #endif
                                                                         retval2, exception2);
                if (bIsUPData)
                {
                    if (retval2->IsString())
                    {
                        std::string sDataA = retval2->GetStringValue().ToString();
                        m_sUserPlugins = UTF8_TO_U(sDataA);
                    }
                }

                retval2 = NULL;
                exception2 = NULL;
                bool bIsDebugMode = CefV8Context::GetCurrentContext()->Eval("(function() { return (window[\"desktop_debug_mode\"] === true) ? true : false; })();",
                                                                            #ifndef CEF_2623
                                                                                        "", 0,
                                                                            #endif
                                                                            retval2, exception2);
                if (bIsDebugMode)
                {
                    if (retval2->IsBool())
                    {
                        m_bIsDebugMode = retval2->GetBoolValue();
                    }
                }
            }

            CefRefPtr<CefV8Value> val = *arguments.begin();
            if (!val->IsValid() || !val->IsString())
            {
                retval = CefV8Value::CreateInt(0);
                return true;
            }

            CefString scriptUrl = val->GetStringValue();
            std::wstring strUrl = scriptUrl.ToWString();

            // 0 - грузить из облака
            // 1 - загружен и исполнен
            // 2 - ждать ответа
            int nResult = 0;
            std::wstring strPath = L"";

            if (strUrl.find(L"sdk/Common/AllFonts.js") != std::wstring::npos ||
                strUrl.find(L"sdkjs/common/AllFonts.js") != std::wstring::npos)
            {
                strPath = m_sFontsData + L"/AllFonts.js";
                nResult = 2;
            }
            else if (m_sVersion == "undefined" || m_sVersion == "reporter_cloud" || m_bIsDebugMode || bIsLocal)
            {
                std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
                if ((m_sVersion == "undefined") && (sUrl.find("index.reporter.html") != std::string::npos))
                {
                    m_sVersion = "reporter_cloud";
                    if (m_etType != Presentation)
                    {
                        m_etType = Presentation;
                        CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
                        message->GetArgumentList()->SetInt(0, (int)m_etType);
                        browser->SendProcessMessage(PID_BROWSER, message);
                    }
                }
                else if (strUrl.find(L"app.js") != std::wstring::npos)
                {
                    // сначала определим тип редактора
                    if (sUrl.find("documenteditor") != std::wstring::npos)
                        m_etType = Document;
                    else if (sUrl.find("presentationeditor") != std::wstring::npos)
                        m_etType = Presentation;
                    else if (sUrl.find("spreadsheeteditor") != std::wstring::npos)
                        m_etType = Spreadsheet;

                    CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
                    message->GetArgumentList()->SetInt(0, (int)m_etType);
                    browser->SendProcessMessage(PID_BROWSER, message);
                }
                retval = CefV8Value::CreateInt(0);
                return true;
            }

            if (0 == nResult)
            {
                std::wstring strAppPath = m_sAppData + L"/webdata/cloud";
                std::wstring strAppPathEditors = strAppPath + L"/" + NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)m_sVersion.c_str(), (LONG)m_sVersion.length());
                if (!NSDirectory::Exists(strAppPathEditors))
                {
                    NSDirectory::CreateDirectory(strAppPathEditors);
                    NSDirectory::CreateDirectory(strAppPathEditors + L"/word");
                    NSDirectory::CreateDirectory(strAppPathEditors + L"/cell");
                    NSDirectory::CreateDirectory(strAppPathEditors + L"/slide");
                }

                if (strUrl.find(L"sdk-all.js") != std::wstring::npos)
                {
                    if (m_etType == Document)
                        strPath = strAppPathEditors + L"/word/sdk-all.js";
                    else if (m_etType == Presentation)
                        strPath = strAppPathEditors + L"/slide/sdk-all.js";
                    else if (m_etType == Spreadsheet)
                        strPath = strAppPathEditors + L"/cell/sdk-all.js";

                    nResult = 2;
                }
                else if (strUrl.find(L"sdk-all-min.js") != std::wstring::npos)
                {
                    if (m_etType == Document)
                        strPath = strAppPathEditors + L"/word/sdk-all-min.js";
                    else if (m_etType == Presentation)
                        strPath = strAppPathEditors + L"/slide/sdk-all-min.js";
                    else if (m_etType == Spreadsheet)
                        strPath = strAppPathEditors + L"/cell/sdk-all-min.js";

                    nResult = 2;
                }
                else if (strUrl.find(L"app.js") != std::wstring::npos)
                {
                    std::wstring sStringUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToWString();

                    // сначала определим тип редактора
                    if (sStringUrl.find(L"documenteditor") != std::wstring::npos)
                        m_etType = Document;
                    else if (sStringUrl.find(L"presentationeditor") != std::wstring::npos)
                        m_etType = Presentation;
                    else if (sStringUrl.find(L"spreadsheeteditor") != std::wstring::npos)
                        m_etType = Spreadsheet;

                    CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
                    message->GetArgumentList()->SetInt(0, (int)m_etType);
                    browser->SendProcessMessage(PID_BROWSER, message);

                    if (m_etType == Document)
                        strPath = strAppPathEditors + L"/word/app.js";
                    else if (m_etType == Presentation)
                        strPath = strAppPathEditors + L"/slide/app.js";
                    else if (m_etType == Spreadsheet)
                        strPath = strAppPathEditors + L"/cell/app.js";

                    nResult = 2;
                }
            }

            if (strPath != L"" && nResult != 0)
            {
                if (nResult == 1)
                {
                    NSFile::CFileBinary oFile;
                    if (oFile.OpenFile(strPath))
                    {
                        int nSize = (int)oFile.GetFileSize();
                        BYTE* scriptData = new BYTE[nSize];
                        DWORD dwReadSize = 0;
                        oFile.ReadFile(scriptData, (DWORD)nSize, dwReadSize);

                        std::string strUTF8((char*)scriptData, (LONG)nSize);

                        delete [] scriptData;

                        CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(strUTF8, "", 0);
                        retval = CefV8Value::CreateInt(1);
                    }
                    else
                    {
                        retval = CefV8Value::CreateInt(0);
                    }
                }
                else
                {
                    CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("load_js");
                    int64 frameId = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
                    message->GetArgumentList()->SetString(0, GetFullUrl2(strUrl, CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToWString()));
                    message->GetArgumentList()->SetString(1, strPath);
                    message->GetArgumentList()->SetInt(2, (int)frameId);
                    browser->SendProcessMessage(PID_BROWSER, message);

                    retval = CefV8Value::CreateInt(2);
                }
            }
            else
            {
                retval = CefV8Value::CreateInt(0);
            }
            return true;
        }
        else if (name == "LoadFontBase64")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString fileUrl = val->GetStringValue();
            std::wstring strUrl = fileUrl.ToWString();

            if (0 != strUrl.find(L"embedded"))
            {
                NSFile::CFileBinary oFile;
                oFile.OpenFile(strUrl);

                int nSize1 = oFile.GetFileSize();
                DWORD dwSize = 0;
                BYTE* pFontData = new BYTE[nSize1];
                oFile.ReadFile(pFontData, (DWORD)nSize1, dwSize);
                oFile.CloseFile();

                char* pDataDst = NULL;
                int nDataDst = 0;
                NSFile::CBase64Converter::Encode(pFontData, nSize1, pDataDst, nDataDst, NSBase64::B64_BASE64_FLAG_NOCRLF);

                std::string sFontBase64(pDataDst, nDataDst);
                RELEASEARRAYOBJECTS(pDataDst);

                int pos1 = strUrl.find_last_of(L"\\");
                int pos2 = strUrl.find_last_of(L"/");

                int nMax = (pos1 > pos2) ? pos1 : pos2;

                //std::wstring sName = strUrl.substr(nMax + 1);
                std::string sName = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strUrl.c_str(), strUrl.length());
                sName = "window[\"" + sName + "\"] = \"" + std::to_string(nSize1) + ";" + sFontBase64 + "\";";

                CefString sJS;
                sJS.FromString(sName);

                CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(sJS, "", 0);
            }
            else
            {
                std::wstring sFileFont = m_sLocalFileFolder + L"/fonts/" + strUrl + L".js";

                if (0 == sFileFont.find(L"file:///"))
                {
                    sFileFont = sFileFont.substr(7);
                    if (!NSFile::CFileBinary::Exists(sFileFont))
                        sFileFont = sFileFont.substr(1);
                }

                std::string sFileCodeJS;
                NSFile::CFileBinary::ReadAllTextUtf8A(sFileFont, sFileCodeJS);

                CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(sFileCodeJS, "", 0);
            }

            return true;
        }
        else if (name == "OpenBinaryFile")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString fileUrl = val->GetStringValue();
            std::wstring strUrl = fileUrl.ToWString();

            m_oCurrentFileBinary.CloseFile();
            m_oCurrentFileBinary.OpenFile(strUrl);

            uint32 nSize = (uint32)m_oCurrentFileBinary.GetFileSize();
            m_nCurrentIndex = 0;

            retval = CefV8Value::CreateUInt(nSize);

            m_pCurrentFileData = new BYTE[nSize];

            return true;
        }
        else if (name == "CloseBinaryFile")
        {
            m_oCurrentFileBinary.CloseFile();
            RELEASEARRAYOBJECTS(m_pCurrentFileData);
            m_nCurrentIndex = 0;

            return true;
        }
        else if (name == "GetBinaryFileData")
        {
            retval = CefV8Value::CreateInt(m_pCurrentFileData[m_nCurrentIndex++]);

            return true;
        }
        else if (name == "getFontsSprite")
        {
            bool bIsRetina = false;
            if (arguments.size() > 0)
            {
                CefRefPtr<CefV8Value> val = *arguments.begin();
                bIsRetina = val->GetBoolValue();
            }

            std::wstring strUrl = (false == bIsRetina) ?
                        (m_sFontsData + L"/fonts_thumbnail.png") :
                        (m_sFontsData + L"/fonts_thumbnail@2x.png");

            NSFile::CFileBinary oFile;
            oFile.OpenFile(strUrl);

            int nSize1 = oFile.GetFileSize();
            DWORD dwSize = 0;
            BYTE* pFontData = new BYTE[nSize1];
            oFile.ReadFile(pFontData, (DWORD)nSize1, dwSize);
            oFile.CloseFile();

            char* pDataDst = NULL;
            int nDataDst = 0;
            NSFile::CBase64Converter::Encode(pFontData, nSize1, pDataDst, nDataDst);

            std::string sFontBase64(pDataDst, nDataDst);
            RELEASEARRAYOBJECTS(pDataDst);

            sFontBase64 = "data:image/jpeg;base64," + sFontBase64;

            retval = CefV8Value::CreateString(sFontBase64.c_str());
            return true;
        }
        else if (name == "SpellCheck")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString sTask = val->GetStringValue();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            int64 frameId = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("spell_check_task");
            message->GetArgumentList()->SetInt(0, (int)m_nEditorId);
            message->GetArgumentList()->SetString(1, sTask);
            message->GetArgumentList()->SetInt(2, (int)frameId);
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "CreateEditorApi")
        {
            volatile bool* pChecker = this->sync_command_check;
            *pChecker = true;

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("create_editor_api");
            browser->SendProcessMessage(PID_BROWSER, message);
            
#if 0
            CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
            if (_frame)
            {
                _frame->ExecuteJavaScript("window.AscDesktopEditorButtonMode = true;", _frame->GetURL(), 0);
            }
#endif
            
            return true;
        }
        else if (name == "ConsoleLog")
        {
#if 0
            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString sTask = val->GetStringValue();

            FILE* ff = fopen("D:\\cef_console.log", "a+");
            fprintf(ff, sTask.ToString().c_str());
            fprintf(ff, "\n");
            fclose(ff);
#endif
            return true;
        }
        else if (name == "GetFrameContent")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString sFrame = val->GetStringValue();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefFrame> _frame = browser->GetFrame(sFrame);
            if (_frame)
            {
                std::string sName = _frame->GetName().ToString();
                std::string sCode = "window[\"AscDesktopEditor\"][\"SetFrameContent\"](document.body.firstChild ? JSON.stringify(document.body.firstChild.innerHTML) : \"\", \"" +
                        sName + "\");";
                //std::string sCode = "alert(document.body.innerHTML);";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }

            return true;
        }
        else if (name == "SetFrameContent")
        {
            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            std::string sFrame =  (*iter)->GetStringValue(); ++iter;
            std::string sFrameName =  (*iter)->GetStringValue();

            if (sFrame.find("\"") != 0)
            {
                sFrame = ("\"" + sFrame + "\"");
            }

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
            if (_frame)
            {
                std::string sCode = "if (window[\"onchildframemessage\"]) { window[\"onchildframemessage\"](" +
                        sFrame + ", \"" + sFrameName + "\"); }";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }

            return true;
        }
        else if (name == "setCookie")
        {
            if (arguments.size() != 5)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_cookie");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(2, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(3, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(4, (*iter)->GetStringValue()); ++iter;
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "setAuth")
        {
            if (arguments.size() != 4)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_cookie");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(2, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(3, "asc_auth_key");
            message->GetArgumentList()->SetString(4, (*iter)->GetStringValue()); ++iter;
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "getCookiePresent")
        {
            if (arguments.size() != 2)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("is_cookie_present");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "getAuth")
        {
            if (arguments.size() != 1)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("is_cookie_present");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(1, "asc_auth_key");
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "checkAuth")
        {
            if (arguments.size() != 1)
                return true;

            CefRefPtr<CefV8Value> _param = *arguments.begin();

            std::vector<CefString> arrKeys;
            _param->GetKeys(arrKeys);

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_check_auth");
            message->GetArgumentList()->SetInt(0, (int)arrKeys.size());

            int nCurrent = 1;
            for (std::vector<CefString>::iterator i = arrKeys.begin(); i != arrKeys.end(); i++)
                message->GetArgumentList()->SetString(nCurrent++, *i);

            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "onDocumentModifiedChanged")
        {
            if (arguments.size() != 1)
                return true;

            if (IsChartEditor())
                return true;

            CefRefPtr<CefV8Value> val = *arguments.begin();
            bool bValue = val->GetBoolValue();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onDocumentModifiedChanged");
            message->GetArgumentList()->SetBool(0, bValue);
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "SetDocumentName")
        {
            if (IsChartEditor())
                return true;

            CefRefPtr<CefV8Value> val = *arguments.begin();
            CefString sName = val->GetStringValue();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_document_name");
            message->GetArgumentList()->SetString(0, sName);
            browser->SendProcessMessage(PID_BROWSER, message);

            // HACK!!!
            if (m_nCryptoMode > 0)
            {
                CefRefPtr<CefFrame> _frame =  CefV8Context::GetCurrentContext()->GetFrame();
                _frame->ExecuteJavaScript("(function() { try { \
DE.controllers.Main.editorConfig.canUseHistory = false; \
DE.controllers.Main.editorConfig.fileChoiceUrl = \"\"; \
DE.controllers.Main.editorConfig.mergeFolderUrl = \"\"; \
DE.controllers.Main.appOptions.fileChoiceUrl = \"\"; \
DE.controllers.Main.appOptions.mergeFolderUrl = \"\"; \
} catch(err){} })();", _frame->GetURL(), 0);
            }

            return true;
        }
        else if (name == "OnSave")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_document_save");
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "js_message")
        {
            if (arguments.size() != 2)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("js_message");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "Print_Start")
        {
            if (arguments.size() != 4)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_start");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetInt(1, (*iter)->GetIntValue()); ++iter;
            message->GetArgumentList()->SetString(2, browser->GetFocusedFrame()->GetURL());
            message->GetArgumentList()->SetString(3, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetInt(4, (*iter)->GetIntValue());
            browser->SendProcessMessage(PID_BROWSER, message);

            m_nCurrentPrintIndex = 0;
            m_bIsPrinting = true;
            return true;
        }
        else if (name == "Print_Page")
        {
            if (arguments.size() != 3)
                return true;

            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_page");
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
            message->GetArgumentList()->SetInt(1, m_nCurrentPrintIndex);
            message->GetArgumentList()->SetDouble(2, (*iter)->GetDoubleValue()); ++iter;
            message->GetArgumentList()->SetDouble(3, (*iter)->GetDoubleValue()); ++iter;
            browser->SendProcessMessage(PID_BROWSER, message);

            m_nCurrentPrintIndex++;
            return true;
        }
        else if (name == "Print_End")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_end");
            browser->SendProcessMessage(PID_BROWSER, message);

            m_nCurrentPrintIndex = 0;
            m_bIsPrinting = false;
            return true;
        }
        else if (name == "Print")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "Set_App_Data")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            m_sAppData = val->GetStringValue().ToWString();

            return true;
        }
        else if (name == "IsSupportNativePrint")
        {
            if (m_sLocalFileFolder.empty())
                retval = CefV8Value::CreateBool(false);
            else
                retval = CefV8Value::CreateBool(true);
            return true;
        }
        else if (name == "CheckNeedWheel")
        {
            //это код, когда под виндоус приходило два раза. Теперь поправили
#if 0
#ifdef WIN32
            m_nSkipMouseWhellCounter++;
            if (m_nSkipMouseWhellCounter == m_nSkipMouseWhellMax)
            {
                m_nSkipMouseWhellCounter = 0;
                retval = CefV8Value::CreateBool(false);
            }
            else
            {
                retval = CefV8Value::CreateBool(true);
            }
#else
            retval = CefV8Value::CreateBool(true);
#endif
#endif
            retval = CefV8Value::CreateBool(true);
            return true;
        }
        else if (name == "GetImageBase64")
        {
            if (arguments.size() < 1)
            {
                retval = CefV8Value::CreateString("");
                return true;
            }

            CefRefPtr<CefV8Value> val = *arguments.begin();
            std::wstring sFileUrl = val->GetStringValue().ToWString();

            if (sFileUrl.find(L"file://") == 0)
            {
                if (NSFile::CFileBinary::Exists(sFileUrl.substr(7)))
                    sFileUrl = sFileUrl.substr(7);
                else if (NSFile::CFileBinary::Exists(sFileUrl.substr(8)))
                    sFileUrl = sFileUrl.substr(8);
            }

            std::string sHeader = "";

            NSFile::CFileBinary oFileBinary;
            if (!oFileBinary.OpenFile(sFileUrl))
            {
                retval = CefV8Value::CreateString("");
                return true;
            }

            int nDetectSize = 50;
            if (nDetectSize > (int)oFileBinary.GetFileSize())
            {
                retval = CefV8Value::CreateString("");
                return true;
            }

            BYTE* pData = new BYTE[nDetectSize];
            memset(pData, 0, nDetectSize);
            DWORD dwRead = 0;
            oFileBinary.ReadFile(pData, (DWORD)nDetectSize, dwRead);
            oFileBinary.CloseFile();

            bool bIsNoHeader = false;
            if (arguments.size() >= 2)
                bIsNoHeader = arguments[1]->GetBoolValue();

            if (!bIsNoHeader)
            {
                CImageFileFormatChecker _checker;

                if (_checker.isBmpFile(pData, nDetectSize))
                    sHeader = "data:image/bmp;base64,";
                else if (_checker.isJpgFile(pData, nDetectSize))
                    sHeader = "data:image/jpeg;base64,";
                else if (_checker.isPngFile(pData, nDetectSize))
                    sHeader = "data:image/png;base64,";
                else if (_checker.isGifFile(pData, nDetectSize))
                    sHeader = "data:image/gif;base64,";
                else if (_checker.isTiffFile(pData, nDetectSize))
                    sHeader = "data:image/tiff;base64,";
            }

            RELEASEARRAYOBJECTS(pData);

            if (sHeader.empty() && !bIsNoHeader)
            {
                retval = CefV8Value::CreateString("");
                return true;
            }

            NSFile::CFileBinary oFile;
            oFile.OpenFile(sFileUrl);

            int nSize1 = oFile.GetFileSize();
            DWORD dwSize = 0;
            BYTE* pFontData = new BYTE[nSize1];
            oFile.ReadFile(pFontData, (DWORD)nSize1, dwSize);
            oFile.CloseFile();

            char* pDataDst = NULL;
            int nDataDst = 0;
            NSFile::CBase64Converter::Encode(pFontData, nSize1, pDataDst, nDataDst, NSBase64::B64_BASE64_FLAG_NOCRLF);

            std::string sFontBase64(pDataDst, nDataDst);
            RELEASEARRAYOBJECTS(pDataDst);

            sFontBase64 = sHeader + sFontBase64;

            retval = CefV8Value::CreateString(sFontBase64.c_str());
            return true;
        }
        else if (name == "SetFullscreen")
        {
            bool bIsFullScreen = false;
            if (arguments.size() > 0)
            {
                CefRefPtr<CefV8Value> val = *arguments.begin();
                bIsFullScreen = val->GetBoolValue();
            }

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

            if (bIsFullScreen)
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onfullscreenenter");
                browser->SendProcessMessage(PID_BROWSER, message);
            }
            else
            {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onfullscreenleave");
                browser->SendProcessMessage(PID_BROWSER, message);
            }

            return true;
        }
        else if (name == "LocalStartOpen")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_loadstart");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileOpen")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_open");
            if (arguments.size() == 1)
            {
                CefRefPtr<CefV8Value> val = *arguments.begin();
                message->GetArgumentList()->SetString(0, val->GetStringValue());
            }
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileCreate")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nId = val->GetIntValue();
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_create");
            message->GetArgumentList()->SetInt(0, nId);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRecoverFolder")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            m_sLocalFileFolderWithoutFile = val->GetStringValue().ToWString();

            m_sLocalFileChanges = m_sLocalFileFolderWithoutFile + L"/changes/changes0.json";
            if (!NSDirectory::Exists(m_sLocalFileFolderWithoutFile + L"/changes"))
                NSDirectory::CreateDirectory(m_sLocalFileFolderWithoutFile + L"/changes");

            std::vector<std::wstring> arMedia = NSDirectory::GetFiles(m_sLocalFileFolderWithoutFile + L"/media");
            m_nLocalImagesNextIndex = (int)arMedia.size() + 1;

            if (0 == m_sLocalFileFolderWithoutFile.find('/'))
                m_sLocalFileFolder = L"file://" + m_sLocalFileFolderWithoutFile;
            else
                m_sLocalFileFolder = L"file:///" + m_sLocalFileFolderWithoutFile;

            return true;
        }
        else if (name == "CheckUserId")
        {
            std::wstring sUserPath = m_sLocalFileFolderWithoutFile + L"/user_name.log";
            int nUserIndex = 1;
            std::string sUserLog = "";

            if (NSFile::CFileBinary::ReadAllTextUtf8A(sUserPath, sUserLog))
            {
                NSFile::CFileBinary::Remove(sUserPath);
                nUserIndex = std::stoi(sUserLog);
                nUserIndex++;
            }

            std::wstring sUserLogW = std::to_wstring(nUserIndex);
            NSFile::CFileBinary::SaveToFile(sUserPath, sUserLogW);

            retval = CefV8Value::CreateString(sUserLogW);

            return true;
        }
        else if (name == "LocalFileRecents")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecents");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileOpenRecent")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nId = val->GetIntValue();
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_openrecents");
            message->GetArgumentList()->SetInt(0, nId);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRemoveRecent")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nId = val->GetIntValue();
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removerecents");
            message->GetArgumentList()->SetInt(0, nId);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRemoveAllRecents")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removeallrecents");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRecovers")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecovers");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileOpenRecover")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nId = val->GetIntValue();
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_openrecovers");
            message->GetArgumentList()->SetInt(0, nId);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRemoveRecover")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nId = val->GetIntValue();
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removerecovers");
            message->GetArgumentList()->SetInt(0, nId);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileRemoveAllRecovers")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removeallrecovers");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileSaveChanges")
        {
            std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

            std::string sParam = (*iter)->GetStringValue().ToString(); ++iter;
            int nDeleteIndex = (*iter)->GetIntValue(); ++iter;
            int nCount = (*iter)->GetIntValue();

            if (nDeleteIndex < m_nCurrentChangesIndex)
            {
                // нужно удалить изменения
                RemoveChanges(nDeleteIndex);
            }
            m_nCurrentChangesIndex = nDeleteIndex + nCount;

            if (nCount != 0)
            {
                FILE* _file = NSFile::CFileBinary::OpenFileNative(m_sLocalFileChanges, L"a+");
                if (NULL != _file)
                {
                    fprintf(_file, "\"");
                    fprintf(_file, sParam.c_str());
                    fprintf(_file, "\",");
                    fclose(_file);
                }
            }
            return true;
        }
        else if (name == "LocalFileGetOpenChangesCount")
        {
            retval = CefV8Value::CreateInt(m_nOpenChangesIndex);
            return true;
        }
        else if (name == "LocalFileSetOpenChangesCount")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            m_nOpenChangesIndex = val->GetIntValue();
            return true;
        }
        else if (name == "LocalFileGetCurrentChangesIndex")
        {
            retval = CefV8Value::CreateInt(m_nCurrentChangesIndex);
            return true;
        }
        else if (name == "LocalFileSave")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onsavestart");

            CefRefPtr<CefV8Value> val = *arguments.begin();
            message->GetArgumentList()->SetString(0, val->GetStringValue());

            if (arguments.size() > 1)
            {
                CefRefPtr<CefV8Value> val_pass = arguments[1];
                if (val_pass->IsString())
                    message->GetArgumentList()->SetString(1, val_pass->GetStringValue());
                else
                    message->GetArgumentList()->SetString(1, "");
            }
            else
                message->GetArgumentList()->SetString(1, "");

            if (arguments.size() > 2)
            {
                CefRefPtr<CefV8Value> val_info = arguments[2];
                if (val_info->IsString())
                    message->GetArgumentList()->SetString(2, val_info->GetStringValue());
                else
                    message->GetArgumentList()->SetString(2, "");
            }
            else
                message->GetArgumentList()->SetString(2, "");

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "LocalFileGetSourcePath")
        {
            retval = CefV8Value::CreateString(m_sLocalFileSrc);
            return true;
        }
        else if (name == "LocalFileSetSourcePath")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            m_sLocalFileSrc = val->GetStringValue().ToWString();
            return true;
        }
        else if (name == "LocalFileGetSaved")
        {
            retval = CefV8Value::CreateBool(m_bLocalIsSaved);
            return true;
        }
        else if (name == "LocalFileSetSaved")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            // saved -> unsaved перейти не может
            if (!m_bLocalIsSaved)
                m_bLocalIsSaved = val->GetBoolValue();
            return true;
        }
        else if (name == "LocalFileGetImageUrl")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            std::wstring sUrl = GetLocalImageUrl(val->GetStringValue().ToWString());
            retval = CefV8Value::CreateString(sUrl);
            return true;
        }
        else if (name == "LocalFileGetImageUrlFromOpenFileDialog")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onaddimage");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "AscBrowserScrollStyle")
        {
            if (m_sScrollStyle.empty())
                return true;

            std::string sCode = "\
window.addEventListener('DOMContentLoaded', function(){ \
var _style = document.createElement('style');_style.type = 'text/css';\
_style.innerHTML = '" + m_sScrollStyle + "'; document.getElementsByTagName('head')[0].appendChild(_style); }, false);";

            CefRefPtr<CefFrame> _frame =  CefV8Context::GetCurrentContext()->GetFrame();
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            return true;
        }
        else if (name == "execCommand")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_exec_command");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;

            if (2 == arguments.size())
            {
                message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            }

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "Logout")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_logout");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*arguments.begin())->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "SetDropFiles")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            int nCount = val->GetArrayLength();

            /*
            FILE* _file = fopen("D:\\dropFiles.txt", "a+");
            fprintf(_file, "---------------------------------\n");

            for (int i = 0; i < nCount; ++i)
            {
                std::string sValue = val->GetValue(i)->GetStringValue().ToString();
                fprintf(_file, sValue.c_str());
                fprintf(_file, "\n");
            }

            fclose(_file);
            */

            m_arDropFiles.clear();
            for (int i = 0; i < nCount; ++i)
            {
                std::wstring sValue = val->GetValue(i)->GetStringValue().ToWString();
                m_arDropFiles.push_back(sValue);
            }
            return true;
        }
        else if (name == "IsImageFile")
        {
            CefRefPtr<CefV8Value> val = *arguments.begin();
            std::wstring sFile = val->GetStringValue().ToWString();

            CImageFileFormatChecker oChecker;
            bool bIsImageFile = oChecker.isImageFile(sFile);
            retval = CefV8Value::CreateBool(bIsImageFile);

            return true;
        }
        else if (name == "GetDropFiles")
        {
            int nCount = (int)m_arDropFiles.size();
            retval = CefV8Value::CreateArray(nCount);
            int nCurrent = 0;
            for (std::vector<std::wstring>::iterator i = m_arDropFiles.begin(); i != m_arDropFiles.end(); i++)
                retval->SetValue(nCurrent++, CefV8Value::CreateString(*i));

            return true;
        }
        else if (name == "DropOfficeFiles")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("DropOfficeFiles");

            int nCurrent = 0;
            for (std::vector<std::wstring>::iterator i = m_arDropFiles.begin(); i != m_arDropFiles.end(); i++)
                 message->GetArgumentList()->SetString(nCurrent++, *i);

            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "SetAdvancedOptions")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_setadvancedoptions");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;

            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "ApplyAction")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_core_check_info");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue());

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "InitJSContext")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_init_js_context");
            browser->SendProcessMessage(PID_BROWSER, message);

            CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
            if (_frame)
            {
                _frame->ExecuteJavaScript("window.AscDesktopEditor.sendSystemMessage = function(arg) { window.AscDesktopEditor.isSendSystemMessage = true;window.AscDesktopEditor._sendSystemMessage(JSON.stringify(arg)); };", _frame->GetURL(), 0);
                _frame->ExecuteJavaScript("window.AscDesktopEditor.GetHash = function(arg, callback) { window.AscDesktopEditor.getHashCallback = callback; window.AscDesktopEditor._GetHash(arg); };", _frame->GetURL(), 0);
                _frame->ExecuteJavaScript("window.AscDesktopEditor.CallInAllWindows = function(arg) { window.AscDesktopEditor._CallInAllWindows(\"(\" + arg.toString() + \")();\"); };", _frame->GetURL(), 0);
                _frame->ExecuteJavaScript("window.AscDesktopEditor.OpenFileCrypt = function(name, url, callback) { window.AscDesktopEditor.openFileCryptCallback = callback; window.AscDesktopEditor._OpenFileCrypt(name, url); };", _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.SendBinary = function(name, value) { \n\
var xhr = new XMLHttpRequest();\n\
//xhr.setRequestHeader(\"Content-Type\", \"application/upload\");\n\
xhr.open(\"POST\", \"ascdesktop://binary/\" + window.AscDesktopEditor.GetEditorId() + \"/\" + name, false);\n\
xhr.send(value);\n\
};",
                _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.OpenFilenameDialog = function(filter, ismulti, callback) {\n\
window.on_native_open_filename_dialog = callback;\n\
window.AscDesktopEditor._OpenFilenameDialog(filter, ismulti);\n\
};", _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.DownloadFiles = function(filesSrc, filesDst, callback) {\n\
window.on_native_download_files = callback;\n\
window.AscDesktopEditor._DownloadFiles(filesSrc, filesDst);\n\
};", _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.SetCryptoMode = function(password, mode, callback) {\n\
window.on_set_crypto_mode = callback;\n\
window.AscDesktopEditor._SetCryptoMode(password, mode, callback ? true : false);\n\
                };", _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.GetAdvancedEncryptedData = function(password, callback) {\n\
window.on_get_advanced_encrypted_data = callback;\n\
window.AscDesktopEditor._GetAdvancedEncryptedData(password);\n\
};", _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window.AscDesktopEditor.SetAdvancedEncryptedData = function(password, data, callback) {\n\
window.on_set_advanced_encrypted_data = callback;\n\
window.AscDesktopEditor._SetAdvancedEncryptedData(password, data);\n\
};", _frame->GetURL(), 0);
            }

            return true;
        }
        else if (name == "NativeViewerOpen")
        {
            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();

            if (arguments.size() == 1) // password
            {
                m_oNativeViewer.SetPassword((*iter)->GetStringValue().ToWString());
                return true;
            }

            std::wstring sOpeningFilePath = (*iter)->GetStringValue().ToWString(); iter++;
            std::wstring sFontsDir = (*iter)->GetStringValue().ToWString(); iter++;
            std::wstring sFileDir = (*iter)->GetStringValue().ToWString(); ++iter;

            m_oNativeViewer.Init(sFileDir, sFontsDir, sOpeningFilePath, this);
            m_bIsNativeViewerMode = true;

            CefRefPtr<CefV8Value> _timerID;
            CefRefPtr<CefV8Exception> _exception;
            if (CefV8Context::GetCurrentContext()->Eval("(function(){ var intervalID = setInterval(function(){ if (!window.NativeFileOpen_error) { return; } window.AscDesktopEditor.NativeFunctionTimer(intervalID); }, 100); return intervalID; })();",
                                                        #ifndef CEF_2623
                                                                    "", 0,
                                                        #endif
                                                        _timerID, _exception))
            {
                m_nNativeOpenFileTimerID = _timerID->GetIntValue();
                //LOGGER_STRING2("timer created: " + std::to_string(m_nNativeOpenFileTimerID));
            }

            m_oNativeViewer.Start(0);

            return true;
        }
        else if (name == "NativeViewerClose")
        {
            m_oNativeViewer.CloseFile();
            return true;
        }
        else if (name == "NativeFunctionTimer")
        {
            int nIntervalID = arguments[0]->GetIntValue();
            //LOGGER_STRING2("NativeFunctionTimer called: " + std::to_string(nIntervalID));

            if (nIntervalID == m_nNativeOpenFileTimerID)
            {
                std::string sBase64File = m_oNativeViewer.GetBase64File();

                if (!sBase64File.empty())
                {
                    CefRefPtr<CefV8Value> _timerID;
                    CefRefPtr<CefV8Exception> _exception;

                    if (sBase64File == "error" || sBase64File == "password")
                    {
                        m_oNativeViewer.Stop();
                        m_oNativeViewer.ClearBase64();
                        CefV8Context::GetCurrentContext()->Eval("window.NativeFileOpen_error(\"" + sBase64File + "\");",
                                                                                        #ifndef CEF_2623
                                                                                                    "", 0,
                                                                                        #endif
                                                                                        _timerID, _exception);
                        return true;
                    }

                    std::string sCode = "clearTimeout(" + std::to_string(m_nNativeOpenFileTimerID) + ");";
                    if (CefV8Context::GetCurrentContext()->Eval(sCode,
                                                                #ifndef CEF_2623
                                                                            "", 0,
                                                                #endif
                                                                _timerID, _exception))
                    {
                        //LOGGER_STRING2("timer stoped: " + std::to_string(m_nNativeOpenFileTimerID));
                    }
                    m_nNativeOpenFileTimerID = -1;

                    CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("native_viewer_onopened");
                    message->GetArgumentList()->SetString(0, (sBase64File == "error") ? "" : sBase64File);
                    message->GetArgumentList()->SetString(1, m_oNativeViewer.GetPassword());
                    browser->SendProcessMessage(PID_BROWSER, message);
                }
            }

            return true;
        }
        else if (name == "NativeViewerGetPageUrl")
        {
            CNativeViewerPageInfo oInfo;
            oInfo.Page = arguments[0]->GetIntValue();
            oInfo.W = arguments[1]->GetIntValue();
            oInfo.H = arguments[2]->GetIntValue();

            int nPageStart = arguments[3]->GetIntValue();
            int nPageEnd = arguments[4]->GetIntValue();

            std::wstring sUrl = m_oNativeViewer.GetPathPageImage(oInfo);
            if (NSFile::CFileBinary::Exists(sUrl))
                retval = CefV8Value::CreateString(sUrl);
            else
            {
                m_oNativeViewer.AddTask(oInfo, nPageStart, nPageEnd);
                retval = CefV8Value::CreateString(L"");
            }

            return true;
        }
        else if (name == "NativeViewerGetCompleteTasks")
        {
            CTemporaryCS oCS(&m_oCompleteTasksCS);

            int nCount1 = (int)m_arCompleteTasks.size();
            int nCount2 = (int)m_arCompleteTextTasks.size();

            retval = CefV8Value::CreateArray(2 + nCount1 * 4 + nCount2 * 6);

            int nCurrent = 0;
            retval->SetValue(nCurrent++, CefV8Value::CreateInt(nCount1));
            retval->SetValue(nCurrent++, CefV8Value::CreateInt(nCount2));

            for (std::list<CSavedPageInfo>::iterator i = m_arCompleteTasks.begin(); i != m_arCompleteTasks.end(); i++)
            {
                retval->SetValue(nCurrent++, CefV8Value::CreateString(i->Url));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Page));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->W));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->H));
            }
            m_arCompleteTasks.clear();

            for (std::list<CSavedPageTextInfo>::iterator i = m_arCompleteTextTasks.begin(); i != m_arCompleteTextTasks.end(); i++)
            {
                retval->SetValue(nCurrent++, CefV8Value::CreateString(i->Info));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Page));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Paragraphs));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Words));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Spaces));
                retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Symbols));
            }
            m_arCompleteTextTasks.clear();

            return true;
        }
        else if (name == "GetInstallPlugins")
        {
            if (m_sSystemPlugins.empty())
            {
                CefRefPtr<CefV8Value> retval2;
                CefRefPtr<CefV8Exception> exception2;

                retval2 = NULL;
                exception2 = NULL;
                bool bIsSPData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_SP\"]();",
                                                                         #ifndef CEF_2623
                                                                                     "", 0,
                                                                         #endif
                                                                         retval2, exception2);
                if (bIsSPData)
                {
                    if (retval2->IsString())
                    {
                        std::string sDataA = retval2->GetStringValue().ToString();
                        m_sSystemPlugins = UTF8_TO_U(sDataA);
                    }
                }

                retval2 = NULL;
                exception2 = NULL;
                bool bIsUPData = CefV8Context::GetCurrentContext()->Eval("window[\"AscDesktopEditor_UP\"]();",
                                                                         #ifndef CEF_2623
                                                                                     "", 0,
                                                                         #endif
                                                                         retval2, exception2);
                if (bIsUPData)
                {
                    if (retval2->IsString())
                    {
                        std::string sDataA = retval2->GetStringValue().ToString();
                        m_sUserPlugins = UTF8_TO_U(sDataA);
                    }
                }
            }

            CPluginsManager oPlugins;
            oPlugins.m_strDirectory = m_sSystemPlugins;
            oPlugins.m_strUserDirectory = m_sUserPlugins;
            oPlugins.m_nCryptoMode = m_nCryptoMode;
            std::string sData = oPlugins.GetPluginsJson(true);
            retval = CefV8Value::CreateString(sData);
            return true;
        }
        else if (name == "PluginInstall")
        {
            CPluginsManager oPlugins;
            oPlugins.m_strDirectory = m_sSystemPlugins;
            oPlugins.m_strUserDirectory = m_sUserPlugins;

            std::wstring sFile = ((*arguments.begin())->GetStringValue()).ToWString();
            oPlugins.AddPlugin(sFile);

            // send to editor
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
            if (!_frame)
                _frame = browser->GetMainFrame();

            std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            return true;
        }
        else if (name == "PluginUninstall")
        {
            CPluginsManager oPlugins;
            oPlugins.m_strDirectory = m_sSystemPlugins;
            oPlugins.m_strUserDirectory = m_sUserPlugins;

            std::wstring sGuid = ((*arguments.begin())->GetStringValue()).ToWString();
            oPlugins.RemovePlugin(sGuid);

            // send to editor
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
            if (!_frame)
                _frame = browser->GetMainFrame();

            std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            return true;
        }
        else if (name == "IsLocalFile")
        {
            bool isUrlCheck = false;
            if (arguments.size() > 0 && m_sLocalFileFolderWithoutFile.empty())
            {
                isUrlCheck = (*arguments.begin())->GetBoolValue();
                if (isUrlCheck)
                {
                    std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
                    if (0 == sUrl.find("file:/"))
                    {
                        retval = CefV8Value::CreateBool(true);
                        return true;
                    }
                }
            }

            if (m_sLocalFileFolderWithoutFile.empty())
                retval = CefV8Value::CreateBool(false);
            else
                retval = CefV8Value::CreateBool(true);

            return true;
        }
        else if (name == "IsFilePrinting")
        {
            retval = CefV8Value::CreateBool(m_bIsPrinting);
            return true;
        }
        else if (name == "Sign")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_sign");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;

            if (iter != arguments.end())
            {
                message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
            }
            else
                message->GetArgumentList()->SetString(1, "");

            if (iter != arguments.end())
            {
                message->GetArgumentList()->SetString(2, (*iter)->GetStringValue()); ++iter;
            }
            else
                message->GetArgumentList()->SetString(2, "");

            if (iter != arguments.end())
            {
                message->GetArgumentList()->SetString(3, (*iter)->GetStringValue()); ++iter;
            }
            else
                message->GetArgumentList()->SetString(3, "");

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "RemoveSignature")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_remove");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "RemoveAllSignatures")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_remove");

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "ViewCertificate")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_viewcertificate");

            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetInt(0, (*iter)->GetIntValue());

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "SelectCertificate")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_selectsertificate");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "GetDefaultCertificate")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_defaultcertificate");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "_OpenFilenameDialog")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_open_filename_dialog");
            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue());            
            int64 id = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
            message->GetArgumentList()->SetString(1, std::to_string(id));
            if (arguments.size() > 1)
            {
                iter++;
                message->GetArgumentList()->SetBool(2, (*iter)->GetBoolValue());
            }
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "SaveQuestion")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_file_save_question");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "startReporter")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_start_reporter");
            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue());
            message->GetArgumentList()->SetString(1, m_sLocalFileFolder);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "endReporter")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_end_reporter");
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "sendToReporter")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_to_reporter");
            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "sendFromReporter")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_from_reporter");
            std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();
            message->GetArgumentList()->SetString(0, (*iter)->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "IsSignaturesSupport")
        {
            retval = CefV8Value::CreateBool(m_bIsSupportSigs);
            return true;
        }
        else if (name == "IsProtectionSupport")
        {
            bool bIsProtect = m_bIsSupportProtect;
            if (bIsProtect)
            {
                if (m_bIsSupportOnlyPass && (m_nCryptoMode > 0))
                    bIsProtect = false;
            }
            retval = CefV8Value::CreateBool(bIsProtect);
            return true;
        }
        else if (name == "SetSupportSign")
        {
            m_bIsSupportSigs = arguments[0]->GetBoolValue();
            return true;
        }
        else if (name == "SetInitFlags")
        {
            int nFlags = arguments[0]->GetIntValue();
            m_bIsSupportOnlyPass = ((nFlags & 0x01) == 0x01) ? true : false;
            m_bIsSupportProtect = ((nFlags & 0x02) == 0x02) ? true : false;

            if (1 < arguments.size())
                m_nCryptoMode = arguments[1]->GetIntValue();

            if (2 < arguments.size())
            {
                CefRefPtr<CefV8Value> _array = arguments[2];
                int nLen = _array->GetArrayLength();

                if (nLen > 0)
                    m_sSystemPlugins = _array->GetValue(0)->GetStringValue().ToWString();

                if (nLen > 1)
                    m_sUserPlugins = _array->GetValue(1)->GetStringValue().ToWString();

                if (nLen > 2)
                    m_sCookiesPath = _array->GetValue(2)->GetStringValue().ToWString();
            }

            return true;
        }
        else if (name == "isBlockchainSupport")
        {
            retval = CefV8Value::CreateBool(m_bIsSupportOnlyPass && (m_nCryptoMode > 0));
            return true;
        }
        else if (name == "OpenAsLocal")
        {
            CefRefPtr<CefV8Value> obj = arguments[0];
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("open_as_local");
            message->GetArgumentList()->SetString(0, obj->GetValue("downloadLink")->GetStringValue());
            message->GetArgumentList()->SetString(1, obj->GetValue("saveLink")->GetStringValue());
            message->GetArgumentList()->SetString(2, obj->GetValue("title")->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "SaveAsCloud")
        {
            // TODO:
            return true;
        }
        else if (name == "_sendSystemMessage")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_system_message");

            int64 frameID = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();

            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetString(1, std::to_string(frameID));

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "_GetHash")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("file_get_hash");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            if (arguments.size() > 1)
                message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
            else
                message->GetArgumentList()->SetString(1, "sha-256");

            int64 frameID = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
            message->GetArgumentList()->SetString(2, std::to_string(frameID));

            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "_CallInAllWindows")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("call_in_all_windows");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "_OpenFileCrypt")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("open_file_crypt");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "buildCryptedStart")
        {
            std::string sContent = arguments[0]->GetStringValue().ToString();
            BYTE* pDataDst = NULL;
            int nLenDst = 0;

            NSFile::CBase64Converter::Decode(sContent.c_str(), sContent.length(), pDataDst, nLenDst);

            NSFile::CFileBinary oFileWithChanges;
            oFileWithChanges.CreateFileW(m_sCryptDocumentFolder + L"/EditorWithChanges.bin");
            oFileWithChanges.WriteFile(pDataDst, nLenDst);
            oFileWithChanges.CloseFile();

            RELEASEARRAYOBJECTS(pDataDst);

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted");
            message->GetArgumentList()->SetString(0, arguments[2]->GetStringValue());
            message->GetArgumentList()->SetString(1, arguments[3]->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "buildCryptedEnd")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted_end");
            bool bIsClosed = arguments[0]->GetBoolValue();
            message->GetArgumentList()->SetBool(0, bIsClosed);
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "SetCryptDocumentFolder")
        {
            m_sCryptDocumentFolder = arguments[0]->GetStringValue().ToWString();
            return true;
        }
        else if (name == "PreloadCryptoImage")
        {
            std::wstring sUrl1 = arguments[0]->GetStringValue().ToWString();
            std::wstring sUrl2 = arguments[1]->GetStringValue().ToWString();

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("preload_crypto_image");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "ResaveFile")
        {
            std::wstring sFile = /*m_sCryptDocumentFolder + */arguments[0]->GetStringValue().ToWString();
            std::string sData = arguments[1]->GetStringValue().ToString();

            std::wstring sFileHeader = L"ascdesktop://fonts/";
            if (0 == sFile.find(sFileHeader))
            {
                sFile = sFile.substr(sFileHeader.length());

#ifdef WIN32
                NSCommon::string_replace(sFile, L"/", L"\\");
#endif
            }

            BYTE* pDataDst = NULL;
            int nSizeDst = 0;
            NSFile::CBase64Converter::Decode(sData.c_str(), (int)sData.length(), pDataDst, nSizeDst);

            if (NSFile::CFileBinary::Exists(sFile))
                NSFile::CFileBinary::Remove(sFile);

            NSFile::CFileBinary oFile;
            oFile.CreateFileW(sFile);
            oFile.WriteFile(pDataDst, (DWORD)nSizeDst);
            oFile.CloseFile();

            RELEASEARRAYOBJECTS(pDataDst);

            return true;
        }
        else if (name == "_DownloadFiles")
        {
            std::wstring sDirTmp = m_sCryptDocumentFolder;
            if (sDirTmp.empty())
                sDirTmp = m_sLocalFileFolderWithoutFile;
            if (sDirTmp.empty())
                sDirTmp = NSFile::CFileBinary::GetTempPath();

            CefRefPtr<CefV8Value> val = arguments[0];
            int nCount = val->GetArrayLength();

            int nCount2 = 0;
            CefRefPtr<CefV8Value> val2 = NULL;
            if (arguments.size() > 1)
            {
                val2 = arguments[1];
                nCount2 = val2->GetArrayLength();
            }

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("download_files");

            int nIndex = 0;
            for (int i = 0; i < nCount; ++i)
            {
                message->GetArgumentList()->SetString(nIndex++, val->GetValue(i)->GetStringValue());

                if (i < nCount2)
                {
                    message->GetArgumentList()->SetString(nIndex++, val2->GetValue(i)->GetStringValue());
                }
                else
                {
                    std::wstring sFileTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(sDirTmp, L"IMG");
                    if (NSFile::CFileBinary::Exists(sFileTmp))
                        NSFile::CFileBinary::Remove(sFileTmp);
                    message->GetArgumentList()->SetString(nIndex++, sFileTmp);
                }
            }

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }
        else if (name == "RemoveFile")
        {
            std::wstring sFile = arguments[0]->GetStringValue();
            if (NSFile::CFileBinary::Exists(sFile))
                NSFile::CFileBinary::Remove(sFile);
            return true;
        }
        else if (name == "_SetCryptoMode")
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_crypto_mode");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetInt(1, arguments[1]->GetIntValue());

            if (arguments.size() > 2)
                message->GetArgumentList()->SetBool(2, arguments[2]->GetBoolValue());
            else
                message->GetArgumentList()->SetBool(2, false);

            int64 frameId = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
            message->GetArgumentList()->SetInt(3, (int)frameId);

            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "GetImageFormat")
        {
            std::wstring sFile = arguments[0]->GetStringValue();
            std::string sExt = "jpg";

            CImageFileFormatChecker _checker;
            if (_checker.isPngFile(sFile))
                sExt = "png";

            retval = CefV8Value::CreateString(sExt);
            return true;
        }
        else if (name == "GetEncryptedHeader")
        {
            retval = CefV8Value::CreateString("ENCRYPTED;");
            return true;
        }
        else if (name == "GetCryptoMode")
        {
            retval = CefV8Value::CreateInt(m_nCryptoMode);
            return true;
        }
        else if (name == "GetSupportCryptoModes")
        {
            NSAscCrypto::CCryptoMode oCryptoMode;
            NSAscCrypto::CCryptoKey keyEnc, keyDec;
            oCryptoMode.Load(keyEnc, keyDec, m_sCookiesPath + L"/user.data");

            CPluginsManager oPlugins;
            oPlugins.m_strDirectory = m_sSystemPlugins;
            oPlugins.m_strUserDirectory = m_sUserPlugins;
            oPlugins.GetInstalledPlugins();

            int nCount = (int)oPlugins.m_arCryptoModes.size();
            retval = CefV8Value::CreateArray(nCount);
            int nCurIndex = 0;
            for (std::map<int, std::string>::iterator iter = oPlugins.m_arCryptoModes.begin(); iter != oPlugins.m_arCryptoModes.end(); iter++)
            {
                int nMode = iter->first;
#ifdef CEF_2623
                CefRefPtr<CefV8Value> val = CefV8Value::CreateObject(NULL);
#else
                CefRefPtr<CefV8Value> val = CefV8Value::CreateObject(NULL, NULL);
#endif
                val->SetValue("type", CefV8Value::CreateInt(nMode), V8_PROPERTY_ATTRIBUTE_NONE);
                val->SetValue("info_presented", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);

                retval->SetValue(nCurIndex++, val);
            }
            return true;
        }
        else if (name == "_GetAdvancedEncryptedData")
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("get_advanced_encrypted_data");
            message->GetArgumentList()->SetInt(0, (int)CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
            message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "_SetAdvancedEncryptedData")
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_advanced_encrypted_data");
            message->GetArgumentList()->SetInt(0, (int)CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
            message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetString(2, arguments[1]->GetStringValue());
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            browser->SendProcessMessage(PID_BROWSER, message);
            return true;
        }
        else if (name == "GetExternalClouds")
        {
            std::wstring sSP = m_sSystemPlugins;
            if (sSP.empty())
            {
                sSP = NSSystem::GetEnvValue("asc-systems-plugins");
            }

            std::wstring sFile = NSCommon::GetDirectoryName(sSP) + L"/externalcloud.json";
            std::string sContent = "";

            if (NSFile::CFileBinary::ReadAllTextUtf8A(sFile, sContent))
            {
                NSCommon::string_replaceA(sContent, "\r", "");
                NSCommon::string_replaceA(sContent, "\n", "");
                NSCommon::string_replaceA(sContent, "\\", "\\\\");
                NSCommon::string_replaceA(sContent, "\"", "\\\"");
                CefRefPtr<CefV8Exception> exception;
                CefV8Context::GetCurrentContext()->Eval("(function(){ return JSON.parse(\"" + sContent + "\"); })();",
                                                                          #ifndef CEF_2623
                                                                                      "", 0,
                                                                          #endif
                                                                          retval, exception);
            }
            else
            {
                retval = CefV8Value::CreateNull();
            }
            return true;
        }
        else if (name == "IsNativeViewer")
        {
            retval = CefV8Value::CreateBool(m_bIsNativeViewerMode);
            return true;
        }
        else if (name == "CryptoDownloadAs")
        {
            std::string fileData = arguments[0]->GetStringValue().ToString();

            NSFile::CFileBinary oFileWithChanges;
            oFileWithChanges.CreateFileW(m_sCryptDocumentFolder + L"/EditorWithChanges.bin");
            oFileWithChanges.WriteFile((BYTE*)fileData.c_str(), (DWORD)fileData.length());
            oFileWithChanges.CloseFile();

            int nFormat = arguments[1]->GetIntValue();
            std::string sParams = arguments[2]->GetStringValue();

            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("crypto_download_as");
            message->GetArgumentList()->SetInt(0, nFormat);
            message->GetArgumentList()->SetString(1, sParams);
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
            browser->SendProcessMessage(PID_BROWSER, message);

            return true;
        }

        // Function does not exist.
        return false;
    }

    void RemoveChanges(const int& nDeleteIndex)
    {
        int nNaturalIndex = m_nOpenChangesIndex + nDeleteIndex;

        // на каждое изменение две кавычки)
        nNaturalIndex <<= 1;

        // not cool realize
        BYTE* pData = NULL;
        DWORD dwSize = 0;
        bool bIsOk = NSFile::CFileBinary::ReadAllBytes(m_sLocalFileChanges, &pData, dwSize);
        int nCounter = 0;

        int nSize = (int)dwSize;
        int nIndex = -1;
        for (int i = 0; i < nSize; i++)
        {
            if ('\"' == pData[i])
            {
                if (nCounter == nNaturalIndex)
                {
                    nIndex = i;
                    break;
                }
                ++nCounter;
            }
        }
        RELEASEARRAYOBJECTS(pData);

        if (-1 != nIndex)
        {
            NSFile::CFileBinary::Truncate(m_sLocalFileChanges, nIndex);
        }
    }

    bool IsNeedDownload(const std::wstring& FilePath)
    {
        int n1 = FilePath.find(L"www.");
        int n2 = FilePath.find(L"http://");
        int n3 = FilePath.find(L"ftp://");
        int n4 = FilePath.find(L"https://");

        if (n1 != std::wstring::npos && n1 < 10)
            return true;
        if (n2 != std::wstring::npos && n2 < 10)
            return true;
        if (n3 != std::wstring::npos && n3 < 10)
            return true;
        if (n4 != std::wstring::npos && n4 < 10)
            return true;

        return false;
    }

    std::wstring GetLocalImageUrl(const std::wstring& sUrl)
    {
        std::map<std::wstring, std::wstring>::iterator _find = m_mapLocalAddImages.find(sUrl);
        if (_find != m_mapLocalAddImages.end())
            return _find->second;

        std::wstring sUrlFile = sUrl;
        if (sUrlFile.find(L"file://") == 0)
        {
            sUrlFile = sUrlFile.substr(7);
            
            // MS Word copy image with url "file://localhost/..." on mac
            if (sUrlFile.find(L"localhost") == 0)
                sUrlFile = sUrlFile.substr(9);
            
            NSCommon::string_replace(sUrlFile, L"%20", L" ");
            
            if (!NSFile::CFileBinary::Exists(sUrlFile))
                sUrlFile = sUrlFile.substr(1);
        }

        if (NSFile::CFileBinary::Exists(sUrlFile))
        {
            return GetLocalImageUrlLocal(sUrlFile, sUrl);
        }
        if (IsNeedDownload(sUrl))
        {
            std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
            if (NSFile::CFileBinary::Exists(sTmpFile))
                NSFile::CFileBinary::Remove(sTmpFile);

            CFileDownloader oDownloader(sUrl, false);
            oDownloader.SetFilePath(sTmpFile);
            oDownloader.Start( 0 );
            while ( oDownloader.IsRunned() )
            {
                NSThreads::Sleep( 10 );
            }

            std::wstring sRet = GetLocalImageUrlLocal(sTmpFile, sUrl);

            if (NSFile::CFileBinary::Exists(sTmpFile))
                NSFile::CFileBinary::Remove(sTmpFile);

            return sRet;
        }
        if (0 == sUrl.find(L"data:"))
        {
            std::wstring::size_type nBase64Start = sUrl.find(L"base64,");
            if (nBase64Start != std::wstring::npos)
            {
                int nStartIndex = nBase64Start + 7;
                int nCount = sUrl.length() - nStartIndex;
                char* pData = new char[nCount];
                const wchar_t* pDataSrc = sUrl.c_str();
                for (int i = 0; i < nCount; ++i)
                    pData[i] = pDataSrc[i + nStartIndex];

                BYTE* pDataDecode = NULL;
                int nLenDecode = 0;
                NSFile::CBase64Converter::Decode(pData, nCount, pDataDecode, nLenDecode);

                RELEASEARRAYOBJECTS(pData);

                std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
                if (NSFile::CFileBinary::Exists(sTmpFile))
                    NSFile::CFileBinary::Remove(sTmpFile);

                NSFile::CFileBinary oFile;
                if (oFile.CreateFileW(sTmpFile))
                {
                    oFile.WriteFile(pDataDecode, (DWORD)nLenDecode);
                    oFile.CloseFile();
                }

                RELEASEARRAYOBJECTS(pDataDecode);

                std::wstring sRet = GetLocalImageUrlLocal(sTmpFile, sUrl);

                if (NSFile::CFileBinary::Exists(sTmpFile))
                    NSFile::CFileBinary::Remove(sTmpFile);

                return sRet;
            }
        }
        if (sUrl.find(L"image") == 0)
            return sUrl;
        return L"error";
    }
    std::wstring GetLocalImageUrlLocal(const std::wstring& sUrl, const std::wstring& sUrlMap)
    {
        std::wstring sUrlTmp = sUrl;
        CImageFileFormatChecker oChecker;
        if (!oChecker.isImageFile(sUrlTmp))
            return L"error";

        if (oChecker.eFileType == _CXIMAGE_FORMAT_PNG)
        {
            std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".png";
            NSFile::CFileBinary::Copy(sUrl, m_sLocalFileFolderWithoutFile + L"/media/" + sRet);
            m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
            return sRet;
        }
        if (oChecker.eFileType == _CXIMAGE_FORMAT_JPG)
        {
            std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".jpg";
            NSFile::CFileBinary::Copy(sUrl, m_sLocalFileFolderWithoutFile + L"/media/" + sRet);
            m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
            return sRet;
        }

        CBgraFrame oFrame;
        if (oFrame.OpenFile(sUrl))
        {
            std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".png";
            oFrame.SaveFile(m_sLocalFileFolderWithoutFile + L"/media/" + sRet, _CXIMAGE_FORMAT_PNG);
            m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
            return sRet;
        }

        if (NULL == m_pLocalApplicationFonts)
        {
            m_pLocalApplicationFonts = NSFonts::NSApplication::Create();
            m_pLocalApplicationFonts->InitializeFromFolder(m_sFontsData);
        }

        MetaFile::IMetaFile* pMetafile = MetaFile::Create(m_pLocalApplicationFonts);
        pMetafile->LoadFromFile(sUrl.c_str());

        if (pMetafile->GetType() == MetaFile::c_lMetaEmf || pMetafile->GetType() == MetaFile::c_lMetaWmf)
        {
            std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex) + L".svg";
            std::wstring sRet1 = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + ((pMetafile->GetType() == MetaFile::c_lMetaEmf) ? L".emf" : L".wmf");

            double x = 0, y = 0, w = 0, h = 0;
            pMetafile->GetBounds(&x, &y, &w, &h);

            double _max = (w >= h) ? w : h;
            double dKoef = 100000.0 / _max;

            int WW = (int)(dKoef * w + 0.5);
            int HH = (int)(dKoef * h + 0.5);

            NSHtmlRenderer::CASCSVGWriter oWriterSVG(false);
            oWriterSVG.SetFontManager(m_pLocalApplicationFonts->GenerateFontManager());
            oWriterSVG.put_Width(WW);
            oWriterSVG.put_Height(HH);
            pMetafile->DrawOnRenderer(&oWriterSVG, 0, 0, WW, HH);

            oWriterSVG.SaveFile(m_sLocalFileFolderWithoutFile + L"/media/" + sRet);

            m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
            return sRet;
        }
        if (pMetafile->GetType() == MetaFile::c_lMetaSvg || pMetafile->GetType() == MetaFile::c_lMetaSvm)
        {
            std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".png";

            double x = 0, y = 0, w = 0, h = 0;
            pMetafile->GetBounds(&x, &y, &w, &h);

            double _max = (w >= h) ? w : h;
            double dKoef = 1000.0 / _max;

            int WW = (int)(dKoef * w + 0.5);
            int HH = (int)(dKoef * h + 0.5);

            std::wstring sSaveRet = m_sLocalFileFolderWithoutFile + L"/media/" + sRet;
            pMetafile->ConvertToRaster(sSaveRet.c_str(), _CXIMAGE_FORMAT_PNG, WW, HH);

            m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
            return sRet;
        }

        RELEASEINTERFACE(pMetafile);

        return L"error";
    }

    std::wstring GetFullUrl(const std::wstring& sUrl, const std::wstring& sBaseUrl)
    {
        std::wstring sUrlSrc = L"";
        if (IsNeedDownload(sUrl))
        {
            sUrlSrc = sUrl;
        }
        else
        {
            if (0 == sUrl.find(wchar_t('/')))
            {
                // нужно брать корень сайта
                int nPos = sBaseUrl.find(L"//");
                if (nPos != std::wstring::npos)
                {
                    nPos = sBaseUrl.find(wchar_t('/'), nPos + 3);
                    if (nPos != std::wstring::npos)
                    {
                        sUrlSrc = sBaseUrl.substr(0, nPos);
                        sUrlSrc += sUrl;
                    }
                }
                if (sUrlSrc.empty())
                {
                    sUrlSrc = sBaseUrl;
                    sUrlSrc += (L"/" + sUrl);
                }
            }
            else
            {
                // брать место урла
                int nPos = sBaseUrl.find_last_of(wchar_t('/'));
                if (std::wstring::npos != nPos)
                {
                    sUrlSrc = sBaseUrl.substr(0, nPos);
                }
                else
                {
                    sUrlSrc = sBaseUrl;
                }
                sUrlSrc += (L"/" + sUrl);
            }
        }
        NSCommon::url_correct(sUrlSrc);
        return sUrlSrc;
    }

    std::wstring GetFullUrl2(const std::wstring& sUrl, const std::wstring& sBaseUrlSrc)
    {
        std::wstring sBaseUrl = sBaseUrlSrc;
        std::wstring::size_type sPosQuest = sBaseUrl.find(L"/index.html?");
        if (sPosQuest != std::wstring::npos)
        {
            sBaseUrl = sBaseUrl.substr(0, sPosQuest + 10);
        }

        return GetFullUrl(sUrl, sBaseUrl);
    }

    bool IsChartEditor()
    {
        if (!CefV8Context::GetCurrentContext())
            return false;

        CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
        if (!_frame)
            return false;

        CefRefPtr<CefFrame> _frameParent = _frame->GetParent();
        if (!_frameParent)
            return false;

        std::string sName = _frameParent->GetName().ToString();
        if (sName != "frameEditor")
            return false;

        return true;
    }

    // Provide the reference counting implementation for this class.
    IMPLEMENT_REFCOUNTING(CAscEditorNativeV8Handler);
};

class ClientRenderDelegate : public client::ClientAppRenderer::Delegate {
 public:
  ClientRenderDelegate()
    : last_node_is_editable_(false)
  {
    m_pAdditional = Create_ApplicationRendererAdditional();
    sync_command_check = false;
    m_bIsNativeViewer = false;
  }

  virtual void OnWebKitInitialized(CefRefPtr<client::ClientAppRenderer> app) OVERRIDE {
    // Create the renderer-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  virtual void OnContextCreated(CefRefPtr<client::ClientAppRenderer> app,
                                CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE {
    message_router_->OnContextCreated(browser,  frame, context);

    // add AscEditorNative
    CefRefPtr<CefV8Value> object = context->GetGlobal();

#ifdef CEF_2623
    CefRefPtr<CefV8Value> objNative = CefV8Value::CreateObject(NULL);
#else
    CefRefPtr<CefV8Value> objNative = CefV8Value::CreateObject(NULL, NULL);
#endif
    CAscEditorNativeV8Handler* pNativeHandlerWrapper = new CAscEditorNativeV8Handler();
    pNativeHandlerWrapper->sync_command_check = &sync_command_check;

    CefRefPtr<CefV8Handler> _nativeHandler = pNativeHandlerWrapper;

    CefRefPtr<CefV8Value> _nativeFunctionCopy = CefV8Value::CreateFunction("Copy", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunctionPaste = CefV8Value::CreateFunction("Paste", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunctionCut = CefV8Value::CreateFunction("Cut", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunctionLoadJS = CefV8Value::CreateFunction("LoadJS", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunctionEditorType = CefV8Value::CreateFunction("SetEditorType", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction11 = CefV8Value::CreateFunction("LoadFontBase64", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction22 = CefV8Value::CreateFunction("getFontsSprite", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction33 = CefV8Value::CreateFunction("SetEditorId", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction34 = CefV8Value::CreateFunction("GetEditorId", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction44 = CefV8Value::CreateFunction("SpellCheck", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction55 = CefV8Value::CreateFunction("CreateEditorApi", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction66 = CefV8Value::CreateFunction("ConsoleLog", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction77 = CefV8Value::CreateFunction("GetFrameContent", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction88 = CefV8Value::CreateFunction("SetFrameContent", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction111 = CefV8Value::CreateFunction("setCookie", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction112 = CefV8Value::CreateFunction("setAuth", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction113 = CefV8Value::CreateFunction("getCookiePresent", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction114 = CefV8Value::CreateFunction("getAuth", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction115 = CefV8Value::CreateFunction("Logout", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction222 = CefV8Value::CreateFunction("onDocumentModifiedChanged", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction301 = CefV8Value::CreateFunction("OpenBinaryFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction302 = CefV8Value::CreateFunction("CloseBinaryFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction303 = CefV8Value::CreateFunction("GetBinaryFileData", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction304 = CefV8Value::CreateFunction("GetImageBase64", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction401 = CefV8Value::CreateFunction("SetDocumentName", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction501 = CefV8Value::CreateFunction("OnSave", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction601 = CefV8Value::CreateFunction("js_message", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction602 = CefV8Value::CreateFunction("CheckNeedWheel", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction603 = CefV8Value::CreateFunction("SetFullscreen", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction701 = CefV8Value::CreateFunction("Print_Start", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction702 = CefV8Value::CreateFunction("Print_Page", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction703 = CefV8Value::CreateFunction("Print_End", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction704 = CefV8Value::CreateFunction("Print", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction705 = CefV8Value::CreateFunction("IsSupportNativePrint", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction801 = CefV8Value::CreateFunction("Set_App_Data", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction901 = CefV8Value::CreateFunction("LocalStartOpen", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction902 = CefV8Value::CreateFunction("LocalFileOpen", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction903 = CefV8Value::CreateFunction("LocalFileRecoverFolder", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction904 = CefV8Value::CreateFunction("LocalFileRecents", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction905 = CefV8Value::CreateFunction("LocalFileOpenRecent", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction906 = CefV8Value::CreateFunction("LocalFileRemoveRecent", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction907 = CefV8Value::CreateFunction("LocalFileRecovers", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction908 = CefV8Value::CreateFunction("LocalFileOpenRecover", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction909 = CefV8Value::CreateFunction("LocalFileRemoveRecover", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction910 = CefV8Value::CreateFunction("LocalFileSaveChanges", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction911 = CefV8Value::CreateFunction("LocalFileCreate", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction912 = CefV8Value::CreateFunction("LocalFileGetOpenChangesCount", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction913 = CefV8Value::CreateFunction("LocalFileSetOpenChangesCount", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction914 = CefV8Value::CreateFunction("LocalFileGetCurrentChangesIndex", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction915 = CefV8Value::CreateFunction("LocalFileSave", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction916 = CefV8Value::CreateFunction("LocalFileGetSourcePath", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction917 = CefV8Value::CreateFunction("LocalFileSetSourcePath", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction918 = CefV8Value::CreateFunction("LocalFileGetSaved", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction919 = CefV8Value::CreateFunction("LocalFileSetSaved", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction920 = CefV8Value::CreateFunction("LocalFileGetImageUrl", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction921 = CefV8Value::CreateFunction("LocalFileGetImageUrlFromOpenFileDialog", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction922 = CefV8Value::CreateFunction("AscBrowserScrollStyle", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction923 = CefV8Value::CreateFunction("checkAuth", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction924 = CefV8Value::CreateFunction("execCommand", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction925 = CefV8Value::CreateFunction("SetDropFiles", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction926 = CefV8Value::CreateFunction("IsImageFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction927 = CefV8Value::CreateFunction("GetDropFiles", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction928 = CefV8Value::CreateFunction("DropOfficeFiles", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction929 = CefV8Value::CreateFunction("SetAdvancedOptions", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction930 = CefV8Value::CreateFunction("LocalFileRemoveAllRecents", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction931 = CefV8Value::CreateFunction("LocalFileRemoveAllRecovers", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction932 = CefV8Value::CreateFunction("CheckUserId", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction933 = CefV8Value::CreateFunction("ApplyAction", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction934 = CefV8Value::CreateFunction("InitJSContext", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction935 = CefV8Value::CreateFunction("NativeViewerOpen", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction936 = CefV8Value::CreateFunction("NativeViewerClose", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction937 = CefV8Value::CreateFunction("NativeFunctionTimer", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction938 = CefV8Value::CreateFunction("NativeViewerGetPageUrl", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction939 = CefV8Value::CreateFunction("NativeViewerGetCompleteTasks", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction940 = CefV8Value::CreateFunction("GetInstallPlugins", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction951 = CefV8Value::CreateFunction("IsLocalFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction952 = CefV8Value::CreateFunction("IsFilePrinting", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction953 = CefV8Value::CreateFunction("Sign", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction954 = CefV8Value::CreateFunction("ViewCertificate", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction955 = CefV8Value::CreateFunction("SelectCertificate", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction956 = CefV8Value::CreateFunction("GetDefaultCertificate", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction957 = CefV8Value::CreateFunction("_OpenFilenameDialog", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction958 = CefV8Value::CreateFunction("SaveQuestion", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction960 = CefV8Value::CreateFunction("startReporter", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction961 = CefV8Value::CreateFunction("endReporter", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction962 = CefV8Value::CreateFunction("sendToReporter", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction963 = CefV8Value::CreateFunction("sendFromReporter", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction964 = CefV8Value::CreateFunction("IsSignaturesSupport", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction965 = CefV8Value::CreateFunction("RemoveSignature", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction966 = CefV8Value::CreateFunction("RemoveAllSignatures", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction967 = CefV8Value::CreateFunction("SetSupportSign", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction968 = CefV8Value::CreateFunction("PluginInstall", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction969 = CefV8Value::CreateFunction("PluginUninstall", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction970 = CefV8Value::CreateFunction("SetInitFlags", _nativeHandler);

    CefRefPtr<CefV8Value> _nativeFunction971 = CefV8Value::CreateFunction("isBlockchainSupport", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction972 = CefV8Value::CreateFunction("OpenAsLocal", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction973 = CefV8Value::CreateFunction("SaveAsCloud", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction974 = CefV8Value::CreateFunction("_sendSystemMessage", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction975 = CefV8Value::CreateFunction("_GetHash", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction976 = CefV8Value::CreateFunction("_CallInAllWindows", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction977 = CefV8Value::CreateFunction("_OpenFileCrypt", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction978 = CefV8Value::CreateFunction("buildCryptedStart", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction979 = CefV8Value::CreateFunction("buildCryptedEnd", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction980 = CefV8Value::CreateFunction("SetCryptDocumentFolder", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction981 = CefV8Value::CreateFunction("PreloadCryptoImage", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction982 = CefV8Value::CreateFunction("ResaveFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction983 = CefV8Value::CreateFunction("_DownloadFiles", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction984 = CefV8Value::CreateFunction("RemoveFile", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction985 = CefV8Value::CreateFunction("GetImageFormat", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction986 = CefV8Value::CreateFunction("_SetCryptoMode", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction987 = CefV8Value::CreateFunction("GetEncryptedHeader", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction988 = CefV8Value::CreateFunction("GetCryptoMode", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction989 = CefV8Value::CreateFunction("GetSupportCryptoModes", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction990 = CefV8Value::CreateFunction("IsProtectionSupport", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction991 = CefV8Value::CreateFunction("_GetAdvancedEncryptedData", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction992 = CefV8Value::CreateFunction("_SetAdvancedEncryptedData", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction993 = CefV8Value::CreateFunction("GetExternalClouds", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction994 = CefV8Value::CreateFunction("IsNativeViewer", _nativeHandler);
    CefRefPtr<CefV8Value> _nativeFunction995 = CefV8Value::CreateFunction("CryptoDownloadAs", _nativeHandler);


    objNative->SetValue("Copy", _nativeFunctionCopy, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Paste", _nativeFunctionPaste, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Cut", _nativeFunctionCut, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LoadJS", _nativeFunctionLoadJS, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SetEditorType", _nativeFunctionEditorType, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LoadFontBase64", _nativeFunction11, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("getFontsSprite", _nativeFunction22, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SetEditorId", _nativeFunction33, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetEditorId", _nativeFunction34, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SpellCheck", _nativeFunction44, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("CreateEditorApi", _nativeFunction55, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("ConsoleLog", _nativeFunction66, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetFrameContent", _nativeFunction77, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SetFrameContent", _nativeFunction88, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("setCookie", _nativeFunction111, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("setAuth", _nativeFunction112, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("getCookiePresent", _nativeFunction113, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("getAuth", _nativeFunction114, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Logout", _nativeFunction115, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("onDocumentModifiedChanged", _nativeFunction222, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("OpenBinaryFile", _nativeFunction301, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("CloseBinaryFile", _nativeFunction302, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetBinaryFileData", _nativeFunction303, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetImageBase64", _nativeFunction304, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SetDocumentName", _nativeFunction401, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("OnSave", _nativeFunction501, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("js_message", _nativeFunction601, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("CheckNeedWheel", _nativeFunction602, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SetFullscreen", _nativeFunction603, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("Print_Start", _nativeFunction701, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Print_Page", _nativeFunction702, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Print_End", _nativeFunction703, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("Print", _nativeFunction704, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsSupportNativePrint", _nativeFunction705, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("Set_App_Data", _nativeFunction801, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalStartOpen", _nativeFunction901, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileOpen", _nativeFunction902, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileRecoverFolder", _nativeFunction903, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileRecents", _nativeFunction904, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileOpenRecent", _nativeFunction905, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileRemoveRecent", _nativeFunction906, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileRecovers", _nativeFunction907, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileOpenRecover", _nativeFunction908, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileRemoveRecover", _nativeFunction909, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileSaveChanges", _nativeFunction910, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileCreate", _nativeFunction911, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileGetOpenChangesCount", _nativeFunction912, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileSetOpenChangesCount", _nativeFunction913, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileGetCurrentChangesIndex", _nativeFunction914, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileSave", _nativeFunction915, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileGetSourcePath", _nativeFunction916, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileSetSourcePath", _nativeFunction917, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileGetSaved", _nativeFunction918, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileSetSaved", _nativeFunction919, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileGetImageUrl", _nativeFunction920, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileGetImageUrlFromOpenFileDialog", _nativeFunction921, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("AscBrowserScrollStyle", _nativeFunction922, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("checkAuth", _nativeFunction923, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("execCommand", _nativeFunction924, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SetDropFiles", _nativeFunction925, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsImageFile", _nativeFunction926, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetDropFiles", _nativeFunction927, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("DropOfficeFiles", _nativeFunction928, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SetAdvancedOptions", _nativeFunction929, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("LocalFileRemoveAllRecents", _nativeFunction930, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("LocalFileRemoveAllRecovers", _nativeFunction931, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("CheckUserId", _nativeFunction932, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("ApplyAction", _nativeFunction933, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("InitJSContext", _nativeFunction934, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("NativeViewerOpen", _nativeFunction935, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("NativeViewerClose", _nativeFunction936, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("NativeFunctionTimer", _nativeFunction937, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("NativeViewerGetPageUrl", _nativeFunction938, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("NativeViewerGetCompleteTasks", _nativeFunction939, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("GetInstallPlugins", _nativeFunction940, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsLocalFile", _nativeFunction951, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsFilePrinting", _nativeFunction952, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("Sign", _nativeFunction953, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("ViewCertificate", _nativeFunction954, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SelectCertificate", _nativeFunction955, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetDefaultCertificate", _nativeFunction956, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_OpenFilenameDialog", _nativeFunction957, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SaveQuestion", _nativeFunction958, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("startReporter", _nativeFunction960, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("endReporter", _nativeFunction961, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("sendToReporter", _nativeFunction962, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("sendFromReporter", _nativeFunction963, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("IsSignaturesSupport", _nativeFunction964, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("RemoveSignature", _nativeFunction965, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("RemoveAllSignatures", _nativeFunction966, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SetSupportSign", _nativeFunction967, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("PluginInstall", _nativeFunction968, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("PluginUninstall", _nativeFunction969, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("SetInitFlags", _nativeFunction970, V8_PROPERTY_ATTRIBUTE_NONE);

    objNative->SetValue("isBlockchainSupport", _nativeFunction971, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("OpenAsLocal", _nativeFunction972, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SaveAsCloud", _nativeFunction973, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_sendSystemMessage", _nativeFunction974, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_GetHash", _nativeFunction975, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_CallInAllWindows", _nativeFunction976, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_OpenFileCrypt", _nativeFunction977, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("buildCryptedStart", _nativeFunction978, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("buildCryptedEnd", _nativeFunction979, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("SetCryptDocumentFolder", _nativeFunction980, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("PreloadCryptoImage", _nativeFunction981, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("ResaveFile", _nativeFunction982, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_DownloadFiles", _nativeFunction983, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("RemoveFile", _nativeFunction984, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetImageFormat", _nativeFunction985, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_SetCryptoMode", _nativeFunction986, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetEncryptedHeader", _nativeFunction987, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetCryptoMode", _nativeFunction988, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetSupportCryptoModes", _nativeFunction989, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsProtectionSupport", _nativeFunction990, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_GetAdvancedEncryptedData", _nativeFunction991, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("_SetAdvancedEncryptedData", _nativeFunction992, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("GetExternalClouds", _nativeFunction993, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("IsNativeViewer", _nativeFunction994, V8_PROPERTY_ATTRIBUTE_NONE);
    objNative->SetValue("CryptoDownloadAs", _nativeFunction995, V8_PROPERTY_ATTRIBUTE_NONE);


    object->SetValue("AscDesktopEditor", objNative, V8_PROPERTY_ATTRIBUTE_NONE);

    CefRefPtr<CefFrame> _frame = context->GetFrame();
    if (_frame)
        _frame->ExecuteJavaScript("window.AscDesktopEditor.AscBrowserScrollStyle();", _frame->GetURL(), 0);

    if (_frame)
        _frame->ExecuteJavaScript("window.AscDesktopEditor.InitJSContext();", _frame->GetURL(), 0);

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_js_context_created");
    browser->SendProcessMessage(PID_BROWSER, message);
  }

  virtual void OnContextReleased(CefRefPtr<client::ClientAppRenderer> app,
                                 CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) OVERRIDE {
    message_router_->OnContextReleased(browser,  frame, context);
  }

  virtual void OnFocusedNodeChanged(CefRefPtr<client::ClientAppRenderer> app,
                                    CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefDOMNode> node) OVERRIDE {
    bool is_editable = (node.get() && node->IsEditable());
    if (is_editable != last_node_is_editable_)
    {
      // Notify the browser of the change in focused element type.
      last_node_is_editable_ = is_editable;
#if 0
      CefRefPtr<CefProcessMessage> message =
          CefProcessMessage::Create(kFocusedNodeChangedMessage);
      message->GetArgumentList()->SetBool(0, is_editable);
      browser->SendProcessMessage(PID_BROWSER, message);
#endif
    }
  }

  virtual bool OnProcessMessageReceived(
      CefRefPtr<client::ClientAppRenderer> app,
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE
{

    std::string sMessageName = message->GetName().ToString();

    if (sMessageName == "keyboard_layout")
    {
        int nKeyboardLayout = message->GetArgumentList()->GetInt(0);
        std::string sLayout = std::to_string(nKeyboardLayout);
        std::string sCode = "window[\"asc_current_keyboard_layout\"] = " + sLayout + ";";

        std::vector<int64> ids;
        browser->GetFrameIdentifiers(ids);

        for (std::vector<int64>::iterator i = ids.begin(); i != ids.end(); i++)
        {
            CefRefPtr<CefFrame> _frame = browser->GetFrame(*i);
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "cef_control_id")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
        {
            int nControlId = message->GetArgumentList()->GetInt(0);
            bool isSupportSign = message->GetArgumentList()->GetBool(1);
            int nParams = message->GetArgumentList()->GetInt(2);
            std::string sControlId = std::to_string(nControlId);
            std::string sCode = "window[\"AscDesktopEditor\"][\"SetEditorId\"](" + sControlId + ");";

            if (isSupportSign)
                sCode += "window[\"AscDesktopEditor\"][\"SetSupportSign\"](true);";
            else
                sCode += "window[\"AscDesktopEditor\"][\"SetSupportSign\"](false);";

            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "on_js_context_created_callback")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
        {
            int nParams = message->GetArgumentList()->GetInt(0);
            int nCryptoMode = message->GetArgumentList()->GetInt(1);

            std::string sSystemPath = message->GetArgumentList()->GetString(2);
            std::string sUserPath = message->GetArgumentList()->GetString(3);
            int nSizeCount = (int)(message->GetArgumentList()->GetSize()) - 2;

            std::string sObject = "[";
            for (int i = 0; i < nSizeCount; i++)
            {
                std::string sValue = message->GetArgumentList()->GetString(i + 2);

                NSCommon::string_replaceA(sValue, "\"", "\\\"");

                sObject += ("\"" + sValue + "\"");
                if (i != (nSizeCount - 1))
                    sObject += ",";
            }
            sObject += "]";

            std::string sCode = ("window[\"AscDesktopEditor\"][\"SetInitFlags\"](" + std::to_string(nParams) + ", " + std::to_string(nCryptoMode) + ", " + sObject + ");");

            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "spell_check_response")
    {
        int64 nFrameId = (int64)message->GetArgumentList()->GetInt(1);
        CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
        if (_frame)
        {
            std::string sCode = "window[\"asc_nativeOnSpellCheck\"](" + message->GetArgumentList()->GetString(0).ToString() + ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "sync_command_end")
    {
        sync_command_check = false;
        return true;
    }
    else if (sMessageName == "on_is_cookie_present")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
        bool bIsPresent = message->GetArgumentList()->GetBool(0);
        std::string sValue = message->GetArgumentList()->GetString(1).ToString();

        std::string sCode = bIsPresent ? ("if (window[\"on_is_cookie_present\"]) { window[\"on_is_cookie_present\"](true, \"" + sValue + "\"); }") :
                                 "if (window[\"on_is_cookie_present\"]) { window[\"on_is_cookie_present\"](false, undefined); }";
        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

        return true;
    }
    else if (sMessageName == "on_check_auth")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
        int nCount = message->GetArgumentList()->GetInt(0);

        std::string sObject = "{";
        for (int i = 0; i < nCount; i++)
        {
            std::string sKey = message->GetArgumentList()->GetString(1 + i * 2);
            std::string sValue = message->GetArgumentList()->GetString(2 + i * 2);

            NSCommon::string_replaceA(sKey, "\"", "\\\"");
            NSCommon::string_replaceA(sValue, "\"", "\\\"");

            sObject += ("\""  + sKey + "\" : \"" + sValue + "\"");
            if (i != (nCount - 1))
                sObject += ",";
        }
        sObject += "}";

        std::string sCode = "if (window[\"on_check_auth\"]) { window[\"on_check_auth\"](" + sObject + "); }";

        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

        return true;
    }
    else if (sMessageName == "on_set_cookie")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();

        std::string sCode = "if (window[\"on_set_cookie\"]) { window[\"on_set_cookie\"](); }";
        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

        return true;
    }
    else if (sMessageName == "document_save")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sCode = "if (window[\"AscDesktopEditor_Save\"]) { window[\"AscDesktopEditor_Save\"](); }";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "print")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sCode = "if (window[\"Asc\"] && window[\"Asc\"][\"editor\"]) { window[\"Asc\"][\"editor\"][\"asc_nativePrint\"](undefined, undefined); }";
            sCode += "else if (window[\"editor\"]) { window[\"editor\"][\"asc_nativePrint\"](undefined, undefined); }";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_load_js")
    {
        int64 frameId = (int64)message->GetArgumentList()->GetInt(2);
        CefRefPtr<CefFrame> _frame = browser->GetFrame(frameId);

        if (_frame)
        {
            std::wstring sFilePath = message->GetArgumentList()->GetString(0).ToWString();
            std::wstring sUrl = message->GetArgumentList()->GetString(1).ToWString();
            NSFile::CFileBinary oFile;
            if (oFile.OpenFile(sFilePath))
            {
                int nSize = (int)oFile.GetFileSize();
                BYTE* scriptData = new BYTE[nSize];
                DWORD dwReadSize = 0;
                oFile.ReadFile(scriptData, (DWORD)nSize, dwReadSize);

                std::string strUTF8((char*)scriptData, nSize);

                delete [] scriptData;
                scriptData = NULL;

                _frame->ExecuteJavaScript(strUTF8, _frame->GetURL(), 0);

                _frame->ExecuteJavaScript("window[\"asc_desktop_on_load_js\"](\"" + U_TO_UTF8(sUrl) + "\");", _frame->GetURL(), 0);
            }
            else
            {
                // все равно посылаем - пусть лучше ошибка в консоль, чем подвисание в requirejs
                _frame->ExecuteJavaScript("window[\"asc_desktop_on_load_js\"](\"" + U_TO_UTF8(sUrl) + "\");", _frame->GetURL(), 0);
            }
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_loadend")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (!_frame)
            _frame = browser->GetMainFrame();

        if (_frame)
        {
            std::wstring sFolder = message->GetArgumentList()->GetString(0).ToWString();
            std::wstring sFileSrc = message->GetArgumentList()->GetString(1).ToWString();

            bool bIsSaved = message->GetArgumentList()->GetBool(2);

            std::wstring sSignatures = message->GetArgumentList()->GetString(3).ToWString();
            NSCommon::string_replace(sSignatures, L"\"", L"\\\"");

            if (bIsSaved)
                _frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(true);", _frame->GetURL(), 0);
            else
                _frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(false);", _frame->GetURL(), 0);

            int nFileDataLen = 0;
            std::string sFileData = GetFileData(sFolder + L"/Editor.bin", nFileDataLen);

            std::string sCode = "window.AscDesktopEditor.LocalFileRecoverFolder(\"" + U_TO_UTF8(sFolder) +
                    "\");window.AscDesktopEditor.LocalFileSetSourcePath(\"" + U_TO_UTF8(sFileSrc) + "\");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            if (NSFile::CFileBinary::Exists(sFolder + L"/changes/changes0.json"))
            {
                std::string sChanges;
                NSFile::CFileBinary::ReadAllTextUtf8A(sFolder + L"/changes/changes0.json", sChanges);
                if (0 < sChanges.length())
                    sChanges[sChanges.length() - 1] = ']';

                const char* pDataCheck = (const char*)sChanges.c_str();
                const char* pDataCheckLimit = pDataCheck + sChanges.length();
                int nCounter = 0;
                while (pDataCheck != pDataCheckLimit)
                {
                    if (*pDataCheck == '\"')
                        ++nCounter;
                    ++pDataCheck;
                }
                nCounter >>= 1;

                sChanges = "window.DesktopOfflineAppDocumentApplyChanges([" +
                        sChanges + ");window.AscDesktopEditor.LocalFileSetOpenChangesCount(" +
                        std::to_string(nCounter) + ");";
                _frame->ExecuteJavaScript(sChanges, _frame->GetURL(), 0);
            }

            sCode = "";
            sCode = "window.DesktopOfflineAppDocumentEndLoad(\"" + U_TO_UTF8(sFolder) + "\", \"" + sFileData + "\", " + std::to_string(nFileDataLen) + ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            sCode = "window.DesktopOfflineAppDocumentSignatures(\"" + U_TO_UTF8(sSignatures) + "\");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_onsaveend")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sFileSrc = message->GetArgumentList()->GetString(0).ToString();
            int nIsSaved = message->GetArgumentList()->GetInt(1);

            // 0 - ok
            // 1 - cancel
            // 2 - error

            if (0 == nIsSaved)
                _frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(true);", _frame->GetURL(), 0);
            else
                _frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(false);", _frame->GetURL(), 0);

            if (!sFileSrc.empty())
            {
                std::string sCode = "window.AscDesktopEditor.LocalFileSetSourcePath(\"" + sFileSrc + "\");";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }

            std::string sCode = "window.DesktopOfflineAppDocumentEndSave(" + std::to_string(nIsSaved);

            std::string sHash = "";
            std::string sPass = "";
            if (4 <= message->GetArgumentList()->GetSize())
            {
                sPass = message->GetArgumentList()->GetString(2).ToString();
                sHash = message->GetArgumentList()->GetString(3).ToString();

                NSCommon::string_replaceA(sPass, "\\", "\\\\");
                NSCommon::string_replaceA(sPass, "\"", "\\\"");

                if (!sPass.empty() && !sHash.empty())
                {
                    sCode += ", \"";
                    sCode += sHash;
                    sCode += "\", \"";
                    sCode += sPass;
                    sCode += "\"";
                }
            }

            sCode += ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

            if (5 <= message->GetArgumentList()->GetSize())
            {
                std::string sUrlDst = message->GetArgumentList()->GetString(4).ToString();

                sCode = "window.DesktopUploadFileToUrl(\"" + sFileSrc + "\", \"" + sUrlDst + "\", \"" + sHash + "\", \"" + sPass + "\");";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_sendrecents")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();

        if (_frame)
        {
            std::wstring sJSON = message->GetArgumentList()->GetString(0).ToWString();
            NSCommon::string_replace(sJSON, L"\\", L"\\\\");

            std::wstring sCode = L"if (window.onupdaterecents) {window.onupdaterecents(" + sJSON + L");}";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_sendrecovers")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();

        if (_frame)
        {
            std::wstring sJSON = message->GetArgumentList()->GetString(0).ToWString();
            NSCommon::string_replace(sJSON, L"\\", L"\\\\");

            std::wstring sCode = L"if (window.onupdaterecovers) {window.onupdaterecovers(" + sJSON + L");}";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_onaddimage")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::wstring sPath = message->GetArgumentList()->GetString(0).ToWString();
            NSCommon::string_replace(sPath, L"\\", L"\\\\");

            std::wstring sCode = L"window.DesktopOfflineAppDocumentAddImageEnd(\"" + sPath + L"\");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_native_message")
    {
        std::string sCommand = message->GetArgumentList()->GetString(0).ToString();
        std::string sParam = message->GetArgumentList()->GetString(1).ToString();
        std::string sFrameName = message->GetArgumentList()->GetString(2).ToString();

        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
        if (!sFrameName.empty())
        {
            _frame = browser->GetFrame(sFrameName);
        }

        if (!_frame)
            return true;

        std::string sCode = "if (window.on_native_message) {window.on_native_message(\"" + sCommand + "\", \"" + sParam + "\");}";
        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

        return true;
    }
    else if (sMessageName == "on_editor_native_message")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sCommand = message->GetArgumentList()->GetString(0).ToString();
            std::string sParam = message->GetArgumentList()->GetString(1).ToString();

            std::string sCode = "if (window.on_editor_native_message) {window.on_editor_native_message(\"" + sCommand + "\", \"" + sParam + "\");}";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "onlocaldocument_additionalparams")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sParam = std::to_string(message->GetArgumentList()->GetInt(0));
            std::string sCode = "window.asc_initAdvancedOptions(" + sParam;

            if (2 <= message->GetArgumentList()->GetSize())
            {
                std::string sHash = message->GetArgumentList()->GetString(1).ToString();
                sCode += ",\"";
                sCode += sHash;
                sCode += "\"";
            }
            if (3 <= message->GetArgumentList()->GetSize())
            {
                std::string sDocInfo = message->GetArgumentList()->GetString(2).ToString();
                sCode += ",\"";
                sCode += sDocInfo;
                sCode += "\"";
            }

            sCode += ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }    
    else if (sMessageName == "native_viewer_onopened")
    {
        m_bIsNativeViewer = true;
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
        {
            std::wstring s1 = message->GetArgumentList()->GetString(0).ToWString();
            std::wstring s2 = message->GetArgumentList()->GetString(1).ToWString();
            std::wstring s3 = message->GetArgumentList()->GetString(2).ToWString();

            NSCommon::string_replace(s1, L"\\", L"/");
            NSCommon::string_replace(s2, L"\\", L"/");
            NSCommon::string_replace(s3, L"\\", L"/");

            std::string sCode = "window.AscDesktopEditor.NativeViewerOpen(\"" + U_TO_UTF8(s1) +
                    "\", \"" + U_TO_UTF8(s2) + "\", \"" + U_TO_UTF8(s3) + "\");";

            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "update_install_plugins")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
        {
            std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_signature_defaultcertificate_ret" ||
             sMessageName == "on_signature_selectsertificate_ret" ||
             sMessageName == "on_signature_viewcertificate_ret")
    {
        std::string sParam = message->GetArgumentList()->GetString(0).ToString();

        std::string sCode = "window.OnNativeReturnCallback(\"" + sMessageName + "\", " + sParam + ");";

        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
    }
    else if (sMessageName == "on_open_filename_dialog")
    {
        CefRefPtr<CefFrame> _frame;

        std::string sId = message->GetArgumentList()->GetString(0).ToString();
        if (sId.empty())
            _frame = GetEditorFrame(browser);
        else
        {
            int64 nId = (int64)std::stoll(sId);
            _frame = browser->GetFrame(nId);
        }
        bool bIsMulti = message->GetArgumentList()->GetBool(1);

        std::wstring sParamCallback = L"";
        if (!bIsMulti)
        {
            std::wstring sPath = message->GetArgumentList()->GetString(2).ToWString();
            NSCommon::string_replace(sPath, L"\\", L"\\\\");
            sParamCallback = L"\"" + sPath + L"\"";
        }
        else
        {
            sParamCallback = L"[";

            int nCount = message->GetArgumentList()->GetSize();
            for (int nIndex = 2; nIndex < nCount; nIndex++)
            {
                std::wstring sPath = message->GetArgumentList()->GetString(nIndex).ToWString();
                NSCommon::string_replace(sPath, L"\\", L"\\\\");
                sParamCallback += (L"\"" + sPath + L"\"");

                if (nIndex < (nCount - 1))
                    sParamCallback += L",";
            }

            sParamCallback += L"]";
        }

        if (_frame)
        {
            std::wstring sPath = message->GetArgumentList()->GetString(0).ToWString();
            NSCommon::string_replace(sPath, L"\\", L"\\\\");

            std::wstring sCode = L"(function() { window.on_native_open_filename_dialog(" + sParamCallback + L"); delete window.on_native_open_filename_dialog; })();";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_signature_update_signatures")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::wstring sSignatures = message->GetArgumentList()->GetString(0).ToWString();
            NSCommon::string_replace(sSignatures, L"\"", L"\\\"");

            std::string sCode = "window.DesktopOfflineAppDocumentSignatures(\"" + U_TO_UTF8(sSignatures) + "\");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_file_save_question")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            bool bValue = message->GetArgumentList()->GetBool(0);
            std::string sCode = "window.DesktopSaveQuestionReturn(";
            sCode += (bValue ? "true" : "false");
            sCode += ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "from_reporter_message")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sParam = message->GetArgumentList()->GetString(0);
            NSCommon::string_replaceA(sParam, "\\", "\\\\");
            NSCommon::string_replaceA(sParam, "\"", "\\\"");
            _frame->ExecuteJavaScript("window.editor.DemonstrationReporterMessages({ data : \"" + sParam + "\" });", _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "to_reporter_message")
    {
        CefRefPtr<CefFrame> _frame = browser->GetMainFrame();

        if (_frame)
        {
            std::string sParam = message->GetArgumentList()->GetString(0);
            NSCommon::string_replaceA(sParam, "\\", "\\\\");
            NSCommon::string_replaceA(sParam, "\"", "\\\"");
            _frame->ExecuteJavaScript("window.editor.DemonstrationToReporterMessages({ data : \"" + sParam + "\" });", _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "file_get_hash_callback")
    {
        int64 frameID = (int64)std::stoll(message->GetArgumentList()->GetString(1).ToString());
        CefRefPtr<CefFrame> _frame = browser->GetFrame(frameID);

        if (_frame)
        {
            std::string sCode = "if (window.AscDesktopEditor.getHashCallback) { window.AscDesktopEditor.getHashCallback(\"";
            sCode += message->GetArgumentList()->GetString(0).ToString();
            sCode += "\"); window.AscDesktopEditor.getHashCallback = null; }";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "send_system_message")
    {
        std::string sArg = message->GetArgumentList()->GetString(0);
        NSCommon::string_replaceA(sArg, "\\", "\\\\");
        NSCommon::string_replaceA(sArg, "\"", "\\\"");

        if (message->GetArgumentList()->GetSize() == 1)
        {
            // main view
            std::vector<int64> identifiers;
            browser->GetFrameIdentifiers(identifiers);

            for (std::vector<int64>::iterator i = identifiers.begin(); i != identifiers.end(); i++)
            {
                int64 k = *i;
                CefRefPtr<CefFrame> frame = browser->GetFrame(k);

                if (frame && (frame->GetName().ToString().find("system_asc") == 0))
                {
                    std::string sCode = "\
    (function(){\n\
    try {\n\
    var _arg = JSON.parse(\"" + sArg + "\");\n\
    window.AscDesktopEditor.isSendSystemMessage = false;\n\
    if (window.onSystemMessage)\n\
    { window.onSystemMessage(_arg); } else if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
    { window.Asc.plugin.onSystemMessage(_arg); }\n\
    }\n\
    catch (err) {\n\
        if (!window.AscDesktopEditor.isSendSystemMessage) {\n\
            window.AscDesktopEditor.sendSystemMessage(_arg);\n\
        }\n\
    }\n\
    delete window.AscDesktopEditor.isSendSystemMessage;\n\
    })();";

                    frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
                    break;
                }
            }
        }
        else
        {
            int64 frameID = (int64)std::stoll(message->GetArgumentList()->GetString(1).ToString());
            CefRefPtr<CefFrame> _frame = browser->GetFrame(frameID);

            if (_frame)
            {
                std::string sCode = "\
    (function(){\n\
    try {\n\
    var _arg = JSON.parse(\"" + sArg + "\");\n\
    if (window.onSystemMessage)\n\
    { window.onSystemMessage(_arg); } else if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
    { window.Asc.plugin.onSystemMessage(_arg); }\n\
    }\n\
    catch (err) {}\n\
    })();";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }

            if (true)
            {
                std::vector<int64> identifiers;
                browser->GetFrameIdentifiers(identifiers);

                for (std::vector<int64>::iterator i = identifiers.begin(); i != identifiers.end(); i++)
                {
                    int64 k = *i;
                    CefRefPtr<CefFrame> _frameOP = browser->GetFrame(k);

                    if (_frameOP && (k != frameID) && (_frameOP->GetName().ToString().find("iframe_asc.{") == 0))
                    {
                        std::string sCode = "\
        (function(){\n\
        try {\n\
        var _arg = JSON.parse(\"" + sArg + "\");\n\
        if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
        { window.Asc.plugin.onSystemMessage(_arg); } \n\
        }\n\
        catch (err) {}\n\
        })();";

                        _frameOP->ExecuteJavaScript(sCode, _frameOP->GetURL(), 0);
                        break;
                    }
                }
            }
        }
        return true;
    }
    else if (sMessageName == "onload_crypt_document")
    {
        std::wstring sFilePathW = message->GetArgumentList()->GetString(0).ToWString();
        std::string sFilePath = U_TO_UTF8(sFilePathW);

        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::wstring sDirectoryRecover = NSFile::GetDirectoryName(sFilePathW);
            std::string sDirectoryRecoverA = U_TO_UTF8(sDirectoryRecover);
            std::string sCode1 = "window.AscDesktopEditor.SetCryptDocumentFolder(\"" + sDirectoryRecoverA + "\");\n";

            std::string sCode = "\
var xhr = new XMLHttpRequest();\n\
xhr.open(\"GET\", \"ascdesktop://fonts/" + sFilePath + "\", true);\n\
xhr.responseType = \"arraybuffer\";\n\
if (xhr.overrideMimeType)\n\
    xhr.overrideMimeType('text/plain; charset=x-user-defined');\n\
else\n\
    xhr.setRequestHeader('Accept-Charset', 'x-user-defined');\n\
\n\
xhr.onload = function()\n\
{\n\
    var fileData = new Uint8Array(xhr.response);\n\
\n\
    window.AscDesktopEditor.openFileCryptCallback(fileData);\n\
    window.AscDesktopEditor.openFileCryptCallback = null;\n\
};\n\
\n\
xhr.send(null);";

            _frame->ExecuteJavaScript(sCode1 + sCode, _frame->GetURL(), 0);
        }

        return true;
    }
    else if (sMessageName == "is_need_build_crypted_file")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
            _frame->ExecuteJavaScript("window.asc_IsNeedBuildCryptedFile();", _frame->GetURL(), 0);
        return true;
    }
    else if (sMessageName == "build_crypted_file")
    {
        std::string sFolderName = message->GetArgumentList()->GetString(0);
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
            _frame->ExecuteJavaScript("window.buildCryptoFile_Start();", _frame->GetURL(), 0);
        return true;
    }
    else if (sMessageName == "build_crypted_file_end")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            std::string sFileSrc = message->GetArgumentList()->GetString(0).ToString();
            int nIsSaved = message->GetArgumentList()->GetInt(1);

            // 0 - ok
            // 1 - cancel
            // 2 - error
            if (!sFileSrc.empty())
            {
                std::string sCode = "window.AscDesktopEditor.LocalFileSetSourcePath(\"" + sFileSrc + "\");";
                _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            }

            std::string sCode = "window.buildCryptoFile_End(\"" + sFileSrc + "\", " + std::to_string(nIsSaved);

            std::string sHash = "";
            std::string sPass = "";
            if (4 <= message->GetArgumentList()->GetSize())
            {
                sPass = message->GetArgumentList()->GetString(2).ToString();
                sHash = message->GetArgumentList()->GetString(3).ToString();

                NSCommon::string_replaceA(sPass, "\\", "\\\\");
                NSCommon::string_replaceA(sPass, "\"", "\\\"");

                if (!sPass.empty() && !sHash.empty())
                {
                    sCode += ", \"";
                    sCode += sHash;
                    sCode += "\", \"";
                    sCode += sPass;
                    sCode += "\"";
                }
            }

            sCode += ");";
            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "on_preload_crypto_image")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

        if (_frame)
        {
            int nType = message->GetArgumentList()->GetInt(0);
            std::wstring sFileSrc = message->GetArgumentList()->GetString(1).ToWString();

            std::string sUrl = U_TO_UTF8(sFileSrc);
            std::string sUrlNatural = sUrl;
            std::string sData = "";
            if (0 == nType)
            {
                sUrlNatural = message->GetArgumentList()->GetString(2).ToString();
                sUrl = "ascdesktop://fonts/" + sUrl;

                std::string sTest = "";
                NSFile::CFileBinary oFile;
                if (oFile.OpenFile(sFileSrc))
                {
                    if (oFile.GetFileSize() > 11)
                    {
                        BYTE data[11];
                        memset(data, 0, 11);
                        DWORD dwSize = 0;
                        oFile.ReadFile(data, 10, dwSize);
                        sTest = std::string((char*)data);
                    }
                }
                oFile.CloseFile();

                if (sTest == "ENCRYPTED;")
                {
                    NSFile::CFileBinary::ReadAllTextUtf8A(sFileSrc, sData);

                    // read extension
                    std::string::size_type nPos = sData.find(';', 10);
                    if (nPos != std::string::npos)
                        sData = "ENCRYPTED;" + sData.substr(nPos + 1);
                }
            }
            else
            {
                // not local url
            }

            std::string sCode = "(function(){\n\
var _url = \"" + sUrlNatural + "\";\n\
var _images = window[\"crypto_images_map\"][_url];\n\
if (!_images) { return; }\n\
for (var i = 0; i < _images.length; i++) \n\
{\n\
_images[i].onload_crypto(\"" + sUrl + "\", \"" + sData + "\");\n\
}\n\
delete window[\"crypto_images_map\"][_url];\n\
})();";

            _frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
            return true;
        }
    }
    else if (sMessageName == "on_download_files")
    {
        CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
        if (_frame)
        {
            std::string sParam = "{";
            int nCount = message->GetArgumentList()->GetSize();
            int nIndex = 0;
            while (nIndex < nCount)
            {
                sParam += ("\"" + message->GetArgumentList()->GetString(nIndex++).ToString() + "\" : \"");

                std::string sFile = message->GetArgumentList()->GetString(nIndex++).ToString();
                NSCommon::string_replaceA(sFile, "\\", "/");

                sParam += (sFile + "\"");

                if (nIndex < nCount)
                    sParam += ", ";
            }
            sParam += "}";

            _frame->ExecuteJavaScript("(function() { window.on_native_download_files(" + sParam + "); delete window.on_native_download_files; })();", _frame->GetURL(), 0);
        }
        return true;
    }
    else if (sMessageName == "set_crypto_mode")
    {
        int nError = message->GetArgumentList()->GetInt(0);
        int64 nFrameId = (int64)message->GetArgumentList()->GetInt(1);

        CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
        if (_frame)
            _frame->ExecuteJavaScript("(function() { if (!window.on_set_crypto_mode) return; window.on_set_crypto_mode(" + std::to_string(nError) + "); delete window.on_set_crypto_mode; })();", _frame->GetURL(), 0);
    }
    else if (sMessageName == "get_advanced_encrypted_data")
    {
        int64 nFrameId = (int64)message->GetArgumentList()->GetInt(0);
        std::string sRet = message->GetArgumentList()->GetString(1);
        NSCommon::string_replaceA(sRet, "\\", "\\\\");

        CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
        if (_frame)
              _frame->ExecuteJavaScript("(function() { if (!window.on_get_advanced_encrypted_data) return; window.on_get_advanced_encrypted_data(\"" + sRet + "\"); delete window.on_get_advanced_encrypted_data; })();", _frame->GetURL(), 0);
    }
    else if (sMessageName == "set_advanced_encrypted_data")
    {
        int64 nFrameId = (int64)message->GetArgumentList()->GetInt(0);
        std::string sRet = message->GetArgumentList()->GetString(1);
        NSCommon::string_replaceA(sRet, "\\", "\\\\");

        CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
        if (_frame)
            _frame->ExecuteJavaScript("(function() { if (!window.on_set_advanced_encrypted_data) return; window.on_set_advanced_encrypted_data(\"" + sRet + "\"); delete window.on_set_advanced_encrypted_data; })();", _frame->GetURL(), 0);
    }
    else if (sMessageName == "crypto_download_as_end")
    {
      CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
      if (_frame)
      {
          _frame->ExecuteJavaScript("(function() { window.CryptoDownloadAsEnd(); })();", _frame->GetURL(), 0);
      }
      return true;
    }

    if (m_pAdditional.is_init() && m_pAdditional->OnProcessMessageReceived(app, browser, source_process, message))
        return true;

    return message_router_->OnProcessMessageReceived(
        browser, source_process, message);
}

private:
  CefRefPtr<CefFrame> GetEditorFrame(CefRefPtr<CefBrowser> browser)
  {
    CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
    if (_frame)
        return _frame;
    return browser->GetMainFrame();
  }

  std::string GetFileData(const std::wstring& sFile, int& nLen)
  {
    if (m_bIsNativeViewer)
    {
        std::string sFileData;
        NSFile::CFileBinary::ReadAllTextUtf8A(sFile, sFileData);

        std::string::size_type pos = sFileData.find(";");
        nLen = std::stoi(sFileData.substr(0, pos));
        sFileData = sFileData.substr(pos + 1);
        return sFileData;
    }

    BYTE* pData = NULL;
    DWORD dwFileLen = 0;
    NSFile::CFileBinary::ReadAllBytes(sFile, &pData, dwFileLen);

    char* pDataBase64 = NULL;
    int nDataBase64Len = 0;

    nLen = (int)dwFileLen;

    NSFile::CBase64Converter::Encode(pData, nLen, pDataBase64, nDataBase64Len, NSBase64::B64_BASE64_FLAG_NOCRLF);

    std::string sFileData(pDataBase64, nDataBase64Len);

    RELEASEARRAYOBJECTS(pDataBase64);
    RELEASEARRAYOBJECTS(pData);

    return sFileData;
  }

 private:
  bool last_node_is_editable_;
  bool sync_command_check;

  // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  NSCommon::smart_ptr<CApplicationRendererAdditionalBase> m_pAdditional;
  bool m_bIsNativeViewer;

  IMPLEMENT_REFCOUNTING(ClientRenderDelegate);
};

void CreateRenderDelegates(client::ClientAppRenderer::DelegateSet& delegates) {
  delegates.insert(new ClientRenderDelegate);
}

// static
}  // namespace client_renderer

void client::ClientAppRenderer::CreateDelegates(client::ClientAppRenderer::DelegateSet& delegates) {
    asc_client_renderer::CreateRenderDelegates(delegates);
}
