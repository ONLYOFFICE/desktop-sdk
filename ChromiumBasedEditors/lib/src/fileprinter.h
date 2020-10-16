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

#ifndef ASC_CEFCONVERTER_FILEPRINTER_H
#define ASC_CEFCONVERTER_FILEPRINTER_H

#include "./applicationmanager_p.h"
#include "./additional/manager.h"

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

public:

    virtual ~IAscNativePrintDocument() {}

    virtual void Draw(IRenderer* pRenderer, int nPageIndex) = 0;
    virtual void PreOpen(int nFileType) = 0;
    virtual void Open(const std::wstring& sPath, const std::wstring& sRecoveryDir) = 0;
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
    CPrintData();
    ~CPrintData();

    void Print_Start(NSFonts::IApplicationFonts* pFonts);
    void Print_End();

    void Print(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settings, const int& nPageIndex);
    bool CheckPrintRotate(NSEditorApi::CAscPrinterContextBase* pContext, const CAscPrintSettings& settings, const int& nPageIndex);
    void DrawOnRenderer(NSGraphics::IGraphicsRenderer* pRenderer, int nPageIndex);
    void TestSaveToRasterFile(std::wstring sFile, int nWidth, int nHeight, int nPageIndex);
    std::wstring GetImagePath(const std::wstring& sPath);
    void FitToPage(float fSourceWidth, float  fSourceHeight, float  fTargetWidth, float fTargetHeight, float& fResX, float& fResY, float& fResWidth, float& fResHeight);
    std::wstring DownloadImage(const std::wstring& strFile);
    void CalculateImagePaths(bool bIsOpenAsLocal = false);
};

#endif // ASC_CEFCONVERTER_FILEPRINTER_H
