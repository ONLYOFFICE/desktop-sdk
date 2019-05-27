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

#ifndef CEFCLIENT_QCEFWEBVIEW_H
#define CEFCLIENT_QCEFWEBVIEW_H

#include <QWidget>
#include <QStyleOption>
#include <QCloseEvent>
#include <QDebug>

#include "./../include/cefview.h"
#include "./../include/applicationmanager.h"

class QCefView : public QWidget, public CCefViewWidgetImpl
{
    Q_OBJECT

signals:
    void _loaded();
    void _closed();

public:
    QCefView(QWidget* parent);
    virtual ~QCefView();

    virtual void paintEvent(QPaintEvent *);
    void SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b);

    virtual void focusInEvent(QFocusEvent* e);
    virtual void resizeEvent(QResizeEvent* e);
    virtual void moveEvent(QMoveEvent* e);
    virtual void closeEvent(QCloseEvent *);

    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);

#if 0
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dropEvent(QDropEvent *e);
#endif

    CCefView* GetCefView();
    void Create(CAscApplicationManager* pManager, CefViewWrapperType eType);
    void CreateReporter(CAscApplicationManager* pManager, CAscReporterData* data);

    virtual void OnMediaStart(NSEditorApi::CAscExternalMedia* data);
    virtual void OnMediaEnd();

    virtual void releaseFromChild();

protected:
    CCefView* m_pCefView;
    QWidget* m_pLoader;

public:
    // CCefViewWidgetImpl
    virtual int parent_x();
    virtual int parent_y();
    virtual int parent_width();
    virtual int parent_height();
    virtual WindowHandleId parent_wid();
    virtual bool parent_window_is_empty();
    virtual void child_loaded();

signals:
    void closeWidget(QCloseEvent *);

protected slots:

    void _loadedSlot();
    void _closedSlot();
};

#endif  // CEFCLIENT_QCEFWEBVIEW_H
