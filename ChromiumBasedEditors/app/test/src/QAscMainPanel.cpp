#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QScrollArea>
#include <QFileDialog>

#include "QAscTabWidget.h"

#include <QStandardPaths>

#include "QAscMainPanel.h"
#include "../../../lib/include/qascprinter.h"

#include <QDesktopServices>

#include <QDebug>

QAscMainPanel::QAscMainPanel( QWidget* parent, CAscApplicationManager* pManager, bool bIsCustomWindow ) : QWidget( parent )
{
    m_bIsCustomWindow = bIsCustomWindow;
    m_pMainButtonsController = NULL;
    m_nIdCurrentDownloadSaveDialog = 0;

    QObject::connect(this, SIGNAL( signal_Download(NSEditorApi::CAscDownloadFileInfo*)), this, SLOT( slot_Download(NSEditorApi::CAscDownloadFileInfo*)), Qt::QueuedConnection );
    QObject::connect(this, SIGNAL( signal_Modified(int,bool)) , this, SLOT( slot_Modified(int,bool)), Qt::QueuedConnection );
    QObject::connect(this, SIGNAL( signal_Print(int,int)) , this, SLOT( slot_Print(int,int)), Qt::QueuedConnection );
    QObject::connect(this, SIGNAL( signal_DialogSave()), this, SLOT( slot_DialogSave()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL( signal_CheckKeyboard()), this, SLOT( slot_CheckKeyboard()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL( signal_LocalFile_Open(QString)), this, SLOT( slot_LocalFile_Open(QString)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL( signal_LocalFile_RecentOpen(NSEditorApi::CAscLocalOpenFileRecent_Recover*)),
                     this, SLOT( slot_LocalFile_RecentOpen(NSEditorApi::CAscLocalOpenFileRecent_Recover*)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_LocalFile_Create(int)), this, SLOT( slot_LocalFile_Create(int)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_LocalFile_SaveDialog(NSEditorApi::CAscLocalSaveFileDialog*)),
                     this, SLOT( slot_LocalFile_SaveDialog(NSEditorApi::CAscLocalSaveFileDialog*)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_LocalFile_AddImage(NSEditorApi::CAscLocalOpenFileDialog*)),
                     this, SLOT( slot_LocalFile_AddImage(NSEditorApi::CAscLocalOpenFileDialog*)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_LocalFile_OpenPortal(QString)),
                     this, SLOT( slot_LocalFile_OpenPortal(QString)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_RemoveTab(int)) , this, SLOT( slot_RemoveTab(int)), Qt::QueuedConnection );

    QObject::connect(this, SIGNAL( signal_LocalFiles_Open(NSEditorApi::CAscLocalOpenFiles*)),
                     this, SLOT( slot_LocalFiles_Open(NSEditorApi::CAscLocalOpenFiles*)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_OnSave(NSEditorApi::CAscDocumentOnSaveData*)),
                     this, SLOT( slot_OnSave(NSEditorApi::CAscDocumentOnSaveData*)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_OpenLink(QString)),
                     this, SLOT( slot_OpenLink(QString)), Qt::QueuedConnection);

    QObject::connect(this, SIGNAL( signal_SaveWatermark(int)),
                     this, SLOT( slot_SaveWatermark(int)), Qt::QueuedConnection);

    m_pManager = pManager;

    setObjectName( "mainPanel" );

    QGridLayout *mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing( 0 );
    mainGridLayout->setMargin( 0 );
    setLayout( mainGridLayout );

    // Central widget
    QWidget *centralWidget = new QWidget( this );
    centralWidget->setObjectName( "centralWidget" );
    centralWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    //centralWidget->setStyleSheet("background-color:#313437");

    QAscTabWidget* pTabs = new QAscTabWidget(centralWidget);
    pTabs->setGeometry(0, 0, centralWidget->width(), centralWidget->height());
    pTabs->m_pManager = m_pManager;

    m_pTabs = (QTabWidget*)pTabs;

    m_pSaveModifiedButton = new QPushButton("", centralWidget);

    m_pSaveModifiedButton->setObjectName( "pushButtoneModified" );
    QObject::connect( m_pSaveModifiedButton, SIGNAL( clicked() ), this, SLOT( pushButtonSaveClicked()) );

    m_pSaveModifiedButton->setStyleSheet("background-color:#FFFFFF;border:none;margin:0;padding:0;");
    m_bIsActivateModified = false;

#if 0
    // прячем эту кнопку
    m_pSaveModifiedButton->setHidden(true);
#endif

    if (m_bIsCustomWindow)
    {
        // Minimize
        m_pButtonMinimize = new QPushButton( "", centralWidget );
        m_pButtonMinimize->setObjectName( "pushButtonMinimize" );
        QObject::connect( m_pButtonMinimize, SIGNAL( clicked() ), this, SLOT( pushButtonMinimizeClicked() ) );

        m_pButtonMinimize->setStyleSheet("QPushButton {background-image:url(:/Icons/new_collapse_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::hover {background-image:url(:/Icons/new_collapse_hover.png);border:none;margin:0;padding:0;}\
    QPushButton::pressed {background-image:url(:/Icons/new_collapse_hover.png);border:none;margin:0;padding:0;}");

        // Maximize
        m_pButtonMaximize = new QPushButton( "", centralWidget );
        m_pButtonMaximize->setObjectName( "pushButtonMaximize" );
        QObject::connect( m_pButtonMaximize, SIGNAL( clicked() ), this, SLOT( pushButtonMaximizeClicked() ) );

        m_pButtonMaximize->setStyleSheet("QPushButton {background-image:url(:/Icons/new_deploy_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::hover {background-image:url(:/Icons/new_deploy_hover.png);border:none;margin:0;padding:0;}\
    QPushButton::pressed {background-image:url(:/Icons/new_deploy_hover.png);border:none;margin:0;padding:0;}");

        // Close
        m_pButtonClose = new QPushButton( "", centralWidget );
        m_pButtonClose->setObjectName( "pushButtonClose" );
        QObject::connect( m_pButtonClose, SIGNAL( clicked() ), this, SLOT( pushButtonCloseClicked() ) );

        m_pButtonClose->setStyleSheet("QPushButton {background-image:url(:/Icons/new_exit_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::hover {background-image:url(:/Icons/new_exit_hover.png);border:none;margin:0;padding:0;}\
    QPushButton::pressed {background-image:url(:/Icons/new_exit_hover.png);border:none;margin:0;padding:0;}");
    }
    else
    {
        m_pButtonMinimize = NULL;
        m_pButtonMaximize = NULL;
        m_pButtonClose = NULL;
    }

    // Main
    m_pButtonMain = new QPushButton( "", centralWidget );
    m_pButtonMain->setObjectName( "pushButtonMain" );
    QObject::connect( m_pButtonMain, SIGNAL( clicked() ), this, SLOT( pushButtonMainClicked() ) );

    m_pButtonMain->setStyleSheet("QPushButton {background-image:url(:/Icons/Documents_active_normal.png);border:none;margin:0;padding:0;}\
QPushButton::hover {background-image:url(:/Icons/Documents_active_normal.png);border:none;margin:0;padding:0;}\
QPushButton::pressed {background-image:url(:/Icons/Documents_active_hover.png);border:none;margin:0;padding:0;}");

    QCefView* pMainWidget = new QCefView(centralWidget);
    pMainWidget->Create(m_pManager, cvwtSimple);
    pMainWidget->setObjectName( "mainPanel" );
    pMainWidget->setHidden(false);

    std::wstring sLocalPath = L"";

    //sLocalPath = L"https://ascdesktop.teamlab.info/products/files/?desktop=true";
    //sLocalPath = L"https://testinfo.teamlab.info/products/files/?desktop=true";
    //sLocalPath = L"https://denisdenis.teamlab.info/products/files/?desktop=true";
    //sLocalPath = L"https://denismy.teamlab.info/products/files/?desktop=true";
    //sLocalPath = L"file:///C:/Users/oleg.korshul/Desktop/desktop/index.html";
    //sLocalPath = L"file:///C:/Users/oleg.korshul/Desktop/desktop2/index.html";
    //sLocalPath = L"https://koroleg.teamlab.info/products/files/?desktop=true";
    //sLocalPath = L"http://isa2:8086";

#if 1
    sLocalPath = pManager->m_oSettings.file_converter_path + L"/../editors/index.html";
    //std::wstring sLocalPath = pManager->m_oSettings.file_converter_path + L"/../editors/local.html";
    NSCommon::string_replace(sLocalPath, L"\\", L"/");
    if (0 == sLocalPath.find(L"/"))
        sLocalPath = L"file://" + sLocalPath;
    else
        sLocalPath = L"file:///" + sLocalPath;
#endif

    m_pMainWidget = (QWidget*)pMainWidget;
    pTabs->m_pMainWidget = m_pMainWidget;
    pTabs->m_pMainButton = m_pButtonMain;

    m_pSeparator = new QWidget(centralWidget);
    m_pSeparator->setObjectName("separator");
    m_pSeparator->setStyleSheet("background-color:#D6D6D7");
    m_pSeparator->setGeometry(0, 28, this->width(), 1);

    mainGridLayout->addWidget( centralWidget );

    m_pDownloadBackground = new QWidget(centralWidget);
    m_pDownloadBackground->setObjectName("downloadBackground");
    m_pDownloadBackground->setStyleSheet("background-color:#646464;");

    m_pDownloadLable = new QLabel(m_pDownloadBackground);
    m_pDownloadLable->setObjectName("downloadLable");

#if 1
    QPalette palette;

    //white text
    QBrush brush(QColor(0, 0, 0, 255));
    brush.setStyle(Qt::SolidPattern);

    //black background
    QBrush brush1(QColor(0x64, 0x64, 0x64, 255));
    brush1.setStyle(Qt::SolidPattern);

    //set white text
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);

    //set black background
    palette.setBrush(QPalette::Active, QPalette::Window, brush1);
    palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);

    //set palette
    m_pDownloadLable->setPalette(palette);

    //set font
    QFont font;
    font.setPixelSize(32);
    font.setBold(true);

    m_pDownloadLable->setFont(font);
    m_pDownloadLable->setAutoFillBackground(true);

    m_pDownloadLable->setText("Oleg");
#endif

    m_pDownloadBackground->setHidden(true);

    RecalculatePlaces();

    m_pManager->SetEventListener(this);
    pMainWidget->GetCefView()->load(sLocalPath);
}

void QAscMainPanel::RecalculatePlaces()
{
    int nWindowW = this->width();
    int nWindowH = this->height();
    int nCaptionH = 29;

    m_pTabs->setGeometry(0, 0, nWindowW, nWindowH);
    m_pButtonMain->setGeometry(0, 0, 108, nCaptionH);

    m_pSeparator->setGeometry(0, 28, this->width(), 1);

    int nStartOffset = 12;
    int nBetweenApp = 12;
    int nButtonW = 12;
    int nY = (nCaptionH - nButtonW) >> 1;
    nY = 5;

    if (m_bIsCustomWindow)
    {
        m_pButtonClose->setGeometry(nWindowW - nStartOffset - nButtonW, nY, nButtonW, nButtonW);
        m_pButtonMaximize->setGeometry(nWindowW - nStartOffset - 2 * nButtonW - nBetweenApp, nY, nButtonW, nButtonW);
        m_pButtonMinimize->setGeometry(nWindowW - nStartOffset - 3 * nButtonW - 2 * nBetweenApp, nY, nButtonW, nButtonW);

        m_pSaveModifiedButton->setGeometry(nWindowW - nStartOffset - 4 * nButtonW - 3 * nBetweenApp, nY, nButtonW, nButtonW);
    }
    else
    {
        m_pSaveModifiedButton->setGeometry(nWindowW - nStartOffset - nButtonW, nY, nButtonW, nButtonW);
    }

    m_pMainWidget->setGeometry(0, nCaptionH, nWindowW, nWindowH - nCaptionH);

    m_pDownloadBackground->setGeometry(0, nWindowH - 100, nWindowW, 100);
    m_pDownloadLable->setGeometry(0, 0, nWindowW, 100);
}

void QAscMainPanel::pushButtonSaveClicked()
{
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();

#if 0
    // licence
    if (false)
    {
        NSEditorApi::CAscLicenceKey* pData = new NSEditorApi::CAscLicenceKey();
        pData->put_Path(L"D:\\test");
        pData->put_ProductId(302);
        pData->put_Key("e030372e-4222-4ff1-ae3d-dc9c2db6f1fd");

        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_SEND_KEY;
        pEvent->m_pData = pData;

        m_pManager->Apply(pEvent);
    }
    else
    {
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_FREE;

        NSEditorApi::CAscLicenceActual* pData = new NSEditorApi::CAscLicenceActual();
        pData->put_Path(L"D:\\test");
        pData->put_ProductId(302);

        pEvent->m_pData = pData;

        m_pManager->Apply(pEvent);
    }
    return;
#endif

#if 0
    CCefView* pView = m_pManager->GetViewById(1);
    pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS;

    NSEditorApi::CAscExecCommandJS* pData = new NSEditorApi::CAscExecCommandJS();
    pData->put_Command(L"command_test");
    pData->put_Param(L"param_test");

    pEvent->m_pData = pData;
    pView->Apply(pEvent);

    return;
#endif

#if 0
    int nCount = m_pTabs->tabBar()->count();
    for (int i = 0; i < nCount; ++i)
    {
        if (i != m_pTabs->tabBar()->currentIndex())
        {
            CCefView* pCefView = ((QCefView*)m_pTabs->widget(m_pTabs->tabBar()->currentIndex()))->GetCefView();
            pCefView->focus(false);
        }
    }
    return;
#endif

#if 0
    if (-1 != m_pTabs->tabBar()->currentIndex())
    {
        CCefView* pCefView = ((QCefView*)m_pTabs->widget(m_pTabs->tabBar()->currentIndex()))->GetCefView();
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_START;
        pCefView->Apply(pEvent);
    }
#endif

    if (!m_bIsActivateModified)
        return;

    bool bIsMain = !m_pMainWidget->isHidden();
    if (bIsMain)
        return;

    if (-1 == m_pTabs->tabBar()->currentIndex())
        return;

    CCefView* pCefView = ((QCefView*)m_pTabs->widget(m_pTabs->tabBar()->currentIndex()))->GetCefView();
    pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SAVE;
    pCefView->Apply(pEvent);
}

void QAscMainPanel::pushButtonMainClicked()
{
    // test logout
    //m_pManager->Logout(L"ascdesktop.teamlab.info");

    if (!m_pMainWidget->isHidden())
        return;

    m_pMainWidget->setHidden(false);

    if (m_pMainWidget->isHidden())
    {
        m_pButtonMain->setStyleSheet("QPushButton {background-image:url(:/Icons/Documents_disabled_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::hover {background-image:url(:/Icons/Documents_disabled_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::pressed {background-image:url(:/Icons/Documents_disabled_hover.png);border:none;margin:0;padding:0;}");

        ((QCefView*)m_pMainWidget)->GetCefView()->focus();
    }
    else
    {
        m_pButtonMain->setStyleSheet("QPushButton {background-image:url(:/Icons/Documents_active_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::hover {background-image:url(:/Icons/Documents_active_normal.png);border:none;margin:0;padding:0;}\
    QPushButton::pressed {background-image:url(:/Icons/Documents_active_hover.png);border:none;margin:0;padding:0;}");

        ((QAscTabWidget*)m_pTabs)->FireFocus();
        m_pTabs->setCurrentIndex(-1);
    }
}

void QAscMainPanel::pushButtonMinimizeClicked()
{
    if (m_pMainButtonsController)
        m_pMainButtonsController->pushButtonMinimizeClicked();
}
void QAscMainPanel::pushButtonMaximizeClicked()
{
    if (m_pMainButtonsController)
        m_pMainButtonsController->pushButtonMaximizeClicked();
}
void QAscMainPanel::pushButtonCloseClicked()
{
    if (m_pMainButtonsController)
        m_pMainButtonsController->pushButtonCloseClicked();
}

void QAscMainPanel::slot_Download(NSEditorApi::CAscDownloadFileInfo* pInfo)
{
    unsigned int nIdDownload = pInfo->get_IdDownload();
    if (pInfo->get_IsComplete())
    {
        // файл докачался, нужно удалить его из списка
        std::map<unsigned int, NSEditorApi::CAscDownloadFileInfo*>::iterator i = m_mapDownloads.find(nIdDownload);

        if (i != m_mapDownloads.end())
        {
            RELEASEINTERFACE((i->second));
            m_mapDownloads.erase(i);
        }
    }
    else
    {
        std::map<unsigned int, NSEditorApi::CAscDownloadFileInfo*>::iterator i = m_mapDownloads.find(nIdDownload);

        if (i != m_mapDownloads.end())
        {
            NSEditorApi::CAscDownloadFileInfo* pCurrent = i->second;
            pCurrent->put_Percent(pInfo->get_Percent());
            pCurrent->put_Speed(pInfo->get_Speed());
            pCurrent->put_IsComplete(pInfo->get_IsComplete());
        }
        else
        {
            if (!pInfo->get_FilePath().empty())
            {
                ADDREFINTERFACE(pInfo);
                m_mapDownloads.insert(std::pair<unsigned int, NSEditorApi::CAscDownloadFileInfo*>(nIdDownload, pInfo));
            }
        }
    }

    RELEASEINTERFACE(pInfo);

    // теперь смотрим, нужно ли показывать загрузки
    if (0 == m_mapDownloads.size())
    {
        m_pDownloadBackground->setHidden(true);
        return;
    }

    m_pDownloadBackground->setHidden(false);
    QString sText = "";

    for (std::map<unsigned int, NSEditorApi::CAscDownloadFileInfo*>::iterator i = m_mapDownloads.begin(); i != m_mapDownloads.end(); i++)
    {
        NSEditorApi::CAscDownloadFileInfo* pInfo = i->second;

        std::wstring sName = pInfo->get_FilePath();
        int n1 = sName.find_last_of((wchar_t)'\\');
        int n2 = sName.find_last_of((wchar_t)'/');

        int n = -1;
        if (n1 != std::wstring::npos)
            n = n1;
        if (n2 != std::wstring::npos && n2 > n1)
            n = n2;

        if (-1 == n)
            continue;

        sName = sName.substr(n + 1);

        sName += L" : %1; ";

        QString ss = QString::fromStdWString(sName);
        int nPercent = pInfo->get_Percent();
        if (nPercent < 0)
            nPercent = 0;

        sText += ss.arg(nPercent);
    }

    m_pDownloadLable->setText(sText);
}

void QAscMainPanel::slot_Modified(int id, bool value)
{
    bool bIsMain = !m_pMainWidget->isHidden();
    if (bIsMain)
        return;

    int nTabIndex = ((QAscTabWidget*)m_pTabs)->GetTabIndexById(id);
    if (nTabIndex == m_pTabs->tabBar()->currentIndex())
    {
        if (value == true)
        {
            if (!m_bIsActivateModified)
            {
                m_pSaveModifiedButton->setStyleSheet("background-color:#00FF00;border:none;margin:0;padding:0;");
                m_bIsActivateModified = true;
            }
        }
        else if (m_bIsActivateModified)
        {
            m_pSaveModifiedButton->setStyleSheet("background-color:#FFFFFF;border:none;margin:0;padding:0;");
            m_bIsActivateModified = false;
        }
    }
}

void QAscMainPanel::slot_OnSave(NSEditorApi::CAscDocumentOnSaveData* pData)
{
    bool bIsCancel = pData->get_IsCancel();
    int nId = pData->get_Id();

    if (((QAscTabWidget*)m_pTabs)->m_nCloseCefId == nId)
    {
        ((QAscTabWidget*)m_pTabs)->m_nCloseCefId = -1;
        if (!bIsCancel)
        {
            // закрываем таб
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_ONCLOSE;

            NSEditorApi::CAscTypeId* pDataId = new NSEditorApi::CAscTypeId();
            pDataId->put_Id(nId);
            pEvent->m_pData = pDataId;

            if (NULL != m_pManager->GetEventListener())
                m_pManager->GetEventListener()->OnEvent(pEvent);
        }
    }
}

void QAscMainPanel::slot_Print(int id, int pagesCount)
{
    QAscPrinterContext* pContext = new QAscPrinterContext();

    //pagesCount = 1;

    pContext->getPrinter()->setFromTo(1, pagesCount);

    QPrintDialog *dialog = new QPrintDialog(pContext->getPrinter(), NULL);
    dialog->setWindowTitle(tr("Print Document"));
    int res = dialog->exec();

    CCefView* pView = m_pManager->GetViewById(id);

    if (res == QDialog::Accepted)
    {
        if (pContext->BeginPaint())
        {
            for (int i = 0; i < pagesCount; ++i)
            {
                pContext->AddRef();

                NSEditorApi::CAscPrintPage* pData = new NSEditorApi::CAscPrintPage();
                pData->put_Context(pContext);
                pData->put_Page(i);

                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE;
                pEvent->m_pData = pData;

                pView->Apply(pEvent);

                if (i != (pagesCount - 1))
                    pContext->getPrinter()->newPage();
            }
            pContext->EndPaint();
        }
    }

    NSEditorApi::CAscMenuEvent* pEventEnd = new NSEditorApi::CAscMenuEvent();
    pEventEnd->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_END;

    pView->Apply(pEventEnd);

    pContext->Release();
}

void QAscMainPanel::slot_DialogSave()
{
    std::wstring sName = m_sDownloadName;

    QStringList arDocumentsPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString sDocumentsPath = (arDocumentsPath.size() == 0) ? QString() : arDocumentsPath[0];

    sDocumentsPath += QString::fromStdWString(L"/" + sName);
    QString sFileQT = QFileDialog::getSaveFileName(NULL, QString(), sDocumentsPath);
    std::wstring sPathQT = sFileQT.toStdWString();
    m_pManager->EndSaveDialog(sPathQT, m_nIdCurrentDownloadSaveDialog);
    m_sDownloadName = L"";
    m_nIdCurrentDownloadSaveDialog = 0;
}

void QAscMainPanel::slot_CheckKeyboard()
{
    if (m_pManager)
        m_pManager->CheckKeyboard();
}

void QAscMainPanel::slot_LocalFile_Open(QString sDir)
{
    QFileDialog _dialog;
    _dialog.setDirectory(sDir);

    std::wstring sFileName = _dialog.getOpenFileName(NULL, "Open Document").toStdWString();

    if (sFileName.empty())
        return;

    ((QAscTabWidget*)m_pTabs)->addLocalFile(sFileName);
}
void QAscMainPanel::slot_LocalFile_Create(int nType)
{
    ((QAscTabWidget*)m_pTabs)->addLocalFileCreate(nType);
}

void QAscMainPanel::slot_LocalFile_RecentOpen(NSEditorApi::CAscLocalOpenFileRecent_Recover* pData)
{
    if (!pData->get_IsRecover())
        ((QAscTabWidget*)m_pTabs)->addLocalFileRecent(pData);
    else
        ((QAscTabWidget*)m_pTabs)->addLocalFileRecover(pData);
    pData->Release();
}

void QAscMainPanel::slot_LocalFile_SaveDialog(NSEditorApi::CAscLocalSaveFileDialog* pData)
{
    std::wstring sName = NSCommon::GetFileName(pData->get_Path());

    QStringList arDocumentsPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString sDocumentsPath = (arDocumentsPath.size() == 0) ? QString() : arDocumentsPath[0];

    sDocumentsPath += QString::fromStdWString(L"/" + sName);
    QString sFileQT = QFileDialog::getSaveFileName(NULL, QString(), sDocumentsPath);

    std::wstring sNewPath = sFileQT.toStdWString();

    int nFormat = CAscApplicationManager::GetFileFormatByExtentionForSave(sNewPath);

    pData->put_Path(sNewPath);
    if (nFormat != -1)
        pData->put_FileType(nFormat);

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);
}
void QAscMainPanel::slot_LocalFile_AddImage(NSEditorApi::CAscLocalOpenFileDialog* pData)
{
    QStringList arDocumentsPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString sDocumentsPath = (arDocumentsPath.size() == 0) ? QString() : arDocumentsPath[0];

    std::wstring sFile = QFileDialog::getOpenFileName(NULL, QString(), sDocumentsPath).toStdWString();

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
    pData->put_Path(sFile);
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);
}
void QAscMainPanel::slot_LocalFile_OpenPortal(QString sPortal)
{
    std::wstring sUrl = sPortal.toStdWString();
    if (sUrl.c_str()[sUrl.length() - 1] != '/')
        sUrl += L"/";
    sUrl += L"products/files/?desktop=true";

    ((QAscTabWidget*)m_pTabs)->addTabUrl(sUrl, sPortal);
}

void QAscMainPanel::slot_RemoveTab(int nIndex)
{
    if (-1 != nIndex)
        m_pTabs->removeTab(nIndex);

    if (0 == m_pTabs->count())
    {
        if (true == m_pMainWidget->isHidden())
            pushButtonMainClicked();
    }
}

void QAscMainPanel::slot_LocalFiles_Open(NSEditorApi::CAscLocalOpenFiles* pData)
{
    std::vector<std::wstring>& arFiles = pData->get_Files();
    for (std::vector<std::wstring>::iterator i = arFiles.begin(); i != arFiles.end(); i++)
    {
        ((QAscTabWidget*)m_pTabs)->addLocalFile(*i);
    }
}

void QAscMainPanel::slot_OpenLink(QString url)
{
#ifdef WIN32
    QDesktopServices::openUrl(QUrl(url));
#else	
    std::string sUrl = url.toUtf8().constData();
    sUrl = "LD_LIBRARY_PATH='' xdg-open '" + sUrl + "'";
    system(sUrl.c_str());
#endif
}

void QAscMainPanel::slot_SaveWatermark(int nId)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(NULL, "AscDesktopEditor", "SAVE WITH WATERMARK?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        CCefView* pView = m_pManager->GetViewById(nId);
        if (NULL != pView)
        {
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SAVE;
            pView->Apply(pEvent);
        }
    }
    else
    {
        // ничего
    }
}

void QAscMainPanel::OpenDialogSave(std::wstring sName, unsigned int nId)
{
    m_nIdCurrentDownloadSaveDialog = nId;
    m_sDownloadName = sName;
    emit signal_DialogSave();
}
void QAscMainPanel::CheckKeyboard()
{
    emit signal_CheckKeyboard();
}

void QAscMainPanel::resizeEvent(QResizeEvent* event)
{
    this->RecalculatePlaces();
}

void QAscMainPanel::OnEvent(NSEditorApi::CAscMenuEvent* pEvent)
{
    if (NULL == pEvent)
        return;

    switch (pEvent->m_nType)
    {
    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB:
    {
        NSEditorApi::CAscCreateTab* pData = (NSEditorApi::CAscCreateTab*)pEvent->m_pData;
        QString sUrl = QString::fromStdWString(pData->get_Url());
        ((QAscTabWidget*)m_pTabs)->OpenPopup(sUrl);
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE:
    {
        NSEditorApi::CAscTabEditorType* pData = (NSEditorApi::CAscTabEditorType*)pEvent->m_pData;
        CCefView* pView = m_pManager->GetViewById(pData->get_Id());
        if (NULL != pView && pView->GetType() == cvwtEditor)
        {
            CCefViewEditor* pEditor = (CCefViewEditor*)pView;
            pEditor->SetEditorType((AscEditorType)pData->get_Type());
        }
        emit ((QAscTabWidget*)m_pTabs)->sendUpdateIcons();
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE: // в тестовом примере - закрываем с восстановлением
    case ASC_MENU_EVENT_TYPE_CEF_ONCLOSE:
    {
        NSEditorApi::CAscTypeId* pId = (NSEditorApi::CAscTypeId*)pEvent->m_pData;
        int nId = pId->get_Id();

        int nIndex = ((QAscTabWidget*)m_pTabs)->GetTabIndexById(nId);

        m_pManager->DestroyCefView(nId);

        emit signal_RemoveTab(nIndex);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START:
    {
        // больше не приходит
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD:
    {        
        NSEditorApi::CAscDownloadFileInfo* pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;

        if (pData->get_IsComplete())
        {
            ((QAscTabWidget*)m_pTabs)->EndDownload(pData->get_Id());
        }

        ADDREFINTERFACE(pData);
        emit signal_Download(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME:
    {
        NSEditorApi::CAscDocumentName* pData = (NSEditorApi::CAscDocumentName*)pEvent->m_pData;

        std::wstring sLog = pData->get_Name() + L", " + pData->get_Path() + L", " + pData->get_Url();

        //qDebug() << QString::fromStdWString(sLog) ;

        QString sUrl = QString::fromStdWString(pData->get_Name());
        ((QAscTabWidget*)m_pTabs)->onSendNameAsync(sUrl, pData->get_Id());

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED:
    {
        NSEditorApi::CAscDocumentModifyChanged* pData = (NSEditorApi::CAscDocumentModifyChanged*)pEvent->m_pData;
        emit signal_Modified(pData->get_Id(), pData->get_Changed());

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE:
    {
        //OutputDebugStringA("onsave");
        NSEditorApi::CAscDocumentOnSaveData* pData = (NSEditorApi::CAscDocumentOnSaveData*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_OnSave(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END:
    {
        NSEditorApi::CAscPrintEnd* pData = (NSEditorApi::CAscPrintEnd*)pEvent->m_pData;
        emit signal_Print(pData->get_Id(), pData->get_PagesCount());
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN:
    {
        NSEditorApi::CAscKeyboardDown* pData = (NSEditorApi::CAscKeyboardDown*)pEvent->m_pData;

        QString sFormat = QString("code: %1, ctrl: %2, shift: %3, alt: %4, cmd: %5").arg(
                    QString::number(pData->get_KeyCode()),
                    QString::number((int)pData->get_IsCtrl()),
                    QString::number((int)pData->get_IsShift()),
                    QString::number((int)pData->get_IsAlt()),
                    QString::number((int)pData->get_IsCommandMac()));

#if 0
        if (pData->get_KeyCode() == 32 && pData->get_IsAlt() == true)
        {
            RECT winrect;

            HWND windowHandle = (HWND)this->parentWidget()->winId();
            GetWindowRect( windowHandle, &winrect );
            TrackPopupMenu( GetSystemMenu( windowHandle, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, windowHandle, NULL);
        }
#endif
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN:
    {
        NSEditorApi::CAscLocalFileOpen* pData = (NSEditorApi::CAscLocalFileOpen*)pEvent->m_pData;
        QString sDir = QString::fromStdWString(pData->get_Directory());
        emit signal_LocalFile_Open(sDir);
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE:
    {
        NSEditorApi::CAscLocalFileCreate* pData = (NSEditorApi::CAscLocalFileCreate*)pEvent->m_pData;
        emit signal_LocalFile_Create(pData->get_Type());
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN:
    {
        NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = (NSEditorApi::CAscLocalOpenFileRecent_Recover*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_LocalFile_RecentOpen(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN:
    {
        NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = (NSEditorApi::CAscLocalOpenFileRecent_Recover*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_LocalFile_RecentOpen(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE:
    {
        NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_LocalFile_SaveDialog(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE:
    {
        NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_LocalFile_AddImage(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND:
    {
        NSEditorApi::CAscExecCommand* pData = (NSEditorApi::CAscExecCommand*)pEvent->m_pData;

        if (pData->get_Command() == L"portal:open")
        {
            emit signal_LocalFile_OpenPortal(QString::fromStdWString(pData->get_Param()));
        }

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN:
    {
        NSEditorApi::CAscLocalOpenFiles* pData = (NSEditorApi::CAscLocalOpenFiles*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        emit signal_LocalFiles_Open(pData);

        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK:
    {
        NSEditorApi::CAscOnOpenExternalLink * pData = (NSEditorApi::CAscOnOpenExternalLink *)pEvent->m_pData;
        emit signal_OpenLink(QString().fromStdWString(pData->get_Url()));
        break;
    }
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_SAVE_WATERMARK:
    {
        NSEditorApi::CAscTypeId* pData = (NSEditorApi::CAscTypeId*)pEvent->m_pData;
        emit signal_SaveWatermark(pData->get_Id());
        break;
    }
    }

    RELEASEINTERFACE(pEvent);
}
