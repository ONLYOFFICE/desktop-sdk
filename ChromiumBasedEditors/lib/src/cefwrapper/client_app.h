/*
 * (c) Copyright Ascensio System SIA 2010-2017
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#ifndef CEF_ASC_CLIENT_APP_H_
#define CEF_ASC_CLIENT_APP_H_

#ifndef CEF_2623
#include "tests/shared/common/client_app.h"
#else
#include "cefclient/common/client_app.h"
#endif

#if defined(_LINUX) && !defined(_MAC)
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#endif

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/Common/FileDownloader/FileDownloader.h"

#include "../../include/applicationmanager.h"

#include "client_renderer_params.h"

//#define DISABLE_WEB_SEQURITY

static int IsForceDpiRound()
{
#ifndef MAC_NO_MAIN_PROCESS
    if (NULL != CAscApplicationManager::GetDpiChecker())
        return 1;
#endif

#ifdef WIN32
#if 1
    return 1;
#else
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetDC(hwnd);
    int nX = GetDeviceCaps(hdc, LOGPIXELSX);
    int nY = GetDeviceCaps(hdc, LOGPIXELSY);

    ReleaseDC(hwnd, hdc);

    double dScale1 = (double)nX / 96.0;
    double dScale2 = (double)nX / 96.0;
    double dScale = (dScale1 < dScale2) ? dScale2 : dScale1;
    int nScale = (int)(dScale + 0.49);

    if (((nX % 96) == 0) && ((nY % 96) == 0))
    {
        if (nScale == 1 || nScale == 2)
            return 0;

        if (nScale < 1)
            return 1;
        if (nScale > 2)
            return 2;

        return 0;
    }
    else
    {
        if (nScale < 1)
            return 1;
        if (nScale > 2)
            return 2;

        return nScale;
    }
#endif
#endif

#if defined(_LINUX) && !defined(_MAC)
    GdkScreen* screen = gdk_screen_get_default();

    if (screen)
    {
        double dScale = gdk_screen_get_resolution(screen);

        int wPx = gdk_screen_get_width(screen);
        int hPx = gdk_screen_get_height(screen);
        int wMm = gdk_screen_get_width_mm(screen);
        int hMm = gdk_screen_get_height_mm(screen);

        if (wMm < 1)
            wMm = 1;
        if (hMm < 1)
            hMm = 1;

        int nDpiX = (int)(0.5 + wPx * 25.4 / wMm);
        int nDpiY = (int)(0.5 + hPx * 25.4 / hMm);
        int nDpi = (nDpiX + nDpiY) >> 1;

        if (nDpi < 10)
            return 0;

        dScale /= nDpi;
        if (dScale < 1)
            return 0;
        else if (dScale > 2)
            return 2;
        else
            return (int)(dScale + 0.49);
    }
#endif

    return 0;
}

class CAppSettings
{
public:
    bool m_GPU;
    bool m_Canvas;
    std::string m_ColorProfile;
    
public:
    CAppSettings(std::map<std::string, std::string>& mapSettings)
    {
        m_GPU = true;
        m_Canvas = true;

        m_ColorProfile = "srgb";
        
#ifndef _MAC
#ifdef WIN32
        m_Canvas = true;
#else
        m_Canvas = false;
#endif
#endif
        
#if defined(_LINUX) && !defined(_MAC)
        m_GPU = false;
#endif
        
        std::map<std::string, std::string>::iterator pairGPU = mapSettings.find("disable-gpu");
        if (pairGPU != mapSettings.end())
        {
            if ("1" == pairGPU->second)
                m_GPU = true;
            else if ("0" == pairGPU->second)
                m_GPU = false;
        }
        std::map<std::string, std::string>::iterator pairCanvas = mapSettings.find("disable-gpu-canvas2d");
        if (pairCanvas != mapSettings.end())
        {
            if ("1" == pairCanvas->second)
                m_Canvas = true;
            else if ("0" == pairCanvas->second)
                m_Canvas = false;
        }
        std::map<std::string, std::string>::iterator pairColorProfile = mapSettings.find("force-color-profile");
        if (pairColorProfile != mapSettings.end())
        {
            if ("default" == pairColorProfile->second)
                m_ColorProfile = "";
            else
                m_ColorProfile = pairColorProfile->second;
        }
    }
    
    void Process(CefRefPtr<CefCommandLine> command_line)
    {
        if (!m_GPU)
        {
            command_line->AppendSwitch("--disable-gpu");
        }
        
        if (!m_Canvas)
        {
            command_line->AppendSwitch("--disable-accelerated-2d-canvas");
            command_line->AppendSwitch("--disable-d3d11");
        }

        if (!m_ColorProfile.empty())
        {
            command_line->AppendSwitchWithValue("--force-color-profile", m_ColorProfile);
        }
    }
};

#ifndef MAC_NO_MAIN_PROCESS

#ifdef CEF_2623
#include "cefclient/browser/client_app_browser.h"
#else
#include "tests/shared/browser/client_app_browser.h"
#endif

class CAscClientAppBrowser : public client::ClientAppBrowser, public CAppSettings
{
public:
    bool m_GPU;
    bool m_Canvas;
    CAscApplicationManager* m_manager;

public:
    CAscClientAppBrowser(std::map<std::string, std::string>& mapSettings, CAscApplicationManager* pManager = NULL) : client::ClientAppBrowser(), CAppSettings(mapSettings)
    {
#ifdef _MAC
        CFileDownloader::SetARCEnabled(true);
#endif
        m_manager = pManager;
    }

    virtual ~CAscClientAppBrowser()
    {
    }
public:
    virtual void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) OVERRIDE
    {
        if (process_type.empty())
        {
            CAppSettings::Process(command_line);

#if defined(_LINUX) && !defined(_MAC)
            if (true)
            {
                // заглушка для АльтЛинукс
                if (NSFile::CFileBinary::Exists(L"/etc/altlinux-release"))
                {
                    command_line->AppendSwitch("--disable-lcd-text");
                }
            }
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitch("--enable-file-cookies");
            command_line->AppendSwitch("--disable-pinch");
            command_line->AppendSwitch("--enable-aggressive-domstorage-flushing");
            command_line->AppendSwitch("--enable-color-correct-rendering");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            //command_line->AppendSwitch("--allow-file-access-from-files");
            //command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");

            int forceDpi = IsForceDpiRound();
            if (0 != forceDpi)
                command_line->AppendSwitchWithValue("--force-device-scale-factor", std::to_string(forceDpi));

#ifdef DISABLE_WEB_SEQURITY
            command_line->AppendSwitch("--disable-web-security");
#endif
        }
    }

    virtual void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE
    {
        size_t nCount = extra_info->GetSize();

        if (m_manager)
        {
            std::vector<std::string> props = m_manager->GetRendererStartupProperties();
            for (std::vector<std::string>::iterator iter = props.begin(); iter != props.end(); iter++)
            {
                extra_info->SetString(nCount++, *iter);
            }
        }

        client::ClientAppBrowser::OnRenderProcessThreadCreated(extra_info);
    }


public:
    IMPLEMENT_REFCOUNTING(CAscClientAppBrowser);
};
#endif

#ifndef MAC_NO_SUB_PROCESS

#ifdef CEF_2623
#include "cefclient/renderer/client_app_renderer.h"
#include "cefclient/common/client_app_other.h"
#else
#include "tests/shared/renderer/client_app_renderer.h"
#include "tests/shared/common/client_app_other.h"
#endif

class CAscClientAppOther : public client::ClientAppOther, public CAppSettings
{
public:
    CAscClientAppOther(std::map<std::string, std::string>& mapSettings) : client::ClientAppOther(), CAppSettings(mapSettings)
    {
#ifdef _MAC
        CFileDownloader::SetARCEnabled(true);
#endif
    }

    virtual ~CAscClientAppOther()
    {
    }
public:
    virtual void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) OVERRIDE
    {
        if (process_type.empty())
        {
            CAppSettings::Process(command_line);

#if defined(_LINUX) && !defined(_MAC)
            if (true)
            {
                // заглушка для АльтЛинукс
                if (NSFile::CFileBinary::Exists(L"/etc/altlinux-release"))
                {
                    command_line->AppendSwitch("--disable-lcd-text");
                }
            }
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitch("--enable-file-cookies");
            command_line->AppendSwitch("--disable-pinch");
            command_line->AppendSwitch("--enable-aggressive-domstorage-flushing");
            command_line->AppendSwitch("--enable-color-correct-rendering");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            //command_line->AppendSwitch("--allow-file-access-from-files");
            //command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");

            int forceDpi = IsForceDpiRound();
            if (0 != forceDpi)
                command_line->AppendSwitchWithValue("--force-device-scale-factor", std::to_string(forceDpi));

#ifdef DISABLE_WEB_SEQURITY
            command_line->AppendSwitch("--disable-web-security");
#endif
        }
    }

public:
    IMPLEMENT_REFCOUNTING(CAscClientAppOther);
};

class CAscClientAppRenderer : public client::ClientAppRenderer, public CAppSettings
{
public:
    CAscClientAppRenderer(std::map<std::string, std::string>& mapSettings) : client::ClientAppRenderer(), CAppSettings(mapSettings)
    {
#ifdef _MAC
        CFileDownloader::SetARCEnabled(true);
#endif
    }

    virtual ~CAscClientAppRenderer()
    {
    }
public:
    virtual void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) OVERRIDE
    {
        if (process_type.empty())
        {
            CAppSettings::Process(command_line);

#if defined(_LINUX) && !defined(_MAC)
            if (true)
            {
                // заглушка для АльтЛинукс
                if (NSFile::CFileBinary::Exists(L"/etc/altlinux-release"))
                {
                    command_line->AppendSwitch("--disable-lcd-text");
                }
            }
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitch("--enable-file-cookies");
            command_line->AppendSwitch("--disable-pinch");
            command_line->AppendSwitch("--enable-aggressive-domstorage-flushing");
            command_line->AppendSwitch("--enable-color-correct-rendering");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            //command_line->AppendSwitch("--allow-file-access-from-files");
            //command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");

            int forceDpi = IsForceDpiRound();
            if (0 != forceDpi)
                command_line->AppendSwitchWithValue("--force-device-scale-factor", std::to_string(forceDpi));

#ifdef DISABLE_WEB_SEQURITY
            command_line->AppendSwitch("--disable-web-security");
#endif
        }
    }

    virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE
    {
        size_t nCount = extra_info->GetSize();

        std::vector<std::string> params;
        for (size_t i = 0; i < nCount; ++i)
            params.push_back(extra_info->GetString(i));

        CAscRendererProcessParams::getInstance().Check(params);
    }
};
#endif

#endif
