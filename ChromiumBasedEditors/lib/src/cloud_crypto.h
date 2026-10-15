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

#ifndef APPLICATION_CLOUDCRYPTO_H
#define APPLICATION_CLOUDCRYPTO_H

#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "./utils.h"
#include "./Logger.h"
#include <map>

class CCloudCrypto
{
public:
    std::wstring GUID;           // id of the encryption plugin
    std::wstring User;
    std::wstring Email;
    std::wstring Portal;
    std::wstring PublicKey;      // user's public key
    std::wstring PrivateKeyEnc;  // private key encrypted with the portal password

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
            std::vector<XmlUtils::CXmlNode> nodes;
            node.GetNodes(L"info", nodes);

            size_t count = nodes.size();
            for (size_t i = 0; i < count; i++)
            {
                XmlUtils::CXmlNode & nodeInfo = nodes[i];

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
            std::vector<XmlUtils::CXmlNode> nodes;
            node.GetNodes(L"info", nodes);

            size_t count = nodes.size();
            for (size_t i = 0; i < count; i++)
            {
                XmlUtils::CXmlNode& nodeInfo = nodes[i];

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
