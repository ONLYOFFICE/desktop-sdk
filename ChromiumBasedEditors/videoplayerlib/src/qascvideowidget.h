#pragma once

#include <QWidget>
#include "lib_base.h"

#include <QVideoWidget>
#include <QMediaPlayer>
#include "qfooterpanel.h"
#include "qvideoplaylist.h"
#include "qascmediaplayer.h"

#ifndef USE_VLC_LIBRARY
#define QASCVIDEOBASE QVideoWidget
#else
#define QASCVIDEOBASE QWidget
#endif

class QAscVideoWidget;
class QAscVideoView_Private
{
public:
	QFooterPanel*   m_pFooter;

	QWidget*        m_pVolumeControl;
	QVideoSlider*   m_pVolumeControlV;

	QVideoPlaylist*     m_pPlaylist;
	QAscVideoWidget*    m_pPlayer;

	bool m_bIsShowingPlaylist;
	bool m_bIsPlay;
	bool m_bIsSeekEnabled;

	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;

	bool m_bIsPresentationMode;
	bool m_bIsPresentationModeMediaTypeSended;

	bool m_bIsDestroy;
};

class QAscVideoWidget : public QASCVIDEOBASE
{
	Q_OBJECT

	QString m_sCurrentSource;

#ifdef USE_VLC_LIBRARY
	CVlcPlayer* m_pVlcPlayer;
	CVlcMedia* m_pMedia;
#else
	QMediaPlayer* m_pEngine;
#endif

	int m_nVolume;

public:
	QWidget* m_pParent;

public:
	QAscVideoWidget(QWidget *parent = 0);
	~QAscVideoWidget();

public:
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

#if defined(_LINUX) && !defined(_MAC)
	virtual void mouseMoveEvent(QMouseEvent* e);
#endif

public:
	void open(QString& sFile);

	void setPlay();
	void setPause();
	void setVolume(int nVolume);
	void setSeek(int nPos);

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
	void slotVlcTimeChanged(qint64 time);
#else
	void slotChangeState(QMediaPlayer::State state);
	void slotPositionChange(qint64 pos);
#endif

public:
	QWidget* m_pView;
};
