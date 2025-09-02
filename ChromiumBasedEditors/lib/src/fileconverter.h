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
#include "../../../../core/Common/OfficeFileErrorDescription.h"

#include "./x2t.h"

std::wstring GetPresentationThemes(CAscApplicationManager* pManager)
{
	std::wstring sThemesPath = pManager->m_oSettings.local_editors_path;
	std::wstring::size_type nPosThemes = sThemesPath.rfind(L"apps/api/documents/index.html");
	if (nPosThemes != std::wstring::npos)
		sThemesPath = sThemesPath.substr(0, nPosThemes);
	sThemesPath += L"../sdkjs/slide/themes";
	return sThemesPath;
}

void X2T_AddThemesPath(NSStringUtils::CStringBuilder& oBuilder, CAscApplicationManager* pManager)
{
	oBuilder.WriteString(L"<m_sThemeDir>");
	oBuilder.WriteEncodeXmlString(GetPresentationThemes(pManager));
	oBuilder.WriteString(L"</m_sThemeDir>");
}

static bool IsFormatSupportCrypto(const int& nFormat)
{
	switch (nFormat)
	{
	case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
	case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF:
	case AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM:
	case AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF:
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
		return true;
	}
	default:
		break;
	}
	return false;
}

class CAscLocalFileInfo
{
public:
	std::wstring m_sFileSrc;
	std::wstring m_sRecoveryDir;

	std::wstring m_sTemplateUrl;

	// чтобы не ждать лишнего времени - грузим страницу и скрипты
	// ОДНОВРЕМЕННО с конвертацией. поэтому нужен счетчик, чтобы запускать открытие
	// после и конвертации и после вызова OfflineStart (js)
	int m_nCounterConvertion;

	int m_nCurrentFileFormat;
	bool m_bIsSaved;

	bool m_bIsModified;

	std::wstring m_sPassword;
	std::wstring m_sSaveJsonParams;

	// for change password in pdf
	std::wstring m_sOldPassword;

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

		m_sTemplateUrl = oSrc.m_sTemplateUrl;

		m_nCounterConvertion = oSrc.m_nCounterConvertion;

		m_nCurrentFileFormat = oSrc.m_nCurrentFileFormat;
		m_bIsSaved = oSrc.m_bIsSaved;

		m_sPassword = oSrc.m_sPassword;
		m_sDocumentInfo = oSrc.m_sDocumentInfo;
		m_sSaveJsonParams = oSrc.m_sSaveJsonParams;

		m_sOldPassword = oSrc.m_sOldPassword;
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
		NSSystem::CLocalFileLocker* m_pLocker;

	public:

		COOXMLZipDirectory(CAscApplicationManager* pManager)
		{
			m_pManager = pManager;
			m_pLocker = NULL;
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
				X2T_AddThemesPath(oBuilder, m_pManager);
				oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
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

		int Close(bool bIsSimple = false)
		{
			if (bIsSimple)
			{
				NSDirectory::DeleteDirectory(m_sDirectory);
				return 0;
			}

			std::wstring sLockerSavedPath;
			if (m_pLocker)
			{
				sLockerSavedPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
				if (NSFile::CFileBinary::Exists(sLockerSavedPath))
					NSFile::CFileBinary::Remove(sLockerSavedPath);
			}
			else
			{
				sLockerSavedPath = m_sFile;
			}

			if (m_sPassword.empty())
			{
				if (!m_pLocker)
					NSFile::CFileBinary::Remove(m_sFile);
				COfficeUtils oCOfficeUtils(NULL);
				HRESULT hRes = oCOfficeUtils.CompressFileOrDirectory(m_sDirectory, sLockerSavedPath, true);
				NSDirectory::DeleteDirectory(m_sDirectory);

				if (m_pLocker && (hRes == S_OK))
				{
					hRes = m_pLocker->SaveFile(sLockerSavedPath) ? S_OK : S_FALSE;
					NSFile::CFileBinary::Remove(sLockerSavedPath);
				}

				return (hRes == S_OK) ? 0 : 1;
			}

			std::wstring sTempFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"PASS");
			if (NSFile::CFileBinary::Exists(sTempFile))
				NSFile::CFileBinary::Remove(sTempFile);

			sTempFile += (L"." + NSFile::GetFileExtention(m_sFile));

			COfficeUtils oCOfficeUtils(NULL);
			oCOfficeUtils.CompressFileOrDirectory(m_sDirectory, sTempFile, true);
			NSDirectory::DeleteDirectory(m_sDirectory);

			int nFormatType = CCefViewEditor::GetFileFormat(m_sFile);

			std::wstring sTempFileXml = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"XML");
			if (NSFile::CFileBinary::Exists(sTempFileXml))
				NSFile::CFileBinary::Remove(sTempFileXml);
			sTempFileXml += L".xml";

			if (!m_pLocker)
				NSFile::CFileBinary::Remove(m_sFile);

			NSStringUtils::CStringBuilder oBuilder;
			oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
			oBuilder.WriteEncodeXmlString(sTempFile);
			oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
			oBuilder.WriteEncodeXmlString(sLockerSavedPath);
			oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");
			oBuilder.WriteString(std::to_wstring(nFormatType));
			oBuilder.WriteString(L"</m_nFormatTo>");
			X2T_AddThemesPath(oBuilder, m_pManager);
			oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
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

			if (0 == nReturnCode && m_pLocker)
			{
				nReturnCode = m_pLocker->SaveFile(sLockerSavedPath) ? 0 : 80;
				NSFile::CFileBinary::Remove(sLockerSavedPath);
			}

			return (0 == nReturnCode) ? 0 : 1;
		}

	public:
		std::wstring GetDirectory()
		{
			return m_sDirectory;
		}
	};
}

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

	// конвертер запустился для сравнения
	std::wstring m_sComparingFile;

public:
	CASCFileConverterToEditor() : NSThreads::CBaseThread()
	{
		m_pEvents = NULL;
		m_pManager = NULL;
		m_pView = NULL;

		m_bIsNativeOpening = false;

		// перешли на новый вьюер => отключили старый
		m_bIsNativeSupport = false;

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
		if (!m_sComparingFile.empty())
		{
			std::wstring sCompareDir = m_oInfo.m_sRecoveryDir + L"/compare";

			if (NSDirectory::Exists(sCompareDir))
				NSDirectory::DeleteDirectory(sCompareDir);
			NSDirectory::CreateDirectory(sCompareDir);

			std::wstring sInputFile = sCompareDir + L"/" + NSFile::GetFileName(m_sComparingFile);
			NSFile::CFileBinary::Copy(m_sComparingFile, sInputFile);

			NSStringUtils::CStringBuilder oBuilder;
			oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
			oBuilder.WriteEncodeXmlString(sInputFile);
			oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
			oBuilder.WriteEncodeXmlString(sCompareDir);
			oBuilder.WriteString(L"/Editor.bin</m_sFileTo><m_nFormatTo>8192</m_nFormatTo>");
			X2T_AddThemesPath(oBuilder, m_pManager);
			oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
			oBuilder.WriteString(L"<m_sFontDir>");
			oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
			oBuilder.WriteString(L"</m_sFontDir>");
			if (!m_sAdditionalConvertation.empty())
				oBuilder.WriteString(m_sAdditionalConvertation);
			m_sAdditionalConvertation = L"";
			oBuilder.WriteString(L"</TaskQueueDataConvert>");

			std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";
			std::wstring sParams = sCompareDir + L"/params.xml";

			NSFile::CFileBinary::SaveToFile(sParams, oBuilder.GetData(), true);
			int nReturnCode = NSX2T::Convert(sConverterExe, sParams, m_pManager, m_pManager->m_pInternal->m_bIsEnableConvertLogs);
			NSFile::CFileBinary::Remove(sParams);

			m_pEvents->OnFileConvertToEditor(nReturnCode);

			m_bRunThread = FALSE;
			return 0;
		}

		if (NSFile::CFileBinary::Exists(m_oInfo.m_sRecoveryDir + L"/Editor.bin"))
		{
			if (true)
			{
				std::wstring sTmp = m_oInfo.m_sRecoveryDir + L"/Editor.bin";
				NSFile::CFileBinary oFile;

				AscEditorType eType = AscEditorType::etDocument;
				if (oFile.OpenFile(sTmp) && 4 < oFile.GetFileSize())
				{
					BYTE _buffer[4];
					DWORD dwRead = 0;
					oFile.ReadFile(_buffer, 4, dwRead);

					if (4 == dwRead)
					{
						if ('X' == _buffer[0] && 'L' == _buffer[1] && 'S' == _buffer[2] && 'Y' == _buffer[3])
							eType = AscEditorType::etSpreadsheet;
						else if ('P' == _buffer[0] && 'P' == _buffer[1] && 'T' == _buffer[2] && 'Y' == _buffer[3])
							eType = AscEditorType::etPresentation;
						else if ('P' == _buffer[0] && 'K' == _buffer[1])
							eType = AscEditorType::etDraw;
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

		std::wstring sNameCorrect = m_sName.empty() ? NSFile::GetFileName(sLocalFilePath) : m_sName;
		NSStringUtils::string_replace(sNameCorrect, L"/", L"_");
		NSStringUtils::string_replace(sNameCorrect, L"\\", L"_");
		std::wstring sDestinationPath = m_oInfo.m_sRecoveryDir + L"/" + sNameCorrect;

		if (!m_oInfo.m_sTemplateUrl.empty())
		{
			if (NSFileDownloader::IsNeedDownload(m_oInfo.m_sTemplateUrl))
			{
				NSNetwork::NSFileTransport::CFileDownloader oDownloader(m_oInfo.m_sTemplateUrl, false);
				oDownloader.SetFilePath(sDestinationPath);

				oDownloader.Start( 0 );
				while ( oDownloader.IsRunned() )
				{
					NSThreads::Sleep( 10 );
				}

				sLocalFilePath = sDestinationPath;
			}
			else
			{
				sLocalFilePath = m_oInfo.m_sTemplateUrl;
			}
		}

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

			AscEditorType eType = AscEditorType::etDocument;
			if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
				eType = AscEditorType::etPresentation;
			else if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
				eType = AscEditorType::etSpreadsheet;
			else if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
				eType = AscEditorType::etPdf;
			else if (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF)
				eType = AscEditorType::etPdf;
			else if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_DRAW)
				eType = AscEditorType::etDraw;

			pData->put_Type((int)eType);

			NSEditorApi::CAscCefMenuEvent* pEvent = m_pView->CreateCefEvent(ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE);
			pEvent->m_pData = pData;
			pListener->OnEvent(pEvent);
		}

		m_sNativeSrcPath = sDestinationPath;
		if (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML)
		{
			std::wstring sExt = NSFile::GetFileExtention(sDestinationPath);
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
						std::wstring sDestinationCopyDir = m_oInfo.m_sRecoveryDir + L"/" + NSFile::GetFileName(sTestDirectory);
						NSDirectory::CreateDirectory(sDestinationCopyDir);

						std::vector<std::wstring> arrFiles = NSDirectory::GetFiles(sTestDirectory, false);

						for (std::vector<std::wstring>::iterator i = arrFiles.begin(); i != arrFiles.end(); i++)
						{
							std::wstring sTestFileCopy = NSFile::GetFileName(*i);
							NSFile::CFileBinary::Copy(*i, sDestinationCopyDir + L"/" + sTestFileCopy);
						}
					}
				}
			}
		}

		sLocalFilePath = CorrectPathW(sLocalFilePath);
		sDestinationPath = CorrectPathW(sDestinationPath);

		// пустые файлы
		if (true)
		{
			NSFile::CFileBinary oFileEmptyCheck;
			if (oFileEmptyCheck.OpenFile(sLocalFilePath))
			{
				if (0 == oFileEmptyCheck.GetFileSize())
				{
					AscEditorType nFormatEmpty = AscEditorType::etDocument;
					if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_PRESENTATION)
						nFormatEmpty = AscEditorType::etPresentation;
					else if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_SPREADSHEET)
						nFormatEmpty = AscEditorType::etSpreadsheet;

					sLocalFilePath = m_pManager->GetNewFilePath(nFormatEmpty);
				}
				oFileEmptyCheck.CloseFile();
			}
		}

		bool bIsCopied = false;
		if (sLocalFilePath != sDestinationPath)
		{
			NSFile::CFileBinary::Copy(sLocalFilePath, sDestinationPath);
			bIsCopied = true;
		}

		bool bIsTestFile = true;
		if (!NSFile::CFileBinary::Exists(sDestinationPath))
		{
			bIsTestFile = false;
		}
		else
		{
			NSFile::CFileBinary oTestFile;
			if (!NSFile::CFileBinary::Exists(sLocalFilePath) && oTestFile.OpenFile(sDestinationPath))
			{
				if (0 == oTestFile.GetFileSize())
					bIsTestFile = false;
				oTestFile.CloseFile();
			}
		}
		if (!bIsTestFile)
		{
			m_pEvents->OnFileConvertToEditor(AVS_ERROR_FILEACCESS);
			m_bRunThread = FALSE;
			return 0;
		}

		if (bIsCopied && (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF || m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA))
			NSSystem::CFileLocker::RemoveRestrictionFlags(sDestinationPath);

		std::wstring sNameInfo = m_oInfo.m_sRecoveryDir + L"/asc_name.info";
		if (NSFile::CFileBinary::Exists(sNameInfo))
			NSFile::CFileBinary::Remove(sNameInfo);

		NSStringUtils::CStringBuilder oBuilderInfo;
		oBuilderInfo.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><info type=\"" + std::to_wstring(m_oInfo.m_nCurrentFileFormat) + L"\" name=\"");
		oBuilderInfo.WriteEncodeXmlString(NSFile::GetFileName(sDestinationPath));
		oBuilderInfo.WriteString(L"\" />");
		NSFile::CFileBinary::SaveToFile(sNameInfo, oBuilderInfo.GetData(), true);

		if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM)
		{
			if (m_bIsNativeSupport)
			{
				NSDirectory::CreateDirectory(m_oInfo.m_sRecoveryDir + L"/media");
				this->NativeViewerOpen(true); // async. sleep!!!
			}
			m_pEvents->OnFileConvertToEditor(0);
			m_bRunThread = FALSE;
			return 0;
		}

		int nOutputFormat = 8192;
		if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_DRAW)
		{
			// Move to binary
			if (false)
			{
				NSFile::CFileBinary::Copy(sLocalFilePath, m_oInfo.m_sRecoveryDir + L"/Editor.bin");

				m_pEvents->OnFileConvertToEditor(0);
				m_bRunThread = FALSE;
				return 0;
			}
			else
			{
				nOutputFormat = 8197;
			}
		}

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
			sParams += oBuilderFonts.GetData();
		}

		NSStringUtils::CStringBuilder oBuilder;
		oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
		oBuilder.WriteEncodeXmlString(sDestinationPath);
		oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
		oBuilder.WriteEncodeXmlString(m_oInfo.m_sRecoveryDir);
		oBuilder.WriteString(L"/Editor.bin</m_sFileTo><m_nFormatTo>");
		oBuilder.AddInt(nOutputFormat);
		oBuilder.WriteString(L"</m_nFormatTo>");
		X2T_AddThemesPath(oBuilder, m_pManager);
		oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
		oBuilder.WriteString(sParams);

		std::wstring sDstTmpDir = NSDirectory::CreateDirectoryWithUniqueName(m_oInfo.m_sRecoveryDir);

		oBuilder.WriteString(L"<m_sTempDir>");
		oBuilder.WriteEncodeXmlString(sDstTmpDir);
		oBuilder.WriteString(L"</m_sTempDir>");

		oBuilder.WriteString(L"</TaskQueueDataConvert>");

		std::wstring sXmlConvert = oBuilder.GetData();

		std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";

		std::wstring sTempFileForParams = m_oInfo.m_sRecoveryDir + L"/params_from.xml";
		NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);

		int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileForParams, m_pManager, m_pManager->m_pInternal->m_bIsEnableConvertLogs);

		if (!m_pManager->m_pInternal->m_bDebugInfoSupport)
		{
			NSFile::CFileBinary::Remove(sTempFileForParams);
			NSDirectory::DeleteDirectory(sDstTmpDir);
		}

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
			m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX ||
			m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF ||
			m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM)
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
				ULONG nBufferSize = 0;
				BYTE *pBuffer = NULL;

				bool bIsNeedCheck = false;
				COfficeUtils oCOfficeUtils(NULL);
				if (S_OK == oCOfficeUtils.LoadFileFromArchive(sFile, L"[Content_Types].xml", &pBuffer, nBufferSize))
				{
					const char *find1 = "application/vnd.openxmlformats-package.digital-signature-origin";
					const char *find2 = "application/vnd.openxmlformats-package.digital-signature-xmlsignature+xml";

					std::string strContentTypes((char*)pBuffer, nBufferSize);
					if (std::string::npos != strContentTypes.find(find1) && std::string::npos != strContentTypes.find(find2))
					{
						bIsNeedCheck = true;
					}
				}
				delete []pBuffer;
				pBuffer = NULL;

				if (bIsNeedCheck)
				{
					std::wstring sUnzipDir = NSDirectory::CreateDirectoryWithUniqueName(NSDirectory::GetTempPath());
					NSDirectory::CreateDirectory(sUnzipDir);

					COfficeUtils oCOfficeUtils2(NULL);
					if (S_OK == oCOfficeUtils2.ExtractToDirectory(sFile, sUnzipDir, NULL, 0))
					{
						m_pVerifier = new COOXMLVerifier(sUnzipDir);
						NSDirectory::DeleteDirectory(sUnzipDir);
					}
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
			NSStringUtils::string_replace(sParam, L"\"", L"\\\"");
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
	bool m_bIsWorking; // not m_bIsRunned

	// временный путь для сохранения (чтобы делать локфайл)
	NSSystem::CLocalFileLocker* m_pLocker;

public:
	CASCFileConverterFromEditor() : NSThreads::CBaseThread()
	{
		m_bIsRetina = false;
		m_nTypeEditorFormat = -1;
		m_bIsEditorWithChanges = false;
		m_bIsWorking = false;
		m_pLocker = NULL;
	}
	virtual void Start(int lPriority)
	{
		if (m_bRunThread)
			return;
		m_bIsWorking = true;
		NSThreads::CBaseThread::Start(lPriority);
	}
	bool IsWorking()
	{
		// эта функция, чтобы знать на ONSAVE - идет ли работа.
		// m_bIsRunThread сбросится позже
		return m_bIsWorking;
	}
	virtual ~CASCFileConverterFromEditor()
	{
		Stop();
	}

	virtual DWORD ThreadProc()
	{
		std::wstring sLocalFilePath = m_oInfo.m_sFileSrc;

		// если true - то делаем архив 123.png.zip
		// если false - то делаем папку 123.png
		bool bIsAddZipToRasterFormats = false;
		std::wstring sRasterDictionaryExtract = L"";

#ifdef WIN32
		if (0 == sLocalFilePath.find(L"//"))
		{
			sLocalFilePath = L"\\\\" + sLocalFilePath.substr(2);
		}
#endif

		if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_IMAGE)
		{
			if (bIsAddZipToRasterFormats)
				sLocalFilePath += L".zip";
			else
			{
				sRasterDictionaryExtract = sLocalFilePath;
				sLocalFilePath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
				if (NSFile::CFileBinary::Exists(sLocalFilePath))
					NSFile::CFileBinary::Remove(sLocalFilePath);
			}
		}

		std::wstring sLockerSavedPath;
		if (m_pLocker)
		{
			sLockerSavedPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
			if (NSFile::CFileBinary::Exists(sLockerSavedPath))
				NSFile::CFileBinary::Remove(sLockerSavedPath);
		}

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
			std::wstring sPathTmp = m_oInfo.m_sRecoveryDir + L"/" + NSFile::GetFileName(m_sOriginalFileNameCrossPlatform);
			oBuilder.WriteEncodeXmlString(sPathTmp);
			oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
		}

		if (!sLockerSavedPath.empty())
			oBuilder.WriteEncodeXmlString(sLockerSavedPath);
		else
			oBuilder.WriteEncodeXmlString(sLocalFilePath);

		oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");

		if (AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA == m_oInfo.m_nCurrentFileFormat)
		{
			oBuilder.WriteString(std::to_wstring(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF));
			sParams += L"<m_bIsPDFA>true</m_bIsPDFA>";
		}
		else if (AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF == m_oInfo.m_nCurrentFileFormat &&
			(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF == m_nTypeEditorFormat ||
			 AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM == m_nTypeEditorFormat ||
			 AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF == m_nTypeEditorFormat))
		{
			oBuilder.WriteString(std::to_wstring(AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF));
		}
		else
		{
			oBuilder.WriteString(std::to_wstring(m_oInfo.m_nCurrentFileFormat));
		}

		oBuilder.WriteString(L"</m_nFormatTo>");
		X2T_AddThemesPath(oBuilder, m_pManager);

		if (NSFile::CFileBinary::Exists(m_oInfo.m_sRecoveryDir + L"/changes/changes0.json"))
			oBuilder.WriteString(L"<m_bFromChanges>true</m_bFromChanges>");
		else
			oBuilder.WriteString(L"<m_bFromChanges>false</m_bFromChanges>");

		oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional><m_sAllFontsPath>");

		oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
		oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath><m_nCsvTxtEncoding>46</m_nCsvTxtEncoding><m_nCsvDelimiter>4</m_nCsvDelimiter>");
		oBuilder.WriteString(sParams);

		if (!m_oInfo.m_sOldPassword.empty() &&
			((m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_CROSSPLATFORM) || !m_sOriginalFileNameCrossPlatform.empty()))
		{
			oBuilder.WriteString(L"<m_sPassword>");
			oBuilder.WriteEncodeXmlString(m_oInfo.m_sOldPassword);
			oBuilder.WriteString(L"</m_sPassword>");
			m_oInfo.m_sOldPassword = L"";
		}

		if (!m_oInfo.m_sPassword.empty() && IsFormatSupportCrypto(m_oInfo.m_nCurrentFileFormat))
		{
			oBuilder.WriteString(L"<m_sSavePassword>");
			oBuilder.WriteEncodeXmlString(m_oInfo.m_sPassword);
			oBuilder.WriteString(L"</m_sSavePassword>");
		}

		if (!m_oInfo.m_sDocumentInfo.empty())
		{
			oBuilder.WriteString(L"<m_sDocumentID>");
			oBuilder.WriteEncodeXmlString(m_oInfo.m_sDocumentInfo);
			oBuilder.WriteString(L"</m_sDocumentID>");
		}
		m_oInfo.m_sDocumentInfo = L"";

		oBuilder.WriteString(L"<m_sJsonParams>");

		if (m_oInfo.m_sSaveJsonParams.empty())
		{
			oBuilder.WriteString(L"{");

			oBuilder.WriteEncodeXmlString(L"\"spreadsheetLayout\":{\"fitToWidth\":1,\"fitToHeight\":1}");
			if (m_bIsRetina)
				oBuilder.WriteEncodeXmlString(L",\"printOptions\":{\"retina\":1}");

			oBuilder.WriteString(L"}");
		}
		else
		{
			oBuilder.WriteEncodeXmlString(m_oInfo.m_sSaveJsonParams);
			m_oInfo.m_sSaveJsonParams = L"";
		}

		oBuilder.WriteString(L"</m_sJsonParams>");

		// disable cache
		oBuilder.WriteString(L"<m_nDoctParams>1</m_nDoctParams>");

		// tmp dir
		std::wstring sDstTmpDir = NSDirectory::CreateDirectoryWithUniqueName(m_oInfo.m_sRecoveryDir);

		oBuilder.WriteString(L"<m_sTempDir>");
		oBuilder.WriteEncodeXmlString(sDstTmpDir);
		oBuilder.WriteString(L"</m_sTempDir>");

		if (m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_IMAGE)
		{
			oBuilder.WriteString(L"<m_oThumbnail><first>false</first>");

			if (m_oInfo.m_nCurrentFileFormat == AVS_OFFICESTUDIO_FILE_IMAGE_JPG)
				oBuilder.WriteString(L"<format>3</format>");

			oBuilder.WriteString(L"</m_oThumbnail>");
		}

		oBuilder.WriteString(L"</TaskQueueDataConvert>");

		std::wstring sXmlConvert = oBuilder.GetData();

		std::wstring sConverterExe = m_pManager->m_oSettings.file_converter_path + L"/x2t";

		std::wstring sTempFileForParams = m_oInfo.m_sRecoveryDir + L"/params_to.xml";
		NSFile::CFileBinary::SaveToFile(sTempFileForParams, sXmlConvert, true);

		if (!m_pManager->m_pInternal->GetEditorPermission())
		{
			// return!
			m_bIsWorking = false;
			m_pEvents->OnFileConvertFromEditor(ASC_CONSTANT_CANCEL_SAVE);
			m_pLocker = NULL;
			m_bRunThread = FALSE;
			return 0;
		}

		if (m_pManager->m_pInternal->m_pAdditional)
			m_pManager->m_pInternal->m_pAdditional->CheckSaveStart(m_oInfo.m_sRecoveryDir, m_nTypeEditorFormat);

		int nReturnCode = NSX2T::Convert(sConverterExe, sTempFileForParams, m_pManager, m_pManager->m_pInternal->m_bIsEnableConvertLogs);

		m_bIsRetina = false;

		if (!m_pManager->m_pInternal->m_bDebugInfoSupport)
		{
			NSFile::CFileBinary::Remove(sTempFileForParams);
			NSDirectory::DeleteDirectory(sDstTmpDir);
		}

		if (m_pLocker)
		{
			m_pLocker->SaveFile(sLockerSavedPath);
			m_pLocker = NULL;

			if (NSFile::CFileBinary::Exists(sLockerSavedPath))
				NSFile::CFileBinary::Remove(sLockerSavedPath);
		}
		else
		{
			if ((m_oInfo.m_nCurrentFileFormat & AVS_OFFICESTUDIO_FILE_IMAGE) != 0)
			{
				if (!bIsAddZipToRasterFormats)
				{
					NSDirectory::CreateDirectory(sRasterDictionaryExtract);
					COfficeUtils oUtils;
					oUtils.ExtractToDirectory(sLocalFilePath, sRasterDictionaryExtract, NULL, 0);
					NSFile::CFileBinary::Remove(sLocalFilePath);
				}
			}
		}

		m_bIsWorking = false;
		m_pEvents->OnFileConvertFromEditor(nReturnCode, m_oInfo.m_sPassword);

		m_pLocker = NULL;

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
	int_64_type m_nFrameId;

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

	bool m_bIsLogs;

public:
	CSimpleConverter()
	{
		m_nOutputFormat = -1;
		m_pEvents = NULL;
		m_pManager = NULL;
		m_bIsLogs = false;
	}

	void Convert(bool bIsLogs = false)
	{
		m_bIsLogs = bIsLogs;
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
			// если true - то делаем архив 123.png.zip
			// если false - то делаем папку 123.png
			bool bIsAddZipToRasterFormats = false;
			std::wstring sRasterDictionaryExtract = L"";

			std::wstring sOutputFile = m_sOutputPath;

			if (m_nOutputFormat & AVS_OFFICESTUDIO_FILE_IMAGE)
			{
				if (bIsAddZipToRasterFormats)
					sOutputFile += L".zip";
				else
				{
					sRasterDictionaryExtract = sOutputFile;
					sOutputFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"OL");
					if (NSFile::CFileBinary::Exists(sOutputFile))
						NSFile::CFileBinary::Remove(sOutputFile);
				}
			}

			std::wstring sAdditionXml = L"";
			NSStringUtils::CStringBuilder oBuilder;
			oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
			oBuilder.WriteEncodeXmlString(m_sRecoverFolder + L"/EditorWithChanges.bin");
			oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
			oBuilder.WriteEncodeXmlString(sOutputFile);
			oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");

			int nOutputFormat = NSCommon::CorrectSaveFormat(m_nOutputFormat);
			if (AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA == nOutputFormat)
			{
				nOutputFormat = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF;
				sAdditionXml = L"<m_bIsPDFA>true</m_bIsPDFA>";
			}


			oBuilder.WriteString(std::to_wstring(nOutputFormat));
			oBuilder.WriteString(L"</m_nFormatTo><m_sFontDir>");
			oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
			oBuilder.WriteString(L"</m_sFontDir>");
			oBuilder.WriteString(UTF8_TO_U(m_sOutputParams));
			oBuilder.WriteString(L"<m_bIsNoBase64>false</m_bIsNoBase64>");
			X2T_AddThemesPath(oBuilder, m_pManager);
			oBuilder.WriteString(L"<m_sAllFontsPath>");
			oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
			oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath>");

			std::wstring sDstTmpDir = NSDirectory::CreateDirectoryWithUniqueName(m_sRecoverFolder);

			oBuilder.WriteString(L"<m_sTempDir>");
			oBuilder.WriteEncodeXmlString(sDstTmpDir);
			oBuilder.WriteString(L"</m_sTempDir>");

			if (!sAdditionXml.empty())
				oBuilder.WriteString(sAdditionXml);

			if (m_nOutputFormat & AVS_OFFICESTUDIO_FILE_IMAGE)
			{
				oBuilder.WriteString(L"<m_oThumbnail><first>false</first>");

				if (m_nOutputFormat == AVS_OFFICESTUDIO_FILE_IMAGE_JPG)
					oBuilder.WriteString(L"<format>3</format>");

				oBuilder.WriteString(L"</m_oThumbnail>");
			}

			oBuilder.WriteString(L"</TaskQueueDataConvert>");

			std::wstring sTempFileForParams = m_sRecoverFolder + L"/params_simple_converter.xml";
			NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

			nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager, m_bIsLogs);

			NSFile::CFileBinary::Remove(sTempFileForParams);
			NSDirectory::DeleteDirectory(sDstTmpDir);

			if (m_nOutputFormat & AVS_OFFICESTUDIO_FILE_IMAGE && !bIsAddZipToRasterFormats && 0 == nReturnCode)
			{
				NSDirectory::CreateDirectory(sRasterDictionaryExtract);
				COfficeUtils oUtils;
				oUtils.ExtractToDirectory(sOutputFile, sRasterDictionaryExtract, NULL, 0);
				NSFile::CFileBinary::Remove(sOutputFile);
			}
		}

		m_pEvents->OnFileConvertFromEditor(nReturnCode);
		NSFile::CFileBinary::Remove(m_sRecoverFolder + L"/EditorWithChanges.bin");

		m_bRunThread = FALSE;
		return 0;
	}
};

class CConvertFileInEditor : public NSThreads::CBaseThread
{
public:
	std::wstring m_sFileFolder;
	std::wstring m_sSrcFilePath;
	int m_nOutputFormat;
	std::string m_sOutputParams;
	int_64_type m_nFrameId;

	bool m_bIsApplyChanges;
	std::wstring m_sDstFilePath;
	std::wstring m_sAdditionalParams;

	IASCFileConverterEvents* m_pEvents;
	CAscApplicationManager* m_pManager;

public:
	CConvertFileInEditor()
	{
		m_nOutputFormat = -1;
		m_pEvents = NULL;
		m_pManager = NULL;
		m_nFrameId = 0;
		m_bIsApplyChanges = false;
	}

	virtual DWORD ThreadProc()
	{
		int nReturnCode = 0;

		std::wstring sSrcFile = L"";
		std::wstring sDstFile = L"";
		std::wstring sDstTmpDir = L"";

		if (!m_bIsApplyChanges)
		{
			if (NSSystem::LockType::ltNone == NSSystem::CLocalFileLocker::IsLocked(m_sSrcFilePath))
			{
				sSrcFile = m_sSrcFilePath;
			}
			else
			{
				std::wstring sFileSrc = m_sFileFolder + L"/" + NSFile::GetFileName(m_sSrcFilePath);
				if (NSFile::CFileBinary::Copy(m_sSrcFilePath, sFileSrc))
					sSrcFile = sFileSrc;
				else
					sSrcFile = m_sSrcFilePath;
			}

			sDstFile = m_sFileFolder + L"/Editor.bin";

			sDstTmpDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(m_sFileFolder, L"FC_");
			if (NSFile::CFileBinary::Exists(sDstTmpDir))
				NSFile::CFileBinary::Remove(sDstTmpDir);
		}
		else
		{
			sSrcFile = m_sFileFolder + L"/Editor.bin";
			sDstFile = m_sDstFilePath;

			sDstTmpDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"FC_");
			if (NSFile::CFileBinary::Exists(sDstTmpDir))
				NSFile::CFileBinary::Remove(sDstTmpDir);
		}

		NSStringUtils::CStringBuilder oBuilder;
		oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
		oBuilder.WriteEncodeXmlString(sSrcFile);

		oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
		oBuilder.WriteEncodeXmlString(sDstFile);
		oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");

		oBuilder.WriteString(std::to_wstring(m_nOutputFormat));
		oBuilder.WriteString(L"</m_nFormatTo><m_sFontDir>");
		oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
		oBuilder.WriteString(L"</m_sFontDir>");
		oBuilder.WriteString(UTF8_TO_U(m_sOutputParams));
		oBuilder.WriteString(L"<m_bIsNoBase64>false</m_bIsNoBase64>");
		X2T_AddThemesPath(oBuilder, m_pManager);
		oBuilder.WriteString(L"<m_sAllFontsPath>");
		oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
		oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath>");

		NSDirectory::CreateDirectory(sDstTmpDir);

		oBuilder.WriteString(L"<m_sTempDir>");
		oBuilder.WriteEncodeXmlString(sDstTmpDir);
		oBuilder.WriteString(L"</m_sTempDir>");

		oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");

		if (m_bIsApplyChanges)
		{
			if (NSFile::CFileBinary::Exists(m_sFileFolder + L"/changes/changes0.json"))
				oBuilder.WriteString(L"<m_bFromChanges>true</m_bFromChanges>");
			else
				oBuilder.WriteString(L"<m_bFromChanges>false</m_bFromChanges>");
		}

		if (!m_sAdditionalParams.empty())
		{
			oBuilder.WriteString(L"<m_sJsonParams>");
			oBuilder.WriteEncodeXmlString(m_sAdditionalParams);
			oBuilder.WriteString(L"</m_sJsonParams>");
		}

		// disable cache
		oBuilder.WriteString(L"<m_nDoctParams>1</m_nDoctParams>");

		oBuilder.WriteString(L"</TaskQueueDataConvert>");

		std::wstring sTempFileForParams = sDstTmpDir + L"/params.xml";
		NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

		nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager);

		NSFile::CFileBinary::Remove(sTempFileForParams);
		NSDirectory::DeleteDirectory(sDstTmpDir);

		m_pEvents->OnFileConvertFromEditor(nReturnCode);

		m_bRunThread = FALSE;
		return 0;
	}

	void SetModeApplyChanges()
	{
		m_bIsApplyChanges = true;
		std::wstring sSrcFileName = NSFile::GetFileName(m_sSrcFilePath);
		std::wstring::size_type posDot = sSrcFileName.rfind(L".");
		if (std::wstring::npos != posDot && 0 != posDot)
			sSrcFileName = sSrcFileName.substr(0, posDot);

		COfficeFileFormatChecker oChecker;
		sSrcFileName += oChecker.GetExtensionByType(m_nOutputFormat);

		m_sDstFilePath = NSFile::CFileBinary::GetTempPath() + L"/" + sSrcFileName;
	}
};

class CSimpleConverterExternal;
class IASCFileConverterExternalEvents
{
public:
	virtual void OnFileConvert(const int& error, CSimpleConverterExternal* converter) = 0;
};

class CSimpleConverterExternal : public NSThreads::CBaseThread
{
public:
	std::wstring m_sInputFile;

	std::wstring m_sTempDirectory;
	std::wstring m_sOutputFile;

	std::wstring m_sOutputParams;
	int m_nOutputFormat;

	CAscApplicationManager* m_pManager;
	IASCFileConverterExternalEvents* m_pEvents;

	int_64_type m_nFrameId;
	int m_nId;

public:
	CSimpleConverterExternal()
	{
		m_sTempDirectory = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"CV");
		if (NSFile::CFileBinary::Exists(m_sTempDirectory))
			NSFile::CFileBinary::Remove(m_sTempDirectory);

		NSDirectory::CreateDirectory(m_sTempDirectory);

		m_sOutputFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSDirectory::GetTempPath(), L"CV");
		if (NSFile::CFileBinary::Exists(m_sOutputFile))
			NSFile::CFileBinary::Remove(m_sOutputFile);

		m_nOutputFormat = -1;
		m_pManager = NULL;
		m_pEvents = NULL;
		m_nFrameId = 0;
		m_nId = 0;
	}

	virtual ~CSimpleConverterExternal()
	{
		NSDirectory::DeleteDirectory(m_sTempDirectory);
	}

	void Convert()
	{
		Start(0);
	}

	virtual DWORD ThreadProc()
	{
		int nReturnCode = 0;

		NSStringUtils::CStringBuilder oBuilder;
		oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
		oBuilder.WriteEncodeXmlString(m_sInputFile);
		oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
		oBuilder.WriteEncodeXmlString(m_sOutputFile);
		oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>");

		std::wstring sAdditionXml = L"";
		if (AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA == m_nOutputFormat)
		{
			m_nOutputFormat = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF;
			sAdditionXml = L"<m_bIsPDFA>true</m_bIsPDFA>";
		}

		oBuilder.WriteString(std::to_wstring(m_nOutputFormat));
		oBuilder.WriteString(L"</m_nFormatTo><m_sFontDir>");
		oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
		oBuilder.WriteString(L"</m_sFontDir>");
		oBuilder.WriteString(m_sOutputParams);
		oBuilder.WriteString(L"<m_bIsNoBase64>false</m_bIsNoBase64>");
		oBuilder.WriteString(L"<m_sAllFontsPath>");
		oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
		oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath>");

		oBuilder.WriteString(L"<m_sTempDir>");
		oBuilder.WriteEncodeXmlString(m_sTempDirectory);
		oBuilder.WriteString(L"</m_sTempDir>");

		if (!sAdditionXml.empty())
			oBuilder.WriteString(sAdditionXml);

		if (m_nOutputFormat & AVS_OFFICESTUDIO_FILE_IMAGE)
		{
			oBuilder.WriteString(L"<m_oThumbnail><first>false</first>");

			if (m_nOutputFormat == AVS_OFFICESTUDIO_FILE_IMAGE_JPG)
				oBuilder.WriteString(L"<format>3</format>");

			oBuilder.WriteString(L"</m_oThumbnail>");
		}

		oBuilder.WriteString(L"</TaskQueueDataConvert>");

		std::wstring sTempFileForParams = m_sTempDirectory + L"/params_simple_converter.xml";
		NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

		nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager, false);

		NSFile::CFileBinary::Remove(sTempFileForParams);
		NSDirectory::DeleteDirectory(m_sTempDirectory);

		m_pEvents->OnFileConvert(nReturnCode, this);

		m_bRunThread = FALSE;
		return 0;
	}
};

bool GetTemplateImage(CAscApplicationManager* pManager, const std::wstring sTempDir,
					  const int& w, const int& h,
					  const std::wstring& sFile, const std::wstring& sImagesDir, const std::wstring& sOutput)
{
	NSStringUtils::CStringBuilder oBuilder;
	oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
	oBuilder.WriteEncodeXmlString(sFile);
	oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
	oBuilder.WriteEncodeXmlString(sOutput);
	oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>1025</m_nFormatTo><m_sFontDir>");
	oBuilder.WriteEncodeXmlString(pManager->m_oSettings.fonts_cache_info_path);
	oBuilder.WriteString(L"</m_sFontDir><m_bIsNoBase64>false</m_bIsNoBase64><m_sAllFontsPath>");
	oBuilder.WriteEncodeXmlString(pManager->m_oSettings.fonts_cache_info_path);
	oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath>");

	oBuilder.WriteString(L"<m_sTempDir>");
	oBuilder.WriteEncodeXmlString(sTempDir);
	oBuilder.WriteString(L"</m_sTempDir>");

	oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
	oBuilder.WriteString(L"<m_bFromChanges>false</m_bFromChanges>");

	std::wstring json_params = L"{\
'spreadsheetLayout':{\
'fitToWidth':0,'fitToHeight':0,'orientation':'landscape',\
'sheetsProps':{\
'0':{\
'headings':false,'printTitlesWidth':null,'printTitlesHeight':null,\
'pageMargins':{'bottom':10,'footer':5,'header':5,'left':5,'right':5,'top':10},\
'pageSetup':{'orientation':1,'width':210,'height':297,'paperUnits':0,'scale':100,'printArea':false,'horizontalDpi':600,'verticalDpi':600,'usePrinterDefaults':true,'fitToHeight':0,'fitToWidth':0}\
}\
}\
},\
'documentLayout':{'drawPlaceHolders':true,'drawFormHighlight':true,'isPrint':true}\
}";

	NSStringUtils::string_replace(json_params, L"'", L"&quot;");

	oBuilder.WriteString(L"<m_sJsonParams>");
	oBuilder.WriteEncodeXmlString(json_params);
	oBuilder.WriteString(L"</m_sJsonParams>");

	// disable cache
	oBuilder.WriteString(L"<m_nDoctParams>1</m_nDoctParams>");

	oBuilder.WriteString(L"<m_oThumbnail><first>true</first><aspect>16</aspect><width>");
	oBuilder.WriteString(std::to_wstring(w));
	oBuilder.WriteString(L"</width><height>");
	oBuilder.WriteString(std::to_wstring(h));
	oBuilder.WriteString(L"</height></m_oThumbnail>");

	oBuilder.WriteString(L"</TaskQueueDataConvert>");

	std::wstring sTempFileForParams = sTempDir + L"/params.xml";
	NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

	int nReturnCode = NSX2T::Convert(pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, pManager);
	NSFile::CFileBinary::Remove(sTempFileForParams);

	return (0 == nReturnCode) ? true : false;
}

#endif // ASC_CEFCONVERTER_FILECONVERTER_H
