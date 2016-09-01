#pragma once

#include <QFile>
#include <QScreen>
#include <QDir>
#include <QCloseEvent>

#include "QAscApplicationManager.h"

#ifdef WIN32
#include <shlwapi.h>
#include <shlobj.h>
#endif

#include <QStandardPaths>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>

#include <QDebug>

#include "../../../../../core/DesktopEditor/common/Directory.h"

static std::wstring GetFileName_main(const std::wstring& sPath)
{
    std::wstring::size_type nPos1 = sPath.rfind('\\');
    std::wstring::size_type nPos2 = sPath.rfind('/');
    std::wstring::size_type nPos = std::wstring::npos;

    if (nPos1 != std::wstring::npos)
    {
        nPos = nPos1;
        if (nPos2 != std::wstring::npos && nPos2 > nPos)
            nPos = nPos2;
    }
    else
        nPos = nPos2;

    if (nPos == std::wstring::npos)
        return sPath;
    return sPath.substr(nPos + 1);
}

static std::wstring GetAppDataPath()
{
#ifdef WIN32
    WCHAR szPath[MAX_PATH];
    // Get path for each computer, non-user specific and non-roaming data.
    if ( SUCCEEDED( SHGetFolderPathW( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath ) ) )
    {
        // TODO: [0]
        // QStringList arr = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

        // Append product-specific path
        std::wstring sAppData(szPath);
        sAppData += L"/ONLYOFFICE";
        return sAppData;
    }
#else

    std::wstring sAppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdWString() ;
    if (sAppData.empty())
        sAppData = QDir::homePath().toStdWString();

    std::wstring sAppPath = NSFile::GetProcessPath();
    std::wstring sAppName = GetFileName_main(sAppPath);

    std::wstring::size_type nPos = sAppData.find(sAppName);
    if (nPos != std::wstring::npos)
        sAppData = sAppData.substr(0, nPos);

    sAppData += L"IvolgaPRO";

    if (!NSDirectory::Exists(sAppData))
        NSDirectory::CreateDirectory(sAppData);

    sAppData += L"/DesktopEditors";

    if (!NSDirectory::Exists(sAppData))
        NSDirectory::CreateDirectory(sAppData);

    if (!NSDirectory::Exists(sAppData + L"/webdata"))
        NSDirectory::CreateDirectory(sAppData + L"/webdata");

    if (!NSDirectory::Exists(sAppData + L"/webdata/cloud"))
        NSDirectory::CreateDirectory(sAppData + L"/webdata/cloud");

    return sAppData;
#endif

    return L"";
}

#include <QMainWindow>
#include <QApplication>
class QAscMainWindow : public QMainWindow
{
    Q_OBJECT
    QWidget* m_pMainPanel;

public:
    explicit QAscMainWindow(QWidget *parent, CAscApplicationManager* pAppManager) : QMainWindow(parent)
    {
        this->setWindowTitle("IvolgaPRO");
        this->resize(1200, 700);

        m_pMainPanel = new QAscMainPanel(this, pAppManager, false);
        this->setCentralWidget(m_pMainPanel);

        ((QAscApplicationManager*)pAppManager)->m_pPanel = (QAscMainPanel*)m_pMainPanel;

        QMetaObject::connectSlotsByName(this);

        pAppManager->StartSpellChecker();
        pAppManager->StartKeyboardChecker();
    }

    ~QAscMainWindow()
    {

    }

    void closeEvent(QCloseEvent* e)
    {
        QAscMainPanel* pPanel = (QAscMainPanel*)m_pMainPanel;
        pPanel->GetAppManager()->DestroyCefView(-1);
    }
};

#if 0
class QNativeEventFilter : public QAbstractNativeEventFilter
{
public:
    CAscApplicationManager* m_pManager;
public:
    QNativeEventFilter(CAscApplicationManager* pManager) : QAbstractNativeEventFilter()
    {
        m_pManager = pManager;
    }
    virtual ~QNativeEventFilter() {}

    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
    {
        //qDebug() << "EventFilter";
        //qDebug() << rand();
        m_pManager->GetApplication()->DoMessageLoopEvent();
        return false;
    }
};

class QApplicationEL : public QApplication
{
public:
    QApplicationEL(int &argc, char **argv) : QApplication(argc, argv)
    {
    }
public:
    void InitMessageLoop(CAscApplicationManager* pManager)
    {
        this->installNativeEventFilter(new QNativeEventFilter(pManager));
    }
};
#endif

static int AscEditor_Main( int argc, char *argv[] )
{
    QApplication a(argc, argv);

    CApplicationCEF* application_cef = new CApplicationCEF();
    CAscApplicationManager* pApplicationManager = new QAscApplicationManager();

    std::wstring sUserPath = GetAppDataPath();
    pApplicationManager->m_oSettings.SetUserDataPath(sUserPath);
    pApplicationManager->m_oSettings.use_system_fonts = true;
    //pApplicationManager->m_oSettings.country = "US";
    //pApplicationManager->m_oSettings.additional_fonts_folder.push_back(NSFile::GetProcessDirectory() + L"/fonts");

#ifdef LINUX
    pApplicationManager->m_oSettings.recover_path = sUserPath + L"/recover";
#endif

    int nReturnCode = application_cef->Init_CEF(pApplicationManager, argc, argv);
    if (application_cef->IsChromiumSubprocess())
    {
        delete application_cef;
        delete pApplicationManager;
        return nReturnCode;
    }

    pApplicationManager->CheckFonts();   

#if 1

    std::wstring sLicenseDir = sUserPath + L"/licence";
    if (!NSDirectory::Exists(sLicenseDir))
    {
#if 1
        NSDirectory::CreateDirectory(sLicenseDir);

        NSEditorApi::CAscMenuEvent* pEvent1 = new NSEditorApi::CAscMenuEvent();
        pEvent1->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_DEMO;

        NSEditorApi::CAscLicenceActual* pData1 = new NSEditorApi::CAscLicenceActual();
        pData1->put_Path(sLicenseDir);
        pData1->put_ProductId(301);

        pEvent1->m_pData = pData1;

        pApplicationManager->Apply(pEvent1);
#endif
    }

    NSDirectory::CreateDirectory(sLicenseDir);

    NSEditorApi::CAscMenuEvent* pEvent1 = new NSEditorApi::CAscMenuEvent();
    pEvent1->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL;

    NSEditorApi::CAscLicenceActual* pData1 = new NSEditorApi::CAscLicenceActual();
    ADDREFINTERFACE(pData1);

    pData1->put_Path(sLicenseDir);
    pData1->put_ProductId(301);

    pEvent1->m_pData = pData1;

    pApplicationManager->Apply(pEvent1);

    RELEASEINTERFACE(pData1);

#endif

    qreal ww = a.primaryScreen()->logicalDotsPerInch();
    qreal hh = a.primaryScreen()->physicalDotsPerInch();

    // Font
    QFont mainFont = a.font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    a.setFont( mainFont );

    // Create window
    QAscMainWindow w(NULL, pApplicationManager);

    w.setStyleSheet("#mainPanel { margin: 0; padding: 0; }\
#systemPanel { margin: 0; padding: 0; } \
#centralWidget { background: #313437; } \
QPushButton:focus{border:none;outline:none;}\
QWidget {border:none;outline:none;}");

    w.show();

    bool bIsOwnMessageLoop = false;
    application_cef->RunMessageLoop(bIsOwnMessageLoop);

    if (!bIsOwnMessageLoop)
    {
        // Launch
        a.exec();
    }

    // release all subprocesses
    pApplicationManager->CloseApplication();

    delete application_cef;
    delete pApplicationManager;

    return 0;
}
