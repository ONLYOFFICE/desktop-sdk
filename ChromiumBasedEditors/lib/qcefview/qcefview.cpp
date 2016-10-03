/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

QCefView::QCefView(QWidget* parent) : QWidget(parent)
{
    m_pCefView = NULL;

    m_pLoader = new QWidget(this);
    m_pLoader->setGeometry(0, 0, width(), height());

    SetBackgroundCefColor(255, 255, 255);

    m_pLoader->setHidden(false);

    QObject::connect(this, SIGNAL( _loaded() ) , this, SLOT( _loadedSlot() ), Qt::QueuedConnection );
}

QCefView::~QCefView()
{
    // release from CApplicationManager
}

void QCefView::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

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
    m_pLoader->setStyleSheet(sStyle);
}

void QCefView::focusInEvent(QFocusEvent* e)
{
    if (NULL != m_pCefView)
        m_pCefView->focus();
}

void QCefView::resizeEvent(QResizeEvent* e)
{
    if (NULL != e)
        QWidget::resizeEvent(e);
    m_pLoader->setGeometry(0, 0, width(), height());
    if (NULL != m_pCefView)
        m_pCefView->resizeEvent();
}

void QCefView::moveEvent(QMoveEvent* e)
{
    if (NULL != e)
        QWidget::moveEvent(e);

    if (NULL != m_pCefView)
        m_pCefView->moveEvent();
}

void QCefView::closeEvent(QCloseEvent* e)
{
    emit closeWidget(e);
}

bool QCefView::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (m_pCefView)
    {
        if (m_pCefView->nativeEvent(NULL, 0, message, result))
            return true;
    }

    return QWidget::nativeEvent(eventType, message, result);
}

#if 0
void QCefView::dragEnterEvent(QDragEnterEvent *e)
{
    m_pCefView->dragEnterEvent(e);
}
void QCefView::dragMoveEvent(QDragMoveEvent *e)
{
    m_pCefView->dragMoveEvent(e);
}
void QCefView::dragLeaveEvent(QDragLeaveEvent *e)
{
    m_pCefView->dragLeaveEvent(e);
}
void QCefView::dropEvent(QDropEvent *e)
{
    m_pCefView->dropEvent(e);
}
#endif

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

// CCefViewWidgetImpl
int QCefView::parent_x() { return this->pos().x(); }
int QCefView::parent_y() { return this->pos().y(); }
int QCefView::parent_width() { return this->width() * this->devicePixelRatio(); }
int QCefView::parent_height() { return this->height() * this->devicePixelRatio(); }
WindowHandleId QCefView::parent_wid()
{
#if 1
    return (WindowHandleId)this->winId();
#else
    return (WindowHandleId)m_pLoader->winId();
#endif
}
bool QCefView::parent_window_is_empty() { return true; }
void QCefView::child_loaded()
{
    emit _loaded();
}

void QCefView::_loadedSlot()
{
    if (!m_pLoader->isHidden())
        m_pLoader->setHidden(true);
}
