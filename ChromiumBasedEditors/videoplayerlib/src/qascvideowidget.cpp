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

#include "qascvideowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWindow>
#include <QScreen>

#include "../qascvideoview.h"

#ifdef ASC_MAIN_WINDOW_HAS_QWINDOW_AS_PARENT
#define USE_ASC_MAINWINDOW_FULLSCREEN
#endif

#ifndef USE_ASC_MAINWINDOW_FULLSCREEN
#ifndef USE_VLC_LIBRARY_VIDEO
#define USE_ASC_MAINWINDOW_FULLSCREEN
#endif
#endif

QAscVideoWidget::QAscVideoWidget(QWidget *parent)
	: QASCVIDEOBASE(parent)
{
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
#ifdef USE_QVIDEO_ITEM
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QGraphicsScene* pScene = new QGraphicsScene(this);
	this->setScene(pScene);
	QGraphicsVideoItem* pItem = new QGraphicsVideoItem();
	pScene->addItem(pItem);
	m_pEngine->setVideoOutput(pItem);
#else
	m_pEngine->setVideoOutput(this);
#endif

#ifndef USE_VLC_LIBRARY
#ifndef QT_VERSION_6
	m_pEngine->setNotifyInterval(500);
#endif
#else
	m_pEngine->setNotifyInterval(500);
#endif


	QObject::connect(m_pEngine, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slotChangeState(QMediaPlayer::State)));
	QObject::connect(m_pEngine, SIGNAL(positionChanged(qint64)), this, SLOT(slotPositionChange(qint64)));
	QObject::connect(m_pEngine, SIGNAL(videoAvailableChanged(bool)), this, SLOT(slotVideoAvailableChanged(bool)));
	QObject::connect(m_pEngine, SIGNAL(durationChanged(qint64)), this, SLOT(slotMediaDurationParsed(qint64)));
	QObject::connect(m_pEngine, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));
#else
	m_pVlcPlayer = new CVlcPlayer(this);
	m_pVlcPlayer->integrateIntoWidget(this);

	QObject::connect(m_pVlcPlayer, SIGNAL(stateChanged(int)), this, SLOT(slotVlcStateChanged(int)));
	QObject::connect(m_pVlcPlayer, SIGNAL(positionChanged(float)), this, SLOT(slotVlcPositionChanged(float)));
	QObject::connect(m_pVlcPlayer, SIGNAL(videoOutputChanged(int)), this, SLOT(slotVlcVideoOutputChanged(int)));

	m_pMedia = nullptr;
#endif
	m_bIsMediaPreloading = false;
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
		if (!m_pView->m_pInternal->m_bIsPresentationMode)
			setFullScreenOnCurrentScreen(!isVideoFullScreen());
	}
	event->accept();
}

void QAscVideoWidget::mousePressEvent(QMouseEvent *event)
{
	QASCVIDEOBASE::mousePressEvent(event);
}

void QAscVideoWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_pView->getMainWindowFullScreen())
	{
		if (!m_pView->Footer()->VolumeControls()->isVisible() && !m_pView->m_pInternal->m_bIsShowingPlaylist)
		{
			m_pView->setCursor(Qt::ArrowCursor);
			// start (or restart) cursor hiding timer
			m_pView->m_pInternal->m_oCursorTimer.start(m_pView->m_pInternal->c_nCursorHidingDelay);
			// start footer hiding timer if it is not started
			if (m_pView->m_pInternal->m_bIsShowingFooter && !m_pView->m_pInternal->m_oFooterTimer.isActive())
				m_pView->m_pInternal->m_oFooterTimer.start(m_pView->m_pInternal->c_nFooterHidingDelay);
		}
	}
	event->accept();
}

#ifdef USE_QVIDEO_ITEM
void QAscVideoWidget::resizeEvent(QResizeEvent* event)
{
	QRect rcontent = contentsRect();
	setSceneRect(0, 0, rcontent.width(), rcontent.height());

	this->scene()->items()[0]->setPos(0, 0);
	((QGraphicsVideoItem*)this->scene()->items()[0])->setSize(QSizeF(width(), height()));
}
#endif

void QAscVideoWidget::setPlay()
{
	preloadMediaIfNeeded();

#ifndef USE_VLC_LIBRARY
	// `play()` slot is called syncronously so there is no need for queueing it
	m_pEngine->play();
#else
	if (m_bIsMediaPreloading)
	{
		// play media after it is loaded
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this]() {
			m_pVlcPlayer->play();
		});
	}
	else
	{
		m_pVlcPlayer->play();
	}
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
	preloadMediaIfNeeded();

#ifndef USE_VLC_LIBRARY
	if (m_bIsMediaPreloading)
	{
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nPos]() {
			setSeek(nPos);
		});
	}
	else
	{
		qint64 nDuration = m_pEngine->duration();
		double dProgress = (double)nPos / 100000.0;
		m_pEngine->setPosition((qint64)(dProgress * nDuration));
	}
#else
	if (m_bIsMediaPreloading)
	{
		// set position of playback after media is loaded
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nPos]() {
			m_pVlcPlayer->setPosition((float)nPos / 100000);
		});
	}
	else
	{
		m_pVlcPlayer->setPosition((float)nPos / 100000);
	}
#endif
}

void QAscVideoWidget::stepBack(int nStep)
{
	preloadMediaIfNeeded();

#ifndef USE_VLC_LIBRARY
	if (m_bIsMediaPreloading)
	{
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nStep]() {
			stepBack(nStep);
		});
	}
	else
	{
		qint64 curPos = getPlayerState(m_pEngine) == QMediaPlayer::StoppedState ? 0 : m_pEngine->position();
		qint64 targetPos = curPos - nStep;
		if (targetPos < 0)
			targetPos = 0;
		m_pEngine->setPosition(targetPos);
	}
#else
	if (m_bIsMediaPreloading)
	{
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nStep]() {
			stepBack(nStep);
		});
	}
	else
	{
		qint64 targetPos = m_pVlcPlayer->time() - nStep;
		qint64 duration = m_pMedia->duration();
		if (targetPos < 0)
			targetPos = 0;
		m_pVlcPlayer->setPosition((float)targetPos / duration);
	}
#endif
}

void QAscVideoWidget::stepForward(int nStep)
{
	preloadMediaIfNeeded();

#ifndef USE_VLC_LIBRARY
	if (m_bIsMediaPreloading)
	{
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nStep]() {
			stepForward(nStep);
		});
	}
	else
	{
		qint64 curPos = getPlayerState(m_pEngine) == QMediaPlayer::StoppedState ? 0 : m_pEngine->position();
		qint64 targetPos = curPos + nStep;
		qint64 duration = m_pEngine->duration();
		if (targetPos >= duration)
			targetPos = duration - 1;
		m_pEngine->setPosition(targetPos);
	}
#else
	if (m_bIsMediaPreloading)
	{
		std::lock_guard<std::mutex> cs(m_oMutex);
		m_oPreloadCmdQueue.push([this, nStep]() {
			stepForward(nStep);
		});
	}
	else
	{
		qint64 targetPos = m_pVlcPlayer->time() + nStep;
		qint64 duration = m_pMedia->duration();
		if (targetPos >= duration)
			targetPos = duration - 1;
		m_pVlcPlayer->setPosition((float)targetPos / duration);
	}
#endif
}

void QAscVideoWidget::open(QString& sFile, bool isPlay)
{
	m_sCurrentSource = sFile;

#ifndef USE_VLC_LIBRARY
	QMediaPlayer_setMedia(m_pEngine, sFile);
	// if `isPlay` is false, then start and immediately pause playback to load video frames
	m_pEngine->play();
	if (!isPlay)
		m_pEngine->pause();
#else
	if (m_pMedia && !sFile.isEmpty())
	{
		delete m_pMedia;
		m_pMedia = nullptr;
	}

	if (sFile.isEmpty())
	{
		m_pVlcPlayer->stop();
		return;
	}

	m_pMedia = new CVlcMedia(GetVlcInstance(), sFile);
	m_pVlcPlayer->open(m_pMedia);
#endif
}

bool QAscVideoWidget::isVideoFullScreen()
{
#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
	return m_pView->getMainWindowFullScreen();
#else
	return isFullScreen();
#endif
}

void QAscVideoWidget::setFullScreenOnCurrentScreen(bool isFullscreen)
{
	if (isFullscreen == isVideoFullScreen())
		return;

	if (m_pView->m_pInternal->m_bIsShowingPlaylist)
		m_pView->TogglePlaylist(0);
	if (!m_pView->Footer()->VolumeControls()->isHidden())
		m_pView->Footer()->VolumeControls()->setHidden(true);

#ifndef USE_VLC_LIBRARY

#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
	m_pView->setMainWindowFullScreen(isFullscreen);
#else
	setFullScreen(isFullscreen);
#endif

#else

	if (isFullscreen)
	{
#ifdef USE_VLC_LIBRARY_VIDEO
		((QVideoWidget*)m_pParent)->setFullScreen(true);
#endif

#ifdef USE_ASC_MAINWINDOW_FULLSCREEN
		m_pView->setMainWindowFullScreen(true);
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
		m_pView->setMainWindowFullScreen(false);
#else
		this->setParent(m_pParent);
		showNormal();
		this->lower();
#endif
	}
#endif

	m_pView->UpdateFullscreenIcon();
	if (!isFullscreen)
	{
		m_pView->resizeEvent(nullptr);
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
		// if media is preloading, then start and immediately pause playback to load video frames
		if (m_bIsMediaPreloading)
		{
			m_pVlcPlayer->pause();
		}
	}
	else if (state == libvlc_Paused)
	{
		stateQ = QMediaPlayer::PausedState;

		if (m_bIsMediaPreloading)
		{
			m_bIsMediaPreloading = false;
			// call functors from command queue
			std::lock_guard<std::mutex> cs(m_oMutex);
			while (!m_oPreloadCmdQueue.empty())
			{
				m_oPreloadCmdQueue.front()();
				m_oPreloadCmdQueue.pop();
			}
		}
	}
	else if (state == libvlc_Ended || state == libvlc_Stopped)
	{
		stateQ = QMediaPlayer::StoppedState;
		m_sCurrentSource = "";
	}

	if (stateQ < 0)
		return;

	emit stateChanged((QMediaPlayer_State)stateQ);
}

void QAscVideoWidget::slotVlcPositionChanged(float position)
{
	if (m_pMedia)
	{
		m_pView->Footer()->setTimeOnLabel(position * m_pMedia->duration());

		emit posChanged(static_cast<int>(100000 * position + 0.5));
	}
}

void QAscVideoWidget::slotVlcVideoOutputChanged(int nVoutCount)
{
	emit videoOutputChanged((bool)nVoutCount);
}
#else

void QAscVideoWidget::slotChangeState(QMediaPlayer_State state)
{
	if (state == QMediaPlayer::PlayingState)
	{
		setVolume(m_nVolume);
	}
	else if (state == QMediaPlayer::StoppedState)
	{
		m_sCurrentSource = "";
	}

	emit stateChanged(state);
}

void QAscVideoWidget::slotPositionChange(qint64 pos)
{
	m_pView->Footer()->setTimeOnLabel(pos);

	qint64 nDuration = m_pEngine->duration();
	if (nDuration == 0)
		return;

	double dProgress = (double)pos / nDuration;
	emit posChanged((int)(100000 * dProgress + 0.5));
}

void QAscVideoWidget::slotVideoAvailableChanged(bool isAvailable)
{
	emit videoOutputChanged(isAvailable);
}

void QAscVideoWidget::slotMediaDurationParsed(qint64 duration)
{
	if (m_bIsMediaPreloading)
	{
		m_bIsMediaPreloading = false;
		// call functors from command queue
		std::lock_guard<std::mutex> cs(m_oMutex);
		while (!m_oPreloadCmdQueue.empty())
		{
			m_oPreloadCmdQueue.front()();
			m_oPreloadCmdQueue.pop();
		}
	}
}

void QAscVideoWidget::slotMediaStatusChanged(QMediaPlayer::MediaStatus mediaStatus)
{
	if (mediaStatus == QMediaPlayer::LoadedMedia)
		emit videoOutputChanged(isVideoAvailable(m_pEngine));
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

void QAscVideoWidget::preloadMediaIfNeeded()
{
	if (m_sCurrentSource.isEmpty() && !m_bIsMediaPreloading)
	{
		m_bIsMediaPreloading = true;
		m_pView->m_pInternal->m_pPlaylist->LoadCurrent();
#ifndef USE_VLC_LIBRARY
		if (m_pEngine->duration())
			m_bIsMediaPreloading = false;
#endif
	}
}
