/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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

#include "include/cef_scheme.h"
#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/graphics/Timer.h"
#include "../../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "./client_resource_handler_async.h"

class CResourceHandlerFileAsync : public NSTimers::CTimer
{
public:
    int m_nId;
    CResourceHandlerFileAsyncCallback* m_handler;
    std::wstring m_sFile;
    CefRefPtr<CefCallback> m_callback;

public:
    CResourceHandlerFileAsync(int nId,
                              CResourceHandlerFileAsyncCallback* handler,
                              const std::wstring& sFile,
                              CefRefPtr<CefCallback> callback) : NSTimers::CTimer()
    {
        m_nId = nId;
        m_handler = handler;
        m_sFile = sFile;
        m_callback = callback;

        SetInterval(100);
        Start(0);
    }
    virtual ~CResourceHandlerFileAsync()
    {
        m_handler->ReleaseWrapper();
    }

    virtual void OnTimer();
};

class CResourceHandlerFileAsyncManager
{
private:
    std::vector<CResourceHandlerFileAsync*> m_handlers;
    int                                     m_handlers_counter;
    NSCriticalSection::CRITICAL_SECTION     m_oCS;
    bool                                    m_bInit;

public:
    CResourceHandlerFileAsyncManager()
    {
        m_handlers_counter = 0;
        m_bInit = true;
        m_oCS.InitializeCriticalSection();
    }
    ~CResourceHandlerFileAsyncManager()
    {
        Destroy();
    }

    void Destroy()
    {
        if (!m_bInit)
            return;

        m_bInit = false;

        m_oCS.Enter();
        for (std::vector<CResourceHandlerFileAsync*>::iterator i = m_handlers.begin(); i != m_handlers.end(); i++)
        {
            CResourceHandlerFileAsync* handler = *i;
            handler->Stop();
            delete handler;
        }
        m_handlers.clear();
        m_oCS.Leave();

        m_oCS.DeleteCriticalSection();
    }

    void Remove(int nId)
    {
        if (!m_bInit)
            return;

        m_oCS.Enter();
        for (std::vector<CResourceHandlerFileAsync*>::iterator i = m_handlers.begin(); i != m_handlers.end(); i++)
        {
            CResourceHandlerFileAsync* handler = *i;
            if (handler->m_nId == nId)
            {
                m_handlers.erase(i);
                handler->StopNoJoin();
                handler->DestroyOnFinish();
                break;
            }
        }
        m_oCS.Leave();
    }

    void Create(CResourceHandlerFileAsyncCallback* handler,
                const std::wstring& sFile,
                CefRefPtr<CefCallback> callback)
    {
        if (!m_bInit)
            return;

        m_oCS.Enter();
        m_handlers.push_back(new CResourceHandlerFileAsync(m_handlers_counter++, handler, sFile, callback));
        m_oCS.Leave();
    }
};

CResourceHandlerFileAsyncManager* g_manager = new CResourceHandlerFileAsyncManager();

void NSResourceHandlerFileAsyncManager::Create(CResourceHandlerFileAsyncCallback* handler,
            const std::wstring& sFile,
            CefRefPtr<CefCallback> callback)
{
    if (!g_manager)
        return;
    g_manager->Create(handler, sFile, callback);
}

void NSResourceHandlerFileAsyncManager::Destroy()
{
    g_manager->Destroy();
    delete g_manager;
    g_manager = NULL;
}

void CResourceHandlerFileAsync::OnTimer()
{
    if (m_bRunThread && NSFile::CFileBinary::Exists(m_sFile))
    {
        m_handler->OnAsyncComplete(m_sFile);
        m_callback->Continue();
        g_manager->Remove(m_nId);
    }
}
