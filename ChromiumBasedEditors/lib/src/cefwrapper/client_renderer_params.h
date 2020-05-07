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

public:
    CAscRendererProcessParams()
    {
    }
    ~CAscRendererProcessParams()
    {
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
