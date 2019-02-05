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

#ifndef CEF_COOKIES_WORKER_H
#define CEF_COOKIES_WORKER_H

#include "include/base/cef_bind.h"
#include "include/cef_cookie.h"
#include "include/wrapper/cef_closure_task.h"
#include <map>

class CCookieFoundCallback
{
public:
    virtual void OnFoundCookie(bool bIsPresent, std::string sValue)     = 0;
    virtual void OnSetCookie()                                          = 0;
    virtual void OnDeleteCookie(bool bIsPresent)                        = 0;

    virtual void OnFoundCookies(std::map<std::string, std::string>& mapValues)     = 0;
};

class CCefCookieVisitor : public CefCookieVisitor
{
public:
    std::map<std::string, std::string>  m_cookies;

    std::string                         m_sDomain;
    std::string                         m_sCookieSearch;

    CCookieFoundCallback*               m_pCallback;

    bool                                m_bIsDelete;
    bool                                m_bIsDeleted;

    std::vector<std::string>            m_arDomains;
    std::map<std::string, std::string>  m_mapFinds;

    CCefCookieVisitor()
    {
        this->AddRef();
        m_pCallback = NULL;
        m_bIsDelete = false;
        m_bIsDeleted = false;
    }
    ~CCefCookieVisitor()
    {
        if (m_pCallback)
        {
            if (!m_bIsDelete)
            {
                if (m_arDomains.size() > 0)
                {
                    for (std::vector<std::string>::const_iterator i = m_arDomains.begin(); i != m_arDomains.end(); i++)
                    {
                        std::string sDomain = *i;
                        std::map<std::string, std::string>::const_iterator _find = m_mapFinds.find(sDomain);
                        if (_find == m_mapFinds.end())
                        {
                            m_mapFinds.insert(std::pair<std::string, std::string>(sDomain, ""));
                        }
                    }

                    m_pCallback->OnFoundCookies(m_mapFinds);
                    return;
                }
                else
                {
                    std::map<std::string, std::string>::iterator i = m_cookies.find(m_sCookieSearch);

                    if (i != m_cookies.end())
                    {
                        m_pCallback->OnFoundCookie(true, i->second);
                    }
                    else
                    {
                        m_pCallback->OnFoundCookie(false, "");
                    }
                }
            }
            else
            {
                m_pCallback->OnDeleteCookie(m_bIsDeleted);
            }
        }

        CefCookieManager::GetGlobalManager(NULL)->FlushStore(NULL);
    }

    virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie)
    {
        std::string sDomain = CefString(&cookie.domain).ToString();

        if (m_arDomains.size() == 0)
        {
            if (sDomain.find(m_sDomain) != std::string::npos)
            {
                std::string sName = CefString(&cookie.name).ToString();
                std::string sValue = CefString(&cookie.value).ToString();

                m_cookies.insert(std::pair<std::string, std::string>(sName, sValue));

                if (m_bIsDelete)
                {
                    m_bIsDeleted = true;
                    deleteCookie = true;
                }
            }
        }
        else
        {
            std::string sName = CefString(&cookie.name).ToString();

            if (sName == m_sCookieSearch)
            {
                std::string sValue = CefString(&cookie.value).ToString();

                for (std::vector<std::string>::iterator i = m_arDomains.begin(); i != m_arDomains.end(); i++)
                {
                    std::string sDomainFind = *i;
                    if (sDomain.find(sDomainFind) != std::string::npos)
                    {
                        m_mapFinds.insert(std::pair<std::string, std::string>(sDomainFind, sValue));

                        std::string sName = CefString(&cookie.name).ToString();
                        std::string sValue = CefString(&cookie.value).ToString();

                        m_cookies.insert(std::pair<std::string, std::string>(sName, sValue));
                    }
                }
            }
        }

        return true;
    }

    void CheckCookiePresent(CefRefPtr<CefCookieManager> manager)
    {
        if (!CefCurrentlyOn(TID_IO))
        {
            CefPostTask(TID_IO,
                base::Bind(&CCefCookieVisitor::CheckCookiePresent, this, manager));
            return;
        }

        manager->VisitAllCookies(this);
        this->Release();
    }

public:
    IMPLEMENT_REFCOUNTING(CCefCookieVisitor);
};

#ifdef CEF_2623
#define CefBase_Class CefBase
#else
#define CefBase_Class CefBaseRefCounted
#endif
class CCefCookieSetter : public CefBase_Class
{
public:
    std::string                         m_sUrl;
    std::string                         m_sDomain;
    std::string                         m_sPath;
    std::string                         m_sCookieKey;
    std::string                         m_sCookieValue;

    CCookieFoundCallback*               m_pCallback;

    CCefCookieSetter()
    {
        this->AddRef();
    }
    ~CCefCookieSetter()
    {
        CefCookieManager::GetGlobalManager(NULL)->FlushStore(NULL);
    }

    void SetCookie(CefRefPtr<CefCookieManager> manager)
    {
        if (!CefCurrentlyOn(TID_IO))
        {
            CefPostTask(TID_IO,
                base::Bind(&CCefCookieSetter::SetCookie, this, manager));
            return;
        }

        CefCookie authorization;

    #if 0
        CefString(&authorization.name).FromString("asc_auth_key");
        CefString(&authorization.value).FromString("mNRRcfOz5/kpQs8+H+ImHZrXrz+ByxHpUdf44jektpNgBCuUCcEHB8CEB6TRJTYCqsN5Ag0UlmS+cyj7G838uRoffSe44N/PW4PYIItmSl7HI8lh3nEmmtYjwRWKFDYi");

        CefString(&authorization.domain).FromString("ascdesktop.teamlab.info");
        CefString(&authorization.path).FromString("/");
    #else
        CefString(&authorization.name).FromString(m_sCookieKey);
        CefString(&authorization.value).FromString(m_sCookieValue);

        CefString(&authorization.domain).FromString(m_sDomain);
        CefString(&authorization.path).FromString(m_sPath);
    #endif

        authorization.httponly = 0;
        authorization.secure = 0;

        authorization.has_expires = true;
        authorization.expires.year = 2200;
        authorization.expires.month = 4;
        authorization.expires.day_of_week = 5;
        authorization.expires.day_of_month = 11;

        bool bIsAddedCookie = manager->SetCookie(m_sUrl, authorization, NULL);

        m_pCallback->OnSetCookie();

        this->Release();
    }

public:
    IMPLEMENT_REFCOUNTING(CCefCookieSetter);
};

#endif // CEF_COOKIES_WORKER_H
