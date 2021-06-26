#include "monitor_info.h"
#include "../../include/applicationmanager.h"

#ifdef WIN32
#include "ShellScalingAPI.h"

typedef HMONITOR (__stdcall *function_MonitorFromWindow)(HWND hwnd, DWORD dwFlags);
typedef HRESULT (__stdcall *function_GetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

#ifdef _DPI_AWARENESS_CONTEXTS_
typedef BOOL (__stdcall *function_SetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);
#endif

typedef HRESULT (__stdcall *function_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef UINT (__stdcall *function_GetDpiForWindow)(HWND hwnd);

class CMonitorPrivate
{
public:
    HINSTANCE m_hInstance;
    HINSTANCE m_hInstanceUser;

    function_GetDpiForMonitor m_func_GetDpiForMonitor;
    function_GetDpiForWindow m_func_GetDpiForWindow;
    function_SetProcessDpiAwareness m_funcSetProcessDpiAwareness;

#ifdef _DPI_AWARENESS_CONTEXTS_
    function_SetProcessDpiAwarenessContext m_funcSetProcessDpiAwarenessContext;
#endif


    CMonitorPrivate()
    {
        m_func_GetDpiForMonitor = NULL;
        m_funcSetProcessDpiAwareness = NULL;

#ifdef _DPI_AWARENESS_CONTEXTS_
        m_funcSetProcessDpiAwarenessContext = NULL;
#endif

        m_hInstance = LoadLibraryA("SHCore.dll");

        if (m_hInstance)
        {
            m_func_GetDpiForMonitor = (function_GetDpiForMonitor)GetProcAddress(m_hInstance, "GetDpiForMonitor");
            if (m_func_GetDpiForMonitor)
            {
                m_funcSetProcessDpiAwareness = (function_SetProcessDpiAwareness)GetProcAddress(m_hInstance, "SetProcessDpiAwareness");
            }
        }

        m_hInstanceUser = LoadLibraryA("User32.dll");
        if (m_hInstanceUser)
        {
            m_func_GetDpiForWindow = (function_GetDpiForWindow)GetProcAddress(m_hInstanceUser, "GetDpiForWindow");

#ifdef _DPI_AWARENESS_CONTEXTS_
            m_funcSetProcessDpiAwarenessContext = (function_SetProcessDpiAwarenessContext)GetProcAddress(m_hInstanceUser, "SetProcessDpiAwarenessContext");
#endif
        }
    }
    ~CMonitorPrivate()
    {
        if (m_hInstance)
            FreeLibrary(m_hInstance);
        if (m_hInstanceUser)
            FreeLibrary(m_hInstanceUser);
    }
};

CMonitorPrivate g_monitor_info;

class CDetecterOldSystems
{
public:
    class CMonitorInfo
    {
    public:
        int pixW;
        int pixH;
        int mmW;
        int mmH;

    public:
        CMonitorInfo()
        {
            pixW = -1;
            pixH = -1;
            mmW = 0;
            mmH = 0;
        }

        bool IsValid()
        {
            return mmW > 0 && mmH > 0;
        }
    };

public:
    static std::map<std::string, CMonitorInfo> g_map_old;

public:
    static bool GetDpi(HWND handle, UINT& iuW, UINT& iuH)
    {
        HMONITOR hMonitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        return GetDpi(hMonitor, iuW, iuH);
    }
    static bool GetDpi(HMONITOR hMonitor, UINT& iuW, UINT& iuH)
    {
        iuW = 96;
        iuH = 96;

        MONITORINFOEXA monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (!GetMonitorInfoA(hMonitor, &monitorInfo))
            return false;

        int nPixW = 0;
        int nPixH = 0;

        std::map<std::string, CMonitorInfo>::iterator find = g_map_old.find((std::string)monitorInfo.szDevice);
        if (find != g_map_old.end())
        {
            CMonitorInfo& info = find->second;
            if (!info.IsValid())
                return false;

            nPixW = info.pixW;
            nPixH = info.pixH;
            iuW = (UINT)((2.54 * info.pixW) / info.mmW);
            iuH = (UINT)((2.54 * info.pixH) / info.mmH);
        }
        else
        {
            DWORD dwW = 0;
            DWORD dwH = 0;
            if (!GetMonitorSizeFromEDID(monitorInfo.szDevice, dwW, dwH))
            {
                CMonitorInfo info;
                g_map_old.insert(std::pair<std::string, CMonitorInfo>((std::string)monitorInfo.szDevice, info));
                return false;
            }

            CMonitorInfo info;
            info.pixW = (int)(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
            info.pixH = (int)(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
            info.mmW = dwW;
            info.mmH = dwH;

            nPixW = info.pixW;
            nPixH = info.pixH;
            iuW = (UINT)((2.54 * info.pixW) / info.mmW);
            iuH = (UINT)((2.54 * info.pixH) / info.mmH);

            g_map_old.insert(std::pair<std::string, CMonitorInfo>((std::string)monitorInfo.szDevice, info));
        }

        if (nPixW <= 1600 && nPixH <= 900)
        {
            iuW = 96;
            iuH = 96;
        }

        if (iuW > 150 && iuW <= 180 && nPixW >= 3700)
            iuW = 192;
        if (iuH > 150 && iuH <= 180 && nPixH >= 2000)
            iuH = 192;

        return true;
    }

private:
    static std::string get_strchr(const std::string& s, const char& c)
    {
        std::string::size_type pos = s.find(c);
        if (std::string::npos == pos)
            return "";
        return s.substr(pos);
    }
    static std::string get_strchr_next(const std::string& s, const char& c)
    {
        std::string::size_type pos = s.find(c);
        if (std::string::npos == pos)
            return "";
        return s.substr(pos + 1);
    }
    static std::string get_strchr_back(const std::string& s, const char& c)
    {
        std::string::size_type pos = s.find(c);
        if (std::string::npos == pos)
            return "";
        return s.substr(0, pos);
    }
    static std::string toHexString(const unsigned char& c)
    {
        std::string test = "0123456789ABCDEF";
        std::string ret;
        ret += test.substr(c & 0x0F, 1);
        if ((c & 0xF0) > 0)
            ret += test.substr((c >> 4) & 0x0F, 1);
        return ret;
    }
    static BOOL GetMonitorDevice( char* adapterName, DISPLAY_DEVICEA &ddMon )
    {
        DWORD devMon = 0;

        while (EnumDisplayDevicesA(adapterName, devMon, &ddMon, 0))
        {
            if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE &&
                ddMon.StateFlags & DISPLAY_DEVICE_ATTACHED) // for ATI, Windows XP
                break;

            devMon++;
        }

        if (ddMon.DeviceString[0] == '\0')
        {
            EnumDisplayDevicesA(adapterName, 0, &ddMon, 0);
            if (ddMon.DeviceString[0] == '\0')
                strcpy(ddMon.DeviceString, "Default Monitor");
        }

        return ddMon.DeviceID[0] != '\0';
    }

    static BOOL GetMonitorSizeFromEDID(char* adapterName, DWORD& Width, DWORD& Height)
    {
        DISPLAY_DEVICEA ddMon;
        ZeroMemory(&ddMon, sizeof(ddMon));
        ddMon.cb = sizeof(ddMon);

        //read edid
        bool result = false;
        Width = 0;
        Height = 0;
        if (GetMonitorDevice(adapterName, ddMon))
        {
            std::string sDeviceID = ddMon.DeviceID;
            std::string model = get_strchr_next(sDeviceID, '\\');
            model = get_strchr_back(model, '\\');
            if (7 < model.length())
                model = model.substr(0, 7);

            std::string str = "SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\";
            std::string path = get_strchr_next(sDeviceID, '\\');
            str += get_strchr_back(path, '\\');
            path = get_strchr_next(path, '\\');

            char strKey[MAX_PATH];
            memcpy(strKey, str.c_str(), str.length());
            strKey[str.length()] = '\0';

            HKEY hKey;
            if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, strKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                DWORD i = 0;
                DWORD size = MAX_PATH;
                FILETIME ft;
                while(RegEnumKeyExA(hKey, i, strKey, &size, NULL, NULL, NULL, &ft) == ERROR_SUCCESS)
                {
                    HKEY hKey2;
                    if(RegOpenKeyExA(hKey, strKey, 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
                    {
                        size = MAX_PATH;
                        if(RegQueryValueExA(hKey2, "Driver", NULL, NULL, (LPBYTE)&strKey, &size) == ERROR_SUCCESS)
                        {
                            std::string test_str(strKey);
                            if (test_str == path)
                            {
                                HKEY hKey3;
                                if(RegOpenKeyExA(hKey2, "Device Parameters", 0, KEY_READ, &hKey3) == ERROR_SUCCESS)
                                {
                                    BYTE EDID[256];
                                    size = 256;
                                    if(RegQueryValueExA(hKey3, "EDID", NULL, NULL, (LPBYTE)&EDID, &size) == ERROR_SUCCESS)
                                    {
                                        DWORD p = 8;
                                        std::string model2;

                                        char byte1 = EDID[p];
                                        char byte2 = EDID[p+1];
                                        model2 += (char)(((byte1 & 0x7C) >> 2) + 64);
                                        model2 += (char)(((byte1 & 3) << 3) + ((byte2 & 0xE0) >> 5) + 64);
                                        model2 += (char)((byte2 & 0x1F) + 64);

                                        model2 += toHexString((EDID[p+3] & 0xf0) >> 4);
                                        model2 += toHexString(EDID[p+3] & 0xf);
                                        model2 += toHexString((EDID[p+2] & 0xf0) >> 4);
                                        model2 += toHexString(EDID[p+2] & 0x0f);

                                        if (model == model2)
                                        {
                                            Width = EDID[21];
                                            Height = EDID[22];
                                            result = true;
                                        }
                                        else
                                        {
                                            // EDID incorrect
                                        }
                                    }
                                    RegCloseKey(hKey3);
                                }
                            }
                        }
                        RegCloseKey(hKey2);
                    }
                    i++;
                }
                RegCloseKey(hKey);
            }
        }

        return result;
    }
};

std::map<std::string, CDetecterOldSystems::CMonitorInfo> CDetecterOldSystems::g_map_old;

double NSMonitor::GetRawMonitorScale(const unsigned int& xDpi, const unsigned int& yDpi)
{
    if (xDpi > 180 && yDpi > 180)
        return 2;

    if (xDpi > 120 && yDpi > 120)
        return 1.5;

    return 1;
}
double NSMonitor::GetRawMonitorDpi(WindowHandleId handle)
{
    if (true)
    {
        UINT iuW = 0;
        UINT iuH = 0;
        if (CDetecterOldSystems::GetDpi(handle, iuW, iuH))
        {
            return GetRawMonitorScale(iuW, iuH);
        }
    }

    if (g_monitor_info.m_func_GetDpiForWindow)
    {
        UINT resDpi = g_monitor_info.m_func_GetDpiForWindow(handle);
        return GetRawMonitorScale(resDpi, resDpi);
    }

    if (g_monitor_info.m_func_GetDpiForMonitor)
    {
        HMONITOR hMonitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        UINT iuW = 0;
        UINT iuH = 0;
        g_monitor_info.m_func_GetDpiForMonitor(hMonitor, MDT_RAW_DPI, &iuW, &iuH);
        return GetRawMonitorScale(iuW, iuH);
    }

    return 1;
}

int Core_SetProcessDpiAwareness(void)
{
#ifdef _DPI_AWARENESS_CONTEXTS_
    if (g_monitor_info.m_funcSetProcessDpiAwarenessContext)
    {
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif
        if (g_monitor_info.m_funcSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            return 0;

    }
#endif

    if (!g_monitor_info.m_funcSetProcessDpiAwareness)
        return 1;

    g_monitor_info.m_funcSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    return 0;
}

int Core_GetMonitorRawDpi(WindowHandleId handle, unsigned int* uiX, unsigned int* uiY)
{
    *uiX = 0;
    *uiY = 0;

    if (true)
    {
        UINT iuW = 0;
        UINT iuH = 0;
        if (CDetecterOldSystems::GetDpi(handle, iuW, iuH))
        {
            *uiX = iuW;
            *uiY = iuH;
            return 0;
        }
    }

    if (g_monitor_info.m_func_GetDpiForWindow)
    {
        UINT resDpi = g_monitor_info.m_func_GetDpiForWindow(handle);

        *uiX = resDpi;
        *uiY = resDpi;

        return 0;
    }

    if (g_monitor_info.m_func_GetDpiForMonitor)
    {
        HMONITOR hMonitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        g_monitor_info.m_func_GetDpiForMonitor(hMonitor, MDT_RAW_DPI, uiX, uiY);
        return 0;
    }
    return -1;
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
        if (true)
        {
            UINT iuW = 0;
            UINT iuH = 0;
            if (CDetecterOldSystems::GetDpi(info.hMonitor, iuW, iuH))
            {
                *uiX = iuW;
                *uiY = iuH;
                return 0;
            }
        }

        if (g_monitor_info.m_func_GetDpiForMonitor)
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
