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

#ifndef APPLICATION_CLOUDCRYPTO_H
#define APPLICATION_CLOUDCRYPTO_H

#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "../../../../core/DesktopEditor/fontengine/application_generate_fonts_common.h"
#include "./Logger.h"
#include <map>

class CCloudCrypto
{
public:
    std::wstring GUID;           // идентификатор плагина для криптования
    std::wstring User;
    std::wstring Email;
    std::wstring Portal;
    std::wstring PublicKey;      // публичный ключ пользователя
    std::wstring PrivateKeyEnc;  // зашифрованный паролем к порталу закрытый ключ

public:
    CCloudCrypto()
    {
    }
    virtual void Write(NSStringUtils::CStringBuilder& builder, bool isMain = true)
    {
        if (isMain)
            builder.WriteString(L"<info>");

        builder.WriteString(L"<guid>");
        builder.WriteEncodeXmlString(GUID);
        builder.WriteString(L"</guid>");

        builder.WriteString(L"<user>");
        builder.WriteEncodeXmlString(User);
        builder.WriteString(L"</user>");

        builder.WriteString(L"<email>");
        builder.WriteEncodeXmlString(Email);
        builder.WriteString(L"</email>");

        builder.WriteString(L"<portal>");
        builder.WriteEncodeXmlString(Portal);
        builder.WriteString(L"</portal>");

        builder.WriteString(L"<publickey>");
        builder.WriteEncodeXmlString(PublicKey);
        builder.WriteString(L"</publickey>");

        builder.WriteString(L"<privatekeyenc>");
        builder.WriteEncodeXmlString(PrivateKeyEnc);
        builder.WriteString(L"</privatekeyenc>");

        if (isMain)
            builder.WriteString(L"</info>");
    }
    virtual void Read(XmlUtils::CXmlNode& node)
    {
        GUID = node.ReadValueString(L"guid");
        User = node.ReadValueString(L"user");
        Email = node.ReadValueString(L"email");
        Portal = node.ReadValueString(L"portal");
        PublicKey = node.ReadValueString(L"publickey");
        PrivateKeyEnc = node.ReadValueString(L"privatekeyenc");
    }
};

class CCloudCryptoDesktop : public CCloudCrypto
{
public:
    std::wstring PrivateKey;

public:
    CCloudCryptoDesktop() : CCloudCrypto()
    {
    }

    virtual void Write(NSStringUtils::CStringBuilder& builder, bool isMain = true)
    {
        if (isMain)
            builder.WriteString(L"<info>");

        CCloudCrypto::Write(builder, false);

        builder.WriteString(L"<privatekey>");
        builder.WriteEncodeXmlString(PrivateKey);
        builder.WriteString(L"</privatekey>");

        if (isMain)
            builder.WriteString(L"</info>");
    }
    virtual void Read(XmlUtils::CXmlNode& node)
    {
        CCloudCrypto::Read(node);
        PrivateKey = node.ReadValueString(L"privatekey");
    }
};

class CCloudCryptoApp
{
private:
    std::wstring m_sPath;
    std::map<std::wstring, CCloudCryptoDesktop> m_mapUsers;

public:
    CCloudCryptoApp(const std::wstring& path)
    {
        m_sPath = path;
        XmlUtils::CXmlNode node;
        if (node.FromXmlFile(path))
        {
            XmlUtils::CXmlNodes nodes;
            node.GetNodes(L"info", nodes);

            int count = nodes.GetCount();
            for (int i = 0; i < count; i++)
            {
                XmlUtils::CXmlNode nodeInfo;
                nodes.GetAt(i, nodeInfo);

                CCloudCryptoDesktop userInfo;
                userInfo.Read(nodeInfo);

                std::wstring sKey = userInfo.User;
                NSCommon::makeLowerW(sKey);
                m_mapUsers.insert(std::pair<std::wstring, CCloudCryptoDesktop>(sKey, userInfo));
            }
        }
    }
    void AddInfo(const CCloudCryptoDesktop& info)
    {
        std::wstring sKey = info.User;
        NSCommon::makeLowerW(sKey);

        std::map<std::wstring, CCloudCryptoDesktop>::iterator find = m_mapUsers.find(sKey);
        if (find == m_mapUsers.end())
        {
            m_mapUsers.insert(std::pair<std::wstring, CCloudCryptoDesktop>(sKey, info));
        }
        else
        {
            CCloudCryptoDesktop& old = find->second;
            old.GUID = info.GUID;
            old.User = info.User;
            old.Email = info.Email;
            old.Portal = info.Portal;
            old.PublicKey = info.PublicKey;
            old.PrivateKeyEnc = info.PrivateKeyEnc;
            old.PrivateKey = info.PrivateKey;
        }
        Save();
    }
    void Save()
    {
        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<users>");
        for (std::map<std::wstring, CCloudCryptoDesktop>::iterator iter = m_mapUsers.begin(); iter != m_mapUsers.end(); iter++)
            iter->second.Write(oBuilder);
        oBuilder.WriteString(L"</users>");

        NSFile::CFileBinary::SaveToFile(m_sPath, oBuilder.GetData());
    }
    ~CCloudCryptoApp()
    {
    }

    CCloudCryptoDesktop* GetInfo(const CCloudCryptoDesktop& info)
    {
        std::wstring sKey = info.User;
        NSCommon::makeLowerW(sKey);
        std::map<std::wstring, CCloudCryptoDesktop>::iterator find = m_mapUsers.find(sKey);
        if (find == m_mapUsers.end())
            return NULL;

        return &find->second;
    }
};

class CCloudCryptoTmpInfo
{
public:
    std::wstring m_sUser;
    std::wstring m_sPassword;
    std::wstring m_sEmail;
    std::wstring m_sDomain;

public:
    void Write(NSStringUtils::CStringBuilder& builder)
    {
        builder.WriteString(L"<info>");

        builder.WriteString(L"<user>");
        builder.WriteEncodeXmlString(m_sUser);
        builder.WriteString(L"</user>");

        builder.WriteString(L"<email>");
        builder.WriteEncodeXmlString(m_sEmail);
        builder.WriteString(L"</email>");

        builder.WriteString(L"<password>");
        builder.WriteEncodeXmlString(m_sPassword);
        builder.WriteString(L"</password>");

        builder.WriteString(L"<domain>");
        builder.WriteEncodeXmlString(m_sDomain);
        builder.WriteString(L"</domain>");

        builder.WriteString(L"</info>");
    }
};
class CCloudCryptoTmpInfoApp
{
public:
    std::wstring m_sPath;
    std::map<std::wstring, CCloudCryptoTmpInfo> m_mapTmpInfo;

public:
    CCloudCryptoTmpInfoApp(const std::wstring& path)
    {
        m_sPath = path;

        XmlUtils::CXmlNode node;
        if (node.FromXmlFile(path))
        {
            XmlUtils::CXmlNodes nodes;
            node.GetNodes(L"info", nodes);

            int count = nodes.GetCount();
            for (int i = 0; i < count; i++)
            {
                XmlUtils::CXmlNode nodeInfo;
                nodes.GetAt(i, nodeInfo);
                addInfo(nodeInfo.ReadValueString(L"user"), nodeInfo.ReadValueString(L"email"), nodeInfo.ReadValueString(L"password"), nodeInfo.ReadValueString(L"domain"));
            }
        }
    }
    void Save()
    {
        if (NSFile::CFileBinary::Exists(m_sPath))
            NSFile::CFileBinary::Remove(m_sPath);

        if (m_mapTmpInfo.empty())
            return;

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<users>");
        for (std::map<std::wstring, CCloudCryptoTmpInfo>::iterator iter = m_mapTmpInfo.begin(); iter != m_mapTmpInfo.end(); iter++)
            iter->second.Write(oBuilder);
        oBuilder.WriteString(L"</users>");

        NSFile::CFileBinary::SaveToFile(m_sPath, oBuilder.GetData());
    }

public:
    void addInfo(const std::wstring& user, const std::wstring& email, const std::wstring& password, const std::wstring& domain)
    {
        CCloudCryptoTmpInfo add;
        add.m_sUser = user; NSCommon::makeLowerW(add.m_sUser);
        add.m_sEmail = email; NSCommon::makeLowerW(add.m_sEmail);
        add.m_sPassword = password;
        add.m_sDomain = domain; NSCommon::makeLowerW(add.m_sDomain);

        std::wstring key = add.m_sEmail + add.m_sDomain;
        std::map<std::wstring, CCloudCryptoTmpInfo>::iterator find = m_mapTmpInfo.find(key);
        if (find == m_mapTmpInfo.end())
        {
            m_mapTmpInfo.insert(std::pair<std::wstring, CCloudCryptoTmpInfo>(key, add));
        }
        else
        {
            find->second.m_sUser = add.m_sUser;
            find->second.m_sEmail = add.m_sEmail;
            find->second.m_sPassword = add.m_sPassword;
            find->second.m_sDomain = add.m_sDomain;
        }
        Save();
    }
    void removeInfo(const std::wstring& email, const std::wstring& domain)
    {
        std::wstring key = email + domain;
        NSCommon::makeLowerW(key);
        std::map<std::wstring, CCloudCryptoTmpInfo>::iterator find = m_mapTmpInfo.find(key);
        if (find != m_mapTmpInfo.end())
            m_mapTmpInfo.erase(find);
        Save();
    }

    CCloudCryptoTmpInfo* getInfo(const std::wstring& email, const std::wstring& domain)
    {
        std::wstring key = email + domain;
        NSCommon::makeLowerW(key);
        std::map<std::wstring, CCloudCryptoTmpInfo>::iterator find = m_mapTmpInfo.find(key);
        if (find != m_mapTmpInfo.end())
            return &find->second;
        return NULL;
    }
};

#endif // APPLICATION_CLOUDCRYPTO_H
