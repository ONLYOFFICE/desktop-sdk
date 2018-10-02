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

#include "./applicationmanager_p.h"
#include "./plugins.h"

#ifdef LINUX
CApplicationCEF* CLinuxData::app_cef            = NULL;
CAscApplicationManager* CLinuxData::app_manager = NULL;

void posix_death_signal(int signum)
{
    // release all subprocesses
    CLinuxData::Close();
    signal(signum, SIG_DFL);
    exit(3);
}
#endif

CAscApplicationSettings::CAscApplicationSettings()
{
    std::wstring sApplicationPath   = NSFile::GetProcessDirectory();
    NSCommon::string_replace(sApplicationPath, L"\\", L"/");

    app_data_path                   = sApplicationPath;

    spell_dictionaries_path         = sApplicationPath + L"/Dictionaries";
    cache_path                      = sApplicationPath + L"/data/cache";
    cookie_path                     = sApplicationPath + L"/data";

    use_system_fonts                = true;
    fonts_cache_info_path           = app_data_path + L"/data/fonts";

    system_plugins_path             = app_data_path + L"/editors/sdkjs-plugins";
    user_plugins_path               = app_data_path + L"/editors/sdkjs-plugins-user";

    local_editors_path              = sApplicationPath + L"/editors/web-apps/apps/api/documents/index.html";
    file_converter_path             = sApplicationPath + L"/converter";
    recover_path                    = app_data_path + L"/data/recover";

    country                         = "RU";

    sign_support                    = true;
    pass_support                    = true;
    protect_support                 = true;

    converter_application_name      = "ONLYOFFICE";
    converter_application_company   = "Ascensio System SIA Copyright (c) 2018";
}

void CAscApplicationSettings::SetUserDataPath(std::wstring sPath)
{
    app_data_path                   = sPath;
    NSCommon::string_replace(app_data_path, L"\\", L"/");

    cache_path                      = app_data_path + L"/data/cache";
    cookie_path                     = app_data_path + L"/data";

    use_system_fonts                = true;
    fonts_cache_info_path           = app_data_path + L"/data/fonts";
    recover_path                    = app_data_path + L"/data/recover";

    user_plugins_path               = app_data_path + L"/data/sdkjs-plugins";
}

void CTimerKeyboardChecker::OnTimer()
{
    if (m_pManager->m_pMain)
        m_pManager->m_pMain->OnNeedCheckKeyboard();
}

CAscApplicationManager::CAscApplicationManager()
{
    m_pInternal = new CAscApplicationManager_Private();
    m_pInternal->m_pMain = this;
    m_pInternal->m_pAdditional = Create_ApplicationManagerAdditional(this);
    m_pInternal->m_pAdditional->m_arApplyEvents = &m_pInternal->m_arApplyEvents;
}

CAscApplicationManager::~CAscApplicationManager()
{
    m_pInternal->Release();
}

void CAscApplicationManager::StartSpellChecker()
{
    m_pInternal->m_oSpellChecker.SetApplicationManager(this);
    m_pInternal->m_oSpellChecker.Init(m_oSettings.spell_dictionaries_path);
    m_pInternal->m_oSpellChecker.Start();
}

void CAscApplicationManager::StopSpellChecker()
{
    m_pInternal->m_oSpellChecker.End();
}

void CAscApplicationManager::SpellCheck(const int& nEditorId, const std::string& sTask, int nId)
{
    m_pInternal->m_oSpellChecker.AddTask(nEditorId, sTask, nId);
}

void CAscApplicationManager::StartKeyboardChecker()
{
    m_pInternal->m_oKeyboardTimer.Start(0);
}
void CAscApplicationManager::OnNeedCheckKeyboard()
{
    // none. evaluate in UI thread
}

void CAscApplicationManager::CheckKeyboard()
{
    m_pInternal->m_oKeyboardChecker.Check(this, -1);
}

void CAscApplicationManager::SendKeyboardAttack()
{
    m_pInternal->m_oKeyboardChecker.Send(this);
}

void CAscApplicationManager::CheckFonts(bool bAsync)
{
    m_pInternal->LocalFiles_Init();

    if (!NSDirectory::Exists(m_oSettings.fonts_cache_info_path))
    {
#ifdef WIN32
        std::wstring sTmpDir = m_oSettings.fonts_cache_info_path;
        NSCommon::string_replace(sTmpDir, L"/", L"\\");
        NSDirectory::CreateDirectories(sTmpDir);
#else
        NSDirectory::CreateDirectories(m_oSettings.fonts_cache_info_path);
#endif
    }

    bool bIsStarted = m_pInternal->IsRunned();
    bool bIsInit = IsInitFonts();

    if (bIsInit)
        return;

    if (bIsStarted)
    {
        if (!bAsync)
        {
            while (!IsInitFonts())
            {
                NSThreads::Sleep(10);
            }
        }
        return;
    }

    m_pInternal->Start(0);
    if (!bAsync)
    {
        while (!IsInitFonts())
        {
            NSThreads::Sleep(10);
        }
    }
}

void CAscApplicationManager::SetEventListener(NSEditorApi::CAscCefMenuEventListener* pListener)
{
    m_pInternal->m_pListener = pListener;
}

NSEditorApi::CAscCefMenuEventListener* CAscApplicationManager::GetEventListener()
{
    return m_pInternal->m_pListener;
}

void CAscApplicationManager::Apply(NSEditorApi::CAscMenuEvent* pEvent)
{
    if (NULL == pEvent)
        return;

    switch (pEvent->m_nType)
    {
        case ASC_MENU_EVENT_TYPE_CEF_KEYBOARDLAYOUT:
        {
            for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); i++)
            {
                ADDREFINTERFACE(pEvent);
                i->second->Apply(pEvent);
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK:
        {
            NSEditorApi::CAscSpellCheckType* pData = (NSEditorApi::CAscSpellCheckType*)pEvent->m_pData;

            std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.find(pData->get_EditorId());
            if (i != m_pInternal->m_mapViews.end())
            {
                ADDREFINTERFACE(pEvent);
                i->second->Apply(pEvent);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD:
        {
            NSEditorApi::CAscDownloadFileInfo* pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;

            if (pData->get_IsComplete())
            {
                int nId = pData->get_Id();
                CCefView* pCefView = this->GetViewById(nId);

                if (-1 != pCefView->GetParentCef())
                    this->DestroyCefView(pData->get_Id());
            }
            
            bool bIsPrivate = false;
            std::wstring s1 = pData->get_Url();
            NSCommon::string_replace(s1, L"/./", L"/");
            if (s1 == m_pInternal->m_strPrivateDownloadUrl)
                bIsPrivate = true;
            
            if (pData->get_IsComplete() && bIsPrivate)
            {
                m_pInternal->End_PrivateDownloadScript();
            }

            if (m_pInternal->m_pListener && !bIsPrivate)
            {
                NSEditorApi::CAscCefMenuEvent* pCefEvent = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD);
                pCefEvent->put_SenderId(pData->get_Id());
                ADDREFINTERFACE(pData);
                pCefEvent->m_pData = pData;
                m_pInternal->m_pListener->OnEvent(pCefEvent);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION:
        {
            NSEditorApi::CAscEditorScript* pData = (NSEditorApi::CAscEditorScript*)pEvent->m_pData;

            this->LockCS(LOCK_CS_SCRIPT);

            // смотрим, зажат ли этот файл
            std::map<std::wstring, std::vector<CEditorFrameId>>::iterator _find = m_pInternal->m_mapLoadedScripts.find(pData->get_Destination());
            if (_find != m_pInternal->m_mapLoadedScripts.end())
            {
                // он качается. нужно записать id в список тех, кто хочет получить скрипт после его загрузки
                CEditorFrameId _id;
                _id.Url = pData->get_Url();
                _id.EditorId = pData->get_Id();
                _id.FrameId = pData->get_FrameId();
                _find->second.push_back(_id);
            }
            else
            {
                // смотрим, есть ли файл на диске
                if (NSFile::CFileBinary::Exists(pData->get_Destination()))
                {
                    CCefView* pView = this->GetViewById(pData->get_Id());
                    if (NULL != pView)
                    {
                        pEvent->AddRef();
                        pView->Apply(pEvent);
                    }
                }
                else
                {
                    // создаем список ожидания
                    std::vector<CEditorFrameId> _arr;
                    CEditorFrameId _id;
                    _id.Url = pData->get_Url();
                    _id.EditorId = pData->get_Id();
                    _id.FrameId = pData->get_FrameId();
                    _arr.push_back(_id);

                    m_pInternal->m_mapLoadedScripts.insert(std::pair<std::wstring, std::vector<CEditorFrameId>>(pData->get_Destination(), _arr));

                    if (std::wstring::npos == pData->get_Url().find(L"sdk/Common/AllFonts.js") &&
                        std::wstring::npos == pData->get_Url().find(L"sdkjs/common/AllFonts.js"))
                    {
                        if (m_pInternal->m_strPrivateDownloadUrl.empty())
                            m_pInternal->Start_PrivateDownloadScript(pData->get_Url(), pData->get_Destination());
                    }
                    else
                    {
                        CCefScriptLoader* pLoader = new CCefScriptLoader();
                        pLoader->m_pListener = this->m_pInternal;
                        pLoader->m_pManager = this;
                        pLoader->m_sUrl = pData->get_Url();
                        pLoader->m_sDestination = pData->get_Destination();
                        pLoader->Start(0);
                    }
                }
            }

            this->UnlockCS(LOCK_CS_SCRIPT);
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG:
        {
            NSEditorApi::CAscSaveDialog* pData = (NSEditorApi::CAscSaveDialog*)pEvent->m_pData;
            m_pInternal->m_nIsCefSaveDialogWait = pData->get_Id();

            this->StartSaveDialog(pData->get_FilePath(), pData->get_IdDownload());
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH:
        {
            NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;

            CCefView* pView = GetViewById(pData->get_Id());
            if (NULL != pView)
            {
                ADDREFINTERFACE(pEvent);
                pView->Apply(pEvent);
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE:
        {
            NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

            CCefView* pView = GetViewById(pData->get_Id());
            if (NULL != pView)
            {
                ADDREFINTERFACE(pEvent);
                pView->Apply(pEvent);
            }
            break;
        }        
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN:
        {
            NSEditorApi::CAscAddPlugin* pData = (NSEditorApi::CAscAddPlugin*)pEvent->m_pData;

            CPluginsManager oPlugins;
            oPlugins.m_strDirectory = m_oSettings.system_plugins_path;
            oPlugins.m_strUserDirectory = m_oSettings.user_plugins_path;
            oPlugins.AddPlugin(pData->get_Path());

            for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); ++i)
            {
                NSEditorApi::CAscMenuEvent* pSendEvent = new NSEditorApi::CAscMenuEvent();
                pSendEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
                i->second->Apply(pSendEvent);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG:
        {
            NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

            CCefView* pView = GetViewById(pData->get_Id());
            if (NULL != pView)
            {
                ADDREFINTERFACE(pEvent);
                pView->Apply(pEvent);
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE:
        {
            int nId = ((NSEditorApi::CAscCefMenuEvent*)pEvent)->get_SenderId();
            CCefView* pView = GetViewById(nId);

            if (NULL != pView)
            {
                ADDREFINTERFACE(pEvent);
                pView->Apply(pEvent);
            }

            break;
        }
        case ASC_MENU_EVENT_TYPE_ENCRYPT_PERSONAL_KEY_EXPORT:
        {
            NSAscCrypto::AscCryptoType eType = m_pInternal->m_nCurrentCryptoMode;
            if (NSAscCrypto::None == eType)
                eType = NSAscCrypto::Simple;

            NSEditorApi::CEncryptData* pData = (NSEditorApi::CEncryptData*)pEvent->m_pData;
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_pInternal->m_mapCrypto.find(eType);
            if (find != m_pInternal->m_mapCrypto.end())
            {
                std::string sData = find->second.m_sValue;
                NSFile::CFileBinary oFile;
                oFile.CreateFileW(pData->get_Path());
                oFile.WriteFile((BYTE*)sData.c_str(), (DWORD)sData.length());
                oFile.CloseFile();
            }
            break;
        }
        case ASC_MENU_EVENT_TYPE_ENCRYPT_PERSONAL_KEY_IMPORT:
        {
            NSAscCrypto::AscCryptoType eType = m_pInternal->m_nCurrentCryptoMode;
            if (NSAscCrypto::None == eType)
                eType = NSAscCrypto::Simple;

            NSEditorApi::CEncryptData* pData = (NSEditorApi::CEncryptData*)pEvent->m_pData;
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_pInternal->m_mapCrypto.find(eType);
            if (find != m_pInternal->m_mapCrypto.end())
            {
                std::string sData;
                if (!pData->get_Path().empty())
                {
                    NSFile::CFileBinary::ReadAllTextUtf8A(pData->get_Path(), sData);
                }
                else
                {
                    std::wstring sValueW = pData->get_Value();
                    sData = U_TO_UTF8(sValueW);
                }

                this->SetCryptoMode(sData, (int)eType);
            }
            break;
        }
        default:
        {
            if (NULL != m_pInternal->m_pAdditional)
                m_pInternal->m_pAdditional->Apply(pEvent);
        }
    }

    RELEASEINTERFACE(pEvent);
}

NSEditorApi::CAscMenuEvent* CAscApplicationManager::ApplySync(NSEditorApi::CAscMenuEvent* pEvent)
{
    return NULL;
}

int CAscApplicationManager::GenerateNextViewId()
{
    m_pInternal->m_nIdCounter++;
    return m_pInternal->m_nIdCounter;
}

CCefView* CAscApplicationManager::CreateCefView(CCefViewWidgetImpl* parent)
{
    m_pInternal->m_nIdCounter++;
    m_pInternal->m_nWindowCounter++;
    CCefView* pView = new CCefView(parent, m_pInternal->m_nIdCounter);
    pView->SetAppManager(this);

    m_pInternal->m_mapViews[m_pInternal->m_nIdCounter] = pView;
    return pView;
}

CCefViewEditor* CAscApplicationManager::CreateCefEditor(CCefViewWidgetImpl* parent)
{
    m_pInternal->m_nIdCounter++;
    m_pInternal->m_nWindowCounter++;
    CCefViewEditor* pView = new CCefViewEditor(parent, m_pInternal->m_nIdCounter);
    pView->SetAppManager(this);

    m_pInternal->m_mapViews[m_pInternal->m_nIdCounter] = pView;
    return pView;
}

CCefViewEditor* CAscApplicationManager::CreateCefPresentationReporter(CCefViewWidgetImpl* parent, CAscReporterData* data)
{    
    m_pInternal->m_nWindowCounter++;
    CCefViewEditor* pView = new CCefViewEditor(parent, data->Id);
    pView->SetAppManager(this);

    m_pInternal->m_mapViews[m_pInternal->m_nIdCounter] = pView;

    pView->LoadReporter(data->ParentId, data->Url);
    if (!data->LocalRecoverFolder.empty())
    {
        pView->OpenReporter(data->LocalRecoverFolder);
    }

    // remove data here
    delete data;
    return pView;
}

CCefView* CAscApplicationManager::GetViewById(int nId)
{
    std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.find(nId);
    if (i == m_pInternal->m_mapViews.end())
        return NULL;
    return i->second;
}
CCefView* CAscApplicationManager::GetViewByUrl(const std::wstring& url)
{
    if (url.empty())
        return NULL;

    for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); ++i)
    {
        CCefView* pView = i->second;
        if (pView->GetUrl() == url)
            return i->second;

        if (pView->GetOriginalUrl() == url)
            return i->second;

        if (pView->GetUrlAsLocal() == url)
            return i->second;
    }
    return NULL;
}
CCefView* CAscApplicationManager::GetViewByRecentId(int nId)
{
    CAscEditorFileInfo oInfo;
    for (std::vector<CAscEditorFileInfo>::iterator i = m_pInternal->m_arRecents.begin(); i != m_pInternal->m_arRecents.end(); i++)
    {
        if (i->m_nId == nId)
        {
            oInfo = *i;
            break;
        }
    }

    if (-1 == oInfo.m_nId)
        return NULL;

    if (oInfo.m_sUrl.empty() && !oInfo.m_sPath.empty())
    {
        for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); ++i)
        {
            CCefView* pView = i->second;

            if (pView->GetType() == cvwtEditor)
            {
                CCefViewEditor* pViewEditor = (CCefViewEditor*)pView;

                std::wstring sPath1 = pViewEditor->GetLocalFilePath();
                std::wstring sPath2 = oInfo.m_sPath;

                NSStringUtils::string_replace(sPath1, L"\\", L"/");
                NSStringUtils::string_replace(sPath2, L"\\", L"/");

                if (sPath1 == sPath2)
                    return pView;
            }
        }
        return NULL;
    }

    return this->GetViewByUrl(oInfo.m_sUrl);
}

std::vector<int> CAscApplicationManager::GetViewsId()
{
    std::vector<int> ret;
    for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); ++i)
    {
        ret.push_back(i->first);
    }
    return ret;
}

void CAscApplicationManager::DestroyCefView(int nId, bool bIsSafe)
{
    if (-1 == nId)
    {
        for (std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.begin(); i != m_pInternal->m_mapViews.end(); i++)
        {
            CCefView* pView = i->second;
            if (pView)
            {
                NSEditorApi::CAscMenuEvent* pEventD = new NSEditorApi::CAscMenuEvent();
                pEventD->m_nType = ASC_MENU_EVENT_TYPE_CEF_DESTROY;
                pView->Apply(pEventD);
            }
        }

        m_pInternal->m_mapViews.clear();
    }
    else
    {
        std::map<int, CCefView*>::iterator i = m_pInternal->m_mapViews.find(nId);
        if (i == m_pInternal->m_mapViews.end())
            return;

        CCefView* pView = i->second;
        if (pView)
        {
            NSEditorApi::CAscMenuEvent* pEventD = new NSEditorApi::CAscMenuEvent();
            pEventD->m_nType = bIsSafe ? ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE : ASC_MENU_EVENT_TYPE_CEF_DESTROY;
            pView->Apply(pEventD);
        }

        m_pInternal->m_mapViews.erase(i);
    }
}

void CAscApplicationManager::Logout(std::wstring strUrl)
{
    m_pInternal->Logout(strUrl, CefCookieManager::GetGlobalManager(NULL));
}

void CAscApplicationManager::LockCS(int nId)
{
    m_pInternal->m_oCS_Scripts.Enter();
}

void CAscApplicationManager::UnlockCS(int nId)
{
    m_pInternal->m_oCS_Scripts.Leave();
}

bool CAscApplicationManager::IsInitFonts()
{
    return (NULL != m_pInternal->m_pApplicationFonts) ? true : false;
}

std::string CAscApplicationManager::GetLibraryPathVariable()
{
#if defined(_LINUX) && !defined(_MAC)
    if (m_pInternal->m_sLD_LIBRARY_PATH.empty())
    {
        char* pEnv = getenv("LD_LIBRARY_PATH");
        m_pInternal->m_sLD_LIBRARY_PATH = std::string(pEnv);
    }
    return m_pInternal->m_sLD_LIBRARY_PATH;
#endif
    return "";
}

CApplicationCEF* CAscApplicationManager::GetApplication()
{
    return m_pInternal->m_pApplication;
}

void CAscApplicationManager::SetApplication(CApplicationCEF* pApp)
{
    m_pInternal->m_pApplication = pApp;

    // создаем все папки, если надо
    if (!NSDirectory::Exists(m_oSettings.recover_path))
        NSDirectory::CreateDirectory(m_oSettings.recover_path);
}

void CAscApplicationManager::SetDebugInfoSupport(bool bIsSupport)
{
    m_pInternal->m_bDebugInfoSupport = bIsSupport;
}

bool CAscApplicationManager::GetDebugInfoSupport()
{
    return m_pInternal->m_bDebugInfoSupport;
}

void CAscApplicationManager::CloseApplication()
{
    this->DestroyCefView(-1);
    this->m_pInternal->CloseApplication();
    if (NULL != m_pInternal->m_pApplication)
        m_pInternal->m_pApplication->Close();
}

NSFonts::IApplicationFonts* CAscApplicationManager::GetApplicationFonts()
{
    return m_pInternal->m_pApplicationFonts;
}

void CAscApplicationManager::StartSaveDialog(const std::wstring& sName, unsigned int nId)
{
    // в этой реализации - всегда отмена
    // хочешь пользоваться - пиши враппер
    this->EndSaveDialog(L"", nId);
}

void CAscApplicationManager::EndSaveDialog(const std::wstring& sPath, unsigned int nId)
{
    std::wstring sPathWin = sPath;
#ifdef WIN32
    NSCommon::string_replace(sPathWin, L"/", L"\\");
#endif

    CCefView* pView = this->GetViewById(m_pInternal->m_nIsCefSaveDialogWait);
    if (NULL != pView)
    {
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG);
        NSEditorApi::CAscSaveDialog* pData = new NSEditorApi::CAscSaveDialog();
        pData->put_Id(pView->GetId());
        pData->put_IdDownload(nId);
        
        if (NSFile::CFileBinary::Exists(sPathWin))
            NSFile::CFileBinary::Remove(sPathWin);

        pData->put_FilePath(sPathWin);

        pEvent->m_pData = pData;
        pView->Apply(pEvent);
    }
}

bool CAscApplicationManager::IsPlatformKeyboardSupport()
{
#ifdef WIN32
    return true;
#endif
    return false;
}

int CAscApplicationManager::GetPlatformKeyboardLayout()
{
#ifdef WIN32
    HWND wFocus = GetFocus();
    DWORD dwThread = GetWindowThreadProcessId(wFocus, 0);
    HKL hkl = GetKeyboardLayout(dwThread);
    int nLang = LOWORD(hkl);
    return nLang;
#endif
    return -1;
}

void CAscApplicationManager::OnDestroyWindow()
{
    m_pInternal->m_nWindowCounter--;
    if (0 == m_pInternal->m_nWindowCounter)
    {
        m_pInternal->m_pApplication->ExitMessageLoop();
    }
}

void CAscApplicationManager::CancelDownload(unsigned int nDownloadId)
{
    if (m_pInternal->m_mapCancelDownloads.end() == m_pInternal->m_mapCancelDownloads.find(nDownloadId))
        m_pInternal->m_mapCancelDownloads.insert(std::pair<unsigned int, bool>(nDownloadId, true));
}

int CAscApplicationManager::GetFileFormatByExtentionForSave(const std::wstring& sFileName)
{
    std::wstring sName = NSCommon::GetFileExtention(sFileName);

    int nFormat = -1;
    if (sName == L"docx")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
    if (sName == L"dotx")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX;
    if (sName == L"odt")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT;
    if (sName == L"rtf")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF;
    if (sName == L"txt")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT;
    if (sName == L"html")
        nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML;

    if (sName == L"xlsx")
        nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
    if (sName == L"xltx")
        nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX;
    if (sName == L"ods")
        nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS;
    if (sName == L"csv")
        nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV;

    if (sName == L"pptx")
        nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
    if (sName == L"potx")
        nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX;
    if (sName == L"odp")
        nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP;

    if (sName == L"pdf")
        nFormat = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF;

    return nFormat;
}

void CAscApplicationManager::InitAdditionalEditorParams(std::wstring& sParams)
{
    m_pInternal->m_sAdditionalUrlParams = sParams;
}

void CAscApplicationManager::DoMessageLoopWork()
{
    CefDoMessageLoopWork();
}

bool CAscApplicationManager::IsExternalEventLoop()
{
    return false;
}

void CAscApplicationManager::ExitExternalEventLoop()
{
    // none
}

int CAscApplicationManager::GetMonitorScaleByIndex(const int& nIndex, unsigned int& nDpiX, unsigned int& nDpiY)
{
    if (m_pInternal->m_nForceDisplayScale > 0)
        return m_pInternal->m_nForceDisplayScale;

#ifdef WIN32
    if (0 != Core_GetMonitorRawDpiByIndex(nIndex, &nDpiX, &nDpiY))
        return -1;

    if (nDpiX > 180 && nDpiY > 180)
        return 2;

    return 1;
#endif

    return -1;
}

int CAscApplicationManager::GetMonitorScaleByWindow(const WindowHandleId& nHandle, unsigned int& nDpiX, unsigned int& nDpiY)
{
    if (m_pInternal->m_nForceDisplayScale > 0)
        return m_pInternal->m_nForceDisplayScale;

#ifdef WIN32
    if (0 != Core_GetMonitorRawDpi(nHandle, &nDpiX, &nDpiY))
        return -1;

    if (nDpiX > 180 && nDpiY > 180)
        return 2;

    return 1;
#endif

    return -1;
}

void CAscApplicationManager::SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* pEvent)
{
    m_pInternal->SetEventToAllMainWindows(pEvent);
}

void CAscApplicationManager::SetCryptoMode(const std::string& sPassword, const int& nMode)
{
    if (0 < nMode && !sPassword.empty())
    {
        std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_pInternal->m_mapCrypto.find((NSAscCrypto::AscCryptoType)nMode);
        if (find != m_pInternal->m_mapCrypto.end())
        {
            find->second.m_sValue = sPassword;
        }
    }

    m_pInternal->m_nCurrentCryptoMode = (NSAscCrypto::AscCryptoType)nMode;

    NSAscCrypto::CCryptoMode oCryptoMode;
    oCryptoMode.Save(m_pInternal->m_cryptoKeyEnc, m_pInternal->m_cryptoKeyDec, m_pInternal->m_mapCrypto, m_oSettings.cookie_path + L"/user.data");

    // не меняем режим для уже открылись
    // m_pInternal->SendCryptoData();

    std::string sCryptoMode = "default";
    if (!sCryptoMode.empty())
        sCryptoMode = std::to_string(m_pInternal->m_nCurrentCryptoMode);

    m_pInternal->CheckSetting("--crypto-mode", sCryptoMode);
    m_pInternal->m_mapSettings.insert(std::pair<std::string, std::string>("crypto-mode", std::to_string(m_pInternal->m_nCurrentCryptoMode)));
    m_pInternal->SaveSettings();

    std::string sCodeInAllFrames = "if (window.onChangeCryptoMode) { window.onChangeCryptoMode(" + std::to_string(m_pInternal->m_nCurrentCryptoMode) + "); }";
    m_pInternal->ExecuteInAllFrames(sCodeInAllFrames);
}

int CAscApplicationManager::GetCryptoMode()
{
    return m_pInternal->m_nCurrentCryptoMode;
}

std::vector<int> CAscApplicationManager::GetSupportCryptoModes()
{
    CPluginsManager oPlugins;
    oPlugins.m_strDirectory = m_oSettings.system_plugins_path;
    oPlugins.m_strUserDirectory = m_oSettings.user_plugins_path;

    oPlugins.GetInstalledPlugins();
    std::vector<int> retValue;

    for (std::map<int, std::string>::iterator iter = oPlugins.m_arCryptoModes.begin(); iter != oPlugins.m_arCryptoModes.end(); iter++)
    {
        retValue.push_back(iter->first);
    }

    return retValue;
}

NSAscCrypto::CAscKeychain* CAscApplicationManager::GetKeychainEngine()
{
    return new NSAscCrypto::CAscKeychain(m_pInternal);
}

/////////////////////////////////////////////////////////////
