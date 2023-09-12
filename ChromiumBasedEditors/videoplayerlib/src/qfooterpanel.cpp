#include "qfooterpanel.h"
#include <QStyleOption>
#include <QPainter>
#include "qpushbutton_icons.h"
#include "qascvideoview.h"
#include "qascvideowidget.h"

#include <QDebug>

QFooterPanel::QFooterPanel(QWidget *parent) : QWidget(parent)
{
	m_nHeigth = 40;

	m_pPlayPause = new QIconPushButton(this, true, "play", "play-active");
	m_pVolume = new QIconPushButton(this, true, "volume", "volume-active");
	m_pFullscreen = new QIconPushButton(this, true, "fullscreen-on", "fullscreen-on-active");
	m_pPlaylist = new QIconPushButton(this, true, "playlist", "playlist-active");

	m_pSlider = new QVideoSlider(this);
	m_pSlider->setOrientation(Qt::Horizontal);
	m_pSlider->setMinimum(0);
	m_pSlider->setMaximum(100000);
	m_pSlider->setValue(0);
	m_pSlider->setSingleStep(1000);
	m_pSlider->setShortcutEnabled(QKeySequence::Back, true);
	m_pSlider->setShortcutEnabled(QKeySequence::Forward, true);
	m_pSlider->SetSeekOnClick(true);

	m_bIsEnabledPlayList = true;
	m_bIsEnabledFullscreen = true;

//#if defined(_LINUX) && !defined(_MAC)
	this->setMouseTracking(true);
	m_pSlider->setMouseTracking(true);
	m_pFullscreen->setMouseTracking(true);
	m_pPlaylist->setMouseTracking(true);
	m_pVolume->setMouseTracking(true);
	m_pPlayPause->setMouseTracking(true);

//	this->setCursor(QCursor(Qt::ArrowCursor));
//#endif
}

void QFooterPanel::resizeEvent(QResizeEvent* e)
{
	double dDpi = QWidget_GetDPI(this);

	int nWidth = this->width();
	int nButW = QWidget_ScaleDPI(30, dDpi);

	int nY = QWidget_ScaleDPI(5, dDpi);
	int nBetweenButtons = QWidget_ScaleDPI(10, dDpi);

	int nLeft = nBetweenButtons;
	m_pPlayPause->setGeometry(nLeft, nY, nButW, nButW); nLeft += (nButW + nBetweenButtons);

	int nRight = nWidth;

	nRight -= (nBetweenButtons + nButW);
	if (m_bIsEnabledPlayList)
	{
		m_pPlaylist->setGeometry(nRight, nY, nButW, nButW); nRight -= (nBetweenButtons + nButW);
	}
	if (m_bIsEnabledFullscreen)
	{
		m_pFullscreen->setGeometry(nRight, nY, nButW, nButW); nRight -= (nBetweenButtons + nButW);
	}

	m_pVolume->setGeometry(nRight, nY, nButW, nButW); nRight -= nBetweenButtons;
	m_pSlider->setGeometry(nLeft, nY, nRight - nLeft, nButW);
}

void QFooterPanel::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

//#if defined(_LINUX) && !defined(_MAC)
//#include <QApplication>
void QFooterPanel::mouseMoveEvent(QMouseEvent* event)
{
	QAscVideoView* pView = static_cast<QAscVideoView*>(this->parentWidget());
	if (pView->getMainWindowFullScreen())
	{
		if (pView->m_pInternal->m_oFooterTimer.isActive())
			pView->m_pInternal->m_oFooterTimer.stop();
	}
	event->accept();
//	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}
//#endif

void QFooterPanel::SetPlayPauseIcon(bool bIsPlay)
{
	QString sI = bIsPlay ? "play" : "pause";
	((QIconPushButton*)m_pPlayPause)->changeIcons(sI, sI + "-active");
}

void QFooterPanel::SetFullscreenIcon(bool bIsFullscreen)
{
	QString sI = bIsFullscreen ? "fullscreen-on" : "fullscreen-off";
	((QIconPushButton*)m_pFullscreen)->changeIcons(sI, sI + "-active");
}
