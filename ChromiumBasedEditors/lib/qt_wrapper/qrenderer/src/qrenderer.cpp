#include "./../include/qrenderer.h"
#include "./../../include/qascprinter.h"
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QTransform>

#include <algorithm>
#include "../../../../../../core/DesktopEditor/graphics/MetafileToRenderer.h"
#include "../../../../../../core/DesktopEditor/raster/ImageFileFormatChecker.h"
#include "../../../../../../core/DesktopEditor/common/File.h"

//#define ENABLE_LOGS

#ifdef _WIN32
#define QRENDERER_SUPPORT_TEXTURE_BRUSH
#endif

#ifdef _WIN32
#define QRENDERER_SUPPORT_TEXTURE_TILE_BRUSH
#endif

#ifdef ENABLE_LOGS
#include <iostream>
#include <QDebug>
#define TELL qDebug() << __func__

namespace
{
	// logger
	template<typename T>
	QString toBin(T int_value)
	{
		QString result;
		for (std::size_t i = 0; i < sizeof(T) * 8; ++i)
		{
			if (i && 0 == i % 8)
				result += ' ';

			result += ((int_value >> i) & (T)1) ? '1' : '0';
		}
		std::reverse(result.begin(), result.end());
		return result;
	}

	QString getLineCap(long val)
	{
		//const long c_ag_LineCapFlat           = 0;
		//const long c_ag_LineCapSquare         = 1;
		//const long c_ag_LineCapTriangle       = 3;
		//const long c_ag_LineCapNoAnchor       = 16;
		//const long c_ag_LineCapSquareAnchor   = 17;
		//const long c_ag_LineCapRoundAnchor    = 18;
		//const long c_ag_LineCapDiamondAnchor  = 19;
		//const long c_ag_LineCapArrowAnchor    = 20;
		//const long c_ag_LineCapAnchorMask     = 240;
		//const long c_ag_LineCapCustom         = 255;

		switch (val)
		{
		case 0:   return "c_ag_LineCapFlat";
		case 1:   return "c_ag_LineCapSquare";
		case 3:   return "c_ag_LineCapTriangle";
		case 16:  return "c_ag_LineCapNoAnchor";
		case 17:  return "c_ag_LineCapSquareAnchor";
		case 18:  return "c_ag_LineCapRoundAnchor";
		case 19:  return "c_ag_LineCapDiamondAnchor";
		case 20:  return "c_ag_LineCapArrowAnchor";
		case 240: return "c_ag_LineCapAnchorMask";
		case 255: return "c_ag_LineCapCustom";
		default:  return "unknown line cap";
		}
	}

	QString getDashCap(long val)
	{
		//const long c_ag_DashCapFlat       = 0;
		//const long c_ag_DashCapRound      = 2;
		//const long c_ag_DashCapTriangle   = 3;

		switch (val)
		{
		case 0:  return "c_ag_DashCapFlat";
		case 2:  return "c_ag_DashCapRound";
		case 3:  return "c_ag_DashCapTriangle";
		default: return "unknown dash cap";
		}
	}

	QString getLineJoin(long val)
	{
		//const long c_ag_LineJoinMiter         = 0;
		//const long c_ag_LineJoinBevel         = 1;
		//const long c_ag_LineJoinRound         = 2;
		//const long c_ag_LineJoinMiterClipped  = 3;

		switch (val)
		{
		case 0:  return "c_ag_LineJoinMiter";
		case 1:  return "c_ag_LineJoinBevel";
		case 2:  return "c_ag_LineJoinRound";
		case 3:  return "c_ag_LineJoinMiterClipped";
		default: return "unknown line join";
		}
	}

	QString getPenAlignment(long val) // UNUSED
	{
		//const long c_ag_PenAlignmentCenter    = 0;
		//const long c_ag_PenAlignmentInset     = 1;
		//const long c_ag_PenAlignmentOutset    = 2;
		//const long c_ag_PenAlignmentLeft      = 3;
		//const long c_ag_PenAlignmentRight     = 4;

		switch (val)
		{
		case 0:  return "c_ag_PenAlignmentCenter";
		case 1:  return "c_ag_PenAlignmentInset";
		case 2:  return "c_ag_PenAlignmentOutset";
		case 3:  return "c_ag_PenAlignmentLeft";
		case 4:  return "c_ag_PenAlignmentRight";
		default: return "unknown alignment";
		}
	}

	QString getBrushType(long val)
	{
		//// old constants for brush type
		//const long c_BrushTypeSolid_                  = 0;
		//const long c_BrushTypeHorizontal_             = 1;
		//const long c_BrushTypeVertical_               = 2;
		//const long c_BrushTypeDiagonal1_              = 3;
		//const long c_BrushTypeDiagonal2_              = 4;
		//const long c_BrushTypeCenter_                 = 5;
		//const long c_BrushTypePathGradient1_          = 6;
		//const long c_BrushTypePathGradient2_          = 7;
		//const long c_BrushTypeTexture_                = 8;
		//const long c_BrushTypeHatch1_                 = 9;
		//const long c_BrushTypeHatch53_                = 61;
		//const long c_BrushTypeGradient1_              = 62;
		//const long c_BrushTypeGradient6_              = 70;

		//const long c_BrushTypeSolid                   = 1000;
		//const long c_BrushTypeHorizontal              = 2001;
		//const long c_BrushTypeVertical                = 2002;
		//const long c_BrushTypeDiagonal1               = 2003;
		//const long c_BrushTypeDiagonal2               = 2004;
		//const long c_BrushTypeCenter                  = 2005;
		//const long c_BrushTypePathGradient1           = 2006; // left for comparability
		//const long c_BrushTypePathGradient2           = 2007; // left for comparability
		//const long c_BrushTypeCylinderHor             = 2008;
		//const long c_BrushTypeCylinderVer             = 2009;
		//const long c_BrushTypeTexture                 = 3008;
		//const long c_BrushTypePattern                 = 3009;
		//const long c_BrushTypeHatch1                  = 4009;
		//const long c_BrushTypeHatch53                 = 4061;
		//const long c_BrushTypeNoFill                  = 5000;
		//const long c_BrushTypeNotSet                  = 5001;

		//const long c_BrushTypeMyTestGradient          = 6000;
		//const long c_BrushTypePathRadialGradient      = 6001;
		//const long c_BrushTypePathConicalGradient     = 6002;
		//const long c_BrushTypePathDiamondGradient     = 6003;
		//const long c_BrushTypePathNewLinearGradient   = 6004;
		//const long c_BrushTypeTriagnleMeshGradient    = 6005;
		//const long c_BrushTypeCurveGradient           = 6006;
		//const long c_BrushTypeTensorCurveGradient     = 6007;

		switch (val)
		{
		case 0:    return "c_BrushTypeSolid_ (old)";
		case 1:    return "c_BrushTypeHorizontal_ (old)";
		case 2:    return "c_BrushTypeVertical_ (old)";
		case 3:    return "c_BrushTypeDiagonal1_ (old)";
		case 4:    return "c_BrushTypeDiagonal2_ (old)";
		case 5:    return "c_BrushTypeCenter_ (old)";
		case 6:    return "c_BrushTypePathGradient1_ (old)";
		case 7:    return "c_BrushTypePathGradient2_ (old)";
		case 8:    return "c_BrushTypeTexture_ (old)";
		case 9:    return "c_BrushTypeHatch1_ (old)";
		case 61:   return "c_BrushTypeHatch53_ (old)";
		case 62:   return "c_BrushTypeGradient1_ (old)";
		case 70:   return "c_BrushTypeGradient6_ (old)";

		case 1000: return "c_BrushTypeSolid";
		case 2001: return "c_BrushTypeHorizontal";
		case 2002: return "c_BrushTypeVertical";
		case 2003: return "c_BrushTypeDiagonal1";
		case 2004: return "c_BrushTypeDiagonal2";
		case 2005: return "c_BrushTypeCenter";
		case 2006: return "c_BrushTypePathGradient1";
		case 2007: return "c_BrushTypePathGradient2";
		case 2008: return "c_BrushTypeCylinderHor";
		case 2009: return "c_BrushTypeCylinderVer";

		case 3008: return "c_BrushTypeTexture";
		case 3009: return "c_BrushTypePattern";
		case 4009: return "c_BrushTypeHatch1";
		case 4061: return "c_BrushTypeHatch53";
		case 5000: return "c_BrushTypeNoFill";
		case 5001: return "c_BrushTypeNotSet";

		case 6000: return "c_BrushTypeMyTestGradient";
		case 6001: return "c_BrushTypePathRadialGradient";
		case 6002: return "c_BrushTypePathConicalGradient";
		case 6003: return "c_BrushTypePathDiamondGradient";
		case 6004: return "c_BrushTypePathNewLinearGradient";
		case 6005: return "c_BrushTypeTriagnleMeshGradient";
		case 6006: return "c_BrushTypeCurveGradient";
		case 6007: return "c_BrushTypeTensorCurveGradient";
		default:   return "unknown type";
		}
	}

	QString getBrushTextureMode(long val)
	{
		//const long c_BrushTextureModeStretch      = 0;
		//const long c_BrushTextureModeTile         = 1;
		//const long c_BrushTextureModeTileCenter   = 2;

		switch (val)
		{
		case 0:  return "c_BrushTextureModeStretch";
		case 1:  return "c_BrushTextureModeTile";
		case 2:  return "c_BrushTextureModeTileCenter";
		default: return "unknown mode";
		}
	}
}

#endif

namespace NSConversions
{
	inline QColor toColor(const LONG& color, const LONG& alpha)
	{
		return QColor(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, alpha & 0xFF);
	}

	void cleanupPixels(void* info)
	{
		Aggplus::CImage* pImage = (Aggplus::CImage*)info;
		RELEASEOBJECT(pImage);
	}
	void cleanupPixels2(void* info)
	{
		unsigned char* data = (unsigned char*)info;
		RELEASEARRAYOBJECTS(data);
	}

	QImage* createTextureImage(Aggplus::CImage* pImage)
	{
		if (!pImage || pImage->GetLastStatus() != Aggplus::Ok)
			return NULL;

		int nWidth = (int)pImage->GetWidth();
		int nHeight = (int)pImage->GetHeight();
		int nStride = (int)pImage->GetStride();

		int nCheckLE = 1;
		bool bIsLE = true;
		if (*(char *)&nCheckLE != 1)
			bIsLE = false;

		// Little Endian
		// QImage::Format_ARGB32 the bytes are ordered: B G R A
		// Big Endian
		// QImage::Format_ARGB32 the bytes are ordered: A R G B

		if (bIsLE)
		{
			size_t nDataSize = 4 * nWidth * nHeight;
			unsigned char* data = new unsigned char[nDataSize];

			if (pImage->GetStride() > 0)
			{
				memcpy(data, pImage->GetData(), nDataSize);
			}
			else
			{
				int stride = 4 * nWidth;
				unsigned char* dataTmp = data;
				unsigned char* dataSrc = pImage->GetData() + stride * (nHeight - 1);
				for (int i = 0; i < nHeight; ++i)
				{
					memcpy(dataTmp, dataSrc, stride);
					dataTmp += stride;
					dataSrc -= stride;
				}
			}

			return new QImage(data, nWidth, nHeight, nStride, QImage::Format_ARGB32, cleanupPixels2, data);
		}

		// copy image
		QImage* pQImage = new QImage(nWidth, nHeight, QImage::Format_ARGB32);
		const uchar* pSrc = pImage->GetData();

		for (int nY = 0; nY < nHeight; ++nY)
		{
			// If you are accessing 32-bpp image data, cast the returned pointer to QRgb*
			// (QRgb has a 32-bit size) and use it to read/write the pixel value
			uchar* line = pQImage->scanLine(nY);
			const uchar* pSrcTmp = pImage->GetData();

			if (nStride < 0)
				pSrcTmp -= ((nHeight - nY - 1) * nStride);
			else
				pSrcTmp += (nY * nStride);

			for (int nX = 0; nX < nWidth; ++nX)
			{
				*line++ = pSrcTmp[3];
				*line++ = pSrcTmp[2];
				*line++ = pSrcTmp[1];
				*line++ = pSrcTmp[0];
				pSrcTmp += 4;
			}
		}

		return pQImage;
	}

	QImage* createTextureImage(const std::wstring& path)
	{
		Aggplus::CImage* pImage = new Aggplus::CImage(path);
		if (pImage->GetLastStatus() != Aggplus::Ok)
		{
			RELEASEOBJECT(pImage);
			return NULL;
		}

		QImage* pQImage = createTextureImage(pImage);
		RELEASEOBJECT(pImage);
		return pQImage;
	}

	QBrush* createTextureBrush(Aggplus::CImage* pImage, const bool& bIsDestroyImage, int& nWidth, int &nHeight)
	{
		if (!pImage || pImage->GetLastStatus() != Aggplus::Ok)
			return NULL;

		QBrush* pBrush = new QBrush();

		nWidth = (int)pImage->GetWidth();
		nHeight = (int)pImage->GetHeight();
		int nStride = (int)pImage->GetStride();

		int nCheckLE = 1;
		bool bIsLE = true;
		if (*(char *)&nCheckLE != 1)
			bIsLE = false;

		// Little Endian
		// QImage::Format_ARGB32 the bytes are ordered: B G R A
		// Big Endian
		// QImage::Format_ARGB32 the bytes are ordered: A R G B

		if (bIsLE)
		{
			if (nStride < 0)
			{
				// непонятное поведение QT. они неправильно работают с stride
				// нельзя послать отрицательное значение. вернее можно - но эффекта 0
				// при этом нельзя послать и pImage->GetData() - nStride * (nHeight - 1) с отрицательным stride
				// поэтому делаем переворот трансформом. возможно в другой версии QT (не 5.15.2) - бага нет

				nStride = -nStride;
				pBrush->setTransform(QTransform(1, 0, 0, -1, 0, nHeight));
			}

			if (bIsDestroyImage)
				pBrush->setTextureImage(QImage(pImage->GetData(), nWidth, nHeight, nStride, QImage::Format_ARGB32, cleanupPixels, pImage));
			else
			{
				size_t nDataSize = 4 * nWidth * nHeight;
				unsigned char* data = new unsigned char[nDataSize];
				memcpy(data, pImage->GetData(), nDataSize);
				pBrush->setTextureImage(QImage(data, nWidth, nHeight, nStride, QImage::Format_ARGB32, cleanupPixels2, data));

				// отложенно печатается, а pImage мы уже удалим - поэтому копируем память.
				//pBrush->setTextureImage(QImage(pImage->GetData(), nWidth, nHeight, nStride, QImage::Format_ARGB32));
			}
		}
		else
		{
			// copy image
			QImage oImage(nWidth, nHeight, QImage::Format_ARGB32);
			const uchar* pSrc = pImage->GetData();
			if (nStride < 0)
				pSrc += nStride * (nHeight - 1);

			for (int nY = 0; nY < nHeight; ++nY)
			{
				// If you are accessing 32-bpp image data, cast the returned pointer to QRgb*
				// (QRgb has a 32-bit size) and use it to read/write the pixel value
				uchar* line = oImage.scanLine(nY);

				for (int nX = 0; nX < nWidth; ++nX)
				{
					*line++ = pSrc[3];
					*line++ = pSrc[2];
					*line++ = pSrc[1];
					*line++ = pSrc[0];
					pSrc += 4;
				}
			}

			pBrush->setTextureImage(oImage);
		}

		return pBrush;
	}

	QBrush* createTextureBrush(const std::wstring &path, int& nWidth, int &nHeight)
	{
		Aggplus::CImage* pImage = new Aggplus::CImage(path);
		if (pImage->GetLastStatus() != Aggplus::Ok)
		{
			RELEASEOBJECT(pImage);
			return NULL;
		}

		return createTextureBrush(pImage, true, nWidth, nHeight);
	}

	void correctBrushTextureTransform(NSStructures::CBrush* pLogicBrush, QBrush* pBrush, QPainterPath* pPath,
	                                  const int& nImageWidth, const int& nImageHeight,
	                                  NSQRenderer::CQRenderer* pRenderer)
	{
		int nTextureMode = c_BrushTextureModeStretch;
		if (pLogicBrush)
			nTextureMode = pLogicBrush->TextureMode;

		QTransform oTransform;

		QRectF oPathBounds = pPath->boundingRect();
		switch (nTextureMode)
		{
		case c_BrushTextureModeStretch:
		{
			if (pLogicBrush && pLogicBrush->Rectable)
			{
				oTransform.translate((oPathBounds.left() - pLogicBrush->Rect.X) / nImageWidth,
									 (oPathBounds.top() - pLogicBrush->Rect.Y) / nImageHeight);
				oTransform.translate(pLogicBrush->Rect.X, pLogicBrush->Rect.Y);

				oTransform.scale(pLogicBrush->Rect.Width / nImageWidth,
				                 pLogicBrush->Rect.Height / nImageHeight);
			}
			else
			{
				oTransform.translate(oPathBounds.left(), oPathBounds.top());

				oTransform.scale(oPathBounds.width() / nImageWidth,
				                 oPathBounds.height() / nImageHeight);
			}
			break;
		}
		case c_BrushTextureModeTile:
		case c_BrushTextureModeTileCenter:
		{
			double dTileDpi = 96.0;
			double dDpiX = dTileDpi;
			double dDpiY = dTileDpi;
			pRenderer->get_DpiX(&dDpiX);
			pRenderer->get_DpiY(&dDpiY);
			double dTileScaleX = 1.0;
			double dTileScaleY = 1.0;
			pRenderer->GetTileScaleX(dTileScaleX);
			pRenderer->GetTileScaleY(dTileScaleY);


			double dScaleX = dDpiX / (dTileDpi * pRenderer->GetCoordTransform().m11() * dTileScaleX);
			double dScaleY = dDpiY / (dTileDpi * pRenderer->GetCoordTransform().m22() * dTileScaleY);

			if (pLogicBrush && pLogicBrush->Rectable)
			{
				double dOffsetX = (oPathBounds.left() - pLogicBrush->Rect.X) / nImageWidth;
				double dOffsetY = (oPathBounds.top() - pLogicBrush->Rect.Y) / nImageHeight;

				//dOffsetX *= (dDpiX / dTileDpi);
				//dOffsetY *= (dDpiY / dTileDpi);

				oTransform.translate(dOffsetX, dOffsetY);
				oTransform.translate(pLogicBrush->OffsetX, pLogicBrush->OffsetY);
				if (pLogicBrush->IsScale)
					oTransform.scale(pLogicBrush->ScaleX, pLogicBrush->ScaleY);
			}

			oTransform.scale(dScaleX, dScaleY);
			break;
		}
		default:
			break;
		}

		oTransform = pBrush->transform() * oTransform;

		pBrush->setTransform(oTransform);
	}
}

NSQRenderer::CQRenderer::CQRenderer(QAscPrinterContext* pContext)
{
	m_pContext = pContext;
	InitDefaults();
}

NSQRenderer::CQRenderer::~CQRenderer()
{
	RELEASEINTERFACE(m_pFontManager);
	RELEASEINTERFACE(m_pAppFonts);
}

void NSQRenderer::CQRenderer::InitDefaults()
{
	m_pAppFonts = NULL;
	m_pFontManager = NULL;
	m_bIsUseTextAsPath = true;

	m_nPixelWidth = 0;
	m_nPixelHeight = 0;

	m_dTileScaleX = 1.0;
	m_dTileScaleY = 1.0;

	m_lCurrentCommand = c_nNone;
	m_oSimpleGraphicsConverter.SetRenderer(this);

	m_oInstalledFont.Size = -1;
}

void NSQRenderer::CQRenderer::SetUseTextAsPath(const bool &bIsUseTextAsPath)
{
	m_bIsUseTextAsPath = bIsUseTextAsPath;
}

void NSQRenderer::CQRenderer::SetFont()
{
	double dDpiX = 0; get_DpiX(&dDpiX);
	double dDpiY = 0; get_DpiY(&dDpiY);
	double dPix = m_oFont.CharSpace * dDpiX / 25.4;

	if (m_oInstalledFont.IsEqual(&m_oFont))
	{
		m_pFontManager->SetCharSpacing(dPix);
		return;
	}

	m_pFontManager->SetStringGID(m_oFont.StringGID);
	m_pFontManager->SetCharSpacing(dPix);

	if (m_oFont.Path.empty())
	{
		m_pFontManager->LoadFontByName(m_oFont.Name, (float)m_oFont.Size, m_oFont.GetStyle(), dDpiX, dDpiY);
	}
	else
	{
		m_pFontManager->LoadFontFromFile(m_oFont.Path, m_oFont.FaceIndex, (float)m_oFont.Size, dDpiX, dDpiY);
	}
	// в этом рендерере транчформ всегда в гарфике. в текстовом всегда единичная.
	// но это менеджер мог быть использован вне рендерера. наверное стоит завести отдельный
	// менеджер для печати через qrenderer
	m_pFontManager->SetTextMatrix(1, 0, 0, 1, 0, 0);

	m_oInstalledFont = m_oFont;
}

void NSQRenderer::CQRenderer::InitFonts(NSFonts::IApplicationFonts *pFonts)
{
#ifdef ENABLE_LOGS
	TELL;
#endif

	RELEASEINTERFACE(m_pAppFonts);
	m_pAppFonts = pFonts;
	ADDREFINTERFACE(m_pAppFonts);

	RELEASEINTERFACE(m_pFontManager);
	m_pFontManager = m_pAppFonts->GenerateFontManager();
}

void NSQRenderer::CQRenderer::SetFontsManager(NSFonts::IFontManager* pFontsManager)
{
	if (NULL == m_pAppFonts)
	{
		m_pAppFonts = pFontsManager->GetApplication();
		ADDREFINTERFACE(m_pAppFonts);
	}

	RELEASEINTERFACE(m_pFontManager);
	m_pFontManager = pFontsManager;
	ADDREFINTERFACE(m_pFontManager);
}

HRESULT NSQRenderer::CQRenderer::get_Type(LONG *lType)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lType)
	{
		*lType = c_nQRenderer;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::NewPage()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_pContext->NewPage();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_Width(double *dWidth)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dWidth)
	{
		*dWidth = m_dLogicalPageWidth;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::get_Height(double *dHeight)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dHeight)
	{
		*dHeight = m_dLogicalPageHeight;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_Width(const double &dWidth)
{
#ifdef ENABLE_LOGS
	TELL << dWidth;
#endif
	m_dLogicalPageWidth = dWidth;
	double realWidth = (0 == m_nPixelWidth) ? (double)paperSize().width() : (double)m_nPixelWidth;

	m_oCoordTransform = {
	    realWidth / m_dLogicalPageWidth,
	    m_oCoordTransform.m12(),
	    m_oCoordTransform.m21(),
	    m_oCoordTransform.m22(),
	    m_oCoordTransform.dx(),
	    m_oCoordTransform.dy()
	};

	applyTransform();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_Height(const double &dHeight)
{
#ifdef ENABLE_LOGS
	TELL << dHeight;
#endif
	m_dLogicalPageHeight = dHeight;
	double realHeight = (0 == m_nPixelHeight) ? (double)paperSize().height() : (double)m_nPixelHeight;

	m_oCoordTransform = {
	    m_oCoordTransform.m11(),
	    m_oCoordTransform.m12(),
	    m_oCoordTransform.m21(),
	    realHeight / m_dLogicalPageHeight,
	    m_oCoordTransform.dx(),
	    m_oCoordTransform.dy()
	};

	applyTransform();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_DpiX(double *dDpiX)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (!dDpiX)
		return S_FALSE;

	int nDpiX = 0;
	int nDpiY = 0;
	m_pContext->GetLogicalDPI(nDpiX, nDpiY);
	*dDpiX = (double)nDpiX;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_DpiY(double *dDpiY)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (!dDpiY)
		return S_FALSE;

	int nDpiX = 0;
	int nDpiY = 0;
	m_pContext->GetLogicalDPI(nDpiX, nDpiY);
	*dDpiY = (double)nDpiY;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenColor(LONG *lColor)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lColor)
	{
		*lColor = m_oPen.Color;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenColor(const LONG &lColor)
{
#ifdef ENABLE_LOGS
	TELL << toBin(lColor);
#endif
	m_oPen.Color = lColor;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenAlpha(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lAlpha)
	{
		*lAlpha = m_oPen.Alpha;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenAlpha(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oPen.Alpha = lAlpha;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenSize(double *dSize)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dSize)
	{
		*dSize = m_oPen.Size;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenSize(const double &dSize)
{
#ifdef ENABLE_LOGS
	TELL << dSize;
#endif
	m_oPen.Size = dSize;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenDashStyle(BYTE *val)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (val)
	{
		*val = m_oPen.DashStyle;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenDashStyle(const BYTE &val)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oPen.DashStyle = val;
	return S_OK;
}


HRESULT NSQRenderer::CQRenderer::get_PenLineStartCap(BYTE *val)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (val)
	{
		*val = m_oPen.LineStartCap;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineStartCap(const BYTE &val)
{
#ifdef ENABLE_LOGS
	TELL << getLineCap(val);
#endif
	m_oPen.LineStartCap = val;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenLineEndCap(BYTE *val)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (val)
	{
		*val = m_oPen.LineEndCap;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineEndCap(const BYTE &val)
{
#ifdef ENABLE_LOGS
	TELL << getLineCap(val);
#endif
	m_oPen.LineEndCap = val;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenLineJoin(BYTE *val)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (val)
	{
		*val = m_oPen.LineJoin;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineJoin(const BYTE &val)
{
#ifdef ENABLE_LOGS
	TELL << getLineJoin(val);
#endif
	m_oPen.LineJoin = val;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenDashOffset(double *dOffset)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dOffset)
	{
		*dOffset = m_oPen.DashOffset;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenDashOffset(const double &dOffset)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oPen.DashOffset = dOffset;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenAlign(LONG *lAlign)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lAlign)
	{
		*lAlign = m_oPen.Align;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenAlign(const LONG &lAlign)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oPen.Align = lAlign;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenMiterLimit(double *dOffset)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dOffset)
	{
		*dOffset = m_oPen.MiterLimit;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenMiterLimit(const double &dOffset)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oPen.MiterLimit = dOffset;
	return S_OK;
}

HRESULT get_BrushTextureImage(Aggplus::CImage** pImage)
{
	return S_OK;
}
HRESULT put_BrushTextureImage(Aggplus::CImage* pImage)
{
	return S_OK;
}
HRESULT get_BrushTransform(Aggplus::CMatrix& oMatrix)
{
	return S_OK;
}
HRESULT put_BrushTransform(const Aggplus::CMatrix& oMatrix)
{
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PenDashPattern(double *pPattern, LONG lCount)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	RELEASEARRAYOBJECTS((m_oPen.DashPattern));
	m_oPen.DashPattern = new double[lCount];
	memcpy(m_oPen.DashPattern, pPattern, lCount * sizeof(double));
	m_oPen.Count = lCount;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushType(LONG *lType)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lType)
	{
		*lType = m_oBrush.Type;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushType(const LONG &lType)
{
#ifdef ENABLE_LOGS
	TELL << getBrushType(lType);
#endif
	m_oBrush.Type = lType;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushColor1(LONG *lColor)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lColor)
	{
		*lColor = m_oBrush.Color1;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushColor1(const LONG &lColor)
{
#ifdef ENABLE_LOGS
	TELL << toBin(lColor);
#endif
	m_oBrush.Color1 = lColor;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushAlpha1(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lAlpha)
	{
		*lAlpha = m_oBrush.Alpha1;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushAlpha1(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBrush.Alpha1 = lAlpha;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushColor2(LONG *lColor)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lColor)
	{
		*lColor = m_oBrush.Color2;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushColor2(const LONG &lColor)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBrush.Color2 = lColor;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushAlpha2(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lAlpha)
	{
		*lAlpha = m_oBrush.Alpha2;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushAlpha2(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBrush.Alpha2 = lAlpha;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTexturePath(std::wstring *bsPath)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (bsPath)
	{
		*bsPath = m_oBrush.TexturePath;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTexturePath(const std::wstring &bsPath)
{
#ifdef ENABLE_LOGS
	TELL << "path size = " << bsPath.size();
#endif
	m_oBrush.TexturePath = bsPath;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTextureMode(LONG *lMode)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lMode)
	{
		*lMode = m_oBrush.TextureMode;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTextureMode(const LONG &lMode)
{
#ifdef ENABLE_LOGS
	TELL << getBrushTextureMode(lMode);
#endif
	m_oBrush.TextureMode = lMode;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTextureAlpha(LONG *lTxAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lTxAlpha)
	{
		*lTxAlpha = m_oBrush.TextureAlpha;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTextureAlpha(const LONG &lTxAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBrush.TextureAlpha = lTxAlpha;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushLinearAngle(double *dAngle)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dAngle)
	{
		*dAngle = m_oBrush.LinearAngle;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushLinearAngle(const double &dAngle)
{
#ifdef ENABLE_LOGS
	TELL << dAngle;
#endif
	m_oBrush.LinearAngle = dAngle;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BrushRect(const INT &val
                                           , const double &left
                                           , const double &top
                                           , const double &width
                                           , const double &height)
{
#ifdef ENABLE_LOGS
	TELL << QRectF{left, top, width, height};
#endif
	m_oBrush.Rectable = val;
	m_oBrush.Rect.X = (float)left;
	m_oBrush.Rect.Y = (float)top;
	m_oBrush.Rect.Width = (float)width;
	m_oBrush.Rect.Height = (float)height;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BrushBounds(const double &left
                                             , const double &top
                                             , const double &width
                                             , const double &height)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBrush.Bounds.left	= (float)left;
	m_oBrush.Bounds.top		= (float)top;
	m_oBrush.Bounds.right	= (float)(left + width);
	m_oBrush.Bounds.bottom	= (float)(top + height);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushOffset(double& offsetX, double& offsetY) const
{
	offsetX = m_oBrush.OffsetX;
	offsetY = m_oBrush.OffsetY;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_BrushOffset(const double& offsetX, const double& offsetY)
{
	m_oBrush.OffsetX = offsetX;
	m_oBrush.OffsetY = offsetY;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushScale(bool& isScale, double& scaleX, double& scaleY) const
{
	isScale = m_oBrush.IsScale;
	scaleX = m_oBrush.ScaleX;
	scaleY = m_oBrush.ScaleY;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_BrushScale(bool isScale, const double& scaleX, const double& scaleY)
{
	m_oBrush.IsScale = isScale;
	m_oBrush.ScaleX = scaleX;
	m_oBrush.ScaleY = scaleY;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_BrushGradientColors(LONG *lColors, double *pPositions, LONG nCount)
{
#ifdef ENABLE_LOGS
	TELL << nCount;
#endif
	m_oBrush.m_arrSubColors.clear();
	if (0 == nCount || nullptr == lColors || nullptr == pPositions)
		return S_FALSE;

	for (LONG i = 0; i < nCount; ++i)
	{
		NSStructures::CBrush::TSubColor color;
		color.color		= lColors[i];
		color.position	= (long)(pPositions[i] * 65536);
		m_oBrush.m_arrSubColors.push_back(color);

#ifdef ENABLE_LOGS
		qDebug() << toBin(lColors[i]) << "at" << pPositions[i];
#endif
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTextureImage(Aggplus::CImage** pImage) { return S_OK; }
HRESULT NSQRenderer::CQRenderer::put_BrushTextureImage(Aggplus::CImage* pImage) { return S_OK; }
HRESULT NSQRenderer::CQRenderer::get_BrushTransform(Aggplus::CMatrix& oMatrix) { return S_OK; }
HRESULT NSQRenderer::CQRenderer::put_BrushTransform(const Aggplus::CMatrix& oMatrix) { return S_OK; }

HRESULT NSQRenderer::CQRenderer::get_FontName(std::wstring *bsName)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (bsName) {
		*bsName = m_oFont.Name;
		return S_OK;
	} else {
		return S_FALSE;
	}
}

HRESULT NSQRenderer::CQRenderer::put_FontName(const std::wstring &bsName)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.Name = bsName;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontPath(std::wstring *bsName)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (bsName)
	{
		*bsName = m_oFont.Path;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontPath(const std::wstring &bsName)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.Path = bsName;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontSize(double *dSize)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dSize)
	{
		*dSize = m_oFont.Size;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontSize(const double &dSize)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.Size = dSize;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontStyle(LONG *lStyle)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lStyle)
	{
		*lStyle = m_oFont.GetStyle();
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontStyle(const LONG &lStyle)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.SetStyle(lStyle);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontStringGID(INT *bGID)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (bGID)
	{
		*bGID = m_oFont.StringGID;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontStringGID(const INT &bGID)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.StringGID = bGID;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontCharSpace(double *dSpace)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (dSpace)
	{
		*dSpace = m_oFont.CharSpace;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontCharSpace(const double &dSpace)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.CharSpace = dSpace;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontFaceIndex(int *lFaceIndex)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (lFaceIndex)
	{
		*lFaceIndex = m_oFont.FaceIndex;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontFaceIndex(const int &lFaceIndex)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oFont.FaceIndex = lFaceIndex;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextCHAR(const LONG &c
                                                     , const double &x
                                                     , const double &y
                                                     , const double &w
                                                     , const double &h)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (m_bIsUseTextAsPath)
	{
		PathCommandEnd();
		PathCommandTextCHAR(c, x, y, w, h);
		DrawPath(c_nWindingFillMode);
		PathCommandEnd();
		return S_OK;
	}

	// TODO:
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawText(const std::wstring &bsText
                                                 , const double &x
                                                 , const double &y
                                                 , const double &w
                                                 , const double &h)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (m_bIsUseTextAsPath)
	{
		PathCommandEnd();
		PathCommandText(bsText, x, y, w, h);
		DrawPath(c_nWindingFillMode);
		PathCommandEnd();
		return S_OK;
	}

	// TODO:
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextExCHAR(const LONG &c
                                                       , const LONG &gid
                                                       , const double &x
                                                       , const double &y
                                                       , const double &w
                                                       , const double &h)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (m_bIsUseTextAsPath)
	{
		PathCommandEnd();
		PathCommandTextExCHAR(c, gid, x, y, w, h);
		DrawPath(c_nWindingFillMode);
		PathCommandEnd();
		return S_OK;
	}

	// TODO:
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextEx(const std::wstring &bsUnicodeText
                                                   , const unsigned int *pGids
                                                   , const unsigned int nGidsCount
                                                   , const double &x
                                                   , const double &y
                                                   , const double &w
                                                   , const double &h)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (m_bIsUseTextAsPath)
	{
		PathCommandEnd();
		PathCommandTextEx(bsUnicodeText, pGids, nGidsCount, x, y, w, h);
		DrawPath(c_nWindingFillMode);
		PathCommandEnd();
		return S_OK;
	}

	// TODO:
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BeginCommand(const DWORD &lType)
{
#ifdef ENABLE_LOGS
	TELL << lType;
#endif
	m_lCurrentCommand = (long)lType;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::EndCommand(const DWORD &lType)
{
#ifdef ENABLE_LOGS
	TELL << lType;
#endif
	switch (lType)
	{
	case c_nClipType:
	{
		bool bIsIn = (c_nClipRegionTypeWinding == (0x0001 & m_lCurrentClipMode));
		m_oPath.setFillRule(bIsIn ? Qt::WindingFill : Qt::OddEvenFill);

		if (m_pContext->GetPainter()->hasClipping())
		{
			m_pContext->GetPainter()->setClipPath(m_oPath, Qt::IntersectClip);
		}
		else
		{
			m_pContext->GetPainter()->setClipPath(m_oPath, Qt::ReplaceClip);
		}
		break;
	}
	case c_nResetClipType:
	{
		m_pContext->GetPainter()->setClipping(false);
		break;
	}
	default:
		break;
	};

	m_lCurrentCommand = c_nNone;
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::PathCommandMoveTo(const double &x, const double &y)
{
#ifdef ENABLE_LOGS
	TELL << QPointF{x, y};
#endif
	if (c_nSimpleGraphicType == m_lCurrentCommand)
	{
		m_oPath.moveTo((qreal)x, (qreal)y);
	}
	else
	{
		m_oSimpleGraphicsConverter.PathCommandMoveTo(x, y);
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandLineTo(const double &x, const double &y)
{
#ifdef ENABLE_LOGS
	TELL << QPointF{x, y};
#endif
	if (c_nSimpleGraphicType == m_lCurrentCommand)
	{
		m_oPath.lineTo((qreal)x, (qreal)y);
	}
	else
	{
		m_oSimpleGraphicsConverter.PathCommandLineTo(x, y);
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandLinesTo(double *points, const int &count)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oSimpleGraphicsConverter.PathCommandLinesTo(points, (LONG)count);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandCurveTo(const double &x1
                                                    , const double &y1
                                                    , const double &x2
                                                    , const double &y2
                                                    , const double &x3
                                                    , const double &y3)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (c_nSimpleGraphicType == m_lCurrentCommand)
	{
		m_oPath.cubicTo({(qreal)x1, (qreal)y1}, {(qreal)x2, (qreal)y2}, {(qreal)x3, (qreal)y3});
	}
	else
	{
		m_oSimpleGraphicsConverter.PathCommandCurveTo(x1, y1, x2, y2, x3, y3);
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandCurvesTo(double *points, const int &count)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oSimpleGraphicsConverter.PathCommandCurvesTo(points, (LONG)count);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandArcTo(const double &x
                                                  , const double &y
                                                  , const double &w
                                                  , const double &h
                                                  , const double &startAngle
                                                  , const double &sweepAngle)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oSimpleGraphicsConverter.PathCommandArcTo(x, y, w, h, startAngle, sweepAngle);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandClose()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (c_nSimpleGraphicType == m_lCurrentCommand)
	{
		m_oPath.closeSubpath();
	}
	else
	{
		m_oSimpleGraphicsConverter.PathCommandClose();
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawPath(const LONG &nType)
{
	if (m_oPath.isEmpty())
		return S_FALSE;

	bool stroke = (0 != (nType & c_nStroke));
	bool windingFill = (0 != (nType & c_nWindingFillMode));
	bool evenOddFill = (0 != (nType & c_nEvenOddFillMode));
	bool fill = (windingFill || evenOddFill);

#ifdef ENABLE_LOGS
	QString mode;
	if (stroke) {
		mode += "stroke ";
	}
	if (windingFill) {
		mode += "winding fill ";
	}
	if (evenOddFill) {
		mode += "odd-even fill ";
	}

	TELL << mode;
#endif

	if (fill)
	{
		if (windingFill)
			m_oPath.setFillRule(Qt::WindingFill);
		else
			m_oPath.setFillRule(Qt::OddEvenFill);

		fillPath(&m_oPath);
	}
	if (stroke)
	{
		m_pContext->GetPainter()->strokePath(m_oPath, pen());
	}

	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandStart()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	// clear - only since 5.13
	//m_oPath.clear();
	m_oPath = QPainterPath();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandGetCurrentPoint(double *x, double *y)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oSimpleGraphicsConverter.PathCommandGetCurrentPoint(x, y);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandTextCHAR(const LONG& lUnicode, const double& dX, const double& dY, const double& dW, const double& dH)
{
	SetFont();
	int nCode = (int)lUnicode;
	m_oSimpleGraphicsConverter.PathCommandText2(&nCode, NULL, 1, m_pFontManager, dX, dY, dW, dH);
	return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandTextExCHAR(const LONG& lUnicode, const LONG& lGid, const double& dX, const double& dY, const double& dW, const double& dH)
{
	SetFont();
	int nCode = (int)lUnicode;
	int nGid = (int)lGid;
	m_oSimpleGraphicsConverter.PathCommandText2(&nCode, &nGid, 1, m_pFontManager, dX, dY, dW, dH);
	return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandText(const std::wstring& wsUnicodeText, const double& dX, const double& dY, const double& dW, const double& dH)
{
	SetFont();
	m_pFontManager->SetStringGID(FALSE);
	m_oSimpleGraphicsConverter.PathCommandText(wsUnicodeText, m_pFontManager, dX, dY, dW, dH, 0);
	return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandTextEx(const std::wstring& wsUnicodeText, const unsigned int* pGids, const unsigned int nGidsCount, const double& dX, const double& dY, const double& dW, const double& dH)
{
	SetFont();
	m_oSimpleGraphicsConverter.PathCommandText2(wsUnicodeText, (const int*)pGids, nGidsCount, m_pFontManager, dX, dY, dW, dH);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::AddPath(const Aggplus::CGraphicsPath& path)
{
	if (path.GetPointCount() == 0)
		return S_FALSE;

	size_t length = path.GetPointCount() + path.GetCloseCount();
	std::vector<Aggplus::PointD> points = path.GetPoints(0, length);

	for (size_t i = 0; i < length; i++)
	{
		if (path.IsCurvePoint(i))
		{
			PathCommandCurveTo(points[i].X, points[i].Y,
							   points[i + 1].X, points[i + 1].Y,
							   points[i + 2].X, points[i + 2].Y);
			i += 2;
		}
		else if (path.IsMovePoint(i))
			PathCommandMoveTo(points[i].X, points[i].Y);
		else if (path.IsLinePoint(i))
			PathCommandLineTo(points[i].X, points[i].Y);
		else
			PathCommandClose();
	}

	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandEnd()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	// clear - only since 5.13
	if (c_nSimpleGraphicType == m_lCurrentCommand)
	{
		//m_oPath.clear();
		m_oPath = QPainterPath();
	}
	else
	{
		m_oSimpleGraphicsConverter.PathCommandEnd();
	}
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawImage(IGrObject *pImage
                                           , const double &x
                                           , const double &y
                                           , const double &w
                                           , const double &h)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	QImage* pQImage = NSConversions::createTextureImage((Aggplus::CImage*)pImage);

	if (pQImage == NULL)
		return S_FALSE;

	m_pContext->GetPainter()->drawImage(QRectF(x, y, w, h), *pQImage);

	RELEASEOBJECT(pQImage);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawImageFromFile(const std::wstring &filePath
                                                   , const double &x
                                                   , const double &y
                                                   , const double &w
                                                   , const double &h
                                                   , const BYTE &lAlpha)
{
#ifdef ENABLE_LOGS
	TELL;
#endif

	std::wstring sTempPath = L"";
	CImageFileFormatChecker oImageFormat(filePath);
	if (_CXIMAGE_FORMAT_WMF == oImageFormat.eFileType ||
	    _CXIMAGE_FORMAT_EMF == oImageFormat.eFileType ||
	    _CXIMAGE_FORMAT_SVM == oImageFormat.eFileType ||
	    _CXIMAGE_FORMAT_SVG == oImageFormat.eFileType)
	{
		MetaFile::IMetaFile* pMetafile = MetaFile::Create(m_pAppFonts);
		if (pMetafile->LoadFromFile(filePath.c_str()))
		{
			sTempPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"AscMetafile_");
			pMetafile->ConvertToRaster(sTempPath.c_str(), 4, 1000, -1);
		}
		RELEASEINTERFACE(pMetafile);
	}

#if 1
	QImage* pQImage = NULL;

	if (sTempPath.empty())
	{
		pQImage = NSConversions::createTextureImage(filePath);
	}
	else
	{
		pQImage = NSConversions::createTextureImage(sTempPath);
		NSFile::CFileBinary::Remove(sTempPath);
	}

	if (pQImage == NULL)
		return S_FALSE;

	if (255 != lAlpha)
		m_pContext->GetPainter()->setOpacity((qreal)lAlpha / 255.0f);

	m_pContext->GetPainter()->drawImage(QRectF(x, y, w, h), *pQImage);

	if (255 != lAlpha)
		m_pContext->GetPainter()->setOpacity(1.0f);

	RELEASEOBJECT(pQImage);
#else
	int nWidth = 0;
	int nHeight = 0;

	QBrush* pBrush = NULL;
	if (sTempPath.empty())
	{
		pBrush = NSConversions::createTextureBrush(filePath, nWidth, nHeight);
	}
	else
	{
		pBrush = NSConversions::createTextureBrush(sTempPath, nWidth, nHeight);
		NSFile::CFileBinary::Remove(sTempPath);
	}

	if (NULL == pBrush)
		return S_FALSE;

	QPainterPath oPath;
	oPath.moveTo((qreal)x, (qreal)y);
	oPath.lineTo((qreal)(x + w), (qreal)y);
	oPath.lineTo((qreal)(x + w), (qreal)(y + h));
	oPath.lineTo((qreal)x, (qreal)(y + h));
	oPath.closeSubpath();

	if (255 != lAlpha)
		m_pContext->GetPainter()->setOpacity((qreal)lAlpha / 255.0f);

	NSConversions::correctBrushTextureTransform(NULL, pBrush, &oPath, nWidth, nHeight, this);
	m_pContext->GetPainter()->fillPath(oPath, *pBrush);

	if (255 != lAlpha)
		m_pContext->GetPainter()->setOpacity(1.0f);

	RELEASEOBJECT(pBrush);
#endif

	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::SetTransform(const double &m11
                                              , const double &m12
                                              , const double &m21
                                              , const double &m22
                                              , const double &dx
                                              , const double &dy)
{
	m_oCurrentTransform = QTransform{m11, m12, m21, m22, dx, dy};
#ifdef ENABLE_LOGS
	TELL << m_oCurrentTransform;
#endif
	applyTransform();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::GetTransform(double *m11
                                              , double *m12
                                              , double *m21
                                              , double *m22
                                              , double *dx
                                              , double *dy)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (!m11 || !m12 || !m21 || !m22 || !dx || !dy)
		return S_FALSE;

	*m11 = (double)m_oCurrentTransform.m11();
	*m12 = (double)m_oCurrentTransform.m12();
	*m21 = (double)m_oCurrentTransform.m21();
	*m22 = (double)m_oCurrentTransform.m22();
	*dx = (double)m_oCurrentTransform.dx();
	*dy = (double)m_oCurrentTransform.dy();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::ResetTransform()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oCurrentTransform.reset();
	applyTransform();
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_ClipMode(LONG *plMode)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (NULL != plMode)
	{
		*plMode = m_lCurrentClipMode;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_ClipMode(const LONG &lMode)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_lCurrentClipMode = lMode;
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandLong(const LONG &lType, const LONG &lCommand)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	if (c_nUseDictionaryFonts == lType && m_pFontManager)
		m_pFontManager->SetUseCorrentFontByName((1 == lCommand) ? true : false);
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDouble(const LONG &lType, const double &dCommand)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	// not used
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandString(const LONG &lType, const std::wstring &sCommand)
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	// not used
	return S_OK;
}

HRESULT NSQRenderer::CQRenderer::SetBaseTransform(const double& m11
                                                  , const double& m12
                                                  , const double& m21
                                                  , const double& m22
                                                  , const double& dx
                                                  , const double& dy)
{
	m_oBaseTransform = QTransform(m11, m12, m21, m22, dx, dy);
#ifdef ENABLE_LOGS
	TELL << m_oBaseTransform;
#endif
	applyTransform();
	return S_OK;
}

void NSQRenderer::CQRenderer::GetBaseTransform(double &m11
                                               , double &m12
                                               , double &m21
                                               , double &m22
                                               , double &dx
                                               , double &dy)
{
	m11 = m_oBaseTransform.m11();
	m12 = m_oBaseTransform.m12();
	m21 = m_oBaseTransform.m21();
	m22 = m_oBaseTransform.m22();
	dx = m_oBaseTransform.dx();
	dy = m_oBaseTransform.dy();
}

void NSQRenderer::CQRenderer::ResetBaseTransform()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_oBaseTransform.reset();
	applyTransform();
}

void NSQRenderer::CQRenderer::PrepareBitBlt(const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
											const double& x, const double& y, const double& w, const double& h, const double& dAngle,
											const double& tileScaleX, const double& tileScaleY)
{
	m_nPixelWidth = nRasterW;
	m_nPixelHeight = nRasterH;

	m_dTileScaleX = tileScaleX;
	m_dTileScaleY = tileScaleY;

	int nPhysicalX = 0;
	int nPhysicalY = 0;
	int nPhysicalW = 0;
	int nPhysicalH = 0;
	m_pContext->GetPhysicalRect(nPhysicalX, nPhysicalY, nPhysicalW, nPhysicalH);

	bool bIsPrintToFile = (m_pContext->getPrinter()->outputFileName().length() != 0) ? true : false;

	double dAngleDeg = dAngle * 180.0 / M_PI;
	if ((std::abs(dAngleDeg - 90) < 1.0) || (std::abs(dAngleDeg - 270) < 1.0))
	{
		float fCenterX = (float)(x + w / 2.0);
		float fCenterY = (float)(y + h / 2.0);

		QTransform oBaseTransform;

		oBaseTransform.translate(fCenterX, fCenterY);
		oBaseTransform.rotate(90);
		oBaseTransform.translate(-fCenterX, -fCenterY);

		int nAreaW = 0;
		int nAreaH = 0;
		m_pContext->GetPrintAreaSize(nAreaW, nAreaH);

		int nOldX = nPhysicalX;
		nPhysicalX = nPhysicalY;
		nPhysicalY = nPhysicalW - nAreaW - nOldX;

		if (bIsPrintToFile)
		{
			// обнуляем сдвиги, напечатается и в отрицательных местах
			nPhysicalX = 0;
			nPhysicalY = 0;
		}

		oBaseTransform.translate(x, y);

		m_oPositionTransform = oBaseTransform;
		applyTransform();
	}
	else
	{
		if (bIsPrintToFile)
		{
			// обнуляем сдвиги, напечатается и в отрицательных местах
			nPhysicalX = 0;
			nPhysicalY = 0;
		}

		m_oPositionTransform.reset();
		m_oPositionTransform.translate(x, y);
		applyTransform();
	}
}

void NSQRenderer::CQRenderer::GetTileScaleX(double& sx) const
{
	sx = m_dTileScaleX;
}

void NSQRenderer::CQRenderer::GetTileScaleY(double& sy) const
{
	sy = m_dTileScaleY;
}

QTransform& NSQRenderer::CQRenderer::GetCoordTransform()
{
	return m_oCoordTransform;
}

void NSQRenderer::CQRenderer::applyTransform()
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	m_pContext->GetPainter()->setTransform(m_oPositionTransform);
	m_pContext->GetPainter()->setTransform(m_oCoordTransform, true);
	m_pContext->GetPainter()->setTransform(m_oBaseTransform, true);
	m_pContext->GetPainter()->setTransform(m_oCurrentTransform, true);

	m_oFullTransform = m_pContext->GetPainter()->transform();
}

QSizeF NSQRenderer::CQRenderer::paperSize() const
{
#ifdef ENABLE_LOGS
	TELL;
#endif
	int x, y, w, h;
	m_pContext->GetPhysicalRect(x, y, w, h);

	return QSizeF(w, h);
}

QPen NSQRenderer::CQRenderer::pen() const
{
	QPen result;
	result.setColor(NSConversions::toColor(m_oPen.Color, m_oPen.Alpha));

	double dWidth = m_oPen.Size;
	double dWidthMinSize = 1.0 / sqrt(m_oCoordTransform.determinant());

	// если размер 0mm - то это однопиксельная линия
	if (dWidth < 0.000001)
		dWidth = dWidthMinSize;

	result.setWidthF((float)dWidth);

	// dash style
	// DashStyleSolid,          // solid
	// DashStyleDash,           // dash 3 space 1
	// DashStyleDot,            // dash 1 space 1
	// DashStyleDashDot,        // dash 3 space 1 dash 1 space 1
	// DashStyleDashDotDot,     // dash 3 space 1 dash 1 space 1 dash 1 space 1
	// DashStyleCustom          // из массива
	switch (m_oPen.DashStyle)
	{
	case (BYTE)Aggplus::DashStyle::DashStyleCustom:
	{
		int patternCount = (int)m_oPen.Count;
		if (patternCount < 1)
		{
			break;
		}
		QVector<qreal> pattern(patternCount);
		for (int i = 0; i < patternCount; ++i)
		{
			pattern[i] = (qreal)m_oPen.DashPattern[i];
		}
		result.setDashPattern(pattern);
		break;
	}
	case (BYTE)Aggplus::DashStyle::DashStyleDash:
	{
		result.setDashPattern({3, 1});
		break;
	}
	case (BYTE)Aggplus::DashStyle::DashStyleDashDot:
	{
		result.setDashPattern({3, 1, 1, 1});
		break;
	}
	case (BYTE)Aggplus::DashStyle::DashStyleDashDotDot:
	{
		result.setDashPattern({3, 1, 1, 1, 1, 1});
		break;
	}
	case (BYTE)Aggplus::DashStyle::DashStyleDot:
	{
		result.setDashPattern({1, 1});
		break;
	}
	case (BYTE)Aggplus::DashStyle::DashStyleSolid:
	default:
	{
		result.setStyle(Qt::PenStyle::SolidLine);
		break;
	}
	}

	// line cap
	switch (m_oPen.LineStartCap)
	{
	case (BYTE)Aggplus::LineCap::LineCapFlat:
	{
		result.setCapStyle(Qt::PenCapStyle::FlatCap);
		break;
	}
	case (BYTE)Aggplus::LineCap::LineCapRound:
	{
		result.setCapStyle(Qt::PenCapStyle::RoundCap);
		break;
	}
	case (BYTE)Aggplus::LineCap::LineCapSquare:
	{
		result.setCapStyle(Qt::PenCapStyle::SquareCap);
		break;
	}
	default:
	{
		result.setCapStyle(Qt::PenCapStyle::RoundCap);
		break;
	}
	}

	// line join
	switch (m_oPen.LineJoin)
	{
	case (BYTE)Aggplus::LineJoin::LineJoinMiter:
	case (BYTE)Aggplus::LineJoin::LineJoinMiterClipped:
	{
		result.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
		break;
	}
	case (BYTE)Aggplus::LineJoin::LineJoinBevel:
	{
		result.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
		break;
	}
	case (BYTE)Aggplus::LineJoin::LineJoinRound:
	{
		result.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
		break;
	}
	default:
	{
		result.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
		break;
	}
	}

	// dash offset
	result.setDashOffset((qreal)m_oPen.DashOffset);

	// miter limit
	result.setMiterLimit((qreal)m_oPen.MiterLimit);

	return result;
}

void NSQRenderer::CQRenderer::fillPath(QPainterPath* pPath)
{
	QBrush* pBrush = NULL;
	switch (m_oBrush.Type)
	{
	case c_BrushTypeSolid:
	{
		pBrush = new QBrush(Qt::SolidPattern);
		pBrush->setColor(NSConversions::toColor(m_oBrush.Color1, m_oBrush.Alpha1));
		m_pContext->GetPainter()->fillPath(m_oPath, *pBrush);
		break;
	}
	case c_BrushTypeTexture:
	{
		if (255 != m_oBrush.TextureAlpha)
			m_pContext->GetPainter()->setOpacity((qreal)m_oBrush.TextureAlpha / 255.0f);

		std::wstring sTempPath = L"";
		CImageFileFormatChecker oImageFormat(m_oBrush.TexturePath);
		if (_CXIMAGE_FORMAT_WMF == oImageFormat.eFileType ||
		    _CXIMAGE_FORMAT_EMF == oImageFormat.eFileType ||
		    _CXIMAGE_FORMAT_SVM == oImageFormat.eFileType ||
		    _CXIMAGE_FORMAT_SVG == oImageFormat.eFileType)
		{
			MetaFile::IMetaFile* pMetafile = MetaFile::Create(m_pAppFonts);
			if (pMetafile->LoadFromFile(m_oBrush.TexturePath.c_str()))
			{
				sTempPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"AscMetafile_");
				pMetafile->ConvertToRaster(sTempPath.c_str(), 4, 1000, -1);
			}
			RELEASEINTERFACE(pMetafile);
		}

		bool bIsUseBrush = true;
#ifndef QRENDERER_SUPPORT_TEXTURE_BRUSH
		if (c_BrushTextureModeStretch == m_oBrush.TextureMode)
		{
			bIsUseBrush = false;
		}
#endif

		if (bIsUseBrush)
		{
			int nImageWidth = 0;
			int nImageHeight = 0;
			if (sTempPath.empty())
			{
				pBrush = NSConversions::createTextureBrush(m_oBrush.TexturePath, nImageWidth, nImageHeight);
			}
			else
			{
				pBrush = NSConversions::createTextureBrush(sTempPath, nImageWidth, nImageHeight);
				NSFile::CFileBinary::Remove(sTempPath);
			}

			if (pBrush)
			{
				NSConversions::correctBrushTextureTransform(&m_oBrush, pBrush, pPath, nImageWidth, nImageHeight, this);
				m_pContext->GetPainter()->fillPath(m_oPath, *pBrush);
			}
		}
		else
		{
			m_pContext->GetPainter()->save();
			m_pContext->GetPainter()->setClipPath(m_oPath, Qt::IntersectClip);

			QRectF oDrawRect;
			if (m_oBrush.Rectable)
			{
				oDrawRect.setLeft(m_oBrush.Rect.X);
				oDrawRect.setTop(m_oBrush.Rect.Y);
				oDrawRect.setWidth(m_oBrush.Rect.Width);
				oDrawRect.setHeight(m_oBrush.Rect.Height);
			}
			else
			{
				oDrawRect = m_oPath.boundingRect();
			}

			QImage* pQImage = NULL;

			if (sTempPath.empty())
			{
				pQImage = NSConversions::createTextureImage(m_oBrush.TexturePath);
			}
			else
			{
				pQImage = NSConversions::createTextureImage(sTempPath);
				NSFile::CFileBinary::Remove(sTempPath);
			}

			if (pQImage != NULL)
			{
				m_pContext->GetPainter()->drawImage(oDrawRect, *pQImage);
			}

			RELEASEOBJECT(pQImage);

			m_pContext->GetPainter()->restore();
		}

		if (255 != m_oBrush.TextureAlpha)
			m_pContext->GetPainter()->setOpacity(1.0f);
	}
	default:
		break;
	}

	RELEASEOBJECT(pBrush);
}

// checker
class CQRendererChecker : public IRenderer
{
private:
	int m_nBrushType;
	int m_nBrushTextureMode;
	bool m_bIsBreak;

public:
	// own functions
	CQRendererChecker()
	{
		m_nBrushType = c_BrushTypeSolid;
		m_nBrushTextureMode = c_BrushTextureModeStretch;
		m_bIsBreak = false;
	}
	virtual ~CQRendererChecker() {}

	// тип рендерера-----------------------------------------------------------------------------
	virtual HRESULT get_Type(LONG* lType) override { return c_nQRenderer; }
	//-------- Функции для работы со страницей --------------------------------------------------
	virtual HRESULT NewPage() override { return S_OK; }
	virtual HRESULT get_Height(double* dHeight) { return S_OK; }
	virtual HRESULT put_Height(const double& dHeight) { return S_OK; }
	virtual HRESULT get_Width(double* dWidth) { return S_OK; }
	virtual HRESULT put_Width(const double& dWidth) { return S_OK; }
	virtual HRESULT get_DpiX(double* dDpiX) { return S_OK; }
	virtual HRESULT get_DpiY(double* dDpiY) { return S_OK; }

	// pen --------------------------------------------------------------------------------------
	virtual HRESULT get_PenColor(LONG* lColor) override { return S_OK; }
	virtual HRESULT put_PenColor(const LONG& lColor) override { return S_OK; }
	virtual HRESULT get_PenAlpha(LONG* lAlpha) override { return S_OK; }
	virtual HRESULT put_PenAlpha(const LONG& lAlpha) override { return S_OK; }
	virtual HRESULT get_PenSize(double* dSize) override { return S_OK; }
	virtual HRESULT put_PenSize(const double& dSize) override { return S_OK; }
	virtual HRESULT get_PenDashStyle(BYTE* val) override { return S_OK; }
	virtual HRESULT put_PenDashStyle(const BYTE& val) override { return S_OK; }
	virtual HRESULT get_PenLineStartCap(BYTE* val) override { return S_OK; }
	virtual HRESULT put_PenLineStartCap(const BYTE& val) override { return S_OK; }
	virtual HRESULT get_PenLineEndCap(BYTE* val) override { return S_OK; }
	virtual HRESULT put_PenLineEndCap(const BYTE& val) override { return S_OK; }
	virtual HRESULT get_PenLineJoin(BYTE* val) override { return S_OK; }
	virtual HRESULT put_PenLineJoin(const BYTE& val) override { return S_OK; }
	virtual HRESULT get_PenDashOffset(double* dOffset) override { return S_OK; }
	virtual HRESULT put_PenDashOffset(const double& dOffset) override { return S_OK; }
	virtual HRESULT get_PenAlign(LONG* lAlign) override { return S_OK; }
	virtual HRESULT put_PenAlign(const LONG& lAlign) override { return S_OK; }
	virtual HRESULT get_PenMiterLimit(double* dOffset) override { return S_OK; }
	virtual HRESULT put_PenMiterLimit(const double& dOffset) override { return S_OK; }
	virtual HRESULT PenDashPattern(double* pPattern, LONG lCount) override { return S_OK; }
	virtual HRESULT get_BrushTextureImage(Aggplus::CImage** pImage) override { return S_OK; }
	virtual HRESULT put_BrushTextureImage(Aggplus::CImage* pImage) override { m_bIsBreak = true; return S_OK; }
	virtual HRESULT get_BrushTransform(Aggplus::CMatrix& oMatrix) override { return S_OK; }
	virtual HRESULT put_BrushTransform(const Aggplus::CMatrix& oMatrix)	override { m_bIsBreak = true; return S_OK; }

	// brush ------------------------------------------------------------------------------------
	virtual HRESULT get_BrushType(LONG* lType) override { return S_OK; }
	virtual HRESULT put_BrushType(const LONG& lType) override { m_nBrushType = lType; return S_OK; }
	virtual HRESULT get_BrushColor1(LONG* lColor) override { return S_OK; }
	virtual HRESULT put_BrushColor1(const LONG& lColor) override { return S_OK; }
	virtual HRESULT get_BrushAlpha1(LONG* lAlpha) override { return S_OK; }
	virtual HRESULT put_BrushAlpha1(const LONG& lAlpha) override { return S_OK; }
	virtual HRESULT get_BrushColor2(LONG* lColor) override { return S_OK; }
	virtual HRESULT put_BrushColor2(const LONG& lColor) override { return S_OK; }
	virtual HRESULT get_BrushAlpha2(LONG* lAlpha) override { return S_OK; }
	virtual HRESULT put_BrushAlpha2(const LONG& lAlpha) override { return S_OK; }
	virtual HRESULT get_BrushTexturePath(std::wstring* bsPath) override { return S_OK; }
	virtual HRESULT put_BrushTexturePath(const std::wstring& bsPath) override { return S_OK; }
	virtual HRESULT get_BrushTextureMode(LONG* lMode) override { return S_OK; }
	virtual HRESULT put_BrushTextureMode(const LONG& lMode) override { m_nBrushTextureMode = lMode; return S_OK; }
	virtual HRESULT get_BrushTextureAlpha(LONG* lTxAlpha) override { return S_OK; }
	virtual HRESULT put_BrushTextureAlpha(const LONG& lTxAlpha) override { return S_OK; }
	virtual HRESULT get_BrushLinearAngle(double* dAngle) override { return S_OK; }
	virtual HRESULT put_BrushLinearAngle(const double& dAngle) override { return S_OK; }
	virtual HRESULT BrushRect(const INT& val, const double& left, const double& top, const double& width, const double& height) override { return S_OK; }
	virtual HRESULT BrushBounds(const double& left, const double& top, const double& width, const double& height) override { return S_OK; }
	virtual HRESULT put_BrushGradientColors(LONG* lColors, double* pPositions, LONG nCount) override { return S_OK; }

	// font -------------------------------------------------------------------------------------
	virtual HRESULT get_FontName(std::wstring* bsName) override { return S_OK; }
	virtual HRESULT put_FontName(const std::wstring& bsName) override { return S_OK; }
	virtual HRESULT get_FontPath(std::wstring* bsName) override { return S_OK; }
	virtual HRESULT put_FontPath(const std::wstring& bsName) override { return S_OK; }
	virtual HRESULT get_FontSize(double* dSize) override { return S_OK; }
	virtual HRESULT put_FontSize(const double& dSize) override { return S_OK; }
	virtual HRESULT get_FontStyle(LONG* lStyle) override { return S_OK; }
	virtual HRESULT put_FontStyle(const LONG& lStyle) override { return S_OK; }
	virtual HRESULT get_FontStringGID(INT* bGID) override { return S_OK; }
	virtual HRESULT put_FontStringGID(const INT& bGID) override { return S_OK; }
	virtual HRESULT get_FontCharSpace(double* dSpace) override { return S_OK; }
	virtual HRESULT put_FontCharSpace(const double& dSpace) override { return S_OK; }
	virtual HRESULT get_FontFaceIndex(int* lFaceIndex) override { return S_OK; }
	virtual HRESULT put_FontFaceIndex(const int& lFaceIndex) override { return S_OK; }

	//-------- Функции для вывода текста --------------------------------------------------------
	virtual HRESULT CommandDrawTextCHAR(const LONG& c
	                                    , const double& x
	                                    , const double& y
	                                    , const double& w
	                                    , const double& h) override { return S_OK; }
	virtual HRESULT CommandDrawText(const std::wstring& bsText
	                                , const double& x
	                                , const double& y
	                                , const double& w
	                                , const double& h) override { return S_OK; }

	virtual HRESULT CommandDrawTextExCHAR(const LONG& c
	                                      , const LONG& gid
	                                      , const double& x
	                                      , const double& y
	                                      , const double& w
	                                      , const double& h) override { return S_OK; }
	virtual HRESULT CommandDrawTextEx(const std::wstring& bsUnicodeText
	                                  , const unsigned int* pGids
	                                  , const unsigned int nGidsCount
	                                  , const double& x
	                                  , const double& y
	                                  , const double& w
	                                  , const double& h) override { return S_OK; }

	//-------- Маркеры для команд ---------------------------------------------------------------
	virtual HRESULT BeginCommand(const DWORD& lType) override { return S_OK; }
	virtual HRESULT EndCommand(const DWORD& lType) override { return S_OK; }

	//-------- Функции для работы с Graphics Path -----------------------------------------------
	virtual HRESULT PathCommandMoveTo(const double& x, const double& y) override { return S_OK; }
	virtual HRESULT PathCommandLineTo(const double& x, const double& y) override { return S_OK; }
	virtual HRESULT PathCommandLinesTo(double* points, const int& count) override { return S_OK; }
	virtual HRESULT PathCommandCurveTo(const double& x1
	                                   , const double& y1
	                                   , const double& x2
	                                   , const double& y2
	                                   , const double& x3
	                                   , const double& y3) override { return S_OK; }
	virtual HRESULT PathCommandCurvesTo(double* points, const int& count) override { return S_OK; }
	virtual HRESULT PathCommandArcTo(const double& x
	                                 , const double& y
	                                 , const double& w
	                                 , const double& h
	                                 , const double& startAngle
	                                 , const double& sweepAngle) override { return S_OK; }
	virtual HRESULT PathCommandClose() override { return S_OK; }
	virtual HRESULT PathCommandEnd() override { return S_OK; }
	virtual HRESULT DrawPath(const LONG& nType) override
	{
		if (nType & 0xFF00)
		{
			switch (m_nBrushType)
			{
			case c_BrushTypeSolid:
			{
				break;
			}
			case c_BrushTypeTexture:
			{
				switch (m_nBrushTextureMode)
				{
				case c_BrushTextureModeTile:
				case c_BrushTextureModeTileCenter:
				{
#ifndef QRENDERER_SUPPORT_TEXTURE_TILE_BRUSH
					m_bIsBreak = true;
#endif
					break;
				}
				default:
					break;
				}
				break;
			}
			default:
			{
				m_bIsBreak = true;
				break;
			}
			}
		}
		return S_OK;
	}
	virtual HRESULT PathCommandStart() override { return S_OK; }
	virtual HRESULT PathCommandGetCurrentPoint(double* x, double* y) override { return S_OK; }

	virtual HRESULT PathCommandTextCHAR(const LONG& c
	                                    , const double& x
	                                    , const double& y
	                                    , const double& w
	                                    , const double& h) override { return S_OK; }
	virtual HRESULT PathCommandText(const std::wstring& bsText
	                                , const double& x
	                                , const double& y
	                                , const double& w
	                                , const double& h) override { return S_OK; }

	virtual HRESULT PathCommandTextExCHAR(const LONG& c
	                                      , const LONG& gid
	                                      , const double& x
	                                      , const double& y
	                                      , const double& w
	                                      , const double& h) override { return S_OK; }
	virtual HRESULT PathCommandTextEx(const std::wstring& sText
	                                  , const unsigned int* pGids
	                                  , const unsigned int nGidsCount
	                                  , const double& x
	                                  , const double& y
	                                  , const double& w
	                                  , const double& h) override { return S_OK; }

	//-------- Функции для вывода изображений ---------------------------------------------------
	virtual HRESULT DrawImage(IGrObject* pImage
	                          , const double& x
	                          , const double& y
	                          , const double& w
	                          , const double& h) override { return S_OK; }
	virtual HRESULT DrawImageFromFile(const std::wstring&
	                                  , const double& x
	                                  , const double& y
	                                  , const double& w
	                                  , const double& h
	                                  , const BYTE& lAlpha = 255) override { return S_OK; }

	// transform --------------------------------------------------------------------------------
	virtual HRESULT SetCommandParams(double dAngle
	                                 , double dLeft
	                                 , double dTop
	                                 , double dWidth
	                                 , double dHeight
	                                 , DWORD lFlags) override { return S_OK; }

	virtual HRESULT SetTransform(const double& m11
	                             , const double& m12
	                             , const double& m21
	                             , const double& m22
	                             , const double& dx
	                             , const double& dy) override { return S_OK; }
	virtual HRESULT GetTransform(double *m11
	                             , double *m12
	                             , double *m21
	                             , double *m22
	                             , double *dx
	                             , double *dy)	override { return S_OK; }
	virtual HRESULT ResetTransform() override { return S_OK; }

	// -----------------------------------------------------------------------------------------
	virtual HRESULT get_ClipMode(LONG* plMode) override { return S_OK; }
	virtual HRESULT put_ClipMode(const LONG& lMode) override { return S_OK; }

	// additiaonal params ----------------------------------------------------------------------
	virtual HRESULT CommandLong(const LONG& lType, const LONG& lCommand) override { return S_OK; }
	virtual HRESULT CommandDouble(const LONG& lType, const double& dCommand) override { return S_OK; }
	virtual HRESULT CommandString(const LONG& lType, const std::wstring& sCommand) override { return S_OK; }

	virtual HRESULT IsExistAdditionalParam(const int& type)
	{
		if (c_nAdditionalParamBreak == type)
			return m_bIsBreak ? S_OK : S_FALSE;
		return S_FALSE;
	}
};

class CQRenderterCorrector : public IMetafileToRenderter
{
public:
	CQRenderterCorrector(IRenderer* pRenderer) : IMetafileToRenderter(pRenderer) {}
	virtual ~CQRenderterCorrector() { RELEASEINTERFACE(m_pRenderer); }

public:
	virtual std::wstring GetImagePath(const std::wstring& sPath) override { return L""; }
	virtual void SetLinearGradiant(const double& x0, const double& y0, const double& x1, const double& y1) override
	{
		m_pRenderer->put_BrushType(/*c_BrushTypePathGradient1*/2006);
	}
	virtual void SetRadialGradiant(const double& dX0, const double& dY0, const double& dR0, const double& dX1, const double& dY1, const double& dR1) override
	{
		m_pRenderer->put_BrushType(/*c_BrushTypePathGradient2*/2007);
	}
	virtual void InitPicker(const std::wstring& sFontsFolder) override {}
	virtual void InitPicker(NSFonts::IApplicationFonts* pFonts) override {}
};

IMetafileToRenderter* NSQRenderer::CQRenderer::GetChecker()
{
	CQRendererChecker* pChecker = new CQRendererChecker();
	return new CQRenderterCorrector(pChecker);
}
