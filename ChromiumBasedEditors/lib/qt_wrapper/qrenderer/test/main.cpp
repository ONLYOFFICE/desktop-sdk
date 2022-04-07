#include "./../../../../../../core/DesktopEditor/graphics/MetafileToRenderer.h" // ConvertBufferToRenderer
#include "./../../../../../../core/DesktopEditor/fontengine/ApplicationFontsWorker.h"
#include "./../../../../../../core/DesktopEditor/common/File.h"
#include "./../../../../../../core/DesktopEditor/common/Directory.h"
#include "./../include/qrenderer.h" // рендерер
#include "./../../include/qascprinter.h" // контекст

#ifdef SUPPORT_DRAWING_FILE
#include "./../../../../../../core/PdfReader/PdfReader.h"
#include "./../../../../../../core/DjVuFile/DjVu.h"
#include "./../../../../../../core/XpsFile/XpsFile.h"
#endif

#include <QApplication>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QDebug>
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>
#include <QScreen>
#include <iostream>
#include <map>

#ifdef GetTempPath
#undef GetTempPath
#endif
#ifdef CreateDirectory
#undef CreateDirectory
#endif

//#define TEST_ON_IMAGE

// для теста печати файлом из js редакторов - нужен класс, который
// конвертирует локальные картинки в абсолютные.
// ну и также base64
// для теста просто реализуем что-то похожее из desktopeditor
class CMetafileToRenderer : public IMetafileToRenderter
{
private:
    // тут копим картинки
    std::map<std::wstring, std::wstring> m_mapImages;
    // временные файлы
    std::map<std::wstring, std::wstring> m_mapImagesDelete;
    // путь к папке с картинками
    std::wstring m_sDocumentImagesPath;

public:
    CMetafileToRenderer(IRenderer* pRenderer, std::wstring sFolder) : IMetafileToRenderter(pRenderer)
    {
        m_sDocumentImagesPath = sFolder;
    }

    virtual ~CMetafileToRenderer() override
    {
        for (std::map<std::wstring, std::wstring>::iterator iter = m_mapImagesDelete.begin(); iter != m_mapImagesDelete.end(); iter++)
        {
            NSFile::CFileBinary::Remove(iter->second);
        }
    }

public:
    virtual std::wstring GetImagePath(const std::wstring& sPath) override
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
        // в тестовом примере не учитываем такие ссылки, чтобы не делать зависимость на kernel_network

        // 4) может это файл файла?
        if (0 == sPath.find(L"media/image") || 0 == sPath.find(L"image") ||
            0 == sPath.find(L"image/display") || 0 == sPath.find(L"display") ||
            (0 == sPath.find(L"media/")))
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

        // 5) может это файл темы?
        // тоже в тестовом примере не поддерживаем темы в редакторе презентаций

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
};

#ifdef SUPPORT_DRAWING_FILE
void printFirstPageOfFile(const std::wstring& sFile, NSQRenderer::CQRenderer* pRenderer, NSFonts::IApplicationFonts* pFonts)
{
    std::wstring sTempDir = NSFile::GetProcessDirectory() + L"/temp";

    if (!NSDirectory::Exists(sTempDir))
        NSDirectory::CreateDirectory(sTempDir);

    IOfficeDrawingFile* pReader = NULL;
    std::wstring sExt = NSFile::GetFileExtention(sFile);

    // чеккер по форматам не используем - чтобы не тащить зависимость.
    // в тестовом примере хватит и расширения
    if (L"pdf" == sExt)
        pReader = new PdfReader::CPdfReader(pFonts);
    else if (L"xps" == sExt)
        pReader = new CXpsFile(pFonts);
    else if (L"djvu" == sExt)
        pReader = new CDjVuFile(pFonts);

    if (pReader)
    {
        pReader->LoadFromFile(sFile);
        pReader->DrawPageOnRenderer(pRenderer, 0, NULL);
        pReader->
    }

    NSDirectory::DeleteDirectory(sTempDir);
}
#endif

void printJsBuffer(const std::wstring& sDirectory, NSQRenderer::CQRenderer* pRenderer)
{
    BYTE* pSampleBuffer = NULL;
    DWORD nSampleBufferLen = 0;
    NSFile::CFileBinary::ReadAllBytes(sDirectory + L"/sample.buffer", &pSampleBuffer, nSampleBufferLen);

    std::cout << "sample buffer length: " << nSampleBufferLen << std::endl;

    CMetafileToRenderer corrector(pRenderer, sDirectory + L"/");
    NSOnlineOfficeBinToPdf::ConvertBufferToRenderer(pSampleBuffer, nSampleBufferLen, &corrector);

    RELEASEARRAYOBJECTS(pSampleBuffer);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // смотрим, изменились ли шрифты в системе, или это первый запуск?
    CApplicationFontsWorker oWorker;
    oWorker.m_sDirectory = NSFile::GetProcessDirectory() + L"/fonts_cache";
    oWorker.m_bIsNeedThumbnails = false;

    if (!NSDirectory::Exists(oWorker.m_sDirectory))
        NSDirectory::CreateDirectory(oWorker.m_sDirectory);

    NSFonts::IApplicationFonts* pFonts = oWorker.Check();

    // путь к примеру. считаем так - если там есть pdf/xps/djvu - то берем первую страницу из этих файлов
    // иначе - команды на печать из редактора js
    std::wstring sExamplePath = NSFile::GetProcessDirectory() + L"/../examples/example3";

#ifdef TEST_ON_IMAGE
    int nDpi = 96;
    QSize a4size = QPageSize::sizePixels(QPageSize::PageSizeId::A4, nDpi);
    QImage paintDevice{a4size, QImage::Format::Format_ARGB32_Premultiplied};
    paintDevice.setDotsPerMeterX(nDpi / 0.0254);
    paintDevice.setDotsPerMeterY(nDpi / 0.0254);
    paintDevice.fill(Qt::GlobalColor::white);
#else
//    QPrinterInfo info = QPrinterInfo::printerInfo("Microsoft XPS Document Writer");
    QPrinter paintDevice;
//    paintDevice.setPageOrientation(QPageLayout::Landscape);
    paintDevice.setOutputFileName("/home/ivaz28/Desktop/output.pdf");
    QPrintDialog dialog(&paintDevice);
    dialog.exec();
#endif

    QAscPrinterContext oContext(&paintDevice);

    oContext.BeginPaint();

    NSQRenderer::CQRenderer renderer(&oContext);
    renderer.InitFonts(pFonts);

#ifdef SUPPORT_DRAWING_FILE
    if (NSFile::CFileBinary::Exists(L"/home/ivaz28/Desktop/albom.pdf"))
        printFirstPageOfFile(L"/home/ivaz28/Desktop/albom.pdf", &renderer, pFonts);
    else if (NSFile::CFileBinary::Exists(sExamplePath + L"/sample.xps"))
        printFirstPageOfFile(sExamplePath + L"/sample.xps", &renderer, pFonts);
    else if (NSFile::CFileBinary::Exists(sExamplePath + L"/sample.djvu"))
        printFirstPageOfFile(sExamplePath + L"/sample.djvu", &renderer, pFonts);
    else
#endif
        printJsBuffer(sExamplePath, &renderer);

    oContext.EndPaint();


    RELEASEINTERFACE(pFonts);

#ifdef TEST_ON_IMAGE
    QWidget w;
    QLabel label;

    label.setPixmap(QPixmap::fromImage(paintDevice));
    QVBoxLayout layout;
    layout.addWidget(&label);
    w.setLayout(&layout);
    w.show();
#endif


    return /*a.exec()*/0;
}
