#include "qfooterpanel.h"
#include <QStyleOption>
#include <QPainter>
#include "qpushbutton_icons.h"
#include "qascvideoview.h"
#include "qascvideowidget.h"

QFooterPanel::QFooterPanel(QWidget* parent, QWidget* pVideoView) : QWidget(parent)
{
	m_pVideoView = pVideoView ? pVideoView : parent;
	QAscVideoView* pView = static_cast<QAscVideoView*>(m_pVideoView);

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

	if (!pView->m_pInternal->m_bIsPresentationMode)
	{
		this->setMouseTracking(true);
		m_pSlider->setMouseTracking(true);
		m_pFullscreen->setMouseTracking(true);
		m_pPlaylist->setMouseTracking(true);
		m_pVolume->setMouseTracking(true);
		m_pPlayPause->setMouseTracking(true);
	}
}

void QFooterPanel::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

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

	// volume controls
	QAscVideoView* pView = static_cast<QAscVideoView*>(m_pVideoView);

	// set volume control geometry
	int nVolumeControlW = QWidget_ScaleDPI(c_nVolumeControlWidth, dDpi);
	int nVolumeControlH = QWidget_ScaleDPI(c_nVolumeControlHeight, dDpi);
	int nVolumeControlX = this->x() + m_pVolume->x() + nButW / 2 - nVolumeControlW / 2;
	int nVolumeControlY = this->y() + m_pVolume->y() - nVolumeControlH - nBetweenButtons;

	QWidget* pVolumeControl = pView->m_pInternal->m_pVolumeControl;
	pVolumeControl->setGeometry(nVolumeControlX, nVolumeControlY, nVolumeControlW, nVolumeControlH);

	// set volume slider geometry
	int nVolumeSliderW = QWidget_ScaleDPI(c_nVolumeSliderWidth, dDpi);
	int nVolumeSliderH = QWidget_ScaleDPI(c_nVolumeSliderHeight, dDpi);
	int nVolumeSliderX = QWidget_ScaleDPI(c_nVolumeSliderX, dDpi);
	int nVolumeSliderY = QWidget_ScaleDPI(c_nVolumeSliderY, dDpi);

	QVideoSlider* pVolumeControlSlider = pView->m_pInternal->m_pVolumeControlV;
	pVolumeControlSlider->setGeometry(nVolumeSliderX, nVolumeSliderY, nVolumeSliderW, nVolumeSliderH);
}

void QFooterPanel::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void QFooterPanel::mouseMoveEvent(QMouseEvent* event)
{
	QAscVideoView* pView = static_cast<QAscVideoView*>(m_pVideoView);
	if (pView->getMainWindowFullScreen())
	{
		// stop cursor hiding timer
		if (pView->m_pInternal->m_oCursorTimer.isActive())
			pView->m_pInternal->m_oCursorTimer.stop();
		// stop footer hiding timer
		if (pView->m_pInternal->m_oFooterTimer.isActive())
			pView->m_pInternal->m_oFooterTimer.stop();
	}
	event->accept();
}

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
