#include "monitor_info.h"
#include "../../include/applicationmanager.h"

#ifdef WIN32
#include "ShellScalingAPI.h"

typedef HMONITOR (__stdcall *function_MonitorFromWindow)(HWND hwnd, DWORD dwFlags);
typedef HRESULT (__stdcall *function_GetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

typedef HRESULT (__stdcall *function_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);

class CMonitorPrivate
{
public:
    HINSTANCE m_hInstance;

    function_GetDpiForMonitor m_func_GetDpiForMonitor;
    function_SetProcessDpiAwareness m_funcSetProcessDpiAwareness;

    CMonitorPrivate()
    {
        m_func_GetDpiForMonitor = NULL;
        m_funcSetProcessDpiAwareness = NULL;
        m_hInstance = LoadLibraryA("SHCore.dll");

        if (m_hInstance)
        {
            m_func_GetDpiForMonitor = (function_GetDpiForMonitor)GetProcAddress(m_hInstance, "GetDpiForMonitor");
            if (m_func_GetDpiForMonitor)
            {
                m_funcSetProcessDpiAwareness = (function_SetProcessDpiAwareness)GetProcAddress(m_hInstance, "SetProcessDpiAwareness");
            }
        }
    }
    ~CMonitorPrivate()
    {
        if (m_hInstance)
            FreeLibrary(m_hInstance);
    }
};

CMonitorPrivate g_monitor_info;

int NSMonitor::GetRawMonitorDpi(WindowHandleId handle)
{
    if (!g_monitor_info.m_func_GetDpiForMonitor)
        return 1;

    HMONITOR hMonitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
    UINT iuW = 0;
    UINT iuH = 0;
    g_monitor_info.m_func_GetDpiForMonitor(hMonitor, MDT_RAW_DPI, &iuW, &iuH);

    if (iuW > 180 && iuH > 180)
        return 2;
    return 1;
}

int Core_SetProcessDpiAwareness(void)
{
    if (!g_monitor_info.m_funcSetProcessDpiAwareness)
        return 1;

    HRESULT hr = g_monitor_info.m_funcSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    return 0;
}

int Core_GetMonitorRawDpi(WindowHandleId handle, unsigned int* uiX, unsigned int* uiY)
{
    *uiX = 0;
    *uiY = 0;

    if (!g_monitor_info.m_func_GetDpiForMonitor)
        return -1;

    HMONITOR hMonitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
    g_monitor_info.m_func_GetDpiForMonitor(hMonitor, MDT_RAW_DPI, uiX, uiY);
    return 0;
}

struct sEnumInfo
{
    int iIndex;
    HMONITOR hMonitor;
};

BOOL CALLBACK GetMonitorByIndex(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    sEnumInfo *info = (sEnumInfo*) dwData;
    if (--info->iIndex < 0)
    {
        info->hMonitor = hMonitor;
        return FALSE;
    }
    return TRUE;
}

int Core_GetMonitorRawDpiByIndex(int index, unsigned int* uiX, unsigned int* uiY)
{
    *uiX = 0;
    *uiY = 0;

    sEnumInfo info;
    info.iIndex = index;
    info.hMonitor = NULL;

    EnumDisplayMonitors(NULL, NULL, GetMonitorByIndex, (LPARAM)&info);
    if (info.hMonitor != NULL)
    {
        if (!g_monitor_info.m_func_GetDpiForMonitor)
            return -1;

        g_monitor_info.m_func_GetDpiForMonitor(info.hMonitor, MDT_RAW_DPI, uiX, uiY);
        return 0;
    }

    return -1;
}

#else

int NSMonitor::GetRawMonitorDpi(WindowHandleId handle)
{
    return -1;
}

#endif
