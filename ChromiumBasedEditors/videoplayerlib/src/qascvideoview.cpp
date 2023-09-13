#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QMimeData>
#include <QStyleOption>
#include <QApplication>

#include "../../../../core/DesktopEditor/common/File.h"

#include "qpushbutton_icons.h"
#include "../qascvideoview.h"
#include "qfooterpanel.h"
#include "qvideoplaylist.h"
#include "qascvideowidget.h"
#include "qvideoslider.h"

#ifdef USE_VLC_LIBRARY
#ifdef WIN32
//#define USE_VLC_LIBRARY_VIDEO
#endif
#endif

#ifdef USE_VLC_LIBRARY_VIDEO

class QVideoWidgetParent : public QVideoWidget
{
	Q_OBJECT

public:
	QWidget* m_pChild;

public:
	explicit QVideoWidgetParent(QWidget *parent = Q_NULLPTR);

public:
	virtual void resizeEvent(QResizeEvent* event);
};

#else

typedef QWidget QVideoWidgetParent;

#endif

#ifdef USE_VLC_LIBRARY_VIDEO

QVideoWidgetParent::QVideoWidgetParent(QWidget* parent) : QVideoWidget(parent)
{
	m_pChild = NULL;
}

void QVideoWidgetParent::resizeEvent(QResizeEvent *event)
{
	QVideoWidget::resizeEvent(event);

	if (m_pChild)
		m_pChild->setGeometry(0, 0, width(), height());
}

#endif

QAscVideoView::QAscVideoView(QWidget *parent, int r, int g, int b) : QWidget(parent)
{
	NSBaseVideoLibrary::Init(parent);
	m_pInternal = new QAscVideoView_Private();

	QWidget_setBackground(this, 0x22, 0x22, 0x22);
	m_pInternal->m_pFooter = new QFooterPanel(this);
	QWidget_setBackground(m_pInternal->m_pFooter, r, g, b);

	this->setStyleSheet("QFrame {border: none;}");

	QWidget* pParentVideo = new QVideoWidgetParent(this);
	pParentVideo->setGeometry(0, 0, width(), height());

	m_pInternal->m_pPlayer = new QAscVideoWidget(pParentVideo);
	m_pInternal->m_pPlayer->m_pView = this;
	m_pInternal->m_pPlayer->installEventFilter(this);

	m_pInternal->m_pPlaylist = new QVideoPlaylist(this);
	m_pInternal->m_pPlaylist->setGeometry(width(), 0, 250, height());

	QObject::connect(m_pInternal->m_pPlaylist, SIGNAL(fileChanged(QString)), this, SLOT(slotOpenFile(QString)));
	m_pInternal->m_pPlaylist->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pListView->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pAdd->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pClear->installEventFilter(this);

	m_pInternal->m_pVolumeControl = new QWidget(this);
	m_pInternal->m_pVolumeControl->setHidden(true);
	m_pInternal->m_pVolumeControl->setGeometry(0, 0, 60, 160);
	m_pInternal->m_pVolumeControl->setStyleSheet("border: none; background-color:#111111");

	m_pInternal->m_pVolumeControlV = new QVideoSlider(m_pInternal->m_pVolumeControl);
	m_pInternal->m_pVolumeControlV->setOrientation(Qt::Vertical);
	m_pInternal->m_pVolumeControlV->setGeometry(15, 20, 30, 120);
	m_pInternal->m_pVolumeControlV->setMinimum(0);
	m_pInternal->m_pVolumeControlV->setMaximum(100);
	m_pInternal->m_pVolumeControlV->setValue(50);
	QObject::connect(m_pInternal->m_pVolumeControlV, SIGNAL(valueChanged(int)), this, SLOT(slotVolumeChanged(int)));

	// animations
	m_pAnimationPlaylist = new QPropertyAnimation(m_pInternal->m_pPlaylist, "pos", this);
	m_pAnimationFooter = new QPropertyAnimation(m_pInternal->m_pFooter, "geometry", this);

	// init events
	QObject::connect(m_pInternal->m_pFooter->m_pPlayPause, SIGNAL(clicked(bool)), this, SLOT(slotPlayPause()));
	QObject::connect(m_pInternal->m_pFooter->m_pVolume, SIGNAL(clicked(bool)), this, SLOT(slotVolume()));
	QObject::connect(m_pInternal->m_pFooter->m_pFullscreen, SIGNAL(clicked(bool)), this, SLOT(slotFullscreen()));
	QObject::connect(m_pInternal->m_pFooter->m_pPlaylist, SIGNAL(clicked(bool)), this, SLOT(slotPlaylist()));

	QObject::connect(m_pInternal->m_pFooter->m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSeekChanged(int)));

	QObject::connect(m_pInternal->m_pPlayer, SIGNAL(stateChanged(QMediaPlayer_State)), this, SLOT(slotPlayerStateChanged(QMediaPlayer_State)));
	QObject::connect(m_pInternal->m_pPlayer, SIGNAL(posChanged(int)), this, SLOT(slotPlayerPosChanged(int)));

	QObject::connect(m_pAnimationFooter, SIGNAL(finished()), this, SLOT(slotFooterAnimationFinished()));
	// timers
	QObject::connect(&m_pInternal->m_oFooterTimer, SIGNAL(timeout()), this, SLOT(slotFooterTimerOverflowed()));
	QObject::connect(&m_pInternal->m_oCursorTimer, SIGNAL(timeout()), this, SLOT(slotCursorTimerOverflowed()));

#ifndef USE_VLC_LIBRARY
	QObject::connect(m_pInternal->m_pPlayer->getEngine(), SIGNAL(videoAvailableChanged(bool)), this, SLOT(slotVideoAvailableChanged(bool)));
#else

#endif

	setAcceptDrops(true);

	this->setMouseTracking(true);
	m_pInternal->m_pPlayer->parentWidget()->setMouseTracking(true);
	m_pInternal->m_pPlayer->setMouseTracking(true);
	m_pInternal->m_pPlaylist->setMouseTracking(true);

	m_pInternal->m_bIsShowingPlaylist = false;
	m_pInternal->m_bIsShowingFooter = true;
	m_pInternal->m_bIsPlay = true;
	m_pInternal->m_bIsEnabledPlayList = true;
	m_pInternal->m_bIsEnabledFullscreen = true;
	m_pInternal->m_bIsPresentationMode = false;
	m_pInternal->m_bIsPresentationModeMediaTypeSended = false;
	m_pInternal->m_bIsDestroy = false;
	m_pInternal->m_bIsMuted = false;

	m_pInternal->m_bIsSeekEnabled = true;

	m_pInternal->m_pFooter->installEventFilter(this);
	m_pInternal->m_pFooter->m_pFullscreen->installEventFilter(this);
	m_pInternal->m_pFooter->m_pVolume->installEventFilter(this);
	m_pInternal->m_pFooter->m_pPlaylist->installEventFilter(this);
	m_pInternal->m_pFooter->m_pSlider->installEventFilter(this);
	m_pInternal->m_pFooter->m_pPlayPause->installEventFilter(this);
}

QAscVideoView::~QAscVideoView()
{

}

void QAscVideoView::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	QSize size = this->size();
	double dDpi = QWidget_GetDPI(this);
	int nFooterH = QWidget_ScaleDPI(m_pInternal->m_pFooter->m_nHeigth, dDpi);

	if (getMainWindowFullScreen())
	{
		m_pInternal->m_pFooter->raise();
		nFooterH = 0;
		m_pInternal->m_bIsShowingFooter = false;
		// start cursor hiding timer
		m_pInternal->m_oCursorTimer.start(m_pInternal->c_nCursorHidingDelay);
	}
	else
	{
		// show cursor
		setCursor(Qt::ArrowCursor);
		// stop all hiding timers
		m_pInternal->m_oCursorTimer.stop();
		m_pInternal->m_oFooterTimer.stop();
	}

	m_pInternal->m_pFooter->setGeometry(0, size.height() - nFooterH, size.width(), nFooterH);

	int nViewHeight = size.height();

	if (!m_pInternal->m_bIsPresentationMode)
		nViewHeight -= nFooterH;

	m_pInternal->m_pPlayer->parentWidget()->setGeometry(0, 0, width(), nViewHeight);
	m_pInternal->m_pPlayer->setGeometry(0, 0, width(), nViewHeight);

	if (m_pInternal->m_bIsPresentationMode)
		nViewHeight -= nFooterH;

	int nVolumeVHeight = QWidget_ScaleDPI(160, dDpi);
	if (nVolumeVHeight > nViewHeight)
		nVolumeVHeight = nViewHeight;

	int nVolumeVHeightSlider = nVolumeVHeight - QWidget_ScaleDPI(40, dDpi);
	if (30 > nVolumeVHeightSlider)
		nVolumeVHeightSlider = 30;

	m_pInternal->m_pVolumeControl->setGeometry(0, 0, QWidget_ScaleDPI(60, dDpi), nVolumeVHeight);
	m_pInternal->m_pVolumeControlV->setGeometry(QWidget_ScaleDPI(15, dDpi), QWidget_ScaleDPI(20, dDpi), QWidget_ScaleDPI(30, dDpi), nVolumeVHeightSlider);

	int nOffsetVolume = QWidget_ScaleDPI(135, dDpi);
	if (!m_pInternal->m_bIsEnabledPlayList)
		nOffsetVolume -= QWidget_ScaleDPI(35, dDpi);
	if (!m_pInternal->m_bIsEnabledFullscreen)
		nOffsetVolume -= QWidget_ScaleDPI(35, dDpi);

	if (getMainWindowFullScreen())
		nViewHeight -= QWidget_ScaleDPI(m_pInternal->m_pFooter->m_nHeigth, dDpi);

	m_pInternal->m_pVolumeControl->move(width() - nOffsetVolume, nViewHeight - m_pInternal->m_pVolumeControl->height());
	m_pInternal->m_pVolumeControlV->resizeEvent(NULL);
	m_pInternal->m_pPlaylist->setGeometry(m_pInternal->m_bIsShowingPlaylist ? (width() - QWidget_ScaleDPI(250, dDpi)) : width(), 0, QWidget_ScaleDPI(250, dDpi), nViewHeight);
}

void QAscVideoView::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void QAscVideoView::dragEnterEvent(QDragEnterEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();

	CVideoExt ext;
	QSet<QString> _exts = ext.getQSet(false);

	for (int i = 0; i < urls.count(); i++)
	{
		QFileInfo oInfo(urls[i].toString());
		if (_exts.contains(oInfo.suffix().toLower()))
		{
			event->acceptProposedAction();
			return;
		}
	}
}

void QAscVideoView::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();

	QStringList files;
	for (int i = 0; i < urls.count(); i++)
	{
		files.append(urls[i].toLocalFile());
	}

	m_pInternal->m_pPlaylist->AddFiles(files, true);
	event->acceptProposedAction();
}

void QAscVideoView::mousePressEvent(QMouseEvent *event)
{
	if (!m_pInternal->m_pVolumeControl->isHidden())
		m_pInternal->m_pVolumeControl->setHidden(true);

	if (event->button() == Qt::LeftButton)
	{
		if (m_pInternal->m_bIsShowingPlaylist)
			Playlist();
	}

	if (getMainWindowFullScreen())
	{
		setCursor(Qt::ArrowCursor);
		// start cursor hiding timer
		m_pInternal->m_oCursorTimer.start(m_pInternal->c_nCursorHidingDelay);
		// toggle footer
		if (m_pInternal->m_bIsShowingFooter)
		{
			// hide footer after a short delay
			QTimer::singleShot(300, this, [this]() {
				if (getMainWindowFullScreen())
					Footer();
			});
		}
		else
		{
			// show footer
			Footer();
		}
	}

	setFocus();
	event->accept();
}

void QAscVideoView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (m_pInternal->m_bIsPresentationMode)
		{
			m_pInternal->m_pFooter->setHidden(!m_pInternal->m_pFooter->isHidden());
			if (!m_pInternal->m_pFooter->isHidden())
			{
				m_pInternal->m_pFooter->raise();
			}
		}
	}
}

void QAscVideoView::keyPressEvent(QKeyEvent *event)
{
	m_pInternal->m_pPlayer->keyPressEvent(event);
	Qt::KeyboardModifiers ee = event->modifiers();
	switch (event->key())
	{
	case Qt::Key_Left:
	{
		m_pInternal->m_pFooter->m_pSlider->event(event);
		break;
	}
	case Qt::Key_Right:
	{
		m_pInternal->m_pFooter->m_pSlider->event(event);
		break;
	}
	case Qt::Key_O:
	{
		if (static_cast<int>(ee) == Qt::ControlModifier)
			m_pInternal->m_pPlaylist->slotButtonAdd();
		break;
	}
	case Qt::Key_Space:
	{
		PlayPause();
		break;
	}
	case Qt::Key_M:
	{
		ToggleMute();
		break;
	}
	case Qt::Key_N:
	{
		m_pInternal->m_pFooter->m_pSlider->setValue(100000);
		m_pInternal->m_pPlaylist->Next();
		break;
	}
	case Qt::Key_P:
	{
		m_pInternal->m_pFooter->m_pSlider->setValue(0);
		m_pInternal->m_pPlaylist->Prev();
	}
	default:
		break;
	}

	event->accept();
}

bool QAscVideoView::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		this->keyPressEvent((QKeyEvent*)event);
		return true;
	}

	if (event->type() == QEvent::Wheel)
	{
		m_pInternal->m_pVolumeControlV->event(event);
		return true;
	}
	return QWidget::eventFilter(watched, event);
}

void QAscVideoView::PlayPause()
{
	if (m_pInternal->m_bIsPlay)
		m_pInternal->m_pPlayer->setPlay();
	else
		m_pInternal->m_pPlayer->setPause();
}

void QAscVideoView::ToggleMute()
{
	if (m_pInternal->m_bIsMuted)
	{
		// restore and apply old volume
		m_pInternal->m_pPlayer->m_nVolume = m_pInternal->m_nMutedVolume;
		m_pInternal->m_pVolumeControlV->setValue(m_pInternal->m_pPlayer->m_nVolume);
		m_pInternal->m_bIsMuted = false;
	}
	else
	{
		// save volume
		m_pInternal->m_nMutedVolume = m_pInternal->m_pPlayer->m_nVolume;
		// set current volume to 0
		m_pInternal->m_pVolumeControlV->setValue(0);
		m_pInternal->m_bIsMuted = true;
	}
}

void QAscVideoView::Volume()
{
	m_pInternal->m_pVolumeControl->setHidden(!m_pInternal->m_pVolumeControl->isHidden());
}

void QAscVideoView::Fullscreen()
{
	m_pInternal->m_pPlayer->setFullScreenOnCurrentScreen(!getMainWindowFullScreen());
	// to correctly handle key press events
	this->setFocus();
}

void QAscVideoView::Playlist(double duration)
{
	m_pInternal->m_bIsShowingPlaylist = !m_pInternal->m_bIsShowingPlaylist;

	double dDpi = QWidget_GetDPI(this);
	int nPlaylistW = QWidget_ScaleDPI(250, dDpi);

	m_pAnimationPlaylist->setDuration(duration);
	if (m_pInternal->m_bIsShowingPlaylist)
	{
		m_pAnimationPlaylist->setEndValue(QPoint(width() - nPlaylistW, 0));
	}
	else
	{
		m_pAnimationPlaylist->setEndValue(QPoint(width(), 0));
	}
	m_pAnimationPlaylist->start();
}

void QAscVideoView::Footer(double duration)
{
	if (m_pInternal->m_bIsShowingFooter && (m_pInternal->m_pVolumeControl->isVisible() || m_pInternal->m_bIsShowingPlaylist))
		return;

	m_pInternal->m_bIsShowingFooter = !m_pInternal->m_bIsShowingFooter;

	QSize size = this->size();
	double dDpi = QWidget_GetDPI(this);
	int nFooterH = QWidget_ScaleDPI(m_pInternal->m_pFooter->m_nHeigth, dDpi);

	m_pAnimationFooter->setDuration(duration);
	if (m_pInternal->m_bIsShowingFooter)
	{
		m_pAnimationFooter->setEndValue(QRect(0, size.height() - nFooterH, m_pInternal->m_pFooter->width(), nFooterH));
	}
	else
	{
		m_pAnimationFooter->setEndValue(QRect(0, size.height(), m_pInternal->m_pFooter->width(), nFooterH));
	}
	m_pAnimationFooter->start();
}

void QAscVideoView::SavePlayListAddons(const QString& sAddon)
{
	m_pInternal->m_pPlaylist->m_sSavePlayListAddon = sAddon;
}

void QAscVideoView::AddFilesToPlaylist(QStringList& files, const bool isStart)
{
	m_pInternal->m_pPlaylist->AddFiles(files, isStart);
}

void QAscVideoView::LoadPlaylist()
{
	m_pInternal->m_pPlaylist->m_pDialogParent = this->getMainWindow();
	m_pInternal->m_pPlaylist->Load();
}

void QAscVideoView::SavePlaylist()
{
	m_pInternal->m_pPlaylist->Save();
}

void QAscVideoView::setPlayListUsed(bool isUsed)
{
	m_pInternal->m_bIsEnabledPlayList = isUsed;
	m_pInternal->m_pFooter->m_bIsEnabledPlayList = isUsed;

	m_pInternal->m_pFooter->m_pPlaylist->setHidden(!isUsed);
}
void QAscVideoView::setFullScreenUsed(bool isUsed)
{
	m_pInternal->m_bIsEnabledFullscreen = isUsed;
	m_pInternal->m_pFooter->m_bIsEnabledFullscreen = isUsed;

	m_pInternal->m_pFooter->m_pFullscreen->setHidden(!isUsed);
}
void QAscVideoView::setPresentationMode(bool isPresentationMode)
{
	m_pInternal->m_bIsPresentationMode = isPresentationMode;
}
void QAscVideoView::setMedia(QString sMedia)
{
	QStringList files;
	files.append(sMedia);
	AddFilesToPlaylist(files, true);
}

void QAscVideoView::Stop()
{
	m_pInternal->m_bIsDestroy = true;
	m_pInternal->m_pPlayer->stop();
}

void QAscVideoView::UpdatePlayPauseIcon()
{
	m_pInternal->m_pFooter->SetPlayPauseIcon(m_pInternal->m_bIsPlay);
}

void QAscVideoView::UpdateFullscreenIcon()
{
	m_pInternal->m_pFooter->SetFullscreenIcon(!getMainWindowFullScreen());
}

void QAscVideoView::setFooterVisible(bool isVisible)
{
	m_pInternal->m_pFooter->setHidden(!isVisible);
}

void QAscVideoView::slotPlayPause()
{
	this->PlayPause();
}

void QAscVideoView::slotVolume()
{
	this->Volume();
}

void QAscVideoView::slotFullscreen()
{
	this->Fullscreen();
}

void QAscVideoView::slotPlaylist()
{
	this->Playlist();
}

void QAscVideoView::slotVolumeChanged(int nValue)
{
	m_pInternal->m_pPlayer->setVolume(nValue);
	if (nValue)
		m_pInternal->m_bIsMuted = false;
}

void QAscVideoView::slotSeekChanged(int nValue)
{
	if (m_pInternal->m_bIsSeekEnabled)
		m_pInternal->m_pPlayer->setSeek(nValue);
}

void QAscVideoView::slotOpenFile(QString sFile)
{
	if (sFile.isEmpty() && m_pInternal->m_bIsPresentationMode)
	{
		slotSeekChanged(0);
		return;
	}
	m_pInternal->m_pPlayer->open(sFile);

	if (sFile == "")
		return;

	std::wstring sFileW = sFile.toStdWString();
	std::wstring sFileName = NSFile::GetFileName(sFileW);
	emit OnTitleChanged(QString::fromStdWString(sFileName));
}

void QAscVideoView::slotPlayerPosChanged(int nPos)
{
	m_pInternal->m_bIsSeekEnabled = false;
	m_pInternal->m_pFooter->m_pSlider->setValue(nPos);
	m_pInternal->m_bIsSeekEnabled = true;
}

void QAscVideoView::slotPlayerStateChanged(QMediaPlayer_State state)
{
	m_pInternal->m_bIsPlay = (state == QMediaPlayer::PlayingState) ? false : true;
	UpdatePlayPauseIcon();

	if (!m_pInternal->m_bIsPlay)
	{
		if (!m_pInternal->m_pFooter->isHidden())
		{
			m_pInternal->m_pFooter->raise();
		}

#ifdef USE_VLC_LIBRARY
		if (m_pInternal->m_bIsPresentationMode && !m_pInternal->m_bIsPresentationModeMediaTypeSended)
		{
			m_pInternal->m_bIsPresentationModeMediaTypeSended = true;
			if (!m_pInternal->m_pPlayer->isAudio() && !m_pInternal->m_bIsDestroy)
				this->show();
		}
#endif
	}

	if (state == QMediaPlayer::StoppedState
		#ifndef USE_VLC_LIBRARY
			&& QMediaPlayer::EndOfMedia == m_pInternal->m_pPlayer->getEngine()->mediaStatus()
		#endif
			)

	{
		// force slider to be put to the end
		m_pInternal->m_pFooter->m_pSlider->setValue(100000);
		// play next video (if any)
		m_pInternal->m_pPlaylist->Next();
	}
}

bool QAscVideoView::getMainWindowFullScreen()
{
	return false;
}
void QAscVideoView::setMainWindowFullScreen(bool bIsFullScreen)
{
	return;
}
QWidget* QAscVideoView::getMainWindow()
{
	return NULL;
}

void QAscVideoView::slotVideoAvailableChanged(bool videoAvailable)
{
	if (m_pInternal->m_bIsPresentationMode && !m_pInternal->m_bIsPresentationModeMediaTypeSended)
	{
		m_pInternal->m_bIsPresentationModeMediaTypeSended = true;
		if (!m_pInternal->m_pPlayer->isAudio() && !m_pInternal->m_bIsDestroy)
			this->show();
	}
}

void QAscVideoView::slotFooterAnimationFinished()
{
	if (!getMainWindowFullScreen())
	{
		// without calling this event, the footer animation may be ended just after user has exited
		//  from fullscreen, so the footer will not be visible
		resizeEvent(nullptr);
	}
	else
	{
		if (m_pInternal->m_bIsShowingFooter)
			m_pInternal->m_oFooterTimer.start(m_pInternal->c_nFooterHidingDelay);
		else
			m_pInternal->m_oFooterTimer.stop();
	}
}

void QAscVideoView::slotFooterTimerOverflowed()
{
	m_pInternal->m_oFooterTimer.stop();
	this->Footer();
}

void QAscVideoView::slotCursorTimerOverflowed()
{
	m_pInternal->m_oCursorTimer.stop();
	setCursor(Qt::BlankCursor);
}
