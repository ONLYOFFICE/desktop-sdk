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

#include "./applicationmanager_p.h"
#include "./plugins.h"
#include "./cefwrapper/client_resource_handler_async.h"
#include "./cefwrapper/monitor_info.h"

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

#if !defined(_MAC) && !defined(CEF_VERSION_107)
void __attribute__((constructor)) preload_engine()
{
	setenv("LD_PRELOAD", "libcef.so", 1);
}
#endif

#endif

#ifdef FILE_SAVE_ADDONS
#include "file_addons.h"
#endif

CAscApplicationSettings::CAscApplicationSettings()
{
	std::wstring sApplicationPath   = NSFile::GetProcessDirectory();
	NSStringUtils::string_replace(sApplicationPath, L"\\", L"/");

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

#ifdef FEATURE_ENABLE_PROTECT
	protect_support                 = true;
#else
	protect_support                 = false;
#endif

#ifdef FEATURE_ENABLE_SIGNATURE
	sign_support                    = true;
#else
	sign_support                    = false;
#endif

	pass_support                    = true;

	user_providers_path             = L"";

	converter_application_name      = "ONLYOFFICE";
	converter_application_company   = "Ascensio System SIA Copyright (c) 2022";

	macroses_support                = true;
	plugins_support                 = true;
}
CAscApplicationSettings::~CAscApplicationSettings()
{
}

void CAscApplicationSettings::SetUserDataPath(std::wstring sPath)
{
	app_data_path                   = sPath;
	NSStringUtils::string_replace(app_data_path, L"\\", L"/");

	cache_path                      = app_data_path + L"/data/cache";
	cookie_path                     = app_data_path + L"/data";

	use_system_fonts                = true;
	fonts_cache_info_path           = app_data_path + L"/data/fonts";
	recover_path                    = app_data_path + L"/data/recover";
	user_dictionaries_path          = app_data_path + L"/data/dictionaries";

	user_plugins_path               = app_data_path + L"/data/sdkjs-plugins";
}

// ---------------------------------------------------------------------------------
class CUserSettings_Private
{
public:
	std::map<std::string, std::string> m_mapSettings;
	CAscApplicationManager* m_pManager;

public:
	CUserSettings_Private()
	{
		m_pManager = NULL;
	}

	void SetManager(CAscApplicationManager* pManager)
	{
		m_pManager = pManager;

		std::wstring sFile = m_pManager->m_oSettings.fonts_cache_info_path + L"/../settings.xml";
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

	std::wstring Get(const std::wstring& name)
	{
		if (!m_pManager)
			return L"";
		std::string sNameA = U_TO_UTF8(name);
		std::map<std::string, std::string>::iterator pair = m_mapSettings.find(sNameA);
		if (pair == m_mapSettings.end())
			return L"";
		return UTF8_TO_U((pair->second));
	}
	void Set(const std::wstring& name, const std::wstring& value)
	{
		if (!m_pManager)
			return;
		std::string sNameA = U_TO_UTF8(name);
		std::map<std::string, std::string>::iterator pair = m_mapSettings.find(sNameA);
		if (pair != m_mapSettings.end())
			m_mapSettings.erase(pair);

		if (L"default" != value)
		{
			std::string sValueA = U_TO_UTF8(value);
			m_mapSettings.insert(std::pair<std::string, std::string>(sNameA, sValueA));
		}
	}
};

CUserSettings::CUserSettings()
{
	m_pInternal = new CUserSettings_Private();
}
CUserSettings::~CUserSettings()
{
	RELEASEOBJECT(m_pInternal);
}
std::wstring CUserSettings::Get(const std::wstring& name)
{
	return m_pInternal->Get(name);
}
void CUserSettings::Set(const std::wstring& name, const std::wstring& value)
{
	m_pInternal->Set(name, value);
	m_pInternal->m_pManager->m_pInternal->SaveSettings(&m_pInternal->m_mapSettings);
	m_pInternal->m_pManager->m_pInternal->LoadSettings();

	if (L"force-scale" == name)
	{
		std::map<int, CCefView*>* mapViews = &m_pInternal->m_pManager->m_pInternal->m_mapViews;
		for (std::map<int, CCefView*>::iterator i = mapViews->begin(); i != mapViews->end(); ++i)
		{
			CCefView* view = i->second;
			view->moveEvent();
		}
	}
}
// ---------------------------------------------------------------------------------

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

	if (NULL != m_pInternal->m_pDpiChecker)
	{
		delete m_pInternal->m_pDpiChecker;
		m_pInternal->m_pDpiChecker = NULL;
	}
}

void CAscApplicationManager::StartSpellChecker()
{
	m_pInternal->m_oSpellChecker.SetApplicationManager(this);
	m_pInternal->m_oSpellChecker.Init(m_oSettings.spell_dictionaries_path, m_oSettings.user_dictionaries_path);
	m_pInternal->m_oSpellChecker.Start();
}

void CAscApplicationManager::StopSpellChecker()
{
	m_pInternal->m_oSpellChecker.End();
}

void CAscApplicationManager::SpellCheck(const int& nEditorId, const std::string& sTask, int_64_type nId)
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
	if (!NSDirectory::Exists(m_oSettings.fonts_cache_info_path))
	{
#ifdef WIN32
		std::wstring sTmpDir = m_oSettings.fonts_cache_info_path;
		NSStringUtils::string_replace(sTmpDir, L"/", L"\\");
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

		bool bIsPrivate = false;
		std::wstring s1 = pData->get_Url();
		NSStringUtils::string_replace(s1, L"/./", L"/");
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
	case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG:
	{
		CCefView* pView = GetViewById(((NSEditorApi::CAscCefMenuEvent*)pEvent)->get_SenderId());

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
	m_pInternal->ChangeEditorViewsCount();
	return pView;
}

CCefViewEditor* CAscApplicationManager::CreateCefPresentationReporter(CCefViewWidgetImpl* parent, CAscReporterData* data)
{    
	m_pInternal->m_nWindowCounter++;
	CCefViewEditor* pView = new CCefViewEditor(parent, data->Id);
	pView->SetAppManager(this);

	m_pInternal->m_mapViews[m_pInternal->m_nIdCounter] = pView;
	m_pInternal->ChangeEditorViewsCount();

	pView->LoadReporter(data);
	if (!data->LocalRecoverFolder.empty())
	{
		pView->OpenReporter(data->LocalRecoverFolder);
	}

	// remove data here
	delete data;
	return pView;
}

std::wstring CAscApplicationManager::GetNewFilePath(const int& nFileFormat)
{
	std::string sCountry = m_oSettings.country;
	NSCommon::makeUpper(sCountry);

	std::wstring sPrefix = NSUrlParse::GetUrlValue(m_pInternal->m_sAdditionalUrlParams, L"lang");
	if (NSDirectory::Exists(m_oSettings.file_converter_path + L"/empty/" + sPrefix))
	{
		sPrefix += L"/";
	}
	else
	{
		// поддержка en-EN (в документах есть en-GB и en-US)
		sPrefix = L"en-US/";
	}

	std::wstring sFilePath = m_oSettings.file_converter_path + L"/empty/" + sPrefix + L"new.";
	std::wstring sExtension = L"docx";

	if (nFileFormat == etPresentation)
		sExtension = L"pptx";
	else if (nFileFormat == etSpreadsheet)
		sExtension = L"xlsx";

	sFilePath += sExtension;
	if (!NSFile::CFileBinary::Exists(sFilePath))
	{
		std::wstring sTestName = m_oSettings.file_converter_path + L"/empty/" + L"new." + sExtension;
		if (NSFile::CFileBinary::Exists(sTestName))
			sFilePath = sTestName;
	}
	return sFilePath;
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
		m_pInternal->ChangeEditorViewsCount();
	}
}

void CAscApplicationManager::Logout(std::wstring strUrl)
{
	m_pInternal->Logout(strUrl, CefCookieManager::GetGlobalManager(nullptr));
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
		if (pEnv)
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
		NSDirectory::CreateDirectories(m_oSettings.recover_path);
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
	NSResourceHandlerFileAsyncManager::Destroy();
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
	NSStringUtils::string_replace(sPathWin, L"/", L"\\");
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
		CefCookieManager::GetGlobalManager(nullptr)->FlushStore(nullptr);
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
	std::wstring sName = NSFile::GetFileExtention(sFileName);

	int nFormat = -1;
	if (sName == L"docx")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
	else if (sName == L"docxf")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF;
	else if (sName == L"oform")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM;
	else if (sName == L"dotx")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX;
	else if (sName == L"odt")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT;
	else if (sName == L"ott")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT;
	else if (sName == L"rtf")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF;
	else if (sName == L"txt")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT;
	else if (sName == L"html")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML;
	else if (sName == L"xlsx")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
	else if (sName == L"xltx")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX;
	else if (sName == L"ods")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS;
	else if (sName == L"ots")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS;
	else if (sName == L"csv")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV;
	else if (sName == L"pptx")
		nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
	else if (sName == L"ppsx")
		nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX;
	else if (sName == L"potx")
		nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX;
	else if (sName == L"odp")
		nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP;
	else if (sName == L"otp")
		nFormat = AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP;
	else if (sName == L"pdf")
		nFormat = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF;
	else if (sName == L"fb2")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2;
	else if (sName == L"epub")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB;
	else if (sName == L"png")
		nFormat = AVS_OFFICESTUDIO_FILE_IMAGE_PNG;
	else if (sName == L"jpg" || sName == L"jpeg")
		nFormat = AVS_OFFICESTUDIO_FILE_IMAGE_JPG;
#ifdef FILE_SAVE_ADDONS
	FILE_SAVE_ADDONS
		#endif

			return nFormat;
}

void CAscApplicationManager::InitAdditionalEditorParams(std::wstring& sParams)
{
	m_pInternal->m_sAdditionalUrlParams = sParams;
}

double CAscApplicationManager::GetMonitorScaleByIndex(const int& nIndex, unsigned int& nDpiX, unsigned int& nDpiY)
{
	if (m_pInternal->m_dForceDisplayScale > 0)
		return m_pInternal->m_dForceDisplayScale;

	CAscDpiChecker* pDpiChecker = CAscApplicationManager::GetDpiChecker();
	if (pDpiChecker)
	{
		unsigned int nDpiX = 0;
		unsigned int nDpiY = 0;
		int nRet = pDpiChecker->GetMonitorDpi(nIndex, &nDpiX, &nDpiY);
		if (nRet >= 0)
		{
			return pDpiChecker->GetScale(nDpiX, nDpiY);
		}
	}

	return -1;
}

double CAscApplicationManager::GetMonitorScaleByWindow(const WindowHandleId& nHandle, unsigned int& nDpiX, unsigned int& nDpiY)
{
	if (m_pInternal->m_dForceDisplayScale > 0)
		return m_pInternal->m_dForceDisplayScale;

	CAscDpiChecker* pDpiChecker = CAscApplicationManager::GetDpiChecker();
	if (pDpiChecker)
	{
		unsigned int nDpiX = 0;
		unsigned int nDpiY = 0;
#ifndef _MAC
		int nRet = pDpiChecker->GetWindowDpi(nHandle, &nDpiX, &nDpiY);
#else
		int nRet = pDpiChecker->GetWindowDpi(const_cast<WindowHandleId>(nHandle), &nDpiX, &nDpiY);
#endif
		if (nRet >= 0)
		{
			return pDpiChecker->GetScale(nDpiX, nDpiY);
		}
	}

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

CAscDpiChecker::CAscDpiChecker(CAscApplicationManager* pManager)
{
	m_pManager = pManager;
}
CAscDpiChecker::~CAscDpiChecker()
{
}
int CAscDpiChecker::GetWindowDpi(WindowHandleId _handle, unsigned int* _dx, unsigned int* _dy)
{
#ifdef WIN32
	return Core_GetMonitorRawDpi(_handle, _dx, _dy);
#endif
	return -1;
}
int CAscDpiChecker::GetMonitorDpi(int _screen, unsigned int* _dx, unsigned int* _dy)
{
#ifdef WIN32
	return Core_GetMonitorRawDpiByIndex(_screen, _dx, _dy);
#endif
	return -1;
}
int CAscDpiChecker::GetWidgetImplDpi(CCefViewWidgetImpl* wid, unsigned int* _dx, unsigned int* _dy)
{
	if (NULL == wid)
		return -1;
#ifdef WIN32
	return GetWindowDpi(wid->cef_handle, _dx, _dy);
#endif
	return -1;
}
double CAscDpiChecker::GetScale(unsigned int dpiX, unsigned int dpiY)
{
	return Core_GetMonitorScale(dpiX, dpiY);
}

double CAscDpiChecker::GetForceScale(unsigned int* dpix, unsigned int* dpiy)
{
	if (!m_pManager)
		return -1;
	if (m_pManager->m_pInternal->m_dForceDisplayScale < 0)
		return -1;

	double dScale = m_pManager->m_pInternal->m_dForceDisplayScale;
	if (dpix) *dpix = (unsigned int)(dScale * 96);
	if (dpiy) *dpiy = (unsigned int)(dScale * 96);
	return dScale;
}

CAscDpiChecker* CAscApplicationManager_Private::m_pDpiChecker = NULL;

CAscDpiChecker* CAscApplicationManager::GetDpiChecker()
{
	return CAscApplicationManager_Private::m_pDpiChecker;
}
CAscDpiChecker* CAscApplicationManager::InitDpiChecker()
{
#ifdef WIN32
	return new CAscDpiChecker(this);
#else
	return NULL;
#endif
}

std::vector<std::string> CAscApplicationManager::GetRendererStartupProperties()
{
	std::vector<std::string> props;

	bool bIsOnlyPassSupport = m_oSettings.pass_support;
	if (bIsOnlyPassSupport)
	{
		if (0 == m_pInternal->m_mapCrypto.size())
			bIsOnlyPassSupport = false;
	}

	int nFlags = 0;
	if (bIsOnlyPassSupport)
		nFlags |= 0x01;

	if (m_oSettings.protect_support)
		nFlags |= 0x02;

	props.push_back("onlypass=" + std::to_string(nFlags));
	props.push_back("cryptomode=" + std::to_string(m_pInternal->m_nCurrentCryptoMode));
	props.push_back("system_plugins_path=" + U_TO_UTF8(m_oSettings.system_plugins_path));
	props.push_back("user_plugins_path=" + U_TO_UTF8(m_oSettings.user_plugins_path));
	props.push_back("cookie_path=" + U_TO_UTF8(m_oSettings.cookie_path));
	props.push_back("app_data_path=" + U_TO_UTF8(m_oSettings.app_data_path));
	props.push_back(m_pInternal->m_bDebugInfoSupport ? "debug_mode=true" : "debug_mode=false");
	props.push_back("fonts_cache_path=" + U_TO_UTF8(m_oSettings.fonts_cache_info_path));
	props.push_back("tmp_folder=" + U_TO_UTF8(m_pInternal->StartTmpDirectory()));
	props.push_back("recovers_folder=" + U_TO_UTF8(m_oSettings.recover_path));
	props.push_back("renderer_process_variable=" + U_TO_UTF8(m_pInternal->m_sRendererJSON));

	if (!m_oSettings.macroses_support) props.push_back("macroses_support=false");
	if (!m_oSettings.plugins_support) props.push_back("plugins_support=false");

	return props;
}

CUserSettings* CAscApplicationManager::GetUserSettings()
{
	CUserSettings* pSettings = new CUserSettings();
	pSettings->m_pInternal->SetManager(this);
	return pSettings;
}

bool CAscApplicationManager::IsResolveLocalFile(const std::wstring& sFile)
{
	return m_pInternal->m_oLocalFilesResolver.Check(sFile);
}

void CAscApplicationManager::AddFileToLocalResolver(const std::wstring& sFile)
{
	return m_pInternal->m_oLocalFilesResolver.AddFile(sFile);
}

void CAscApplicationManager::SetRendererProcessVariable(const std::wstring& sVariable)
{
	m_pInternal->m_sRendererJSON = sVariable;
}

std::wstring CAscApplicationManager::GetExternalSchemeName()
{
	if (m_pInternal->m_pAdditional)
		return m_pInternal->m_pAdditional->GetExternalSchemeName();
	return L"oo-office";
}

bool NSCommon::CSystemWindowScale::g_isUseSystemScalingInit = false;
bool NSCommon::CSystemWindowScale::g_isUseSystemScaling = false;

bool CAscApplicationManager::IsUseSystemScaling()
{
	return NSCommon::CSystemWindowScale::IsUseSystemScaling();
}
double Core_GetMonitorScale(const unsigned int& xDpi, const unsigned int& yDpi)
{
	// допустимые значения: 1; 1.25; 1.5; 1.75; 2;
	double dScale = (xDpi + yDpi) / (2 * 96.0);
	int nCount = (int)((dScale + 0.125) / 0.25);
	dScale = 0.25 * nCount;
	if (dScale > 2) dScale = 2;
	if (dScale < 1) dScale = 1;
	return dScale;
}
