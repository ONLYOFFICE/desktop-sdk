#include "../qfooterpanel.h"

#include <QStyleOption>
#include <QPainter>

#include "qfooterpanel_private.h"
#include "../qascvideoview.h"
#include "qascvideowidget.h"

QFooterPanel::QFooterPanel(QWidget* parent, QAscVideoView* pView) : QWidget(parent)
{
	m_pInternal = new QFooterPanel_Private();
	m_pInternal->m_pView = pView;
	double dpi = QWidgetUtils::GetDPI(parent);
	QWidgetUtils::SetDPI(this, dpi);

	// footer widgets
	m_pInternal->m_pPlayPause = new QIconPushButton(this, true, "play", "play-active");
	m_pInternal->m_pVolume = new QIconPushButton(this, true, "volume", "volume-active");
	m_pInternal->m_pFullscreen = new QIconPushButton(this, true, "fullscreen-on", "fullscreen-on-active");
	m_pInternal->m_pPlaylist = new QIconPushButton(this, true, "playlist", "playlist-active");

	m_pInternal->m_pSlider = new QVideoSlider(this);
	m_pInternal->m_pSlider->setOrientation(Qt::Horizontal);
	m_pInternal->m_pSlider->setMinimum(0);
	m_pInternal->m_pSlider->setMaximum(100000);
	m_pInternal->m_pSlider->setValue(0);
	m_pInternal->m_pSlider->setSingleStep(1000);
	m_pInternal->m_pSlider->setShortcutEnabled(QKeySequence::Back, true);
	m_pInternal->m_pSlider->setShortcutEnabled(QKeySequence::Forward, true);

	// volume control widgets
	m_pInternal->m_pVolumeControl = new QWidget(parent);
	QWidgetUtils::SetDPI(m_pInternal->m_pVolumeControl, dpi);
	m_pInternal->m_pVolumeControl->setHidden(true);

	m_pInternal->m_pVolumeControlV = new QVideoSlider(m_pInternal->m_pVolumeControl, QVideoSlider::htCircle);
	m_pInternal->m_pVolumeControlV->setOrientation(Qt::Vertical);
	m_pInternal->m_pVolumeControlV->setMinimum(0);
	m_pInternal->m_pVolumeControlV->setMaximum(100);
	m_pInternal->m_pVolumeControlV->setValue(50);

	// connect signals
	QObject::connect(m_pInternal->m_pPlayPause, SIGNAL(clicked(bool)), this, SLOT(onPlayPauseBtnClicked()));
	QObject::connect(m_pInternal->m_pVolume, SIGNAL(clicked(bool)), this, SLOT(onVolumeBtnClicked()));
	QObject::connect(m_pInternal->m_pFullscreen, SIGNAL(clicked(bool)), this, SLOT(onFullscreenBtnClicked()));
	QObject::connect(m_pInternal->m_pPlaylist, SIGNAL(clicked(bool)), this, SLOT(onPlaylistBtnClicked()));

	QObject::connect(m_pInternal->m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(onSeekChanged(int)));
	QObject::connect(m_pInternal->m_pVolumeControlV, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));

	// set event filters
	m_pInternal->m_pFullscreen->installEventFilter(pView);
	m_pInternal->m_pVolume->installEventFilter(pView);
	m_pInternal->m_pPlaylist->installEventFilter(pView);
	m_pInternal->m_pSlider->installEventFilter(pView);
	m_pInternal->m_pPlayPause->installEventFilter(pView);

	// set mouse tracking
	if (!pView->m_pInternal->m_bIsPresentationMode)
	{
		this->setMouseTracking(true);
		m_pInternal->m_pSlider->setMouseTracking(true);
		m_pInternal->m_pFullscreen->setMouseTracking(true);
		m_pInternal->m_pPlaylist->setMouseTracking(true);
		m_pInternal->m_pVolume->setMouseTracking(true);
		m_pInternal->m_pPlayPause->setMouseTracking(true);
	}

	// init flags
	m_pInternal->m_bIsEnabledPlayList = true;
	m_pInternal->m_bIsEnabledFullscreen = true;
}

QFooterPanel::~QFooterPanel()
{
	delete m_pInternal;
}

void QFooterPanel::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	double dDpi = QWidgetUtils::GetDPI(this);

	int nButW = QWidgetUtils::ScaleDPI(m_pInternal->c_nButtonsWidth, dDpi);
	int nY = QWidgetUtils::ScaleDPI(m_pInternal->c_nButtonsY, dDpi);
	int nBetweenButtons = QWidgetUtils::ScaleDPI(m_pInternal->c_nButtonsBetween, dDpi);

	int nLeft = nBetweenButtons;
	m_pInternal->m_pPlayPause->setGeometry(nLeft, nY, nButW, nButW); nLeft += (nButW + nBetweenButtons);

	int nRight = this->width();

	nRight -= (nBetweenButtons + nButW);
	if (m_pInternal->m_bIsEnabledPlayList)
	{
		m_pInternal->m_pPlaylist->setGeometry(nRight, nY, nButW, nButW); nRight -= (nBetweenButtons + nButW);
	}
	if (m_pInternal->m_bIsEnabledFullscreen)
	{
		m_pInternal->m_pFullscreen->setGeometry(nRight, nY, nButW, nButW); nRight -= (nBetweenButtons + nButW);
	}

	m_pInternal->m_pVolume->setGeometry(nRight, nY, nButW, nButW); nRight -= nBetweenButtons;
	m_pInternal->m_pSlider->setGeometry(nLeft, nY, nRight - nLeft, nButW);

	// set volume control geometry
	int nVolumeControlW = QWidgetUtils::ScaleDPI(m_pInternal->c_nVolumeControlWidth, dDpi);
	int nVolumeControlH = QWidgetUtils::ScaleDPI(m_pInternal->c_nVolumeControlHeight, dDpi);
	int nVolumeControlX = this->x() + m_pInternal->m_pVolume->x() + nButW / 2 - nVolumeControlW / 2;
	int nVolumeControlY = this->y() + m_pInternal->m_pVolume->y() - nVolumeControlH - nBetweenButtons;

	m_pInternal->m_pVolumeControl->setGeometry(nVolumeControlX, nVolumeControlY, nVolumeControlW, nVolumeControlH);

	// set volume slider geometry
	int nVolumeSliderW = QWidgetUtils::ScaleDPI(m_pInternal->c_nVolumeSliderWidth, dDpi);
	int nVolumeSliderH = QWidgetUtils::ScaleDPI(m_pInternal->c_nVolumeSliderHeight, dDpi);
	int nVolumeSliderX = (nVolumeControlW - nVolumeSliderW) / 2;
	int nVolumeSliderY = (nVolumeControlH - nVolumeSliderH) / 2;

	m_pInternal->m_pVolumeControlV->setGeometry(nVolumeSliderX, nVolumeSliderY, nVolumeSliderW, nVolumeSliderH);
}

void QFooterPanel::mouseMoveEvent(QMouseEvent* event)
{
	if (m_pInternal->m_pView->getMainWindowFullScreen())
	{
		QAscVideoView* pView = m_pInternal->m_pView;
		// stop cursor hiding timer
		if (pView->m_pInternal->m_oCursorTimer.isActive())
			pView->m_pInternal->m_oCursorTimer.stop();
		// stop footer hiding timer
		if (pView->m_pInternal->m_oFooterTimer.isActive())
			pView->m_pInternal->m_oFooterTimer.stop();
	}
	event->accept();
}

void QFooterPanel::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

int QFooterPanel::GetHeight()
{
	return QWidgetUtils::ScaleDPI(this, m_pInternal->c_nHeight);
}

int QFooterPanel::GetMinWidth()
{
	return QWidgetUtils::ScaleDPI(this, m_pInternal->c_nMinWidth);
}

int QFooterPanel::GetMaxWidth()
{
	return QWidgetUtils::ScaleDPI(this, m_pInternal->c_nMaxWidth);
}

QWidget* QFooterPanel::VolumeControls()
{
	return m_pInternal->m_pVolumeControl;
}

void QFooterPanel::ApplySkin(CFooterSkin::Type type)
{
	CFooterSkin skin = CFooterSkin::getSkin(type);
	// footer and volume control backgrounds
	QWidgetUtils::SetBackground(this, QColor(skin.m_oFooterStyleOpt.m_sBgColor));
	m_pInternal->m_pVolumeControl->setStyleSheet("border: none; background-color:" + skin.m_oFooterStyleOpt.m_sVolumeControlBgColor + ";");
	// buttons
	m_pInternal->m_pPlayPause->setStyleOptions(skin.m_oButtonStyleOpt);
	m_pInternal->m_pVolume->setStyleOptions(skin.m_oButtonStyleOpt);
	m_pInternal->m_pFullscreen->setStyleOptions(skin.m_oButtonStyleOpt);
	m_pInternal->m_pPlaylist->setStyleOptions(skin.m_oButtonStyleOpt);
	// sliders
	m_pInternal->m_pSlider->setStyleOptions(skin.m_oSliderStyleOpt);
	m_pInternal->m_pVolumeControlV->setStyleOptions(skin.m_oSliderStyleOpt);
}

void QFooterPanel::onPlayPauseBtnClicked()
{
	m_pInternal->m_pView->PlayPause();
}

void QFooterPanel::onVolumeBtnClicked()
{
	VolumeControls()->setHidden(!VolumeControls()->isHidden());
}

void QFooterPanel::onFullscreenBtnClicked()
{
	m_pInternal->m_pView->Fullscreen();
}

void QFooterPanel::onPlaylistBtnClicked()
{
	m_pInternal->m_pView->TogglePlaylist();
}

void QFooterPanel::onSeekChanged(int nValue)
{
	m_pInternal->m_pView->ChangeSeek(nValue);
}

void QFooterPanel::onVolumeChanged(int nValue)
{
	m_pInternal->m_pView->ChangeVolume(nValue);
}

void QFooterPanel::SetPlayPauseIcon(bool bIsPlay)
{
	QString sI = bIsPlay ? "play" : "pause";
	m_pInternal->m_pPlayPause->setIcons(sI, sI + "-active");
}

void QFooterPanel::SetFullscreenIcon(bool bIsFullscreen)
{
	QString sI = bIsFullscreen ? "fullscreen-on" : "fullscreen-off";
	m_pInternal->m_pFullscreen->setIcons(sI, sI + "-active");
}
