/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./applicationmanager_p.h"
#include "./plugins.h"
#include "./cefwrapper/client_resource_handler_async.h"
#include "./cefwrapper/monitor_info.h"

#include "../native-plugins/src/plugin_manager.h"
#include "../native-plugins/src/plugin_host_browser.h"
#include "../../../../core/DesktopEditor/common/Path.h"

#ifndef _WIN32
#include <stdio.h>
#endif

#if defined(_MAC)
# include "mac_keyboardlayout.h"
#else
# include "keyboardlayout.h"
#endif

#ifdef LINUX
CApplicationCEF* CLinuxData::app_cef = NULL;
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
	std::wstring sApplicationPath = NSFile::GetProcessDirectory();
	NSStringUtils::string_replace(sApplicationPath, L"\\", L"/");

	app_data_path = sApplicationPath;

	spell_dictionaries_path = sApplicationPath + L"/Dictionaries";
	cache_path              = sApplicationPath + L"/data/cache";
	cookie_path             = sApplicationPath + L"/data";

	use_system_fonts        = true;
	fonts_cache_info_path   = app_data_path + L"/data/fonts";

	system_plugins_path     = app_data_path + L"/editors/sdkjs-plugins";
	user_plugins_path       = app_data_path + L"/editors/sdkjs-plugins-user";

	local_editors_path      = sApplicationPath + L"/editors/web-apps/apps/api/documents/index.html";
	file_converter_path     = sApplicationPath + L"/converter";
	recover_path            = app_data_path + L"/data/recover";

	system_templates_path   = file_converter_path + L"/templates";
	templates_image_width   = 140;
	templates_image_height  = 200;

	country = "RU";

#ifdef FEATURE_ENABLE_PROTECT
	protect_support = true;
#else
	protect_support = false;
#endif

#ifdef FEATURE_ENABLE_SIGNATURE
	sign_support = true;
#else
	sign_support = false;
#endif

	pass_support = true;

	user_providers_path = L"";

	converter_application_name    = "ONLYOFFICE";
	converter_application_company = "Ascensio System SIA Copyright (c) 2022";

	macroses_support = true;
	plugins_support = true;
}
CAscApplicationSettings::~CAscApplicationSettings()
{
}

void CAscApplicationSettings::SetUserDataPath(std::wstring sPath)
{
	app_data_path = sPath;
	NSStringUtils::string_replace(app_data_path, L"\\", L"/");

	cache_path             = app_data_path + L"/data/cache";
	cookie_path            = app_data_path + L"/data";

	use_system_fonts       = true;
	fonts_cache_info_path  = app_data_path + L"/data/fonts";
	recover_path           = app_data_path + L"/data/recover";
	user_dictionaries_path = app_data_path + L"/data/dictionaries";

	user_plugins_path      = app_data_path + L"/data/sdkjs-plugins";

	templates_cache_info_path = app_data_path + L"/data/templates_cache";
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

		std::vector<XmlUtils::CXmlNode> oNodes;
		if (!oNode.GetChilds(oNodes))
			return;

		size_t nCount = oNodes.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			XmlUtils::CXmlNode& oSetting = oNodes[i];

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
	m_pInternal->m_oTemplatesCache.SetManager(this);
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
	if (GetEventListener())
		GetEventListener()->OnEvent(new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD));
}

std::vector<std::pair<std::string, std::string>> CAscApplicationManager::GetKeyboardLayoutList() const
{
#if !defined(_MAC)
    KeyboardLayout kbl;
    return kbl.GetKeyboardLayoutList();
#else
    return std::vector<std::pair<std::string, std::string>>();
#endif
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

		// check whether this file is already locked
		std::map<std::wstring, std::vector<CEditorFrameId>>::iterator _find = m_pInternal->m_mapLoadedScripts.find(pData->get_Destination());
		if (_find != m_pInternal->m_mapLoadedScripts.end())
		{
			// it is being downloaded. add the id to the list of those waiting for the script once it is loaded
			CEditorFrameId _id;
			_id.Url = pData->get_Url();
			_id.EditorId = pData->get_Id();
			_id.FrameId = pData->get_FrameId();
			_find->second.push_back(_id);
		}
		else
		{
			// check whether the file is already on disk
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
				// create the waiting list
				std::vector<CEditorFrameId> _arr;
				CEditorFrameId _id;
				_id.Url = pData->get_Url();
				_id.EditorId = pData->get_Id();
				_id.FrameId = pData->get_FrameId();
				_arr.push_back(_id);

				m_pInternal->m_mapLoadedScripts.insert(std::pair<std::wstring, std::vector<CEditorFrameId>>(pData->get_Destination(), _arr));

				if (std::wstring::npos == pData->get_Url().find(L"sdk/Common/AllFonts.js") && std::wstring::npos == pData->get_Url().find(L"sdkjs/common/AllFonts.js"))
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

std::wstring CAscApplicationManager::GetNewFilePath(const AscEditorType& nFileFormat)
{
	std::string sCountry = m_oSettings.country;
	NSCommon::makeUpper(sCountry);

	std::wstring sPrefix = NSUrlParse::GetUrlValue(m_pInternal->m_sAdditionalUrlParams, L"lang");
	if (!sPrefix.empty() && NSDirectory::Exists(m_oSettings.file_converter_path + L"/empty/" + sPrefix))
	{
		sPrefix += L"/";
	}
	else
	{
		// en-EN support (documents have en-GB and en-US)
		sPrefix = L"en-US/";
	}

	std::wstring sFilePath = m_oSettings.file_converter_path + L"/empty/" + sPrefix + L"new.";
	std::wstring sExtension = L"docx";

	if (nFileFormat == AscEditorType::etPresentation)
		sExtension = L"pptx";
	else if (nFileFormat == AscEditorType::etSpreadsheet)
		sExtension = L"xlsx";
	else if (nFileFormat == AscEditorType::etDocumentMasterForm || nFileFormat == AscEditorType::etDocumentMasterOForm /* this should not happen, for now - only from a template */)
		sExtension = L"pdf";
	else if (nFileFormat == AscEditorType::etPdf)
		sExtension = L"pdf";
	else if (nFileFormat == AscEditorType::etDraw)
		sExtension = L"vsdx";

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
		m_pInternal->m_bIsExitMessageLoop = true;
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

	// create all the folders if needed
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
	m_pInternal->StopNativePlugins();
	NSResourceHandlerFileAsyncManager::Destroy();
	this->DestroyCefView(-1);
	this->m_pInternal->CloseApplication();
	if (NULL != m_pInternal->m_pApplication)
		m_pInternal->m_pApplication->Close();
}

void CAscApplicationManager_Private::StartNativePlugins()
{
	if (NULL != m_pNativePluginManager)
		return;
	m_pNativePluginHost = new CBrowserPluginHost(m_pMain);
	m_pNativePluginManager = new NSNativePlugin::CNativePluginManager();
	m_pNativePluginManager->SetHost(m_pNativePluginHost);
	m_pNativePluginManager->SetDirectories(m_arNativePluginsDirs);
	m_pNativePluginManager->LoadAll();
}

void CAscApplicationManager_Private::StopNativePlugins()
{
	if (NULL != m_pNativePluginManager)
	{
		// the host is deleted right after, so the stop never ends for it
		if (NULL != m_pNativePluginHost)
			m_pNativePluginHost->BeginStop();
		m_pNativePluginManager->StopAll();
		delete m_pNativePluginManager;
		m_pNativePluginManager = NULL;
	}
	if (NULL != m_pNativePluginHost)
	{
		delete m_pNativePluginHost;
		m_pNativePluginHost = NULL;
	}
}

bool CAscApplicationManager_Private::NativePluginDispatchMessage(const std::string& sName, const std::string& sArgsJson)
{
	if (NULL != m_pNativePluginManager)
		return m_pNativePluginManager->DispatchPluginMessage(sName, sArgsJson);
	return false;
}

#ifdef ADDON_STOP_PROCESSES
// ADDON_STOP_PROCESSES: StopNativeCodeInDirectory also ends the processes
// started from the directory (the AI chat's docbuilder-tools), so a removed or
// replaced addon lets go of its executable at once. Without it a running
// executable keeps its file until the application exits, and the addon change
// is finished at the next start (PrepareAddonDirectory).

// sDirectory in every spelling a process command line may carry it in: ".."
// resolved, and on Windows also its long and its 8.3 short form - a plugin
// starts its executable by the short path when the long one has a space in it
// (aitools-bridge does).
static std::vector<std::wstring> DirectorySpellings(const std::wstring& sDirectory)
{
	std::vector<std::wstring> arResult;
	std::wstring sNormalized = NSSystemPath::NormalizePath(sDirectory);
	arResult.push_back(sNormalized);
#ifdef _WIN32
	std::wstring sNative = sNormalized;
	NSStringUtils::string_replace(sNative, L"/", L"\\");
	typedef DWORD (WINAPI *TPathFn)(LPCWSTR, LPWSTR, DWORD);
	TPathFn arFns[2] = { GetLongPathNameW, GetShortPathNameW };
	for (int i = 0; i < 2; ++i)
	{
		DWORD nNeed = arFns[i](sNative.c_str(), NULL, 0);
		if (0 == nNeed)
			continue;
		std::vector<wchar_t> aBuffer(nNeed + 1);
		DWORD nLen = arFns[i](sNative.c_str(), &aBuffer[0], (DWORD)aBuffer.size());
		if (0 != nLen && nLen < aBuffer.size())
			arResult.push_back(std::wstring(&aBuffer[0], nLen));
	}
#endif
	return arResult;
}

// Ends, in the frame it runs in, every process started (window.ExternalProcess)
// from an executable inside one of arDirectories.
static std::string EndProcessesScript(const std::vector<std::wstring>& arDirectories)
{
	std::string sDirectories;
	for (size_t i = 0; i < arDirectories.size(); ++i)
	{
		std::string sDirectory = U_TO_UTF8(arDirectories[i]);
		NSStringUtils::string_replaceA(sDirectory, "\\", "\\\\");
		NSStringUtils::string_replaceA(sDirectory, "\"", "\\\"");
		if (!sDirectories.empty())
			sDirectories += ",";
		sDirectories += "\"" + sDirectory + "\"";
	}
#ifdef _WIN32
	std::string sIgnoreCase = "true";
#else
	std::string sIgnoreCase = "false";
#endif

	return std::string(R"JS((function(dirs, ignoreCase) {
	var reg = window["_external_process_callback"];
	var editor = window["AscDesktopEditor"];
	if (!reg || !editor || !editor["_endProcess"])
		return;
	// one spelling for a path: a leading quote dropped, '/' separators, ".." resolved
	function norm(s) {
		s = String(s).replace(/^\s+/, "");
		if (s.charAt(0) === "\"")
			s = s.substring(1);
		var parts = s.replace(/\\/g, "/").split("/"), out = [];
		for (var i = 0; i < parts.length; i++) {
			if (parts[i] === "..") {
				if (out.length > 1)
					out.pop();
			} else if (parts[i] !== "." && (parts[i] !== "" || i === 0)) {
				out.push(parts[i]);
			}
		}
		s = out.join("/");
		return ignoreCase ? s.toLowerCase() : s;
	}
	var prefixes = [];
	for (var i = 0; i < dirs.length; i++) {
		var d = norm(dirs[i]);
		prefixes.push(d.charAt(d.length - 1) === "/" ? d : d + "/");
	}
	for (var id in reg) {
		var p = reg[id];
		if (!p || typeof p.command !== "string")
			continue;
		var c = norm(p.command);
		for (var j = 0; j < prefixes.length; j++) {
			if (0 === c.indexOf(prefixes[j])) {
				// not p.end(): that drops the process from the registry first,
				// and whoever started it would never hear that it stopped
				editor["_endProcess"](p.id);
				break;
			}
		}
	}
})()JS") + "[" + sDirectories + "], " + sIgnoreCase + ");";
}
#endif

void CAscApplicationManager_Private::StopNativeCodeInDirectory(const std::wstring& sDirectory)
{
#ifdef ADDON_STOP_PROCESSES
	// The processes: their handles live in the JS of the main view's frames.
	CCefView* pMainView = GetViewWithMinId();
	if (NULL != pMainView)
		pMainView->ExecuteInAllFrames(EndProcessesScript(DirectorySpellings(sDirectory)), true);
#endif

	// The plugins of every renderer (the host hands the message over on the
	// CEF UI thread, whichever thread this is).
	if (NULL != m_pNativePluginHost)
		m_pNativePluginHost->SendToRenderers("native_plugins_stop_dir", U_TO_UTF8(sDirectory));

	// The plugins of this process.
	if (NULL != m_pNativePluginManager)
	{
		if (NULL != m_pNativePluginHost)
			m_pNativePluginHost->BeginStop();
		m_pNativePluginManager->StopInDirectory(sDirectory);
		if (NULL != m_pNativePluginHost)
			m_pNativePluginHost->EndStop();
	}

	// Renderers started from now on are not handed the directory.
	for (std::vector<std::wstring>::iterator it = m_arNativePluginsDirs.begin(); it != m_arNativePluginsDirs.end(); )
	{
		if (NSSystemPath::IsPathInDirectory(sDirectory, *it))
			it = m_arNativePluginsDirs.erase(it);
		else
			++it;
	}
}

#ifdef USE_ADDON_MODE
// What a running application could not finish with the addon directory - on
// Windows a library still loaded or an executable still running keeps its file
// - is finished at the next start, before anything is loaded from it:
//   <addon>.remove  a marker: delete <addon>
//   <addon>.new     an installed addon waiting to replace <addon>
static std::wstring AddonDirectory(const CAscApplicationSettings& oSettings)
{
	return oSettings.user_plugins_path + L"/../addon";
}
static std::wstring AddonRemoveMarker(const std::wstring& sAddon)
{
	return sAddon + L".remove";
}
static std::wstring AddonStaging(const std::wstring& sAddon)
{
	return sAddon + L".new";
}

// The renderers let go of the addon asynchronously: they are given ~2 s.
static bool DeleteDirectoryWithRetries(const std::wstring& sDirectory)
{
	for (int i = 0; i < 20; ++i)
	{
		if (NSDirectory::Exists(sDirectory))
			NSDirectory::DeleteDirectory(sDirectory);
		if (!NSDirectory::Exists(sDirectory))
			return true;
		NSThreads::Sleep(100);
	}
	return false;
}

static bool RenameDirectory(const std::wstring& sFrom, const std::wstring& sTo)
{
#ifdef _WIN32
	return 0 != MoveFileExW(sFrom.c_str(), sTo.c_str(), 0);
#else
	std::string sFromA = U_TO_UTF8(sFrom);
	std::string sToA = U_TO_UTF8(sTo);
	return 0 == rename(sFromA.c_str(), sToA.c_str());
#endif
}

static void WriteMarker(const std::wstring& sPath)
{
	NSFile::CFileBinary oFile;
	if (oFile.CreateFileW(sPath))
		oFile.CloseFile();
}

// The platform this application is built for, in the build_tools spelling.
// The addon package is chosen for it, and the addon carries native code for it
// alone: the application writes it into the addon it installs (<addon>/platform)
// and drops an addon that names another one - left behind by a build for
// another architecture in the same user profile (x64 replaced by arm64).
static const char* AddonPlatform()
{
#if defined(_WIN32)
# if defined(_M_ARM64)
	return "win_arm64";
# elif defined(_M_X64) || defined(_WIN64)
	return "win_64";
# else
	return "win_32";
# endif
#elif defined(__APPLE__)
# if defined(__aarch64__) || defined(__arm64__)
	return "mac_arm64";
# else
	return "mac_64";
# endif
#else
# if defined(__aarch64__)
	return "linux_arm64";
# elif defined(__x86_64__)
	return "linux_64";
# else
	return "linux_32";
# endif
#endif
}

static std::wstring AddonPlatformFile(const std::wstring& sAddon)
{
	return sAddon + L"/platform";
}

static bool WriteAddonPlatform(const std::wstring& sAddon)
{
	NSFile::CFileBinary oFile;
	if (!oFile.CreateFileW(AddonPlatformFile(sAddon)))
		return false;
	std::string sPlatform = AddonPlatform();
	bool bWritten = oFile.WriteFile((const BYTE*)sPlatform.c_str(), (DWORD)sPlatform.length());
	oFile.CloseFile();
	return bWritten;
}

// No file at all is another platform too: the addon was installed by a build
// that did not write it down, and nothing tells what its native code is for.
static bool IsAddonForThisPlatform(const std::wstring& sAddon)
{
	std::string sPlatform;
	if (!NSFile::CFileBinary::ReadAllTextUtf8A(AddonPlatformFile(sAddon), sPlatform))
		return false;
	while (!sPlatform.empty() && (unsigned char)sPlatform[sPlatform.length() - 1] <= ' ')
		sPlatform.erase(sPlatform.length() - 1);
	return sPlatform == AddonPlatform();
}
#endif

bool CAscApplicationManager_Private::PrepareAddonDirectory()
{
#ifdef USE_ADDON_MODE
	std::wstring sAddon = AddonDirectory(m_pMain->m_oSettings);
	std::wstring sMarker = AddonRemoveMarker(sAddon);
	std::wstring sStaging = AddonStaging(sAddon);

	if (NSFile::CFileBinary::Exists(sMarker))
	{
		if (NSDirectory::Exists(sAddon))
			NSDirectory::DeleteDirectory(sAddon);
		if (!NSDirectory::Exists(sAddon))
			NSFile::CFileBinary::Remove(sMarker);
	}

	if (NSDirectory::Exists(sStaging))
	{
		if (NSDirectory::Exists(sAddon))
			NSDirectory::DeleteDirectory(sAddon);
		if (!NSDirectory::Exists(sAddon))
			RenameDirectory(sStaging, sAddon);
	}

	if (NSDirectory::Exists(sAddon) && !IsAddonForThisPlatform(sAddon))
	{
		NSDirectory::DeleteDirectory(sAddon);
		if (NSDirectory::Exists(sAddon))
			WriteMarker(sMarker);
	}

	return !NSFile::CFileBinary::Exists(sMarker) && !NSDirectory::Exists(sStaging);
#else
	return true;
#endif
}

NSFonts::IApplicationFonts* CAscApplicationManager::GetApplicationFonts()
{
	return m_pInternal->m_pApplicationFonts;
}

void CAscApplicationManager::StartSaveDialog(const std::wstring& sName, unsigned int nId)
{
	// in this implementation it is always a cancel
	// if you want to use it - write a wrapper
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
#if defined(WIN32) || defined(_MAC)
	return true;
#elif defined(_LINUX)
	KeyboardLayout kl;
	return kl.IsKeyboardSupport();
#endif

	return false;
}

int CAscApplicationManager::GetPlatformKeyboardLayout()
{
#if defined(WIN32)
	HWND wFocus = GetFocus();
	DWORD dwThread = GetWindowThreadProcessId(wFocus, 0);
	HKL hkl = GetKeyboardLayout(dwThread);
	int nLang = LOWORD(hkl);
	return nLang;
#elif defined(_MAC)
    uint16_t nLang = GetKeyboardLayout();
    if (nLang != 0)
        return nLang;
#elif defined(_LINUX)
	KeyboardLayout kl;
    uint16_t layout = kl.GetKeyboardLayout();
    if (layout != 0)
        return layout;
#endif

	return -1;
}

void CAscApplicationManager::OnDestroyWindow()
{
	m_pInternal->m_nWindowCounter--;
	if (0 == m_pInternal->m_nWindowCounter)
	{
		CefCookieManager::GetGlobalManager(nullptr)->FlushStore(nullptr);
		m_pInternal->m_bIsExitMessageLoop = true;
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
	else if (sName == L"md")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_MD;
	else if (sName == L"html")
		nFormat = AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML;
	else if (sName == L"xlsx")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
	else if (sName == L"xltx")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX;
	else if (sName == L"xls")
		nFormat = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS;
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

void CAscApplicationManager::SetEventToAllWindows(NSEditorApi::CAscMenuEvent* pEvent)
{
	m_pInternal->SetEventToAllWindows(pEvent);
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

	// do not change the mode for the already opened ones
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

	oPlugins.CheckInstalledPlugins();
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
	if (dpix)
		*dpix = (unsigned int)(dScale * 96);
	if (dpiy)
		*dpiy = (unsigned int)(dScale * 96);
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
	if (!m_pInternal->m_arNativePluginsDirs.empty())
	{
		const std::vector<std::wstring>& arNP = m_pInternal->m_arNativePluginsDirs;
		props.push_back("native-plugins-dir-count=" + std::to_string(arNP.size()));
		for (size_t i = 0; i < arNP.size(); ++i)
			props.push_back("native-plugins-dir-" + std::to_string(i) + "=" + U_TO_UTF8(arNP[i]));
	}
	props.push_back("cookie_path=" + U_TO_UTF8(m_oSettings.cookie_path));
	props.push_back("app_data_path=" + U_TO_UTF8(m_oSettings.app_data_path));
	props.push_back(m_pInternal->m_bDebugInfoSupport ? "debug_mode=true" : "debug_mode=false");
	props.push_back("fonts_cache_path=" + U_TO_UTF8(m_oSettings.fonts_cache_info_path));
	props.push_back("tmp_folder=" + U_TO_UTF8(m_pInternal->StartTmpDirectory()));
	props.push_back("recovers_folder=" + U_TO_UTF8(m_oSettings.recover_path));
	props.push_back("renderer_process_variable=" + U_TO_UTF8(m_pInternal->m_sRendererJSON));
	props.push_back("renderer_process_variable=" + U_TO_UTF8(m_pInternal->m_sRendererJSON));

	if (m_pInternal->m_bDisableAI)
		props.push_back("disable-ai=1");

	if (!m_oSettings.macroses_support)
		props.push_back("macroses_support=false");
	if (!m_oSettings.plugins_support)
		props.push_back("plugins_support=false");

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

void CAscApplicationManager::UpdatePlugins(const std::wstring& json)
{
	CCefView* pMainView = m_pInternal->GetViewForSystemMessages();
	if (!pMainView)
		return;

	std::string value = U_TO_UTF8(json);
	std::string sCode = "(function(){window.on_update_plugin_info && window.on_update_plugin_info(" + value + ");})();";
	pMainView->ExecuteInAllFrames(sCode, false);
}

void CAscApplicationManager::SetRecentPin(const int& nId, const bool& bIsPin)
{
	m_pInternal->Recents_Pin(nId, bIsPin);
}

std::wstring CAscApplicationManager::GetExternalSchemeName()
{
	if (m_pInternal->m_pAdditional)
		return m_pInternal->m_pAdditional->GetExternalSchemeName();
	return L"oo-office";
}

std::string CAscApplicationManager::GetErrorPageAddon(const ErrorPageType& type)
{
	std::string sAddon = "";
	switch (type)
	{
	case ErrorPageType::Network:
		sAddon = "?page=cloudfile";
		break;
	case ErrorPageType::Crash:
		sAddon = "?page=fileerr";
		break;
	default:
		break;
	}

	return sAddon;
}

bool CAscApplicationManager::InstallPluginFromStore(const std::wstring& sName)
{
	CPluginsManager oPlugins;
	oPlugins.m_strDirectory = m_oSettings.system_plugins_path;
	oPlugins.m_strUserDirectory = m_oSettings.user_plugins_path;

	if (!oPlugins.InstallPluginFromStore(sName))
		return false;

	for (std::map<int, CCefView*>::iterator iterView = m_pInternal->m_mapViews.begin(); iterView != m_pInternal->m_mapViews.end(); iterView++)
	{
		CCefView* pView = iterView->second;
		if (pView->GetType() != cvwtEditor)
			continue;

		CCefViewEditor* pEditor = (CCefViewEditor*)pView;
		pEditor->UpdatePlugins();
	}

	return true;
}

bool CAscApplicationManager::RemoveRecentByViewId(const int& nId)
{
	CCefView* pView = GetViewById(nId);
	if (!pView)
		return false;

	int nRecentId = pView->GetRecentId();
	if (-1 == nRecentId)
		return false;

	m_pInternal->Recents_Remove(nRecentId);
	return true;
}

int CAscApplicationManager::CallCommand(const std::wstring& command)
{
	return 0;
}

int CAscApplicationManager::InstallMainAddon(const std::wstring& path)
{
#ifndef USE_ADDON_MODE
	return 1;
#else
	std::wstring sAddon = AddonDirectory(m_oSettings);
	std::wstring sStaging = AddonStaging(sAddon);

	// The package is unpacked beside the installed addon and checked there, so
	// a package that does not fit leaves the installed addon as it was.
	if (NSDirectory::Exists(sStaging))
		NSDirectory::DeleteDirectory(sStaging);
	NSDirectory::CreateDirectory(sStaging);
	COfficeUtils oCOfficeUtils(NULL);
	// the addon brings native code: executables need their permissions and macOS frameworks their symlinks
	HRESULT hr = oCOfficeUtils.ExtractToDirectory(path, sStaging, NULL, 0, true, true);

	bool bFits = false;
	std::string sAddonVersion;
	std::wstring sPath = sStaging + L"/sdkjs/word/sdk-all-min.js";
	if (hr == S_OK && NSFile::CFileBinary::Exists(sPath))
	{
		sAddonVersion = m_pInternal->ReadSdkJSVersionLine(sPath);
		bFits = (sAddonVersion == m_pInternal->m_sSdkJSVersion);
	}
	// an addon that does not name its platform is dropped at the next start
	if (!bFits || !WriteAddonPlatform(sStaging))
	{
		NSDirectory::DeleteDirectory(sStaging);
		return 1;
	}

	// It fits: the installed addon goes. If it cannot go yet, the new one waits
	// beside it and takes its place at the next start (PrepareAddonDirectory).
	if (NSDirectory::Exists(sAddon))
	{
		m_pInternal->StopNativeCodeInDirectory(sAddon);
		if (DeleteDirectoryWithRetries(sAddon))
			RenameDirectory(sStaging, sAddon);
	}
	else
	{
		RenameDirectory(sStaging, sAddon);
	}
	// a removal still pending is superseded by the new addon
	NSFile::CFileBinary::Remove(AddonRemoveMarker(sAddon));

	m_pInternal->m_bIsInstalledAddon = true;
	m_pInternal->m_bIsAddonCompatible = true;
	m_pInternal->m_sAddonVersion = sAddonVersion;
	return 0;
#endif
}
int CAscApplicationManager::RemoveMainAddon()
{
#ifndef USE_ADDON_MODE
	return 1;
#else
	std::wstring sAddon = AddonDirectory(m_oSettings);
	std::wstring sStaging = AddonStaging(sAddon);

	// an installed addon still waiting for the next start goes too
	if (NSDirectory::Exists(sStaging))
		NSDirectory::DeleteDirectory(sStaging);

	if (NSDirectory::Exists(sAddon))
	{
		m_pInternal->StopNativeCodeInDirectory(sAddon);
		// what is still held is deleted at the next start (PrepareAddonDirectory)
		if (!DeleteDirectoryWithRetries(sAddon))
			WriteMarker(AddonRemoveMarker(sAddon));
	}

	m_pInternal->m_bIsInstalledAddon = false;
	m_pInternal->m_bIsAddonCompatible = false;
	m_pInternal->m_sAddonVersion.clear();
	return 0;
#endif
}
bool CAscApplicationManager::CheckMainAddon(std::string* version) const
{
#ifndef USE_ADDON_MODE
	return false;
#else
	if (!m_pInternal->m_bIsInstalledAddon)
		return false;

	if (m_pInternal->m_bIsAddonCompatible)
		return true;

	if (version)
	{
		version->clear();

		// "Version: 9.5.0 (build:132)" -> "9.5.0.132"
		// Every non-digit is a separator; runs of separators collapse into one dot.
		const std::string& sRaw = m_pInternal->m_sAddonVersion;
		std::string sResult;

		sResult.reserve(sRaw.length());
		size_t nParts = 0;
		bool bSeparator = true;
		for (const char c : sRaw)
		{
			if (c < '0' || c > '9')
			{
				bSeparator = true;
				continue;
			}
			if (bSeparator)
			{
				if (++nParts > 4)
					break;
				if (!sResult.empty())
					sResult += '.';
				bSeparator = false;
			}
			sResult += c;
		}
		*version = sResult;
	}

	return m_pInternal->m_bIsAddonCompatible;
#endif
}

bool NSCommon::CSystemWindowScale::g_isUseSystemScalingInit = false;
bool NSCommon::CSystemWindowScale::g_isUseSystemScaling = false;

bool CAscApplicationManager::IsUseSystemScaling()
{
	return NSCommon::CSystemWindowScale::IsUseSystemScaling();
}
double Core_GetMonitorScale(const unsigned int& xDpi, const unsigned int& yDpi)
{
	// allowed values: 1; 1.25; 1.5; 1.75; 2; 2.25, 2.5; 2.75, 3; 3.5; 4; 4.5; 5;
	double dScale = (xDpi + yDpi) / (2 * 96.0);
	int nCount = (int)((dScale + 0.125) / 0.25);
	dScale = 0.25 * nCount;
	if (dScale > 5)
		dScale = 5;
	if (dScale < 1)
		dScale = 1;

	if (dScale > 3)
		dScale = 0.5 * ((int)(2 * dScale));

	return dScale;
}
