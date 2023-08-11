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

static libvlc_instance_t* g_vlc_instance = NULL;

void NSBaseVideoLibrary::Init(QObject* parent)
{
    if (g_vlc_instance)
        return;

    if (!parent)
        return;

#ifdef _WIN32
    CheckWindowsOld();
#endif

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

	qputenv("VLC_PLUGIN_PATH", QString::fromStdWString(sPluginsDir).toLocal8Bit());
	g_vlc_instance = libvlc_new(0, nullptr);
}
void NSBaseVideoLibrary::Destroy()
{
    if (g_vlc_instance)
	{
		libvlc_release(g_vlc_instance);
		g_vlc_instance = nullptr;
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
