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

#ifndef APPLICATION_MANAGER_UTILS_H
#define APPLICATION_MANAGER_UTILS_H

#include <string>
#include <string.h>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Logger.h"
#include "../../../../core/Common/FileDownloader/FileDownloader.h"
#include "../../../../core/DesktopEditor/fontengine/ApplicationFontsWorker.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../core/DesktopEditor/common/File.h"

namespace NSCommon
{
    static void makeUpper(std::string& url)
    {
        int nLen = (int)url.length();
        char* pStr = (char*)url.c_str();

        for (int i = 0; i < nLen; ++i)
        {
            if (pStr[i] >= 'a' && pStr[i] <= 'z')
                pStr[i] = pStr[i] + 'A' - 'a';
        }
    }
    static void makeUpperW(std::wstring& url)
    {
        int nLen = (int)url.length();
        wchar_t* pStr = (wchar_t*)url.c_str();

        for (int i = 0; i < nLen; ++i)
        {
            if (pStr[i] >= 'a' && pStr[i] <= 'z')
                pStr[i] = pStr[i] + 'A' - 'a';
        }
    }
    static void makeLower(std::string& url)
    {
        int nLen = (int)url.length();
        char* pStr = (char*)url.c_str();

        for (int i = 0; i < nLen; ++i)
        {
            if (pStr[i] >= 'A' && pStr[i] <= 'Z')
                pStr[i] = pStr[i] + 'a' - 'A';
        }
    }
    static void makeLowerW(std::wstring& url)
    {
        int nLen = (int)url.length();
        wchar_t* pStr = (wchar_t*)url.c_str();

        for (int i = 0; i < nLen; ++i)
        {
            if (pStr[i] >= 'A' && pStr[i] <= 'Z')
                pStr[i] = pStr[i] + 'a' - 'A';
        }
    }

    static std::wstring::size_type FindLowerCase(const std::wstring& string, const std::wstring& find)
    {
        std::wstring sTmp = string;
        NSCommon::makeLowerW(sTmp);
        return sTmp.find(find);
    }
    static std::wstring::size_type FindLowerCaseR(const std::wstring& string, const std::wstring& find)
    {
        std::wstring sTmp = string;
        NSCommon::makeLowerW(sTmp);
        return sTmp.rfind(find);
    }
    static std::string::size_type FindLowerCase(const std::string& string, const std::string& find)
    {
        std::string sTmp = string;
        NSCommon::makeLower(sTmp);
        return sTmp.find(find);
    }
    static std::string::size_type FindLowerCaseR(const std::string& string, const std::string& find)
    {
        std::string sTmp = string;
        NSCommon::makeLower(sTmp);
        return sTmp.rfind(find);
    }

    static void url_correct(std::wstring& url)
    {
        NSStringUtils::string_replace(url, L"/./", L"/");

        size_t posn = 0;
        while (std::wstring::npos != (posn = url.find(L"/../")))
        {
            std::wstring::size_type pos2 = url.rfind(L"/", posn - 1);

            if (std::wstring::npos != pos2)
            {
                url.erase(pos2, posn - pos2 + 3);
            }
        }

        // MAC
        if (0 == url.find(L"file:/") && 0 != url.find(L"file://"))
        {
            url.replace(0, 6, L"file:///");
        }
    }
}

class CFileDownloaderWrapper
{
private:
    std::wstring m_sUrl;
    std::wstring m_sOutput;
    bool m_bIsDownloaded;

public:
    CFileDownloaderWrapper(const std::wstring& sUrl, const std::wstring& sOutput)
    {
        m_sUrl = sUrl;
        m_sOutput = sOutput;

        if (m_sOutput.empty())
        {
            m_sOutput = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"DW");
            if (NSFile::CFileBinary::Exists(m_sOutput))
                NSFile::CFileBinary::Remove(m_sOutput);
        }

        m_bIsDownloaded = false;
    }

    int DownloadSync()
    {
        CFileDownloader oDownloader(m_sUrl, false);
        oDownloader.SetFilePath(m_sOutput);
        oDownloader.Start( 0 );
        while ( oDownloader.IsRunned() )
        {
            NSThreads::Sleep( 10 );
        }
        m_bIsDownloaded = oDownloader.IsFileDownloaded();
        return 0;
    }

    bool IsFileDownloaded()
    {
        return m_bIsDownloaded;
    }
    std::wstring GetFilePath()
    {
        return m_sOutput;
    }
};

class CSystemVariablesMemory
{
private:
    std::vector<char*> m_data;

public:
    CSystemVariablesMemory()
    {
    }
    ~CSystemVariablesMemory()
    {
        for (std::vector<char*>::iterator iter = m_data.begin(); iter != m_data.end(); iter++)
        {
            char* rem = *iter;
            if (rem)
                free(rem);
        }
        m_data.clear();
    }

    char* Push(const std::string& str)
    {
        size_t str_len = (size_t)str.length();
        char* sValueStr = (char*)malloc((str_len + 1) * sizeof(char));
        memcpy(sValueStr, str.c_str(), str_len * sizeof(char));
        sValueStr[str_len] = '\0';
        m_data.push_back(sValueStr);
        return sValueStr;
    }
};

namespace NSSystem
{
    static void SetEnvValueA(const std::string& sName, const std::string& sValue)
    {
#ifdef WIN32
        std::wstring sNameW = UTF8_TO_U(sName);
        std::wstring sValueW = UTF8_TO_U(sValue);
        SetEnvironmentVariable(sNameW.c_str(), sValueW.c_str());
#else
        static char buffer[100000]; // on all process
        static int offset = 0;

        std::string tmp = sName + "=" + sValue;
        size_t len = tmp.length();

        memcpy(buffer + offset, tmp.c_str(), sizeof(char) * len);
        buffer[offset + len] = '\0';
        putenv(buffer + offset);
        offset += (len + 1);
#endif        
    }
    static void SetEnvValue(const std::string& sName, const std::wstring& sValue)
    {
        std::string sValueA = U_TO_UTF8(sValue);
        return SetEnvValueA(sName, sValueA);
    }
    static std::string GetEnvValueA(const std::string& sName)
    {
#ifdef WIN32
        std::wstring sNameW = UTF8_TO_U(sName);
        const DWORD buffSize = 65535;
        wchar_t buffer[buffSize];
        if (GetEnvironmentVariable(sNameW.c_str(), buffer, buffSize))
        {
            std::wstring sValueW(buffer);
            std::string sValue = U_TO_UTF8(sValueW);
            return sValue;
        }
        else
        {
            return "";
        }
#else
        char* pPath = getenv(sName.c_str());
        if (NULL != pPath)
            return std::string(pPath);
        return "";
#endif
    }
    static std::wstring GetEnvValue(const std::string& sName)
    {
        std::string sValueA = GetEnvValueA(sName);
        std::wstring sValue = UTF8_TO_U(sValueA);
        return sValue;
    }
}

namespace NSUrlParse
{
    static std::wstring GetUrlValue(const std::wstring& sValue, const std::wstring& sProp)
    {
        std::wstring::size_type pos1 = sValue.find(sProp + L"=");
        if (std::wstring::npos == pos1)
            return L"";

        pos1 += (sProp.length() + 1);

        std::wstring::size_type pos2 = sValue.find(L"&", pos1);
        if (std::wstring::npos == pos2)
        {
            return sValue.substr(pos1);
        }
        return sValue.substr(pos1, pos2  - pos1);
    }
}

namespace NSFileDownloader
{
    static bool IsNeedDownload(const std::wstring& FilePath)
    {
        std::wstring::size_type n1 = FilePath.find(L"www.");
        std::wstring::size_type n2 = FilePath.find(L"http://");
        std::wstring::size_type n3 = FilePath.find(L"ftp://");
        std::wstring::size_type n4 = FilePath.find(L"https://");

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
}

class CJSONSimple
{
private:
    NSStringUtils::CStringBuilder builder;
    bool m_isSlash;

public:
    CJSONSimple(bool isSlash = false)
    {
        m_isSlash = isSlash;
    }

public:
    std::wstring GetData()
    {
        return builder.GetData();
    }

    void Start()
    {
        builder.WriteString(L"{");
    }

    void End()
    {
        builder.WriteString(L"}");
    }

    void Next()
    {
        builder.WriteString(L",");
    }

    void Write(const std::wstring& name, const std::wstring& value)
    {
        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
        builder.WriteString(name);
        m_isSlash ? builder.WriteString(L"\\\":") : builder.WriteString(L"\":");

        std::wstring s = value;
        if (m_isSlash)
            NSStringUtils::string_replace(s, L"\"", L"\\\"");

        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
        builder.WriteString(s);
        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
    }
    void Write(const std::wstring& name, const std::string& value)
    {
        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
        builder.WriteString(name);
        m_isSlash ? builder.WriteString(L"\\\":") : builder.WriteString(L"\":");

        std::wstring s = UTF8_TO_U(value);
        if (m_isSlash)
            NSStringUtils::string_replace(s, L"\"", L"\\\"");

        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
        builder.WriteString(s);
        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
    }
    void Write(const std::wstring& name, const int& value)
    {
        m_isSlash ? builder.WriteString(L"\\\"") : builder.WriteString(L"\"");
        builder.WriteString(name);
        m_isSlash ? builder.WriteString(L"\\\":") : builder.WriteString(L"\":");

        builder.AddInt(value);
    }
};

#ifdef _LINUX

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

#endif // APPLICATION_MANAGER_UTILS_H
