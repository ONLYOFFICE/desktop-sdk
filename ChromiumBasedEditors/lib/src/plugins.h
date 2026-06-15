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

#ifndef APPLICATION_PLUGINS_H
#define APPLICATION_PLUGINS_H

#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/OfficeUtils/src/OfficeUtils.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "./utils.h"
#include <sstream>

//#include "./plugins_resources.h"
#include <map>
#include <set>

#ifdef GetTempPath
#undef GetTempPath
#endif

class CExternalPluginInfo
{
public:
	std::string sGuid;
	std::string sName;
	std::string sNameObject;
	std::string sConfigName;
	std::string sUrl;
	bool isOnlyofficeScheme;
	bool isUser;
	int nType;
};

class CPluginsManager
{
public:
	bool m_isSupportSystemUpdate;

	std::wstring m_strDirectory;
	std::wstring m_strUserDirectory;
	std::wstring m_sAddonsDirectory;

	int m_nCryptoMode;
	std::map<int, std::string> m_arCryptoModes;

	bool m_bCryptoDisabledOnStart;
	bool m_bCryptoDisableForLocal;
	bool m_bCryptoDisableForInternalCloud;
	bool m_bCryptoDisableForExternalCloud;

	std::string m_strCryptoPluginAttack;

	// a plugin not for the editor, but for the main page (for system messages)
	std::vector<CExternalPluginInfo> m_arExternals;
	bool m_bIsSupportMultiplugins;

public:
	CPluginsManager()
	{
		m_nCryptoMode = 0;

		m_bCryptoDisabledOnStart = false;
		m_bCryptoDisableForLocal = false;
		m_bCryptoDisableForInternalCloud = false;
		m_bCryptoDisableForExternalCloud = false;

		// Ability to pseudo-update system plugins
		// In effect, replacing a system plugin with a newer one from the user folder
		m_isSupportSystemUpdate = true;

		m_bIsSupportMultiplugins = false;
	}

	static std::set<std::string>& GetDisableGUIDS()
	{
		static std::set<std::string> s = {};
		return s;
	}

	static void DisableAI()
	{
		std::set<std::string>& guids = GetDisableGUIDS();
		guids.insert("{9DC93CDB-B576-4F0C-B55E-FCC9C48DD007}");
		guids.insert("{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}");
	}

	std::string GetPluginsJson(const bool& bActivePlugins = true, const bool& bCheckCrypto = false,
							   const bool& bIsSupportMacroses = true, const bool& bIsSupportPlugins = true)
	{
		if (!NSDirectory::Exists(m_strUserDirectory))
			NSDirectory::CreateDirectory(m_strUserDirectory);

		std::string sPluginsJSON = "[";

		if (bActivePlugins)
		{
			std::string sPlugins = ParsePluginDir(L"", bCheckCrypto, false, bIsSupportMacroses, bIsSupportPlugins);
			sPluginsJSON += sPlugins;
		}
		else
		{
			// Search for plugins in the user folder for possible restoration
			std::wstring sBackupDir = m_strUserDirectory + L"/backup";
			if (NSDirectory::Exists(sBackupDir))
			{
				std::string sPlugins = ParsePluginDir(sBackupDir, bCheckCrypto, true);

				sPluginsJSON += sPlugins;
			}
		}

		sPluginsJSON += "]";

		return sPluginsJSON;
	}

	void CheckInstalledPlugins()
	{
		ParsePluginDir(L"", true, false, true, true, true);
	}

	bool AddPlugin(const std::wstring& sFilePlugin)
	{
		if (!NSDirectory::Exists(m_strUserDirectory))
			NSDirectory::CreateDirectory(m_strUserDirectory);

		std::wstring sTemp = m_strUserDirectory + L"/temp_asc_plugin_directory";
		std::wstring sTempExt = sTemp;
		std::wstring sPluginName = L"";

		if (NSDirectory::Exists(sTemp))
			NSDirectory::DeleteDirectory(sTemp);
		NSDirectory::CreateDirectory(sTemp);

		COfficeUtils oOfficeUtils(NULL);
		if (S_OK == oOfficeUtils.ExtractToDirectory(sFilePlugin, sTempExt, NULL, 0))
		{
			std::wstring sConfig = sTempExt + L"/config.json";

			// zip with subfolder
			if (!NSFile::CFileBinary::Exists(sConfig))
			{
				std::vector<std::wstring> aDirs = NSDirectory::GetDirectories(sTempExt);
				if (aDirs.size() == 1)
				{
					sTempExt = aDirs[0];
					sConfig = sTempExt + L"/config.json";
				}
			}

			std::string sJson = "";
			if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfig, sJson))
			{
				std::string::size_type pos1 = sJson.find("\"name\"");
				std::string::size_type pos2 = sJson.find("\"guid\"");
				std::string::size_type pos3 = sJson.find("\"variations\"");

				if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos)
				{
					std::string::size_type pos4 = sJson.find('{', pos2);
					std::string::size_type pos5 = sJson.find('}', pos2);
					if (pos4 != std::string::npos && pos5 != std::string::npos)
					{
						std::string sPluginNameA = sJson.substr(pos4, pos5 - pos4 + 1);
						sPluginName = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sPluginNameA.c_str(), (LONG)sPluginNameA.length());
					}
				}
			}
		}

		if (sPluginName.empty())
			return false;

		std::wstring sNew = m_strUserDirectory + L"/" + sPluginName;
		NSDirectory::DeleteDirectory(sNew);
		NSDirectory::CreateDirectory(sNew);

		NSDirectory::CopyDirectory(sTempExt, sNew);
		NSDirectory::DeleteDirectory(sTemp);

		// Change the links to v1
		ReplaceHtmlUrls(sNew);

		return true;
	}

	bool RestorePlugin(const std::wstring& sGuid)
	{
		bool bResult = false;

		std::wstring sAdd = sGuid;
		std::wstring::size_type pos = sGuid.find(L"{");
		if (pos != std::wstring::npos && pos != 0)
			sAdd = sGuid.substr(pos);

		std::wstring sPluginDir = m_strUserDirectory + L"/" + sAdd;
		std::wstring sPluginBackupDir = m_strUserDirectory + L"/backup/" + sAdd;

		if (NSDirectory::Exists(sPluginBackupDir))
		{
			NSDirectory::CopyDirectory(sPluginBackupDir, sPluginDir);
			NSDirectory::DeleteDirectory(sPluginBackupDir);

			// Change the links to v1
			ReplaceHtmlUrls(sPluginDir);

			bResult = true;
		}

		return bResult;
	}

	bool RemovePlugin(const std::wstring& sGuid, const bool& bBackup)
	{
		bool bResult = false;

		std::wstring sAdd = sGuid;
		std::wstring::size_type pos = sGuid.find(L"{");
		if (pos != std::wstring::npos && pos != 0)
			sAdd = sGuid.substr(pos);

		std::wstring sPluginDir = m_strUserDirectory + L"/" + sAdd;

		if (NSDirectory::Exists(sPluginDir))
		{
			if (bBackup)
			{
				std::wstring sBackupDir = m_strUserDirectory + L"/backup";
				std::wstring sPluginBackupDir = sBackupDir + L"/" + sAdd;

				if (!NSDirectory::Exists(sBackupDir))
					NSDirectory::CreateDirectory(sBackupDir);

				if (!NSDirectory::Exists(sPluginBackupDir))
					NSDirectory::DeleteDirectory(sPluginBackupDir);

				NSDirectory::CopyDirectory(sPluginDir, sPluginBackupDir);
			}

			NSDirectory::DeleteDirectory(sPluginDir);
			bResult = true;
		}

		return bResult;
	}

	bool InstallPluginFromStore(const std::wstring& name)
	{
		std::wstring sPackageUrl = L"https://github.com/ONLYOFFICE/onlyoffice.github.io/releases/latest/download/" + name + L".plugin";

		std::wstring sTmpFile = NSDirectory::GetTempPath() + L"/temp_asc_plugin.plugin";
		if (NSFile::CFileBinary::Exists(sTmpFile))
			NSFile::CFileBinary::Remove(sTmpFile);

		CFileDownloaderWrapper oDownloader(sPackageUrl, sTmpFile);
		oDownloader.DownloadSync();

		bool bResult = false;
		if (NSFile::CFileBinary::Exists(sTmpFile))
		{
			bResult = AddPlugin(sTmpFile);
			NSFile::CFileBinary::Remove(sTmpFile);
		}

		return bResult;
	}

private:
	void ReplaceInFile(const std::wstring& sFile, const std::wstring& sText, const std::wstring& sReplace)
	{
		std::wstring sContent;
		if (NSFile::CFileBinary::ReadAllTextUtf8(sFile, sContent))
		{
			NSStringUtils::string_replace(sContent, sText, sReplace);
			NSFile::CFileBinary::Remove(sFile);

			NSFile::CFileBinary oFile;
			oFile.CreateFileW(sFile);
			oFile.WriteStringUTF8(sContent);
			oFile.CloseFile();
		}
	}

	void ReplaceHtmlUrls(const std::wstring& sPluginDir)
	{
		std::vector<std::wstring> arFiles = NSDirectory::GetFiles(sPluginDir, true);
		for (size_t i = 0; i < arFiles.size(); i++)
		{
			std::wstring sPath = arFiles[i];
			std::wstring sExt = NSFile::GetFileExtention(sPath);

			if (sExt == L"html")
			{
				// Compute the folder depth relative to the local v1 (UserDir / PluginName / N / *.html)
				std::wstring _sPath = sPath;
				NSStringUtils::string_replace(_sPath, m_strUserDirectory, L"");
				NSStringUtils::string_replace(_sPath, L"\\", L"/");

				std::wstring sLayers = L"";
				for (size_t j = 1; j < _sPath.length(); j++)
				{
					if (_sPath[j] == L'/')
						sLayers += L"../";
				}
				ReplaceInFile(sPath, L"https://onlyoffice.github.io/sdkjs-plugins/", sLayers);
			}
		}
	}

	std::vector<std::string> GetDirPlugins(std::wstring sDir)
	{
		// System plugins can also be updated; in that case they will be in the user folder
		// We need to find the overlaps and remove the outdated ones from the system list
		// TODO: just a test for now, but this class should be refactored — build the plugin lists in the constructor, after the folders are set

		std::vector<std::string> arPlugins;
		std::vector<std::wstring> arDir = NSDirectory::GetDirectories(sDir);

		std::set<std::string>& disabledPlugins = CPluginsManager::GetDisableGUIDS();

		for (size_t i = 0; i < arDir.size(); ++i)
		{
			std::string sJson;
			if (NSFile::CFileBinary::ReadAllTextUtf8A(arDir[i] + L"/config.json", sJson))
			{
				std::string::size_type pos1 = sJson.find("asc.{");
				std::string::size_type pos2 = sJson.find('}', pos1);

				if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
				{
					std::string guid = sJson.substr(pos1 + 4, pos2 - pos1 + 1 - 4);
					if (disabledPlugins.find(guid) == disabledPlugins.end())
						arPlugins.push_back(guid);
				}
			}
		}

		return arPlugins;
	}

	int GetIntVersion(const std::string& version)
	{
		int major = 0, minor = 0, patch = 0;
		int* out = &major;
		int count = 0;

		for (size_t i = 0, len = version.length(); i < len; ++i)
		{
			char c = version[i];
			if (c >= '0' && c <= '9')
			{
				*out = (*out * 10) + (c - '0');
			}
			else if (c == '.')
			{
				++count;
				if (count == 1)
					out = &minor;
				else if (count == 2)
					out = &patch;
				else
					break;
			}
			else
			{
				// skip
			}
		}

		return major * 1000000 + minor * 1000 + patch;

	}

	std::string ParsePluginDir(const std::wstring& sDir, const bool& bCheckCrypto = false, const bool& bIsBackup = false,
							   const bool& bIsSupportMacroses = true, const bool& bIsSupportPlugins = true, const bool& isFullInfos = false)
	{
		std::vector<std::wstring> _arPlugins;
		std::vector<int> _arSources; // 0 - system, 1 - user, 2 - addon

		// By default, the addon plugins folder is located next to the user one.
		// Addon plugins behave exactly like user ones — just in a different folder.
		if (m_sAddonsDirectory.empty())
			m_sAddonsDirectory = m_strUserDirectory + L"/../addon/sdkjs-plugins";

		if (bIsSupportPlugins)
		{
			if (sDir.empty())
			{
				std::vector<std::string> arSysPlugins = GetDirPlugins(m_strDirectory);

				// User plugins are gathered from the user folder and (in addon
				// mode) from the addon folder. They behave identically; when a guid matches in
				// different folders, we keep the newer version.
				std::vector<std::string> arUserPlugins;
				std::map<std::string, std::wstring> mUserPluginsDir;

				auto appendUserPlugins = [this, &arUserPlugins, &mUserPluginsDir](const std::wstring& sFolder)
				{
					if (!NSDirectory::Exists(sFolder))
						return;

					std::vector<std::string> arGuids = GetDirPlugins(sFolder);
					for (std::vector<std::string>::iterator i = arGuids.begin(); i != arGuids.end(); i++)
					{
						std::map<std::string, std::wstring>::iterator it = mUserPluginsDir.find(*i);
						if (it == mUserPluginsDir.end())
						{
							mUserPluginsDir.insert(std::pair<std::string, std::wstring>(*i, sFolder));
							arUserPlugins.push_back(*i);
						}
						else
						{
							std::string sJsonOld, sJsonNew;
							NSFile::CFileBinary::ReadAllTextUtf8A(it->second + L"/" + UTF8_TO_U((*i)) + L"/config.json", sJsonOld);
							NSFile::CFileBinary::ReadAllTextUtf8A(sFolder + L"/" + UTF8_TO_U((*i)) + L"/config.json", sJsonNew);
							if (GetIntVersion(GetStringValue(sJsonNew, "version")) > GetIntVersion(GetStringValue(sJsonOld, "version")))
								it->second = sFolder;
						}
					}
				};

				appendUserPlugins(m_strUserDirectory);
#ifdef USE_ADDON_MODE
				// Only the actual reading of addon plugins is enabled by the define
				appendUserPlugins(m_sAddonsDirectory);
#endif

				if (!bIsSupportMacroses)
				{
					const std::string sMacrosesGuid = "{E6978D28-0441-4BD7-8346-82FAD68BCA3B}";
					for (std::vector<std::string>::iterator i = arSysPlugins.begin(); i != arSysPlugins.end(); i++)
					{
						if (*i == sMacrosesGuid)
						{
							arSysPlugins.erase(i);
							break;
						}
					}
					for (std::vector<std::string>::iterator i = arUserPlugins.begin(); i != arUserPlugins.end(); i++)
					{
						if (*i == sMacrosesGuid)
						{
							mUserPluginsDir.erase(*i);
							arUserPlugins.erase(i);
							break;
						}
					}
				}

				for (std::vector<std::string>::iterator i = arSysPlugins.begin(); i != arSysPlugins.end(); i++)
				{
					std::string sGuidSys = *i;
					bool bIsAdd = true;
					for (std::vector<std::string>::iterator j = arUserPlugins.begin(); j != arUserPlugins.end(); j++)
					{
						std::string sGuidUser = *j;
						if (sGuidSys == sGuidUser)
						{
							std::string sJSONSys;
							NSFile::CFileBinary::ReadAllTextUtf8A(m_strDirectory + L"/" + UTF8_TO_U(sGuidSys) + L"/config.json", sJSONSys);

							std::string sJSONUser;
							NSFile::CFileBinary::ReadAllTextUtf8A(mUserPluginsDir[sGuidUser] + L"/" + UTF8_TO_U(sGuidUser) + L"/config.json", sJSONUser);

							std::string sVersionSys = GetStringValue(sJSONSys, "version");
							std::string sVersionUser = GetStringValue(sJSONUser, "version");

							int nVersionSys = GetIntVersion(sVersionSys);
							int nVersionUser = GetIntVersion(sVersionUser);

							if (m_isSupportSystemUpdate && nVersionSys < nVersionUser)
							{
								bIsAdd = false;
							}
							else
							{
								mUserPluginsDir.erase(sGuidUser);
								arUserPlugins.erase(j);
							}
							break;
						}
					}
					if (bIsAdd)
					{
						_arPlugins.push_back(m_strDirectory + L"/" + UTF8_TO_U(sGuidSys));
						_arSources.push_back(0);
					}
				}

				for (std::vector<std::string>::iterator j = arUserPlugins.begin(); j != arUserPlugins.end(); j++)
				{
					const std::wstring& sFolder = mUserPluginsDir[*j];
					_arPlugins.push_back(sFolder + L"/" + UTF8_TO_U((*j)));
					_arSources.push_back(sFolder == m_sAddonsDirectory ? 2 : 1);
				}
			}
			else
			{
				_arPlugins = NSDirectory::GetDirectories(sDir);
				for (size_t i = 0; i < _arPlugins.size(); ++i)
					_arSources.push_back(0);
			}
		}

		std::vector<std::string> arSystem;
		std::vector<std::string> arUser;
		std::vector<std::string> arAddon;

		for (size_t i = 0; i < _arPlugins.size(); ++i)
		{
			std::string sJson = "";
			if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.json", sJson))
			{
				int nSource = _arSources[i];
				bool isSystem = (0 == nSource);
				bool isAddon = (2 == nSource);
				if (isFullInfos)
				{
					CheckEncryption(sJson, false);
					bool bIsExternal = CheckExternal(sJson, isSystem, nSource);

					// check for desktop
					if (m_bIsSupportMultiplugins && !bIsExternal && NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/configDesktop.json", sJson))
					{
						CheckExternal(sJson, isSystem, nSource);
					}
				}
				else
				{
					// Get the GUID
					std::string sGuid = "";
					std::string::size_type pos1 = sJson.find("asc.{");
					std::string::size_type pos2 = sJson.find('}', pos1);

					if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
					{
						sGuid = sJson.substr(pos1, pos2 - pos1 + 1);
					}

					if (!CheckEncryption(sJson, bCheckCrypto))
						continue;

					// !!! this needs to be discussed, since such plugins may exist in the user folder
					if (CheckExternal(sJson, i == 0, nSource))
						continue;

					pos1 = sJson.find('{');
					pos2 = sJson.find_last_of('}');

					if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
					{
						std::string sConfigContent = sJson.substr(pos1, pos2 - pos1 + 1);

						if (bIsBackup)
						{
							// set the property for further actions
							pos2 = sConfigContent.find_last_of('}');
							sConfigContent = sConfigContent.substr(0, pos2) + ", \"backup\": true }";
						}

						if (isAddon)
							arAddon.push_back(sConfigContent);
						else if (isSystem)
							arSystem.push_back(sConfigContent);
						else
							arUser.push_back(sConfigContent);
					}
				}
			}
		}

		auto serializePlugins = [](const std::wstring& dir, const std::vector<std::string>& data, const std::string& sType) {
			std::wstring ret = dir + L"/";
			if (0 == ret.find('/'))
				ret = L"file://" + ret;
			else
				ret = L"file:///" + ret;

			// "type" (system / user / addon) tells which plugin comes from which folder
			std::string serializeData = "{ \"url\" : \"" + U_TO_UTF8(ret) + "\", \"type\" : \"" + sType + "\", \"pluginsData\" : [";

			int nCount = 0;
			for (std::vector<std::string>::const_iterator i = data.cbegin(); i != data.cend(); i++)
			{
				if (nCount > 0)
					serializeData += ",";

				++nCount;
				serializeData += *i;
			}

			serializeData += "] }";

			return serializeData;
		};

		std::string sPlugins;
		if (sDir.empty())
		{
			sPlugins = serializePlugins(m_strDirectory, arSystem, "system") + "," + serializePlugins(m_strUserDirectory, arUser, "user");
			if (!arAddon.empty())
				sPlugins += "," + serializePlugins(m_sAddonsDirectory, arAddon, "addon");
		}
		else
		{
			sPlugins = serializePlugins(sDir, arSystem, "user");
		}

		NSStringUtils::string_replaceA(sPlugins, "\r\n", "");
		NSStringUtils::string_replaceA(sPlugins, "\n", "");

		return sPlugins;
	}

	bool CheckEncryption(const std::string& strJson, const bool& checkCrypto)
	{
		if ("desktop" == GetStringValue(strJson, "initDataType") && "encryption" == GetStringValue(strJson, "initData"))
		{
			int nMode = 1;
			std::string sMode = GetStringValue(strJson, "cryptoMode");
			std::string sModeExt = GetStringValue(strJson, "cryptoModeExt");
			if (!sMode.empty())
				nMode = std::stoi(sMode);

			if (checkCrypto && sModeExt == "disable" && m_strCryptoPluginAttack.empty())
				return false;

			std::string sGuid = GetStringValue(strJson, "guid");

			if (checkCrypto)
			{
				if (!m_strCryptoPluginAttack.empty() )
				{
					std::string s1 = sGuid;
					NSStringUtils::string_replaceA(s1, "asc.", "");
					std::string s2 = m_strCryptoPluginAttack;
					NSStringUtils::string_replaceA(s2, "asc.", "");

					if (s1 == s2)
						return true;

					return false;
				}

				if (m_nCryptoMode != nMode)
				{
					return false;
				}
			}

			if (m_arCryptoModes.find(nMode) == m_arCryptoModes.end())
				m_arCryptoModes.insert(std::pair<int, std::string>(nMode, sGuid));
			else if (checkCrypto)
				return false;

		}
		return true;
	}

	bool CheckExternal(const std::string& sJson, bool isSystem, const int& nSource)
	{
		if ("desktop-external" == GetStringValue(sJson, "initDataType"))
		{
			std::string sStartMode = GetStringValue(sJson, "cryptoDisabledOnStart");
			if (sStartMode == "true" || sStartMode == "1")
				m_bCryptoDisabledOnStart = true;

			std::string sCryptoDisableForLocal = GetStringValue(sJson, "cryptoDisabledForLocal");
			if (sCryptoDisableForLocal == "true" || sCryptoDisableForLocal == "1") m_bCryptoDisableForLocal = true;

			std::string sCryptoDisableForInternalCloud = GetStringValue(sJson, "cryptoDisabledForInternalCloud");
			if (sCryptoDisableForInternalCloud == "true" || sCryptoDisableForInternalCloud == "1") m_bCryptoDisableForInternalCloud = true;

			std::string sCryptoDisableForExternalCloud = GetStringValue(sJson, "cryptoDisabledForExternalCloud");
			if (sCryptoDisableForExternalCloud == "true" || sCryptoDisableForExternalCloud == "1") m_bCryptoDisableForExternalCloud = true;

			std::string::size_type pos1 = sJson.find("asc.{");
			std::string::size_type pos2 = sJson.find('}', pos1);

			if (pos1 != std::string::npos &&
					pos2 != std::string::npos &&
					pos2 > pos1)
			{
				CExternalPluginInfo info;
				info.sGuid = sJson.substr(pos1, pos2 - pos1 + 1);
				info.sName = GetStringValue(sJson, "name");
				info.sNameObject = GetObjectValue(sJson, "nameLocale");
				info.isUser = isSystem ? false : true;
				info.sUrl = GetStringValue(sJson, "url");
				info.isOnlyofficeScheme = (std::string::npos != sJson.find("onlyofficeScheme")) ? true : false;
				info.nType = nSource;

				m_arExternals.push_back(info);
			}
			return true;
		}
		return false;
	}

public:
	static std::string GetStringValue(const std::string& strJson, const std::string& strName)
	{
		std::string::size_type nPosStartName = strJson.find("\"" + strName + "\"");
		if (nPosStartName == std::string::npos)
			return "";

		nPosStartName += (strName.length() + 2);
		std::string::size_type pos1 = strJson.find("\"", nPosStartName);
		if (pos1 == std::string::npos)
			return "";

		std::string::size_type pos2 = strJson.find("\"", pos1 + 1);
		if (pos2 == std::string::npos)
			return "";

		return strJson.substr(pos1 + 1, pos2 - pos1 - 1);
	}
	static std::string GetArrayValue(const std::string& strJson, const std::string& strName)
	{
		std::string::size_type nPosStartName = strJson.find("\"" + strName + "\"");
		if (nPosStartName == std::string::npos)
			return "";

		nPosStartName += (strName.length() + 2);
		std::string::size_type pos1 = strJson.find("[", nPosStartName);
		if (pos1 == std::string::npos)
			return "";

		std::string::size_type pos2 = strJson.find("]", pos1 + 1);
		if (pos2 == std::string::npos)
			return "";

		return strJson.substr(pos1, pos2 - pos1 + 1);
	}
	static std::string GetObjectValue(const std::string& strJson, const std::string& strName)
	{
		std::string::size_type nPosStartName = strJson.find("\"" + strName + "\"");
		if (nPosStartName == std::string::npos)
			return "";

		nPosStartName += (strName.length() + 2);
		std::string::size_type pos1 = strJson.find("{", nPosStartName);
		if (pos1 == std::string::npos)
			return "";

		std::string::size_type pos2 = strJson.find("}", pos1 + 1);
		if (pos2 == std::string::npos)
			return "";

		return strJson.substr(pos1, pos2 - pos1 + 1);
	}
	static std::wstring GetStringValueW(const std::string& strJson, const std::string& strName)
	{
		std::string sRet = GetStringValue(strJson, strName);
		return UTF8_TO_U(sRet);
	}
	static std::wstring GetObjectValueW(const std::string& strJson, const std::string& strName)
	{
		std::string sRet = GetObjectValue(strJson, strName);
		return UTF8_TO_U(sRet);
	}
};

#endif // APPLICATION_PLUGINS_H
