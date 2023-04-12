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

#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/OfficeUtils/src/OfficeUtils.h"

//#include "./plugins_resources.h"
#include <map>

class CExternalPluginInfo
{
public:
	std::string sGuid;
	std::string sName;
	std::string sNameObject;
	bool isUser;
};

class CPluginsManager
{
public:
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

public:
	CPluginsManager()
	{
		m_nCryptoMode = 0;

		m_bCryptoDisabledOnStart = false;
		m_bCryptoDisableForLocal = false;
		m_bCryptoDisableForInternalCloud = false;
		m_bCryptoDisableForExternalCloud = false;
	}

	std::string GetPluginsJson(const bool& bActivePlugins = true, const bool& bCheckCrypto = false)
	{
		if (!NSDirectory::Exists(m_strUserDirectory))
			NSDirectory::CreateDirectory(m_strUserDirectory);

		std::string sPluginsJSON = "[";

		if (bActivePlugins)
		{
			for (int i = 0; i < 2; i++)
			{
				std::wstring sDir = (i == 0) ? m_strDirectory : m_strUserDirectory;

				std::string sPlugins = ParsePluginDir(sDir, bCheckCrypto, false);

				sPluginsJSON += sPlugins;

				if (i == 0)
					sPluginsJSON += ", ";
			}
		}
		else
		{
			// поиск плагинов в папке пользователя для возможного восстановления
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

		for (int i = 0; i < 2; i++)
		{
			std::wstring sDir = (i == 0) ? m_strDirectory : m_strUserDirectory;

			std::vector<std::wstring> _arPlugins = NSDirectory::GetDirectories(sDir);
			int nCount = (int)_arPlugins.size();
			for (int i = 0; i < nCount; ++i)
			{
				std::string sJson;
				if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.json", sJson))
				{
					CheckEncryption(sJson, false);
					CheckExternal(sJson, i == 0);

					std::string::size_type pos1 = sJson.find("asc.{");
					std::string::size_type pos2 = sJson.find('}', pos1);

					if (pos1 != std::string::npos &&
							pos2 != std::string::npos &&
							pos2 > pos1)
					{
						arCongigs.push_back(sJson.substr(pos1, pos2 - pos1 + 1));
					}
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

		return true;
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

	void string_replaceA(std::string& text, const std::string& replaceFrom, const std::string& replaceTo)
	{
		size_t posn = 0;
		while (std::string::npos != (posn = text.find(replaceFrom, posn)))
		{
			text.replace(posn, replaceFrom.length(), replaceTo);
			posn += replaceTo.length();
		}
	}

private:
	std::string ParsePluginDir(const std::wstring& sDir, const bool& bCheckCrypto = false, const bool& bIsBackup = false)
	{
		std::wstring sParam = sDir + L"/";
		if (0 == sParam.find('/'))
			sParam = L"file://" + sParam;
		else
			sParam = L"file:///" + sParam;

		std::string sPlugins = "{ \"url\" : \"" + U_TO_UTF8(sParam) + "\", \"pluginsData\" : [";
		std::string sData = "";
		int nCountPlugins = 0;

		std::vector<std::wstring> _arPlugins = NSDirectory::GetDirectories(sDir);

		size_t nCount = _arPlugins.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			std::string sJson = "";
			if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.json", sJson))
			{
				if (!CheckEncryption(sJson, bCheckCrypto))
					continue;

				if (CheckExternal(sJson, i == 0))
					continue;

				std::string::size_type pos1 = sJson.find('{');
				std::string::size_type pos2 = sJson.find_last_of('}');

				if (pos1 != std::string::npos &&
						pos2 != std::string::npos &&
						pos2 > pos1)
				{
					if (nCountPlugins > 0)
						sData += ",";

					nCountPlugins++;
					std::string sConfigContent = sJson.substr(pos1, pos2 - pos1 + 1);

					if (bIsBackup)
					{
						// выставляем признак
						pos2 = sConfigContent.find_last_of('}');
						sConfigContent = sConfigContent.substr(0, pos2) + ", \"backup\": true }";
					}

					sData += sConfigContent;
				}
			}
		}

		sPlugins += sData;
		sPlugins += "] }";

		string_replaceA(sPlugins, "\r\n", "");
		string_replaceA(sPlugins, "\n", "");

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
					string_replaceA(s1, "asc.", "");
					std::string s2 = m_strCryptoPluginAttack;
					string_replaceA(s2, "asc.", "");

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
