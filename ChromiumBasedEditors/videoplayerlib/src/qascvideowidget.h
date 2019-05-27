#pragma once

#include <QWidget>
#include "lib_base.h"

#include <QVideoWidget>
#include <QMediaPlayer>
#include "qfooterpanel.h"
#include "qvideoplaylist.h"

#ifndef USE_VLC_LIBRARY
#define QASCVIDEOBASE QVideoWidget
#else
#define QASCVIDEOBASE VlcWidgetVideo
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

    QMediaPlayer* m_pEngine;

#ifdef USE_VLC_LIBRARY
    VlcMediaPlayer* m_pVlcPlayer;
    VlcMedia* m_pMedia;
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

    QMediaPlayer* getEngine();

    bool isAudio();
    void stop();

signals:
    void stateChanged(QMediaPlayer::State);
    void posChanged(int);

public slots:
    void slotChangeState(QMediaPlayer::State state);
    void slotPositionChange(qint64 pos);    

#ifdef USE_VLC_LIBRARY
    void slotVlcStateChanged();
    void slotVlcTimeChanged(int time);
#endif

public:
    QWidget* m_pView;
};
