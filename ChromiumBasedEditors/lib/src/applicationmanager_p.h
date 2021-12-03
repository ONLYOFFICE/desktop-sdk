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

#ifndef APPLICATION_MANAGER_PRIVATE_H
#define APPLICATION_MANAGER_PRIVATE_H

#include "../include/applicationmanager.h"
#include <map>

#include "../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "../../../../core/DesktopEditor/graphics/BaseThread.h"
#include "../../../../core/DesktopEditor/graphics/Timer.h"

#include "../src/cookiesworker.h"

#include <iostream>
#include <set>

#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "./cloud_crypto.h"

#define ASC_CONSTANT_CANCEL_SAVE 0x00005678
#define LOCK_CS_SCRIPT 0

#include "./additional/manager.h"
#include "./additional/renderer.h"

#include "crypto_mode.h"
#include "plugins.h"
#include "providers.h"

#include "utils.h"
#include "../../../../core/DesktopEditor/xmlsec/src/include/XmlCertificate.h"

#ifdef CEF_2623
#define MESSAGE_IN_BROWSER
#else
// с версии выше 74 - убрать определение
//#define MESSAGE_IN_BROWSER
#endif

#ifdef MESSAGE_IN_BROWSER
#define SEND_MESSAGE_TO_BROWSER_PROCESS(message) CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, message)
#define SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message) browser->SendProcessMessage(PID_RENDERER, message)
#else
#define SEND_MESSAGE_TO_BROWSER_PROCESS(message) CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, message)
#define SEND_MESSAGE_TO_RENDERER_PROCESS(browser, message) browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message)
#endif

#define NO_CACHE_WEB_CLOUD_SCRIPTS

#ifdef LINUX
#include "signal.h"
void posix_death_signal(int signum);

class CLinuxData
{
public:
    static CApplicationCEF*         app_cef;
    static CAscApplicationManager*  app_manager;

public:
    static void Check(CAscApplicationManager* manager)
    {
        if (NULL == app_manager)
        {
            app_manager = manager;
            signal(SIGSEGV, posix_death_signal);
            signal(SIGTERM, posix_death_signal);
        }
    }
    static void Close()
    {
        if (NULL != app_manager)
        {
            app_manager->CloseApplication();
        }
    }
};

#endif

namespace NSStringUtils
{
    // locale independent (simple double convert)
    static double GetDouble(const std::string& sValue)
    {
        char delim = '.';

        std::string::size_type posDelim = sValue.find(delim);
        if (posDelim == std::wstring::npos)
        {
            delim = ',';
            posDelim = sValue.find(delim);
        }

        if (std::string::npos == posDelim)
        {
            return std::stod(sValue);
        }

        std::string sUnLocale = sValue;
        sUnLocale.erase(posDelim, 1);

        double dIntValue = (double)std::stoi(sUnLocale);
        int nCount10 = sValue.length() - posDelim - 1;
        for (int i = 0; i < nCount10; ++i)
            dIntValue = (dIntValue / 10);

        return dIntValue;
    }
}

namespace NSVersion
{
    static int GetMajorVersion(const std::string& sVersion)
    {
        if (sVersion.empty())
            return 0;
        std::string::size_type pos = sVersion.find_first_of(".,");
        int nVersion = 0;
        if (pos != std::string::npos)
        {
            try
            {
                nVersion = std::stoi(sVersion.substr(0, pos));
            }
            catch (...)
            {
                nVersion = 0;
            }
        }
        return nVersion;
    }
}

namespace NSCommon
{
    static std::wstring GetBaseDomain(const std::wstring& url, bool bIsHeader = false)
    {
        std::wstring::size_type pos1 = url.find(L"//");
        if (std::wstring::npos == pos1)
            pos1 = 0;
        pos1 += 2;

        std::wstring::size_type pos2 = url.find(L"/", pos1);
        if (std::wstring::npos == pos2)
            return L"";

        if (!bIsHeader)
            return url.substr(pos1, pos2 - pos1);
        return url.substr(0, pos2);
    }

    static int CorrectSaveFormat(const int& nFormat)
    {
        switch (nFormat)
        {
        case 2305:
            return AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA;
        default:
            break;
        }
        return nFormat;
    }
}

class CAscReporterData
{
public:
    int Id;
    int ParentId;
    std::wstring Url;
    std::wstring LocalRecoverFolder;
    std::string Version;

    std::string CloudCryptoInfo;

public:

    CAscReporterData()
    {
        Id = -1;
        ParentId = -1;
    }
};

#ifdef _MAC
#define DISABLE_LOCK_FUNCTIONALITY
#endif

namespace NSSystem
{
#ifndef _WIN32
#include <fcntl.h>
#endif

    class CLocalFileLocker
    {
    private:
        std::wstring m_sFile;
#ifdef _WIN32
        HANDLE m_nDescriptor;
#else
        int m_nDescriptor;
#endif

    public:
        CLocalFileLocker(const std::wstring& sFile)
        {
#ifdef _WIN32
            m_nDescriptor = INVALID_HANDLE_VALUE;
#else
            m_nDescriptor = -1;
#endif

            if (sFile.empty())
                return;

            m_sFile = sFile;
            Lock();
        }
        bool Lock()
        {
            if (!IsSupportFunctionality())
                return true;

            Unlock();
#ifdef _WIN32
            std::wstring sFileFull = CorrectPathW(m_sFile);
            DWORD dwFileAttributes = 0;//GetFileAttributesW(sFileFull.c_str());
            m_nDescriptor = CreateFileW(sFileFull.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFileAttributes, NULL);
            if (m_nDescriptor != NULL && m_nDescriptor != INVALID_HANDLE_VALUE)
            {
                //LARGE_INTEGER lFileSize;
                //GetFileSizeEx(m_nDescriptor, &lFileSize);
                //LockFile(m_nDescriptor, 0, 0, lFileSize.LowPart, (DWORD)lFileSize.HighPart);
            }
            else
            {
                m_nDescriptor = INVALID_HANDLE_VALUE;
            }
#else
            std::string sFileA = U_TO_UTF8(m_sFile);
            m_nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
            if (-1 == m_nDescriptor)
                return true;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            _lock.l_type   = F_WRLCK;
            _lock.l_whence = SEEK_SET;
            _lock.l_start  = 0;
            _lock.l_len    = 0;
            _lock.l_pid    = getpid();

            fcntl(m_nDescriptor, F_SETLKW, &_lock);
#endif
            return true;
        }
        bool Unlock()
        {
            if (!IsSupportFunctionality())
                return true;

#ifdef _WIN32
            if (INVALID_HANDLE_VALUE != m_nDescriptor)
            {
                LARGE_INTEGER lFileSize;
                GetFileSizeEx(m_nDescriptor, &lFileSize);
                UnlockFile(m_nDescriptor, 0, 0, lFileSize.LowPart, (DWORD)lFileSize.HighPart);

                CloseHandle(m_nDescriptor);
                m_nDescriptor = INVALID_HANDLE_VALUE;
            }            
#else
            if (-1 == m_nDescriptor)
                return true;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            _lock.l_type   = F_UNLCK;
            _lock.l_whence = SEEK_SET;
            _lock.l_start  = 0;
            _lock.l_len    = 0;
            _lock.l_pid    = getpid();

            fcntl(m_nDescriptor, F_SETLKW, &_lock);
            close(m_nDescriptor);

            m_nDescriptor = -1;
#endif
            return true;
        }
        ~CLocalFileLocker()
        {
            Unlock();
        }

        static bool IsLocked(const::std::wstring& sFile)
        {
            if (!IsSupportFunctionality())
                return false;

            bool isLocked = false;
#ifdef _WIN32
            HANDLE hFile = CreateFileW(sFile.c_str(),                   // открываемый файл
                                        GENERIC_READ | GENERIC_WRITE,   // открываем для чтения и записи
                                        0,                              // для совместного чтения
                                        NULL,                           // защита по умолчанию
                                        OPEN_EXISTING,                  // только существующий файл
                                        FILE_ATTRIBUTE_NORMAL,          // обычный файл
                                        NULL);                          // атрибутов шаблона нет
            if (hFile == INVALID_HANDLE_VALUE)
            {
                if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(sFile.c_str()))
                    isLocked = true;
            }
            CloseHandle(hFile);
#else
            std::string sFileA = U_TO_UTF8(sFile);
            int nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
            if (-1 == nDescriptor)
                return false;

            struct flock _lock;
            memset(&_lock, 0, sizeof(_lock));
            fcntl(nDescriptor, F_GETLK, &_lock);
            if (F_WRLCK == (_lock.l_type & F_WRLCK))
                isLocked = true;
            close(nDescriptor);
#endif
            return isLocked;
        }

        std::wstring GetFileLocked()
        {
            return m_sFile;
        }

        static bool IsSupportFunctionality()
        {
#ifdef DISABLE_LOCK_FUNCTIONALITY
            return false;
#else
            return true;
#endif
        }

        bool SaveFile(const std::wstring& sFile)
        {
            DWORD nSection = 10 * 1024 * 1024;
            DWORD nFileSize = 0;

            NSFile::CFileBinary oFile;
            if (!oFile.OpenFile(sFile))
                return false;

            nFileSize = (DWORD)oFile.GetFileSize();

            DWORD nChunkSize = nSection;
            DWORD nNeedWrite = nFileSize;
            BYTE* pMemoryBuffer = NULL;
            if (nFileSize > nSection)
            {
                pMemoryBuffer = new BYTE[nSection];
            }
            else
            {
                nChunkSize = nFileSize;
                pMemoryBuffer = new BYTE[nFileSize];
            }

            bool bRes = true;
#ifdef _WIN32
            SetFilePointer(m_nDescriptor, 0, 0, FILE_BEGIN);
#else
            lseek(m_nDescriptor, 0, SEEK_SET);
#endif

            DWORD dwRead = 0;
            DWORD dwWrite = 0;
            while (nNeedWrite > 0)
            {
                if (nNeedWrite < nChunkSize)
                    nChunkSize = nNeedWrite;
                oFile.ReadFile(pMemoryBuffer, nChunkSize, dwRead);

#ifdef _WIN32
                WriteFile(m_nDescriptor, pMemoryBuffer, nChunkSize, &dwWrite, NULL);
#else
                dwWrite = (DWORD)write(m_nDescriptor, pMemoryBuffer, nChunkSize);
#endif

                if (dwRead != nChunkSize || dwWrite != nChunkSize)
                {
                    bRes = false;
                    break;
                }

                nNeedWrite -= nChunkSize;
            }

#ifdef _WIN32
            SetFilePointer(m_nDescriptor, (LONG)nFileSize, 0, FILE_BEGIN);
            SetEndOfFile(m_nDescriptor);
#else
            lseek(m_nDescriptor, (DWORD)nFileSize, SEEK_SET);
            ftruncate(m_nDescriptor, (DWORD)nFileSize);
#endif

            oFile.CloseFile();
            RELEASEARRAYOBJECTS(pMemoryBuffer);

            if (!bRes)
            {
                Unlock();
                if (NSFile::CFileBinary::Exists(m_sFile))
                    NSFile::CFileBinary::Remove(m_sFile);
                bRes = NSFile::CFileBinary::Copy(sFile, m_sFile);
                Lock();
            }

            return bRes;
        }
    };

    class CLocalFilesResolver
    {
    private:
        class CResolveFolder
        {
        public:
            std::wstring m_sFolder;
            std::vector<std::wstring> m_arFilesExclude;

        public:
            CResolveFolder(const std::wstring& sFolder)
            {
                m_sFolder = sFolder;
            }
            CResolveFolder(const std::wstring& sFolder, const std::vector<std::wstring>& arExcludes)
            {
                m_sFolder = sFolder;
                m_arFilesExclude = arExcludes;
            }

            bool Check(const std::wstring& sDir, const std::wstring& sFile)
            {
                if (0 != sDir.find(m_sFolder))
                    return false;

                if (0 == m_arFilesExclude.size())
                    return true;

                std::wstring sFileName = NSFile::GetFileName(sFile);
                for (std::vector<std::wstring>::iterator i = m_arFilesExclude.begin(); i != m_arFilesExclude.end(); i++)
                {
                    if (*i == sFileName)
                        return false;
                }

                return true;
            }
        };

    public:
        std::wstring m_sFontsFolder;
        std::set<std::wstring> m_arFontsFolders;

        std::vector<CResolveFolder> m_arFolders;
        std::set<std::wstring> m_arFilesFromDialog;

        bool m_bIsLocalUrl;

    public:
        CLocalFilesResolver()
        {
            m_bIsLocalUrl = false;
        }
        ~CLocalFilesResolver()
        {
        }

    public:

        void Init(const std::wstring& sFonts, const std::wstring& sRecovers = L"")
        {
            m_sFontsFolder = CorrectDir(sFonts);
            AddDir(sRecovers);
        }

        void AddDir(const std::wstring& sFileDir)
        {
            if (!sFileDir.empty())
                m_arFolders.push_back(CResolveFolder(CorrectDir(sFileDir)));
        }
        void AddDir(const std::wstring& sFileDir, const std::vector<std::wstring>& arExcludes)
        {
            if (!sFileDir.empty())
                m_arFolders.push_back(CResolveFolder(CorrectDir(sFileDir), arExcludes));
        }

        void AddFile(const std::wstring& sFilePath)
        {
            std::wstring sFile = CorrectPath(sFilePath);
            m_arFilesFromDialog.insert(sFile);
        }

        void CheckUrl(const std::wstring& sUrl)
        {
            m_bIsLocalUrl = false;
            if (0 == sUrl.find(L"file:/"))
                m_bIsLocalUrl = true;
        }

    public:
        bool CheckFont(const std::wstring& sFilePath)
        {
            if (m_bIsLocalUrl)
                return true;

            if (0 == m_arFontsFolders.size())
            {
                std::wstring strFontsCheckPath = m_sFontsFolder + L"/fonts.log";
                std::wstring sLastDir = L"";

                NSFile::CFileBinary oFile;
                if (oFile.OpenFile(strFontsCheckPath))
                {
                    int nSize = oFile.GetFileSize();
                    char* pBuffer = new char[nSize];
                    DWORD dwReaden = 0;
                    oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
                    oFile.CloseFile();

                    int nStart = 0;
                    int nCur = nStart;
                    for (; nCur < nSize; ++nCur)
                    {
                        if (pBuffer[nCur] == '\n')
                        {
                            int nEnd = nCur - 1;
                            if (nEnd > nStart)
                            {
                                int nSymbolsCount = nEnd - nStart + 1;
                            #ifdef _WIN32
                                for (int i = 0; i < nSymbolsCount; ++i)
                                {
                                    if (pBuffer[nStart + i] == '\\')
                                        pBuffer[nStart + i] = '/';
                                }
                            #endif

                                bool bIsAdd = true;
                                if (sLastDir.empty())
                                {
                                    std::string sHeader(pBuffer + nStart, nEnd - nStart + 1);
                                    if (0 == sHeader.find("ONLYOFFICE_FONTS_VERSION_"))
                                        bIsAdd = false;
                                }

                                if (bIsAdd)
                                {
                                    int nLast = nEnd;
                                    while (nLast > nStart && pBuffer[nLast] != '/')
                                        --nLast;

                                    std::wstring sDir = NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)pBuffer + nStart, nLast - nStart);
                                    if (sDir != sLastDir)
                                        m_arFontsFolders.insert(sDir);

                                }
                            }
                            nStart = nCur + 1;
                        }
                    }

                    delete[] pBuffer;
                }
            }

            std::wstring sFile = CorrectPath(sFilePath);
            std::wstring sDir = NSFile::GetDirectoryName(sFile);
            return (m_arFontsFolders.end() != m_arFontsFolders.find(sDir)) ? true : false;
        }
        bool CheckNoFont(const std::wstring& sFilePath)
        {
            if (m_bIsLocalUrl)
                return true;

            std::wstring sFile = CorrectPath(sFilePath);
            std::wstring sDir = NSFile::GetDirectoryName(sFile);
            for (std::vector<CResolveFolder>::iterator i = m_arFolders.begin(); i != m_arFolders.end(); i++)
            {
                if (i->Check(sDir, sFile))
                    return true;
            }

            return (m_arFilesFromDialog.end() != m_arFilesFromDialog.find(sFile)) ? true : false;
        }
        bool Check(const std::wstring& sFile)
        {
            if (CheckNoFont(sFile))
                return true;
            return CheckFont(sFile);
        }

    private:
        std::wstring CorrectPath(const std::wstring& sFilePath)
        {
            std::wstring sFile = sFilePath;
        #ifdef _WIN32
            for (std::wstring::size_type pos = 0, size = sFile.length(); pos < size; ++pos)
            if (sFile[pos] == '\\')
                sFile[pos] = '/';
        #endif
            NSCommon::url_correct(sFile);

            if (0 == sFile.find(L"file://"))
            {
                sFile = sFile.substr(7);
            #ifdef _WIN32
                if (0 == sFile.find(L"/"))
                    sFile.erase(0, 1);
            #endif
            }

            return sFile;
        }
        std::wstring CorrectDir(const std::wstring& sDirPath)
        {
            std::wstring sDir = CorrectPath(sDirPath);
            if (!sDir.empty() && (sDir.back() == '/'))
                sDir.pop_back();
            return sDir;
        }
    };
}

class COfficeFileFormatCheckerWrapper : public COfficeFileFormatChecker
{
public:
    int nFileType2;
public:
    COfficeFileFormatCheckerWrapper() : COfficeFileFormatChecker()
    {
        nFileType2 = nFileType;
    }
public:
    bool isOfficeFile2(const std::wstring& fileName, bool isCheckLocal = false)
    {
        // check empty file!!!
        NSFile::CFileBinary oFile;
        if (oFile.OpenFile(fileName))
        {
            long nFileSize = oFile.GetFileSize();
            if (0 == nFileSize)
            {
                std::wstring sFileExt = NSFile::GetFileExtention(fileName);
                NSCommon::makeLowerW(sFileExt);
                int nFormat = GetFormatByExtension(L"." + sFileExt);

                switch (nFormat)
                {
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX:
                    case AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT:
                    case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:
                    case AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP:
                    case AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX:
                    case AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX:
                    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS:
                    {
                        this->nFileType = nFormat;
                        this->nFileType2 = this->nFileType;
                        return true;
                    }
                }
            }
        }
        bool isOfficeFileBase = isOfficeFile(fileName);

        if (isCheckLocal)
        {
            // check locked files
            std::wstring sFileExt = NSFile::GetFileExtention(fileName);
            NSCommon::makeLowerW(sFileExt);
            int nFormat = GetFormatByExtension(L"." + sFileExt);
            if (nFormat != 0)
            {
                if (NSSystem::CLocalFileLocker::IsLocked(fileName))
                {
                    std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"TMP");
                    if (NSFile::CFileBinary::Exists(sTmpFile))
                        NSFile::CFileBinary::Remove(sTmpFile);
                    sTmpFile += (L"." + NSFile::GetFileExtention(fileName));
                    NSFile::CFileBinary::Copy(fileName, sTmpFile);
                    isOfficeFileBase = isOfficeFile(sTmpFile);
                    NSFile::CFileBinary::Remove(sTmpFile);
                }
            }
        }

        if (!isOfficeFileBase)
            return false;

        this->nFileType2 = this->nFileType;
        switch (this->nFileType)
        {
            case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
            {
                std::wstring sFileExt = NSFile::GetFileExtention(fileName);
                NSCommon::makeLowerW(sFileExt);
                if (L"oform" == sFileExt)
                    this->nFileType2 = AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM;
                else if (L"docxf" == sFileExt)
                    this->nFileType2 = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF;
                break;
            }
            case AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT:
            {
                if (!IsOpenAsTxtFile(fileName))
                    isOfficeFileBase = false;
                break;
            }
            case AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO:
            {
                std::wstring sExt = NSFile::GetFileExtention(fileName);
                this->nFileType2 = COfficeFileFormatChecker::GetFormatByExtension(L"." + sExt);
                break;
            }
            default:
                break;
        }

#ifdef DISABLE_OFORM_SUPPORT
        if (this->nFileType2 == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM ||
            this->nFileType2 == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF)
        {
            this->nFileType2 = AVS_OFFICESTUDIO_FILE_UNKNOWN;
            return false;
        }
#endif

        return isOfficeFileBase;
    }

    bool IsOpenAsTxtFile(const std::wstring& fileName)
    {
        std::wstring sExt = NSFile::GetFileExtention(fileName);
        NSCommon::makeLowerW(sExt);
        if (sExt == L"txt" || sExt == L"xml" || sExt == L"md")
            return true;
        return false;
    }
};

// -------------------------------------------------------
#define SDKJS_ADDONS_VERSION 1
class CSdkjsAddons
{
private:
    std::wstring m_sPath;
public:
    CSdkjsAddons(const std::wstring& sUserPlugins)
    {
        m_sPath = sUserPlugins + L"/addons";
    }

    std::vector<std::string> GetFeaturesArray()
    {
        std::vector<std::string> retArray;
        int nCounterVersion = 0;
        NSFile::CFileBinary oFile;
        if (oFile.OpenFile(m_sPath))
        {
            int nSize = oFile.GetFileSize();
            char* pBuffer = new char[nSize];
            DWORD dwReaden = 0;
            oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
            oFile.CloseFile();

            int nStart = 0;
            int nCur = nStart;
            for (; nCur < nSize; ++nCur)
            {
                if (pBuffer[nCur] == '\n')
                {
                    int nEnd = nCur - 1;
                    if (nEnd >= nStart)
                    {
                        ++nCounterVersion;
                        if (2 < nCounterVersion)
                        {
                            std::string s(pBuffer + nStart, nEnd - nStart + 1);
                            retArray.push_back(s);
                        }
                    }
                    nStart = nCur + 1;
                }
            }

            delete[] pBuffer;
        }
        return retArray;
    }

    std::string GetFeaturesJSArray()
    {
        std::vector<std::string> arrFeatures = GetFeaturesArray();
        std::string sFeatures = "[";
        int nCounter = 0;
        for (std::vector<std::string>::const_iterator iter = arrFeatures.begin(); iter != arrFeatures.end(); iter++, nCounter++)
        {
            if (0 != nCounter)
                sFeatures += ",";
            sFeatures += "'";
            sFeatures += *iter;
            sFeatures += "'";
        }
        sFeatures += "]";
        return sFeatures;
    }

    void CheckVersion(const std::wstring& sDirectory)
    {
        std::string sHashLocal = "";
        ICertificate* pCert = ICertificate::CreateInstance();

        sHashLocal += pCert->GetHash(sDirectory + L"/word/sdk-all-min.js", OOXML_HASH_ALG_SHA256);
        sHashLocal += pCert->GetHash(sDirectory + L"/slide/sdk-all-min.js", OOXML_HASH_ALG_SHA256);
        sHashLocal += pCert->GetHash(sDirectory + L"/cell/sdk-all-min.js", OOXML_HASH_ALG_SHA256);

        delete pCert;

        // check old file
        bool bIsNewVersion = false;
        int nCounterVersion = 0;
        NSFile::CFileBinary oFile;
        if (oFile.OpenFile(m_sPath))
        {
            int nSize = oFile.GetFileSize();
            char* pBuffer = new char[nSize];
            DWORD dwReaden = 0;
            oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
            oFile.CloseFile();

            int nStart = 0;
            int nCur = nStart;
            for (; nCur < nSize; ++nCur)
            {
                if (pBuffer[nCur] == '\n')
                {
                    int nEnd = nCur - 1;
                    if (nEnd >= nStart)
                    {
                        std::string s(pBuffer + nStart, nEnd - nStart + 1);
                        ++nCounterVersion;
                        if (1 == nCounterVersion)
                        {
                            if (SDKJS_ADDONS_VERSION != std::stoi(s))
                            {
                                bIsNewVersion = true;
                                break;
                            }

                        }
                        else if (2 == nCounterVersion)
                        {
                            if (sHashLocal != s)
                                bIsNewVersion = true;
                            break;
                        }
                    }
                    nStart = nCur + 1;
                }
            }

            if (nCounterVersion < 2)
                bIsNewVersion = true;

            delete[] pBuffer;
        }
        else
        {
            bIsNewVersion = true;
        }

        if (bIsNewVersion)
        {
            if (NSFile::CFileBinary::Exists(m_sPath))
                NSFile::CFileBinary::Remove(m_sPath);

            std::string sContent = std::to_string((int)SDKJS_ADDONS_VERSION);
            sContent += "\n";

            sContent += sHashLocal;
            sContent += "\n";

            NSFile::CFileBinary oFileDst;
            oFileDst.CreateFileW(m_sPath);
            oFileDst.WriteFile((BYTE*)sContent.c_str(), (DWORD)sContent.size());
            oFileDst.CloseFile();
        }
    }

    std::string CheckCloud(CefRefPtr<CefV8Context> context, bool& bIsChanged)
    {
        std::string sFeatures;

        CefRefPtr<CefV8Value> retval;
        CefRefPtr<CefV8Exception> exception;
        bool bValid = false;
        bIsChanged = false;

        bValid = context->Eval("(function() { if (!window.Asc || !window.Asc.Addons) return ''; var features = ''; for (var i in window.Asc.Addons) { var j = i; if (j === \"content-\\u0441ontrols\") j = \"content-controls\"; features += (j + ' '); } return features; })();",
#ifndef CEF_2623
    "", 0,
#endif
retval, exception);

        if (bValid && retval && retval->IsString())
            sFeatures = retval->GetStringValue().ToString();

        std::map<std::string, bool> mapFeatures;

        // check old file
        int nCounterVersion = 0;
        std::string sSHA;
        NSFile::CFileBinary oFile;
        if (oFile.OpenFile(m_sPath))
        {
            int nSize = oFile.GetFileSize();
            char* pBuffer = new char[nSize];
            DWORD dwReaden = 0;
            oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
            oFile.CloseFile();

            int nStart = 0;
            int nCur = nStart;
            for (; nCur < nSize; ++nCur)
            {
                if (pBuffer[nCur] == '\n')
                {
                    int nEnd = nCur - 1;
                    if (nEnd >= nStart)
                    {
                        std::string s(pBuffer + nStart, nEnd - nStart + 1);
                        ++nCounterVersion;
                        if (1 == nCounterVersion)
                        {
                            if (SDKJS_ADDONS_VERSION != std::stoi(s))
                                break;
                        }
                        else if (2 == nCounterVersion)
                        {
                            sSHA = s;
                        }
                        else
                        {
                            mapFeatures.insert(std::pair<std::string, bool>(s, true));
                        }
                    }
                    nStart = nCur + 1;
                }
            }

            delete[] pBuffer;
        }
        else
        {
            // такого быть не должно
            return sFeatures;
        }

        // check cloud
        bool bIsNewFeatures = false;
        if (true)
        {
            int nSize = (int)sFeatures.length();
            const char* pBuffer = sFeatures.c_str();

            int nStart = 0;
            int nCur = nStart;
            for (; nCur < nSize; ++nCur)
            {
                if (pBuffer[nCur] == ' ')
                {
                    int nEnd = nCur - 1;
                    if (nEnd > nStart)
                    {
                        std::string s(pBuffer + nStart, nEnd - nStart + 1);
                        if (mapFeatures.find(s) == mapFeatures.end())
                        {
                            mapFeatures.insert(std::pair<std::string, bool>(s, true));
                            bIsNewFeatures = true;
                        }
                    }
                    nStart = nCur + 1;
                }
            }
        }

        if (bIsNewFeatures)
        {
            if (NSFile::CFileBinary::Exists(m_sPath))
                NSFile::CFileBinary::Remove(m_sPath);

            std::string sContent = std::to_string((int)SDKJS_ADDONS_VERSION);
            sContent += "\n";

            sContent += sSHA;
            sContent += "\n";

            for (std::map<std::string, bool>::iterator iter = mapFeatures.begin(); iter != mapFeatures.end(); iter++)
            {
                sContent += iter->first;
                sContent += "\n";
            }

            NSFile::CFileBinary oFileDst;
            oFileDst.CreateFileW(m_sPath);
            oFileDst.WriteFile((BYTE*)sContent.c_str(), (DWORD)sContent.size());
            oFileDst.CloseFile();

            bIsChanged = true;
        }

        return sFeatures;
    }

    void CheckLocal(CefRefPtr<CefV8Context> context)
    {
        std::string sFeatures;

        // check file
        int nCounterVersion = 0;
        NSFile::CFileBinary oFile;
        if (oFile.OpenFile(m_sPath))
        {
            int nSize = oFile.GetFileSize();
            char* pBuffer = new char[nSize];
            DWORD dwReaden = 0;
            oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
            oFile.CloseFile();

            int nStart = 0;
            int nCur = nStart;
            for (; nCur < nSize; ++nCur)
            {
                if (pBuffer[nCur] == '\n')
                {
                    int nEnd = nCur - 1;
                    if (nEnd >= nStart)
                    {
                        std::string s(pBuffer + nStart, nEnd - nStart + 1);
                        ++nCounterVersion;
                        if (1 == nCounterVersion)
                        {
                            if (SDKJS_ADDONS_VERSION != std::stoi(s))
                                break;
                        }
                        else if (2 == nCounterVersion)
                        {
                            // SHA
                        }
                        else
                        {
                            sFeatures += (s + " ");
                        }
                    }
                    nStart = nCur + 1;
                }
            }

            delete[] pBuffer;
        }

        CefRefPtr<CefV8Value> retval;
        CefRefPtr<CefV8Exception> exception;

        std::string sCode = "(function(features){\n\
var addons = window.Asc ? window.Asc.Addons : null;\n\
if (!addons) return; var arr = features.split(' ');\n\
for (var i = 0, len = arr.length; i < len; i++) { if (arr[i] && false === addons[arr[i]]) addons[arr[i]] = true; }\n\
})(\"" + sFeatures + "\");";

        bool bValid = context->Eval(sCode,
#ifndef CEF_2623
    "", 0,
#endif
retval, exception);
    }
};
// -------------------------------------------------------

class CCefScriptLoader : public NSThreads::CBaseThread
{
public:
    class ICefScriptLoaderCallback
    {
    public:
        virtual void OnLoad(CCefScriptLoader*, bool error) {}
    };

public:
    std::string     m_sVersion;
    std::wstring    m_sUrl;
    std::wstring    m_sDestination;

    CAscApplicationManager* m_pManager;
    ICefScriptLoaderCallback* m_pListener;

public:

    CCefScriptLoader() : NSThreads::CBaseThread()
    {
        m_pListener = NULL;
        m_pManager = NULL;
    }

protected:
    virtual DWORD ThreadProc()
    {
        if (std::wstring::npos != m_sUrl.find(L"sdk/Common/AllFonts.js") ||
            std::wstring::npos != m_sUrl.find(L"sdkjs/common/AllFonts.js"))
        {
            while (!m_pManager->IsInitFonts())
            {
                NSThreads::Sleep( 10 );
            }

            m_bRunThread = FALSE;

            if (NULL != m_pListener)
                m_pListener->OnLoad(this, false);
        }
        else
        {
            // старый код. теперь используется вью портала
            NSNetwork::NSFileTransport::CFileDownloader oDownloader(m_sUrl, false);
            oDownloader.SetFilePath(m_sDestination);

            oDownloader.Start( 0 );
            while ( oDownloader.IsRunned() )
            {
                NSThreads::Sleep( 10 );
            }

            m_bRunThread = FALSE;

            if (NULL != m_pListener)
                m_pListener->OnLoad(this, oDownloader.IsFileDownloaded() ? false : true);
        }

        return 0;
    }
};

///////////////////////////////////////////////////////
class CEditorFrameId
{
public:
    int EditorId;
    int_64_type FrameId;
    std::wstring Url;
};

class CAscApplicationManager_Private;
class CTimerKeyboardChecker : public NSTimers::CTimer
{
    CAscApplicationManager_Private* m_pManager;
public:
    CTimerKeyboardChecker(CAscApplicationManager_Private* pManagerPrivate) : NSTimers::CTimer()
    {
        m_pManager = pManagerPrivate;
        SetInterval(100);
    }
    virtual ~CTimerKeyboardChecker()
    {
    }

protected:
    virtual void OnTimer();
};

class CAscEditorFileInfo
{
public:
    std::wstring    m_sPath;
    int             m_nId;
    int             m_nFileType;
    bool            m_bIsViewer;
    std::wstring    m_sDate;
    std::wstring    m_sUrl;
    std::wstring    m_sExternalCloudId;
    std::wstring    m_sParentUrl;
    bool            m_bIsCrypted;

public:
    CAscEditorFileInfo()
    {
        m_sPath = L"";
        m_nId = -1;
        m_nFileType = 0;
        m_bIsViewer = false;
        m_bIsCrypted = false;
    }

    void UpdateDate()
    {
        time_t timer;
        time(&timer);

        struct tm* timeinfo;
        timeinfo = localtime(&timer);

        m_sDate = Get2(timeinfo->tm_mday) + L"." +
                    Get2(timeinfo->tm_mon + 1) + L"." +
                    std::to_wstring(timeinfo->tm_year + 1900) + L" " +
                    Get2(timeinfo->tm_hour) + L":" +
                    Get2(timeinfo->tm_min);
    }

    std::wstring Get2(const int& nVal)
    {
        if (nVal < 10)
            return L"0" + std::to_wstring(nVal);
        return std::to_wstring(nVal);
    }
};

class CRecentParent
{
public:
    std::wstring Url;
    std::wstring Parent;
};

namespace NSArgumentList
{
    static int64 GetInt64(CefRefPtr<CefListValue> args, const int& index)
    {
        std::string tmp = args->GetString(index).ToString();
        return (int64)std::stoll(tmp);
    }
    static bool SetInt64(CefRefPtr<CefListValue> args, const int& index, const int64& value)
    {
        std::string tmp = std::to_string(value);
        return args->SetString(index, tmp);
    }
}

class CAscApplicationManager_Private : public CefBase_Class,
        public CCookieFoundCallback,
        public NSThreads::CBaseThread,
        public CCefScriptLoader::ICefScriptLoaderCallback,
        public NSAscCrypto::IAscKeyChainListener,
        public CApplicationFontsWorkerBreaker
{
public:
    CAscSpellChecker m_oSpellChecker;

    CAscKeyboardChecker m_oKeyboardChecker;
    CTimerKeyboardChecker m_oKeyboardTimer;

    // счетчик всех view
    int m_nIdCounter;

    // счетчик всех view
    int m_nWindowCounter;

    // id <=> view
    std::map<int, CCefView*> m_mapViews;

    // показывать ли консоль для дебага
    bool m_bDebugInfoSupport;

    // экспериментальные возможности
    bool m_bExperimentalFeatures;

    // использовать ли внешнюю очередь сообщений
    bool m_bIsUseExternalMessageLoop;
    IExternalMessageLoop* m_pExternalMessageLoop;

    // event listener
    NSEditorApi::CAscCefMenuEventListener* m_pListener;

    // application fonts for all editors
    NSFonts::IApplicationFonts* m_pApplicationFonts;

    // используется для загрузки скриптов
    // url <-> все те, кто ждет загрузку этого скрипта. по окончании загрузки - всем отсылается евент
    NSCriticalSection::CRITICAL_SECTION m_oCS_Scripts;
    std::map<std::wstring, std::vector<CEditorFrameId>> m_mapLoadedScripts;

    // id для вью, который вызвал FileDialog
    int m_nIsCefSaveDialogWait;
    
    // внутренние скачки (неюзерские)
    std::wstring m_strPrivateDownloadUrl;
    std::wstring m_strPrivateDownloadPath;
    
    // мап отмененных загрузок
    std::map<unsigned int, bool> m_mapCancelDownloads;

    // Recents & recovers
    NSCriticalSection::CRITICAL_SECTION m_oCS_LocalFiles;
    std::vector<CAscEditorFileInfo> m_arRecents;
    std::vector<CAscEditorFileInfo> m_arRecovers;
    
    // дополнения к ссылкам
    std::wstring m_sAdditionalUrlParams;    

    // сообщения, которые отправятся в view после инициализации js контекста
    std::vector<NSEditorApi::CAscMenuEvent*> m_arApplyEvents;

    // настройки приложения
    std::map<std::string, std::string> m_mapSettings;

    // если ！= -1, то используется для scale всех view
    double m_dForceDisplayScale;

    // флаг для принудительной перегенерации шрифтов (используется при изменении настроек, какие шрифты использовать)
    bool m_bIsUpdateFontsAttack;

    // используется только для Linux snap.
    std::string m_sLD_LIBRARY_PATH;

    // crypto
    std::map<std::wstring, int> m_mapOnlyPass;
    bool m_bCryptoDisableForLocal; // поддерживает ли плагин криптования шифрование локальных файлов
    bool m_bCryptoDisableForInternalCloud; // поддерживает ли плагин криптования шифрование файлов из нашего облака
    bool m_bCryptoDisableForExternalCloud; // поддерживает ли плагин криптования шифрование файлов из чужого облака

    std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue> m_mapCrypto;
    NSAscCrypto::AscCryptoType m_nCurrentCryptoMode;
    NSAscCrypto::CCryptoKey m_cryptoKeyEnc;
    NSAscCrypto::CCryptoKey m_cryptoKeyDec;

    NSAscCrypto::CAscKeychain* m_pKeyChain;

    // плагины не для редактора, а для десктопа (на стартовой странице)
    std::vector<CExternalPluginInfo> m_arExternalPlugins;

    // те, кто подключает onlyoffice
    std::vector<CExternalCloudRegister> m_arExternalClouds;

    // критическая секция для всех системных сообщений всех view
    NSCriticalSection::CRITICAL_SECTION m_oCS_SystemMessages;

    // настройки для ссылок на редакторы
    std::wstring m_mainPostFix;
    std::wstring m_mainLang;

    // ссылки, откуда файл открыт. (для кнопок 'домой' в редакторах)
    std::vector<CRecentParent> m_arRecentParents;

    // dpi checker
    static CAscDpiChecker* m_pDpiChecker;

    // ссылки на нужные классы
    CAscApplicationManager* m_pMain;
    CApplicationCEF* m_pApplication;

    // дополнения к редактору (для внешних подключений)
    CApplicationManagerAdditionalBase* m_pAdditional;

    // логи конвертера
    bool m_bIsEnableConvertLogs;

    bool m_bIsOnlyEditorWindowMode;

    // не даем грузить любые локальные файлы
    NSSystem::CLocalFilesResolver m_oLocalFilesResolver;

    // json, отправляемый в процесс рендерера
    std::wstring m_sRendererJSON;

public:
    IMPLEMENT_REFCOUNTING(CAscApplicationManager_Private);

public:
    CAscApplicationManager_Private() : m_oKeyboardTimer(this)
    {
        this->AddRef();

        m_pListener = NULL;
        m_nIdCounter = 0;
        m_nWindowCounter = 0;

        m_pMain = NULL;
        m_pApplicationFonts = NULL;
        m_pApplication = NULL;

        m_bDebugInfoSupport = false;
        m_bExperimentalFeatures = false;

        m_bIsUseExternalMessageLoop = false;
        m_pExternalMessageLoop = NULL;

        m_nIsCefSaveDialogWait = -1;        

        m_sAdditionalUrlParams = L"";
        m_pAdditional = NULL;

        m_dForceDisplayScale = -1;

        m_bIsUpdateFontsAttack = false;

        m_nCurrentCryptoMode = NSAscCrypto::None;

        m_pKeyChain = NULL;

        m_bIsEnableConvertLogs = false;

        m_bCryptoDisableForLocal = false;
        m_bCryptoDisableForInternalCloud = false;
        m_bCryptoDisableForExternalCloud = false;

        m_bIsOnlyEditorWindowMode = false;

        m_oCS_Scripts.InitializeCriticalSection();
        m_oCS_LocalFiles.InitializeCriticalSection();
        m_oCS_SystemMessages.InitializeCriticalSection();

        COfficeUtils::SetAddonFlag(ZLIB_ADDON_FLAG_WINDOWS_SHARED_WRITE);
    }
    virtual ~CAscApplicationManager_Private()
    {
        RELEASEOBJECT(m_pExternalMessageLoop);

        CloseApplication();
        RELEASEOBJECT(m_pAdditional);

        m_oCS_Scripts.DeleteCriticalSection();
        m_oCS_LocalFiles.DeleteCriticalSection();
        m_oCS_SystemMessages.DeleteCriticalSection();
    }

    std::wstring StartTmpDirectory()
    {
        std::wstring sTmp = m_pMain->m_oSettings.cache_path + L"/work_temp";
        if (!NSDirectory::Exists(sTmp))
            NSDirectory::CreateDirectory(sTmp);
        return sTmp;
    }
    void EndTmpDirectory()
    {
        std::wstring sTmp = m_pMain->m_oSettings.cache_path + L"/work_temp";
        if (NSDirectory::Exists(sTmp))
            NSDirectory::DeleteDirectory(sTmp);
    }

    bool GetEditorPermission()
    {
        // разрешение на редактирование
        return m_pAdditional ? m_pAdditional->GetEditorPermission() : true;
    }

    // исполнить код во всех view
    void ExecuteInAllFrames(const std::string& sCode);

    // вызывается, если меняется количество открытых редакторов
    void ChangeEditorViewsCount();

    void CloseApplication()
    {
        if (NSFile::CFileBinary::Exists(m_pMain->m_oSettings.user_plugins_path + L"/cloud_crypto_tmp.xml"))
            NSFile::CFileBinary::Remove(m_pMain->m_oSettings.user_plugins_path + L"/cloud_crypto_tmp.xml");

        EndTmpDirectory();

        Stop();
        m_oKeyboardTimer.Stop();        
        m_oSpellChecker.End();
    }    

    // logout из портала -----------------------------------------------------------------------
    void Logout(std::wstring strUrl, CefRefPtr<CefCookieManager> manager)
    {
        if (0 == strUrl.find(L"onlyoffice.com"))
            return;

        CCefCookieVisitor* pVisitor = new CCefCookieVisitor();
        pVisitor->m_pCallback       = this;
        pVisitor->m_bIsDelete       = true;
        pVisitor->m_sDomain         = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(strUrl.c_str(), strUrl.length());

        if (0 == pVisitor->m_sDomain.find("https://"))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(8);
        else if (0 == pVisitor->m_sDomain.find("http://"))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(7);

        std::string::size_type pos = pVisitor->m_sDomain.find("?");
        if (pos != std::string::npos)
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);
        pos = pVisitor->m_sDomain.rfind("/");
        if ((pos != std::string::npos) && ((pos + 1) == pVisitor->m_sDomain.length()))
            pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);

        // check port
        std::string::size_type posLen = pVisitor->m_sDomain.length();
        std::string::size_type posPort = pVisitor->m_sDomain.find(":");
        while (std::string::npos != posPort)
        {
            if (posPort < (posLen - 1))
            {
                char cNext = pVisitor->m_sDomain.c_str()[posPort + 1];
                if (cNext >= '0' && cNext <= '9')
                {
                    pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, posPort);
                    break;
                }
            }
            posPort = pVisitor->m_sDomain.find(":", posPort + 1);
        }

        if (true)
        {
            pos = NSCommon::FindLowerCaseR(pVisitor->m_sDomain, "/products/files");
            if ((pos != std::string::npos) && ((pos + 15) == pVisitor->m_sDomain.length()))
                pVisitor->m_sDomain = pVisitor->m_sDomain.substr(0, pos);
        }
        
        pVisitor->CheckCookiePresent(CefCookieManager::GetGlobalManager(NULL));
    }
    virtual void OnFoundCookie(bool bIsPresent, std::string sValue)
    {
        // not used
    }
    virtual void OnSetCookie()
    {
        // not used
    }
    virtual void OnFoundCookies(std::map<std::string, std::string>& mapValues)
    {
        // not used
    }
    virtual void OnDeleteCookie(bool bIsPresent)
    {
        if (NULL != m_pMain && NULL != m_pMain->GetEventListener())
        {
            NSEditorApi::CAscCefMenuEvent* pEvent = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT);
            m_pMain->GetEventListener()->OnEvent(pEvent);
        }
    }

    // работа с настройками редактора ----------------------------------------------------------
    void LoadSettings()
    {
        m_mapSettings.clear();
        std::wstring sFile = m_pMain->m_oSettings.fonts_cache_info_path + L"/../settings.xml";
        XmlUtils::CXmlNode oNode;
        if (!oNode.FromXmlFile(sFile))
            return;

        XmlUtils::CXmlNodes oNodes;
        if (!oNode.GetChilds(oNodes))
            return;

        int nCount = oNodes.GetCount();
        for (int i = 0; i < nCount; ++i)
        {
            XmlUtils::CXmlNode oSetting;
            oNodes.GetAt(i, oSetting);

            std::wstring nameW = oSetting.GetName();
            std::string name = U_TO_UTF8(nameW);
            std::wstring valueW = oSetting.GetText();
            std::string value = U_TO_UTF8(valueW);

            if (value != "default")
                m_mapSettings.insert(std::pair<std::string, std::string>(name, value));
        }
    }
    void SaveSettings(std::map<std::string, std::string>* settings = NULL)
    {
        std::map<std::string, std::string>* _map = settings;
        if (NULL == _map)
            _map = &m_mapSettings;

        std::wstring sFile = m_pMain->m_oSettings.fonts_cache_info_path + L"/../settings.xml";
        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<Settings>");

        for (std::map<std::string, std::string>::iterator pair = _map->begin(); pair != _map->end(); pair++)
        {
            std::string name = pair->first;
            std::string value = pair->second;
            std::wstring nameW = UTF8_TO_U(name);
            std::wstring valueW = UTF8_TO_U(value);

            oBuilder.AddCharSafe('<');
            oBuilder.WriteString(nameW);
            oBuilder.AddCharSafe('>');
            oBuilder.WriteEncodeXmlString(valueW);
            oBuilder.AddCharSafe('<');
            oBuilder.AddCharSafe('/');
            oBuilder.WriteString(nameW);
            oBuilder.AddCharSafe('>');
        }

        oBuilder.WriteString(L"</Settings>");

        NSFile::CFileBinary::SaveToFile(sFile, oBuilder.GetData());

        // after - check settings
        m_dForceDisplayScale = -1;
        std::map<std::string, std::string>::iterator pairForceDisplayScale = _map->find("force-scale");
        if (pairForceDisplayScale != _map->end())
            m_dForceDisplayScale = NSStringUtils::GetDouble(pairForceDisplayScale->second);

        std::map<std::string, std::string>::iterator pairCryptoMode = _map->find("crypto-mode");
        if (pairCryptoMode != _map->end())
            m_nCurrentCryptoMode = (NSAscCrypto::AscCryptoType)std::stoi(pairCryptoMode->second);

        std::map<std::string, std::string>::iterator pairConvertLogs = _map->find("converter-logging");
        if (pairConvertLogs != _map->end())
            m_bIsEnableConvertLogs = ("1" == pairConvertLogs->second) ? true : false;

        std::map<std::string, std::string>::iterator pairEML = _map->find("external-message-loop");
        if (pairEML != _map->end())
            m_bIsUseExternalMessageLoop = ("1" == pairEML->second) ? true : false;
    }
    void CheckSetting(const std::string& sName, const std::string& sValue)
    {
        if ("--ascdesktop-support-debug-info" == sName)
        {
            m_bDebugInfoSupport = true;
            return;
        }
        if ("--ascdesktop-enable-experimental-features" == sName)
        {
            m_bExperimentalFeatures = true;
            return;
        }
        if ("--single-window-app" == sName)
        {
            m_bIsOnlyEditorWindowMode = true;
            return;
        }
        if ("--edit" == sName || "--view" == sName || "--review" == sName)
        {
            // files
            return;
        }

        bool bIsChanged = false;
        const char* namePtr = sName.c_str();

        while (*namePtr != '\0')
        {
            char c = *namePtr++;
            if ('&' == c || '\'' == c || '<' == c || '>' == c || '\"' == c)
                return;
        }

        if (0 != sName.find("--"))
            return;

        std::string name = sName.substr(2);
        std::map<std::string, std::string>::iterator pair = m_mapSettings.find(name);
        if (pair != m_mapSettings.end())
        {
            if (sValue != "default")
            {
                if (pair->second != sValue)
                    bIsChanged = true;

                pair->second = sValue;
            }
            else
            {
                m_mapSettings.erase(pair);
                bIsChanged = true;
            }
        }
        else
        {
            if (sValue != "default")
            {
                m_mapSettings.insert(std::pair<std::string, std::string>(name, sValue));
                bIsChanged = true;
            }
        }

        if ("--use-opentype-fonts" == sName && bIsChanged)
        {
            m_bIsUpdateFontsAttack = true;
        }
    }

    // вспомогательные функции
    CCefView* GetViewWithMinId()
    {
        CCefView* pMinView = NULL;
        int nMin = 0xFFFF;
        for (std::map<int, CCefView*>::iterator i = m_mapViews.begin(); i != m_mapViews.end(); i++)
        {
            CCefView* pView = i->second;
            if (pView->GetType() == cvwtSimple && pView->GetId() < nMin)
            {
                nMin = pView->GetId();
                pMinView = pView;
            }
        }
        return pMinView;
    }
    CCefView* GetViewForSystemMessages()
    {
        return GetViewWithMinId();
    }
    bool TestExternal(const std::wstring& sId, CExternalCloudRegister& ex)
    {
        for (std::vector<CExternalCloudRegister>::iterator iter = m_arExternalClouds.begin(); iter != m_arExternalClouds.end(); iter++)
        {
            if (sId == iter->id)
            {
                ex = *iter;
                return true;
            }
        }
        return false;
    }
    void SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* pEvent)
    {
        for (std::map<int, CCefView*>::iterator i = m_mapViews.begin(); i != m_mapViews.end(); i++)
        {
           CCefView* pView = i->second;
           if (pView->GetType() == cvwtSimple)
           {
               pEvent->AddRef();
               pView->Apply(pEvent);
           }
        }

        pEvent->Release();
    }

    // загрузка скриптов ----------------------------------------------------------------------
    virtual void OnLoad(CCefScriptLoader* pLoader, bool error) OVERRIDE
    {
        // коллбэк на загрузку скрипта

        m_pMain->LockCS(LOCK_CS_SCRIPT);
        
        private_OnLoad(pLoader->m_sUrl, pLoader->m_sDestination);
        RELEASEOBJECT(pLoader);

        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }    
    void Start_PrivateDownloadScript(const std::wstring& sUrl, const std::wstring& sDestination)
    {
        m_strPrivateDownloadUrl = sUrl;
        m_strPrivateDownloadPath = sDestination;
        
        NSCommon::url_correct(m_strPrivateDownloadUrl);
        
        CCefView* pMainView = GetViewWithMinId();
        if (NULL != pMainView)
            pMainView->StartDownload(m_strPrivateDownloadUrl);
    }
    void End_PrivateDownloadScript()
    {
        private_OnLoad(m_strPrivateDownloadUrl, m_strPrivateDownloadPath);

        m_pMain->LockCS(LOCK_CS_SCRIPT);

        m_strPrivateDownloadUrl = L"";
        m_strPrivateDownloadPath = L"";

        if (!m_mapLoadedScripts.empty())
        {
            for (std::map<std::wstring, std::vector<CEditorFrameId>>::iterator i = m_mapLoadedScripts.begin(); i != m_mapLoadedScripts.end(); i++)
            {
                std::wstring _url = i->second[0].Url;
                std::wstring _dst = i->first;

                if (std::wstring::npos == _url.find(L"sdk/Common/AllFonts.js") &&
                    std::wstring::npos == _url.find(L"sdkjs/common/AllFonts.js"))
                {
                    Start_PrivateDownloadScript(_url, _dst);
                    break;
                }
            }
        }

        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }
    void private_OnLoad(const std::wstring& sUrl, const std::wstring& sDestination)
    {
        m_pMain->LockCS(LOCK_CS_SCRIPT);

        std::map<std::wstring, std::vector<CEditorFrameId>>::iterator i = m_mapLoadedScripts.find(sDestination);

        if (i != m_mapLoadedScripts.end())
        {
            // другого и быть не может
            NSEditorApi::CAscEditorScript* pData = new NSEditorApi::CAscEditorScript();
            pData->put_Url(sUrl);
            pData->put_Destination(sDestination);

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION;
            pEvent->m_pData = pData;

            std::vector<CEditorFrameId>& _arr = i->second;
            for (std::vector<CEditorFrameId>::iterator it = _arr.begin(); it != _arr.end(); it++)
            {
                CCefView* pView = m_pMain->GetViewById((*it).EditorId);

                if (NULL != pView)
                {
                    pEvent->AddRef();

                    pData->put_FrameId((*it).FrameId);
                    pView->Apply(pEvent);
                }
            }

            RELEASEINTERFACE(pEvent);
        }

        m_mapLoadedScripts.erase(i);

        m_pMain->UnlockCS(LOCK_CS_SCRIPT);
    }

    // отслеживаем шрифты ---------------------------------------------------------------------
    virtual DWORD ThreadProc()
    {
        CApplicationFontsWorker oWorker;
        oWorker.m_sDirectory = m_pMain->m_oSettings.fonts_cache_info_path;
        oWorker.m_bIsUseSystemFonts = m_pMain->m_oSettings.use_system_fonts;
        oWorker.m_arAdditionalFolders = m_pMain->m_oSettings.additional_fonts_folder;

#if defined(_LINUX)
        std::wstring sHome = GetHomeDirectory();
        if (!sHome.empty())
        {
#ifdef _MAC
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/Library/Fonts");
#else
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/.fonts");
            oWorker.m_arAdditionalFolders.push_back(sHome + L"/.local/share/fonts");
#endif
        }
#endif

        oWorker.m_bIsUseAllVersions = true;
        oWorker.m_bIsUseOpenType = true;
        oWorker.m_bIsNeedThumbnails = true;
        oWorker.m_bSeparateThumbnails = true;

        oWorker.SetBreaker(this);
        m_pApplicationFonts = oWorker.Check();

        if (true)
        {
            // check features
            CSdkjsAddons oAddons(m_pMain->m_oSettings.user_plugins_path);

            std::wstring sEditorsPath = m_pMain->m_oSettings.local_editors_path;
            std::wstring::size_type nPosPostfix = sEditorsPath.rfind(L"web-apps/apps/api/documents/index.html");
            if (nPosPostfix != std::wstring::npos)
                sEditorsPath = sEditorsPath.substr(0, nPosPostfix);
            sEditorsPath += L"sdkjs";

            oAddons.CheckVersion(sEditorsPath);
        }

        oWorker.CheckThumbnails();

        m_bRunThread = FALSE;
        return 0;
    }

    // работа со скачиванием файлов -----------------------------------------------------------
    std::wstring GetPrivateDownloadUrl()
    {
        return m_strPrivateDownloadUrl;
    }
    std::wstring GetPrivateDownloadPath()
    {
        std::wstring sRet = m_strPrivateDownloadPath;
#ifdef WIN32
        NSStringUtils::string_replace(sRet, L"/", L"\\");
#endif
        return sRet;
    }
    bool IsCanceled(const unsigned int& nId)
    {
        std::map<unsigned int, bool>::iterator iter = m_mapCancelDownloads.find(nId);
        if (iter == m_mapCancelDownloads.end())
            return false;
        return true;
    }

    // работа с Recents & Recovers ------------------------------------------------------------
    void LocalFiles_Init()
    {
        Recents_Load();
        Recovers_Load();
    }
    void Recents_Load()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        m_arRecents.clear();

        std::wstring sXmlPath = m_pMain->m_oSettings.recover_path + L"/../recents.xml";
        XmlUtils::CXmlNode oNodeRecents;
        if (!oNodeRecents.FromXmlFile(sXmlPath))
            return;
        XmlUtils::CXmlNodes oNodes = oNodeRecents.GetNodes(L"file");
        int nCount = oNodes.GetCount();
        int nCountAdd = 0;

        std::map<std::wstring, bool> map_files;

        for (int i = 0; i < nCount; ++i)
        {
            XmlUtils::CXmlNode oFile;
            oNodes.GetAt(i, oFile);
            std::wstring sPath = oFile.GetAttribute(L"path");

            if (/*NSFile::CFileBinary::Exists(sPath) && */(map_files.find(sPath) == map_files.end()) && !sPath.empty())
            {
                CAscEditorFileInfo oInfo;
                oInfo.m_nId = i;
                oInfo.m_sPath = sPath;
                oInfo.m_nFileType = oFile.ReadAttributeInt(L"type");
                oInfo.m_sDate = oFile.GetAttribute(L"date");
                oInfo.m_sUrl = oFile.GetAttribute(L"url");
                oInfo.m_sExternalCloudId = oFile.GetAttribute(L"externalcloud");
                oInfo.m_sParentUrl = oFile.GetAttribute(L"parent");
                oInfo.m_bIsCrypted = (oFile.GetAttribute(L"crypted") == L"1") ? true : false;
                m_arRecents.push_back(oInfo);

                map_files.insert(std::pair<std::wstring, bool>(sPath, true));
            }
        }

        Recents_Dump(false);
    }
    void Recents_Add(const std::wstring& sPathSrc, const int& nType, const std::wstring& sUrl = L"", const std::wstring& sExternalCloudId = L"", const std::wstring& sParentUrl = L"", bool bIsCrypted = false)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::wstring sPath = sPathSrc;
#ifdef WIN32
        if (!NSFileDownloader::IsNeedDownload(sPath))
            NSStringUtils::string_replace(sPath, L"/", L"\\");
#endif

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (sPath == i->m_sPath)
            {
                m_arRecents.erase(i);
                break;
            }
            if (!sUrl.empty() && (sUrl == i->m_sUrl))
            {
                m_arRecents.erase(i);
                break;
            }
        }
        CAscEditorFileInfo oInfo;
        oInfo.m_nId = 0;
        oInfo.m_sPath = sPath;
        oInfo.m_sUrl = sUrl;
        oInfo.m_sExternalCloudId = sExternalCloudId;
        oInfo.m_sParentUrl = sParentUrl;
        oInfo.m_bIsCrypted = bIsCrypted;
        oInfo.UpdateDate();

        oInfo.m_nFileType = nType;
        m_arRecents.insert(m_arRecents.begin(), oInfo);

        int nId = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            i->m_nId = nId++;
        }

        Recents_Dump();
    }
    void Recents_Remove(const int& nId)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (nId == i->m_nId)
            {
                m_arRecents.erase(i);
                break;
            }
        }

        int nIdCurrent = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            i->m_nId = nIdCurrent++;
        }

        Recents_Dump();
    }
    void Recents_RemoveAll()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        m_arRecents.clear();

        Recents_Dump();
    }
    void Recents_Dump(bool bIsSend = true)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::wstring sXmlPath = m_pMain->m_oSettings.recover_path + L"/../recents.xml";
        if (NSFile::CFileBinary::Exists(sXmlPath))
            NSFile::CFileBinary::Remove(sXmlPath);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><recents>");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            oBuilder.WriteString(L"<file path=\"");
            oBuilder.WriteEncodeXmlString(i->m_sPath);
            oBuilder.WriteString(L"\" type=\"");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L"\" date=\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\" url=\"");
            oBuilder.WriteEncodeXmlString(i->m_sUrl);

            if (!i->m_sExternalCloudId.empty())
            {
                oBuilder.WriteString(L"\" externalcloud=\"");
                oBuilder.WriteEncodeXmlString(i->m_sExternalCloudId);
            }

            if (i->m_sParentUrl.empty())
            {
                if (NSFile::CFileBinary::Exists(i->m_sPath))
                {
                    oBuilder.WriteString(L"\" parent=\"");
                    oBuilder.WriteEncodeXmlString(NSFile::GetDirectoryName(i->m_sPath));
                }
                else
                {
                    oBuilder.WriteString(L"\" parent=\"");
                }
            }
            else
            {
                oBuilder.WriteString(L"\" parent=\"");
                oBuilder.WriteEncodeXmlString(i->m_sParentUrl);
            }

            if (i->m_bIsCrypted)
            {
                oBuilder.WriteString(L"\" crypted=\"1");
            }

            oBuilder.WriteString(L"\" />");
        }
        oBuilder.WriteString(L"</recents>");

        NSFile::CFileBinary::SaveToFile(sXmlPath, oBuilder.GetData(), true);

        if (bIsSend)
        {
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS);
            NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
            pData->put_JSON(Recents_GetJSON());
            pEvent->m_pData = pData;

            SetEventToAllMainWindows(pEvent);
        }
    }
    std::wstring Recents_GetJSON()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"[");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecents.begin(); i != m_arRecents.end(); i++)
        {
            if (1 < oBuilder.GetCurSize())
                oBuilder.WriteString(L",");

            oBuilder.WriteString(L"{id:");
            oBuilder.AddInt(i->m_nId);
            oBuilder.WriteString(L",type:");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L",crypted:");
            oBuilder.AddInt(i->m_bIsCrypted ? 1 : 0);
            oBuilder.WriteString(L",path:\"");
            oBuilder.WriteEncodeXmlString(i->m_sPath);
            oBuilder.WriteString(L"\",modifyed:\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\",parent:\"");
            if (i->m_sParentUrl.empty())
            {
                if (NSFile::CFileBinary::Exists(i->m_sPath))
                {
                    oBuilder.WriteEncodeXmlString(NSFile::GetDirectoryName(i->m_sPath));
                }
                else
                {
                    oBuilder.WriteEncodeXmlString(L"");
                }
            }
            else
            {
                oBuilder.WriteEncodeXmlString(i->m_sParentUrl);
            }
            oBuilder.WriteString(L"\"}");
        }
        oBuilder.WriteString(L"]");
        return oBuilder.GetData();
    }

    void Recovers_Load()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        std::vector<std::wstring> arDirectories = NSDirectory::GetDirectories(m_pMain->m_oSettings.recover_path);
        int nCount = (int)arDirectories.size();

        int nId = 0;

        for (int i = 0; i < nCount; ++i)
        {
            std::vector<std::wstring> arDirectoriesFiles = NSDirectory::GetFiles(arDirectories[i]);
            int nCountFilesRecover = (int)arDirectoriesFiles.size();

            std::wstring sName;
            int nType = -1;
            bool bIsViewer = false;

            std::wstring sRecoveryLocker = arDirectories[i] + L"/rec.lock";
            if (NSFile::CFileBinary::Exists(sRecoveryLocker) && NSSystem::CLocalFileLocker::IsLocked(sRecoveryLocker))
                continue;

            if (!NSFile::CFileBinary::Exists(arDirectories[i] + L"/changes/changes0.json"))
            {
                NSDirectory::DeleteDirectory(arDirectories[i]);
                continue;
            }

            for (int j = 0; j < nCountFilesRecover; ++j)
            {
                std::wstring sTmp = NSFile::GetFileName(arDirectoriesFiles[j]);
                if (L"Editor.bin" == sTmp)
                {
                    NSFile::CFileBinary oFile;
                    if (oFile.OpenFile(arDirectoriesFiles[j]) && 4 < oFile.GetFileSize())
                    {
                        BYTE _buffer[4];
                        DWORD dwRead = 0;
                        oFile.ReadFile(_buffer, 4, dwRead);

                        if (4 == dwRead)
                        {
                            if ('X' == _buffer[0] && 'L' == _buffer[1] && 'S' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX;
                            else if ('P' == _buffer[0] && 'P' == _buffer[1] && 'T' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;
                            else if ('D' == _buffer[0] && 'O' == _buffer[1] && 'C' == _buffer[2] && 'Y' == _buffer[3])
                                nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                            else
                            {
                                nType = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                                bIsViewer = true;
                            }
                        }
                        oFile.CloseFile();
                    }
                }
            }

            std::wstring sNameInfo = arDirectories[i] + L"/asc_name.info";
            if (!NSFile::CFileBinary::Exists(sNameInfo) || nType == -1)
                continue;

            XmlUtils::CXmlNode oNode;
            if (oNode.FromXmlFile(sNameInfo))
            {
                CAscEditorFileInfo oInfo;
                oInfo.m_nFileType = oNode.ReadAttributeInt(L"type");
                oInfo.m_nId = nId++;
                oInfo.m_sPath = arDirectories[i] + L"/" + oNode.GetAttribute(L"name");
                oInfo.m_bIsViewer = bIsViewer;
                m_arRecovers.push_back(oInfo);
            }
        }
    }
    void Recovers_Remove(const int& nId, bool bIsAttack = false)
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            if (nId == i->m_nId)
            {
                if (bIsAttack)
                {
                    NSDirectory::DeleteDirectory(NSFile::GetDirectoryName(i->m_sPath));
                }

                m_arRecovers.erase(i);                
                break;
            }
        }

        int nIdCurrent = 0;
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            i->m_nId = nIdCurrent++;
        }

        Recovers_Dump();
    }
    void Recovers_RemoveAll()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            NSDirectory::DeleteDirectory(NSFile::GetDirectoryName(i->m_sPath));
        }

        m_arRecovers.clear();
        Recovers_Dump();
    }
    void Recovers_Dump()
    {
        CTemporaryCS oCS(&m_oCS_LocalFiles);

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"[");
        for (std::vector<CAscEditorFileInfo>::iterator i = m_arRecovers.begin(); i != m_arRecovers.end(); i++)
        {
            if (1 < oBuilder.GetCurSize())
                oBuilder.WriteString(L",");

            oBuilder.WriteString(L"{id:");
            oBuilder.AddInt(i->m_nId);
            oBuilder.WriteString(L",type:");
            oBuilder.AddInt(i->m_nFileType);
            oBuilder.WriteString(L",path:\"");
            std::wstring sPath = i->m_sPath;
#ifdef WIN32
            NSStringUtils::string_replace(sPath, L"/", L"\\");
#endif
            oBuilder.WriteEncodeXmlString(sPath);
            oBuilder.WriteString(L"\",modifyed:\"");
            oBuilder.WriteEncodeXmlString(i->m_sDate);
            oBuilder.WriteString(L"\"}");
        }
        oBuilder.WriteString(L"]");
        std::wstring sJSON = oBuilder.GetData();

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS);
        NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
        pData->put_JSON(sJSON);
        pEvent->m_pData = pData;

        SetEventToAllMainWindows(pEvent);
    }

    // crypto ---------------------------------------------------------------------------------
    void LoadCryptoData()
    {
        m_pKeyChain = m_pMain->GetKeychainEngine();
        m_pKeyChain->Check(m_pMain->m_oSettings.cookie_path + L"/user.data");
    }
    void SendCryptoData(CefRefPtr<CefFrame> frame = NULL)
    {
        std::wstring sPass = L"";
        if (0 != m_nCurrentCryptoMode)
        {
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_mapCrypto.find(m_nCurrentCryptoMode);
            if (find != m_mapCrypto.end())
                sPass = UTF8_TO_U(find->second.m_sValue);
        }

        NSStringUtils::string_replace(sPass, L"\\", L"\\\\");
        NSStringUtils::string_replace(sPass, L"\"", L"\\\"");

        std::wstring sCode = L"(function() { \n\
window.AscDesktopEditor.CryptoMode = " + std::to_wstring(m_nCurrentCryptoMode) + L";\n\
window.AscDesktopEditor.CryptoPassword = \"" + sPass + L"\";\n\
})();";

        if (frame)
        {
            frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
            return;
        }

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_EXECUTE_JS_CODE);
        NSEditorApi::CAscJSMessage* pData = new NSEditorApi::CAscJSMessage();
        pData->put_Value(sCode);
        pEvent->m_pData = pData;

        for (std::map<int, CCefView*>::iterator iterView = m_mapViews.begin(); iterView != m_mapViews.end(); iterView++)
        {
            CCefView* pTmp = iterView->second;

            ADDREFINTERFACE(pEvent);
            pTmp->Apply(pEvent);
        }

        RELEASEINTERFACE(pEvent);
    }
    virtual void OnKeyChainComplete(NSAscCrypto::CCryptoKey& keyEnc, NSAscCrypto::CCryptoKey& keyDec)
    {
        m_cryptoKeyEnc = keyEnc;
        m_cryptoKeyDec = keyDec;

        NSAscCrypto::CCryptoMode oCryptoMode;
        oCryptoMode.Load(m_cryptoKeyEnc, m_cryptoKeyDec, m_pMain->m_oSettings.cookie_path + L"/user.data");

        for (std::vector<NSAscCrypto::CAscCryptoJsonValue>::iterator iter = oCryptoMode.m_modes.begin(); iter != oCryptoMode.m_modes.end(); iter++)
        {
            std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator find = m_mapCrypto.find(iter->m_eType);
            if (find == m_mapCrypto.end())
            {
                m_mapCrypto.insert(std::pair<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>(iter->m_eType, *iter));
            }
            else
            {
                find->second.m_sValue = iter->m_sValue;
            }
        }

        // создаем ключи для режимов
        bool bIsResave = false;

        // 1) simple
        std::map<NSAscCrypto::AscCryptoType, NSAscCrypto::CAscCryptoJsonValue>::iterator findSimple = m_mapCrypto.find(NSAscCrypto::Simple);
        if (findSimple != m_mapCrypto.end())
        {
            NSAscCrypto::CAscCryptoJsonValue& simple = findSimple->second;
            if (simple.m_sValue.empty())
            {
                std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                char data[1025];
                int alphanumlen = alphanum.length();
                srand((unsigned int)NSTimers::GetTickCount());
                for (int i = 0; i < 1024; ++i)
                {
                    data[i] = (char)alphanum[rand() % (alphanumlen - 1)];
                }
                data[1024] = 0;
                simple.m_sValue = std::string(data);
                bIsResave = true;
            }
        }

        if (bIsResave)
        {
            m_pMain->SetCryptoMode("", m_nCurrentCryptoMode);
        }

        RELEASEOBJECT(m_pKeyChain);
    }

    virtual bool IsFontsWorkerRunned()
    {
        return (m_bRunThread == TRUE) ? true : false;
    }
};

#endif // APPLICATION_MANAGER_PRIVATE_H
