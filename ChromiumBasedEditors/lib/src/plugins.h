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
        m_sPluginBase = "(function(window, undefined){\n\
\n\
                window.Asc = window.Asc || {};\n\
                window.Asc.plugin = null;\n\
\n\
                window.plugin_sendMessage = function sendMessage(data)\n\
                {\n\
                    window.parent.postMessage(data, \"*\");\n\
                };\n\
\n\
                function onMessage(event)\n\
                {\n\
                    if (!window.Asc.plugin)\n\
                        return;\n\
\n\
                    if (typeof(event.data) == \"string\")\n\
                    {\n\
                        var pluginData = {};\n\
                        try\n\
                        {\n\
                            pluginData = JSON.parse(event.data);\n\
                        }\n\
                        catch(err)\n\
                        {\n\
                            pluginData = {};\n\
                        }\n\
\n\
                        if (pluginData.guid != window.Asc.plugin.guid)\n\
                            return;\n\
\n\
                        var type = pluginData.type;\n\
\n\
                        if (type == \"init\")\n\
                            window.Asc.plugin.info = pluginData;\n\
\n\
                        switch (type)\n\
                        {\n\
                            case \"init\":\n\
                            {\n\
                                window.Asc.plugin.executeCommand = function(type, data)\n\
                                {\n\
                                    window.Asc.plugin.info.type = type;\n\
                                    window.Asc.plugin.info.data = data;\n\
\n\
                                    var _message = \"\";\n\
                                    try\n\
                                    {\n\
                                        _message = JSON.stringify(window.Asc.plugin.info);\n\
                                    }\n\
                                    catch(err)\n\
                                    {\n\
                                        _message = \"{ \\\"\" + type + \"\\\" : \\\"\" + data + \"\\\" }\";\n\
                                    }\n\
                                    window.plugin_sendMessage(_message);\n\
                                };\n\
\n\
                                window.Asc.plugin.init(window.Asc.plugin.info.data);\n\
                                break;\n\
                            }\n\
                            case \"button\":\n\
                            {\n\
                                window.Asc.plugin.button(parseInt(pluginData.button));\n\
                                break;\n\
                            }\n\
                            default:\n\
                                break;\n\
                        }\n\
                    }\n\
                }\n\
\n\
                if (window.addEventListener)\n\
                {\n\
                    window.addEventListener(\"message\", onMessage, false);\n\
                }\n\
                else\n\
                {\n\
                    window.attachEvent(\"onmessage\", onMessage);\n\
                }\n\
\n\
                window.onload = function()\n\
                {\n\
                    var obj = {\n\
                        type : \"initialize\",\n\
                        guid : window.Asc.plugin.guid\n\
                    };\n\
                    window.plugin_sendMessage(JSON.stringify(obj));\n\
                };\n\
            })(window, undefined);";
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
            if (NSFile::CFileBinary::ReadAllTextUtf8A(_arPlugins[i] + L"/config.js", sJson))
            {
                if (sJson.find("window.Asc.plugin") != std::string::npos)
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
            std::wstring sConfig = sTemp + L"/config.js";

            std::string sJson = "";
            if (NSFile::CFileBinary::ReadAllTextUtf8A(sConfig, sJson))
            {
                if (sJson.find("window.Asc.plugin") != std::string::npos)
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
