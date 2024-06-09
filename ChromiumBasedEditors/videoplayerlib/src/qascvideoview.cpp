#include "../qascvideoview.h"

#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QMimeData>
#include <QStyleOption>

#include "../../../../core/DesktopEditor/common/File.h"

#include "qvideoplaylist.h"
#include "qascvideowidget.h"
#include "qfooterpanel_private.h"

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

QAscVideoView::QAscVideoView(QWidget *parent, bool bIsPresentationMode) : QWidget(parent)
{
	NSBaseVideoLibrary::Init(parent);
	m_pInternal = new QAscVideoView_Private();
	m_pInternal->m_bIsPresentationMode = bIsPresentationMode;
	QWidgetUtils::SetDPI(this, QWidgetUtils::GetDPI(parent));

	QWidgetUtils::SetBackground(this, QColor(0x22, 0x22, 0x22));
	// create footer as a separated from QAscVideoView widget
	m_pInternal->m_pFooter = new QFooterPanel(parent, this);

	QWidget* pParentVideo = new QVideoWidgetParent(this);
	pParentVideo->setGeometry(0, 0, width(), height());

	m_pInternal->m_pPlayer = new QAscVideoWidget(pParentVideo);
	m_pInternal->m_pPlayer->m_pView = this;
	m_pInternal->m_pPlayer->installEventFilter(this);

	m_pInternal->m_pPlaylist = new QVideoPlaylist(this);
	m_pInternal->m_pPlaylist->setGeometry(width(), 0, 250, height());

	QObject::connect(m_pInternal->m_pPlaylist, SIGNAL(fileChanged(QString,bool)), this, SLOT(slotOpenFile(QString,bool)));
	m_pInternal->m_pPlaylist->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pListView->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pAdd->installEventFilter(this);
	m_pInternal->m_pPlaylist->m_pClear->installEventFilter(this);

	// animations
	m_pAnimationPlaylist = new QPropertyAnimation(m_pInternal->m_pPlaylist, "pos", this);
	m_pAnimationFooter = new QPropertyAnimation(m_pInternal->m_pFooter, "geometry", this);

	// init events
	QObject::connect(m_pInternal->m_pPlayer, SIGNAL(stateChanged(QMediaPlayer_State)), this, SLOT(slotPlayerStateChanged(QMediaPlayer_State)));
	QObject::connect(m_pInternal->m_pPlayer, SIGNAL(posChanged(int)), this, SLOT(slotPlayerPosChanged(int)));
	QObject::connect(m_pInternal->m_pPlayer, SIGNAL(videoOutputChanged(bool)), this, SLOT(slotVideoAvailableChanged(bool)));

	QObject::connect(m_pAnimationFooter, SIGNAL(finished()), this, SLOT(slotFooterAnimationFinished()));
	// timers
	QObject::connect(&m_pInternal->m_oFooterTimer, SIGNAL(timeout()), this, SLOT(slotFooterTimerOverflowed()));
	QObject::connect(&m_pInternal->m_oCursorTimer, SIGNAL(timeout()), this, SLOT(slotCursorTimerOverflowed()));

	setAcceptDrops(true);

	if (!m_pInternal->m_bIsPresentationMode)
	{
		this->setMouseTracking(true);
		m_pInternal->m_pPlayer->parentWidget()->setMouseTracking(true);
		m_pInternal->m_pPlayer->setMouseTracking(true);
		m_pInternal->m_pPlaylist->setMouseTracking(true);
	}

	m_pInternal->m_bIsShowingPlaylist = false;
	m_pInternal->m_bIsShowingFooter = true;
	m_pInternal->m_bIsPlay = true;
	m_pInternal->m_bIsEnabledPlayList = true;
	m_pInternal->m_bIsEnabledFullscreen = true;
	m_pInternal->m_bIsPresentationModeMediaTypeSended = false;
	m_pInternal->m_bIsDestroy = false;
	m_pInternal->m_bIsMuted = false;

	m_pInternal->m_bIsSeekEnabled = true;

	m_pInternal->m_pFooter->installEventFilter(this);
}

QAscVideoView::~QAscVideoView()
{
	delete m_pInternal;
}

void QAscVideoView::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	double dDpi = QWidgetUtils::GetDPI(this);

	if (m_pInternal->m_bIsEnabledFullscreen)
	{
		if (getMainWindowFullScreen())
		{
			Footer()->raise();
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
	}

	int nViewHeight = height();
	int nVideoHeight = nViewHeight;
	if (!m_pInternal->m_bIsPresentationMode && !getMainWindowFullScreen())
		nVideoHeight -= Footer()->GetHeight();

	m_pInternal->m_pPlayer->parentWidget()->setGeometry(0, 0, width(), nVideoHeight);
	m_pInternal->m_pPlayer->setGeometry(0, 0, width(), nVideoHeight);

	if (m_pInternal->m_bIsEnabledPlayList)
	{
		int nPlaylistHeight = nViewHeight - Footer()->GetHeight();
		m_pInternal->m_pPlaylist->setGeometry(m_pInternal->m_bIsShowingPlaylist ? (width() - QWidgetUtils::ScaleDPI(250, dDpi)) : width(), 0, QWidgetUtils::ScaleDPI(250, dDpi), nPlaylistHeight);
	}
	else
	{
		m_pInternal->m_pPlaylist->hide();
	}
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
	if (!Footer()->VolumeControls()->isHidden())
		Footer()->VolumeControls()->setHidden(true);

	if (event->button() == Qt::LeftButton)
	{
		if (m_pInternal->m_bIsShowingPlaylist)
			TogglePlaylist();
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
					ToggleFooter();
			});
		}
		else
		{
			// show footer
			ToggleFooter();
		}
	}

	setFocus();
	event->accept();
}

void QAscVideoView::keyPressEvent(QKeyEvent *event)
{
	m_pInternal->m_pPlayer->keyPressEvent(event);
	Qt::KeyboardModifiers ee = event->modifiers();
	switch (event->key())
	{
	case Qt::Key_Left:
	{
		m_pInternal->m_pPlayer->stepBack();
		break;
	}
	case Qt::Key_Right:
	{
		m_pInternal->m_pPlayer->stepForward();
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
		Footer()->m_pInternal->m_pSlider->setValue(100000);
		m_pInternal->m_pPlaylist->Next();
		break;
	}
	case Qt::Key_P:
	{
		Footer()->m_pInternal->m_pSlider->setValue(0);
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
		// don't change volume if wheel event was caused by playlist scrolling
		if (m_pInternal->m_bIsShowingPlaylist && m_pInternal->m_pPlaylist->isScrollBarVisible() &&
			(watched == m_pInternal->m_pPlaylist->m_pListView || watched == m_pInternal->m_pPlaylist))
			return true;

		Footer()->m_pInternal->m_pVolumeControlV->event(event);
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

void QAscVideoView::ChangeVolume(int nValue)
{
	if (nValue < 0)
		nValue = 0;
	if (nValue > 100)
		nValue = 100;

	m_pInternal->m_pFooter->m_pInternal->m_pVolumeControlV->setValue(nValue);
}

void QAscVideoView::ChangeSeek(int nValue)
{
	if (m_pInternal->m_bIsSeekEnabled)
		m_pInternal->m_pPlayer->setSeek(nValue);
}

void QAscVideoView::ToggleMute()
{
	if (m_pInternal->m_bIsMuted)
	{
		// restore and apply old volume
		m_pInternal->m_pPlayer->m_nVolume = m_pInternal->m_nMutedVolume;
		Footer()->m_pInternal->m_pVolumeControlV->setValue(m_pInternal->m_pPlayer->m_nVolume);
		m_pInternal->m_bIsMuted = false;
	}
	else
	{
		// save volume
		m_pInternal->m_nMutedVolume = m_pInternal->m_pPlayer->m_nVolume;
		// set current volume to 0
		Footer()->m_pInternal->m_pVolumeControlV->setValue(0);
		m_pInternal->m_bIsMuted = true;
	}
}

void QAscVideoView::Fullscreen()
{
	m_pInternal->m_pPlayer->setFullScreenOnCurrentScreen(!getMainWindowFullScreen());
	// to correctly handle key press events
	this->setFocus();
}

void QAscVideoView::TogglePlaylist(double duration)
{
	m_pInternal->m_bIsShowingPlaylist = !m_pInternal->m_bIsShowingPlaylist;

	double dDpi = QWidgetUtils::GetDPI(this);
	int nPlaylistW = QWidgetUtils::ScaleDPI(250, dDpi);

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

void QAscVideoView::ToggleFooter(double duration)
{
	if (m_pInternal->m_bIsShowingFooter && (Footer()->VolumeControls()->isVisible() || m_pInternal->m_bIsShowingPlaylist))
		return;

	m_pInternal->m_bIsShowingFooter = !m_pInternal->m_bIsShowingFooter;

	QSize size = this->size();
	int nFooterH = Footer()->GetHeight();

	m_pAnimationFooter->setDuration(duration);
	if (m_pInternal->m_bIsShowingFooter)
	{
		m_pAnimationFooter->setEndValue(QRect(0, size.height() - nFooterH, Footer()->width(), nFooterH));
	}
	else
	{
		m_pAnimationFooter->setEndValue(QRect(0, size.height(), Footer()->width(), nFooterH));
	}
	m_pAnimationFooter->start();
}

void QAscVideoView::SavePlayListAddons(const QString& sAddon)
{
	m_pInternal->m_pPlaylist->m_sSavePlayListAddon = sAddon;
}

QFooterPanel* QAscVideoView::Footer()
{
	return m_pInternal->m_pFooter;
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
	Footer()->m_pInternal->m_bIsEnabledPlayList = isUsed;

	Footer()->m_pInternal->m_pPlaylist->setHidden(!isUsed);
}
void QAscVideoView::setFullScreenUsed(bool isUsed)
{
	m_pInternal->m_bIsEnabledFullscreen = isUsed;
	Footer()->m_pInternal->m_bIsEnabledFullscreen = isUsed;

	Footer()->m_pInternal->m_pFullscreen->setHidden(!isUsed);
}
void QAscVideoView::setMedia(QString sMedia, bool isStart)
{
	QStringList files;
	files.append(sMedia);
	AddFilesToPlaylist(files, isStart);
}

void QAscVideoView::Stop()
{
	m_pInternal->m_bIsDestroy = true;
	m_pInternal->m_pPlayer->stop();
}

void QAscVideoView::RemoveFromPresentation()
{
	if (!m_pInternal->m_bIsPresentationMode)
		return;

	this->Stop();

	this->hide();
	this->deleteLater();

	Footer()->hide();
	Footer()->deleteLater();

	Footer()->VolumeControls()->hide();
	Footer()->VolumeControls()->deleteLater();
}

void QAscVideoView::UpdatePlayPauseIcon()
{
	Footer()->SetPlayPauseIcon(m_pInternal->m_bIsPlay);
}

void QAscVideoView::UpdateFullscreenIcon()
{
	Footer()->SetFullscreenIcon(!getMainWindowFullScreen());
}

void QAscVideoView::slotOpenFile(QString sFile, bool isPlay)
{
	if (sFile.isEmpty() && m_pInternal->m_bIsPresentationMode)
	{
		slotPlayerPosChanged(0);
		return;
	}
	m_pInternal->m_pPlayer->open(sFile, isPlay);

	if (sFile == "")
		return;

	std::wstring sFileW = sFile.toStdWString();
	std::wstring sFileName = NSFile::GetFileName(sFileW);
	emit titleChanged(QString::fromStdWString(sFileName));
}

void QAscVideoView::slotPlayerPosChanged(int nPos)
{
	m_pInternal->m_bIsSeekEnabled = false;
	Footer()->m_pInternal->m_pSlider->setValue(nPos);
	m_pInternal->m_bIsSeekEnabled = true;
}

void QAscVideoView::slotPlayerStateChanged(QMediaPlayer_State state)
{
	m_pInternal->m_bIsPlay = (state == QMediaPlayer::PlayingState) ? false : true;
	UpdatePlayPauseIcon();

	if (!m_pInternal->m_bIsPlay)
	{
		if (!Footer()->isHidden())
		{
			Footer()->raise();
		}
	}

	if (state == QMediaPlayer::StoppedState
		#ifndef USE_VLC_LIBRARY
			&& QMediaPlayer::EndOfMedia == m_pInternal->m_pPlayer->getEngine()->mediaStatus()
		#endif
			)

	{
		// force slider to be put to the end
		slotPlayerPosChanged(100000);
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

void QAscVideoView::slotVideoAvailableChanged(bool isVideoAvailable)
{
	if (m_pInternal->m_bIsPresentationMode && !m_pInternal->m_bIsPresentationModeMediaTypeSended)
	{
		m_pInternal->m_bIsPresentationModeMediaTypeSended = true;
		if (!m_pInternal->m_pPlayer->isAudio() && !m_pInternal->m_bIsDestroy)
		{
			this->show();
			Footer()->show();
		}
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
	this->ToggleFooter();
}

void QAscVideoView::slotCursorTimerOverflowed()
{
	m_pInternal->m_oCursorTimer.stop();
	setCursor(Qt::BlankCursor);
}
