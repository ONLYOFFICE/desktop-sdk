/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#ifndef ASC_CEFCONVERTER_FILECONVERTER_H
#define ASC_CEFCONVERTER_FILECONVERTER_H

#include "./applicationmanager_p.h"
#include "../../../../core/Common/OfficeFileFormatChecker.h"

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

class IASCFileConverterEvents
{
public:
    virtual void OnFileConvertToEditor(const int& nError) = 0;
    virtual void OnFileConvertFromEditor(const int& nError) = 0;
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

public:
    CASCFileConverterToEditor() : NSThreads::CBaseThread()
    {
        m_pEvents = NULL;
        m_pManager = NULL;
        m_pView = NULL;

        m_bIsNativeOpening = false;
        m_bIsNativeSupport = true;
    }
    virtual ~CASCFileConverterToEditor()
    {
        Stop();
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

                NSEditorApi::CAscMenuEventListener* pListener = m_pManager->GetEventListener();
                NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
                pData->put_Id(m_pView->GetId());
                pData->put_Type((int)eType);

                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE;
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
            NSEditorApi::CAscMenuEventListener* pListener = m_pManager->GetEventListener();
            NSEditorApi::CAscTabEditorType* pData = new NSEditorApi::CAscTabEditorType();
            pData->put_Id(m_pView->GetId());

            AscEditorType eType = etDocument;
            if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
                eType = etPresentation;
            if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
                eType = etSpreadsheet;

            pData->put_Type((int)eType);

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE;
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

        int nReturnCode = 0;

        std::wstring sTempFileForParams = m_oInfo.m_sRecoveryDir + L"/params_from.xml";
        NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);

#ifdef WIN32
        std::wstring sApp = L"x2t32 ";

        if (NSFile::CFileBinary::Exists(sConverterExe + L".exe"))
        {
            sApp = L"x2t ";
            sConverterExe += L".exe";
        }
        else
            sConverterExe += L"32.exe";

        STARTUPINFO sturtupinfo;
        ZeroMemory(&sturtupinfo,sizeof(STARTUPINFO));
        sturtupinfo.cb = sizeof(STARTUPINFO);

        sApp += (L"\"" + sTempFileForParams + L"\"");
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
        std::string sXmlA = U_TO_UTF8(sTempFileForParams);

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
            const char* nenv[2];
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = NULL;
#else
            const char* nenv[3];
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = sPATH.c_str();
            nenv[2] = NULL;
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

        NSFile::CFileBinary::Remove(sTempFileForParams);
        m_pEvents->OnFileConvertToEditor(nReturnCode);

        m_bRunThread = FALSE;
        return 0;
    }

    void NativeViewerOpen(bool bIsEnabled = false);
    void NativeViewerOpenEnd(const std::string& sBase64);
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

public:
    CASCFileConverterFromEditor() : NSThreads::CBaseThread()
    {
        m_bIsRetina = false;
        m_nTypeEditorFormat = -1;        
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
            oBuilder.WriteString(L"/Editor.bin</m_sFileFrom><m_sFileTo>");
        }
        else
        {
            std::wstring sPathTmp = m_oInfo.m_sRecoveryDir + L"/" + NSCommon::GetFileName(m_sOriginalFileNameCrossPlatform);
            oBuilder.WriteEncodeXmlString(sPathTmp);
            oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
        }
        
        oBuilder.WriteEncodeXmlString(sLocalFilePath);
        
        oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");
        oBuilder.WriteString(std::to_wstring(m_oInfo.m_nCurrentFileFormat));
        oBuilder.WriteString(L"</m_nFormatTo><m_sThemeDir>");
        oBuilder.WriteEncodeXmlString(sThemesPath);
        oBuilder.WriteString(L"</m_sThemeDir><m_bFromChanges>true</m_bFromChanges><m_bDontSaveAdditional>true</m_bDontSaveAdditional><m_sAllFontsPath>");
        oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
        oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath><m_nCsvTxtEncoding>46</m_nCsvTxtEncoding><m_nCsvDelimiter>4</m_nCsvDelimiter>");
        oBuilder.WriteString(sParams);

        int nDoctRendererParam = 0;
        if (m_bIsRetina)
            nDoctRendererParam |= 0x01;
        //if (true) // печать пдф (лист = страница)
        //    nDoctRendererParam |= 0x02;

        oBuilder.WriteString(L"<m_nDoctParams>");
        oBuilder.WriteString(std::to_wstring(nDoctRendererParam));
        oBuilder.WriteString(L"</m_nDoctParams>");

        oBuilder.WriteString(L"</TaskQueueDataConvert>");

        std::wstring sXmlConvert = oBuilder.GetData();

        std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";

        int nReturnCode = 0;

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

#ifdef WIN32
        std::wstring sApp = L"x2t32 ";

        if (NSFile::CFileBinary::Exists(sConverterExe + L".exe"))
        {
            sApp = L"x2t ";
            sConverterExe += L".exe";
        }
        else
            sConverterExe += L"32.exe";

        STARTUPINFO sturtupinfo;
        ZeroMemory(&sturtupinfo,sizeof(STARTUPINFO));
        sturtupinfo.cb = sizeof(STARTUPINFO);

        sApp += (L"\"" + sTempFileForParams + L"\"");
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
        std::string sXmlA = U_TO_UTF8(sTempFileForParams);

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
            const char* nenv[2];
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = NULL;
#else
            const char* nenv[3];
            nenv[0] = sLibraryDir.c_str();
            nenv[1] = sPATH.c_str();
            nenv[2] = NULL;
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
        
        m_sOriginalFileNameCrossPlatform = L"";
        m_bIsRetina = false;

        NSFile::CFileBinary::Remove(sTempFileForParams);
        m_pEvents->OnFileConvertFromEditor(nReturnCode);

        if (m_pManager->m_pInternal->m_pAdditional)
            m_pManager->m_pInternal->m_pAdditional->CheckSaveEnd();

        m_bRunThread = FALSE;
        return 0;
    }
};


#endif // ASC_CEFCONVERTER_FILECONVERTER_H
