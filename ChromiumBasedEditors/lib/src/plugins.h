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

#ifndef APPLICATION_PLUGINS_H
#define APPLICATION_PLUGINS_H

#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/OfficeUtils/src/OfficeUtils.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "./utils.h"

//#include "./plugins_resources.h"
#include <map>

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
};

class CPluginsManager
{
public:
	bool m_isSupportSystemUpdate;

	std::wstring m_strDirectory;
	std::wstring m_strUserDirectory;

	int m_nCryptoMode;
	std::map<int, std::string> m_arCryptoModes;

	bool m_bCryptoDisabledOnStart;
	bool m_bCryptoDisableForLocal;
	bool m_bCryptoDisableForInternalCloud;
	bool m_bCryptoDisableForExternalCloud;

	std::string m_strCryptoPluginAttack;

	// плагин не для редактора, а для главной страницы (для системных сообщенией)
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

		// Возможность псевдо-обновления системных плагинов
		// По факту подмена системного на более новый из папки пользователя
		m_isSupportSystemUpdate = true;

		m_bIsSupportMultiplugins = false;
	}

	std::string GetPluginsJson(const bool& bActivePlugins = true, const bool& bCheckCrypto = false,
							   const bool& bIsSupportMacroses = true, const bool& bIsSupportPlugins = true)
	{
		if (!NSDirectory::Exists(m_strUserDirectory))
			NSDirectory::CreateDirectory(m_strUserDirectory);

		std::string sPluginsJSON = "[";

		if (bActivePlugins)
		{
			for (int i = 0; i < 2; i++)
			{
				std::wstring sDir = (i == 0) ? m_strDirectory : m_strUserDirectory;

				std::string sPlugins = ParsePluginDir(sDir, bCheckCrypto, false, bIsSupportMacroses, bIsSupportPlugins);

				sPluginsJSON += sPlugins;

				if (i == 0)
					sPluginsJSON += ", ";
			}
		}
		else
		{
			// Поиск плагинов в папке пользователя для возможного восстановления
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

	std::vector<std::string> GetInstalledPlugins()
	{
		std::vector<std::string> arCongigs;
		std::vector<std::string> arUserPlugins = GetDirPlugins(m_strUserDirectory);

		for (int i = 0; i < 2; i++)
		{
			std::wstring sDir = (i == 0) ? m_strDirectory : m_strUserDirectory;
			std::vector<std::wstring> _arPlugins = NSDirectory::GetDirectories(sDir);

			for (size_t j = 0; j < _arPlugins.size(); j++)
			{
				std::string sJson;
				bool bIsExternal = false;
				if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[j] + L"/config.json", sJson))
				{
					CheckEncryption(sJson, false);

					// !!! это надо обсудить, т.к. возможны такие плагины в папке пользователя
					bIsExternal = CheckExternal(sJson, i == 0);

					std::string::size_type pos1 = sJson.find("asc.{");
					std::string::size_type pos2 = sJson.find('}', pos1);

					if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
					{
						std::string sGuid = sJson.substr(pos1, pos2 - pos1 + 1);

						// Пропускаем обновлённые системные плагины
						if ( m_isSupportSystemUpdate && sDir == m_strDirectory &&
							 std::find(arUserPlugins.begin(), arUserPlugins.end(), sGuid) == arUserPlugins.end() )
						{
							arCongigs.push_back(sGuid);
						}
					}
				}

				// check for desktop
				if (m_bIsSupportMultiplugins && !bIsExternal && NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[j] + L"/configDesktop.json", sJson))
				{
					CheckExternal(sJson, i == 0);
				}
			}
		}

		return arCongigs;
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

		// Изменить ссылки на v1
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

			// Изменить ссылки на v1
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
				// Считаем уровень папки относитьльно локальной v1 (UserDir / PluginName / N / *.html)
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
		// Системные плагины могут быть тоже обновлены, они будут в папке пользователя
		// Нужно найти пересечения и удалить старые из списка системных
		// TODO: пока тест, но надо отрефакторить этот класс, получать списки плагинов в конструкторе, после задания папок

		std::vector<std::string> arPlugins;
		std::vector<std::wstring> arDir = NSDirectory::GetDirectories(sDir);

		for (size_t i = 0; i < arDir.size(); ++i)
		{
			std::string sJson;
			if (NSFile::CFileBinary::ReadAllTextUtf8A(arDir[i] + L"/config.json", sJson))
			{
				std::string::size_type pos1 = sJson.find("asc.{");
				std::string::size_type pos2 = sJson.find('}', pos1);

				if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
				{
					arPlugins.push_back(sJson.substr(pos1, pos2 - pos1 + 1));
				}
			}
		}

		return arPlugins;
	}

	std::string ParsePluginDir(const std::wstring& sDir, const bool& bCheckCrypto = false, const bool& bIsBackup = false,
							   const bool& bIsSupportMacroses = true, const bool& bIsSupportPlugins = true)
	{
		std::wstring sParam = sDir + L"/";
		if (0 == sParam.find('/'))
			sParam = L"file://" + sParam;
		else
			sParam = L"file:///" + sParam;

		std::string sPlugins = "{ \"url\" : \"" + U_TO_UTF8(sParam) + "\", \"pluginsData\" : [";
		std::string sData = "";
		int nCountPlugins = 0;

		std::vector<std::wstring> _arPlugins;
		if (bIsSupportPlugins)
			_arPlugins = NSDirectory::GetDirectories(sDir);

		std::vector<std::string> arSysPlugins = GetDirPlugins(m_strDirectory);
		std::vector<std::string> arUserPlugins = GetDirPlugins(m_strUserDirectory);

		for (size_t i = 0; i < _arPlugins.size(); ++i)
		{
			if (!bIsSupportMacroses && (std::wstring::npos != _arPlugins[i].find(L"{E6978D28-0441-4BD7-8346-82FAD68BCA3B}")))
				continue;

			std::string sJson = "";
			if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.json", sJson))
			{
				// Получаем GUID
				std::string sGuid = "";
				std::string::size_type pos1 = sJson.find("asc.{");
				std::string::size_type pos2 = sJson.find('}', pos1);

				if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
				{
					sGuid = sJson.substr(pos1, pos2 - pos1 + 1);

					// Пропускаем обновлённые системные плагины
					if ( m_isSupportSystemUpdate && sDir == m_strDirectory )
					{
						if ( std::find(arUserPlugins.begin(), arUserPlugins.end(), sGuid) != arUserPlugins.end() )
							continue;
					}
				}

				if (!CheckEncryption(sJson, bCheckCrypto))
					continue;

				// !!! это надо обсудить, т.к. возможны такие плагины в папке пользователя
				if (CheckExternal(sJson, i == 0))
					continue;

				pos1 = sJson.find('{');
				pos2 = sJson.find_last_of('}');

				if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
				{
					if (nCountPlugins > 0)
						sData += ",";

					nCountPlugins++;
					std::string sConfigContent = sJson.substr(pos1, pos2 - pos1 + 1);

					// Если это обновленный системный плагин в папке пользователя,
					// то нужно запретить его удаление как и было до обновления,
					// выставляем свойство и парсим в pluginMethod_GetInstalledPlugins
					if ( m_isSupportSystemUpdate && sDir == m_strUserDirectory )
					{
						if ( std::find(arSysPlugins.begin(), arSysPlugins.end(), sGuid) != arSysPlugins.end() )
						{
							pos2 = sConfigContent.find_last_of('}');
							sConfigContent = sConfigContent.substr(0, pos2) + ", \"canRemoved\": false }";
						}
					}

					if (bIsBackup)
					{
						// устанавливаем свойство для дальнейших действий
						pos2 = sConfigContent.find_last_of('}');
						sConfigContent = sConfigContent.substr(0, pos2) + ", \"backup\": true }";
					}

					sData += sConfigContent;
				}
			}
		}

		sPlugins += sData;
		sPlugins += "] }";

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

	bool CheckExternal(const std::string& sJson, bool isSystem)
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
