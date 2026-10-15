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

#include "./../include/qdpichecker.h"

#include <QScreen>
#include <QApplication>
#include "./../include/qcefview.h"

#ifdef QT_VERSION_LESS_5_15
#include <QDesktopWidget>
#endif

#ifdef _LINUX
#ifndef QT_VERSION_6
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif
#endif

QDpiChecker::QDpiChecker(CAscApplicationManager* pManager) : CAscDpiChecker(pManager)
{
}

int QDpiChecker::GetWindowDpi(WindowHandleId wid, unsigned int* dx, unsigned int* dy)
{
	double dForceScale = GetForceScale(dx, dy);
	if (dForceScale > 0)
		return 0;
	return CAscDpiChecker::GetWindowDpi(wid, dx, dy);
}

int QDpiChecker::GetMonitorDpi(int nScreenNumber, unsigned int* dx, unsigned int* dy)
{
	double dForceScale = GetForceScale(dx, dy);
	if (dForceScale > 0)
		return 0;

	int nBaseRet = CAscDpiChecker::GetMonitorDpi(nScreenNumber, dx, dy);
	if (-1 != nBaseRet)
		return nBaseRet;

	QScreen * _screen;
	if (nScreenNumber >=  0 && nScreenNumber < QApplication::screens().count())
		_screen = QApplication::screens().at(nScreenNumber);
	else {
		nScreenNumber = 0;
		_screen = QApplication::primaryScreen();
	}

    int nDpiX = _screen->logicalDotsPerInchX();
    int nDpiY = _screen->logicalDotsPerInchY();

// #ifdef _LINUX
// 	if ( QX11Info::isPlatformX11() )
// 	{
// 		int _x11_dpix = QX11Info::appDpiX(nScreenNumber),
// 				_x11_dpiy = QX11Info::appDpiY(nScreenNumber);

// 		if ( nDpiX < _x11_dpix ) nDpiX = _x11_dpix;
// 		if ( nDpiY < _x11_dpiy ) nDpiY = _x11_dpiy;
// 	}
// #endif

// 	QSize size = _screen->size();
// 	if (size.width() <= 1600 && size.height() <= 900)
// 	{
// 		nDpiX = 96;
// 		nDpiY = 96;
// 	}

// 	if (nDpiX > 150 && nDpiX < 180 && nDpiY > 150 && nDpiY < 180 && size.width() >= 3840 && size.height() >= 2160)
// 	{
// 		nDpiX = 192;
// 		nDpiY = 192;
// 	}

	*dx = nDpiX;
	*dy = nDpiY;

	return 0;
}

// app realize
int QDpiChecker::GetWidgetImplDpi(CCefViewWidgetImpl* w, unsigned int* dx, unsigned int* dy)
{
	return GetWidgetDpi((QCefView*)w, dx, dy);
}

int QDpiChecker::GetWidgetDpi(QWidget* w, unsigned int* dx, unsigned int* dy)
{
	double dForceScale = GetForceScale(dx, dy);
	if (dForceScale > 0)
		return 0;

	if (0 == QApplication::screens().count())
	{
		*dx = 96;
		*dy = 96;
		return 0;
	}
#ifndef QT_VERSION_LESS_5_15
	int nScreenNumber = QApplication::screens().indexOf(w->screen());
#else
	int nScreenNumber = QApplication::desktop()->screenNumber(w);
#endif
	return GetMonitorDpi(nScreenNumber, dx, dy);
}
