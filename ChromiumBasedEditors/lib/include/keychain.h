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
        // keyEnc - это тот, что хранится на диске
        // keyDec - это натуральный ключ
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
