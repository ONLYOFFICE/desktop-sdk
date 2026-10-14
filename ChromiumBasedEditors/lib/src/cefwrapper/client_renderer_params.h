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

#ifndef CEF_ASC_CLIENT_RENDERER_PARAMS_H_
#define CEF_ASC_CLIENT_RENDERER_PARAMS_H_

#include <string>
#include <vector>
#include <map>
#include "../../../../../core/DesktopEditor/common/File.h"

class CAscRendererProcessParams
{
private:
    std::map<std::string, std::string> m_params;
    std::map<std::string, std::string> m_props;
    std::map<int, std::string> m_xfaViewers;

public:
    CAscRendererProcessParams()
    {
    }
    ~CAscRendererProcessParams()
    {
    }

    void SetXfaViewer(int id, const std::string& url)
    {
        if (!url.empty())
            m_xfaViewers[id] = url;
        else
            m_xfaViewers.erase(id);
    }

    std::string GetXfaViewer(int id) const
    {
        const auto found = m_xfaViewers.find(id);
        return found == m_xfaViewers.end() ? "" : found->second;
    }

    void Check(const std::vector<std::string>& params)
    {
        for (std::vector<std::string>::const_iterator iter = params.begin(); iter != params.end(); iter++)
        {
            const std::string& prop = *iter;

            std::string::size_type pos = prop.find('=');
            if (std::string::npos != pos)
            {
                std::string key = prop.substr(0, pos);
                std::string value = (pos < (prop.length() - 1)) ? prop.substr(pos + 1) : "";

                std::map<std::string, std::string>::iterator find = m_params.find(key);
                if (find != m_params.end())
                    m_params.erase(find);

                m_params.insert(std::make_pair(key, value));
            }
        }
    }

#if 0
    void Print()
    {
        for (std::map<std::string, std::string>::iterator iter = m_params.begin(); iter != m_params.end(); iter++)
        {
            FILE * f = fopen("D:\\rend.txt", "a+");
            fprintf(f, iter->first.c_str());
            fprintf(f, " : ");
            fprintf(f, iter->second.c_str());
            fprintf(f, "\n");
            fclose(f);
        }
    }
#endif

    std::string GetValue(const std::string& key, const std::string& default_value = "")
    {
        std::map<std::string, std::string>::iterator find = m_params.find(key);
        if (find != m_params.end())
            return find->second;
        return default_value;
    }

    std::wstring GetValueW(const std::string& key, const std::wstring& default_value = L"")
    {
        std::map<std::string, std::string>::iterator find = m_params.find(key);
        if (find != m_params.end())
        {
            std::string secA = find->second;
            return UTF8_TO_U(secA);
        }
        return default_value;
    }

    int GetValueInt(const std::string& key, const int& default_value = 0)
    {
        std::map<std::string, std::string>::iterator find = m_params.find(key);
        if (find != m_params.end())
            return std::stoi(find->second);
        return default_value;
    }

    std::string GetProperty(const std::string& name)
    {
        std::map<std::string, std::string>::iterator find = m_props.find(name);
        if (find == m_props.end())
            return "";
        return find->second;
    }
    void SetProperty(const std::string& name, const std::string& value)
    {
        std::map<std::string, std::string>::iterator find = m_props.find(name);
        if (find != m_props.end())
            m_props.erase(find);
        m_props.insert(std::pair<std::string, std::string>(name, value));
    }

    static CAscRendererProcessParams& getInstance()
    {
        static CAscRendererProcessParams _params;
        return _params;
    }
};

#endif
