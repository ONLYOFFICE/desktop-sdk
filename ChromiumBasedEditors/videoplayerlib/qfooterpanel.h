#ifndef Q_FOOTER_PANEL_H
#define Q_FOOTER_PANEL_H

#include <QWidget>

#include "src/videoplayerlib_export.h"
#include "src/style/skins_footer.h"

class QAscVideoView;
class QFooterPanel_Private;
class VIDEO_LIB_EXPORT QFooterPanel : public QWidget
{
	Q_OBJECT

	friend class QAscVideoView;

public:
	QFooterPanel(QWidget* parent, QAscVideoView* pView);
	~QFooterPanel();

public:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void paintEvent(QPaintEvent* event);

public:
	int GetHeight();
	int GetMinWidth();
	int GetMaxWidth();

	QWidget* VolumeControls();

	void ApplySkin(CFooterSkin::Type type);
	void SetRoundedCorners(bool isRounded = true);

	void updateStyle();
	void setStyleOptions(const CFooterStyleOptions& opt);

	void setTimeOnLabel(qint64 time);

public slots:
	void onPlayPauseBtnClicked();
	void onVolumeBtnClicked();
	void onFullscreenBtnClicked();
	void onPlaylistBtnClicked();
	void onRewindBackBtnClicked();
	void onRewindForwardBtnClicked();

	void onSeekChanged(int nValue);
	void onVolumeChanged(int nValue);

private:
	void SetPlayPauseIcon(bool bIsPlay);
	void SetFullscreenIcon(bool bIsFullscreen);

private:
	QFooterPanel_Private* m_pInternal;
};

#endif	// Q_FOOTER_PANEL_H
