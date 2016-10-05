/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#ifndef APPLICATION_PLUGINS_H
#define APPLICATION_PLUGINS_H

#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/OfficeUtils/src/OfficeUtils.h"

class CPluginsManager
{
public:
    std::wstring m_strDirectory;
    std::string m_sPluginBase;

public:
    CPluginsManager()
    {
        // sdkjs-plugins/pluginBase.js
        m_sPluginBase = "(function(n,t){function i(n,r){if(!n||!(\"object\"==typeof n||\"array\"==typeof n))return n;var e=r===t?{}:r,f,u;for(f in n)n.hasOwnProperty(f)&&(u=n[f],e[f]=u&&\"object\"==typeof u?i(u):u);return e}function r(t){if(n.Asc.plugin){if(n.plugin_onMessage){n.plugin_onMessage(t);return}if(typeof t.data==\"string\"){var i={};try{i=JSON.parse(t.data)}catch(r){i={}}(i.type==\"plugin_init\")&&eval(i.data)}}}n.Asc=n.Asc||{};n.Asc.plugin={};n.onload=function(){var t=new XMLHttpRequest;t.open(\"get\",\"./config.json\",!0);t.responseType=\"json\";t.onload=function(){var r,u;(t.status==200 || t.status==0)&&(r=t.response,i(r,n.Asc.plugin),u={type:\"initialize\",guid:n.Asc.plugin.guid},n.parent.postMessage(JSON.stringify(u),\"*\"))};t.send()};n.addEventListener?n.addEventListener(\"message\",r,!1):n.attachEvent(\"onmessage\",r)})(window,undefined);";
    }

    std::string GetPluginsJson()
    {
        if (!NSDirectory::Exists(m_strDirectory))
            NSDirectory::CreateDirectory(m_strDirectory);

        std::wstring strBase = m_strDirectory + L"/pluginBase.js";
        if (NSFile::CFileBinary::Exists(strBase))
            NSFile::CFileBinary::Remove(strBase);

        std::wstring strBaseCSS = m_strDirectory + L"/plugins.css";
        if (NSFile::CFileBinary::Exists(strBaseCSS))
            NSFile::CFileBinary::Remove(strBaseCSS);

        if (true)
        {
            NSFile::CFileBinary oFile;
            if (oFile.CreateFileW(strBase))
            {
                oFile.WriteFile((BYTE*)m_sPluginBase.c_str(), (DWORD)m_sPluginBase.length());
                oFile.CloseFile();
            }

            NSFile::CFileBinary oFileCSS;
            if (oFile.CreateFileW(strBaseCSS))
            {
                std::string sCSS = ".btn-text-default{background:#fff;border:1px solid #cfcfcf;border-radius:2px;color:#444;font-size:11px;font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;height:22px;cursor:pointer}.btn-text-default:focus{outline:0;outline-offset:0}.btn-text-default:hover{background-color:#d8dadc!important}.btn-text-default:active,.btn-text-default.active{background-color:#7d858c!important;color:#fff;-webkit-box-shadow:none;box-shadow:none}.btn-text-default[disabled]:hover,.btn-text-default.disabled:hover,.btn-text-default[disabled]:active,.btn-text-default[disabled].active,.btn-text-default.disabled:active,.btn-text-default.disabled.active{background-color:#fff!important;color:#444;cursor:default}.btn-text-default[disabled],.btn-text-default.disabled{opacity:.65}.form-control{border:1px solid #cfcfcf;border-radius:2px;box-sizing:border-box;color:#444;font-size:11px;height:22px;padding:1px 3px;-webkit-box-shadow:none;box-shadow:none;-webkit-user-select:text;-moz-user-select:text;-ms-user-select:text;user-select:text}.form-control:focus{border-color:#cfcfcf;outline:0;-webkit-box-shadow:none;box-shadow:none}.form-control[readonly]{background-color:#fff;cursor:pointer}.form-control[disabled]{background-color:#fff;cursor:default;-webkit-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.defaultlable{color:#444;cursor:default;font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size:11px;font-weight:400;-webkit-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.noselect{-khtml-user-select:none;user-select:none;-moz-user-select:none;-webkit-user-select:none}.ps-container .ps-scrollbar-y-rail{position:absolute;right:3px;width:10px}.ps-scrollbar-y{position:absolute;right:0;width:9px;background-color:#f1f1f1;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px;visibility:visible;display:block;box-sizing:border-box;background-image:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAANCAQAAAAz1Zf0AAAAIUlEQVR42mNgAILz/0GQAQo+/gdBBqLAqE5ydH5k+sgEANHgUH2JtDRHAAAAAElFTkSuQmCC);background-repeat:no-repeat;background-position:0 center;border:1px solid #cfcfcf}.ps-container .ps-scrollbar-y-rail:hover .ps-scrollbar-y,.ps-container .ps-scrollbar-y-rail.hover .ps-scrollbar-y{background-color:#cfcfcf;background-position:-7px center}.ps-container.ps-in-scrolling .ps-scrollbar-y-rail .ps-scrollbar-y{background-color:#adadad;border-color:#adadad;background-position:-7px center}";
                oFile.WriteFile((BYTE*)sCSS.c_str(), (DWORD)sCSS.length());
                oFile.CloseFile();
            }
        }

        std::wstring sParam = m_strDirectory + L"/";
        if (0 == sParam.find('/'))
            sParam = L"file://" + sParam;
        else
            sParam = L"file:///" + sParam;

        std::string sPlugins = "{ \"url\" : \"" + U_TO_UTF8(sParam) + "\", \"pluginsData\" : [";
        std::string sData = "";
        int nCountPlugins = 0;

        CArray<std::wstring> _arPlugins = NSDirectory::GetDirectories(m_strDirectory);
        int nCount = _arPlugins.GetCount();
        for (int i = 0; i < nCount; ++i)
        {
            std::string sJson = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.json", sJson))
            {
                std::string::size_type pos1 = sJson.find('{');
                std::string::size_type pos2 = sJson.find_last_of('}');

                if (pos1 != std::string::npos &&
                    pos2 != std::string::npos &&
                    pos2 > pos1)
                {
                    if (nCountPlugins > 0)
                        sData += ",";
                    nCountPlugins++;
                    sData += sJson.substr(pos1, pos2 - pos1 + 1);
                }
            }
        }

        sPlugins += sData;

        sPlugins += "] }";

        //string_replaceA(sPlugins, "\"", "\\\"");
        string_replaceA(sPlugins, "\r\n", "");
        string_replaceA(sPlugins, "\n", "");

        return sPlugins;
    }

    bool AddPlugin(std::wstring sFilePlugin)
    {
        if (!NSDirectory::Exists(m_strDirectory))
            NSDirectory::CreateDirectory(m_strDirectory);

        std::wstring sTemp = m_strDirectory + L"/temp_asc_plugin_directory";
        NSDirectory::CreateDirectory(sTemp);

        std::wstring sPluginName = L"";

        COfficeUtils oOfficeUtils(NULL);
        if (S_OK == oOfficeUtils.ExtractToDirectory(sFilePlugin, sTemp, NULL, 0))
        {
            std::wstring sConfig = sTemp + L"/config.json";

            std::string sJson = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfig, sJson))
            {
                std::string::size_type posUrl = sJson.find("\"url\"");
                if (posUrl != std::string::npos)
                {
                    posUrl += 5;
                    std::string::size_type pos1 = sJson.find('\"', posUrl);
                    if (pos1 != std::string::npos)
                    {
                        std::string::size_type pos2 = sJson.find('\"', pos1 + 1);
                        if (pos2 != std::string::npos)
                        {
                            std::string sUrl = sJson.substr(pos1 + 1, pos2 - pos1 - 1);

                            std::string::size_type posDst2 = sUrl.find_last_of('/');
                            if (posDst2 != std::string::npos)
                            {
                                std::string::size_type posDst1 = sUrl.find_last_of('/', posDst2 - 1);
                                if (posDst1 == std::string::npos)
                                    posDst1 = 0;
                                else
                                    posDst1 += 1;

                                std::string sName = sUrl.substr(posDst1, posDst2 - posDst1);
                                sPluginName = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sName.c_str(), (LONG)sName.length());
                            }
                        }
                    }
                }
            }
        }

        NSDirectory::DeleteDirectory(sTemp);
        if (sPluginName.empty())
            return false;

        std::wstring sNew = m_strDirectory + L"/" + sPluginName;
        NSDirectory::DeleteDirectory(sNew);
        NSDirectory::CreateDirectory(sNew);

        oOfficeUtils.ExtractToDirectory(sFilePlugin, sNew, NULL, 0);
        return true;
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
};

#endif // APPLICATION_PLUGINS_H
