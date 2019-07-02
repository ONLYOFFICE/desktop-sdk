#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QMimeData>
#include <QStyleOption>
#include <QPropertyAnimation>

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

    // init events
    QObject::connect(m_pInternal->m_pFooter->m_pPlayPause, SIGNAL(clicked(bool)), this, SLOT(slotPlayPause()));
    QObject::connect(m_pInternal->m_pFooter->m_pVolume, SIGNAL(clicked(bool)), this, SLOT(slotVolume()));
    QObject::connect(m_pInternal->m_pFooter->m_pFullscreen, SIGNAL(clicked(bool)), this, SLOT(slotFullscreen()));
    QObject::connect(m_pInternal->m_pFooter->m_pPlaylist, SIGNAL(clicked(bool)), this, SLOT(slotPlaylist()));

    QObject::connect(m_pInternal->m_pFooter->m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSeekChanged(int)));

    QObject::connect(m_pInternal->m_pPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slotPlayerStateChanged(QMediaPlayer::State)));
    QObject::connect(m_pInternal->m_pPlayer, SIGNAL(posChanged(int)), this, SLOT(slotPlayerPosChanged(int)));

#ifndef USE_VLC_LIBRARY
    QObject::connect(m_pInternal->m_pPlayer->getEngine(), SIGNAL(videoAvailableChanged(bool)), this, SLOT(slotVideoAvailableChanged(bool)));
#else

#endif

    setAcceptDrops(true);

#if defined(_LINUX) && !defined(_MAC)
    this->setMouseTracking(true);
    m_pInternal->m_pPlayer->setMouseTracking(true);
    m_pInternal->m_pPlaylist->setMouseTracking(true);
#endif

    m_pInternal->m_bIsShowingPlaylist = false;
    m_pInternal->m_bIsPlay = true;
    m_pInternal->m_bIsEnabledPlayList = true;
    m_pInternal->m_bIsEnabledFullscreen = true;
    m_pInternal->m_bIsPresentationMode = false;
    m_pInternal->m_bIsPresentationModeMediaTypeSended = false;
    m_pInternal->m_bIsDestroy = false;
    UpdatePlayPause();

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
    if (this->getMainWindowFullScreen())
        nFooterH = 0;

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

    m_pInternal->m_pVolumeControl->move(width() - nOffsetVolume, nViewHeight - m_pInternal->m_pVolumeControl->height());
    m_pInternal->m_pVolumeControlV->resizeEvent(NULL);
    m_pInternal->m_pPlaylist->setGeometry(m_pInternal->m_bIsShowingPlaylist ? (width() - QWidget_ScaleDPI(250, dDpi)) : width(), 0, QWidget_ScaleDPI(250, dDpi), nViewHeight);
}

void QAscVideoView::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
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

    setFocus();
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

#if defined(_LINUX) && !defined(_MAC)
#include <QApplication>
void QAscVideoView::mouseMoveEvent(QMouseEvent* e)
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}
#endif

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
    default:
        break;
    }
}

bool QAscVideoView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        this->keyPressEvent((QKeyEvent*)event);
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

void QAscVideoView::Volume()
{
    m_pInternal->m_pVolumeControl->setHidden(!m_pInternal->m_pVolumeControl->isHidden());
}

void QAscVideoView::Fullscreen()
{
    m_pInternal->m_pPlayer->setFullScreenOnCurrentScreen(true);
}

void QAscVideoView::Playlist(double duration)
{
    m_pInternal->m_bIsShowingPlaylist = !m_pInternal->m_bIsShowingPlaylist;

    if (m_pInternal->m_bIsShowingPlaylist)
    {
        QPropertyAnimation *animation = new QPropertyAnimation(m_pInternal->m_pPlaylist, "pos");
        animation->setDuration(duration);
        animation->setStartValue(QPoint(m_pInternal->m_pPlaylist->x(), 0));
        double dDpi = QWidget_GetDPI(this);
        animation->setEndValue(QPoint(width() - QWidget_ScaleDPI(250, dDpi), 0));
        animation->start();
    }
    else
    {
        QPropertyAnimation *animation = new QPropertyAnimation(m_pInternal->m_pPlaylist, "pos");
        animation->setDuration(duration);
        animation->setStartValue(QPoint(m_pInternal->m_pPlaylist->x(), 0));
        animation->setEndValue(QPoint(width(), 0));
        animation->start();
    }
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
    if (m_pInternal->m_bIsShowingPlaylist)
        Playlist(0);
    if (!m_pInternal->m_pVolumeControl->isHidden())
        m_pInternal->m_pVolumeControl->setHidden(true);
    this->Fullscreen();
}

void QAscVideoView::slotPlaylist()
{
    this->Playlist();
}

void QAscVideoView::slotVolumeChanged(int nValue)
{
    m_pInternal->m_pPlayer->setVolume(nValue);
}

void QAscVideoView::slotSeekChanged(int nValue)
{
    if (m_pInternal->m_bIsSeekEnabled)
        m_pInternal->m_pPlayer->setSeek(nValue);
}

void QAscVideoView::UpdatePlayPause()
{
    m_pInternal->m_pFooter->SetPlayPause(m_pInternal->m_bIsPlay);
}

void QAscVideoView::slotOpenFile(QString sFile)
{
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

void QAscVideoView::slotPlayerStateChanged(QMediaPlayer::State state)
{
    m_pInternal->m_bIsPlay = (state == QMediaPlayer::PlayingState) ? false : true;
    UpdatePlayPause();

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
