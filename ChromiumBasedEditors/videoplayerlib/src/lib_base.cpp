#include "../qascvideoview.h"
#include "./lib_base.h"

#ifdef USE_VLC_LIBRARY

#ifdef _WIN32
#include <windows.h>

LONG WINAPI vlc_exception_filter(struct _EXCEPTION_POINTERS *lpExceptionInfo)
{
	exit(0);
	return EXCEPTION_CONTINUE_EXECUTION;
}

#ifdef QT_VERSION_6
#include <QOperatingSystemVersion>
static void CheckWindowsOld()
{
	QOperatingSystemVersion osVersion = QOperatingSystemVersion::current();

	if ((osVersion.majorVersion() == 5) || (osVersion.majorVersion() == 6 && osVersion.minorVersion() == 0))
	{
		SetErrorMode(SEM_FAILCRITICALERRORS);
		SetUnhandledExceptionFilter(vlc_exception_filter);
	}
}
#else
#include <QSysInfo>
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

#endif

void NSBaseVideoLibrary::Init(QObject* parent)
{
	if (!parent)
		return;

#ifdef _WIN32
	CheckWindowsOld();
#endif
}

void NSBaseVideoLibrary::SetVerbosityLevel(int nVerbose)
{
	CVlcInstance::setVerbosityLevel(nVerbose);
}

#else

void NSBaseVideoLibrary::Init(QObject* parent)
{
}

void NSBaseVideoLibrary::SetVerbosityLevel(int nVerbose)
{
}

#endif
