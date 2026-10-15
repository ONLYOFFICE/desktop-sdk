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

#ifndef APPLICATION_KEYCHAIN_H
#define APPLICATION_KEYCHAIN_H

#include <string>
#include <string.h>

#define ASC_ENCRYPTED_USER_MASK     0x54434E45
#define ASC_ENCRYPTED_USER_VERSION  1

namespace NSAscCrypto
{
    class CCryptoKey
    {
    public:
        unsigned char*  data;
        int             len;

    public:
        CCryptoKey()
        {
            data = NULL;
            len = 0;
        }

        CCryptoKey(const CCryptoKey& oSrc)
        {
            data = NULL;
            len = oSrc.len;
            if (0 != len)
            {
                data = new unsigned char[len];
                memcpy(data, oSrc.data, len);
            }
        }

        CCryptoKey& operator =(const CCryptoKey& oSrc)
        {
            if (data)
                delete [] data;
            len = oSrc.len;
            if (0 != len)
            {
                data = new unsigned char[len];
                memcpy(data, oSrc.data, len);
            }
            return *this;
        }

        void Create(int length)
        {
            len = length;
            data = new unsigned char[len];
        }

        ~CCryptoKey()
        {
            if (data)
                delete [] data;
        }
    };

    class IAscKeyChainListener
    {
    public:
        // keyEnc - this is the one stored on disk
        // keyDec - this is the plain key
        virtual void OnKeyChainComplete(CCryptoKey& keyEnc, CCryptoKey& keyDec) = 0;
    };

    class CAscKeychain
    {
    public:
        IAscKeyChainListener* m_pListener;

    public:
        CAscKeychain(IAscKeyChainListener* pListener) { m_pListener = pListener; }
        virtual ~CAscKeychain() {}

        void Check(const std::wstring& sFile);

    public:
        virtual void Load(const std::string& name, CCryptoKey& keyEnc);
        virtual void Save(const std::string& name, CCryptoKey& keyDec);
    };
}

#endif // APPLICATION_KEYCHAIN_H
