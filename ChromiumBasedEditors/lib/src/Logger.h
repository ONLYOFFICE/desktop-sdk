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

#ifndef LOGGER_PRIVATE_H
#define LOGGER_PRIVATE_H

#include <string>
#include "../../../../core/DesktopEditor/graphics/Timer.h"

#if 0
class CLoggerSpeed
{
public:
    DWORD m_dwTime;

public:
    CLoggerSpeed()
    {
        m_dwTime = NSTimers::GetTickCount();
    }
    void Lap(const std::string& details)
    {
        DWORD dwCur = NSTimers::GetTickCount();
        FILE* f = fopen("D:\\doctrenderer.speed", "a+");
        std::string sTmp = details + ": %d\n";
        fprintf(f, sTmp.c_str(), (int)(dwCur - m_dwTime));
        fclose(f);
        m_dwTime = dwCur;
    }
    static void Log(const std::wstring& details)
    {
        FILE* f = fopen("D:\\core.log", "a+");
        std::string detailsA = U_TO_UTF8(details);
        detailsA += "\n";
        fprintf(f, detailsA.c_str());
        fclose(f);
    }
    static void Log(const std::string& details)
    {
        FILE* f = fopen("D:\\core.log", "a+");
        std::string detailsA = details;
        detailsA += "\n";
        fprintf(f, detailsA.c_str());
        fclose(f);
    }
};

#define LOGGER_SPEED_START CLoggerSpeed __logger_speed;
#define LOGGER_SPEED_LAP(__logger_param) __logger_speed.Lap(#__logger_param);
#define LOGGER_STRING(__logger_param) CLoggerSpeed::Log(#__logger_param);
#define LOGGER_STRING2(__logger_param) CLoggerSpeed::Log(__logger_param);
#else
#define LOGGER_SPEED_START
#define LOGGER_SPEED_LAP(__logger_param)
#define LOGGER_STRING(__logger_param)
#define LOGGER_STRING2(__logger_param)
#endif

#endif // LOGGER_PRIVATE_H
