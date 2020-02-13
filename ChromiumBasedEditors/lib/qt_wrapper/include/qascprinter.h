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

#include "./../../include/base.h"
#include "./../../include/applicationmanager_events.h"

class DESKTOP_DECL QAscPrinterContext : public NSEditorApi::CAscPrinterContextBase
{
private:
    QPrinter m_oPrinter;
    QPainter m_oPainter;
    bool m_bIsUsePainter;

public:
    QAscPrinterContext(QPrinter::PrinterMode eMode = QPrinter::HighResolution);
    QAscPrinterContext(const QPrinterInfo& pi, QPrinter::PrinterMode eMode = QPrinter::HighResolution);
    virtual ~QAscPrinterContext();

    bool BeginPaint();
    void EndPaint();

    QPrinter* getPrinter();

    virtual void GetLogicalDPI(int& nDpiX, int& nDpiY);

    virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH);

    virtual void GetPrintAreaSize(int& nW, int& nH);

    virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
                        const double& x, const double& y, const double& w, const double& h, const double& dAngle);

private:
    void DrawImage(QPainter* painter, const QImage& image, const QRect& rect, const QRect& rectSrc);
};

#endif  // QASCPRINTER_H
