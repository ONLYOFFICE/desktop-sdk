#ifndef QASCIMAGEVIEW_H
#define QASCIMAGEVIEW_H

#include <QWidget>
#include <QLabel>

#include <QDir>
#include <QFileInfo>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QPropertyAnimation>
#include <QtCore/QtGlobal>

#include "src/videoplayerlib_export.h"
#include "src/qmultimedia.h"
#include "qfooterpanel.h"
#include "qwidgetutils.h"

namespace NSBaseVideoLibrary
{
	void Init(QObject* parent);
	VIDEO_LIB_EXPORT void SetVerbosityLevel(int nVerbose);
}

class QAscVideoView_Private;
class VIDEO_LIB_EXPORT QAscVideoView : public QWidget
{
	Q_OBJECT
public:
	explicit QAscVideoView(QWidget *parent, bool bIsPresentationMode = false);
	virtual ~QAscVideoView();

public:
	virtual void resizeEvent(QResizeEvent* e);

	virtual void paintEvent(QPaintEvent *);

	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

	void mousePressEvent(QMouseEvent *event);

	void keyPressEvent(QKeyEvent *event);
	bool eventFilter(QObject *watched, QEvent *event);

	virtual bool getMainWindowFullScreen();
	virtual void setMainWindowFullScreen(bool bIsFullScreen);
	virtual QWidget* getMainWindow();

public:
	void Play();
	void Pause();
	void TogglePause();
	void ChangeVolume(int nValue);
	void ChangeSeek(int nValue);
	void ToggleMute();
	void Fullscreen();
	void TogglePlaylist(double duration = 100);
	void ToggleFooter(double duration = 150);
	void SavePlayListAddons(const QString& sAddon);

	QFooterPanel* Footer();

	void AddFilesToPlaylist(QStringList& files, const bool isStart = false);
	void LoadPlaylist();
	void SavePlaylist();

	void setPlayListUsed(bool isUsed);
	void setFullScreenUsed(bool isUsed);
	void setMedia(QString sMedia, bool isStart = true);

	void Stop();
	void RemoveFromPresentation();

	void UpdatePlayPauseIcon();
	void UpdateFullscreenIcon();

signals:
	void titleChanged(const QString& sTitle);
	void onKeyDown(int key, Qt::KeyboardModifiers mods);

public slots:
	void slotOpenFile(QString sFile, bool isPlay);

	void slotPlayerPosChanged(int nPos);
	void slotPlayerStateChanged(QMediaPlayer_State state);
	void slotVideoAvailableChanged(bool isVideoAvailable);

	void slotFooterAnimationFinished();
	void slotFooterTimerOverflowed();
	void slotCursorTimerOverflowed();

private:
	QPropertyAnimation* m_pAnimationPlaylist;
	QPropertyAnimation* m_pAnimationFooter;

public:
	QAscVideoView_Private* m_pInternal;
};

#endif // QASCIMAGEVIEW_H
