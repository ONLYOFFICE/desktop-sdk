/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef APPLICATION_MANAGER_PLUGIN_PERMISSIONS_H
#define APPLICATION_MANAGER_PLUGIN_PERMISSIONS_H

#include <string>
#include <vector>

#include "./utils.h"
#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Path.h"

namespace NSPluginPermissions
{
	static const wchar_t* kPrivilegedPluginGuid = L"{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}";
	static const wchar_t* kPrivilegedPluginEntries[] = {
		L"index.html",
		L"mcp-servers.html",
		NULL
	};

	inline bool IsRestrictedMethod(const std::string& sName)
	{
		static const char* aRestricted[] = {
			"_createProcess",
			"_endProcess",
			"_stdin",

			"getToolFunctions",
			"callToolFunction",

			"getMcpServers",

			"_getPortalAuthToken",
			NULL
		};

		for (int i = 0; NULL != aRestricted[i]; ++i)
		{
			if (sName == aRestricted[i])
				return true;
		}

		return false;
	}

	namespace NSPrivate
	{
		inline int HexValue(const char c)
		{
			if (c >= '0' && c <= '9') return c - '0';
			if (c >= 'a' && c <= 'f') return c - 'a' + 10;
			if (c >= 'A' && c <= 'F') return c - 'A' + 10;
			return -1;
		}

		// %XX -> byte, UTF-8 -> wstring.
		inline std::wstring UrlDecode(const std::wstring& sSrc)
		{
			std::string sSrcUtf8 = U_TO_UTF8(sSrc);

			std::string sBytes;
			sBytes.reserve(sSrcUtf8.length());

			for (std::string::size_type i = 0, len = sSrcUtf8.length(); i < len; ++i)
			{
				char c = sSrcUtf8[i];
				if ('%' == c && (i + 2) < len)
				{
					int nHi = HexValue(sSrcUtf8[i + 1]);
					int nLo = HexValue(sSrcUtf8[i + 2]);
					if (nHi >= 0 && nLo >= 0)
					{
						char cDecoded = (char)((nHi << 4) | nLo);
						if (0 == cDecoded)
							return L"";
						sBytes += cDecoded;
						i += 2;
						continue;
					}
				}
				sBytes += c;
			}

			return NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sBytes.c_str(), (LONG)sBytes.length());
		}

		inline std::wstring PreparePath(const std::wstring& sSrc)
		{
			std::wstring sPath = UrlDecode(sSrc);
			if (sPath.empty())
				return sPath;

#ifdef _WIN32
			// "/C:/..." -> "C:/..."
			if (sPath.length() > 2 && (L'/' == sPath[0] || L'\\' == sPath[0]) && L':' == sPath[2])
				sPath.erase(0, 1);
#else
			if (L'/' != sPath[0])
				sPath = L"/" + sPath;
#endif

			return sPath;
		}

		inline bool IsSamePath(const std::wstring& sFirst, const std::wstring& sSecond)
		{
			return NSSystemPath::IsPathInDirectory(sFirst, sSecond) &&
			       NSSystemPath::IsPathInDirectory(sSecond, sFirst);
		}
	}

	// equal onlyoffice://plugin/<plugins_dir>/{GUID}/index.html
	inline bool IsPrivilegedFrame(const std::wstring& sFrameUrl,
	                              const std::wstring& sSystemPlugins,
	                              const std::wstring& sUserPlugins)
	{
		const std::wstring sScheme = L"onlyoffice://plugin/";
		if (0 != sFrameUrl.find(sScheme))
			return false;

		std::wstring sPath = sFrameUrl.substr(sScheme.length());

		std::wstring::size_type nCut = sPath.find_first_of(L"?#");
		if (std::wstring::npos != nCut)
			sPath = sPath.substr(0, nCut);

		sPath = NSPrivate::PreparePath(sPath);
		if (sPath.empty())
			return false;

		std::vector<std::wstring> arDirs;
		if (!sSystemPlugins.empty())
			arDirs.push_back(sSystemPlugins);
		if (!sUserPlugins.empty())
		{
			arDirs.push_back(sUserPlugins);
#ifdef USE_ADDON_MODE
			arDirs.push_back(sUserPlugins + L"/../addon/sdkjs-plugins");
#endif
		}

		const std::wstring sPluginDir = std::wstring(L"/") + kPrivilegedPluginGuid + L"/";

		for (std::vector<std::wstring>::iterator i = arDirs.begin(); i != arDirs.end(); i++)
		{
			for (int j = 0; NULL != kPrivilegedPluginEntries[j]; ++j)
			{
				if (NSPrivate::IsSamePath(sPath, *i + sPluginDir + kPrivilegedPluginEntries[j]))
					return true;
			}
		}

		return false;
	}

	inline bool IsMethodAllowed(const std::string& sName,
	                            const std::wstring& sFrameUrl,
	                            const std::wstring& sSystemPlugins,
	                            const std::wstring& sUserPlugins)
	{
		if (!IsRestrictedMethod(sName))
			return true;

		return IsPrivilegedFrame(sFrameUrl, sSystemPlugins, sUserPlugins);
	}

	inline bool IsProtectedGuid(const std::wstring& sGuid)
	{
		std::wstring sValue = sGuid;

		std::wstring::size_type nPos = sValue.find(L'{');
		if (std::wstring::npos == nPos)
			return false;
		if (0 != nPos)
			sValue = sValue.substr(nPos);

		std::wstring sProtected = kPrivilegedPluginGuid;

		NSCommon::makeLowerW(sValue);
		NSCommon::makeLowerW(sProtected);

		return sValue == sProtected;
	}
}

#endif // APPLICATION_MANAGER_PLUGIN_PERMISSIONS_H
