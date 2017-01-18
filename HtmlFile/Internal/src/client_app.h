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

#include "cefclient/common/client_app.h"
#include "cefclient/browser/client_app_browser.h"
#include "cefclient/renderer/client_app_renderer.h"
#include "cefclient/common/client_app_other.h"
#include "cefclient/renderer/client_renderer.h"

#include "./clienthandler.h"

namespace asc_client_renderer
{
    // Create the render delegate.
    void CreateRenderDelegates(client::ClientAppRenderer::DelegateSet& delegates);
}

class CAscClientAppBrowser : public client::ClientAppBrowser
{
public:
    CAscClientAppBrowser() : client::ClientAppBrowser()
    {
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
#ifndef _MAC
            command_line->AppendSwitch("--disable-accelerated-2d-canvas");
            command_line->AppendSwitch("--disable-d3d11");
#endif

#if defined(_LINUX) && !defined(_MAC)
            command_line->AppendSwitch("--disable-gpu");
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            command_line->AppendSwitch("--allow-file-access-from-files");
            command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");
        }
    }

public:
    IMPLEMENT_REFCOUNTING(CAscClientAppBrowser);
};

class CAscClientAppOther : public client::ClientAppOther
{
public:
    CAscClientAppOther() : client::ClientAppOther()
    {
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
#ifndef _MAC
            command_line->AppendSwitch("--disable-accelerated-2d-canvas");
            command_line->AppendSwitch("--disable-d3d11");
#endif

#if defined(_LINUX) && !defined(_MAC)
            command_line->AppendSwitch("--disable-gpu");
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            //command_line->AppendSwitch("--allow-file-access-from-files");
            //command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");
        }
    }

public:
    IMPLEMENT_REFCOUNTING(CAscClientAppOther);
};

class CAscClientAppRenderer : public client::ClientAppRenderer
{
public:
    CAscClientAppRenderer() : client::ClientAppRenderer()
    {
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
#ifndef _MAC
            command_line->AppendSwitch("--disable-accelerated-2d-canvas");
            command_line->AppendSwitch("--disable-d3d11");
#endif

#if defined(_LINUX) && !defined(_MAC)
            command_line->AppendSwitch("--disable-gpu");
#endif

            command_line->AppendSwitch("--disable-extensions");
            command_line->AppendSwitchWithValue("--log-severity", "disable");

            //command_line->AppendSwitch("--allow-file-access-from-files");
            //command_line->AppendSwitch("--allow-file-access");

            //command_line->AppendSwitch("--allow-running-insecure-content");

            //command_line->AppendSwitch("--disable-web-security");
        }
    }

    virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
                                       OVERRIDE
    {
        asc_client_renderer::CreateRenderDelegates(delegates_);

        DelegateSet::iterator it = delegates_.begin();
        for (; it != delegates_.end(); ++it)
          (*it)->OnRenderThreadCreated(this, extra_info);
    }
};

#endif
