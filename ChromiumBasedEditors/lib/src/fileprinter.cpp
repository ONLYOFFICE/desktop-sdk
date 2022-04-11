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

#include "./fileprinter.h"

CPrintData::CPrintData()
{
    m_pApplicationFonts = NULL;
    m_pFontManager = NULL;
    m_pCache = NULL;

    m_nCurrentPage = -1;
    m_eEditorType = etDocument;

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

    m_eEditorType = etDocument;
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
            // нужно брать корень сайта
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
            // брать место урла
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
            // нужно брать корень сайта
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
            // брать место урла
            int nPos = m_sFrameUrl.find_last_of(wchar_t('/'));
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

void CPrintData::rotateRender(IRenderer *pRender, const double &fPrintWidthMM, const double &fPrintHeightMM, const double &dAngle)
{
    double dAngleDeg = dAngle * 180.0 / M_PI;
    if ((std::abs(dAngleDeg - 90) < 1.0) || (std::abs(dAngleDeg - 270) < 1.0))
    {
        double m11, m12, m21, m22, mdx, mdy;
        const double fPrintSideRatio = fPrintHeightMM / fPrintWidthMM;
        m11 = 0;
        m12 = fPrintSideRatio/2;
        m21 = -fPrintSideRatio;
        m22 = 0;
        mdx = fPrintHeightMM;
        mdy = 0;
        pRender->SetTransform(m11, m12, m21, m22, mdx, mdy);
    }
}


#include "../../../../core/DesktopEditor/graphics/MetafileToRenderer.h"
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


void CPrintData::DrawOnRenderer(IRenderer* pRenderer, int nPageIndex)
{
    CMetafileToRenderterDesktop oCorrector(pRenderer);
    oCorrector.m_pPrintData = this;

    // maybe we should use boost::shared_array ?
    BYTE* dstArray = NULL;
    int len = 0;
    NSFile::CBase64Converter::Decode(m_arPages[nPageIndex].Base64.c_str(), m_arPages[nPageIndex].Base64.length(), dstArray, len);

    NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(dstArray, len, &oCorrector);

    RELEASEARRAYOBJECTS(dstArray);
}

std::wstring CPrintData::GetImagePath(const std::wstring& sPath)
{
    // 1) смотрим в мапе
    std::map<std::wstring, std::wstring>::iterator iFind = m_mapImages.find(sPath);
    if (iFind != m_mapImages.end())
        return iFind->second;

    // 2) в мапе нет. смотрим - может путь правильный совсем
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

    // 3) смотрим, может это прямая ссылка
    if (NSFileDownloader::IsNeedDownload(sPath))
    {
        std::wstring sFileDownload = this->DownloadImage(sPath);
        m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sFileDownload));
        m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, sFileDownload));
        return sFileDownload;
    }

    // 4) может это файл файла?
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
        std::wstring sUrl2 = L"";
        if (sExt == L"svg")
        {
            sUrl = m_sDocumentImagesPath + sPath2.substr(0, nPos) + L".emf";
            sUrl2 = m_sDocumentImagesPath + sPath2.substr(0, nPos) + L".wmf";
        }

        if (NSFileDownloader::IsNeedDownload(m_sDocumentImagesPath))
        {
            std::wstring s1 = this->DownloadImage(sUrl);
            if (!s1.empty())
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s1));
                m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, s1));
                return s1;
            }
            if (!sUrl2.empty())
            {
                std::wstring s2 = this->DownloadImage(sUrl2);
                if (!s2.empty())
                {
                    m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s2));
                    m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, s2));
                    return s2;
                }
            }
            m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
            return sPath;
        }
        else
        {
            if (!sUrl.empty() && 0 == sUrl.find(L"file://"))
            {
                std::wstring s1 = sUrl.substr(7);
                std::wstring s2 = sUrl.substr(8);
                if (NSFile::CFileBinary::Exists(s1))
                {
                    sUrl = s1;
                }
                else if (NSFile::CFileBinary::Exists(s2))
                {
                    sUrl = s2;
                }
            }
            if (!sUrl2.empty() && 0 == sUrl2.find(L"file://"))
            {
                std::wstring s1 = sUrl2.substr(7);
                std::wstring s2 = sUrl2.substr(8);
                if (NSFile::CFileBinary::Exists(s1))
                {
                    sUrl2 = s1;
                }
                else if (NSFile::CFileBinary::Exists(s2))
                {
                    sUrl2 = s2;
                }
            }

            if (NSFile::CFileBinary::Exists(sUrl))
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sUrl));
                return sUrl;
            }
            else if (NSFile::CFileBinary::Exists(sUrl2))
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sUrl2));
                return sUrl2;
            }
            else
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
                return sPath;
            }
        }
    }

    // 5) может это файл темы?
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
        std::wstring sUrl2 = L"";
        if (sExt == L"svg")
        {
            sUrl = sPresentationThemesPath + sPath.substr(0, nPos) + L".emf";
            sUrl2 = sPresentationThemesPath + sPath.substr(0, nPos) + L".wmf";
        }

        if (NSFileDownloader::IsNeedDownload(m_sPresentationThemesPath))
        {
            std::wstring s1 = this->DownloadImage(sUrl);
            if (!s1.empty())
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s1));
                m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, s1));
                return s1;
            }
            if (!sUrl2.empty())
            {
                std::wstring s2 = this->DownloadImage(sUrl2);
                if (!s2.empty())
                {
                    m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, s2));
                    m_mapImagesDelete.insert(std::pair<std::wstring, std::wstring>(sPath, s2));
                    return s2;
                }
            }
            m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
            return sPath;
        }
        else
        {
            if (!sUrl.empty() && 0 == sUrl.find(L"file://"))
            {
                std::wstring s1 = sUrl.substr(7);
                std::wstring s2 = sUrl.substr(8);
                if (NSFile::CFileBinary::Exists(s1))
                {
                    sUrl = s1;
                }
                else if (NSFile::CFileBinary::Exists(s2))
                {
                    sUrl = s2;
                }
            }
            if (!sUrl2.empty() && 0 == sUrl2.find(L"file://"))
            {
                std::wstring s1 = sUrl2.substr(7);
                std::wstring s2 = sUrl2.substr(8);
                if (NSFile::CFileBinary::Exists(s1))
                {
                    sUrl2 = s1;
                }
                else if (NSFile::CFileBinary::Exists(s2))
                {
                    sUrl2 = s2;
                }
            }

            if (NSFile::CFileBinary::Exists(sUrl))
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sUrl));
                return sUrl;
            }
            else if (NSFile::CFileBinary::Exists(sUrl2))
            {
                m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sUrl2));
                return sUrl2;
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

    // ошибка
    m_mapImages.insert(std::pair<std::wstring, std::wstring>(sPath, sPath));
    return sPath;
}

bool CPrintData::CheckPrintRotate(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settingsConst, const int& nPageIndex)
{
    if (nPageIndex < 0 || nPageIndex >= (int)m_arPages.size())
        return false;

    CAscPrintSettings settings = settingsConst;

    if (m_eEditorType == etPresentation)
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
    int nPrintWidthPix; // всей страницы
    int nPrintHeightPix;
    int nPrintPageWidthPix; // только области печати
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
        // печатать нужно только в области печати
        // приравниваем высоту страницы к высоте области печати
        nPrintWidthPix  = nPrintPageWidthPix;
        nPrintHeightPix = nPrintPageHeightPix;
        // обнуляем поправки на непечатаемую область
        nPrintOffsetX = 0;
        nPrintOffsetY = 0;
    }

    // подсчитываем размеры страницы в милиметрах
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
                // выбираем лучший вариант по площади
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
                //если не вписывается, но вписывается повернутое
                dLeftPix    = nPrintWidthPix - (dHeightPix + dWidthPix ) / 2;
                dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
                dAngle      = tmp_M_PI_2;   //90
            }
        }
        else
        {
            if (dWidthPix < nPrintWidthPix) //если размеры позволяют, то распологаем по центру
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
            // переворачиваем
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
                // переворачиваем
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
            // проверяем выходит ли картинка за габариты
            if (fPrintWidthMM < fPageWidth || fPrintHeightMM < fPageHeight)
            {
                // проверяем выходит ли повернутая картинка за габариты
                if (fPrintHeightMM < fPageWidth || fPrintWidthMM < fPageHeight)
                {
                    // выбираем, где больше площадь у повернутого или нет
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
                        // поворачиваем
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
                    // поворачиваем
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
                dLeftPix = nPrintWidthPix / 2 - dWidthPix / 2; // по центру по горизонтали
                dTopPix = 0; // сверху по вертикали
                dAngle = 0;
            }
        }
        else
        {
            // проверяем выходит ли картинка за габариты
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
                dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2; // по центру по горизонтали
                dTopPix     = 0; // сверху по вертикали
            }
        }
    }

    if (dAngle < 0.01 && dAngle > -0.01)
        return false;
    return true;
}

void CPrintData::Print(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settingsConst, const int& nPageIndex)
{
    if (nPageIndex < 0 || nPageIndex >= (int)m_arPages.size())
        return;

    CAscPrintSettings settings = settingsConst;

    if (m_eEditorType == etPresentation)
    {
        settings.Mode = CAscPrintSettings::pmFit;
        settings.ZoomEnable = true;
    }

    double dLeftPix;
    double dTopPix;
    double dWidthPix;
    double dHeightPix;
    double dAngle;
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
    int nPrintWidthPix; // всей страницы
    int nPrintHeightPix;
    int nPrintPageWidthPix; // только области печати
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
        // печатать нужно только в области печати
        // приравниваем высоту страницы к высоте области печати
        nPrintWidthPix  = nPrintPageWidthPix;
        nPrintHeightPix = nPrintPageHeightPix;
        // обнуляем поправки на непечатаемую область
        nPrintOffsetX = 0;
        nPrintOffsetY = 0;
    }

    // подсчитываем размеры страницы в милиметрах
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
                // выбираем лучший вариант по площади
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
                //если не вписывается, но вписывается повернутое
                dLeftPix    = nPrintWidthPix - (dHeightPix + dWidthPix ) / 2;
                dTopPix     = nPrintHeightPix / 2 - dHeightPix / 2;
                dAngle      = tmp_M_PI_2;   //90
            }
        }
        else
        {
            if (dWidthPix < nPrintWidthPix) //если размеры позволяют, то распологаем по центру
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
            // переворачиваем
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
                // переворачиваем
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
            // проверяем выходит ли картинка за габариты
            if (fPrintWidthMM < fPageWidth || fPrintHeightMM < fPageHeight)
            {
                // проверяем выходит ли повернутая картинка за габариты
                if (fPrintHeightMM < fPageWidth || fPrintWidthMM < fPageHeight)
                {
                    // выбираем, где больше площадь у повернутого или нет
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
                        // поворачиваем
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
                    // поворачиваем
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
                dLeftPix = nPrintWidthPix / 2 - dWidthPix / 2; // по центру по горизонтали
                dTopPix = 0; // сверху по вертикали
                dAngle = 0;
            }
        }
        else
        {
            // проверяем выходит ли картинка за габариты
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
                dLeftPix    = nPrintWidthPix / 2 - dWidthPix / 2; // по центру по горизонтали
                dTopPix     = 0; // сверху по вертикали
            }
        }
    }
    dLeftPix -= nPrintOffsetX;
    dTopPix -= nPrintOffsetY;

    int nRasterW = (int)(dWidthPix + 0.5);
    int nRasterH = (int)(dHeightPix + 0.5);
#ifdef _XCODE
    // 16 bit align pixPerRow
    nRasterW += 8;
    nRasterW = (nRasterW - (nRasterW & 0x0F));

    nRasterH += 8;
    nRasterH = (nRasterH - (nRasterH & 0x0F));
#endif

    IRenderer* pNativeRenderer(nullptr);
    pNativeRenderer = (IRenderer*)pContext->GetNativeRenderer();
    if (NULL != pNativeRenderer && false)
    {
        pContext->InitRenderer(pNativeRenderer, m_pFontManager);

        // set base transform
        rotateRender(pNativeRenderer, fPrintWidthMM, fPrintHeightMM, dAngle);

        if (NULL == m_pNativePrinter)
        {
            this->DrawOnRenderer(pNativeRenderer, nPageIndex);
        }
        else
        {
            pNativeRenderer->put_Width(fPageWidth);
            pNativeRenderer->put_Height(fPageHeight);
            m_pNativePrinter->Draw(pNativeRenderer, nPageIndex);
        }

        if (pNativeRenderer->IsCorrectRendered())
        {
            if (m_pAdditional)
                m_pAdditional->Check_Print(pNativeRenderer, m_pFontManager, nRasterW, nRasterH, fPageWidth, fPageHeight);


            RELEASEINTERFACE(pNativeRenderer);
            return;
        }

    }

    CBgraFrame oFrame;
    oFrame.put_Width(nRasterW);
    oFrame.put_Height(nRasterH);
    oFrame.put_Stride(4 * nRasterW);

    BYTE* pDataRaster = new BYTE[4 * nRasterW * nRasterH];
    memset(pDataRaster, 0xFF, 4 * nRasterW * nRasterH);
    oFrame.put_Data(pDataRaster);

    NSGraphics::IGraphicsRenderer* pRenderer = NSGraphics::Create();
    pRenderer->SetFontManager(m_pFontManager);
    pRenderer->SetImageCache(m_pCache);

    pRenderer->CreateFromBgraFrame(&oFrame);
#ifndef _XCODE
    pRenderer->SetSwapRGB(false);
#else
    pRenderer->SetSwapRGB(true);
#endif

    pRenderer->SetTileImageDpi(96.0);

    if (NULL == m_pNativePrinter)
        this->DrawOnRenderer(pRenderer, nPageIndex);
    else
    {
        pRenderer->put_Width(fPageWidth);
        pRenderer->put_Height(fPageHeight);
        m_pNativePrinter->Draw(pRenderer, nPageIndex);
    }

    if (m_pAdditional)
        m_pAdditional->Check_Print(pRenderer, m_pFontManager, nRasterW, nRasterH, fPageWidth, fPageHeight);

    RELEASEINTERFACE(pRenderer);

#if 0
    oFrame.SaveFile(L"D:\\ttttt.png", 4);
#endif

    pContext->BitBlt(oFrame.get_Data(), 0, 0, nRasterW, nRasterH,
                     dLeftPix, dTopPix, dWidthPix, dHeightPix, dAngle);
    
#ifdef _XCODE
    oFrame.put_Data(NULL);
#endif
}

void CPrintData::FitToPage(float fSourceWidth, float  fSourceHeight, float  fTargetWidth, float fTargetHeight, float& fResX, float& fResY, float& fResWidth, float& fResHeight)
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

void CPrintData::TestSaveToRasterFile(std::wstring sFile, int nWidth, int nHeight, int nPageIndex)
{
    CBgraFrame oFrame;
    int nRasterW = nWidth;
    int nRasterH = nHeight;
    oFrame.put_Width(nRasterW);
    oFrame.put_Height(nRasterH);
    oFrame.put_Stride(-4 * nRasterW);

    BYTE* pDataRaster = new BYTE[4 * nRasterW * nRasterH];
    memset(pDataRaster, 0xFF, 4 * nRasterW * nRasterH);
    oFrame.put_Data(pDataRaster);

    NSGraphics::IGraphicsRenderer* pRenderer = NSGraphics::Create();
    pRenderer->SetFontManager(m_pFontManager);
    pRenderer->SetImageCache(m_pCache);

    pRenderer->CreateFromBgraFrame(&oFrame);
    pRenderer->SetSwapRGB(false);

    this->DrawOnRenderer(pRenderer, nPageIndex);

    RELEASEINTERFACE(pRenderer);

    oFrame.SaveFile(sFile, 4);
}
