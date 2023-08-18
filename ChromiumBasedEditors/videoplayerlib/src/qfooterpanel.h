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
	explicit QFooterPanel(QWidget *parent = 0);

	virtual void resizeEvent(QResizeEvent* e);

	virtual void paintEvent(QPaintEvent *);

#if defined(_LINUX) && !defined(_MAC)
	virtual void mouseMoveEvent(QMouseEvent* e);
#endif

public:
	QPushButton* m_pPlayPause;
	QPushButton* m_pVolume;
	QPushButton* m_pFullscreen;
	QPushButton* m_pPlaylist;

	QVideoSlider* m_pSlider;

	int m_nHeigth;

	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;

public:
	void SetPlayPause(bool bIsPlay);
};

#endif // QFOOTERPANEL_H
