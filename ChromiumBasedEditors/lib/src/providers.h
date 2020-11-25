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

#ifndef APPLICATION_PROVIDERS_H
#define APPLICATION_PROVIDERS_H

#include "./plugins.h"
#include "./utils.h"

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

class CExternalClouds
{
public:
    std::wstring m_sSystemDirectory;
    std::wstring m_sUserDirectory;

public:
    CExternalClouds()
    {
    }

    void Load(std::vector<CExternalCloudRegister>& arClouds)
    {
        std::vector<std::wstring> arSystemDirs = NSDirectory::GetDirectories(m_sSystemDirectory);
        std::vector<std::wstring> arUserDirs;

        if (!m_sUserDirectory.empty())
            arUserDirs = NSDirectory::GetDirectories(m_sUserDirectory);

        if (!arUserDirs.empty())
            arSystemDirs.insert(arSystemDirs.end(), arUserDirs.begin(), arUserDirs.end());

        for (std::vector<std::wstring>::iterator iter = arSystemDirs.begin(); iter != arSystemDirs.end(); iter++)
        {
            std::wstring sConfigFile = *iter + L"/config.json";

            std::string sConfigData = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfigFile, sConfigData))
            {
                CExternalCloudRegister cloudEx;
                cloudEx.id = CPluginsManager::GetStringValueW(sConfigData, "id");
                cloudEx.name = CPluginsManager::GetStringValueW(sConfigData, "name");
                cloudEx.test_editor = CPluginsManager::GetStringValueW(sConfigData, "editorPage");
                std::wstring sCryptoTest = CPluginsManager::GetStringValueW(sConfigData, "cryptoSupport");
                if (sCryptoTest == L"true" || sCryptoTest == L"1")
                    cloudEx.crypto_support = true;
                arClouds.push_back(cloudEx);
            }
        }
    }

    std::string GetAllJSON()
    {
        std::vector<std::wstring> arSystemDirs = NSDirectory::GetDirectories(m_sSystemDirectory);
        std::vector<std::wstring> arUserDirs;

        if (!m_sUserDirectory.empty())
            arUserDirs = NSDirectory::GetDirectories(m_sUserDirectory);

        std::string sResult = "[";

        for (std::vector<std::wstring>::iterator iter = arSystemDirs.begin(); iter != arSystemDirs.end(); iter++)
        {
            std::wstring sConfigFile = *iter + L"/config.json";

            std::string sConfigData = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfigFile, sConfigData))
            {
                std::string::size_type find_id = sConfigData.find("{");
                if (std::string::npos != find_id)
                {
                    sConfigData = (sConfigData.substr(0, find_id + 1) + "\"path\":\"./providers/" + U_TO_UTF8(NSFile::GetFileName(*iter)) + "\"," + sConfigData.substr(find_id + 1));
                }

                NSCommon::string_replaceA(sConfigData, "\r", "");
                NSCommon::string_replaceA(sConfigData, "\n", "");
                NSCommon::string_replaceA(sConfigData, "\\", "\\\\");
                NSCommon::string_replaceA(sConfigData, "\"", "\\\"");

                if (1 != sResult.length())
                    sResult += ",";

                sResult += sConfigData;
            }
        }

        for (std::vector<std::wstring>::iterator iter = arUserDirs.begin(); iter != arUserDirs.end(); iter++)
        {
            std::wstring sConfigFile = *iter + L"/config.json";

            std::string sConfigData = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfigFile, sConfigData))
            {
                std::string::size_type find_id = sConfigData.find("{");
                if (std::string::npos != find_id)
                {
                    std::wstring sDirPath = *iter;
                    NSCommon::string_replace(sDirPath, L"\\", L"/");
                    sConfigData = (sConfigData.substr(0, find_id + 1) + "\"path\":\"" + U_TO_UTF8(sDirPath) + "\"," + sConfigData.substr(find_id + 1));
                }

                NSCommon::string_replaceA(sConfigData, "\r", "");
                NSCommon::string_replaceA(sConfigData, "\n", "");
                NSCommon::string_replaceA(sConfigData, "\\", "\\\\");
                NSCommon::string_replaceA(sConfigData, "\"", "\\\"");

                if (1 != sResult.length())
                    sResult += ",";

                sResult += sConfigData;
            }
        }

        sResult += "]";
        return sResult;
    }
};

#endif // APPLICATION_PROVIDERS_H
