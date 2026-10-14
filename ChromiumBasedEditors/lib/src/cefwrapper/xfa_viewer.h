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

#ifndef XFA_VIEWER_H
#define XFA_VIEWER_H

#include <algorithm>
#include <string>

namespace NSXfaViewer
{
inline std::string FileUrl(const std::string& path)
{
    static const char hex[] = "0123456789ABCDEF";
    std::string normalized = path;
#ifdef _WIN32
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
#endif
    std::string result = normalized.empty() || normalized[0] != '/' ? "file:///" : "file://";
#ifdef _WIN32
    if (normalized.find("//") == 0)
        result = "file:";
#endif
    for (unsigned char c : normalized)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '/' || c == ':' ||
            c == '-' || c == '_' || c == '.' || c == '~')
            result += c;
        else
        {
            result += '%';
            result += hex[c >> 4];
            result += hex[c & 15];
        }
    }
    return result;
}

inline bool IsViewerUrl(const std::string& url, const std::string& viewer)
{
    return url == viewer || (url.size() > viewer.size() &&
        url.compare(0, viewer.size(), viewer) == 0 && url[viewer.size()] == '#');
}

inline bool IsAllowedUrl(const std::string& url, const std::string& viewer,
                         const std::string& source)
{
    const std::string page = viewer.substr(0, viewer.find('?'));
    const std::string assets = page.substr(0, page.rfind('/') + 1);
    if (IsViewerUrl(url, viewer) || url == source || url.find("data:") == 0 ||
        url.find("blob:file:///") == 0 || url.find("blob:null/") == 0)
        return true;
    if (url.compare(0, assets.size(), assets) != 0)
        return false;
    const std::string path = "/" + url.substr(assets.size(), url.find_first_of("?#", assets.size()) - assets.size()) + "/";
    return path.find('%') == std::string::npos && path.find('\\') == std::string::npos &&
        path.find("/../") == std::string::npos && path.find("/./") == std::string::npos;
}
}

#endif
