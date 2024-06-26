#pragma once

#include <QWidget>
#include <QTimer>
#include "lib_base.h"

#include "qmultimedia.h"
#include "../qfooterpanel.h"
#include "qvideoplaylist.h"

#include <functional>
#include <queue>
#include <mutex>

#ifndef USE_VLC_LIBRARY
#define USE_QVIDEO_ITEM
#ifndef USE_QVIDEO_ITEM
#define QASCVIDEOBASE QVideoWidget
#else
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#define QASCVIDEOBASE QGraphicsView
#endif
#else
#define QASCVIDEOBASE QWidget
#endif

class QAscVideoWidget;
class QAscVideoView_Private
{
public:
	QFooterPanel*	m_pFooter;

	QVideoPlaylist*		m_pPlaylist;
	QAscVideoWidget*	m_pPlayer;

	QTimer				m_oFooterTimer;
	QTimer				m_oCursorTimer;

	bool m_bIsShowingPlaylist;
	bool m_bIsShowingFooter;
	bool m_bIsPlay;
	bool m_bIsSeekEnabled;

	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;

	bool m_bIsPresentationMode;
	bool m_bIsPresentationModeMediaTypeSended;

	bool m_bIsDestroy;

	bool m_bIsMuted;
	int m_nMutedVolume;

	// constants
	const int c_nFooterHidingDelay = 2000;
	const int c_nCursorHidingDelay = 3000;
};

class QAscVideoView;
class QAscVideoWidget : public QASCVIDEOBASE
{
	Q_OBJECT

public:
	QAscVideoWidget(QWidget *parent = 0);
	~QAscVideoWidget();

public:
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);

#ifdef USE_QVIDEO_ITEM
	void resizeEvent(QResizeEvent* event);
#endif

public:
	void open(QString& sFile, bool isPlay);

	void setPlay();
	void setPause();
	void setVolume(int nVolume);
	void setSeek(int nPos);

	void stepBack(int nStep = 5000);
	void stepForward(int nStep = 5000);

	bool isVideoFullScreen();
	void setFullScreenOnCurrentScreen(bool isFullscreen);

#ifndef USE_VLC_LIBRARY
	QMediaPlayer* getEngine();
#endif

	bool isAudio();
	void stop();

private:
	// if there is no media playing, preloads the current one
	void preloadMediaIfNeeded();

signals:
	void stateChanged(QMediaPlayer_State);
	void posChanged(int);
	void videoOutputChanged(bool isVideoAvailable);

public slots:
#ifdef USE_VLC_LIBRARY
	void slotVlcStateChanged(int state);
	void slotVlcPositionChanged(float position);
	void slotVlcVideoOutputChanged(int nVoutCount);
#else
	void slotChangeState(QMediaPlayer::State state);
	void slotPositionChange(qint64 pos);
	void slotVideoAvailableChanged(bool isAvailable);
	void slotMediaDurationParsed(qint64 duration);
#endif

public:
	QAscVideoView* m_pView;
	int m_nVolume;

private:
	QString m_sCurrentSource;

	bool m_bIsMediaPreloading;
	std::queue<std::function<void()>> m_oPreloadCmdQueue;
	std::mutex m_oMutex;

#ifdef USE_VLC_LIBRARY
	CVlcPlayer* m_pVlcPlayer;
	CVlcMedia* m_pMedia;
#else
	QMediaPlayer* m_pEngine;
#endif
};
