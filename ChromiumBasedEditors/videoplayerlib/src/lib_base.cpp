#include "../qascvideoview.h"
#include "./lib_base.h"

#ifdef USE_VLC_LIBRARY

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
