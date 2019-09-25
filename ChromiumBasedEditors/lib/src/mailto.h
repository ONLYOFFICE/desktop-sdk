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

#ifndef ASC_MAIL_TO_H
#define ASC_MAIL_TO_H

#include "../../../../core/DesktopEditor/common/File.h"

#ifdef _WIN32
#include <Windows.h>
#endif

class CMailToCommandLine
{
public:
    static std::wstring GetMailApp(std::wstring& sMailAppType)
    {
        std::vector<std::wstring> arApps;
        arApps.push_back(L"outlook");
        arApps.push_back(L"r7");
        arApps.push_back(L"thunderbird");

    #ifdef _WIN32
        WCHAR windir[MAX_PATH];
        GetWindowsDirectory(windir, MAX_PATH);
        GetWindowsDirectoryW(windir, MAX_PATH);

        std::wstring sWindowsApp(windir);
        std::wstring sPr1 = sWindowsApp + L"\\..\\Program Files";
        std::wstring sPr2 = sWindowsApp + L"\\..\\Program Files (x86)";
        std::wstring sTest;

        for (std::vector<std::wstring>::iterator iterPr = arApps.begin(); iterPr != arApps.end(); iterPr++)
        {
            std::wstring sProgram = *iterPr;
            sMailAppType = sProgram;

            if (sProgram == L"outlook")
            {
                int nVersionMax = 19;
                int nVersionMin = 10;

                for (int nVersion = nVersionMax; nVersion >= nVersionMin; --nVersion)
                {
                    sTest = sPr1 + L"\\Microsoft Office\\Office" + std::to_wstring(nVersion) + L"\\outlook.exe";
                    if (NSFile::CFileBinary::Exists(sTest))
                        return sTest;

                    sTest = sPr1 + L"\\Microsoft Office\\root\\Office" + std::to_wstring(nVersion) + L"\\outlook.exe";
                    if (NSFile::CFileBinary::Exists(sTest))
                        return sTest;

                    sTest = sPr2 + L"\\Microsoft Office\\Office" + std::to_wstring(nVersion) + L"\\outlook.exe";
                    if (NSFile::CFileBinary::Exists(sTest))
                        return sTest;

                    sTest = sPr2 + L"\\Microsoft Office\\root\\Office" + std::to_wstring(nVersion) + L"\\outlook.exe";
                    if (NSFile::CFileBinary::Exists(sTest))
                        return sTest;
                }
            }
            else if (sProgram == L"r7")
            {
                sTest = sPr1 + L"\\R7-Office\\Organizer\\organizer.exe";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;

                sTest = sPr2 + L"\\R7-Office\\Organizer\\organizer.exe";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;
            }
            else if (sProgram == L"thunderbird")
            {
                sTest = sPr1 + L"\\Mozilla Thunderbird\\thunderbird.exe";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;

                sTest = sPr2 + L"\\Mozilla Thunderbird\\thunderbird.exe";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;
            }
        }
    #endif

    #if defined(_LINUX) && !defined(_MAC)

        std::wstring sTest;
        for (std::vector<std::wstring>::iterator iterPr = arApps.begin(); iterPr != arApps.end(); iterPr++)
        {
            std::wstring sProgram = *iterPr;
            sMailAppType = sProgram;

            if (sProgram == L"r7")
            {
                sTest = L"/opt/r7-office/organizer/organizer";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;
            }
            else if (sProgram == L"thunderbird")
            {
                sTest = L"/usr/bin/thunderbird";
                if (NSFile::CFileBinary::Exists(sTest))
                    return sTest;
            }
        }

    #endif

        sMailAppType = L"";
        return L"";
    }

    static void GetArguments(std::vector<std::wstring>& args, const std::wstring& sAppType, const std::wstring& sFilePath)
    {
        std::wstring sSubject = NSFile::GetFileName(sFilePath);

        if (L"outlook" == sAppType)
        {
            args.push_back(L"/c");
            args.push_back(L"ipm.note");
            args.push_back(L"/m");

            std::wstring sParam = L"?subject=" + CefURIEncode(sSubject, false).ToWString();
            args.push_back(sParam);

            if (!sFilePath.empty())
            {
                args.push_back(L"/a");

                std::wstring sFilePathSrc = sFilePath;
                NSCommon::string_replace(sFilePathSrc, L"/", L"\\");
                args.push_back(sFilePathSrc);
            }
        }
        else if (L"r7" == sAppType)
        {
            args.push_back(L"-compose");

            bool bIsAddFile = true;
            std::wstring sFilePathSrc = sFilePath;
#ifdef _WIN32
            if (0 == sFilePath.find(L"\\\\") || 0 == sFilePath.find(L"//"))
            {
                NSCommon::string_replace(sFilePathSrc, L"/", L"\\");
                bIsAddFile = false;
            }
#endif
            if (bIsAddFile)
                sFilePathSrc = L"file://" + sFilePathSrc;

            std::wstring sParam = L"to='',subject='" + sSubject + L"',attachment='" + sFilePathSrc + L"'";
            args.push_back(sParam);
        }
        else if (L"thunderbird" == sAppType)
        {
            args.push_back(L"-compose");

            bool bIsAddFile = true;
            std::wstring sFilePathSrc = sFilePath;
#ifdef _WIN32
            if (0 == sFilePath.find(L"\\\\") || 0 == sFilePath.find(L"//"))
            {
                NSCommon::string_replace(sFilePathSrc, L"/", L"\\");
                bIsAddFile = false;
            }
#endif
            if (bIsAddFile)
                sFilePathSrc = L"file://" + sFilePathSrc;

            std::wstring sParam = L"to='',subject='" + sSubject + L"',attachment='" + sFilePath + L"'";
            args.push_back(sParam);
        }
    }
};

#if 0
class CMailTo
{
public:
    std::wstring m_sReceiver;
    std::wstring m_sSubject;
    std::wstring m_sText;
    std::wstring m_sFilePath;
    std::wstring m_sFileTmp;
    bool m_bIsFile;

    std::vector<std::wstring> m_arTmps;

public:
    CMailTo()
    {
    }
    ~CMailTo()
    {
        for (std::vector<std::wstring>::iterator iter = m_arTmps.begin(); iter != m_arTmps.end(); iter++)
        {
            NSFile::CFileBinary::Remove(*iter);
        }
    }

    void CreateDefault(std::wstring sFilePath)
    {
        m_sFilePath = sFilePath;
        m_sReceiver = L"";
        m_sText = L"";
        m_sSubject = NSFile::GetFileName(m_sFilePath);

        CreateTmp();
        m_arTmps.push_back(m_sFileTmp);

        m_bIsFile = true;
    }

    void CreateDefaultUrl(std::wstring sFileUrl)
    {
        m_sFilePath = L"";
        m_sReceiver = L"";
        m_sText = sFileUrl;
        m_sSubject = L"File";
        m_bIsFile = false;

        CreateTmp();
        m_arTmps.push_back(m_sFileTmp);
    }

    std::string GetMimeType(std::string sExt)
    {
        if ("docx" == sExt)
            return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        if ("doc" == sExt)
            return "application/msword";
        if ("odt" == sExt)
            return "application/vnd.oasis.opendocument.text";
        if ("rtf" == sExt)
            return "application/rtf";
        if ("txt" == sExt)
            return "text/plain";
        if ("html" == sExt)
            return "text/html";
        if ("epub" == sExt)
            return "application/epub+zip";
        if ("docm" == sExt)
            return "application/vnd.ms-word.document.macroenabled.12";
        if ("dotx" == sExt)
            return "application/vnd.openxmlformats-officedocument.wordprocessingml.template";
        if ("ott" == sExt)
            return "application/vnd.oasis.opendocument.text-template";

        if ("pptx" == sExt)
            return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        if ("ppt" == sExt)
            return "application/vnd.ms-powerpoint";
        if ("ppsx" == sExt)
            return "application/vnd.openxmlformats-officedocument.presentationml.slideshow";
        if ("pps" == sExt)
            return "application/vnd.ms-powerpoint";
        if ("pptm" == sExt)
            return "application/vnd.ms-powerpoint.presentation.macroenabled.12";
        if ("ppsm" == sExt)
            return "application/vnd.ms-powerpoint.slideshow.macroenabled.12";
        if ("potx" == sExt)
            return "application/vnd.openxmlformats-officedocument.presentationml.template";
        if ("odp" == sExt)
            return "application/vnd.oasis.opendocument.presentation";
        if ("otp" == sExt)
            return "application/vnd.oasis.opendocument.presentation-template";

        if ("xlsx" == sExt)
            return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        if ("xls" == sExt)
            return "application/vnd.ms-excel";
        if ("csv" == sExt)
            return "text/csv";
        if ("xlsm" == sExt)
            return "application/vnd.ms-excel.sheet.macroenabled.12";
        if ("xltx" == sExt)
            return "application/vnd.openxmlformats-officedocument.spreadsheetml.template";
        if ("xltm" == sExt)
            return "application/vnd.ms-excel.template.macroenabled.12";
        if ("ods" == sExt)
            return "application/vnd.oasis.opendocument.spreadsheet";
        if ("ots" == sExt)
            return "application/vnd.oasis.opendocument.spreadsheet-template";

        if ("pdf" == sExt)
            return "application/pdf";
        if ("djvu" == sExt)
            return "image/vnd.djvu";
        if ("xps" == sExt)
            return "application/vnd.ms-xpsdocument";

        return "text/plain";
    }

    std::wstring GetMimeType(std::wstring sFilePath)
    {
        std::wstring sExt = NSFile::GetFileExtention(sFilePath);
        std::string sExtA = U_TO_UTF8(sExt);
        std::string sMime = GetMimeType(sExtA);
        return UTF8_TO_U(sMime);
    }

    void CreateTmp()
    {
        if (!m_sFileTmp.empty())
            return;

        m_sFileTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"EML");
        if (NSFile::CFileBinary::Exists(m_sFileTmp))
            NSFile::CFileBinary::Remove(m_sFileTmp);

        m_sFileTmp += L".eml";
    }

    void Send()
    {
        CreateTmp();

        std::wstring sContent = L"Content-Type: multipart/alternative; boundary=\"BitshiftDynamicsMailerBoundary\"\r\n";
        sContent += L"To: ";
        sContent += m_sReceiver;
        sContent += L"\r\n";

        sContent += L"Subject: ";
        sContent += m_sSubject;
        sContent += L"\r\n";

        sContent += L"Mime-Version: 1.0 BitshiftDynamics Mailer\r\n\r\n";

        sContent += L"--BitshiftDynamicsMailerBoundary\r\n";
        sContent += L"Content-Transfer-Encoding: quoted-printable\r\n";
        sContent += L"Content-Type: text/plain;\r\n";
        sContent += L"        charset=utf-8\r\n\r\n";
        sContent += m_sText;
        sContent += L"\r\n\r\n";

        int nDataDst = 0;
        char* pDataDst = NULL;

        if (!m_sFilePath.empty())
        {
            std::wstring sFileName = NSCommon::GetFileName(m_sFilePath);
            std::wstring sMimeType = GetMimeType(m_sFilePath);

            sContent += L"\r\n--BitshiftDynamicsMailerBoundary\r\n";
            sContent += L"Content-Type: multipart/mixed;\r\n";
            sContent += L"        boundary=\"BitshiftDynamicsMailerBoundary\"\r\n\r\n";

            sContent += L"--BitshiftDynamicsMailerBoundary\r\n";
            sContent += L"Content-Disposition: inline;\r\n        filename=\"";
            sContent += sFileName;
            sContent += L"\"\r\nContent-Type: ";
            sContent += sMimeType;
            sContent += L";\r\n        name=\"";
            sContent += sFileName;
            sContent += L"\"\r\nContent-Transfer-Encoding: base64\r\n\r\n";

            BYTE* pDataFile = NULL;
            DWORD dwDataLen = 0;
            NSFile::CFileBinary::ReadAllBytes(m_sFilePath, &pDataFile, dwDataLen);
            NSFile::CBase64Converter::Encode(pDataFile, (int)dwDataLen, pDataDst, nDataDst);

            RELEASEARRAYOBJECTS(pDataFile);
        }

        std::string sHeader = U_TO_UTF8(sContent);

        NSFile::CFileBinary oFile;
        oFile.CreateFileW(m_sFileTmp);
        oFile.WriteFile((BYTE*)sHeader.c_str(), (DWORD)sHeader.length());

        if (nDataDst != 0)
            oFile.WriteFile((BYTE*)pDataDst, (DWORD)nDataDst);

        oFile.CloseFile();

        RELEASEARRAYOBJECTS(pDataDst);
    }
};
#endif

#endif
