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

#include "./../include/qcefview.h"
#include <QPainter>
#include <QApplication>

class QCefViewProps
{
public:
	QWindow* m_window;
public:
	QCefViewProps()
	{
		m_window = NULL;
	}
};

QCefView::QCefView(QWidget* parent, const QSize& initial_size) : QWidget(parent)
{
	m_pCefView = NULL;
	m_pProperties = NULL;

	if (!initial_size.isEmpty())
		resize(initial_size);

	QObject::connect(this, SIGNAL( _loaded() ) , this, SLOT( _loadedSlot() ), Qt::QueuedConnection );
	QObject::connect(this, SIGNAL( _closed() ) , this, SLOT( _closedSlot() ), Qt::QueuedConnection );

	if (IsSupportLayers())
		this->installEventFilter(this);
}

QCefView::~QCefView()
{
	// release from CApplicationManager
	if (m_pProperties)
	{
		delete m_pProperties;
		m_pProperties = NULL;
	}
}

bool QCefView::eventFilter(QObject *watched, QEvent *event)
{
	if (this == watched && event->type() == QEvent::Resize)
		OnMediaEnd(true);

	return QWidget::eventFilter(watched, event);
}

bool QCefView::setFocusToCef()
{
	if (!m_pCefView)
		return false;

	if (m_pCefView->IsDestroy())
		return false;

	bool isActivate = false;
#ifdef _WIN32
	HWND hwndForeground = ::GetForegroundWindow();
	HWND hwndQCef = (HWND)this->winId();
	if (::IsChild(hwndForeground, hwndQCef))
		isActivate = true;
#endif

#if defined (_LINUX) && !defined(_MAC)
	// TODO: check foreground window
	isActivate = true;
#endif

	if (isActivate)
		m_pCefView->focus(true);

	//qDebug() << "focus: id = " << m_pCefView->GetId() << ", use = " << isActivate;
	return isActivate;
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
	cef_width = width();
	cef_height = height();

	if (m_pOverride)
		m_pOverride->setGeometry(0, 0, cef_width, cef_height);
	if (m_pCefView)
		m_pCefView->resizeEvent();
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
	Init();
}

void QCefView::CreateReporter(CAscApplicationManager* pManager, CAscReporterData* data)
{
	Init();
	m_pCefView = pManager->CreateCefPresentationReporter(this, data);
}

void QCefView::OnMediaStart(NSEditorApi::CAscExternalMedia* data)
{
}
void QCefView::OnMediaEnd(bool isFromResize)
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
	QString sR = QString::number((int)r, 16);
	QString sG = QString::number((int)g, 16);
	QString sB = QString::number((int)b, 16);
	if (sR.length() < 2)
		sR = "0" + sR;
	if (sG.length() < 2)
		sG = "0" + sG;
	if (sB.length() < 2)
		sB = "0" + sB;

	QString sColor = sR + sG + sB;
	QString sStyle = "background-color:#" + sColor + ";";
	this->setStyleSheet(sStyle);
}

void QCefView::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
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

	int nW = width();
	int nH = height();
	if (CAscApplicationManager::IsUseSystemScaling() && nW > 2 && nH > 2 && m_pCefView && m_pCefView->isDoubleResizeEvent())
		SetWindowPos(_child, _parent, 0, 0, nW - 1, nH - 1, SWP_NOZORDER);

	SetWindowPos(_child, _parent, 0, 0, nW, nH, SWP_NOZORDER);
}

void QCefView::AfterCreate()
{
}

bool QCefView::IsSupportLayers()
{
	return true;
}
void QCefView::SetCaptionMaskSize(int)
{
	// not using
}

#endif

#if defined (_LINUX) && !defined(_MAC)
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include <QMimeData>

// OVERRIDE WIDGET
QCefEmbedWindow::QCefEmbedWindow(QPointer<QCefView> _qcef_parent, QWindow* _parent) : QWindow(_parent), qcef_parent(_qcef_parent)
{
	m_nCaptionSize = 0;
	this->installEventFilter(this);
}

void QCefEmbedWindow::resizeEvent(QResizeEvent* e)
{
	if (0 != m_nCaptionSize)
	{
		setMask(QRegion());
		setMask(QRegion(0, m_nCaptionSize, width(), height() - m_nCaptionSize));
	}

	if (qcef_parent)
		qcef_parent->UpdateSize();
}
void QCefEmbedWindow::moveEvent(QMoveEvent* e)
{
	if (qcef_parent)
		qcef_parent->UpdateSize();
}

void QCefEmbedWindow::SetCaptionMaskSize(int size)
{
	if (m_nCaptionSize == size)
		return;
	m_nCaptionSize = size;
	if (0 == m_nCaptionSize)
		setMask(QRegion());
}

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

#include <X11/Xlib.h>

void QCefView::Init()
{
	if (IsSupportLayers())
	{
		Display* display = (Display*)CefGetXDisplay();
		Window x11root = XDefaultRootWindow(display);
		Window x11w = XCreateSimpleWindow(display, x11root, 0, 0, width(), height(), 0, 0,
										  (m_pCefView && m_pCefView->GetType() != cvwtEditor) ? 0xFFFFFFFF : 0xFFF4F4F4);
		XReparentWindow(display, x11w, this->winId(), 0, 0);
		XMapWindow(display, x11w);
		XDestroyWindow(display, x11root);
		cef_handle = x11w;
	}
	else
	{
		QWindow* win = new QCefEmbedWindow(this);
		m_pProperties = new QCefViewProps();
		m_pProperties->m_window = win;
		cef_handle = (WindowHandleId)(win->winId());
	}
	cef_width = width();
	cef_height = height();
}

void QCefView::AfterCreate()
{
	if (IsSupportLayers())
		return;
	m_pOverride = QWidget::createWindowContainer(m_pProperties->m_window, this);
	connect(m_pOverride.operator ->(), &QWidget::destroyed, this, [=](QObject*) {
		deleteLater();
	});
}

bool QCefView::IsSupportLayers()
{
	return true;
}
void QCefView::SetCaptionMaskSize(int size)
{
	if (m_pProperties && m_pProperties->m_window)
		((QCefEmbedWindow*)m_pProperties->m_window)->SetCaptionMaskSize(size);
}

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

void SetWindowSize(Window window, QWidget* parent)
{
	if (window > 0)
	{
		Display* xdisplay = (Display*)CefGetXDisplay();
		XWindowChanges changes = {};
		changes.x = 0;
		changes.y = 0;
		changes.width = parent->width();
		changes.height = parent->height();
		XConfigureWindow(xdisplay,
						 window,
						 CWX | CWY | CWHeight | CWWidth,
						 &changes);
	}
}

void QCefView::UpdateSize()
{
	if (IsSupportLayers())
		SetWindowSize(cef_handle, this);
	SetWindowSize(GetChild(cef_handle), this);
}

#endif
