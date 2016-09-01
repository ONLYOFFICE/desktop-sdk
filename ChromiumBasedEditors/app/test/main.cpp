#include <QFile>
#include <QScreen>
#include <QDir>

#include "mainwindow.h"
#include "./src/QAscApplicationManager.h"

#ifdef WIN32
#include <shlwapi.h>
#include <shlobj.h>
#endif

#include <QStandardPaths>

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
    std::wstring sAppData = QDir::homePath().toStdWString();
    sAppData += L"/ONLYOFFICE";
    return sAppData;
#endif

    return L"";
}

static int AscEditor_MainWindows( int argc, char *argv[] )
{
    QApplication a(argc, argv);

    CApplicationCEF* application_cef = new CApplicationCEF();
    CAscApplicationManager* pApplicationManager = new QAscApplicationManager();

    std::wstring sUserPath = GetAppDataPath();
    pApplicationManager->m_oSettings.SetUserDataPath(sUserPath);

    int nReturnCode = application_cef->Init_CEF(pApplicationManager, argc, argv);
    if (application_cef->IsChromiumSubprocess())
    {
        delete application_cef;
        delete pApplicationManager;
        return nReturnCode;
    }

    pApplicationManager->CheckFonts();

    a.setStyleSheet("#mainPanel { margin: 0; padding: 0; }\
#systemPanel { margin: 0; padding: 0; } \
#centralWidget { background: #313437; }");

    // Font
    QFont mainFont = a.font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    a.setFont( mainFont );

    // Background color
    HBRUSH windowBackground = CreateSolidBrush( RGB( 49, 52, 55 ) );

    // Create window
    CMainWindow window( &a, windowBackground, 100, 100, 1280, 800, pApplicationManager );
    window.setMinimumSize( 100, 100 );

    ((QAscApplicationManager*)pApplicationManager)->m_pPanel = window.mainPanel->m_pPanel;

    pApplicationManager->StartSpellChecker();
    pApplicationManager->StartKeyboardChecker();

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

int main( int argc, char *argv[] )
{
    return AscEditor_MainWindows(argc, argv);
}
