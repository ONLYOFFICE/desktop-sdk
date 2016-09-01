#ifndef QMAINPANEL_H
#define QMAINPANEL_H

#include <QMouseEvent>
#include <QResizeEvent>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include <QLabel>

#include "../../../lib/include/applicationmanager.h"

class CMainButtonsController
{
public:
    virtual void pushButtonMinimizeClicked() {}
    virtual void pushButtonMaximizeClicked() {}
    virtual void pushButtonCloseClicked() {}
};

class QAscMainPanel : public QWidget, public NSEditorApi::CAscMenuEventListener
{
    Q_OBJECT

signals:
    void signal_Download( NSEditorApi::CAscDownloadFileInfo* );
    void signal_Modified(int, bool);
    void signal_OnSave( NSEditorApi::CAscDocumentOnSaveData* );
    void signal_Print(int, int);
    void signal_DialogSave();
    void signal_CheckKeyboard();
    void signal_LocalFile_Open(QString);
    void signal_LocalFile_Create(int);
    void signal_LocalFile_RecentOpen(NSEditorApi::CAscLocalOpenFileRecent_Recover*);
    void signal_LocalFile_SaveDialog(NSEditorApi::CAscLocalSaveFileDialog*);
    void signal_LocalFile_AddImage(NSEditorApi::CAscLocalOpenFileDialog*);
    void signal_LocalFile_OpenPortal(QString);
    void signal_RemoveTab(int);
    void signal_LocalFiles_Open(NSEditorApi::CAscLocalOpenFiles*);
    void signal_OpenLink(QString);
    void signal_SaveWatermark(int);

public:
    QAscMainPanel( QWidget *parent, CAscApplicationManager* pManager, bool bIsCustomWindow );

public slots:
    void pushButtonSaveClicked();
    void pushButtonMainClicked();

    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void pushButtonCloseClicked();

    void slot_Download( NSEditorApi::CAscDownloadFileInfo* );
    void slot_Modified(int, bool);
    void slot_OnSave(NSEditorApi::CAscDocumentOnSaveData*);
    void slot_Print(int, int);
    void slot_DialogSave();
    void slot_CheckKeyboard();
    void slot_LocalFile_Open(QString);
    void slot_LocalFile_Create(int);
    void slot_LocalFile_RecentOpen(NSEditorApi::CAscLocalOpenFileRecent_Recover*);
    void slot_LocalFile_SaveDialog(NSEditorApi::CAscLocalSaveFileDialog*);
    void slot_LocalFile_AddImage(NSEditorApi::CAscLocalOpenFileDialog*);
    void slot_LocalFile_OpenPortal(QString);
    void slot_RemoveTab(int);
    void slot_LocalFiles_Open(NSEditorApi::CAscLocalOpenFiles*);
    void slot_OpenLink(QString);
    void slot_SaveWatermark(int);

public:
    WId GetHwndForKeyboard()
    {
        return ((QWidget*)m_pTabs->parent())->winId();
    }
    void SetMainButtonsController(CMainButtonsController* pController)
    {
        m_pMainButtonsController = pController;
    }

public:
    void resizeEvent(QResizeEvent* event);
    virtual void OnEvent(NSEditorApi::CAscMenuEvent* pEvent);
    virtual bool IsSupportEvent(int nEventType) { return true; }

    void OpenDialogSave(std::wstring sName, unsigned int nId);
    void CheckKeyboard();

    CAscApplicationManager* GetAppManager() { return m_pManager; }

protected:
    void RecalculatePlaces();

protected:
    QPushButton*    m_pButtonMain;
    QWidget*        m_pMainWidget;

    QPushButton*    m_pButtonMinimize;
    QPushButton*    m_pButtonMaximize;
    QPushButton*    m_pButtonClose;

    QPushButton*    m_pSaveModifiedButton;
    bool            m_bIsActivateModified;

    QTabWidget*     m_pTabs;

    QWidget*        m_pSeparator;

    std::wstring    m_sDownloadName;

    CAscApplicationManager* m_pManager;

    std::map<unsigned int, NSEditorApi::CAscDownloadFileInfo*> m_mapDownloads;
    QWidget*        m_pDownloadBackground;
    QLabel*         m_pDownloadLable;

    bool            m_bIsCustomWindow;

    unsigned int    m_nIdCurrentDownloadSaveDialog;

    CMainButtonsController* m_pMainButtonsController;
};

#endif // QMAINPANEL_H
