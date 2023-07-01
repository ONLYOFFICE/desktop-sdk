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
#include "./../include/qascprinter.h"
#include "./../qrenderer/include/qrenderer.h"
#include <qmath.h>

#ifdef WIN32
#include <signal.h>
#include <iostream>
typedef void (*SignalHandlerPointer)(int);
#endif

#include "../../../../../core/DesktopEditor/graphics/MetafileToRenderer.h"

QAscPrinterContext::QAscPrinterContext(QPrinter::PrinterMode eMode) : NSEditorApi::CAscPrinterContextBase(), m_oPrinter(eMode)
{
	m_bIsUsePainter = false;
	setDefaults();
}
QAscPrinterContext::QAscPrinterContext(const QPrinterInfo& pi, QPrinter::PrinterMode eMode) : NSEditorApi::CAscPrinterContextBase(),
	m_oPrinter(pi, eMode), m_bIsUsePainter(false)
{
	setDefaults();
}

bool QAscPrinterContext::BeginPaint()
{
	m_bIsUsePainter = true;
	//m_oPrinter.setFullPage(true);
	bool bResult = true;

#ifdef WIN32
	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGSEGV, [](int signum){
		std::cout << "Signal " << signum << std::endl;
		throw "!Access Violation!";
	});
#endif
	try
	{
		if (!m_pDevice)
		{
			m_oPrinter.setPageMargins(QMarginsF(0,0,0,0), QPageLayout::Millimeter);
			bResult = m_oPainter.begin(&m_oPrinter);
		}
		else
		{
			bResult = m_oPainter.begin(m_pDevice);

			if (pdtSimple == m_eDeviceType)
				m_oPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		}
	}
	catch (...)
	{
		return false;
	}
	return bResult;
}
void QAscPrinterContext::EndPaint()
{
	if (m_bIsUsePainter)
		m_oPainter.end();
	m_bIsUsePainter = false;
}

QPrinter* QAscPrinterContext::getPrinter()
{
	return &m_oPrinter;
}

QAscPrinterContext::~QAscPrinterContext()
{
}

void QAscPrinterContext::GetLogicalDPI(int& nDpiX, int& nDpiY)
{
	if (m_pDevice)
	{
		nDpiX = m_pDevice->logicalDpiX();
		nDpiY = m_pDevice->logicalDpiY();
		return;
	}

	nDpiX = m_oPrinter.logicalDpiX();
	nDpiY = m_oPrinter.logicalDpiY();
}

void QAscPrinterContext::GetPhysicalRect(int& nX, int& nY, int& nW, int& nH)
{
	if (m_pDevice)
	{
		nX = 0;
		nY = 0;
		nW = m_pDevice->width();
		nH = m_pDevice->height();
		return;
	}

#ifndef QT_VERSION_6
	QRect rect1 = m_oPrinter.pageRect();
	QRect rect2 = m_oPrinter.paperRect();
#else
	QRect rect1 = m_oPrinter.pageLayout().fullRectPixels(m_oPrinter.resolution());
	QRect rect2 = m_oPrinter.pageLayout().paintRectPixels(m_oPrinter.resolution());
#endif

	nX = rect1.x();
	nY = rect1.y();
	nW = rect2.width();
	nH = rect2.height();
}

void QAscPrinterContext::GetPrintAreaSize(int& nW, int& nH)
{
	if (m_pDevice)
	{
		nW = m_pDevice->width();
		nH = m_pDevice->height();
		return;
	}

#ifndef QT_VERSION_6
	QRect rect = m_oPrinter.pageRect();
#else
	QRect rect = m_oPrinter.pageLayout().fullRectPixels(m_oPrinter.resolution());
#endif

	nW = rect.width();
	nH = rect.height();
}

void QAscPrinterContext::BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
								const double& x, const double& y, const double& w, const double& h, const double& dAngle)
{
	int nPhysicalX = 0;
	int nPhysicalY = 0;
	int nPhysicalW = 0;
	int nPhysicalH = 0;
	this->GetPhysicalRect(nPhysicalX, nPhysicalY, nPhysicalW, nPhysicalH);

	//m_oPrinter.setFullPage(true);

	// TODO: must be faster!!! (create CBgraFrame as QPixmap???)
	//QPainter painter(&m_oPrinter);

	QPainter* painter = NULL;

	if (m_bIsUsePainter)
		painter = &m_oPainter;
	else
	{
		painter = new QPainter();
		painter->begin(&m_oPrinter);
	}

	painter->save();

	bool bIsPrintToFile = (m_oPrinter.outputFileName().length() != 0);

	bool bIsClip = painter->hasClipping();
	painter->setClipping(false);

	//QImage oImage(pBGRA + 4 * ((nRasterH - 1) * nRasterW), nRasterW, nRasterH, -4 * nRasterW, QImage::Format_ARGB32);
	QImage oImage(pBGRA, nRasterW, nRasterH, 4 * nRasterW, QImage::Format_ARGB32);

	double dAngleDeg = dAngle * 180.0 / M_PI;
	if ((std::abs(dAngleDeg - 90) < 1.0) || (std::abs(dAngleDeg - 270) < 1.0))
	{
		//painter->translate(-nPhysicalX, -nPhysicalY);

		float fCenterX = (float)(x + w / 2.0);
		float fCenterY = (float)(y + h / 2.0);

		painter->translate(fCenterX, fCenterY);
		painter->rotate(90);
		painter->translate(-fCenterX, -fCenterY);

		int nWDst = nRasterW;
		int nHDst = nRasterH;

		int nAreaW = 0;
		int nAreaH = 0;
		this->GetPrintAreaSize(nAreaW, nAreaH);

		int nOldX = nPhysicalX;
		nPhysicalX = nPhysicalY;
		nPhysicalY = nPhysicalW - nAreaW - nOldX;

		if (bIsPrintToFile)
		{
			// обнуляем сдвиги, напечатается и в отрицательных местах
			nPhysicalX = 0;
			nPhysicalY = 0;
		}

		nWDst -= nPhysicalX;
		nHDst -= nPhysicalY;
		QRect rect((int)x + nPhysicalX, (int)y + nPhysicalY, nWDst, nHDst);
		QRect rectSrc(nPhysicalX, nPhysicalY, nWDst, nHDst);

		this->DrawImage(painter, oImage, rect, rectSrc);
	}
	else
	{
		int nWDst = nRasterW;
		int nHDst = nRasterH;

		if (bIsPrintToFile)
		{
			// обнуляем сдвиги, напечатается и в отрицательных местах
			nPhysicalX = 0;
			nPhysicalY = 0;
		}

		nWDst -= nPhysicalX;
		nHDst -= nPhysicalY;
		QRect rect((int)x + nPhysicalX, (int)y + nPhysicalY, nWDst, nHDst);
		QRect rectSrc(nPhysicalX, nPhysicalY, nWDst, nHDst);

		this->DrawImage(painter, oImage, rect, rectSrc);
	}

	painter->restore();

	if (!m_bIsUsePainter)
	{
		painter->end();
		delete painter;
	}

	//m_oPrinter.setFullPage(false);
}

void QAscPrinterContext::PrepareBitBlt(void* pRenderer, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
									   const double& x, const double& y, const double& w, const double& h, const double& dAngle)
{
	((NSQRenderer::CQRenderer*)pRenderer)->PrepareBitBlt(nRasterX, nRasterY, nRasterW, nRasterH,
														 x, y, w, h, dAngle);
}

void QAscPrinterContext::DrawImage(QPainter* painter, const QImage& image, const QRect& rect, const QRect& rectSrc)
{
#if 0
	// нельзя просто печатать весь растр целиком, так как некоторые принтеры
	// не пропускают столько информации.
	painter->drawImage(rect, image, rectSrc);
#endif

	int nHDst = rect.height();
	int nMaxHeight = 100;
	int nCurHeight = 0;
	while (nCurHeight < nHDst)
	{
		int nRowH = nHDst - nCurHeight;
		if (nRowH > nMaxHeight)
			nRowH = nMaxHeight;

		QRect _rect = rect;
		_rect.setY(_rect.y() + nCurHeight);
		_rect.setHeight(nRowH);
		QRect _rectSrc = rectSrc;
		_rectSrc.setY(_rectSrc.y() + nCurHeight);
		_rectSrc.setHeight(nRowH);

		painter->drawImage(_rect, image, _rectSrc);

		nCurHeight += nRowH;
	}
}

void QAscPrinterContext::setDefaults()
{
	QPrinterInfo oInfo(m_oPrinter);
	m_oPrinter.setDuplex(oInfo.defaultDuplexMode());
	m_oPrinter.setPageSize(oInfo.defaultPageSize());

	m_pDevice = NULL;
	m_eDeviceType = pdtNone;
}

void* QAscPrinterContext::GetNativeRenderer()
{
	return new NSQRenderer::CQRenderer(this);
}
void* QAscPrinterContext::GetNativeRendererUnsupportChecker()
{
	return NSQRenderer::CQRenderer::GetChecker();
}

// not desktop common
QAscPrinterContext::QAscPrinterContext(QPaintDevice *pDevice) : m_oPrinter(QPrinter::PrinterResolution)
{
	m_pDevice = pDevice;
	m_eDeviceType = pdtSimple;
}
QAscPrinterContext::QAscPrinterContext(QPagedPaintDevice *pDevice) : m_oPrinter(QPrinter::PrinterResolution)
{
	m_pDevice = pDevice;
	m_eDeviceType = pdtPaged;
}
QPainter* QAscPrinterContext::GetPainter()
{
	return &m_oPainter;
}

void QAscPrinterContext::NewPage()
{
	if (m_pDevice && pdtPaged == m_eDeviceType)
		((QPagedPaintDevice*)m_pDevice)->newPage();
}

void QAscPrinterContext::InitRenderer(void* pRenderer, void* pFontManager)
{
	if (NULL == pRenderer)
		return;
	NSQRenderer::CQRenderer* pQRenderer = (NSQRenderer::CQRenderer*)pRenderer;
	if (NULL != pFontManager)
	{
		pQRenderer->SetFontsManager((NSFonts::IFontManager*)pFontManager);
	}
}

void QAscPrinterContext::SetPageOrientation(int nOrientaion)
{
	// отключаем это, из-за бага
	// https://bugzilla.onlyoffice.com/show_bug.cgi?id=59263
	// в QT ориентрацию нужно создавать ДО newPage(), но так как картинки отправляются отложенно(?)
	// то смена ориентации влияет на печать картинок (клиппируется по странице в новой ориентации).
	// похоже на баг в  QT. При переходе на новую версию  QT - пересмотреть.

	//m_oPrinter.setPageOrientation((0 == nOrientaion) ? QPageLayout::Portrait : QPageLayout::Landscape);
}

void QAscPrinterContext::SaveState()
{
	if (m_bIsUsePainter)
		m_oPainter.save();
}
void QAscPrinterContext::RestoreState()
{
	if (m_bIsUsePainter)
		m_oPainter.restore();
}
