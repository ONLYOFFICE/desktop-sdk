#ifndef QFOOTERPANEL_H
#define QFOOTERPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QSlider>
#include "qvideoslider.h"

class QFooterPanel : public QWidget
{
	Q_OBJECT
public:
	explicit QFooterPanel(QWidget* parent, QWidget* pVideoView = nullptr);

	virtual void resizeEvent(QResizeEvent* e);

	virtual void paintEvent(QPaintEvent *);

	virtual void mouseMoveEvent(QMouseEvent* event);

public:
	QPushButton* m_pPlayPause;
	QPushButton* m_pVolume;
	QPushButton* m_pFullscreen;
	QPushButton* m_pPlaylist;

	QVideoSlider* m_pSlider;

	QWidget* m_pVideoView;

	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;

	// constants
	const int c_nHeight = 40;

	const int c_nVolumeControlWidth = 60;
	const int c_nVolumeControlHeight = 160;

	const int c_nVolumeSliderX = 15;
	const int c_nVolumeSliderY = 20;
	const int c_nVolumeSliderWidth = 30;
	const int c_nVolumeSliderHeight = 120;

public:
	void SetPlayPauseIcon(bool bIsPlay);
	void SetFullscreenIcon(bool bIsFullscreen);
};

#endif // QFOOTERPANEL_H
