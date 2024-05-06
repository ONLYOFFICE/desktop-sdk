#pragma once

#include <QWidget>
#include <QTimer>
#include "lib_base.h"

#include "qmultimedia.h"
#include "../qfooterpanel.h"
#include "qvideoplaylist.h"

#ifndef USE_VLC_LIBRARY
#define QASCVIDEOBASE QVideoWidget
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

public:
	void open(QString& sFile);

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

signals:
	void stateChanged(QMediaPlayer_State);
	void posChanged(int);

public slots:
#ifdef USE_VLC_LIBRARY
	void slotVlcStateChanged(int state);
	void slotVlcPositionChanged(float position);
#else
	void slotChangeState(QMediaPlayer::State state);
	void slotPositionChange(qint64 pos);
#endif

public:
	QAscVideoView* m_pView;
	int m_nVolume;

private:
	QString m_sCurrentSource;

#ifdef USE_VLC_LIBRARY
	CVlcPlayer* m_pVlcPlayer;
	CVlcMedia* m_pMedia;
#else
	QMediaPlayer* m_pEngine;
#endif
};
