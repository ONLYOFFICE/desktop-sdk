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
	explicit QFooterPanel(QWidget *parent = nullptr, bool bIsPresentationMode = false);

	virtual void resizeEvent(QResizeEvent* e);

	virtual void paintEvent(QPaintEvent *);

	virtual void mouseMoveEvent(QMouseEvent* event);

public:
	QPushButton* m_pPlayPause;
	QPushButton* m_pVolume;
	QPushButton* m_pFullscreen;
	QPushButton* m_pPlaylist;

	QVideoSlider* m_pSlider;

	int m_nHeigth;

	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;
	bool m_bIsPresentationMode;

public:
	void SetPlayPauseIcon(bool bIsPlay);
	void SetFullscreenIcon(bool bIsFullscreen);
};

#endif // QFOOTERPANEL_H
