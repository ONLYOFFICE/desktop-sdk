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

#include "./fileprinter.h"
#include "../../../../core/DesktopEditor/graphics/MetafileToRenderer.h"
#include "../../../../core/DesktopEditor/graphics/MetafileToRendererCheck.h"
#include "../../../../core/PdfFile/PdfFile.h"
#include "../../../../core/DesktopEditor/graphics/ImageFilesCache.h"
#include "../../../../core/DesktopEditor/graphics/pro/Graphics.h"

CPrintData::CPrintData()
{
	m_pApplicationFonts = NULL;
	m_pFontManager = NULL;
	m_pCache = NULL;

	m_nCurrentPage = -1;
	m_eEditorType = AscEditorType::etDocument;

	m_pNativePrinter = NULL;

	m_pAdditional = NULL;

	m_bIsOpenAsLocal = false;
}
CPrintData::~CPrintData()
{
	RELEASEOBJECT(m_pNativePrinter);

	RELEASEINTERFACE(m_pFontManager);
	RELEASEINTERFACE(m_pCache);
}

void CPrintData::Print_Start(NSFonts::IApplicationFonts* pFonts)
{
	if (NULL == m_pApplicationFonts)
		m_pApplicationFonts = pFonts;

	m_pFontManager = m_pApplicationFonts->GenerateFontManager();
	NSFonts::IFontsCache* pFontsCache = NSFonts::NSFontCache::Create();
	pFontsCache->SetStreams(m_pApplicationFonts->GetStreams());
	m_pFontManager->SetOwnerCache(pFontsCache);

	m_pCache = NSImages::NSFilesCache::Create(m_pApplicationFonts);

	m_nCurrentPage = -1;

	if (m_pAdditional)
		m_pAdditional->Print_Start();
}

void CPrintData::Print_End()
{
	if (m_pAdditional)
		m_pAdditional->Print_End();

	RELEASEOBJECT(m_pNativePrinter);
	RELEASEINTERFACE(m_pFontManager);
	RELEASEINTERFACE(m_pCache);

	m_pApplicationFonts->GetStreams()->Clear();

	for (std::map<std::wstring, std::wstring>::iterator iter = m_mapImagesDelete.begin(); iter != m_mapImagesDelete.end(); iter++)
	{
		NSFile::CFileBinary::Remove(iter->second);
	}
	m_mapImages.clear();
	m_mapImagesDelete.clear();

	m_nCurrentPage = -1;
	m_arPages.clear();

	m_eEditorType = AscEditorType::etDocument;
}

std::wstring CPrintData::DownloadImage(const std::wstring& strFile)
{
	CFileDownloaderWrapper oDownloader(strFile, L"");
	oDownloader.DownloadSync();

	std::wstring strFileName;
	if ( oDownloader.IsFileDownloaded() )
	{
		strFileName = oDownloader.GetFilePath();
	}
	else
	{
		strFileName = oDownloader.GetFilePath();
		NSFile::CFileBinary::Remove(strFileName);
		strFileName = L"";
	}
	return strFileName;
}

void CPrintData::CalculateImagePaths(bool bIsOpenAsLocal)
{
	m_bIsOpenAsLocal = bIsOpenAsLocal;
	m_sDocumentImagesPath = L"";
	if (!bIsOpenAsLocal && NSFileDownloader::IsNeedDownload(m_sFrameUrl) && !NSFileDownloader::IsNeedDownload(m_sDocumentUrl))
	{
		if (0 == m_sDocumentUrl.find(wchar_t('/')))
		{
			// take the site root
			int nPos = m_sFrameUrl.find(L"//");
			if (nPos != std::wstring::npos)
			{
				nPos = m_sFrameUrl.find(wchar_t('/'), nPos + 3);
				if (nPos != std::wstring::npos)
				{
					m_sDocumentImagesPath = m_sFrameUrl.substr(0, nPos);
					m_sDocumentImagesPath += m_sDocumentUrl;
				}
			}
			if (m_sDocumentImagesPath.empty())
			{
				m_sDocumentImagesPath = m_sFrameUrl;
				m_sDocumentImagesPath += (L"/" + m_sDocumentUrl);
			}
		}
		else
		{
			// take the folder of the url
			int nPos = m_sFrameUrl.find_last_of(wchar_t('/'));
			if (std::wstring::npos != nPos)
			{
				m_sDocumentImagesPath = m_sFrameUrl.substr(0, nPos + 1);
			}
			else
			{
				m_sDocumentImagesPath = m_sFrameUrl;
			}
			m_sDocumentImagesPath += (L"/" + m_sDocumentUrl);
		}
	}
	else
	{
		m_sDocumentImagesPath = m_sDocumentUrl;
	}

	m_sPresentationThemesPath = L"";
	if ((NSFileDownloader::IsNeedDownload(m_sFrameUrl) || (m_sFrameUrl.find(L"file://") == 0))
			&& !NSFileDownloader::IsNeedDownload(m_sThemesUrl))
	{
		if (0 == m_sThemesUrl.find(wchar_t('/')))
		{
			// take the site root
			int nPos = m_sFrameUrl.find(L"//");
			if (nPos != std::wstring::npos)
			{
				nPos = m_sFrameUrl.find(wchar_t('/'), nPos + 3);
				if (nPos != std::wstring::npos)
				{
					m_sPresentationThemesPath = m_sFrameUrl.substr(0, nPos);
					//m_sPresentationThemesPath += m_sThemesUrl;
				}
			}
			if (m_sPresentationThemesPath.empty())
			{
				m_sPresentationThemesPath = m_sFrameUrl;
				//m_sPresentationThemesPath += (L"/" + m_sThemesUrl);
			}
		}
		else
		{
			// take the folder of the url
			int nPos = m_sFrameUrl.find(L"/index.html");
			if (std::wstring::npos != nPos)
			{
				m_sPresentationThemesPath = m_sFrameUrl.substr(0, nPos + 1);
			}
			else
			{
				m_sPresentationThemesPath = m_sFrameUrl;
			}
			//m_sPresentationThemesPath += (L"/" + m_sThemesUrl);
		}
	}
	else
	{
		m_sPresentationThemesPath = m_sThemesUrl;
	}
}

class CMetafileToRenderterDesktop : public IMetafileToRenderter
{
public:
	CPrintData* m_pPrintData;

public:
	CMetafileToRenderterDesktop(IRenderer* pRenderer) : IMetafileToRenderter(pRenderer)
	{
		m_pPrintData = NULL;
	}

public:
	virtual std::wstring GetImagePath(const std::wstring& sImagePath)
	{
		return m_pPrintData->GetImagePath(sImagePath);
	}
};

std::wstring CPrintData::GetImagePath(const std::wstring& sPath)
{
	// 1) look in the map
	std::map<std::wstring, std::wstring>::iterator iFind = m_mapImages.find(sPath);
	if (iFind != m_mapImages.end())
		return iFind->second;

	// 2) not in the map. check - maybe the path is already correct
	if (NSFile::CFileBinary::Exists(sPath))
	{
		m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
		return sPath;
	}

	if (sPath.find(L"file://") == 0)
	{
		std::wstring s1 = sPath.substr(7);
		std::wstring s2 = sPath.substr(8);
		if (NSFile::CFileBinary::Exists(s1))
		{
			m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s1));
			return s1;
		}
		if (NSFile::CFileBinary::Exists(s2))
		{
			m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s2));
			return s2;
		}
	}

	// 3) check, maybe it is a direct link
	if (NSFileDownloader::IsNeedDownload(sPath))
	{
		std::wstring sFileDownload = this->DownloadImage(sPath);
		m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sFileDownload));
		m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, sFileDownload));
		return sFileDownload;
	}

	// 4) maybe it is a media file of the document?
	if (0 == sPath.find(L"media/image") || 0 == sPath.find(L"image") ||
		0 == sPath.find(L"image/display") || 0 == sPath.find(L"display") ||
		((0 == sPath.find(L"media/") && m_bIsOpenAsLocal)))
	{
		std::wstring sExt = L"";
		int nPos = sPath.find_last_of(wchar_t('.'));
		if (std::wstring::npos != nPos)
		{
			sExt = sPath.substr(nPos + 1);
		}

		std::wstring sPath2 = sPath;
		if (0 == sPath.find(L"image") || 0 == sPath.find(L"display"))
		{
			nPos += 6;
			sPath2 = L"media/" + sPath;
		}

		std::wstring sUrl = m_sDocumentImagesPath + sPath2;

		std::wstring sMetafileUrlBase = L"";
		if (sExt == L"svg")
			sMetafileUrlBase = m_sDocumentImagesPath + sPath2.substr(0, nPos);

		if (NSFileDownloader::IsNeedDownload(m_sDocumentImagesPath))
		{
			std::wstring sResultImagePath = L"";

			if (!sMetafileUrlBase.empty())
			{
				sResultImagePath = this->DownloadImage(sMetafileUrlBase + L".emf");
				if (sMetafileUrlBase.empty())
					sResultImagePath = this->DownloadImage(sMetafileUrlBase + L".wmf");
			}

			if (sResultImagePath.empty())
				sResultImagePath = this->DownloadImage(sUrl);

			if (!sResultImagePath.empty())
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				return sResultImagePath;
			}

			m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
			return sPath;
		}
		else
		{
			if (!sUrl.empty() && 0 == sUrl.find(L"file://"))
			{
				if (NSFile::CFileBinary::Exists(sUrl.substr(7)))
				{
					sUrl = sUrl.substr(7);
					if (!sMetafileUrlBase.empty())
						sMetafileUrlBase = sMetafileUrlBase.substr(7);
				}
				else if (NSFile::CFileBinary::Exists(sUrl.substr(8)))
				{
					sUrl = sUrl.substr(8);
					if (!sMetafileUrlBase.empty())
						sMetafileUrlBase = sMetafileUrlBase.substr(8);
				}
			}

			std::wstring sResultImagePath = L"";
			if (!sMetafileUrlBase.empty())
			{
				if (NSFile::CFileBinary::Exists(sMetafileUrlBase + L".emf"))
					sResultImagePath = sMetafileUrlBase + L".emf";
				else if (NSFile::CFileBinary::Exists(sMetafileUrlBase + L".wmf"))
					sResultImagePath = sMetafileUrlBase + L".wmf";
			}

			if (sResultImagePath.empty())
				sResultImagePath = sUrl;

			if (NSFile::CFileBinary::Exists(sResultImagePath))
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				return sResultImagePath;
			}
			else
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
				return sPath;
			}
		}
	}

	// 5) maybe it is a theme file?
	bool bIsThemesUrl1 = (0 == sPath.find(m_sThemesUrl)) ? true : false;
	bool bIsThemesUrl2 = (0 == sPath.find(L"theme")) ? true : false;
	if (!m_sThemesUrl.empty() && (bIsThemesUrl1 || bIsThemesUrl2))
	{
		std::wstring sExt = L"";
		int nPos = sPath.find_last_of(wchar_t('.'));
		if (std::wstring::npos != nPos)
		{
			sExt = sPath.substr(nPos + 1);
		}

		std::wstring sPresentationThemesPath = m_sPresentationThemesPath;
		if (bIsThemesUrl2)
			sPresentationThemesPath += m_sThemesUrl;

		std::wstring sUrl = sPresentationThemesPath + sPath;

		std::wstring sMetafileUrlBase = L"";
		if (sExt == L"svg")
			sMetafileUrlBase = sPresentationThemesPath + sPath.substr(0, nPos);

		if (NSFileDownloader::IsNeedDownload(m_sPresentationThemesPath))
		{
			std::wstring sResultImagePath = L"";

			if (!sMetafileUrlBase.empty())
			{
				sResultImagePath = this->DownloadImage(sMetafileUrlBase + L".emf");
				if (sMetafileUrlBase.empty())
					sResultImagePath = this->DownloadImage(sMetafileUrlBase + L".wmf");
			}

			if (sResultImagePath.empty())
				sResultImagePath = this->DownloadImage(sUrl);

			if (!sResultImagePath.empty())
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				return sResultImagePath;
			}

			m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
			return sPath;
		}
		else
		{
			if (!sUrl.empty() && 0 == sUrl.find(L"file://"))
			{
				if (NSFile::CFileBinary::Exists(sUrl.substr(7)))
				{
					sUrl = sUrl.substr(7);
					if (!sMetafileUrlBase.empty())
						sMetafileUrlBase = sMetafileUrlBase.substr(7);
				}
				else if (NSFile::CFileBinary::Exists(sUrl.substr(8)))
				{
					sUrl = sUrl.substr(8);
					if (!sMetafileUrlBase.empty())
						sMetafileUrlBase = sMetafileUrlBase.substr(8);
				}
			}

			std::wstring sResultImagePath = L"";
			if (!sMetafileUrlBase.empty())
			{
				if (NSFile::CFileBinary::Exists(sMetafileUrlBase + L".emf"))
					sResultImagePath = sMetafileUrlBase + L".emf";
				else if (NSFile::CFileBinary::Exists(sMetafileUrlBase + L".wmf"))
					sResultImagePath = sMetafileUrlBase + L".wmf";
			}

			if (sResultImagePath.empty())
				sResultImagePath = sUrl;

			if (NSFile::CFileBinary::Exists(sResultImagePath))
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sResultImagePath));
				return sResultImagePath;
			}
			else
			{
				m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
				return sPath;
			}
		}
	}

	// 6) base64?
	if (0 == sPath.find(L"data:"))
	{
		int nPos = sPath.find(wchar_t(','));

		if (nPos != std::wstring::npos)
		{
			int nLenBase64 = sPath.length() - nPos - 1;
			const wchar_t* pSrc = sPath.c_str() + nPos + 1;

			char* pData = new char[nLenBase64];
			for (int i = 0; i < nLenBase64; ++i)
				pData[i] = (char)(pSrc[i]);

			int nLenDecode = 0;
			BYTE* pDstData = NULL;

			bool bRes = NSFile::CBase64Converter::Decode(pData, nLenBase64, pDstData, nLenDecode);

			std::wstring sTmpFile = sPath;
			if (bRes)
			{
				sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"Image64");
				NSFile::CFileBinary oFile;
				if (oFile.CreateFileW(sTmpFile))
				{
					oFile.WriteFile(pDstData, nLenDecode);
					oFile.CloseFile();
				}
			}

			RELEASEARRAYOBJECTS(pData);
			RELEASEARRAYOBJECTS(pDstData);

			m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sTmpFile));
			m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, sTmpFile));
			return sTmpFile;
		}
	}

	// failure
	m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
	return sPath;
}

void CPrintData::FitToPage(float fSourceWidth, float fSourceHeight, float fTargetWidth, float fTargetHeight, float& fResX, float& fResY, float& fResWidth, float& fResHeight)
{
	if (fSourceWidth * fTargetHeight > fTargetWidth * fSourceHeight)
	{
		fResHeight = fTargetWidth * fSourceHeight / fSourceWidth;
		fResWidth = fTargetWidth;

		fResX = 0;
		fResY = fTargetHeight / 2 - fResHeight / 2;
	}
	else
	{
		fResWidth = fTargetHeight * fSourceWidth / fSourceHeight;
		fResHeight = fTargetHeight;
		fResY = 0;
		fResX = fTargetWidth / 2 - fResWidth / 2;
	}
}

CPrintData::CPrintContextPageData CPrintData::CheckPrintRotate(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settingsConst, const int& nPageIndex)
{
	CPrintContextPageData oData;

	if (nPageIndex < 0 || nPageIndex >= (int)m_arPages.size())
		return oData;

	CAscPrintSettings settings = settingsConst;

	if (m_eEditorType == AscEditorType::etPresentation)
	{
		settings.Mode = CAscPrintSettings::pmFit;
		settings.ZoomEnable = true;
	}

	double dLeftPix;
	double dTopPix;
	double dWidthPix;
	double dHeightPix;
	double dAngle = 0;
	double fPrintWidthMM;
	double fPrintHeightMM;

	double fPageWidth = m_arPages[nPageIndex].Width;
	double fPageHeight = m_arPages[nPageIndex].Height;

	double tmp_ONE_INCH = 2.54;
	double tmp_M_PI_2 = agg::pi / 2;

	int nPrintDpiX;
	int nPrintDpiY;
	int nPrintOffsetX;
	int nPrintOffsetY;
	int nPrintWidthPix; // of the whole page
	int nPrintHeightPix;
	int nPrintPageWidthPix; // of the print area only
	int nPrintPageHeightPix;

	pContext->GetLogicalDPI(nPrintDpiX, nPrintDpiY);
	pContext->GetPhysicalRect(nPrintOffsetX, nPrintOffsetY, nPrintWidthPix, nPrintHeightPix);
	pContext->GetPrintAreaSize(nPrintPageWidthPix, nPrintPageHeightPix);

	if( -1 != settings.WidthPix && -1 != settings.HeightPix )
	{
		nPrintWidthPix      = settings.WidthPix;
		nPrintHeightPix     = settings.HeightPix;
		nPrintPageWidthPix  = settings.WidthPix;
		nPrintPageHeightPix = settings.HeightPix;
	}

	if (settings.PrintableArea)
	{
		// we must print inside the print area only
		// so make the page size equal to the print area size
		nPrintWidthPix  = nPrintPageWidthPix;
		nPrintHeightPix = nPrintPageHeightPix;
		// and drop the offsets for the non-printable area
		nPrintOffsetX = 0;
		nPrintOffsetY = 0;
	}

	// calculate the page size in millimeters
	fPrintWidthMM   = 10 * tmp_ONE_INCH * nPrintWidthPix / nPrintDpiX;
	fPrintHeightMM  = 10 * tmp_ONE_INCH * nPrintHeightPix / nPrintDpiX;

	if (CAscPrintSettings::pm100  == settings.Mode)
	{
		dWidthPix   = nPrintDpiX * fPageWidth / ( 10 * tmp_ONE_INCH );
		dHeightPix  = nPrintDpiX * fPageHeight / ( 10 * tmp_ONE_INCH );
		if (true == settings.RotateEnable && ( nPrintWidthPix < dWidthPix || nPrintHeightPix < dHeightPix))
		{
			if (nPrintWidthPix < dHeightPix || nPrintHeightPix < dWidthPix)
			{
				// pick the variant with the bigger area
				double dWidth1  = nPrintWidthPix < dWidthPix ? nPrintWidthPix : dWidthPix;
				double dHeight1 = nPrintHeightPix < dHeightPix ? nPrintHeightPix : dHeightPix;

				double dWidth2  = nPrintWidthPix < dHeightPix ? nPrintWidthPix : dHeightPix;
				double dHeight2 = nPrintHeightPix < dWidthPix ? nPrintHeightPix : dWidthPix;

				if (dWidth1 * dHeight1 >= dWidth2 * dHeight2)
				{
					dLeftPix = 0;
					dTopPix = 0;
				}
				else
				{
					dLeftPix = nPrintWidthPix - ( dHeightPix + dWidthPix ) / 2;
					dTopPix = dWidthPix / 2 - dHeightPix / 2;
					dAngle = tmp_M_PI_2;    // 90
				}
			}
			else
			{
				//does not fit as is, but fits when rotated
				dLeftPix    = nPrintWidthPix - (dHeightPix + dWidthPix ) / 2;
				dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
				dAngle      = tmp_M_PI_2;   //90
			}
		}
		else
		{
			if (dWidthPix < nPrintWidthPix) //if the size allows, place it in the center
				dLeftPix = nPrintWidthPix / 2 - dWidthPix / 2;
			else
				dLeftPix = 0;
			dTopPix = 0;
		}
	}
	else if (CAscPrintSettings::pmStretch  == settings.Mode)
	{
		if (settings.RotateEnable && (fPageWidth / fPageHeight - 1) * (fPrintWidthMM / fPrintHeightMM - 1) < 0)
		{
			// turn it sideways
			dWidthPix   = nPrintHeightPix;
			dHeightPix  = nPrintWidthPix;
			dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2;
			dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
			dAngle      = tmp_M_PI_2;   // 90
		}
		else
		{
			dWidthPix = nPrintWidthPix;
			dHeightPix = nPrintHeightPix;
			dLeftPix = 0;
			dTopPix = 0;
		}
	}
	else
	{
		if (settings.ZoomEnable && settings.RotateEnable)
		{
			bool bRotate = false;
			if ((fPageWidth / fPageHeight - 1) * ( fPrintWidthMM / fPrintHeightMM - 1) < 0)
			{
				// turn it sideways
				double dTemp    = fPrintWidthMM;
				fPrintWidthMM   = fPrintHeightMM;
				fPrintHeightMM  = dTemp;
				dAngle          = tmp_M_PI_2;   // 90
				bRotate         = true;
			}
			float fFitX = 0;
			float fFitY = 0;
			float fFitWidth = 0;
			float fFitHeight = 0;
			FitToPage(fPageWidth, fPageHeight, fPrintWidthMM, fPrintHeightMM, fFitX, fFitY, fFitWidth, fFitHeight);

			dWidthPix = nPrintDpiX * fFitWidth / (10 * tmp_ONE_INCH);
			dHeightPix = nPrintDpiY * fFitHeight / (10 * tmp_ONE_INCH);
			if (true == bRotate)
			{
				dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2;
				dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
			}
			else
			{
				dLeftPix    = nPrintDpiX * fFitX / (10 * tmp_ONE_INCH);
				dTopPix     = nPrintDpiY * fFitY / (10 * tmp_ONE_INCH);
			}
		}
		else if (settings.ZoomEnable)
		{
			float fFitX = 0;
			float fFitY = 0;
			float fFitWidth = 0;
			float fFitHeight = 0;
			FitToPage(fPageWidth, fPageHeight, fPrintWidthMM, fPrintHeightMM, fFitX, fFitY, fFitWidth, fFitHeight);
			dWidthPix   = nPrintDpiX * fFitWidth / (10 * tmp_ONE_INCH);
			dHeightPix  = nPrintDpiY * fFitHeight / (10 * tmp_ONE_INCH);
			dLeftPix    = nPrintDpiX * fFitX / (10 * tmp_ONE_INCH);
			dTopPix     = nPrintDpiY * fFitY / (10 * tmp_ONE_INCH);
			dAngle      = 0;
		}
		else if (settings.RotateEnable)
		{
			// check whether the picture overflows the bounds
			if (fPrintWidthMM < fPageWidth || fPrintHeightMM < fPageHeight)
			{
				// check whether the rotated picture overflows the bounds
				if (fPrintHeightMM < fPageWidth || fPrintWidthMM < fPageHeight)
				{
					// pick whichever has the bigger area - rotated or not
					float fFitX1 = 0;
					float fFitY1 = 0;
					float fFitWidth1 = 0;
					float fFitHeight1 = 0;
					FitToPage( fPageWidth, fPageHeight, fPrintWidthMM, fPrintHeightMM, fFitX1, fFitY1, fFitWidth1, fFitHeight1 );

					float fFitX2 = 0;
					float fFitY2 = 0;
					float fFitWidth2 = 0;
					float fFitHeight2 = 0;
					FitToPage( fPageWidth, fPageHeight, fPrintHeightMM, fPrintWidthMM, fFitX2, fFitY2, fFitWidth2, fFitHeight2 );
					if (fFitWidth1 * fFitHeight1 < fFitWidth2 * fFitHeight2)
					{
						// rotate it
						dAngle      = tmp_M_PI_2;   // 90
						dWidthPix   = nPrintDpiX * fFitWidth2 / (10 * tmp_ONE_INCH);
						dHeightPix  = nPrintDpiY * fFitHeight2 / (10 * tmp_ONE_INCH);
						dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2;
						dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
					}
					else
					{
						dAngle      = 0;
						dWidthPix   = nPrintDpiX * fFitWidth1 / (10 * tmp_ONE_INCH);
						dHeightPix  = nPrintDpiY * fFitHeight1 / (10 * tmp_ONE_INCH);
						dLeftPix    = nPrintDpiX * fFitX1 / (10 * tmp_ONE_INCH);
						dTopPix     = nPrintDpiY * fFitY1 / (10 * tmp_ONE_INCH);
					}
				}
				else
				{
					// rotate it
					dWidthPix   = nPrintDpiX * fPageWidth / (10 * tmp_ONE_INCH);
					dHeightPix  = nPrintDpiY * fPageHeight / (10 * tmp_ONE_INCH);
					dLeftPix    = nPrintWidthPix - (dHeightPix + dWidthPix) / 2;
					dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
					dAngle      = tmp_M_PI_2;   // 90
				}
			}
			else
			{
				dWidthPix = nPrintDpiX * fPageWidth / ( 10 * tmp_ONE_INCH );
				dHeightPix = nPrintDpiY * fPageHeight / ( 10 * tmp_ONE_INCH );
				dLeftPix = nPrintWidthPix / 2 - dWidthPix / 2; // centered horizontally
				dTopPix = 0; // aligned to the top vertically
				dAngle = 0;
			}
		}
		else
		{
			// check whether the picture overflows the bounds
			if (fPrintWidthMM < fPageWidth || fPrintHeightMM < fPageHeight)
			{
				float fFitX = 0;
				float fFitY = 0;
				float fFitWidth = 0;
				float fFitHeight = 0;
				FitToPage(fPageWidth, fPageHeight, fPrintWidthMM, fPrintHeightMM, fFitX, fFitY, fFitWidth, fFitHeight);
				dWidthPix   = nPrintDpiX * fFitWidth / (10 * tmp_ONE_INCH);
				dHeightPix  = nPrintDpiY * fFitHeight / (10 * tmp_ONE_INCH);
				dLeftPix    = nPrintDpiX * fFitX / (10 * tmp_ONE_INCH);
				dTopPix     = nPrintDpiY * fFitY / (10 * tmp_ONE_INCH);
			}
			else
			{
				dWidthPix   = nPrintDpiX * fPageWidth / (10 * tmp_ONE_INCH);
				dHeightPix  = nPrintDpiY * fPageHeight / (10 * tmp_ONE_INCH);
				dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2; // centered horizontally
				dTopPix     = 0; // aligned to the top vertically
			}
		}
	}

	dLeftPix -= nPrintOffsetX;
	dTopPix -= nPrintOffsetY;

	oData.Angle = dAngle;

	oData.LeftPix = dLeftPix;
	oData.TopPix = dTopPix;
	oData.WidthPix = dWidthPix;
	oData.HeightPix = dHeightPix;

	oData.PrintWidthMM = fPrintWidthMM;
	oData.PrintHeightMM = fPrintHeightMM;

	oData.PageWidth = fPageWidth;
	oData.PageHeight = fPageHeight;

	oData.Valid = true;

	return oData;
}

void CPrintData::Print(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settingsConst, const int& nPageIndex)
{
	CPrintContextPageData oPagePrintData = CheckPrintRotate(pContext, settingsConst, nPageIndex);
	if (!oPagePrintData.Valid)
		return;

	int nRasterW = (int)(oPagePrintData.WidthPix + 0.5);
	int nRasterH = (int)(oPagePrintData.HeightPix + 0.5);

	double dTileScaleX = 1.0;
	double dTileScaleY = 1.0;

	if (oPagePrintData.PageWidth * oPagePrintData.PrintHeightMM > oPagePrintData.PrintWidthMM * oPagePrintData.PageHeight)
		dTileScaleX = dTileScaleY = oPagePrintData.PageWidth / oPagePrintData.PrintWidthMM;
	else
		dTileScaleX = dTileScaleY = oPagePrintData.PageHeight / oPagePrintData.PrintHeightMM;

#ifdef _XCODE
	// 16 bit align pixPerRow
	nRasterW += 8;
	nRasterW = (nRasterW - (nRasterW & 0x0F));

	nRasterH += 8;
	nRasterH = (nRasterH - (nRasterH & 0x0F));
#endif

	// decoded base64 commands
	BYTE* pPageCommands = NULL;
	int nPageCommandsLen = 0;

	if (NULL == m_pNativePrinter)
	{
		NSFile::CBase64Converter::Decode(m_arPages[nPageIndex].Base64.c_str(), m_arPages[nPageIndex].Base64.length(), pPageCommands, nPageCommandsLen);
	}

	IRenderer* pNativeRenderer = (IRenderer*)pContext->GetNativeRenderer();
#if 0
	// enable this if you want to test printing to raster
	RELEASEINTERFACE(pNativeRenderer);
#endif
	if (NULL != pNativeRenderer)
	{
		IMetafileToRenderter* pNativeRendererChecker = (IMetafileToRenderter*)pContext->GetNativeRendererUnsupportChecker();
		if (NULL != pNativeRendererChecker)
		{
			// check for support. once the renderer supports every command - GetNativeRendererUnsupportChecker must return NULL
			if (NULL == m_pNativePrinter)
				NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(pPageCommands, nPageCommandsLen, pNativeRendererChecker);
			else
				m_pNativePrinter->Draw(pNativeRendererChecker->m_pRenderer, nPageIndex);

			if (S_OK == pNativeRendererChecker->m_pRenderer->IsExistAdditionalParam(c_nAdditionalParamBreak))
			{
				// dropped throw/catch here: exceptions across dynamic libraries misbehave when libstd/libgcc are linked statically
				RELEASEINTERFACE(pNativeRenderer);
			}
		}
		RELEASEOBJECT(pNativeRendererChecker);
	}

	IRenderer* pDrawingRenderer = pNativeRenderer;
	CBgraFrame* pBgraFrame = NULL;

	// this is needed when pages alternate （NativeRenderer/Raster)
	m_pFontManager->SetTextMatrix(1, 0, 0, 1, 0, 0);

	bool bIsNeedRestore = false;
	if (NULL != pNativeRenderer)
	{
		bIsNeedRestore = true;
		pContext->SaveState();

		pContext->PrepareBitBlt(pNativeRenderer, 0, 0, nRasterW, nRasterH,
								oPagePrintData.LeftPix, oPagePrintData.TopPix, oPagePrintData.WidthPix, oPagePrintData.HeightPix, oPagePrintData.Angle, dTileScaleX, dTileScaleY);

		pContext->InitRenderer(pNativeRenderer, m_pFontManager);
	}
	else
	{
		pBgraFrame = new CBgraFrame();
		pBgraFrame->put_Width(nRasterW);
		pBgraFrame->put_Height(nRasterH);
		pBgraFrame->put_Stride(4 * nRasterW);

		BYTE* pDataRaster = new BYTE[4 * nRasterW * nRasterH];
		memset(pDataRaster, 0xFF, 4 * nRasterW * nRasterH);
		pBgraFrame->put_Data(pDataRaster);

		NSGraphics::IGraphicsRenderer* pGraphicsRenderer = NSGraphics::Create();
		pGraphicsRenderer = NSGraphics::Create();
		pGraphicsRenderer->SetFontManager(m_pFontManager);
		pGraphicsRenderer->SetImageCache(m_pCache);

		pGraphicsRenderer->CreateFromBgraFrame(pBgraFrame);
#ifndef _XCODE
		pGraphicsRenderer->SetSwapRGB(false);
#else
		pGraphicsRenderer->SetSwapRGB(true);
#endif

		if (!m_pNativePrinter)
			pGraphicsRenderer->SetTileImageDpi(96.0);

		pDrawingRenderer = pGraphicsRenderer;
	}

	if (NULL == m_pNativePrinter)
	{
		CMetafileToRenderterDesktop oCorrector(pDrawingRenderer);
		oCorrector.m_pPrintData = this;

		NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(pPageCommands, nPageCommandsLen, &oCorrector);
	}
	else
	{
		pDrawingRenderer->put_Width(oPagePrintData.PageWidth);
		pDrawingRenderer->put_Height(oPagePrintData.PageHeight);
		m_pNativePrinter->Draw(pDrawingRenderer, nPageIndex);
	}

	if (m_pAdditional)
		m_pAdditional->Check_Print(pDrawingRenderer, m_pFontManager, nRasterW, nRasterH, oPagePrintData.PageWidth, oPagePrintData.PageHeight);

	RELEASEINTERFACE(pDrawingRenderer);
	RELEASEARRAYOBJECTS(pPageCommands);

	if (pBgraFrame)
	{
#if 0
		pBgraFrame->SaveFile(L"FILE", 4);
#endif

		pContext->BitBlt(pBgraFrame->get_Data(), 0, 0, nRasterW, nRasterH,
						 oPagePrintData.LeftPix, oPagePrintData.TopPix, oPagePrintData.WidthPix, oPagePrintData.HeightPix, oPagePrintData.Angle);

#ifdef _XCODE
		pBgraFrame->put_Data(NULL);
#endif

		RELEASEOBJECT(pBgraFrame);
	}

	if (bIsNeedRestore)
		pContext->RestoreState();
}

// Cloud save to drawing formats
class CMetafileToRenderterPDF : public CMetafileToRenderterDesktop
{
public:
	CMetafileToRenderterPDF(IRenderer* pRenderer) : CMetafileToRenderterDesktop(pRenderer)
	{
	}

public:
	virtual void SetLinearGradiant(const double& x0, const double& y0, const double& x1, const double& y1)
	{
		((CPdfFile*)m_pRenderer)->SetLinearGradient(x0, y0, x1, y1);
	}

	virtual void SetRadialGradiant(const double& dX0, const double& dY0, const double& dR0, const double& dX1, const double& dY1, const double& dR1)
	{
		((CPdfFile*)m_pRenderer)->SetRadialGradient(dX0, dY0, dR0, dX1, dY1, dR1);
	}
};

CCloudPDFSaver::CCloudPDFSaver()
{
	m_pData = NULL;
	m_nDataLen = 0;
	m_pEvents = NULL;
}

CCloudPDFSaver::~CCloudPDFSaver()
{
	Stop();

	RELEASEARRAYOBJECTS(m_pData);
}

void CCloudPDFSaver::LoadData(const std::string& sBase64)
{
	NSFile::CBase64Converter::Decode(sBase64.c_str(), sBase64.length(), m_pData, m_nDataLen);
}

void CCloudPDFSaver::GetResultPdf(const std::wstring& sOutputFile, const std::wstring& sTempDir)
{
	CPdfFile oPdfResult(m_oPrintData.m_pApplicationFonts);
	oPdfResult.SetTempDirectory(sTempDir);

	oPdfResult.LoadFromFile(m_sPdfFileSrc, L"", m_sPdfFileSrcPassword.c_str(), m_sPdfFileSrcPassword.c_str());
	oPdfResult.EditPdf(sOutputFile);

	CConvertFromBinParams oConvertParams;
	oConvertParams.m_sInternalMediaDirectory = m_oPrintData.m_sDocumentImagesPath;
	oConvertParams.m_sMediaDirectory = oConvertParams.m_sInternalMediaDirectory;

	if (m_nDataLen > 4)
		oPdfResult.AddToPdfFromBinary(m_pData + 4, (unsigned int)(m_nDataLen - 4), &oConvertParams);
	oPdfResult.Close();

	// remove temporary src file
	NSFile::CFileBinary::Remove(m_sPdfFileSrc);
}

DWORD CCloudPDFSaver::ThreadProc()
{
	std::wstring sTempDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"PR_");
	if (NSFile::CFileBinary::Exists(sTempDir))
		NSFile::CFileBinary::Remove(sTempDir);
	NSDirectory::CreateDirectory(sTempDir);

	if (m_nOutputFormat == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF ||
		m_nOutputFormat == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA)
	{
		if (m_sPdfFileSrc.empty())
		{
			CPdfFile oPdfFile(m_oPrintData.m_pApplicationFonts);
			oPdfFile.SetTempDirectory(sTempDir);
			oPdfFile.CreatePdf((m_nOutputFormat == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA) ? true : false);

			CMetafileToRenderterPDF oCorrector(&oPdfFile);
			oCorrector.m_pPrintData = &m_oPrintData;

			NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(m_pData, m_nDataLen, &oCorrector);

			oPdfFile.SaveToFile(m_sOutputFileName);
		}
		else
		{
			GetResultPdf(m_sOutputFileName, sTempDir);
		}
	}
	else
	{
		NSDirectory::CreateDirectory(m_sOutputFileName);

		COfficeFileFormatChecker oChecker;
		std::wstring sExt = oChecker.GetExtensionByType(m_nOutputFormat);
		if (sExt.empty())
			sExt = L".png";

		if (m_sPdfFileSrc.empty())
		{
			NSOnlineOfficeBinToPdf::CMetafilePagesInfo oInfo;
			oInfo.CheckBuffer(m_pData, m_nDataLen);

			int nPagesCount = oInfo.PagesCount;
			if (0 != nPagesCount)
			{
				NSFonts::IFontManager* pFontManager = m_oPrintData.m_pApplicationFonts->GenerateFontManager();
				NSFonts::IFontsCache* pFontsCache = NSFonts::NSFontCache::Create();
				pFontsCache->SetStreams(m_oPrintData.m_pApplicationFonts->GetStreams());
				pFontManager->SetOwnerCache(pFontsCache);
				CImageFilesCache* pImagesCache = new CImageFilesCache(m_oPrintData.m_pApplicationFonts);

				for (int nPageIndex = 0; (nPageIndex < nPagesCount) && m_bRunThread; ++nPageIndex)
				{
					CBgraFrame oFrame;
					int nRasterW = (int)(96 * oInfo.arSizes[nPageIndex].width / 25.4);
					int nRasterH = (int)(96 * oInfo.arSizes[nPageIndex].height / 25.4);

					oFrame.put_Width(nRasterW);
					oFrame.put_Height(nRasterH);
					oFrame.put_Stride(4 * nRasterW);

					BYTE* pDataRaster = new BYTE[4 * nRasterW * nRasterH];
					memset(pDataRaster, 0xFF, 4 * nRasterW * nRasterH);
					oFrame.put_Data(pDataRaster);

					NSGraphics::IGraphicsRenderer* pRenderer = NSGraphics::Create();
					pRenderer->SetFontManager(pFontManager);
					pRenderer->SetImageCache(pImagesCache);

					pRenderer->CreateFromBgraFrame(&oFrame);
					pRenderer->SetTileImageDpi(96.0);
					pRenderer->SetSwapRGB(false);

					CMetafileToRenderterDesktop oCorrector(pRenderer);
					oCorrector.m_pPrintData = &m_oPrintData;

					BYTE* pBufferPage = oInfo.arSizes[nPageIndex].data;
					int nLen = m_nDataLen - ((int)(pBufferPage - m_pData));
					NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(pBufferPage, nLen, &oCorrector);

					RELEASEINTERFACE(pRenderer);

					int nImageFormat = 4; // PNG
					if (AVS_OFFICESTUDIO_FILE_IMAGE_JPG == m_nOutputFormat)
						nImageFormat = 3;

					oFrame.SaveFile(m_sOutputFileName + L"/image" + std::to_wstring(nPageIndex + 1) + sExt, nImageFormat);
				}

				RELEASEINTERFACE(pFontManager);
				RELEASEINTERFACE(pImagesCache);
			}
		}
		else
		{
			std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"PDFT");
			if (NSFile::CFileBinary::Exists(sTmpFile))
				NSFile::CFileBinary::Remove(sTmpFile);

			GetResultPdf(sTmpFile, sTempDir);

			CPdfFile oPdfResult(m_oPrintData.m_pApplicationFonts);
			oPdfResult.SetTempDirectory(sTempDir);

			if (oPdfResult.LoadFromFile(sTmpFile, L"", m_sPdfFileSrcPassword.c_str(), m_sPdfFileSrcPassword.c_str()))
			{
				int nPagesCount = oPdfResult.GetPagesCount();
				int nImageFormat = 4; // PNG
				if (AVS_OFFICESTUDIO_FILE_IMAGE_JPG == m_nOutputFormat)
					nImageFormat = 3;

				for (int nPageIndex = 0; (nPageIndex < nPagesCount) && m_bRunThread; ++nPageIndex)
				{
					std::wstring sResFile = m_sOutputFileName + L"/image" + std::to_wstring(nPageIndex + 1) + sExt;
					oPdfResult.ConvertToRaster(nPageIndex, sResFile, nImageFormat);
				}
			}

			oPdfResult.Close();
			NSFile::CFileBinary::Remove(sTmpFile);
		}
	}

	NSDirectory::DeleteDirectory(sTempDir);

	m_bRunThread = FALSE;
	m_pEvents->OnFileConvertFromEditor(0, L"");
	return 0;
}
