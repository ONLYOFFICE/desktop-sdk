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

#ifndef ASC_CEFCONVERTER_FILECONVERTER_H
#define ASC_CEFCONVERTER_FILECONVERTER_H

#include "./applicationmanager_p.h"
#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "../../../../core/DesktopEditor/xmlsec/src/include/OOXMLVerifier.h"
#include "../../../../core/DesktopEditor/xmlsec/src/include/OOXMLSigner.h"
#include "../../../../core/OfficeUtils/src/OfficeUtils.h"

#include "../../../../core/DesktopEditor/raster/BgraFrame.h"
#include "../../../../core/DesktopEditor/graphics/pro/Image.h"

#ifdef LINUX
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#endif

class CAscLocalFileInfo
{
public:
    std::wstring m_sFileSrc;
    std::wstring m_sRecoveryDir;

    // чтобы не ждать лишнего времени - грузим страницу и скрипты
    // ОДНОВРЕМЕННО с конвертацией. поэтому нужен счетчик, чтобы запускать открытие
    // после и конвертации и после вызова OfflineStart (js)
    int m_nCounterConvertion;

    int m_nCurrentFileFormat;
    bool m_bIsSaved;

    bool m_bIsModified;

    std::wstring m_sPassword;

    std::wstring m_sDocumentInfo;

public:
    CAscLocalFileInfo()
    {
        m_nCounterConvertion = 0;
        m_nCurrentFileFormat = 0;
        m_bIsSaved = true;
        m_bIsModified = false;
    }
    ~CAscLocalFileInfo()
    {
    }

    CAscLocalFileInfo& operator=(const CAscLocalFileInfo& oSrc)
    {
        m_sFileSrc = oSrc.m_sFileSrc;
        m_sRecoveryDir = oSrc.m_sRecoveryDir;

        m_nCounterConvertion = oSrc.m_nCounterConvertion;

        m_nCurrentFileFormat = oSrc.m_nCurrentFileFormat;
        m_bIsSaved = oSrc.m_bIsSaved;

        m_sPassword = oSrc.m_sPassword;
        m_sDocumentInfo = oSrc.m_sDocumentInfo;
        return *this;
    }
};

class CAscLocalFileInfoCS
{
public:
    NSCriticalSection::CRITICAL_SECTION m_oCS;
    CAscLocalFileInfo m_oInfo;
    bool m_bIsRetina;

public:
    CAscLocalFileInfoCS()
    {
        m_oCS.InitializeCriticalSection();
        m_bIsRetina = false;
    }
    ~CAscLocalFileInfoCS()
    {
        m_oCS.DeleteCriticalSection();
    }

    void SetupOptions(CAscLocalFileInfo& oInfo)
    {
        CTemporaryCS oCS(&m_oCS);
        oInfo = m_oInfo;
    }
};

namespace NSX2T
{
    int Convert(const std::wstring& sConverterPath, const std::wstring sXmlPath, CAscApplicationManager* pManager)
    {
        int nReturnCode = 0;
        std::wstring sConverterExe = sConverterPath;

#ifdef WIN32
        sConverterExe += L".exe";
        std::wstring sApp = L"x2t ";

        STARTUPINFO sturtupinfo;
        ZeroMemory(&sturtupinfo,sizeof(STARTUPINFO));
        sturtupinfo.cb = sizeof(STARTUPINFO);

        sApp += (L"\"" + sXmlPath + L"\"");
        wchar_t* pCommandLine = NULL;
        if (true)
        {
            pCommandLine = new wchar_t[sApp.length() + 1];
            memcpy(pCommandLine, sApp.c_str(), sApp.length() * sizeof(wchar_t));
            pCommandLine[sApp.length()] = (wchar_t)'\0';
        }

        HANDLE ghJob = CreateJobObject(NULL, NULL);

        if (ghJob)
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

            // Configure all child processes associated with the job to terminate when the
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if ( 0 == SetInformationJobObject( ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
            {
                CloseHandle(ghJob);
                ghJob = NULL;
            }
        }

        PROCESS_INFORMATION processinfo;
        ZeroMemory(&processinfo,sizeof(PROCESS_INFORMATION));
        BOOL bResult = CreateProcessW(sConverterExe.c_str(), pCommandLine,
                                   NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sturtupinfo, &processinfo);

        if (bResult && ghJob)
        {
            AssignProcessToJobObject(ghJob, processinfo.hProcess);
        }

        ::WaitForSingleObject(processinfo.hProcess, INFINITE);

        RELEASEARRAYOBJECTS(pCommandLine);

        //get exit code
        DWORD dwExitCode = 0;
        if (GetExitCodeProcess(processinfo.hProcess, &dwExitCode))
        {
            nReturnCode = (int)dwExitCode;
        }

        CloseHandle(processinfo.hProcess);
        CloseHandle(processinfo.hThread);

        if (ghJob)
        {
            CloseHandle(ghJob);
            ghJob = NULL;
        }

#endif

#ifdef LINUX
        pid_t pid = fork(); // create child process
        int status;

        std::string sProgramm = U_TO_UTF8(sConverterExe);
        std::string sXmlA = U_TO_UTF8(sXmlPath);

        switch (pid)
        {
        case -1: // error
            break;

        case 0: // child process
        {
            std::string sLibraryDir = sProgramm;
            std::string sPATH = sProgramm;
            if (std::string::npos != sProgramm.find_last_of('/'))
            {
                sLibraryDir = "LD_LIBRARY_PATH=" + sProgramm.substr(0, sProgramm.find_last_of('/'));
#ifndef _MAC
                sLibraryDir += ":";
                sLibraryDir += pManager->GetLibraryPathVariable();
#endif
                sPATH = "PATH=" + sProgramm.substr(0, sProgramm.find_last_of('/'));
            }

#ifdef _MAC
            sLibraryDir = "DY" + sLibraryDir;
#endif

            const char* nargs[3];
            nargs[0] = sProgramm.c_str();
            nargs[1] = sXmlA.c_str();
            nargs[2] = NULL;

#ifndef _MAC
            const char* nenv[4];
            std::string s1 = "APPLICATION_NAME=" + pManager->m_oSettings.converter_application_name;
            std::string s2 = "COMPANY_NAME=" + pManager->m_oSettings.converter_application_company;
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = s1.c_str();
            nenv[2] = s2.c_str();
            nenv[3] = NULL;
#else
            const char* nenv[5];
            std::string s1 = "APPLICATION_NAME=" + pManager->m_oSettings.converter_application_name;
            std::string s2 = "COMPANY_NAME=" + pManager->m_oSettings.converter_application_company;
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = sPATH.c_str();
            nenv[2] = s1.c_str();
            nenv[3] = s2.c_str();
            nenv[4] = NULL;
#endif

            execve(sProgramm.c_str(),
                   (char * const *)nargs,
                   (char * const *)nenv);
            exit(EXIT_SUCCESS);
            break;
        }
        default: // parent process, pid now contains the child pid
            while (-1 == waitpid(pid, &status, 0)); // wait for child to complete
            if (WIFEXITED(status))
            {
                nReturnCode =  WEXITSTATUS(status);
            }
            break;
        }
#endif

        return nReturnCode;
    }
}

namespace NSOOXMLPassword
{
    class COOXMLZipDirectory
    {
    private:
        std::wstring m_sFile;
        std::wstring m_sDirectory;
        std::wstring m_sPassword;

        CAscApplicationManager* m_pManager;

    public:

        COOXMLZipDirectory(CAscApplicationManager* pManager)
        {
            m_pManager = pManager;
        }

        int Open(const std::wstring& sFile, const std::wstring& sPassword)
        {
            m_sFile = sFile;
            m_sPassword = sPassword;

            std::wstring sTempFile = m_sFile;
            if (!m_sPassword.empty())
            {
                int nFormatType = CCefViewEditor::GetFileFormat(m_sFile);

                sTempFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"PASS");
                if (NSFile::CFileBinary::Exists(sTempFile))
                    NSFile::CFileBinary::Remove(sTempFile);

                std::wstring sTempFileXml = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"XML");
                if (NSFile::CFileBinary::Exists(sTempFileXml))
                    NSFile::CFileBinary::Remove(sTempFileXml);
                sTempFileXml += L".xml";

                NSStringUtils::CStringBuilder oBuilder;
                oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
                oBuilder.WriteEncodeXmlString(m_sFile);
                oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
                oBuilder.WriteEncodeXmlString(sTempFile);
                oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");
                oBuilder.WriteString(std::to_wstring(nFormatType));
                oBuilder.WriteString(L"</m_nFormatTo>");
                oBuilder.WriteString(L"<m_sThemeDir>./themes</m_sThemeDir><m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
                oBuilder.WriteString(L"<m_sPassword>");
                oBuilder.WriteEncodeXmlString(m_sPassword);
                oBuilder.WriteString(L"</m_sPassword>");
                oBuilder.WriteString(L"<m_sFontDir>");
                oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
                oBuilder.WriteString(L"</m_sFontDir>");
                oBuilder.WriteString(L"</TaskQueueDataConvert>");

                std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";
                NSFile::CFileBinary::SaveToFile(sTempFileXml, oBuilder.GetData(), true);

                int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileXml, m_pManager);

                NSFile::CFileBinary::Remove(sTempFileXml);
            }

            m_sDirectory = NSDirectory::CreateDirectoryWithUniqueName(NSDirectory::GetTempPath());
            NSDirectory::CreateDirectory(m_sDirectory);

            COfficeUtils oCOfficeUtils(NULL);
            oCOfficeUtils.ExtractToDirectory(sTempFile, m_sDirectory, NULL, 0);

            if (!m_sPassword.empty())
                NSFile::CFileBinary::Remove(sTempFile);

            return 0;
        }

        int Close()
        {
            if (m_sPassword.empty())
            {
                NSFile::CFileBinary::Remove(m_sFile);
                COfficeUtils oCOfficeUtils(NULL);
                oCOfficeUtils.CompressFileOrDirectory(m_sDirectory, m_sFile, true);
                NSDirectory::DeleteDirectory(m_sDirectory);
                return 0;
            }

            std::wstring sTempFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"PASS");
            if (NSFile::CFileBinary::Exists(sTempFile))
                NSFile::CFileBinary::Remove(sTempFile);

            sTempFile += (L"." + NSCommon::GetFileExtention(m_sFile));

            COfficeUtils oCOfficeUtils(NULL);
            oCOfficeUtils.CompressFileOrDirectory(m_sDirectory, sTempFile, true);
            NSDirectory::DeleteDirectory(m_sDirectory);

            int nFormatType = CCefViewEditor::GetFileFormat(m_sFile);

            std::wstring sTempFileXml = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"XML");
            if (NSFile::CFileBinary::Exists(sTempFileXml))
                NSFile::CFileBinary::Remove(sTempFileXml);
            sTempFileXml += L".xml";

            NSFile::CFileBinary::Remove(m_sFile);

            NSStringUtils::CStringBuilder oBuilder;
            oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
            oBuilder.WriteEncodeXmlString(sTempFile);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
            oBuilder.WriteEncodeXmlString(m_sFile);
            oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");
            oBuilder.WriteString(std::to_wstring(nFormatType));
            oBuilder.WriteString(L"</m_nFormatTo>");
            oBuilder.WriteString(L"<m_sThemeDir>./themes</m_sThemeDir><m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
            oBuilder.WriteString(L"<m_sSavePassword>");
            oBuilder.WriteEncodeXmlString(m_sPassword);
            oBuilder.WriteString(L"</m_sSavePassword>");
            oBuilder.WriteString(L"<m_sFontDir>");
            oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
            oBuilder.WriteString(L"</m_sFontDir>");
            oBuilder.WriteString(L"</TaskQueueDataConvert>");

            std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";
            NSFile::CFileBinary::SaveToFile(sTempFileXml, oBuilder.GetData(), true);

            int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileXml, m_pManager);

            NSFile::CFileBinary::Remove(sTempFile);
            NSFile::CFileBinary::Remove(sTempFileXml);

            return 0;
        }

    public:
        std::wstring GetDirectory()
        {
            return m_sDirectory;
        }
    };
}

class IASCFileConverterEvents
{
public:
    virtual void OnFileConvertToEditor(const int& nError) = 0;
    virtual void OnFileConvertFromEditor(const int& nError, const std::wstring& sPass = L"") = 0;
};

class CASCFileConverterToEditor : public NSThreads::CBaseThread
{
public:
    CAscLocalFileInfo m_oInfo;

    IASCFileConverterEvents* m_pEvents;
    CAscApplicationManager* m_pManager;
    CCefView* m_pView;
    std::wstring m_sName;

    std::wstring m_sAdditionalConvertation;

    bool m_bIsNativeOpening;
    std::wstring m_sNativeSrcPath;

    bool m_bIsNativeSupport;

    COOXMLVerifier* m_pVerifier;
    NSFonts::IApplicationFonts* m_pFonts;

public:
    CASCFileConverterToEditor() : NSThreads::CBaseThread()
    {
        m_pEvents = NULL;
        m_pManager = NULL;
        m_pView = NULL;

        m_bIsNativeOpening = false;
        m_bIsNativeSupport = true;

        m_pVerifier = NULL;

        m_pFonts = NULL;
    }
    virtual ~CASCFileConverterToEditor()
    {
        Stop();
        RELEASEOBJECT(m_pVerifier);
        NSBase::Release(m_pFonts);
    }

    virtual void Stop()
    {
        m_bIsNativeOpening = false;
        NSThreads::CBaseThread::Stop();
    }

    virtual DWORD ThreadProc()
    {
        if (NSFile::CFileBinary::Exists(m_oInfo.m_sRecoveryDir + L"/Editor.bin"))
        {
            if (true)
            {
                std::wstring sTmp = m_oInfo.m_sRecoveryDir + L"/Editor.bin";
                NSFile::CFileBinary oFile;

                AscEditorType eType = etDocument;
                if (oFile.OpenFile(sTmp) && 4 < oFile.GetFileSize())
                {
                    BYTE _buffer[4];
                    DWORD dwRead = 0;
                    oFile.ReadFile(_buffer, 4, dwRead);

                    if (4 == dwRead)
                    {
                        if ('X' == _buffer[0] && 'L' == _buffer[1] && 'S' == _buffer[2] && 'Y' == _buffer[3])
                            eType = etSpreadsheet;
                        else if ('P' == _buffer[0] && 'P' == _buffer[1] && 'T' == _buffer[2] && 'Y' == _buffer[3])
                            eType = etPresentation;
                    }
                    oFile.CloseFile();
                }

                NSEditorApi::CAscCefMenuEventListener* pListener = m_pManager->GetEventListener();
                NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
                pData->put_Id(m_pView->GetId());
                pData->put_Type((int)eType);

                NSEditorApi::CAscCefMenuEvent* pEvent = m_pView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE);
                pEvent->m_pData = pData;
                pListener->OnEvent(pEvent);
            }

            // обновили страницу. считаем, что конвертация прошла успешно
            m_pEvents->OnFileConvertToEditor(0);
            return 0;
        }

        std::wstring sLocalFilePath = m_oInfo.m_sFileSrc;
#ifdef WIN32
        if (0 == sLocalFilePath.find(L"//"))
        {
            sLocalFilePath = L"\\\\" + sLocalFilePath.substr(2);
        }
#endif

        if (true)
        {
            NSEditorApi::CAscCefMenuEventListener* pListener = m_pManager->GetEventListener();
            NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
            pData->put_Id(m_pView->GetId());

            AscEditorType eType = etDocument;
            if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
                eType = etPresentation;
            if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
                eType = etSpreadsheet;

            pData->put_Type((int)eType);

            NSEditorApi::CAscCefMenuEvent* pEvent = m_pView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE);
            pEvent->m_pData = pData;
            pListener->OnEvent(pEvent);
        }

        std::wstring sDestinationPath = m_oInfo.m_sRecoveryDir + L"/" + (m_sName.empty() ? NSCommon::GetFileName(sLocalFilePath) : m_sName);
        m_sNativeSrcPath = sDestinationPath;
        if (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML)
        {
            std::wstring sExt = NSCommon::GetFileExtention(sDestinationPath);
            std::string sExtA = U_TO_UTF8(sExt);
            NSCommon::makeUpper(sExtA);

            if (sExtA != "HTML" && sExtA != "HTM")
                sDestinationPath += L".html";

            if (true)
            {
                // ***_files
                size_t nIndex = sLocalFilePath.length();
                if (nIndex > sExt.length())
                    nIndex -= (sExt.length() + 1);

                if (nIndex > 0)
                {
                    std::wstring sTestDirectory = sLocalFilePath.substr(0, nIndex);
                    sTestDirectory += L"_files";
                    if (NSDirectory::Exists(sTestDirectory))
                    {
                        // copy
                        std::wstring sDestinationCopyDir = m_oInfo.m_sRecoveryDir + L"/" + NSCommon::GetFileName(sTestDirectory);
                        NSDirectory::CreateDirectory(sDestinationCopyDir);

                        std::vector<std::wstring> arrFiles = NSDirectory::GetFiles(sTestDirectory, false);

                        for (std::vector<std::wstring>::iterator i = arrFiles.begin(); i != arrFiles.end(); i++)
                        {
                            std::wstring sTestFileCopy = NSCommon::GetFileName(*i);
                            NSFile::CFileBinary::Copy(*i, sDestinationCopyDir + L"/" + sTestFileCopy);
                        }
                    }
                }
            }
        }

        NSFile::CFileBinary::Copy(sLocalFilePath, sDestinationPath);

        if (m_bIsNativeSupport && m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
        {
            NSDirectory::CreateDirectory(m_oInfo.m_sRecoveryDir + L"/media");
            this->NativeViewerOpen(true); // async. sleep!!!
            m_pEvents->OnFileConvertToEditor(0);

            m_bRunThread = FALSE;
            return 0;
        }

        std::wstring sNameInfo = m_oInfo.m_sRecoveryDir + L"/asc_name.info";
        if (NSFile::CFileBinary::Exists(sNameInfo))
            NSFile::CFileBinary::Remove(sNameInfo);

        NSStringUtils::CStringBuilder oBuilderInfo;
        oBuilderInfo.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><info type=\"" + std::to_wstring(m_oInfo.m_nCurrentFileFormat) + L"\" name=\"");
        oBuilderInfo.WriteEncodeXmlString(NSCommon::GetFileName(sDestinationPath));
        oBuilderInfo.WriteString(L"\" />");
        NSFile::CFileBinary::SaveToFile(sNameInfo, oBuilderInfo.GetData(), true);

        std::wstring sParams = L"";
        if (!m_sAdditionalConvertation.empty())
        {
            sParams += m_sAdditionalConvertation;
        }
        m_sAdditionalConvertation = L"";

        std::wstring strDirectoryFonts = m_pManager->m_oSettings.fonts_cache_info_path;
        while (!NSFile::CFileBinary::Exists(strDirectoryFonts + L"/font_selection.bin"))
            NSThreads::Sleep(100);

        if (true)
        {
            NSStringUtils::CStringBuilder oBuilderFonts;
            oBuilderFonts.WriteString(L"<m_sFontDir>");
            oBuilderFonts.WriteEncodeXmlString(strDirectoryFonts);
            oBuilderFonts.WriteString(L"</m_sFontDir>");

#ifdef WIN32
            oBuilderFonts.WriteString(L"<m_sHtmlFileInternalPath>");
            oBuilderFonts.WriteEncodeXmlString(NSFile::GetProcessDirectory() + L"/");
            oBuilderFonts.WriteString(L"</m_sHtmlFileInternalPath>");
#endif

#if defined(_LINUX) && !defined(_MAC)
            oBuilderFonts.WriteString(L"<m_sHtmlFileInternalPath>");
            oBuilderFonts.WriteEncodeXmlString(NSFile::GetProcessDirectory() + L"/");
            oBuilderFonts.WriteString(L"</m_sHtmlFileInternalPath>");
#endif

            sParams += oBuilderFonts.GetData();
        }

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
        oBuilder.WriteEncodeXmlString(sDestinationPath);
        oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
        oBuilder.WriteEncodeXmlString(m_oInfo.m_sRecoveryDir);
        oBuilder.WriteString(L"/Editor.bin</m_sFileTo><m_nFormatTo>8192</m_nFormatTo>");
        oBuilder.WriteString(L"<m_sThemeDir>./themes</m_sThemeDir><m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
        oBuilder.WriteString(sParams);
        oBuilder.WriteString(L"</TaskQueueDataConvert>");

        std::wstring sXmlConvert = oBuilder.GetData();

        std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";

        std::wstring sTempFileForParams = m_oInfo.m_sRecoveryDir + L"/params_from.xml";
        NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);

        int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileForParams, m_pManager);

        NSFile::CFileBinary::Remove(sTempFileForParams);

        if (0 == nReturnCode)
            CheckSignatures(sDestinationPath);

        m_pEvents->OnFileConvertToEditor(nReturnCode);

        m_bRunThread = FALSE;
        return 0;
    }

    void NativeViewerOpen(bool bIsEnabled = false);
    void NativeViewerOpenEnd(const std::string& sBase64);

    void CheckSignatures(const std::wstring& sFile)
    {
        if (!m_pManager->m_oSettings.sign_support)
            return;

        RELEASEOBJECT(m_pVerifier);
        if (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX ||
            m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX ||
            m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX)
        {
            COfficeFileFormatChecker oChecker;
            oChecker.isOfficeFile(sFile);
            if (AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO == oChecker.nFileType)
            {
                NSOOXMLPassword::COOXMLZipDirectory oZIP(m_pManager);
                oZIP.Open(sFile, m_oInfo.m_sPassword);

                m_pVerifier = new COOXMLVerifier(oZIP.GetDirectory());

                oZIP.Close();
            }
            else
            {
                std::wstring sUnzipDir = NSCommon::GetDirectoryName(sFile) + L"/" + NSCommon::GetFileName(sFile) + L"_uncompress";
                NSDirectory::CreateDirectory(sUnzipDir);

                COfficeUtils oCOfficeUtils(NULL);
                if (S_OK == oCOfficeUtils.ExtractToDirectory(sFile, sUnzipDir, NULL, 0))
                {
                    m_pVerifier = new COOXMLVerifier(sUnzipDir);
                    NSDirectory::DeleteDirectory(sUnzipDir);
                }
            }
        }
    }

    void CheckSignaturesByDir(const std::wstring& sFolder)
    {
        RELEASEOBJECT(m_pVerifier);
        m_pVerifier = new COOXMLVerifier(sFolder);
    }

    std::wstring GetSignaturesJSON()
    {
        if (NULL == m_pVerifier)
            return L"";

        int nCount = m_pVerifier->GetSignatureCount();
        if (0 == nCount)
            return L"";

        NSStringUtils::CStringBuilder oBuilder;
        oBuilder.WriteString(L"{\"count\":");
        oBuilder.AddInt(nCount);
        oBuilder.WriteString(L", \"data\":[");

        for (int i = 0; i < nCount; i++)
        {
            COOXMLSignature* pSign = m_pVerifier->GetSignature(i);

            if (0 != i)
                oBuilder.WriteString(L",");
            oBuilder.WriteString(L"{\"name\":\"");
            std::wstring sParam = pSign->GetCertificate()->GetSignerName();
            NSCommon::string_replace(sParam, L"\"", L"\\\"");
            oBuilder.WriteString(sParam);
            oBuilder.WriteString(L"\",\"guid\":\"");
            std::string sGuid = pSign->GetGuid();
            oBuilder.WriteString(UTF8_TO_U(sGuid));
            oBuilder.WriteString(L"\",\"date\":\"");
            std::string sDate = pSign->GetDate();
            oBuilder.WriteString(UTF8_TO_U(sDate));
            oBuilder.WriteString(L"\",\"valid\":");
            oBuilder.AddInt(pSign->GetValid());
            oBuilder.WriteString(L",\"image_valid\":\"");
            sGuid = GetPngFromBase64(pSign->GetImageValidBase64());
            oBuilder.WriteString(UTF8_TO_U(sGuid));
            oBuilder.WriteString(L"\",\"image_invalid\":\"");
            sGuid = GetPngFromBase64(pSign->GetImageInvalidBase64());
            oBuilder.WriteString(UTF8_TO_U(sGuid));
            oBuilder.WriteString(L"\"}");
        }

        oBuilder.WriteString(L"]}");

        return oBuilder.GetData();
    }

    std::string GetPngFromBase64(const std::string& sBase64)
    {
        if (sBase64.empty())
            return "";

        std::wstring sTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
        std::wstring sTmp2 = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");

        BYTE* pData = NULL;
        int nBase64InputLen = 0;
        NSFile::CBase64Converter::Decode(sBase64.c_str(), (int)sBase64.length(), pData, nBase64InputLen);

        NSFile::CFileBinary oFile;
        oFile.CreateFileW(sTmp);
        oFile.WriteFile(pData, (DWORD)nBase64InputLen);
        oFile.CloseFile();

        RELEASEARRAYOBJECTS(pData);
        nBase64InputLen = 0;

        if (NULL == m_pFonts)
        {
            m_pFonts = NSFonts::NSApplication::Create();
            m_pFonts->InitializeFromFolder(m_pManager->m_oSettings.fonts_cache_info_path);
        }

        bool bIsGood = false;
        MetaFile::IMetaFile* pMeta = MetaFile::Create(m_pFonts);
        if (pMeta->LoadFromFile(sTmp.c_str()))
        {
            pMeta->ConvertToRaster(sTmp2.c_str(), 4, 300);
            bIsGood = true;
        }
        else
        {
            CBgraFrame oFrame;
            if (oFrame.OpenFile(sTmp))
            {
                oFrame.SaveFile(sTmp2, 4);
                bIsGood = true;
            }
        }
        RELEASEINTERFACE(pMeta);

        NSFile::CFileBinary::Remove(sTmp);
        if (!bIsGood)
        {
            NSFile::CFileBinary::Remove(sTmp2);
            return sBase64;
        }

        DWORD dwLen = 0;
        NSFile::CFileBinary::ReadAllBytes(sTmp2, &pData, dwLen);

        char* base64 = NULL;
        int base64Len = 0;

        NSFile::CBase64Converter::Encode(pData, (int)dwLen, base64, base64Len, NSBase64::B64_BASE64_FLAG_NOCRLF);

        std::string sRet(base64, base64Len);

        RELEASEARRAYOBJECTS(base64);

        NSFile::CFileBinary::Remove(sTmp2);
        return sRet;
    }
};

class CASCFileConverterFromEditor : public NSThreads::CBaseThread
{
public:
    CAscLocalFileInfo m_oInfo;

    std::wstring m_sOriginalFileNameCrossPlatform;
    bool m_bIsRetina;
    int m_nTypeEditorFormat;

    IASCFileConverterEvents* m_pEvents;
    CAscApplicationManager* m_pManager;

    bool m_bIsEditorWithChanges;

public:
    CASCFileConverterFromEditor() : NSThreads::CBaseThread()
    {
        m_bIsRetina = false;
        m_nTypeEditorFormat = -1;
        m_bIsEditorWithChanges = false;
    }
    virtual ~CASCFileConverterFromEditor()
    {
        Stop();
    }

    virtual DWORD ThreadProc()
    {
        std::wstring sLocalFilePath = m_oInfo.m_sFileSrc;
#ifdef WIN32
        if (0 == sLocalFilePath.find(L"//"))
        {
            sLocalFilePath = L"\\\\" + sLocalFilePath.substr(2);
        }
#endif

        bool bIsUseTmpFileDst = false;
        std::wstring sUseTmpFileDst = L"";
        if (0 == sLocalFilePath.find(L"\\\\") || 0 == sLocalFilePath.find(L"//"))
        {
            bIsUseTmpFileDst = true;

            sUseTmpFileDst = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
            if (NSFile::CFileBinary::Exists(sUseTmpFileDst))
                NSFile::CFileBinary::Remove(sUseTmpFileDst);
        }

        std::wstring sThemesPath = m_pManager->m_oSettings.local_editors_path;
        std::wstring::size_type nPosThemes = sThemesPath.rfind(L"apps/api/documents/index.html");
        if (nPosThemes != std::wstring::npos)
            sThemesPath = sThemesPath.substr(0, nPosThemes);
        sThemesPath += L"../sdkjs/slide/themes";

        std::wstring strDirectoryFonts = m_pManager->m_oSettings.fonts_cache_info_path;
        while (!NSFile::CFileBinary::Exists(strDirectoryFonts + L"/font_selection.bin"))
            NSThreads::Sleep(100);

        std::wstring sParams;
        if (true)
        {
            NSStringUtils::CStringBuilder oBuilderFonts;
            oBuilderFonts.WriteString(L"<m_sFontDir>");
            oBuilderFonts.WriteEncodeXmlString(strDirectoryFonts);
            oBuilderFonts.WriteString(L"</m_sFontDir>");
            sParams += oBuilderFonts.GetData();
        }

        NSStringUtils::CStringBuilder oBuilder;

        oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");

        if (m_sOriginalFileNameCrossPlatform.empty())
        {
            oBuilder.WriteEncodeXmlString(m_oInfo.m_sRecoveryDir);

            if (!m_bIsEditorWithChanges)
                oBuilder.WriteString(L"/Editor.bin</m_sFileFrom><m_sFileTo>");
            else
                oBuilder.WriteString(L"/EditorWithChanges.bin</m_sFileFrom><m_sFileTo>");
        }
        else
        {
            std::wstring sPathTmp = m_oInfo.m_sRecoveryDir + L"/" + NSCommon::GetFileName(m_sOriginalFileNameCrossPlatform);
            oBuilder.WriteEncodeXmlString(sPathTmp);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
        }
        
        if (bIsUseTmpFileDst)
            oBuilder.WriteEncodeXmlString(sUseTmpFileDst);
        else
            oBuilder.WriteEncodeXmlString(sLocalFilePath);
        
        oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");

        if (AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA == m_oInfo.m_nCurrentFileFormat)
        {
            oBuilder.WriteString(std::to_wstring(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF));
            sParams += L"<m_bIsPDFA>true</m_bIsPDFA>";
        }
        else
        {
            oBuilder.WriteString(std::to_wstring(m_oInfo.m_nCurrentFileFormat));
        }

        oBuilder.WriteString(L"</m_nFormatTo><m_sThemeDir>");
        oBuilder.WriteEncodeXmlString(sThemesPath);
        oBuilder.WriteString(L"</m_sThemeDir>");

        if (NSFile::CFileBinary::Exists(m_oInfo.m_sRecoveryDir + L"/changes/changes0.json"))
            oBuilder.WriteString(L"<m_bFromChanges>true</m_bFromChanges>");
        else
            oBuilder.WriteString(L"<m_bFromChanges>false</m_bFromChanges>");

        oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional><m_sAllFontsPath>");

        oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
        oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath><m_nCsvTxtEncoding>46</m_nCsvTxtEncoding><m_nCsvDelimiter>4</m_nCsvDelimiter>");
        oBuilder.WriteString(sParams);

        if (!m_oInfo.m_sPassword.empty())
        {
            switch (m_oInfo.m_nCurrentFileFormat)
            {
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:
                case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS:
                case AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP:
                case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF:
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX:
                case AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX:
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT:
                case AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS:
                {
                    oBuilder.WriteString(L"<m_sSavePassword>");
                    oBuilder.WriteEncodeXmlString(m_oInfo.m_sPassword);
                    oBuilder.WriteString(L"</m_sSavePassword>");
                    break;
                }
                default:
                    break;
            }
        }

        if (!m_oInfo.m_sDocumentInfo.empty())
        {
            oBuilder.WriteString(L"<m_sDocumentID>");
            oBuilder.WriteEncodeXmlString(m_oInfo.m_sDocumentInfo);
            oBuilder.WriteString(L"</m_sDocumentID>");
        }
        m_oInfo.m_sDocumentInfo = L"";

        int nDoctRendererParam = 0;
        if (m_bIsRetina)
            nDoctRendererParam |= 0x01;
        if (true) // печать пдф (лист = страница)
            nDoctRendererParam |= 0x02;

        nDoctRendererParam |= 0x04; // disable fast doctrenderer (no rights to dump common information)

        oBuilder.WriteString(L"<m_nDoctParams>");
        oBuilder.WriteString(std::to_wstring(nDoctRendererParam));
        oBuilder.WriteString(L"</m_nDoctParams>");

        oBuilder.WriteString(L"</TaskQueueDataConvert>");

        std::wstring sXmlConvert = oBuilder.GetData();

        std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";

        std::wstring sTempFileForParams = m_oInfo.m_sRecoveryDir + L"/params_to.xml";
        NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);
        
        if (!m_pManager->m_pInternal->GetEditorPermission())
        {
            // return!
            m_pEvents->OnFileConvertFromEditor(ASC_CONSTANT_CANCEL_SAVE);
            m_bRunThread = FALSE;
            return 0;
        }

        if (m_pManager->m_pInternal->m_pAdditional)
            m_pManager->m_pInternal->m_pAdditional->CheckSaveStart(m_oInfo.m_sRecoveryDir, m_nTypeEditorFormat);

        int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileForParams, m_pManager);

        if (bIsUseTmpFileDst)
        {
            NSFile::CFileBinary::Remove(sLocalFilePath);
            NSFile::CFileBinary::Copy(sUseTmpFileDst, sLocalFilePath);
            NSFile::CFileBinary::Remove(sUseTmpFileDst);
        }

        m_sOriginalFileNameCrossPlatform = L"";
        m_bIsRetina = false;

        NSFile::CFileBinary::Remove(sTempFileForParams);
        m_pEvents->OnFileConvertFromEditor(nReturnCode, m_oInfo.m_sPassword);

        if (m_pManager->m_pInternal->m_pAdditional)
            m_pManager->m_pInternal->m_pAdditional->CheckSaveEnd();

        m_bRunThread = FALSE;
        return 0;
    }
};

class CTextDocxConverterCallback
{
public:
    virtual void CTextDocxConverterCallback_OnConvert(std::wstring sData) = 0;
};

class CTextDocxConverter : public NSThreads::CBaseThread
{
public:
    int m_nFrameId;

    std::wstring m_sData;
    std::wstring m_sPassword;

    std::wstring m_sFileDocx;
    bool m_bIsToDocx;

    CAscApplicationManager* m_pManager;
    CTextDocxConverterCallback* m_pCallback;

public:
    CTextDocxConverter() : NSThreads::CBaseThread()
    {
        m_nFrameId = 0;
        m_pManager = NULL;
        m_bIsToDocx = false;
        m_pCallback = NULL;
    }
    virtual ~CTextDocxConverter()
    {
        Stop();
    }

    void ToDocx()
    {
        m_bIsToDocx = true;
        Start(0);
    }

    void ToData()
    {
        m_bIsToDocx = false;
        Start(0);
    }

    virtual DWORD ThreadProc()
    {
        if (!NSDirectory::Exists(m_pManager->m_oSettings.user_plugins_path))
            NSDirectory::CreateDirectory(m_pManager->m_oSettings.user_plugins_path);

        m_sFileDocx = m_pManager->m_oSettings.user_plugins_path + L"/advanced_crypto_data.docx";

        if (m_bIsToDocx)
        {
            std::wstring sTxtFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
            if (NSFile::CFileBinary::Exists(sTxtFile))
                NSFile::CFileBinary::Remove(sTxtFile);

            NSFile::CFileBinary oFile;
            oFile.CreateFileW(sTxtFile);
            oFile.WriteStringUTF8(L"[data]" + m_sData + L"[data]", true);
            oFile.CloseFile();

            std::wstring strDirectoryFonts = m_pManager->m_oSettings.fonts_cache_info_path;
            while (!NSFile::CFileBinary::Exists(strDirectoryFonts + L"/font_selection.bin"))
                NSThreads::Sleep(100);

            NSStringUtils::CStringBuilder oBuilder;
            oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
            oBuilder.WriteEncodeXmlString(sTxtFile);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
            oBuilder.WriteEncodeXmlString(m_sFileDocx);
            oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>65</m_nFormatTo><m_sFontDir>");
            oBuilder.WriteEncodeXmlString(strDirectoryFonts);
            oBuilder.WriteString(L"</m_sFontDir>");

            if (!m_sPassword.empty())
            {
                oBuilder.WriteString(L"<m_sSavePassword>");
                oBuilder.WriteEncodeXmlString(m_sPassword);
                oBuilder.WriteString(L"</m_sSavePassword>");
            }

            oBuilder.WriteString(L"</TaskQueueDataConvert>");

            std::wstring sTempFileForParams = sTxtFile + L"_params_from.xml";
            NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

            int nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager);

            NSFile::CFileBinary::Remove(sTempFileForParams);
            NSFile::CFileBinary::Remove(sTxtFile);

            if (0 != nReturnCode)
                m_sData = L"";
        }
        else
        {
            std::wstring sTxtFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
            if (NSFile::CFileBinary::Exists(sTxtFile))
                NSFile::CFileBinary::Remove(sTxtFile);

            std::wstring strDirectoryFonts = m_pManager->m_oSettings.fonts_cache_info_path;
            while (!NSFile::CFileBinary::Exists(strDirectoryFonts + L"/font_selection.bin"))
                NSThreads::Sleep(100);

            NSStringUtils::CStringBuilder oBuilder;
            oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
            oBuilder.WriteEncodeXmlString(m_sFileDocx);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
            oBuilder.WriteEncodeXmlString(sTxtFile);
            oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>69</m_nFormatTo><m_sFontDir>");
            oBuilder.WriteEncodeXmlString(strDirectoryFonts);
            oBuilder.WriteString(L"</m_sFontDir>");

            if (!m_sPassword.empty())
            {
                oBuilder.WriteString(L"<m_sPassword>");
                oBuilder.WriteEncodeXmlString(m_sPassword);
                oBuilder.WriteString(L"</m_sPassword>");
            }

            oBuilder.WriteString(L"</TaskQueueDataConvert>");

            std::wstring sTempFileForParams = sTxtFile + L"_params_from.xml";
            NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

            int nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager);

            NSFile::CFileBinary::Remove(sTempFileForParams);

            if (nReturnCode == 0)
            {
                NSFile::CFileBinary::ReadAllTextUtf8(sTxtFile, m_sData);

                std::wstring::size_type pos1 = m_sData.find(L"[data]");
                std::wstring::size_type pos2 = m_sData.rfind(L"[data]");

                if (pos1 != std::wstring::npos && pos2 != std::wstring::npos)
                {
                    m_sData = m_sData.substr(pos1 + 6, pos2 - pos1 - 6);
                }
                else
                {
                    m_sData = L"";
                }
            }
            else
            {
                m_sData = L"";
            }

            NSFile::CFileBinary::Remove(sTxtFile);
        }

        m_pCallback->CTextDocxConverterCallback_OnConvert(m_sData);

        m_nFrameId = 0;
        m_sData = L"";
        m_sPassword = L"";

        m_bRunThread = FALSE;
        return 0;
    }
};

class CSimpleConverter : public NSThreads::CBaseThread
{
public:
    std::wstring m_sRecoverFolder;
    std::wstring m_sOutputPath;
    int m_nOutputFormat;
    std::string m_sOutputParams;

    IASCFileConverterEvents* m_pEvents;
    CAscApplicationManager* m_pManager;

public:
    CSimpleConverter()
    {
        m_nOutputFormat = -1;
        m_pEvents = NULL;
        m_pManager = NULL;
    }

    void Convert()
    {
        Start(0);
    }

    virtual DWORD ThreadProc()
    {
        int nReturnCode = 0;

        if (m_nOutputFormat == AVS_OFFICESTUDIO_FILE_OTHER_HTMLZIP)
        {
            // html полностью сделали на клиенте
            NSFile::CFileBinary::Copy(m_sRecoverFolder + L"/EditorWithChanges.bin", m_sOutputPath);
        }
        else
        {
            NSStringUtils::CStringBuilder oBuilder;
            oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
            oBuilder.WriteEncodeXmlString(m_sRecoverFolder + L"/EditorWithChanges.bin");
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
            oBuilder.WriteEncodeXmlString(m_sOutputPath);
            oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");
            oBuilder.WriteString(std::to_wstring(m_nOutputFormat));
            oBuilder.WriteString(L"</m_nFormatTo><m_sFontDir>");
            oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
            oBuilder.WriteString(L"</m_sFontDir>");
            oBuilder.WriteString(UTF8_TO_U(m_sOutputParams));
            oBuilder.WriteString(L"<m_bIsNoBase64>false</m_bIsNoBase64>");
            oBuilder.WriteString(L"</TaskQueueDataConvert>");

            std::wstring sTempFileForParams = m_sRecoverFolder + L"/params_simple_converter.xml";
            NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

            nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager);
            NSFile::CFileBinary::Remove(sTempFileForParams);
        }

        m_pEvents->OnFileConvertFromEditor(nReturnCode);
        NSFile::CFileBinary::Remove(m_sRecoverFolder + L"/EditorWithChanges.bin");

        m_bRunThread = FALSE;
        return 0;
    }
};

#endif // ASC_CEFCONVERTER_FILECONVERTER_H
