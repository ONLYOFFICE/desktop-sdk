#ifndef Q_FOOTER_PANEL_H
#define Q_FOOTER_PANEL_H

#include <QWidget>

#include "src/videoplayerlib_export.h"

class QAscVideoView;
class QFooterPanel_Private;
class VIDEO_LIB_EXPORT QFooterPanel : public QWidget
{
	Q_OBJECT

	friend class QAscVideoView;

public:
	enum SkinType
	{
		stLight,
		stDark
	};

public:
	QFooterPanel(QWidget* parent, QAscVideoView* pView);
	~QFooterPanel();

public:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void paintEvent(QPaintEvent* event);

public:
	inline int GetHeight();
	inline int GetMinWidth();
	inline int GetMaxWidth();

	inline QWidget* VolumeControls();

	void ApplySkin(SkinType skin);

public slots:
	void onPlayPauseBtnClicked();
	void onVolumeBtnClicked();
	void onFullscreenBtnClicked();
	void onPlaylistBtnClicked();

	void onSeekChanged(int nValue);
	void onVolumeChanged(int nValue);

private:
	void SetPlayPauseIcon(bool bIsPlay);
	void SetFullscreenIcon(bool bIsFullscreen);

private:
	QFooterPanel_Private* m_pInternal;
};

#endif	// Q_FOOTER_PANEL_H
