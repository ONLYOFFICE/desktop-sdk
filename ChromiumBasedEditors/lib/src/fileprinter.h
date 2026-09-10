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

#ifndef ASC_CEFCONVERTER_FILEPRINTER_H
#define ASC_CEFCONVERTER_FILEPRINTER_H

#include "./applicationmanager_p.h"

class CPagePrintData
{
public:
	std::string Base64;
	double      Width;
	double      Height;

	CPagePrintData()
	{
		Width   = 0;
		Height  = 0;
	}
};

class IAscNativePrintDocument
{
public:
	std::wstring            m_sFilePath;
	NSFonts::IApplicationFonts*  m_pApplicationFonts;
	std::wstring            m_sTempFolder;
	std::wstring            m_sCMapFolder;
	std::wstring            m_sImagesDirectory;

public:

	virtual ~IAscNativePrintDocument() {}

	virtual void Draw(IRenderer* pRenderer, int nPageIndex) = 0;
	virtual void PreOpen(int nFileType) = 0;
	virtual void Open(const std::wstring& sPath, const std::wstring& sRecoveryDir, const std::wstring& sChangesFile, const std::string& sPrintParams) = 0;
	virtual void Close() = 0;

	virtual void Check(std::vector<CPagePrintData>& arPages) = 0;
};

class CPrintData
{
public:
	std::vector<CPagePrintData> m_arPages;
	int m_nCurrentPage;
	std::wstring m_sFrameUrl;
	std::wstring m_sDocumentUrl;
	std::wstring m_sThemesUrl;

	std::map<std::wstring, std::wstring> m_mapImages;
	std::map<std::wstring, std::wstring> m_mapImagesDelete;

	std::wstring m_sDocumentImagesPath;
	std::wstring m_sPresentationThemesPath;

	NSFonts::IFontManager* m_pFontManager;
	NSImages::IImageFilesCache* m_pCache;

	NSFonts::IApplicationFonts* m_pApplicationFonts;
	AscEditorType m_eEditorType;

	IAscNativePrintDocument* m_pNativePrinter;

	std::wstring m_sTempWaterMark;
	std::wstring m_sRasterWatermark;
	int m_lRasterWatermarkW;
	int m_lRasterWatermarkH;

	bool m_bIsOpenAsLocal;

	CApplicationManagerAdditionalBase* m_pAdditional;

public:
	class CPrintContextPageData
	{
	public:
		double LeftPix;
		double TopPix;
		double WidthPix;
		double HeightPix;
		double Angle;
		double PrintWidthMM;
		double PrintHeightMM;

		double PageWidth;
		double PageHeight;

		bool Valid;

	public:
		CPrintContextPageData()
		{
			LeftPix = 0;
			TopPix = 0;
			WidthPix = 0;
			HeightPix = 0;
			Angle = 0;
			PrintWidthMM = 0;
			PrintHeightMM = 0;

			PageWidth = 0;
			PageHeight = 0;

			Valid = false;
		}

		inline bool IsRotate()
		{
			if (Angle < 0.01 && Angle > -0.01)
				return false;
			return true;
		}
	};

public:
	CPrintData();
	~CPrintData();

	void Print_Start(NSFonts::IApplicationFonts* pFonts);
	void Print_End();

	void FitToPage(float fSourceWidth, float fSourceHeight, float fTargetWidth, float fTargetHeight, float& fResX, float& fResY, float& fResWidth, float& fResHeight);
	CPrintContextPageData CheckPrintRotate(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settings, const int& nPageIndex);
	void Print(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settings, const int& nPageIndex);

	bool DrawOnRenderer(IRenderer* pRenderer, int nPageIndex, CPrintContextPageData* pPageData, IRenderer* pChecker = NULL);

	std::wstring GetImagePath(const std::wstring& sPath);
	std::wstring DownloadImage(const std::wstring& strFile);
	void CalculateImagePaths(bool bIsOpenAsLocal = false);
};

// saving to PDF/PDFA/images
class IASCFileConverterEvents;
class CCloudPDFSaver : public NSThreads::CBaseThread
{
public:
	std::wstring m_sOutputFileName;
	int m_nOutputFormat;

	CPrintData m_oPrintData;
	IASCFileConverterEvents* m_pEvents;

	// pdf command binary or changes in pdf
	BYTE* m_pData;
	int m_nDataLen;

	std::wstring m_sPdfFileSrc;
	std::wstring m_sPdfFileSrcPassword;

public:
	CCloudPDFSaver();
	~CCloudPDFSaver();

	void LoadData(const std::string& sBase64);
	void GetResultPdf(const std::wstring& sOutputFile, const std::wstring& sTempDir);

	virtual DWORD ThreadProc();
};

#endif // ASC_CEFCONVERTER_FILEPRINTER_H
