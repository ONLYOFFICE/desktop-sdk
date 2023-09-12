/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "../qascvideoview.h"
#include "qascvideowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWindow>
#include <QScreen>

#ifdef ASC_MAIN_WINDOW_HAS_QWINDOW_AS_PARENT
#define USE_ASC_MAINWINDOW_FULLSCREEN
#endif

#ifndef USE_ASC_MAINWINDOW_FULLSCREEN
#ifndef USE_VLC_LIBRARY_VIDEO
#define USE_ASC_MAINWINDOW_FULLSCREEN
#endif
#endif

#include <QDebug>

QAscVideoWidget::QAscVideoWidget(QWidget *parent)
	: QASCVIDEOBASE(parent)
{
	m_pParent = parent;
	m_nVolume = 50;
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	/*
	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	setPalette(p);

	setAttribute(Qt::WA_OpaquePaintEvent);
	*/

#ifndef USE_VLC_LIBRARY
	m_pEngine = new QMediaPlayer(parent);
	m_pEngine->setVideoOutput(this);

	QObject::connect(m_pEngine, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slotChangeState(QMediaPlayer::State)));
	QObject::connect(m_pEngine, SIGNAL(positionChanged(qint64)), this, SLOT(slotPositionChange(qint64)));
#else
	m_pVlcPlayer = new CVlcPlayer(this);
	m_pVlcPlayer->integrateIntoWidget(this);

	QObject::connect(m_pVlcPlayer, SIGNAL(stateChanged(int)), this, SLOT(slotVlcStateChanged(int)));
	QObject::connect(m_pVlcPlayer, SIGNAL(timeChanged(qint64)), this, SLOT(slotVlcTimeChanged(qint64)));

	m_pMedia = nullptr;
#endif
}

QAscVideoWidget::~QAscVideoWidget()
{
#ifdef USE_VLC_LIBRARY
	m_pVlcPlayer->stop();
	if (m_pMedia)
		delete m_pMedia;
#endif
}

void QAscVideoWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && isVideoFullScreen())
	{
		setFullScreenOnCurrentScreen(false);
		event->accept();
	}
	else if ((((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) && event->modifiers() & Qt::AltModifier) ||
			 (event->key() == Qt::Key_F))
	{
		setFullScreenOnCurrentScreen(!isVideoFullScreen());
		event->accept();
	}
	else
	{
		QASCVIDEOBASE::keyPressEvent(event);
	}
}

void QAscVideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		QAscVideoView* pView = (QAscVideoView*)(m_pParent->parentWidget());
		if (!pView->m_pInternal->m_bIsPresentationMode)
			setFullScreenOnCurrentScreen(!isVideoFullScreen());
	}
	event->accept();
}

void QAscVideoWidget::mousePressEvent(QMouseEvent *event)
{
	QASCVIDEOBASE::mousePressEvent(event);
}

//#include <QApplication>
void QAscVideoWidget::mouseMoveEvent(QMouseEvent* event)
{
	QAscVideoView* pView = static_cast<QAscVideoView*>(m_pParent->parentWidget());
	if (pView->getMainWindowFullScreen())
	{
		if (!pView->m_pInternal->m_pVolumeControl->isVisible() && !pView->m_pInternal->m_bIsShowingPlaylist)
		{
			pView->setCursor(Qt::ArrowCursor);
			// start (or restart) cursor hiding timer
			pView->m_pInternal->m_oCursorTimer.start(pView->m_pInternal->c_nCursorHidingDelay);
			// start footer hiding timer if it is not started
			if (pView->m_pInternal->m_bIsShowingFooter && !pView->m_pInternal->m_oFooterTimer.isActive())
				pView->m_pInternal->m_oFooterTimer.start(pView->m_pInternal->c_nFooterHidingDelay);
		}
	}
	event->accept();
//	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void QAscVideoWidget::setPlay()
{
	if (m_sCurrentSource.isEmpty())
	{
		QAscVideoView* pView = (QAscVideoView*)(m_pParent->parentWidget());
		pView->m_pInternal->m_pPlaylist->PlayCurrent();
		return;
	}

#ifndef USE_VLC_LIBRARY
	m_pEngine->play();
#else
	m_pVlcPlayer->play();
#endif
}

void QAscVideoWidget::setPause()
{
#ifndef USE_VLC_LIBRARY
	m_pEngine->pause();
#else
	m_pVlcPlayer->pause();
#endif
}

void QAscVideoWidget::setVolume(int nVolume)
{
	m_nVolume = nVolume;
#ifndef USE_VLC_LIBRARY
	QMediaPlayer_setVolume(m_pEngine, nVolume);
#else
	m_pVlcPlayer->setVolume(nVolume * 2);
#endif
}

void QAscVideoWidget::setSeek(int nPos)
{
#ifndef USE_VLC_LIBRARY
	qint64 nDuration = m_pEngine->duration();
	double dProgress = (double)nPos / 100000.0;
	m_pEngine->setPosition((qint64)(dProgress * nDuration));
#else
	libvlc_time_t nDuration = m_pMedia ? m_pMedia->duration() : 0;
	double dProgress = (double)nPos / 100000.0;
	m_pVlcPlayer->setTime((int)(dProgress * nDuration));
#endif
}

void QAscVideoWidget::open(QString& sFile)
{
	m_sCurrentSource = sFile;
#ifndef USE_VLC_LIBRARY
	QMediaPlayer_setMedia(m_pEngine, sFile);
	m_pEngine->play();
#else

	if (!m_pMedia && !sFile.isEmpty())
	{
		delete m_pMedia;
		m_pMedia = nullptr;
	}

	if (sFile.isEmpty())
	{
		m_pVlcPlayer->stop();
		return;
	}

	m_pMedia = new CVlcMedia(reinterpret_cast<libvlc_instance_t*>(NSBaseVideoLibrary::GetLibrary()), sFile);
	m_pVlcPlayer->open(m_pMedia);
#endif
}

bool QAscVideoWidget::isVideoFullScreen()
{
#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
	return ((QAscVideoView*)(m_pParent->parentWidget()))->getMainWindowFullScreen();
#else
	return isFullScreen();
#endif
}

void QAscVideoWidget::setFullScreenOnCurrentScreen(bool isFullscreen)
{
	if (isFullscreen == isVideoFullScreen())
		return;

	QAscVideoView* pView = static_cast<QAscVideoView*>(m_pParent->parentWidget());
#ifndef USE_VLC_LIBRARY

#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
	((QAscVideoView*)(m_pParent->parentWidget()))->setMainWindowFullScreen(isFullscreen);
#else
	setFullScreen(isFullscreen);
#endif

#else
	if (pView->m_pInternal->m_bIsShowingPlaylist)
		pView->Playlist(0);
	if (!pView->m_pInternal->m_pVolumeControl->isHidden())
		pView->m_pInternal->m_pVolumeControl->setHidden(true);

	if (isFullscreen)
	{
#ifdef USE_VLC_LIBRARY_VIDEO
		((QVideoWidget*)m_pParent)->setFullScreen(true);
#endif

#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
		pView->setMainWindowFullScreen(true);
#else
		QPoint pt = mapToGlobal(pos());
		QRect rect = QApplication::desktop()->screenGeometry(m_pParent);

		this->setParent(NULL);
		this->showFullScreen();
		this->setGeometry(rect);
#endif
	}
	else
	{
#ifdef USE_VLC_LIBRARY_VIDEO
		((QVideoWidget*)m_pParent)->setFullScreen(false);
		m_pParent->lower();
#endif

#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
		pView->setMainWindowFullScreen(false);
#else
		this->setParent(m_pParent);
		showNormal();
		this->lower();
#endif
	}
#endif

	pView->UpdateFullscreenIcon();
	if (!isFullscreen)
	{
		pView->resizeEvent(nullptr);
	}
}

#ifdef USE_VLC_LIBRARY
void QAscVideoWidget::slotVlcStateChanged(int state)
{
	int stateQ = -1;

	if (state == libvlc_Playing)
	{
		stateQ = QMediaPlayer::PlayingState;
		setVolume(m_nVolume);
	}
	else if (state == libvlc_Paused)
	{
		stateQ = QMediaPlayer::PausedState;
	}
	else if (state == libvlc_Ended)
	{
		stateQ = QMediaPlayer::StoppedState;
	}

	if (stateQ < 0)
		return;

	emit stateChanged((QMediaPlayer_State)stateQ);
}

void QAscVideoWidget::slotVlcTimeChanged(qint64 time)
{
	libvlc_time_t nDuration = m_pMedia->duration();
	double dProgress = (double)time / nDuration;
	emit posChanged((int)(100000 * dProgress + 0.5));
}
#else

void QAscVideoWidget::slotChangeState(QMediaPlayer_State state)
{
	if (QMediaPlayer::PlayingState == state)
		setVolume(m_nVolume);

	emit stateChanged(state);
}

void QAscVideoWidget::slotPositionChange(qint64 pos)
{
	qint64 nDuration = m_pEngine->duration();
	double dProgress = (double)pos / nDuration;
	emit posChanged((int)(100000 * dProgress + 0.5));
}

QMediaPlayer* QAscVideoWidget::getEngine()
{
	return m_pEngine;
}
#endif

bool QAscVideoWidget::isAudio()
{
#ifdef USE_VLC_LIBRARY
	if (!m_pVlcPlayer || !m_pMedia)
		return true;

	return m_pVlcPlayer->isAudio();
#else
	if (m_pEngine && !QMediaPlayer_isAudio(m_pEngine))
		return false;
	return true;
#endif    
}

void QAscVideoWidget::stop()
{
#ifdef USE_VLC_LIBRARY
	if (m_pVlcPlayer)
		m_pVlcPlayer->stop();
#else
	if (m_pEngine)
		m_pEngine->stop();
#endif
}
