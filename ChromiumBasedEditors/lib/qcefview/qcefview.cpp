/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#include "./qcefview.h"

#include <QPainter>

QCefView::QCefView(QWidget* parent, const QSize& initial_size) : QWidget(parent)
{
    m_pCefView = NULL;
    m_pProperties = NULL;

    if (!initial_size.isEmpty())
        resize(initial_size);

    Init();

    QObject::connect(this, SIGNAL( _loaded() ) , this, SLOT( _loadedSlot() ), Qt::QueuedConnection );
    QObject::connect(this, SIGNAL( _closed() ) , this, SLOT( _closedSlot() ), Qt::QueuedConnection );
}

QCefView::~QCefView()
{
    // release from CApplicationManager
}

// focus
void QCefView::focusInEvent(QFocusEvent* e)
{
    if (m_pCefView)
        m_pCefView->focus(true);
}
void QCefView::focusOutEvent(QFocusEvent* e)
{
    return;
    if (m_pCefView)
        m_pCefView->focus(false);
}

// move/resize
void QCefView::resizeEvent(QResizeEvent* e)
{
    if (m_pOverride)
        m_pOverride->setGeometry(0, 0, width(), height());
    if (m_pCefView)
        m_pCefView->resizeEvent();
    cef_width = width();
    cef_height = height();
}
void QCefView::moveEvent(QMoveEvent* e)
{
    if (m_pCefView)
        m_pCefView->moveEvent();
    QWidget::moveEvent(e);
}

// close
void QCefView::closeEvent(QCloseEvent* e)
{
    emit closeWidget(e);
}

CCefView* QCefView::GetCefView()
{
    return m_pCefView;
}

void QCefView::Create(CAscApplicationManager* pManager, CefViewWrapperType eType)
{
    switch (eType)
    {
    case cvwtSimple:
        {
            m_pCefView = pManager->CreateCefView(this);
            break;
        }
    case cvwtEditor:
        {
            m_pCefView = pManager->CreateCefEditor(this);
            break;
        }
    default:
        break;
    }
}

void QCefView::CreateReporter(CAscApplicationManager* pManager, CAscReporterData* data)
{
    m_pCefView = pManager->CreateCefPresentationReporter(this, data);
}

void QCefView::OnMediaStart(NSEditorApi::CAscExternalMedia* data)
{
}
void QCefView::OnMediaEnd()
{
}

// events
void QCefView::OnRelease()
{
    emit _closed();
}
void QCefView::OnLoaded()
{
    emit _loaded();
}

// slots
void QCefView::_loadedSlot()
{
}
void QCefView::_closedSlot()
{
    this->OnMediaEnd();
}

// get natural view
QWidget* QCefView::GetViewWidget()
{
    return m_pOverride ? m_pOverride : this;
}

// background color
void QCefView::SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b)
{
}

#ifdef _WIN32

void QCefView::Init()
{
    cef_handle = reinterpret_cast<WindowHandleId>(winId());
    //cef_ex_style = WS_EX_NOACTIVATE;
    cef_width = width();
    cef_height = height();
}

void QCefView::UpdateSize()
{
    HWND _parent = reinterpret_cast<HWND>(winId());
    HWND _child = GetWindow(_parent, GW_CHILD);
    SetWindowPos(_child, _parent, 0, 0, width(), height(), SWP_NOZORDER);
}

void QCefView::AfterCreate()
{
}

#endif

#if defined (_LINUX) && !defined(_MAC)
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include <QMimeData>

class QCefViewProps
{
public:
    QWindow* m_window;
};

// OVERRIDE WIDGET
QCefEmbedWindow::QCefEmbedWindow(QPointer<QCefView> _qcef_parent, QWindow* _parent) : QWindow(_parent), qcef_parent(_qcef_parent)
{
    this->installEventFilter(this);
}

void QCefEmbedWindow::resizeEvent(QResizeEvent* e)
{
    if (qcef_parent)
        qcef_parent->UpdateSize();
}
void QCefEmbedWindow::moveEvent(QMoveEvent* e)
{
    if (qcef_parent)
        qcef_parent->UpdateSize();
}

#include <QDebug>
bool QCefEmbedWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (this != watched)
        return false;

    switch (event->type())
    {
        case QEvent::DragEnter:
        {
            if (!qcef_parent)
                return false;

            QDragEnterEvent* e = (QDragEnterEvent*)event;
            QList<QUrl> urls = e->mimeData()->urls();

            bool isSupport = false;
            QSet<QString> _exts;
            _exts << "docx" << "doc" << "odt" << "rtf" << "txt" << "doct" << "dotx" << "ott";
            _exts << "html" << "mht" << "epub";
            _exts << "pptx" << "ppt" << "odp" << "ppsx" << "pptt" << "potx" << "otp";
            _exts << "xlsx" << "xls" << "ods" << "csv" << "xlst" << "xltx" << "ots";
            _exts << "pdf" << "djvu" << "xps";
            _exts << "plugin";

            for (int i = 0; i < urls.length(); i++)
            {
                QFileInfo oInfo(urls[i].toString());
                if (!_exts.contains(oInfo.suffix()))
                {
                    isSupport = false;
                    break;
                }
                isSupport = true;
            }

            if (isSupport)
                e->acceptProposedAction();
            else
            {
                e->setDropAction(Qt::IgnoreAction);
                e->accept();
            }

            return true;
        }
        case QEvent::Drop:
        {
            if (!qcef_parent)
                return false;

            QDropEvent* e = (QDropEvent*)event;
            QList<QUrl> urls = e->mimeData()->urls();

            QList<QString> files;
            for (int i = 0; i < urls.length(); i++)
            {
                QString qpath = urls[i].path();
                std::wstring path = qpath.toStdWString();

                std::wstring::size_type nPosPluginExt = path.rfind(L".plugin");
                std::wstring::size_type nUrlLen = path.length();
                if ((nPosPluginExt != std::wstring::npos) && ((nPosPluginExt + 7) == nUrlLen))
                {
                    // register plugin
                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                    pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
                    NSEditorApi::CAscAddPlugin* pData = new NSEditorApi::CAscAddPlugin();
                    pData->put_Path(path);
                    pEvent->m_pData = pData;

                    qcef_parent->GetCefView()->GetAppManager()->Apply(pEvent);
                }
                else
                {
                    files.push_back(qpath);
                }
            }

            if (files.length() > 0)
            {
                emit qcef_parent->onDropFiles(files);
            }

            e->acceptProposedAction();
            return true;
        }
    default:
        break;
    }

    return QWindow::eventFilter(watched, event);
}

void QCefView::Init()
{
    QWindow* win = new QCefEmbedWindow(this);

    QCefViewProps* props = new QCefViewProps();
    props->m_window = win;
    m_pProperties = props;

    cef_handle = (WindowHandleId)(win->winId());
    cef_width = width();
    cef_height = height();
}

void QCefView::AfterCreate()
{
    QCefViewProps* props = (QCefViewProps*)m_pProperties;
    m_pOverride = QWidget::createWindowContainer(props->m_window, this);
    delete m_pProperties;
    m_pProperties = NULL;

    connect(m_pOverride.operator ->(), &QWidget::destroyed, this, [=](QObject*) {
        deleteLater();
    });
}

#include <X11/Xlib.h>

Window GetChild(Window parent)
{
    Display* xdisplay = (Display*)CefGetXDisplay();
    Window root_ret;
    Window parent_ret;
    Window* children_ret;
    unsigned int child_count_ret;
    Status status = XQueryTree(xdisplay, parent, &root_ret, &parent_ret, &children_ret, &child_count_ret);
    Window ret = 0;
    if (status != 0 && child_count_ret > 0)
    {
        ret = children_ret[0];
        XFree(children_ret);
    }
    return ret;
}

void QCefView::UpdateSize()
{
    Window child = GetChild(cef_handle);
    if (child > 0)
    {
        Display* xdisplay = (Display*)CefGetXDisplay();
        XWindowChanges changes = {};
        changes.x = 0;
        changes.y = 0;
        changes.width = width();
        changes.height = height();
        XConfigureWindow(xdisplay,
                         child,
                         CWX | CWY | CWHeight | CWWidth,
                         &changes);
    }
}

#endif
