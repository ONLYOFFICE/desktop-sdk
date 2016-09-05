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
        if (!NSFile::CFileBinary::Exists(strBase))
        {
            NSFile::CFileBinary oFile;
            if (oFile.CreateFileW(strBase))
            {
                oFile.WriteFile((BYTE*)m_sPluginBase.c_str(), (DWORD)m_sPluginBase.length());
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
                std::string::size_type pos = sJson.find("\"name\"");
                if (pos != std::string::npos)
                {
                    pos += 6;
                    std::string::size_type pos1 = sJson.find('\"', pos);
                    if (pos1 != std::string::npos)
                    {
                        std::string::size_type pos2 = sJson.find('\"', pos1 + 1);

                        if (pos2 != std::string::npos && pos2 > pos1)
                        {
                            std::string sName = sJson.substr(pos1 + 1, pos2 - pos1 - 1);
                            sPluginName = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)sName.c_str(), (LONG)sName.length());
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
