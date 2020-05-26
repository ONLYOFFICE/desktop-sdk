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

#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "./cloud_crypto.h"

#define ASC_CONSTANT_CANCEL_SAVE 0x00005678
#define LOCK_CS_SCRIPT 0

#include "./additional/manager.h"
#include "./additional/renderer.h"

#include "crypto_mode.h"
#include "plugins.h"

#include "utils.h"

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

#define NO_CACHE_WEB_CLOUD_SCRIPTS

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

class CAscReporterData
{
public:
    int Id;
    int ParentId;
    std::wstring Url;
    std::wstring LocalRecoverFolder;

    std::string CloudCryptoInfo;

public:

    CAscReporterData()
    {
        Id = -1;
        ParentId = -1;
    }
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
            CFileDownloader oDownloader(m_sUrl, false);
            oDownloader.SetFilePath(m_sDestination);

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
    int_64_type FrameId;
    std::wstring Url;
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
    std::wstring    m_sExternalCloudId;
    std::wstring    m_sParentUrl;

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

class CExternalCloudRegister
{
public:
    std::wstring id;
    std::wstring name;
    std::wstring test_editor;
    std::string correct_code;
    bool crypto_support;

public:
    CExternalCloudRegister()
    {
        id = L"";
        name = L"";
        test_editor = L"";
        correct_code = "";
        crypto_support = false;
    }

    CExternalCloudRegister(const CExternalCloudRegister& src)
    {
        id = src.id;
        name = src.name;
        test_editor = src.test_editor;
        correct_code = src.correct_code;
        crypto_support = src.crypto_support;
    }

    CExternalCloudRegister& operator=(const CExternalCloudRegister& src)
    {
        id = src.id;
        name = src.name;
        test_editor = src.test_editor;
        correct_code = src.correct_code;
        crypto_support = src.crypto_support;
        return *this;
    }
};

namespace NSSystem
{
#ifndef _WIN32
#include <fcntl.h>
#endif

    class CLocalFileLocker
    {
    private:
        std::wstring m_sFile;
#ifdef _WIN32
        NSFile::CFileBinary m_oLocker;
#else
        int m_nDescriptor;
#endif

    public:
        CLocalFileLocker(const std::wstring& sFile)
        {
            m_sFile = sFile;
#ifndef _WIN32
            m_nDescriptor = -1;
#endif

            Lock();
        }
        bool Lock()
        {
            Unlock();
#ifdef _WIN32
            m_oLocker.OpenFile(m_sFile);
#else
            std::string sFileA = U_TO_UTF8(m_sFile);
            m_nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
            if (-1 == m_nDescriptor)
                return true;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            _lock.l_type   = F_WRLCK;
            _lock.l_whence = SEEK_SET;
            _lock.l_start  = 0;
            _lock.l_len    = 0;
            _lock.l_pid    = getpid();

            fcntl(m_nDescriptor, F_SETLKW, &_lock);
#endif
            return true;
        }
        bool Unlock()
        {
#ifdef _WIN32
            m_oLocker.CloseFile();
#else
            if (-1 == m_nDescriptor)
                return true;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            _lock.l_type   = F_UNLCK;
            _lock.l_whence = SEEK_SET;
            _lock.l_start  = 0;
            _lock.l_len    = 0;
            _lock.l_pid    = getpid();

            fcntl(m_nDescriptor, F_SETLKW, &_lock);
            close(m_nDescriptor);
#endif
            return true;
        }
        ~CLocalFileLocker()
        {
            Unlock();
        }

        static bool IsLocked(const::std::wstring& sFile)
        {
            bool isLocked = false;
#ifdef _WIN32
            HANDLE hFile = CreateFileW(sFile.c_str(),                   // открываемый файл
                                        GENERIC_READ | GENERIC_WRITE,   // открываем для чтения и записи
                                        0,                              // для совместного чтения
                                        NULL,                           // защита по умолчанию
                                        OPEN_EXISTING,                  // только существующий файл
                                        FILE_ATTRIBUTE_NORMAL,          // обычный файл
                                        NULL);                          // атрибутов шаблона нет
            if (hFile == INVALID_HANDLE_VALUE)
            {
                if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(sFile.c_str()))
                    isLocked = true;
            }
            CloseHandle(hFile);
#else
            std::string sFileA = U_TO_UTF8(sFile);
            int nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
            if (-1 == nDescriptor)
                return false;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            fcntl(nDescriptor, F_GETLK, &_lock);
            if (F_WRLCK == (_lock.l_type & F_WRLCK))
                isLocked = true;
            close(nDescriptor);
#endif
            return isLocked;
        }
    };
}

class CRecentParent
{
public:
    std::wstring Url;
    std::wstring Parent;
};

namespace NSArgumentList
{
    static int64 GetInt64(CefRefPtr<CefListValue> args, const int& index)
    {
        std::string tmp = args->GetString(index).ToString();
        return (int64)std::stoll(tmp);
    }
    static bool SetInt64(CefRefPtr<CefListValue> args, const int& index, const int64& value)
    {
        std::string tmp = std::to_string(value);
        return args->SetString(index, tmp);
    }
}

class CAscApplicationManager_Private : public CefBase_Class,
        public CCookieFoundCallback,
        public NSThreads::CBaseThread,
        public CCefScriptLoader::ICefScriptLoaderCallback,
        public NSAscCrypto::IAscKeyChainListener
{
public:
    CAscSpellChecker m_oSpellChecker;

    CAscKeyboardChecker m_oKeyboardChecker;
    CTimerKeyboardChecker m_oKeyboardTimer;

    // счетчик всех view
    int m_nIdCounter;

    // счетчик всех view
    int m_nWindowCounter;

    // id <=> view
    std::map<int, CCefView*> m_mapViews;

    // показывать ли консоль для дебага
    bool m_bDebugInfoSupport;

    // использовать ли внешнюю очередь сообщений
    bool m_bIsUseExternalMessageLoop;
    IExternalMessageLoop* m_pExternalMessageLoop;

    // event listener
    NSEditorApi::CAscCefMenuEventListener* m_pListener;

    // application fonts for all editors
    NSFonts::IApplicationFonts* m_pApplicationFonts;

    // используется для загрузки скриптов
    // url <-> все те, кто ждет загрузку этого скрипта. по окончании загрузки - всем отсылается евент
    NSCriticalSection::CRITICAL_SECTION m_oCS_Scripts;
    std::map<std::wstring, std::vector<CEditorFrameId>> m_mapLoadedScripts;

    // id для вью, который вызвал FileDialog
    int m_nIsCefSaveDialogWait;
    
    // внутренние скачки (неюзерские)
    std::wstring m_strPrivateDownloadUrl;
    std::wstring m_strPrivateDownloadPath;
    
    // мап отмененных загрузок
    std::map<unsigned int, bool> m_mapCancelDownloads;

    // Recents & recovers
    NSCriticalSection::CRITICAL_SECTION m_oCS_LocalFiles;
    std::vector<CAscEditorFileInfo> m_arRecents;
    std::vector<CAscEditorFileInfo> m_arRecovers;
    
    // дополнения к ссылкам
    std::wstring m_sAdditionalUrlParams;    

    // сообщения, которые отправятся в view после инициализации js контекста
    std::vector<NSEditorApi::CAscMenuEvent*> m_arApplyEvents;

    // настройки приложения
    std::map<std::string, std::string> m_mapSettings;

    // если ！= -1, то используется для scale всех view
    int m_nForceDisplayScale;

    // флаг для принудительной перегенерации шрифтов (используется при изменении настроек, какие шрифты использовать)
    bool m_bIsUpdateFontsAttack;

    // используется только для Linux snap.
    std::string m_sLD_LIBRARY_PATH;

    // crypto
    std::map<std::wstring, int> m_mapOnlyPass;
    bool m_bCryptoDisableForLocal; // поддерживает ли плагин криптования шифрование локальных файлов
    bool m_bCryptoDisableForInternalCloud; // поддерживает ли плагин криптования шифрование файлов из нашего облака
    bool m_bCryptoDisableForExternalCloud; // поддерживает ли плагин криптования шифрование файлов из чужого облака

    std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue> m_mapCrypto;
    NSAscCrypto::AscCryptoType m_nCurrentCryptoMode;
    NSAscCrypto::CCryptoKey m_cryptoKeyEnc;
    NSAscCrypto::CCryptoKey m_cryptoKeyDec;

    NSAscCrypto::CAscKeychain* m_pKeyChain;

    // плагины не для редактора, а для десктопа (на стартовой странице)
    std::vector<CExternalPluginInfo> m_arExternalPlugins;

    // те, кто подключает onlyoffice
    std::vector<CExternalCloudRegister> m_arExternalClouds;

    // критическая секция для всех системных сообщений всех view
    NSCriticalSection::CRITICAL_SECTION m_oCS_SystemMessages;

    // настройки для ссылок на редакторы
    std::wstring m_mainPostFix;
    std::wstring m_mainLang;

    // ссылки, откуда файл открыт. (для кнопок 'домой' в редакторах)
    std::vector<CRecentParent> m_arRecentParents;

    // dpi checker
    static CAscDpiChecker* m_pDpiChecker;

    // ссылки на нужные классы
    CAscApplicationManager* m_pMain;
    CApplicationCEF* m_pApplication;

    // дополнения к редактору (для внешних подключений)
    CApplicationManagerAdditionalBase* m_pAdditional;

    // логи конвертера
    bool m_bIsEnableConvertLogs;

public:
    IMPLEMENT_REFCOUNTING(CAscApplicationManager_Private);

public:
    CAscApplicationManager_Private() : m_oKeyboardTimer(this)
    {
        this->AddRef();

        m_pListener = NULL;
        m_nIdCounter = 0;
        m_nWindowCounter = 0;

        m_pMain = NULL;
        m_pApplicationFonts = NULL;
        m_pApplication = NULL;

        m_bDebugInfoSupport = false;
        m_bIsUseExternalMessageLoop = false;
        m_pExternalMessageLoop = NULL;

        m_nIsCefSaveDialogWait = -1;        

        m_sAdditionalUrlParams = L"";
        m_pAdditional = NULL;

        m_nForceDisplayScale = -1;

        m_bIsUpdateFontsAttack = false;

        m_nCurrentCryptoMode = NSAscCrypto::None;

        m_pKeyChain = NULL;

        m_bIsEnableConvertLogs = false;

        m_bCryptoDisableForLocal = false;
        m_bCryptoDisableForInternalCloud = false;
        m_bCryptoDisableForExternalCloud = false;

        m_oCS_Scripts.InitializeCriticalSection();
        m_oCS_LocalFiles.InitializeCriticalSection();
        m_oCS_SystemMessages.InitializeCriticalSection();
    }
    virtual ~CAscApplicationManager_Private()
    {
        RELEASEOBJECT(m_pExternalMessageLoop);

        CloseApplication();
        RELEASEOBJECT(m_pAdditional);

        m_oCS_Scripts.DeleteCriticalSection();
        m_oCS_LocalFiles.DeleteCriticalSection();
        m_oCS_SystemMessages.DeleteCriticalSection();
    }

    bool GetEditorPermission()
    {
        // разрешение на редактирование
        return m_pAdditional ? m_pAdditional->GetEditorPermission() : true;
    }

    // исполнить код во всех view
    void ExecuteInAllFrames(const std::string& sCode);

    // вызывается, если меняется количество открытых редакторов
    void ChangeEditorViewsCount();

    void CloseApplication()
    {
        if (NSFile::CFileBinary::Exists(m_pMain->m_oSettings.user_plugins_path + L"/cloud_crypto_tmp.xml"))
            NSFile::CFileBinary::Remove(m_pMain->m_oSettings.user_plugins_path + L"/cloud_crypto_tmp.xml");

        Stop();
        m_oKeyboardTimer.Stop();        
        m_oSpellChecker.End();
    }    

    // logout из портала -----------------------------------------------------------------------
    void Logout(std::wstring strUrl, CefRefPtr<CefCookieManager> manager)
    {
        CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
        pVisitor->m_pCallback       = this;
        pVisitor->m_bIsDelete       = true;
        pVisitor->m_sDomain         = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strUrl.c_str(), strUrl.length());

        if (0 == pVisitor->m_sDomain.find("https://"))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(8);
        else if (0 == pVisitor->m_sDomain.find("http://"))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(7);
        
        std::string::size_type pos = pVisitor->m_sDomain.find("?");
        if (pos != std::string::npos)
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);
        pos = pVisitor->m_sDomain.rfind("/");
        if ((pos != std::string::npos) && ((pos + 1) == pVisitor->m_sDomain.length()))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);

        if (true)
        {
            pos = NSCommon::FindLowerCaseR(pVisitor->m_sDomain, "/products/files");
            if ((pos != std::string::npos) && ((pos + 15) == pVisitor->m_sDomain.length()))
                pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);
        }
        
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
            NSEditorApi::CAscCefMenuEvent* pEvent = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT);
            m_pMain->GetEventListener()->OnEvent(pEvent);
        }
    }

    // работа с настройками редактора ----------------------------------------------------------
    void LoadSettings()
    {
        std::wstring sFile = m_pMain->m_oSettings.fonts_cache_info_path + L"/settings.xml";
        XmlUtils::CXmlNode oNode;
        if (!oNode.FromXmlFile(sFile))
            return;

        XmlUtils::CXmlNodes oNodes;
        if (!oNode.GetChilds(oNodes))
            return;

        int nCount = oNodes.GetCount();
        for (int i = 0; i < nCount; ++i)
        {
            XmlUtils::CXmlNode oSetting;
            oNodes.GetAt(i, oSetting);

            std::wstring nameW = oSetting.GetName();
            std::string name = U_TO_UTF8(nameW);
            std::wstring valueW = oSetting.GetText();
            std::string value = U_TO_UTF8(valueW);

            m_mapSettings.insert(std::pair<std::string, std::string>(name, value));
        }
    }
    void SaveSettings()
    {
        std::wstring sFile = m_pMain->m_oSettings.fonts_cache_info_path + L"/settings.xml";
        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<Settings>");

        for (std::map<std::string, std::string>::iterator pair = m_mapSettings.begin(); pair != m_mapSettings.end(); pair++)
        {
            std::string name = pair->first;
            std::string value = pair->second;
            std::wstring nameW = UTF8_TO_U(name);
            std::wstring valueW = UTF8_TO_U(value);

            oBuilder.AddCharSafe('<');
            oBuilder.WriteString(nameW);
            oBuilder.AddCharSafe('>');
            oBuilder.WriteEncodeXmlString(valueW);
            oBuilder.AddCharSafe('<');
            oBuilder.AddCharSafe('/');
            oBuilder.WriteString(nameW);
            oBuilder.AddCharSafe('>');
        }

        oBuilder.WriteString(L"</Settings>");

        NSFile::CFileBinary::SaveToFile(sFile, oBuilder.GetData());

        // after - check settings
        std::map<std::string, std::string>::iterator pairForceDisplayScale = m_mapSettings.find("force-scale");
        if (pairForceDisplayScale != m_mapSettings.end())
            m_nForceDisplayScale = std::stoi(pairForceDisplayScale->second);

        std::map<std::string, std::string>::iterator pairCryptoMode = m_mapSettings.find("crypto-mode");
        if (pairCryptoMode != m_mapSettings.end())
            m_nCurrentCryptoMode = (NSAscCrypto::AscCryptoType)std::stoi(pairCryptoMode->second);

        std::map<std::string, std::string>::iterator pairConvertLogs = m_mapSettings.find("converter-logging");
        if (pairConvertLogs != m_mapSettings.end())
            m_bIsEnableConvertLogs = ("1" == pairConvertLogs->second) ? true : false;

        std::map<std::string, std::string>::iterator pairEML = m_mapSettings.find("external-message-loop");
        if (pairEML != m_mapSettings.end())
            m_bIsUseExternalMessageLoop = ("1" == pairEML->second) ? true : false;
    }
    void CheckSetting(const std::string& sName, const std::string& sValue)
    {
        if ("--ascdesktop-support-debug-info" == sName)
        {
            m_bDebugInfoSupport = true;
            return;
        }        

        bool bIsChanged = false;
        const char* namePtr = sName.c_str();

        while (*namePtr != '\0')
        {
            char c = *namePtr++;
            if ('&' == c || '\'' == c || '<' == c || '>' == c || '\"' == c)
                return;
        }

        if (0 != sName.find("--"))
            return;

        std::string name = sName.substr(2);
        std::map<std::string, std::string>::iterator pair = m_mapSettings.find(name);
        if (pair != m_mapSettings.end())
        {
            if (sValue != "default")
            {
                if (pair->second != sValue)
                    bIsChanged = true;

                pair->second = sValue;
            }
            else
            {
                m_mapSettings.erase(pair);
                bIsChanged = true;
            }
        }
        else
        {
            if (sValue != "default")
            {
                m_mapSettings.insert(std::pair<std::string, std::string>(name, sValue));
                bIsChanged = true;
            }
        }

        if ("--use-opentype-fonts" == sName && bIsChanged)
        {
            m_bIsUpdateFontsAttack = true;
        }
    }

    // вспомогательные функции
    CCefView* GetViewWithMinId()
    {
        CCefView* pMinView = NULL;
        int nMin = 0xFFFF;
        for (std::map<int, CCefView*>::iterator i = m_mapViews.begin(); i != m_mapViews.end(); i++)
        {
            CCefView* pView = i->second;
            if (pView->GetType() == cvwtSimple && pView->GetId() < nMin)
            {
                nMin = pView->GetId();
                pMinView = pView;
            }
        }
        return pMinView;
    }
    CCefView* GetViewForSystemMessages()
    {
        return GetViewWithMinId();
    }
    bool TestExternal(const std::wstring& sId, CExternalCloudRegister& ex)
    {
        for (std::vector<CExternalCloudRegister>::iterator iter = m_arExternalClouds.begin(); iter != m_arExternalClouds.end(); iter++)
        {
            if (sId == iter->id)
            {
                ex = *iter;
                return true;
            }
        }
        return false;
    }
    void SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* pEvent)
    {
        for (std::map<int, CCefView*>::iterator i = m_mapViews.begin(); i != m_mapViews.end(); i++)
        {
           CCefView* pView = i->second;
           if (pView->GetType() == cvwtSimple)
           {
               pEvent->AddRef();
               pView->Apply(pEvent);
           }
        }

        pEvent->Release();
    }

    // загрузка скриптов ----------------------------------------------------------------------
    virtual void OnLoad(CCefScriptLoader* pLoader, bool error) OVERRIDE
    {
        // коллбэк на загрузку скрипта

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
        
        CCefView* pMainView = GetViewWithMinId();
        if (NULL != pMainView)
            pMainView->StartDownload(m_strPrivateDownloadUrl);
    }
    void End_PrivateDownloadScript()
    {
        private_OnLoad(m_strPrivateDownloadUrl, m_strPrivateDownloadPath);

        m_pMain->LockCS(LOCK_CS_SCRIPT);

        m_strPrivateDownloadUrl = L"";
        m_strPrivateDownloadPath = L"";

        if (!m_mapLoadedScripts.empty())
        {
            for (std::map<std::wstring, std::vector<CEditorFrameId>>::iterator i = m_mapLoadedScripts.begin(); i != m_mapLoadedScripts.end(); i++)
            {
                std::wstring _url = i->second[0].Url;
                std::wstring _dst = i->first;

                if (std::wstring::npos == _url.find(L"sdk/Common/AllFonts.js") &&
                    std::wstring::npos == _url.find(L"sdkjs/common/AllFonts.js"))
                {
                    Start_PrivateDownloadScript(_url, _dst);
                    break;
                }
            }
        }

        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }
    void private_OnLoad(const std::wstring& sUrl, const std::wstring& sDestination)
    {
        m_pMain->LockCS(LOCK_CS_SCRIPT);

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

        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }

    // отслеживаем шрифты ---------------------------------------------------------------------
    virtual DWORD ThreadProc()
    {
        CApplicationFontsWorker oWorker;
        oWorker.m_sDirectory = m_pMain->m_oSettings.fonts_cache_info_path;
        oWorker.m_bIsUseSystemFonts = m_pMain->m_oSettings.use_system_fonts;
        oWorker.m_arAdditionalFolders = m_pMain->m_oSettings.additional_fonts_folder;

#if defined(_LINUX)
        std::wstring sHome = GetHomeDirectory();
        if (!sHome.empty())
        {
#ifdef _MAC
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/Library/Fonts");
#else
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/.fonts");
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/.local/share/fonts");
#endif
        }
#endif

        oWorker.m_bIsUseOpenType = true;
        oWorker.m_bIsNeedThumbnails = true;

        m_pApplicationFonts = oWorker.Check();

        m_bRunThread = FALSE;
        return 0;
    }

    // работа со скачиванием файлов -----------------------------------------------------------
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

    // работа с Recents & Recovers ------------------------------------------------------------
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
                oInfo.m_sExternalCloudId = oFile.GetAttribute(L"externalcloud");
                oInfo.m_sParentUrl = oFile.GetAttribute(L"parent");
                m_arRecents.push_back(oInfo);

                map_files.insert(std::pair<std::wstring, bool>(sPath, true));
            }
        }

        Recents_Dump(false);
    }
    void Recents_Add(const std::wstring& sPathSrc, const int& nType, const std::wstring& sUrl = L"", const std::wstring& sExternalCloudId = L"", const std::wstring& sParentUrl = L"")
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::wstring sPath = sPathSrc;
#ifdef WIN32
        if (!NSFileDownloader::IsNeedDownload(sPath))
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
        oInfo.m_sExternalCloudId = sExternalCloudId;
        oInfo.m_sParentUrl = sParentUrl;
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

            if (!i->m_sExternalCloudId.empty())
            {
                oBuilder.WriteString(L"\" externalcloud=\"");
                oBuilder.WriteEncodeXmlString(i->m_sExternalCloudId);
            }

            if (i->m_sParentUrl.empty())
            {
                if (NSFile::CFileBinary::Exists(i->m_sPath))
                {
                    oBuilder.WriteString(L"\" parent=\"");
                    oBuilder.WriteEncodeXmlString(NSFile::GetDirectoryName(i->m_sPath));
                }
                else
                {
                    oBuilder.WriteString(L"\" parent=\"");
                }
            }
            else
            {
                oBuilder.WriteString(L"\" parent=\"");
                oBuilder.WriteEncodeXmlString(i->m_sParentUrl);
            }

            oBuilder.WriteString(L"\" />");
        }
        oBuilder.WriteString(L"</recents>");

        NSFile::CFileBinary::SaveToFile(sXmlPath, oBuilder.GetData(), true);

        if (bIsSend)
        {
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS);
            NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
            pData->put_JSON(Recents_GetJSON());
            pEvent->m_pData = pData;

            SetEventToAllMainWindows(pEvent);
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
            oBuilder.WriteString(L"\",parent:\"");
            if (i->m_sParentUrl.empty())
            {
                if (NSFile::CFileBinary::Exists(i->m_sPath))
                {
                    oBuilder.WriteEncodeXmlString(NSFile::GetDirectoryName(i->m_sPath));
                }
                else
                {
                    oBuilder.WriteEncodeXmlString(L"");
                }
            }
            else
            {
                oBuilder.WriteEncodeXmlString(i->m_sParentUrl);
            }
            oBuilder.WriteString(L"\"}");
        }
        oBuilder.WriteString(L"]");
        return oBuilder.GetData();
    }

    void Recovers_Load()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::vector<std::wstring> arDirectories = NSDirectory::GetDirectories(m_pMain->m_oSettings.recover_path);
        int nCount = (int)arDirectories.size();

        int nId = 0;

        for (int i = 0; i < nCount; ++i)
        {
            std::vector<std::wstring> arDirectoriesFiles = NSDirectory::GetFiles(arDirectories[i]);
            int nCountFilesRecover = (int)arDirectoriesFiles.size();

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

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS);
        NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
        pData->put_JSON(sJSON);
        pEvent->m_pData = pData;

        SetEventToAllMainWindows(pEvent);
    }

    // crypto ---------------------------------------------------------------------------------
    void LoadCryptoData()
    {
        m_pKeyChain = m_pMain->GetKeychainEngine();
        m_pKeyChain->Check(m_pMain->m_oSettings.cookie_path + L"/user.data");
    }
    void SendCryptoData(CefRefPtr<CefFrame> frame = NULL)
    {
        std::wstring sPass = L"";
        if (0 != m_nCurrentCryptoMode)
        {
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_mapCrypto.find(m_nCurrentCryptoMode);
            if (find != m_mapCrypto.end())
                sPass = UTF8_TO_U(find->second.m_sValue);
        }

        NSCommon::string_replace(sPass, L"\\", L"\\\\");
        NSCommon::string_replace(sPass, L"\"", L"\\\"");

        std::wstring sCode = L"(function() { \n\
window.AscDesktopEditor.CryptoMode = " + std::to_wstring(m_nCurrentCryptoMode) + L";\n\
window.AscDesktopEditor.CryptoPassword = \"" + sPass + L"\";\n\
})();";

        if (frame)
        {
            frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
            return;
        }

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_EXECUTE_JS_CODE);
        NSEditorApi::CAscJSMessage* pData = new NSEditorApi::CAscJSMessage();
        pData->put_Value(sCode);
        pEvent->m_pData = pData;

        for (std::map<int, CCefView*>::iterator iterView = m_mapViews.begin(); iterView != m_mapViews.end(); iterView++)
        {
            CCefView* pTmp = iterView->second;

            ADDREFINTERFACE(pEvent);
            pTmp->Apply(pEvent);
        }

        RELEASEINTERFACE(pEvent);
    }
    virtual void OnKeyChainComplete(NSAscCrypto::CCryptoKey& keyEnc, NSAscCrypto::CCryptoKey& keyDec)
    {
        m_cryptoKeyEnc = keyEnc;
        m_cryptoKeyDec = keyDec;

        NSAscCrypto::CCryptoMode oCryptoMode;
        oCryptoMode.Load(m_cryptoKeyEnc, m_cryptoKeyDec, m_pMain->m_oSettings.cookie_path + L"/user.data");

        for (std::vector<NSAscCrypto::CAscCryptoJsonValue>::iterator iter = oCryptoMode.m_modes.begin(); iter != oCryptoMode.m_modes.end(); iter++)
        {
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_mapCrypto.find(iter->m_eType);
            if (find == m_mapCrypto.end())
            {
                m_mapCrypto.insert(std::pair<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>(iter->m_eType, *iter));
            }
            else
            {
                find->second.m_sValue = iter->m_sValue;
            }
        }

        // создаем ключи для режимов
        bool bIsResave = false;

        // 1) simple
        std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator findSimple = m_mapCrypto.find(NSAscCrypto::Simple);
        if (findSimple != m_mapCrypto.end())
        {
            NSAscCrypto::CAscCryptoJsonValue& simple = findSimple->second;
            if (simple.m_sValue.empty())
            {
                std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                char data[1025];
                int alphanumlen = alphanum.length();
                srand((unsigned int)NSTimers::GetTickCount());
                for (int i = 0; i < 1024; ++i)
                {
                    data[i] = (char)alphanum[rand() % (alphanumlen - 1)];
                }
                data[1024] = 0;
                simple.m_sValue = std::string(data);
                bIsResave = true;
            }
        }

        if (bIsResave)
        {
            m_pMain->SetCryptoMode("", m_nCurrentCryptoMode);
        }

        RELEASEOBJECT(m_pKeyChain);
    }
};

#endif // APPLICATION_MANAGER_PRIVATE_H
