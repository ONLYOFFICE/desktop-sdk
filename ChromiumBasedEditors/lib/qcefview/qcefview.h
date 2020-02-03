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

#include <QWindow>
#include <QWidget>
#include <QStyleOption>
#include <QCloseEvent>
#include <QDebug>
#include <QPointer>

#include "./../include/cefview.h"
#include "./../include/applicationmanager.h"

class QCefViewProps;
class QCefView : public QWidget, public CCefViewWidgetImpl
{
    Q_OBJECT

Q_SIGNALS:
    void onDropFiles(QList<QString> files);

public:
    QCefView(QWidget* parent, const QSize& initial_size = QSize());
    virtual ~QCefView();

    // focus
    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);

    // move/resize
    virtual void resizeEvent(QResizeEvent* e);
    virtual void moveEvent(QMoveEvent* e);
    virtual void UpdateSize();

    // After create
    virtual void AfterCreate();

    // close
    virtual void closeEvent(QCloseEvent *e);

    // work with cefview
    CCefView* GetCefView();
    void Create(CAscApplicationManager* pManager, CefViewWrapperType eType);
    void CreateReporter(CAscApplicationManager* pManager, CAscReporterData* data);

    // multimedia
    virtual void OnMediaStart(NSEditorApi::CAscExternalMedia* data);
    virtual void OnMediaEnd();

    // events
    virtual void OnLoaded();
    virtual void OnRelease();

    // get natural view
    QWidget* GetViewWidget();

    // background color
    void SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b);
    void paintEvent(QPaintEvent *event);

    // check support z-index
    static bool IsSupportLayers();
    void SetCaptionMaskSize(int);

protected:
    CCefView* m_pCefView;
    QPointer<QWidget> m_pOverride;
    QCefViewProps* m_pProperties;

    void Init();

signals:
    void closeWidget(QCloseEvent *);
    void _loaded();
    void _closed();

protected slots:
    void _loadedSlot();
    void _closedSlot();
};

#if defined (_LINUX) && !defined(_MAC)

class QCefEmbedWindow : public QWindow
{
    Q_OBJECT

private:
    int m_nCaptionSize;

public:
    explicit QCefEmbedWindow(QPointer<QCefView> _qcef_parent, QWindow* _parent = NULL);
    void SetCaptionMaskSize(int);

protected:
    virtual void moveEvent(QMoveEvent*);
    virtual void resizeEvent(QResizeEvent*);
    virtual bool eventFilter(QObject *watched, QEvent *event);   

 private:
    QPointer<QCefView> qcef_parent;
};

#endif

#endif  // CEFCLIENT_QCEFWEBVIEW_H
