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

#ifndef APPLICATION_NATIVEVIEWER_H
#define APPLICATION_NATIVEVIEWER_H

#include "../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "../../../../core/DesktopEditor/graphics/BaseThread.h"
#include "../../../../core/DesktopEditor/graphics/Timer.h"
#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/DesktopEditor/graphics/pro/Fonts.h"
#include "../../../../core/DesktopEditor/graphics/pro/Graphics.h"
#include "../../../../core/DesktopEditor/raster/BgraFrame.h"

#include "../../../../core/Common/OfficeFileFormatChecker.h"
#include "../../../../core/PdfReader/PdfReader.h"
#include "../../../../core/DjVuFile/DjVu.h"
#include "../../../../core/XpsFile/XpsFile.h"
#include "../../../../core/HtmlRenderer/include/HTMLRenderer3.h"

#include "Logger.h"

#include <list>
#include <math.h>

class INativeViewer_Events
{
public:
    virtual void OnDocumentOpened(const std::string& sBase64) = 0;
    virtual void OnPageSaved(const std::wstring& sUrl, const int& nPageNum, const int& nPageW, const int& nPageH) = 0;
    virtual void OnPageText(const int& page, const int& paragraphs, const int& words, const int& symbols, const int& spaces, const std::string& sBase64Data) = 0;
};

class CNativeViewerPageInfo
{
public:
    int Page;
    int W;
    int H;

public:
    CNativeViewerPageInfo()
    {
        Clear();
    }

    bool operator==(const CNativeViewerPageInfo& other) const {
      return ((Page == other.Page) && (W == other.W) && (H == other.H));
    }

    void Clear()
    {
        Page = -1;
        W = 0;
        H = 0;
    }
};

class CNativeViewer : public NSTimers::CTimer
{
private:
    int                     m_nFileType;

    IOfficeDrawingFile*     m_pReader;

    std::wstring            m_sFileDir;
    std::wstring            m_sFontsDir;

    std::wstring            m_sPassword;

    std::list<CNativeViewerPageInfo> m_arTasks;
    CNativeViewerPageInfo m_oCurrentTask;
    NSCriticalSection::CRITICAL_SECTION m_oCS;

    NSFonts::IApplicationFonts* m_pFonts;

    int                     m_lPagesCount;

    INativeViewer_Events*   m_pEvents;

    NSFonts::IFontManager*      m_pFontManager;
    NSImages::IImageFilesCache* m_pImageCache;

    std::wstring            m_sOpeningFilePath;
    std::wstring            m_sOpeningFilePathSrc;
    std::string             m_sBase64File;

    double*                 m_pPageWidths;
    double*                 m_pPageHeights;

    int m_lTextPageCount;
    NSHtmlRenderer::CASCHTMLRenderer3* m_pHtmlRenderer;

    int m_nIntervalTimerDraw;
    int m_nIntervalTimer;

public:
    CNativeViewer() : NSTimers::CTimer()
    {
        m_nFileType = 0;

        m_pReader = NULL;
        m_pEvents = NULL;

        m_pFontManager = NULL;
        m_pImageCache = NULL;

        m_oCS.InitializeCriticalSection();

        m_pPageWidths = NULL;
        m_pPageHeights = NULL;

        m_pHtmlRenderer = NULL;
        m_lTextPageCount = 0;

        m_nIntervalTimer = 1;
        m_nIntervalTimerDraw = 40;
        SetInterval(m_nIntervalTimer);

        m_pFonts = NSFonts::NSApplication::Create();
    }

    virtual ~CNativeViewer()
    {
        Stop();
        CloseFile();

        RELEASEINTERFACE(m_pFontManager);
        RELEASEINTERFACE(m_pImageCache);

        m_oCS.DeleteCriticalSection();

        RELEASEARRAYOBJECTS(m_pPageWidths);
        RELEASEARRAYOBJECTS(m_pPageHeights);

        NSBase::Release(m_pFonts);
    }

public:

    std::wstring GetPassword()
    {
        return m_sPassword;
    }

    void SetPassword(std::wstring sPassword)
    {
        CTemporaryCS oCS(&m_oCS);
        m_sPassword = sPassword;
        Start(0);
    }

    void ClearBase64()
    {
        CTemporaryCS oCS(&m_oCS);
        m_sBase64File = "";
        m_sOpeningFilePath = m_sOpeningFilePathSrc;
    }

    std::string GetBase64File()
    {
        CTemporaryCS oCS(&m_oCS);
        return m_sBase64File;
    }

    void SetEventListener(INativeViewer_Events* pEvents)
    {
        m_pEvents = pEvents;
    }

    void Init(const std::wstring& sFileDir,
                const std::wstring& sFontsDir,
                const std::wstring& sOpeningFilePath,
                INativeViewer_Events* pEvents)
    {
        m_sFileDir = sFileDir;
        m_sFontsDir = sFontsDir;
        m_sOpeningFilePath = sOpeningFilePath;
        m_sOpeningFilePathSrc = m_sOpeningFilePath;
        m_pEvents = pEvents;

        this->Start(0);
    }

public:

    void OpenFile(const std::wstring& sFilePath, int nFileType = 0)
    {
        m_nFileType = 0;
        if (0 == nFileType)
        {
            COfficeFileFormatChecker oChecker;
            if (oChecker.isOfficeFile(sFilePath))
                nFileType = oChecker.nFileType;
        }

        if (0 != nFileType && m_sPassword.empty())
            m_pFonts->InitializeFromFolder(m_sFontsDir);

        int nFileTypeOpen = 0;

        switch (nFileType)
        {
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF:
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA:
        {
            m_pReader = new PdfReader::CPdfReader(m_pFonts);
            nFileTypeOpen = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF;
            break;
        }
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS:
        {
            m_pReader = new CXpsFile(m_pFonts);
            nFileTypeOpen = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS;
            break;
        }
        case AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU:
        {
            m_pReader = new CDjVuFile(m_pFonts);
            nFileTypeOpen = AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU;
            break;
        }
        default:
            break;
        }

        NSDirectory::CreateDirectory(m_sFileDir + L"/tmp");
        m_pReader->SetTempDirectory(m_sFileDir + L"/tmp");

        if (m_pReader->LoadFromFile(sFilePath, L"", m_sPassword, m_sPassword))
        {
            m_nFileType = nFileTypeOpen;
            m_pHtmlRenderer = new NSHtmlRenderer::CASCHTMLRenderer3();
        }

        std::string sBase64Info = "";
        if (m_nFileType > 0)
        {
            m_pFontManager = m_pFonts->GenerateFontManager();
            NSFonts::IFontsCache* pFontsCache = NSFonts::NSFontCache::Create();
            pFontsCache->SetStreams(m_pFonts->GetStreams());
            m_pFontManager->SetOwnerCache(pFontsCache);
            m_pImageCache = NSImages::NSFilesCache::Create(m_pFonts);
            pFontsCache->SetCacheSize(16);
            //m_pFontManager->SetSubpixelRendering(true, false);

            m_lPagesCount = m_pReader->GetPagesCount();

            if (0 < m_lPagesCount)
            {
                m_pPageWidths = new double[m_lPagesCount];
                m_pPageHeights = new double[m_lPagesCount];
            }

            bool bIsBreak = false;

            //LOGGER_STRING2("pagescount: " + std::to_string(m_lPagesCount))
            if (m_lPagesCount > 0)
            {
                int* pData = new int[1 + m_lPagesCount * 2];
                int* pDataCur = pData;
                *pDataCur++ = m_lPagesCount;
                for (int i = 0; i < m_lPagesCount; ++i)
                {
                    double dWidth = 0, dHeight = 0, dDpiX = 0, dDpiY = 0;
                    m_pReader->GetPageInfo(i, &dWidth, &dHeight, &dDpiX, &dDpiY);

                    if (fabs(dWidth) < 0.1 || fabs(dHeight) < 0.1 || fabs(dDpiX) < 0.1 || fabs(dDpiY) < 0.1)
                    {
                        sBase64Info = "";
                        bIsBreak = true;
                        break;
                    }

                    dWidth = dWidth * 25.4 / dDpiX;
                    dHeight = dHeight * 25.4 / dDpiY;

                    m_pPageWidths[i] = dWidth;
                    m_pPageHeights[i] = dHeight;

                    *pDataCur++ = (int)(dWidth * 10000);
                    *pDataCur++ = (int)(dHeight * 10000);

                    //LOGGER_STRING2("page" + std::to_string(i) + ": " + std::to_string(dWidth) + " - " + std::to_string(dHeight));
                }

                if (!bIsBreak)
                {
                    char* pDataDst = NULL;
                    int nDataDst = 0;
                    NSFile::CBase64Converter::Encode((BYTE*)pData, (1 + 2 * m_lPagesCount) * sizeof(int), pDataDst, nDataDst, NSBase64::B64_BASE64_FLAG_NOCRLF);
                    sBase64Info = std::string(pDataDst, nDataDst);
                    RELEASEARRAYOBJECTS(pDataDst);

                    sBase64Info = std::to_string((1 + 2 * m_lPagesCount) * sizeof(int)) + ";" + sBase64Info;
                }
                else
                {
                    sBase64Info = "error";
                }

                RELEASEARRAYOBJECTS(pData);
            }
        }
        else
        {
            if (nFileTypeOpen == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF)
            {
                PdfReader::CPdfReader* pPdfReader = static_cast<PdfReader::CPdfReader*>(m_pReader);
                if (PdfReader::errorEncrypted == pPdfReader->GetError())
                {
                    sBase64Info = "password";
                }
                else
                {
                    sBase64Info = "error";
                }
            }
            else
            {
                sBase64Info = "error";
            }
        }

        m_oCS.Enter();
        m_sBase64File = sBase64Info;
        m_oCS.Leave();
    }

    void CloseFile()
    {
        CTemporaryCS oCS(&m_oCS);

        Stop();
        if (m_nFileType > 0)
        {
            m_pReader->Close();
            RELEASEOBJECT(m_pReader);
        }

        m_nFileType = 0;
    }

    std::wstring AddTask(const CNativeViewerPageInfo& oInfo, int nPageStart, int nPageEnd)
    {
        CTemporaryCS oCS(&m_oCS);

        // 1) удаляем из очереди все страницы, которых уже нет на экране
        // а также страницу с таким же индексом
        std::list<CNativeViewerPageInfo>::iterator find = m_arTasks.begin();
        while (find != m_arTasks.end())
        {
            if (find->Page < nPageStart || find->Page > nPageEnd || find->Page == oInfo.Page)
                find = m_arTasks.erase(find);
            else
                find++;
        }
        // 2) теперь нужно понять, есть ли такая страница на диске
        if (oInfo == m_oCurrentTask)
            return L"";

        // 3) проверяем, есть ли нужный файл.
        std::wstring sPath = GetPathPageImage(oInfo);
        if (NSFile::CFileBinary::Exists(sPath))
            return sPath;

        // 4) ничего не подходит, добавляем задачу
        m_arTasks.push_back(oInfo);
        return L"";
    }

    inline std::wstring GetPathPageImage(const CNativeViewerPageInfo& oInfo)
    {
        return m_sFileDir + L"/media/page" + std::to_wstring(oInfo.Page) + L"_" + std::to_wstring(oInfo.W) + L"x" + std::to_wstring(oInfo.H) + L".png";
    }

protected:
    virtual void OnTimer()
    {
        // функция отрисовки страницы
        if (!m_sOpeningFilePath.empty())
        {
            this->OpenFile(m_sOpeningFilePath);
            m_sOpeningFilePath = L"";
            return;
        }

        m_oCS.Enter();
        if (0 == m_arTasks.size())
        {
            // очередь пуста. Ничего не делаем
            m_oCS.Leave();

            // теперь смотрим, есть ли текстовые задачи
            if (m_pHtmlRenderer)
            {
                if (m_nFileType == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU)
                {
                    m_lTextPageCount = m_lPagesCount;
                    m_pEvents->OnPageText(m_lPagesCount - 1, 0, 0, 0, 0, "0;");
                }

                if (m_lTextPageCount >= m_lPagesCount)
                {
                    RELEASEOBJECT(m_pHtmlRenderer);
                }
                else
                {
                    if (0 == m_lTextPageCount)
                    {
                        m_pHtmlRenderer->SetOnlyTextMode(true);
                        m_pHtmlRenderer->CreateOfficeFile(L"temp/temp", m_sFontsDir);
                    }

                    m_pHtmlRenderer->NewPage();
                    m_pHtmlRenderer->BeginCommand(c_nPageType);

                    m_pHtmlRenderer->put_Width(m_pPageWidths[m_lTextPageCount]);
                    m_pHtmlRenderer->put_Height(m_pPageHeights[m_lTextPageCount]);

                    m_pHtmlRenderer->SetAdditionalParam("DisablePageEnd", L"yes");
                    m_pReader->DrawPageOnRenderer(m_pHtmlRenderer, m_lTextPageCount, NULL);
                    m_pHtmlRenderer->SetAdditionalParam("DisablePageEnd", L"no");

                    int paragraphs = 0;
                    int words = 0;
                    int symbols = 0;
                    int spaces = 0;
                    std::string info;
                    m_pHtmlRenderer->GetLastPageInfo(paragraphs, words, symbols, spaces, info);

                    m_pHtmlRenderer->EndCommand(c_nPageType);
                    ++m_lTextPageCount;

                    m_pEvents->OnPageText(m_lTextPageCount - 1, paragraphs, words, symbols, spaces, info);
                }

                if (m_lTextPageCount >= m_lPagesCount)
                {
                    RELEASEOBJECT(m_pHtmlRenderer);
                    SetInterval(m_nIntervalTimerDraw);
                }
            }

            return;
        }

        m_oCurrentTask = m_arTasks.front();
        m_arTasks.pop_front();
        m_oCS.Leave();

        CBgraFrame oFrame;
        int nRasterW = m_oCurrentTask.W;
        int nRasterH = m_oCurrentTask.H;

        int nMax = 4000;
        if (nRasterW > nMax || nRasterH > nMax)
        {
            int nRasterMax = std::max(nRasterW, nRasterH);
            nRasterW = (int)(nRasterW * ((double)nMax / nRasterMax));
            nRasterH = (int)(nRasterH * ((double)nMax / nRasterMax));
        }

        oFrame.put_Width(nRasterW);
        oFrame.put_Height(nRasterH);
        oFrame.put_Stride(4 * nRasterW);

        BYTE* pDataRaster = new BYTE[4 * nRasterW * nRasterH];
        memset(pDataRaster, 0xFF, 4 * nRasterW * nRasterH);
        oFrame.put_Data(pDataRaster);

        NSGraphics::IGraphicsRenderer* pRenderer = NSGraphics::Create();
        pRenderer->SetFontManager(m_pFontManager);
        pRenderer->SetImageCache(m_pImageCache);

        pRenderer->CreateFromBgraFrame(&oFrame);
        pRenderer->SetTileImageDpi(96.0);

        pRenderer->SetSwapRGB(false);
        pRenderer->put_Width(m_pPageWidths[m_oCurrentTask.Page]);
        pRenderer->put_Height(m_pPageHeights[m_oCurrentTask.Page]);

        m_pReader->DrawPageOnRenderer(pRenderer, m_oCurrentTask.Page, NULL);

        RELEASEINTERFACE(pRenderer);

        std::wstring sPath = GetPathPageImage(m_oCurrentTask);
        oFrame.SaveFile(sPath, 4);

        if (sPath.c_str()[0] == '/')
            sPath = L"file://" + sPath;
        else
            sPath = L"file:///" + sPath;

        m_pEvents->OnPageSaved(sPath, m_oCurrentTask.Page, m_oCurrentTask.W, m_oCurrentTask.H);

        m_oCS.Enter();
        m_oCurrentTask.Clear();
        m_oCS.Leave();
    }
};

#endif // APPLICATION_NATIVEVIEWER_H
