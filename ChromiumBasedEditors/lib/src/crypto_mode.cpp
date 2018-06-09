/*
 * (c) Copyright Ascensio System SIA 2010-2017
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

#include "crypto_mode.h"
#include "aes256.cpp"
#include "./../../../../core/DesktopEditor/common/File.h"

#ifdef WIN32
#include "Wincrypt.h"
#endif

CCryptoMode::CCryptoMode()
{
    m_sPassword = L"";
    m_nMode = 0;
}

void CCryptoMode::Save(const std::wstring& sPassFile)
{
    std::string sPass = U_TO_UTF8(m_sPassword);

    if (NSFile::CFileBinary::Exists(sPassFile))
        NSFile::CFileBinary::Remove(sPassFile);

    NSFile::CFileBinary oFile;
    oFile.CreateFileW(sPassFile);

    // encryption key (random)
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int alphanumlen = alphanum.length();
    ByteArray keyEncArray;
    BYTE keyEncArrayBuffer[256];
    for (int i = 0; i < 256; ++i)
    {
        keyEncArrayBuffer[i] = (BYTE)alphanum[rand() % (alphanumlen - 1)];
        keyEncArray.push_back(keyEncArrayBuffer[i]);
    }

    // password utf8 => vector<BYTE>
    int nLenPass = (int)sPass.length();
    ByteArray passArray;
    for (int i = 0; i < nLenPass; ++i)
    {
        passArray.push_back((BYTE)sPass.c_str()[i]);
    }

    // encode
    ByteArray passEnc;
    Aes256::encrypt(keyEncArray, passArray, passEnc);

    BYTE* keyEncArrayCrypt = NULL;
    int keyEncArrayCryptLen = 0;

    // crypt encKey!!!
#ifdef WIN32
    DATA_BLOB DataIn;
    DataIn.pbData = keyEncArrayBuffer;
    DataIn.cbData = 256;

    DATA_BLOB DataOut;
    CryptProtectData(&DataIn, L"", NULL, NULL, NULL, 0, &DataOut);
    keyEncArrayCrypt = DataOut.pbData;
    keyEncArrayCryptLen = DataOut.cbData;
#endif

#if defined(_LINUX) && !defined(_MAC)
    keyEncArrayCrypt = keyEncArrayBuffer;
    keyEncArrayCryptLen = 256;
    // TODO
#endif

#ifdef _MAC
    keyEncArrayCrypt = keyEncArrayBuffer;
    keyEncArrayCryptLen = 256;
    // TODO
#endif

    // Save to file
    int nFileSize = 4 + keyEncArrayCryptLen + 4 + (int)passEnc.size() + 1;
    BYTE* pFileData = new BYTE[nFileSize];

    int nIndex = 0;
    int nIntValue = keyEncArrayCryptLen;
    pFileData[nIndex++] = nIntValue & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 8) & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 16) & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 24) & 0xFF;

    memcpy(pFileData + nIndex, keyEncArrayCrypt, keyEncArrayCryptLen);
    nIndex += keyEncArrayCryptLen;

    nIntValue = (int)passEnc.size();
    pFileData[nIndex++] = nIntValue & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 8) & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 16) & 0xFF;
    pFileData[nIndex++] = (nIntValue >> 24) & 0xFF;

    int nSize = (int)passEnc.size();
    for (int i = 0; i < nSize; ++i)
        pFileData[nIndex++] = passEnc[i];

    pFileData[nIndex++] = m_nMode;

    oFile.WriteFile(pFileData, (DWORD)nFileSize);
    oFile.CloseFile();

    RELEASEARRAYOBJECTS(pFileData);

#ifdef WIN32
    LocalFree((HLOCAL)DataOut.pbData);
#endif
}

void CCryptoMode::Load(const std::wstring& sPassFile)
{
    NSFile::CFileBinary oFile;
    if (!oFile.OpenFile(sPassFile))
    {
        m_sPassword = L"";
        m_nMode = 0;
        return;
    }

    int nFileSize = (int)oFile.GetFileSize();
    BYTE* pFileData = new BYTE[nFileSize];
    DWORD dwRead = 0;
    oFile.ReadFile(pFileData, (DWORD)nFileSize, dwRead);

    // encryption key
    int nIndex = 0;
    int nLenEncKey = (pFileData[nIndex]) | (pFileData[nIndex+1] << 8) | (pFileData[nIndex+2] << 16) | (pFileData[nIndex+3] << 24);
    nIndex += 4;

    // decrypt encKey!!!
    BYTE* keyEncArrayCrypto = pFileData + nIndex;
    BYTE* keyEncArrayBuffer = NULL;
    int keyEncArrayBufferLen = 0;
    nIndex += nLenEncKey;

#ifdef WIN32
    DATA_BLOB DataIn;
    DataIn.pbData = keyEncArrayCrypto;
    DataIn.cbData = nLenEncKey;

    DATA_BLOB DataOut;
    LPWSTR pDescrOut =  NULL;
    CryptUnprotectData(&DataIn, &pDescrOut, NULL, NULL, NULL, 0, &DataOut);
    keyEncArrayBuffer = DataOut.pbData;
    keyEncArrayBufferLen = DataOut.cbData;
#endif

#if defined(_LINUX) && !defined(_MAC)
    keyEncArrayCrypt = keyEncArrayBuffer;
    keyEncArrayCryptLen = nLenEncKey;
    // TODO
#endif

#ifdef _MAC
    keyEncArrayCrypt = keyEncArrayBuffer;
    keyEncArrayCryptLen = nLenEncKey;
    // TODO
#endif

    ByteArray keyEncArray;
    for (int i = 0; i < keyEncArrayBufferLen; ++i)
    {
        keyEncArray.push_back(keyEncArrayBuffer[i]);
    }

    int nLenPass = (pFileData[nIndex]) | (pFileData[nIndex+1] << 8) | (pFileData[nIndex+2] << 16) | (pFileData[nIndex+3] << 24);
    nIndex += 4;

    ByteArray passArray;
    for (int i = 0; i < nLenPass; ++i)
    {
        passArray.push_back(pFileData[nIndex++]);
    }

    m_nMode = pFileData[nIndex++];

    // decrypt encKey!!!
    // TODO:

    // decode
    ByteArray passDec;
    Aes256::decrypt(keyEncArray, passArray, passDec);

    int nLen = (int)passDec.size();
    char* pDst = new char[nLen + 1];
    for (int i = 0; i < nLen; ++i)
        pDst[i] = (char)passDec[i];
    pDst[nLen] = 0;

    std::string sPassD(pDst);
    m_sPassword = UTF8_TO_U(sPassD);

    RELEASEARRAYOBJECTS(pDst);

#ifdef WIN32
    LocalFree((HLOCAL)DataOut.pbData);
#endif
}
