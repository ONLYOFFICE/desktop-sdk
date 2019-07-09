#include "../qascvideoview.h"
#include "./lib_base.h"

#ifdef USE_VLC_LIBRARY

#ifdef _WIN32
#include <QSysInfo>
#include <windows.h>

LONG WINAPI vlc_exception_filter(struct _EXCEPTION_POINTERS *lpExceptionInfo)
{
    exit(0);
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void CheckWindowsOld()
{
    switch (QSysInfo::windowsVersion())
    {
    case QSysInfo::WV_2000:
    case QSysInfo::WV_XP:
    case QSysInfo::WV_2003:
    case QSysInfo::WV_VISTA:
    {
        SetErrorMode(SEM_FAILCRITICALERRORS);
        SetUnhandledExceptionFilter(vlc_exception_filter);
        break;
    }
    default:
        break;
    }
}

#endif

#include <QCoreApplication>

#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/Directory.h"

static VlcInstance* g_vlc_instance = NULL;

void NSBaseVideoLibrary::Init(QObject* parent)
{
    if (g_vlc_instance)
        return;

    if (!parent)
        return;

#ifdef _WIN32
    CheckWindowsOld();
#endif

    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    std::wstring sProcessDir = NSFile::GetProcessDirectory();
    std::wstring sPluginsDir = sProcessDir + L"/plugins";
    if (!NSDirectory::Exists(sPluginsDir))
    {
        std::vector<std::wstring> dirs = NSDirectory::GetDirectories(sProcessDir + L"/..");
        for (std::vector<std::wstring>::iterator i = dirs.begin(); i != dirs.end(); i++)
        {
            std::wstring sTest = *i + L"/plugins";
            if (NSDirectory::Exists(sTest))
            {
                sPluginsDir = sTest;
                break;
            }
        }
    }

    VlcCommon::setPluginPath(QString::fromStdWString(sPluginsDir));
    g_vlc_instance = new VlcInstance(VlcCommon::args(), NULL);
}
void NSBaseVideoLibrary::Destroy()
{
    if (g_vlc_instance)
    {
        delete g_vlc_instance;
        g_vlc_instance = NULL;
    }
}
void* NSBaseVideoLibrary::GetLibrary()
{
    return (void*)g_vlc_instance;
}

#else

void NSBaseVideoLibrary::Init(QObject* parent)
{
}
void NSBaseVideoLibrary::Destroy()
{
}
void* NSBaseVideoLibrary::GetLibrary()
{
    return NULL;
}

#endif
