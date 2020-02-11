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

#ifndef QASCPRINTER_H
#define QASCPRINTER_H

#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include <QPaintEngine>
#include <qmath.h>

#include "./applicationmanager_events.h"

#ifdef WIN32
#include <signal.h>
#include <iostream>
typedef void (*SignalHandlerPointer)(int);
#endif

class QAscPrinterContext : public NSEditorApi::CAscPrinterContextBase
{
private:
    QPrinter m_oPrinter;
    QPainter m_oPainter;
    bool m_bIsUsePainter;

public:
    QAscPrinterContext(QPrinter::PrinterMode eMode = QPrinter::HighResolution) : NSEditorApi::CAscPrinterContextBase(), m_oPrinter(eMode)
    {
        m_bIsUsePainter = false;
    }
    QAscPrinterContext(const QPrinterInfo& pi, QPrinter::PrinterMode eMode = QPrinter::HighResolution) : NSEditorApi::CAscPrinterContextBase(),
        m_oPrinter(pi, eMode), m_bIsUsePainter(false)
    {
    }

    bool BeginPaint()
    {
        m_bIsUsePainter = true;
        //m_oPrinter.setFullPage(true);

#ifdef WIN32
        SignalHandlerPointer previousHandler;
        previousHandler = signal(SIGSEGV, [](int signum){
            std::cout << "Signal " << signum << std::endl;
            throw "!Access Violation!";
        });
#endif
        try
        {
            m_oPainter.begin(&m_oPrinter);
        }
        catch (...)
        {
            return false;
        }
        return true;
    }
    void EndPaint()
    {
        if (m_bIsUsePainter)
            m_oPainter.end();
        m_bIsUsePainter = false;
    }

    QPrinter* getPrinter()
    {
        return &m_oPrinter;
    }

    virtual ~QAscPrinterContext()
    {
    }

    virtual void GetLogicalDPI(int& nDpiX, int& nDpiY)
    {
        nDpiX = m_oPrinter.logicalDpiX();
        nDpiY = m_oPrinter.logicalDpiY();
    }

    virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH)
    {
        QRect rect1 = m_oPrinter.pageRect();
        QRect rect2 = m_oPrinter.paperRect();

        nX = rect1.x();
        nY = rect1.y();
        nW = rect2.width();
        nH = rect2.height();
    }

    virtual void GetPrintAreaSize(int& nW, int& nH)
    {
        QRect rect = m_oPrinter.pageRect();
        nW = rect.width();
        nH = rect.height();
    }

    virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
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

private:
    void DrawImage(QPainter* painter, const QImage& image, const QRect& rect, const QRect& rectSrc)
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
};

#endif  // QASCPRINTER_H
