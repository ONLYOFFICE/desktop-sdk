/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#ifndef APPLICATION_MANAGER_PRIVATE_H
#define APPLICATION_MANAGER_PRIVATE_H

#include "../include/applicationmanager.h"
#include <map>

#include "../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "../../../../core/DesktopEditor/graphics/BaseThread.h"
#include "../../../../core/DesktopEditor/graphics/Timer.h"

#include "../src/cookiesworker.h"

#include <iostream>
#include <set>

#include "./filedownloader.h"
#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "../../../../core/Common/OfficeFileFormatChecker.h"

#include "../../../../core/DesktopEditor/fontengine/application_generate_fonts.h"

#include <stdio.h>
#include <time.h>

#define ASC_CONSTANT_CANCEL_SAVE 0x00005678

#define LOCK_CS_SCRIPT 0

#include "Logger.h"

#include "./additional/manager.h"
#include "./additional/renderer.h"

#ifdef LINUX
#include "signal.h"
void posix_death_signal(int signum);

class CLinuxData
{
public:
    static CApplicationCEF*         app_cef;
    static CAscApplicationManager*  app_manager;

public:
    static void Check(CAscApplicationManager* manager)
    {
        if (NULL == app_manager)
        {
            app_manager = manager;
            signal(SIGSEGV, posix_death_signal);
            signal(SIGTERM, posix_death_signal);
        }
    }
    static void Close()
    {
        if (NULL != app_manager)
        {
            app_manager->CloseApplication();
        }
    }
};

#endif

#if defined(_LINUX)

#include <pwd.h>
static std::wstring GetHomeDirectory()
{
    const char* sHome = std::getenv("home");

    if (sHome == NULL)
    {
        sHome = getpwuid(getuid())->pw_dir;
    }

    if (NULL == sHome)
        return L"";

    std::string temp = sHome;
    return NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)temp.c_str(), (LONG)temp.length());
}

#endif

class CCefFileDownloaderThread : public NSThreads::CBaseThread, public IFileDownloaderEvents
{
public :
    CCefFileDownloaderThread(CAscApplicationManager* pManager, std::wstring sFileUrl, std::wstring sFileDst) : NSThreads::CBaseThread()
    {
        m_sFilePath = sFileDst;
        m_sFileUrl  = sFileUrl;
        m_bComplete = false;
        m_bWork = false;
        m_pManager = pManager;
        m_pDownloader = NULL;
    }
    ~CCefFileDownloaderThread()
    {
        RELEASEOBJECT(m_pDownloader);
    }

    bool IsFileDownloaded()
    {
        return m_bComplete;
    }

    void OnProgress(int nProgress)
    {
    }

    void OnDownload(bool bIsSuccess)
    {
        m_bComplete = bIsSuccess;
        m_bWork = false;
    }

protected :

    virtual DWORD ThreadProc ()
    {
        m_bComplete = false;
        m_bWork = true;

        m_pDownloader = new CCefFileDownloader(this);
        m_pDownloader->DownloadFile(m_pManager, m_sFileUrl, m_sFilePath);

        while (m_bWork)
        {
            NSThreads::Sleep(100);
        }

        m_bRunThread = FALSE;
        return 0;
    }

protected :
    std::wstring    m_sFilePath;       // Путь к сохраненному файлу на диске
    std::wstring    m_sFileUrl;        // Ссылка на скачивание файла

    bool            m_bComplete;       // Закачался файл или нет
    bool            m_bWork;

    CCefFileDownloader* m_pDownloader;
    CAscApplicationManager* m_pManager;
};

class CCefScriptLoader : public NSThreads::CBaseThread
{
public:
    class ICefScriptLoaderCallback
    {
    public:
        virtual void OnLoad(CCefScriptLoader*, bool error) {}
    };

public:
    std::string     m_sVersion;
    std::wstring    m_sUrl;
    std::wstring    m_sDestination;

    CAscApplicationManager* m_pManager;
    ICefScriptLoaderCallback* m_pListener;

public:

    CCefScriptLoader() : NSThreads::CBaseThread()
    {
        m_pListener = NULL;
        m_pManager = NULL;
    }

protected:
    virtual DWORD ThreadProc()
    {
        if (std::wstring::npos != m_sUrl.find(L"sdk/Common/AllFonts.js") ||
            std::wstring::npos != m_sUrl.find(L"sdkjs/common/AllFonts.js"))
        {
            while (!m_pManager->IsInitFonts())
            {
                NSThreads::Sleep( 10 );
            }

            m_bRunThread = FALSE;

            if (NULL != m_pListener)
                m_pListener->OnLoad(this, false);
        }
        else
        {
            // старый код. теперь используется вью портала
#ifdef WIN32
            CFileDownloader oDownloader(m_sUrl, false);
            oDownloader.SetFilePath(m_sDestination);
#else
            CCefFileDownloaderThread oDownloader(m_pManager, m_sUrl, m_sDestination);
#endif
            oDownloader.Start( 0 );
            while ( oDownloader.IsRunned() )
            {
                NSThreads::Sleep( 10 );
            }

            m_bRunThread = FALSE;

            if (NULL != m_pListener)
                m_pListener->OnLoad(this, oDownloader.IsFileDownloaded() ? false : true);
        }

        return 0;
    }
};

///////////////////////////////////////////////////////
class CEditorFrameId
{
public:
    int EditorId;
    int FrameId;
};

class CAscApplicationManager_Private;
class CTimerKeyboardChecker : public NSTimers::CTimer
{
    CAscApplicationManager_Private* m_pManager;
public:
    CTimerKeyboardChecker(CAscApplicationManager_Private* pManagerPrivate) : NSTimers::CTimer()
    {
        m_pManager = pManagerPrivate;
        SetInterval(100);
    }
    virtual ~CTimerKeyboardChecker()
    {
    }

protected:
    virtual void OnTimer();
};

class CAscEditorFileInfo
{
public:
    std::wstring    m_sPath;
    int             m_nId;
    int             m_nFileType;
    bool            m_bIsViewer;
    std::wstring    m_sDate;
    std::wstring    m_sUrl;

public:
    CAscEditorFileInfo()
    {
        m_sPath = L"";
        m_nId = -1;
        m_nFileType = 0;
        m_bIsViewer = false;
    }

    void UpdateDate()
    {
        time_t timer;
        time(&timer);

        struct tm* timeinfo;
        timeinfo = localtime(&timer);

        m_sDate = Get2(timeinfo->tm_mday) + L"." +
                    Get2(timeinfo->tm_mon + 1) + L"." +
                    std::to_wstring(timeinfo->tm_year + 1900) + L" " +
                    Get2(timeinfo->tm_hour) + L":" +
                    Get2(timeinfo->tm_min);
    }

    std::wstring Get2(const int& nVal)
    {
        if (nVal < 10)
            return L"0" + std::to_wstring(nVal);
        return std::to_wstring(nVal);
    }
};

class CAscApplicationManager_Private : public CefBase, public CCookieFoundCallback, public NSThreads::CBaseThread, public CCefScriptLoader::ICefScriptLoaderCallback
{
public:
    CAscSpellChecker    m_oSpellChecker;
    CAscKeyboardChecker m_oKeyboardChecker;

    int                 m_nIdCounter;

    NSEditorApi::CAscMenuEventListener* m_pListener;

    std::map<int, CCefView*> m_mapViews;    

    CAscApplicationManager* m_pMain;

    CApplicationFonts* m_pApplicationFonts;

    NSCriticalSection::CRITICAL_SECTION m_oCS_Scripts;
    std::map<std::wstring, std::vector<CEditorFrameId>> m_mapLoadedScripts;

    CApplicationCEF* m_pApplication;
    bool m_bDebugInfoSupport;

    int m_nIsCefSaveDialogWait;

    CTimerKeyboardChecker m_oKeyboardTimer;
    
    int m_nWindowCounter;
    
    std::wstring m_strPrivateDownloadUrl;
    std::wstring m_strPrivateDownloadPath;
    
    std::map<unsigned int, bool> m_mapCancelDownloads;

    NSCriticalSection::CRITICAL_SECTION m_oCS_LocalFiles;
    std::vector<CAscEditorFileInfo> m_arRecents;
    std::vector<CAscEditorFileInfo> m_arRecovers;
    
    std::wstring m_sAdditionalUrlParams;    

    std::vector<NSEditorApi::CAscMenuEvent*> m_arApplyEvents;

    CApplicationManagerAdditionalBase* m_pAdditional;

public:
    CAscApplicationManager_Private() : m_oKeyboardTimer(this)
    {
        m_pListener = NULL;
        m_nIdCounter = 0;
        this->AddRef();
        m_pMain = NULL;

        m_pApplicationFonts = NULL;

        m_pApplication = NULL;

        m_bDebugInfoSupport = false;

        m_nIsCefSaveDialogWait = -1;
        m_nWindowCounter = 0;

        m_oCS_Scripts.InitializeCriticalSection();
        m_oCS_LocalFiles.InitializeCriticalSection();
        
        m_sAdditionalUrlParams = L"";

        m_pAdditional = NULL;
    }
    bool GetEditorPermission()
    {
        return m_pAdditional ? m_pAdditional->GetEditorPermission() : true;
    }

    virtual ~CAscApplicationManager_Private()
    {
        CloseApplication();
        RELEASEOBJECT(m_pAdditional);
        m_oCS_Scripts.DeleteCriticalSection();
        m_oCS_LocalFiles.DeleteCriticalSection();
    }

    void CloseApplication()
    {
        m_oKeyboardTimer.Stop();
        Stop();
        m_oSpellChecker.End();
    }

    void Logout(std::wstring strUrl, CefRefPtr<CefCookieManager> manager)
    {
        CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
        pVisitor->m_pCallback       = this;
        pVisitor->m_bIsDelete       = true;
        pVisitor->m_sDomain         = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strUrl.c_str(), strUrl.length());

        pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));
    }

    virtual void OnFoundCookie(bool bIsPresent, std::string sValue)
    {
        // not used
    }

    virtual void OnSetCookie()
    {
        // not used
    }

    virtual void OnFoundCookies(std::map<std::string, std::string>& mapValues)
    {
        // not used
    }

    virtual void OnDeleteCookie(bool bIsPresent)
    {
        if (NULL != m_pMain && NULL != m_pMain->GetEventListener())
        {
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT;

            m_pMain->GetEventListener()->OnEvent(pEvent);
        }
    }

public:
    IMPLEMENT_REFCOUNTING(CAscApplicationManager_Private);

public:
    virtual void OnLoad(CCefScriptLoader* pLoader, bool error) OVERRIDE
    {
        m_pMain->LockCS(LOCK_CS_SCRIPT);
        
        private_OnLoad(pLoader->m_sUrl, pLoader->m_sDestination);

        RELEASEOBJECT(pLoader);
        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }
    
    void Start_PrivateDownloadScript(const std::wstring& sUrl, const std::wstring& sDestination)
    {
        m_strPrivateDownloadUrl = sUrl;
        m_strPrivateDownloadPath = sDestination;
        
        NSCommon::url_correct(m_strPrivateDownloadUrl);
        
        CCefView* pMainView = NULL;
        int nMinId = 10000;
        for (std::map<int, CCefView*>::iterator i = m_mapViews.begin(); i != m_mapViews.end(); i++)
        {
            CCefView* pView = i->second;
            if (pView->GetType() == cvwtSimple)
            {
                if (nMinId > pView->GetId())
                {
                    pMainView = pView;
                    nMinId = pView->GetId();
                }
            }
        }

        // нуллом быть не может
        if (NULL != pMainView)
        {
            pMainView->StartDownload(m_strPrivateDownloadUrl);
        }
    }
    void End_PrivateDownloadScript()
    {
        m_pMain->LockCS(LOCK_CS_SCRIPT);
        private_OnLoad(m_strPrivateDownloadUrl, m_strPrivateDownloadPath);
        m_strPrivateDownloadUrl = L"";
        m_strPrivateDownloadPath = L"";
        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }

protected:
    
    void private_OnLoad(const std::wstring& sUrl, const std::wstring& sDestination)
    {
        std::map<std::wstring, std::vector<CEditorFrameId>>::iterator i = m_mapLoadedScripts.find(sDestination);
        
        if (i != m_mapLoadedScripts.end())
        {
            // другого и быть не может
            
            NSEditorApi::CAscEditorScript* pData = new NSEditorApi::CAscEditorScript();
            pData->put_Url(sUrl);
            pData->put_Destination(sDestination);
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION;
            pEvent->m_pData = pData;
            
            std::vector<CEditorFrameId>& _arr = i->second;
            for (std::vector<CEditorFrameId>::iterator it = _arr.begin(); it != _arr.end(); it++)
            {
                CCefView* pView = m_pMain->GetViewById((*it).EditorId);
                
                if (NULL != pView)
                {
                    pEvent->AddRef();
                    
                    pData->put_FrameId((*it).FrameId);
                    pView->Apply(pEvent);
                }
            }
            
            RELEASEINTERFACE(pEvent);
        }
        
        m_mapLoadedScripts.erase(i);
    }

    virtual DWORD ThreadProc()
    {
        //DWORD dwTime1 = NSTimers::GetTickCount();

        CArray<std::string> strFonts;
        std::wstring strDirectory = m_pMain->m_oSettings.fonts_cache_info_path;

        std::wstring strAllFontsJSPath = strDirectory + L"/AllFonts.js";
        std::wstring strThumbnailsFolder = strDirectory;
        std::wstring strFontsSelectionBin = strDirectory + L"/font_selection.bin";

        if (true)
        {
            NSFile::CFileBinary oFile;
            if (oFile.OpenFile(strDirectory + L"/fonts.log"))
            {
                int nSize = oFile.GetFileSize();
                char* pBuffer = new char[nSize];
                DWORD dwReaden = 0;
                oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
                oFile.CloseFile();

                int nStart = 0;
                int nCur = nStart;
                for (; nCur < nSize; ++nCur)
                {
                    if (pBuffer[nCur] == '\n')
                    {
                        int nEnd = nCur - 1;
                        if (nEnd > nStart)
                        {
                            std::string s(pBuffer + nStart, nEnd - nStart + 1);
                            strFonts.Add(s);
                        }
                        nStart = nCur + 1;
                    }
                }

                delete[] pBuffer;
            }
        }

        CApplicationFonts* oApplicationF = new CApplicationFonts();
        CArray<std::wstring> strFontsW_Cur;

        if (m_pMain->m_oSettings.use_system_fonts)
            strFontsW_Cur = oApplicationF->GetSetupFontFiles();
        
#if defined(_LINUX)
        std::wstring sHome = GetHomeDirectory();
        if (!sHome.empty())
        {
#ifdef _MAC
            NSDirectory::GetFiles2(sHome + L"/Library/Fonts", strFontsW_Cur, true);
#else
            NSDirectory::GetFiles2(sHome + L"/.fonts", strFontsW_Cur, true);
            NSDirectory::GetFiles2(sHome + L"/.local/share/fonts", strFontsW_Cur, true);
#endif
        }
#endif

        for (std::vector<std::wstring>::iterator i = m_pMain->m_oSettings.additional_fonts_folder.begin(); i != m_pMain->m_oSettings.additional_fonts_folder.end(); i++)
        {
            NSDirectory::GetFiles2(*i, strFontsW_Cur, true);
        }

        bool bIsEqual = true;
        if (strFonts.GetCount() != strFontsW_Cur.GetCount())
            bIsEqual = false;

        if (bIsEqual)
        {
            int nCount = strFonts.GetCount();
            for (int i = 0; i < nCount; ++i)
            {
                if (strFonts[i] != NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strFontsW_Cur[i].c_str(), strFontsW_Cur[i].length()))
                {
                    bIsEqual = false;
                    break;
                }
            }
        }

        if (bIsEqual)
        {
            if (!NSFile::CFileBinary::Exists(strFontsSelectionBin))
                bIsEqual = false;
        }

        if (!bIsEqual)
        {
            if (NSFile::CFileBinary::Exists(strAllFontsJSPath))
                NSFile::CFileBinary::Remove(strAllFontsJSPath);
            if (NSFile::CFileBinary::Exists(strFontsSelectionBin))
                NSFile::CFileBinary::Remove(strFontsSelectionBin);
            
            if (strFonts.GetCount() != 0)
                NSFile::CFileBinary::Remove(strDirectory + L"/fonts.log");

            NSFile::CFileBinary oFile;
            oFile.CreateFileW(strDirectory + L"/fonts.log");
            int nCount = strFontsW_Cur.GetCount();
            for (int i = 0; i < nCount; ++i)
            {
                oFile.WriteStringUTF8(strFontsW_Cur[i]);
                oFile.WriteFile((BYTE*)"\n", 1);
            }
            oFile.CloseFile();
            
            int nFlag = 0;
#if 1
            nFlag = 2;
#endif

            oApplicationF->InitializeFromArrayFiles(strFontsW_Cur, nFlag);

            NSCommon::SaveAllFontsJS(*oApplicationF, strAllFontsJSPath, strThumbnailsFolder, strFontsSelectionBin);

            //NSFile::CFileBinary::Copy(strAllFontsJSPath, m_pMain->m_oSettings.file_converter_path + L"/../editors/sdk/Common/AllFonts.js");

    #ifdef _GENERATE_FONT_MAP_
            NSCommon::DumpFontsDictionary(L"D:\\fonts_dictionary.txt");
    #endif
        }

        //DWORD dwTime2 = NSTimers::GetTickCount();

    #if 0
        FILE* f = fopen("D:\\FONTS\\speed.log", "a+");
        fprintf(f, "equal: %d, time: %d\n", bIsEqual ? 1 : 0, (int)(dwTime2 - dwTime1));
        fclose(f);
    #endif

        m_pApplicationFonts = oApplicationF;

        m_bRunThread = FALSE;

        // TODO:
        m_pApplicationFonts->InitializeFromFolder(strDirectory);

        return 0;
    }

public:

    // DOWNLOADS
    std::wstring GetPrivateDownloadUrl()
    {
        return m_strPrivateDownloadUrl;
    }
    std::wstring GetPrivateDownloadPath()
    {
        std::wstring sRet = m_strPrivateDownloadPath;
#ifdef WIN32
        NSCommon::string_replace(sRet, L"/", L"\\");
#endif
        return sRet;
    }

    bool IsCanceled(const unsigned int& nId)
    {
        std::map<unsigned int, bool>::iterator iter = m_mapCancelDownloads.find(nId);
        if (iter == m_mapCancelDownloads.end())
            return false;
        return true;
    }

    void LocalFiles_Init()
    {
        Recents_Load();
        Recovers_Load();
    }

    void Recents_Load()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        m_arRecents.clear();

        std::wstring sXmlPath = m_pMain->m_oSettings.recover_path + L"/../recents.xml";
        XmlUtils::CXmlNode oNodeRecents;
        if (!oNodeRecents.FromXmlFile(sXmlPath))
            return;
        XmlUtils::CXmlNodes oNodes = oNodeRecents.GetNodes(L"file");
        int nCount = oNodes.GetCount();
        int nCountAdd = 0;

        std::map<std::wstring, bool> map_files;

        for (int i = 0; i < nCount; ++i)
        {
            XmlUtils::CXmlNode oFile;
            oNodes.GetAt(i, oFile);
            std::wstring sPath = oFile.GetAttribute(L"path");

            if (/*NSFile::CFileBinary::Exists(sPath) && */(map_files.find(sPath) == map_files.end()) && !sPath.empty())
            {
                CAscEditorFileInfo oInfo;
                oInfo.m_nId = i;
                oInfo.m_sPath = sPath;
                oInfo.m_nFileType = oFile.ReadAttributeInt(L"type");
                oInfo.m_sDate = oFile.GetAttribute(L"date");
                oInfo.m_sUrl = oFile.GetAttribute(L"url");
                m_arRecents.push_back(oInfo);

                map_files.insert(std::pair<std::wstring, bool>(sPath, true));
            }
        }

        Recents_Dump(false);
    }
    void Recents_Add(const std::wstring& sPathSrc, const int& nType, const std::wstring& sUrl = L"")
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::wstring sPath = sPathSrc;
#ifdef WIN32
        if (!CFileDownloader::IsNeedDownload(sPath))
            NSCommon::string_replace(sPath, L"/", L"\\");
#endif

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (sPath == i->m_sPath)
            {
                m_arRecents.erase(i);
                break;
            }
            if (!sUrl.empty() && (sUrl == i->m_sUrl))
            {
                m_arRecents.erase(i);
                break;
            }
        }
        CAscEditorFileInfo oInfo;
        oInfo.m_nId = 0;
        oInfo.m_sPath = sPath;
        oInfo.m_sUrl = sUrl;
        oInfo.UpdateDate();

        oInfo.m_nFileType = nType;
        m_arRecents.insert(m_arRecents.begin(), oInfo);

        int nId = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            i->m_nId = nId++;
        }

        Recents_Dump();
    }
    void Recents_Remove(const int& nId)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (nId == i->m_nId)
            {
                m_arRecents.erase(i);
                break;
            }
        }

        int nIdCurrent = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            i->m_nId = nIdCurrent++;
        }

        Recents_Dump();
    }
    void Recents_RemoveAll()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        m_arRecents.clear();

        Recents_Dump();
    }

    void Recents_Dump(bool bIsSend = true)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::wstring sXmlPath = m_pMain->m_oSettings.recover_path + L"/../recents.xml";
        if (NSFile::CFileBinary::Exists(sXmlPath))
            NSFile::CFileBinary::Remove(sXmlPath);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><recents>");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            oBuilder.WriteString(L"<file path=\"");
            oBuilder.WriteEncodeXmlString(i->m_sPath);
            oBuilder.WriteString(L"\" type=\"");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L"\" date=\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\" url=\"");
            oBuilder.WriteEncodeXmlString(i->m_sUrl);
            oBuilder.WriteString(L"\" />");
        }
        oBuilder.WriteString(L"</recents>");

        NSFile::CFileBinary::SaveToFile(sXmlPath, oBuilder.GetData(), true);

        if (bIsSend)
        {
            CCefView* pMainView = m_pMain->GetViewById(1);
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS);
            NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
            pData->put_Id(pMainView->GetId());
            pData->put_JSON(Recents_GetJSON());
            pEvent->m_pData = pData;
            pMainView->Apply(pEvent);
        }
    }
    std::wstring Recents_GetJSON()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"[");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (1 < oBuilder.GetCurSize())
                oBuilder.WriteString(L",");

            oBuilder.WriteString(L"{id:");
            oBuilder.AddInt(i->m_nId);
            oBuilder.WriteString(L",type:");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L",path:\"");
            oBuilder.WriteEncodeXmlString(i->m_sPath);
            oBuilder.WriteString(L"\",modifyed:\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\"}");
        }
        oBuilder.WriteString(L"]");
        return oBuilder.GetData();
    }

    void Recovers_Load()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        CArray<std::wstring> arDirectories = NSDirectory::GetDirectories(m_pMain->m_oSettings.recover_path);
        int nCount = arDirectories.GetCount();

        int nId = 0;

        for (int i = 0; i < nCount; ++i)
        {
            CArray<std::wstring> arDirectoriesFiles = NSDirectory::GetFiles(arDirectories[i]);
            int nCountFilesRecover = arDirectoriesFiles.GetCount();

            std::wstring sName;
            int nType = -1;
            bool bIsViewer = false;

            if (!NSFile::CFileBinary::Exists(arDirectories[i] + L"/changes/changes0.json"))
            {
                NSDirectory::DeleteDirectory(arDirectories[i]);
                continue;
            }

            for (int j = 0; j < nCountFilesRecover; ++j)
            {
                std::wstring sTmp = NSCommon::GetFileName(arDirectoriesFiles[j]);
                if (L"Editor.bin" == sTmp)
                {
                    NSFile::CFileBinary oFile;
                    if (oFile.OpenFile(arDirectoriesFiles[j]) && 4 < oFile.GetFileSize())
                    {
                        BYTE _buffer[4];
                        DWORD dwRead = 0;
                        oFile.ReadFile(_buffer, 4, dwRead);

                        if (4 == dwRead)
                        {
                            if ('X' == _buffer[0] && 'L' == _buffer[1] && 'S' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
                            else if ('P' == _buffer[0] && 'P' == _buffer[1] && 'T' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
                            else if ('D' == _buffer[0] && 'O' == _buffer[1] && 'C' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                            else
                            {
                                nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                                bIsViewer = true;
                            }
                        }
                        oFile.CloseFile();
                    }
                }
            }

            std::wstring sNameInfo = arDirectories[i] + L"/asc_name.info";
            if (!NSFile::CFileBinary::Exists(sNameInfo) || nType == -1)
                continue;

            XmlUtils::CXmlNode oNode;
            if (oNode.FromXmlFile(sNameInfo))
            {
                CAscEditorFileInfo oInfo;
                oInfo.m_nFileType = oNode.ReadAttributeInt(L"type");
                oInfo.m_nId = nId++;
                oInfo.m_sPath = arDirectories[i] + L"/" + oNode.GetAttribute(L"name");
                oInfo.m_bIsViewer = bIsViewer;
                m_arRecovers.push_back(oInfo);
            }
        }
    }
    void Recovers_Remove(const int& nId, bool bIsAttack = false)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            if (nId == i->m_nId)
            {
                if (bIsAttack)
                {
                    NSDirectory::DeleteDirectory(NSCommon::GetDirectoryName(i->m_sPath));
                }

                m_arRecovers.erase(i);                
                break;
            }
        }

        int nIdCurrent = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            i->m_nId = nIdCurrent++;
        }

        Recovers_Dump();
    }
    void Recovers_RemoveAll()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            NSDirectory::DeleteDirectory(NSCommon::GetDirectoryName(i->m_sPath));
        }

        m_arRecovers.clear();
        Recovers_Dump();
    }

    void Recovers_Dump()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"[");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            if (1 < oBuilder.GetCurSize())
                oBuilder.WriteString(L",");

            oBuilder.WriteString(L"{id:");
            oBuilder.AddInt(i->m_nId);
            oBuilder.WriteString(L",type:");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L",path:\"");
            std::wstring sPath = i->m_sPath;
#ifdef WIN32
            NSCommon::string_replace(sPath, L"/", L"\\");
#endif
            oBuilder.WriteEncodeXmlString(sPath);
            oBuilder.WriteString(L"\",modifyed:\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\"}");
        }
        oBuilder.WriteString(L"]");
        std::wstring sJSON = oBuilder.GetData();

        CCefView* pMainView = m_pMain->GetViewById(1);
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS);
        NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
        pData->put_Id(pMainView->GetId());
        pData->put_JSON(sJSON);
        pEvent->m_pData = pData;
        pMainView->Apply(pEvent);
    }
};

#endif // APPLICATION_MANAGER_PRIVATE_H
