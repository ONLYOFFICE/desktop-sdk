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

#include "./../include/qcefview_media.h"
#include <QTimer>

#include <algorithm>

#include <QDebug>

QCefView_Media::QCefView_Media(QWidget* parent, const QSize& size) : QCefView(parent, size)
{
	m_pMediaView = NULL;
}

QCefView_Media::~QCefView_Media()
{
	OnMediaEnd();
}

void QCefView_Media::OnMediaStart(NSEditorApi::CAscExternalMedia* data)
{
	if (m_pMediaView)
		return;

	// set DPI for this widget BEFORE creating media view
	QWidgetUtils::SetDPI(this, m_pCefView->GetDeviceScale());

	m_pMediaView = new QAscVideoView(this, true);
	m_pMediaView->setPlayListUsed(false);
	m_pMediaView->setFullScreenUsed(false);

	// set video view geometry
	int nWidth = data->get_BoundsW();
	m_pMediaView->setGeometry(data->get_BoundsX(), data->get_BoundsY(), nWidth, data->get_BoundsH());
	// set footer geometry
	QFooterPanel* pFooter = m_pMediaView->Footer();
	int nFooterWidth = std::min(std::max(nWidth, pFooter->GetMinWidth()), pFooter->GetMaxWidth());
	int nFooterX = data->get_BoundsX() + nWidth / 2 - nFooterWidth / 2;
	pFooter->setGeometry(nFooterX, data->get_BoundsY() + data->get_BoundsH() + 12, nFooterWidth, pFooter->GetHeight());
	pFooter->SetRoundedCorners(true);
	// set footer skin and show it explicitly
	pFooter->ApplySkin(CFooterSkin::tDark);
	pFooter->show();

	if (m_pCefView && m_pCefView->IsPresentationReporter())
		m_pMediaView->ToggleMute();

	// for vlc:
#if defined(_WIN32) && defined(USE_VLC_LIBRARY)
	m_pMediaView->show();
#endif

	m_pMediaView->setMedia(QString::fromStdWString(data->get_Url()));
}

void QCefView_Media::OnMediaEnd(bool isFromResize)
{
	if (!m_pMediaView)
		return;

	m_pMediaView->RemoveFromPresentation();
	m_pMediaView = nullptr;

	if (isFromResize)
	{
		// под виндоус проблемы с ресайзом.
		QTimer::singleShot(100, this, [this]() {
			this->resizeEvent(nullptr);
		});
	}
}

void QCefView_Media::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	std::string sCmd = data->get_Cmd();

	if (sCmd == "showMediaControl")
	{
		showMediaControl(data);
	}
	else if (sCmd == "hideMediaControl")
	{
		hideMediaControl();
	}
	else
	{
		qDebug() << "Command " << QString::fromStdString("\"" + sCmd + "\"") << "can not be handled.";
	}
}

void QCefView_Media::showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	if (m_pMediaView)
		return;

	// set DPI for this widget BEFORE creating media view
	QWidgetUtils::SetDPI(this, m_pCefView->GetDeviceScale());

	m_pMediaView = new QAscVideoView(this, true);
	m_pMediaView->setPlayListUsed(false);
	m_pMediaView->setFullScreenUsed(false);

	// m_pMediaView->setGeometry(100, 100, 100, 100);

	QFooterPanel* pFooter = m_pMediaView->Footer();
	pFooter->setGeometry(data->get_ControlRectX(), data->get_ControlRectY(), pFooter->GetMinWidth(), pFooter->GetHeight());

	pFooter->ApplySkin(CFooterSkin::tDark);
	pFooter->show();
}

void QCefView_Media::hideMediaControl()
{
	if (!m_pMediaView)
		return;

	m_pMediaView->RemoveFromPresentation();
	m_pMediaView = nullptr;
}
