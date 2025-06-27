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

// uncomment to watch debug info
// #define DEBUG_MEDIA_PLAYER

#ifdef DEBUG_MEDIA_PLAYER
#include <iostream>
#endif

QCefView_Media::QCefView_Media(QWidget* parent, const QSize& size) : QCefView(parent, size)
{
	m_pMediaView = NULL;
}

QCefView_Media::~QCefView_Media()
{
	OnMediaEnd();
}

#ifdef _LINUX
#include <QX11Info>
QWidget* getMainPanel(QWidget* widget, int& x, int& y)
{
	x = 0; y = 0;
	if (!QX11Info::isCompositingManagerRunning())
		return widget;

	QWidget* cur = widget;
	while (cur->parentWidget() && cur->objectName() != "mainPanel")
	{
		if (cur->objectName() == "ascTabWidget")
		{
			x += cur->x();
			y += cur->y();
		}
		cur = cur->parentWidget();
	}

	// presenter view
	x += widget->x();
	y += widget->y();

	if (cur)
		return cur;

	return widget;
}
#else
QWidget* getMainPanel(QWidget* cur, int& x, int& y)
{
    x = 0; y = 0;
    return cur;
}
#endif

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
	// panel and video widget commands
	if (sCmd == "showMediaControl")
	{
		showMediaControl(data);
	}
	else if (sCmd == "hideMediaControl")
	{
		hideMediaControl();
	}
	else if (sCmd == "update")
	{
		updateGeometry(data);
	}
	// player commands
	else
	{
		// handle the possible case when player is not yet initialized
		if (m_pMediaView == nullptr)
		{
#ifdef DEBUG_MEDIA_PLAYER
			std::cerr << "Requested player command \"" << sCmd << "\" on an unitialized player." << std::endl;
#endif
			return;
		}
		// handle player commands
		if (sCmd == "play" || sCmd == "resume")
		{
			m_pMediaView->Play();
		}
		else if (sCmd == "pause")
		{
			m_pMediaView->Pause();
		}
		else if (sCmd == "stop")
		{
			m_pMediaView->Stop();
		}
		else if (sCmd == "togglePause")
		{
			m_pMediaView->TogglePause();
		}
		else
		{
#ifdef DEBUG_MEDIA_PLAYER
			std::cerr << "Media player command \"" << sCmd << "\" can not be handled." << std::endl;
#endif
		}
	}

#ifdef DEBUG_MEDIA_PLAYER
	std::string indent = "";
	std::cout << "MediaPlayerData: {" << std::endl;
	indent = "  ";

	std::cout << indent << "Cmd: " << sCmd << "," << std::endl;

	if (sCmd != "hideMediaControl")
	{
		std::cout << indent << "FrameRect: {" << std::endl;
		indent = "    ";
		std::cout << indent << "X: " << data->get_FrameRectX() << "," << std::endl;
		std::cout << indent << "Y: " << data->get_FrameRectY() << "," << std::endl;
		std::cout << indent << "W: " << data->get_FrameRectW() << "," << std::endl;
		std::cout << indent << "H: " << data->get_FrameRectH() << "," << std::endl;
		indent = "  ";
		std::cout << indent << "}," << std::endl;

		std::cout << indent << "FrameBounds: {" << std::endl;
		indent = "    ";
		std::cout << indent << "X: " << data->get_FrameBoundsX() << "," << std::endl;
		std::cout << indent << "Y: " << data->get_FrameBoundsY() << "," << std::endl;
		std::cout << indent << "W: " << data->get_FrameBoundsW() << "," << std::endl;
		std::cout << indent << "H: " << data->get_FrameBoundsH() << "," << std::endl;
		indent = "  ";
		std::cout << indent << "}," << std::endl;

		std::cout << indent << "ControlRect: {" << std::endl;
		indent = "    ";
		std::cout << indent << "X: " << data->get_ControlRectX() << "," << std::endl;
		std::cout << indent << "Y: " << data->get_ControlRectY() << "," << std::endl;
		std::cout << indent << "W: " << data->get_ControlRectW() << "," << std::endl;
		std::cout << indent << "H: " << data->get_ControlRectH() << "," << std::endl;
		indent = "  ";
		std::cout << indent << "}," << std::endl;

		std::cout << indent << "IsSelected: " << std::boolalpha << data->get_IsSelected() << "," << std::endl;

		std::cout << indent << "Rotation: " << data->get_Rotation() << "," << std::endl;

		std::cout << indent << "FlipH: " << std::boolalpha << data->get_FlipH() << "," << std::endl;
		std::cout << indent << "FlipV: " << std::boolalpha << data->get_FlipV() << "," << std::endl;

		std::wcout << "  MediaFile: \"" << data->get_Url() << "\"," << std::endl;

		std::cout << indent << "Fullscreen: " << std::boolalpha << data->get_Fullscreen() << "," << std::endl;
		std::cout << indent << "IsVideo: " << std::boolalpha << data->get_IsVideo() << "," << std::endl;
		std::cout << indent << "Mute: " << std::boolalpha << data->get_Mute() << "," << std::endl;

		std::cout << indent << "Volume: " << data->get_Volume() << "," << std::endl;

		std::cout << indent << "StartTime: " << data->get_StartTime() << "," << std::endl;
		std::cout << indent << "EndTime: " << data->get_EndTime() << "," << std::endl;
		std::cout << indent << "From: " << data->get_From() << "," << std::endl;

		std::cout << indent << "Theme: " << data->get_Theme() << "," << std::endl;

		indent = "";
	}

	std::cout << "}\n" << std::endl;
#endif
}

void QCefView_Media::showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	if (m_pMediaView)
		return;

	// if media is not selected don't do anything
	if (!data->get_IsSelected())
		return;

	// set DPI for this widget BEFORE creating media view
	QWidgetUtils::SetDPI(this, m_pCefView->GetDeviceScale());

	int xOffset = 0, yOffset = 0;
	m_pMediaView = new QAscVideoView(getMainPanel(this, xOffset, yOffset), true);

	// NOTE: uncomment if you want to handle key events from media player
	// QObject::connect(m_pMediaView, SIGNAL(onKeyDown(int,Qt::KeyboardModifiers)),
	// 				 this, SLOT(onMediaKeyDown(int,Qt::KeyboardModifiers)));

	m_pMediaView->setPlayListUsed(false);
	m_pMediaView->setFullScreenUsed(false);

	m_pMediaView->setGeometry(xOffset + data->get_FrameBoundsX(), yOffset + data->get_FrameBoundsY(), data->get_FrameBoundsW(), data->get_FrameBoundsH());

	QFooterPanel* pFooter = m_pMediaView->Footer();
	pFooter->setGeometry(xOffset + data->get_ControlRectX(), yOffset + data->get_ControlRectY(), data->get_ControlRectW(), pFooter->GetHeight());

	// show before skin! (problem in fullscreen mode)
	pFooter->show();
	std::string sTheme = data->get_Theme();
	if (sTheme == "dark")
		pFooter->ApplySkin(CFooterSkin::tDark);
	else
		pFooter->ApplySkin(CFooterSkin::tLight);
	pFooter->SetRoundedCorners();

	m_pMediaView->setMedia(QString::fromStdWString(data->get_Url()), false);

	int nVolume = data->get_Volume();
	if (data->get_Mute() || m_pCefView->IsPresentationReporter())
	{
		m_pMediaView->ToggleMute();
	}
	else if (nVolume != -1)
	{
		m_pMediaView->ChangeVolume(nVolume);
	}
}

void QCefView_Media::hideMediaControl()
{
	if (!m_pMediaView)
		return;

	m_pMediaView->RemoveFromPresentation();
	m_pMediaView = nullptr;
}

void QCefView_Media::updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	if (!m_pMediaView)
		return;

	int xOffset = 0, yOffset = 0;
	getMainPanel(this, xOffset, yOffset);
	m_pMediaView->setGeometry(xOffset + data->get_FrameBoundsX(), yOffset + data->get_FrameBoundsY(), data->get_FrameBoundsW(), data->get_FrameBoundsH());
	QFooterPanel* pFooter = m_pMediaView->Footer();
	pFooter->setGeometry(xOffset + data->get_ControlRectX(), yOffset + data->get_ControlRectY(), data->get_ControlRectW(), pFooter->GetHeight());
}

void QCefView_Media::onMediaKeyDown(int key, Qt::KeyboardModifiers mods)
{
	// NOTE: here can be handled some key events while media is playing
	// switch (key)
	// {
	// default:
	// 	break;
	// }
}
