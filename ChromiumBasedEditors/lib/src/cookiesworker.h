/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CEF_COOKIES_WORKER_H
#define CEF_COOKIES_WORKER_H

#include "include/cef_base.h"
#include "include/base/cef_callback.h"
#include "include/cef_cookie.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/internal/cef_time.h"
#include "../include/base.h"
#include <map>
#include <string>
#include <vector>

#ifdef CEF_2623
#include "include/base/cef_bind.h"
#endif

#ifdef CEF_VERSION_ABOVE_102
#define BASE_BIND base::BindOnce
#else
#define BASE_BIND base::Bind
#endif

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

		CefCookieManager::GetGlobalManager(nullptr)->FlushStore(nullptr);
	}

	virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie)
	{
		std::string sDomain = CefString(&cookie.domain).ToString();
		std::string sDomainPath = CefString(&cookie.path).ToString();
		std::string sDomainWithPath = sDomain + sDomainPath;

		if (m_arDomains.size() == 0)
		{
			if (sDomain.find(m_sDomain) != std::string::npos ||
					sDomainWithPath.find(m_sDomain) != std::string::npos)
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
					bool bIsFind = false;

					if (sDomain.find(sDomainFind) != std::string::npos ||
						sDomainWithPath.find(sDomainFind) != std::string::npos)
					{
						bIsFind = true;
					}
					else
					{
						std::string::size_type posPort = sDomainFind.rfind(':');
						if (std::string::npos != posPort && posPort > 7)
						{
							std::string sDomainPathWithoutPort = sDomainFind.substr(0, posPort);
							if (sDomain.find(sDomainPathWithoutPort) != std::string::npos ||
								sDomainWithPath.find(sDomainPathWithoutPort) != std::string::npos)
							{
								bIsFind = true;
							}
						}
					}

					if (bIsFind)
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
#ifdef CEF_VERSION_ABOVE_102
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieVisitor::CheckCookiePresent, base::WrapRefCounted(this), manager));
#else
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieVisitor::CheckCookiePresent, this, manager));
#endif
			return;
		}

		manager->VisitAllCookies(this);
		this->Release();
	}

public:
	IMPLEMENT_REFCOUNTING(CCefCookieVisitor);
};

// Single cookie value lookup routed back to the requesting frame (getPortalAuthToken).
class CCookieValueCallback
{
public:
	virtual void OnFoundCookieValue(int_64_type nFrameId, const std::string& sCallbackId, bool bFound, const std::string& sValue) = 0;
};

class CCefCookieValueVisitor : public CefCookieVisitor
{
public:
	std::string                         m_sDomain;      // host[:port], no scheme
	std::string                         m_sCookieName;

	int_64_type                         m_nFrameId;
	std::string                         m_sCallbackId;

	CCookieValueCallback*               m_pCallback;

	bool                                m_bFound;
	std::string                         m_sValue;

	CCefCookieValueVisitor()
	{
		this->AddRef();
		m_nFrameId = -1;
		m_pCallback = NULL;
		m_bFound = false;
	}
	~CCefCookieValueVisitor()
	{
		// CEF drops its reference after the last Visit (or immediately when nothing matched)
		if (m_pCallback)
			m_pCallback->OnFoundCookieValue(m_nFrameId, m_sCallbackId, m_bFound, m_bFound ? m_sValue : "");
	}

	static std::string GetHost(const std::string& sDomain)
	{
		std::string sHost = sDomain;
		std::string::size_type pos = sHost.find("://");
		if (std::string::npos != pos)
			sHost = sHost.substr(pos + 3);
		pos = sHost.find_first_of("/?#");
		if (std::string::npos != pos)
			sHost = sHost.substr(0, pos);
		pos = sHost.rfind(':');
		if (std::string::npos != pos && std::string::npos == sHost.find(']', pos))
			sHost = sHost.substr(0, pos);
		return sHost;
	}

	static std::string ToLowerAscii(const std::string& sSrc)
	{
		std::string sRes = sSrc;
		for (std::string::size_type i = 0; i < sRes.length(); ++i)
		{
			if (sRes[i] >= 'A' && sRes[i] <= 'Z')
				sRes[i] = (char)(sRes[i] - 'A' + 'a');
		}
		return sRes;
	}

	// cookie domains are stored lowercased; ".example.com" matches example.com and any subdomain
	static bool IsDomainMatch(const std::string& sCookieDomainSrc, const std::string& sHostSrc)
	{
		std::string sDomain = ToLowerAscii(sCookieDomainSrc);
		std::string sHost = ToLowerAscii(sHostSrc);
		if (!sDomain.empty() && '.' == sDomain[0])
			sDomain = sDomain.substr(1);
		if (sDomain.empty() || sHost.empty())
			return false;
		if (sDomain == sHost)
			return true;
		if (sHost.length() > sDomain.length() &&
			0 == sHost.compare(sHost.length() - sDomain.length(), sDomain.length(), sDomain) &&
			'.' == sHost[sHost.length() - sDomain.length() - 1])
			return true;
		return false;
	}

	virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie)
	{
		if (m_bFound)
			return false;

		std::string sName = CefString(&cookie.name).ToString();
		if (sName != m_sCookieName)
			return true;

		std::string sCookieDomain = CefString(&cookie.domain).ToString();
		if (!IsDomainMatch(sCookieDomain, GetHost(m_sDomain)))
			return true;

		m_sValue = CefString(&cookie.value).ToString();
		m_bFound = true;
		return false;
	}

	void FindCookie(CefRefPtr<CefCookieManager> manager)
	{
		if (!CefCurrentlyOn(TID_IO))
		{
#ifdef CEF_VERSION_ABOVE_102
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieValueVisitor::FindCookie, base::WrapRefCounted(this), manager));
#else
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieValueVisitor::FindCookie, this, manager));
#endif
			return;
		}

		std::string sHost = GetHost(m_sDomain);
		if (!sHost.empty())
		{
			// url-scoped visit: exact cookie-domain semantics, includes httpOnly cookies;
			// non-secure cookies are visible for an https url as well
			manager->VisitUrlCookies("https://" + m_sDomain + "/", true, this);
		}

		this->Release();
	}

public:
	IMPLEMENT_REFCOUNTING(CCefCookieValueVisitor);
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
		CefCookieManager::GetGlobalManager(nullptr)->FlushStore(nullptr);
	}

	void Correct()
	{
		std::string::size_type pos = m_sUrl.find("?");
		if (pos != std::string::npos)
			m_sUrl = m_sUrl.substr(0, pos);

		pos = m_sDomain.find("?");
		if (pos != std::string::npos)
			m_sDomain = m_sDomain.substr(0, pos);

		pos = m_sPath.find("?");
		if (pos != std::string::npos)
			m_sPath = m_sPath.substr(0, pos);
	}

	void SetCookie(CefRefPtr<CefCookieManager> manager)
	{
		if (!CefCurrentlyOn(TID_IO))
		{
#ifdef CEF_VERSION_ABOVE_102
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieSetter::SetCookie, base::WrapRefCounted(this), manager));
#else
			CefPostTask(TID_IO, BASE_BIND(&CCefCookieSetter::SetCookie, this, manager));
#endif
			return;
		}

		CefCookie authorization;

		CefString(&authorization.name).FromString(m_sCookieKey);
		CefString(&authorization.value).FromString(m_sCookieValue);

		CefString(&authorization.domain).FromString(m_sDomain);
		CefString(&authorization.path).FromString(m_sPath);

		authorization.httponly = 0;
		authorization.secure = 0;

		authorization.has_expires = true;

#if defined(CEF_VERSION_ABOVE_102) && !defined(CEF_VERSION_103)
		cef_time_t cef_time;
		cef_time.year = 2200;
		cef_time.month = 4;
		cef_time.day_of_week = 5;
		cef_time.day_of_month = 11;

		cef_time.hour = 0;
		cef_time.minute = 0;
		cef_time.second = 0;
		cef_time.millisecond = 0;

		// since chromium 104 maximum 400 days!!!
		cef_time_to_basetime(&cef_time, &authorization.expires);
#else
        authorization.expires.year = 2200;
        authorization.expires.month = 4;
        authorization.expires.day_of_week = 5;
        authorization.expires.day_of_month = 11;
#endif

		manager->SetCookie(m_sUrl, authorization, nullptr);

		m_pCallback->OnSetCookie();

		this->Release();
	}

public:
	IMPLEMENT_REFCOUNTING(CCefCookieSetter);
};

#endif // CEF_COOKIES_WORKER_H
