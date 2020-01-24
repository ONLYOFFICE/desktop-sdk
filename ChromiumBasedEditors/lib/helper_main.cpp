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

#include "./include/applicationmanager.h"
#include "./../../../core/DesktopEditor/common/File.h"

int main( int argc, char *argv[] )
{
#ifdef _WIN32
    Core_SetProcessDpiAwareness();
#endif

    CApplicationCEF application;
    CAscApplicationManager manager;

    std::wstring user_data_path = L"C:/Users/oleg.korshul/AppData/Local/ONLYOFFICE/DesktopEditors";
    manager.m_oSettings.cookie_path                   = user_data_path + L"/cookie";
    manager.m_oSettings.recover_path                  = user_data_path + L"/recover";
    manager.m_oSettings.fonts_cache_info_path         = user_data_path + L"/fonts";

    std::wstring app_path = NSFile::GetProcessDirectory();
    manager.m_oSettings.spell_dictionaries_path       = app_path + L"/dictionaries";
    manager.m_oSettings.file_converter_path           = app_path + L"/converter";
    manager.m_oSettings.recover_path                  = user_data_path + L"/recover";
    manager.m_oSettings.user_plugins_path             = user_data_path + L"/sdkjs-plugins";
    manager.m_oSettings.additional_fonts_folder.push_back(app_path + L"/fonts");

    return application.Init_CEF(&manager, argc, argv);
}
