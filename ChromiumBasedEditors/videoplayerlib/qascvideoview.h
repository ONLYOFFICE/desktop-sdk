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
#include "./src/qmultimedia.h"

#if defined(BUILD_VIDEO_LIBRARY)
#  define VIDEO_LIB_EXPORT Q_DECL_EXPORT
#else
#  define VIDEO_LIB_EXPORT Q_DECL_IMPORT
#endif

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
	explicit QAscVideoView(QWidget *parent, int r, int g, int b, bool bIsPresentationMode = false);
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
	void PlayPause();
	void ToggleMute();
	void Volume();
	void Fullscreen();
	void Playlist(double duration = 100);	// toggle playlist display
	void Footer(double duration = 150);		// toggle footer display
	void SavePlayListAddons(const QString& sAddon);

	int GetFooterHeight();
	void SetFooterGeometry(int ax, int ay, int aw, int ah);

	void AddFilesToPlaylist(QStringList& files, const bool isStart = false);
	void LoadPlaylist();
	void SavePlaylist();

	void setPlayListUsed(bool isUsed);
	void setFullScreenUsed(bool isUsed);
	void setMedia(QString sMedia);

	void Stop();
	void RemoveFromPresentation();

	void UpdatePlayPauseIcon();
	void UpdateFullscreenIcon();

signals:
	void titleChanged(const QString& sTitle);

public slots:
	void slotPlayPause();
	void slotVolume();
	void slotFullscreen();
	void slotPlaylist();
	void slotVolumeChanged(int nValue);
	void slotSeekChanged(int nValue);

	void slotOpenFile(QString sFile);

	void slotPlayerPosChanged(int nPos);
	void slotPlayerStateChanged(QMediaPlayer_State state);
	void slotVideoAvailableChanged(bool videoAvailable);

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
