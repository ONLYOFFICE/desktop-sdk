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

#ifndef NOCORS_FILE_DOWNLOADER_H
#define NOCORS_FILE_DOWNLOADER_H

#include "include/cef_urlrequest.h"
#include "include/cef_request_context.h"

#include "../../../../core/DesktopEditor/common/File.h"

#include <string>

class INoCorsFileDownloaderEvents
{
public:
	virtual ~INoCorsFileDownloaderEvents() {}
	virtual void OnNoCorsFileDownloaded(std::wstring sUrl, std::wstring sPath, bool bIsSuccess) = 0;
};

class CNoCorsFileDownloader : public CefURLRequestClient
{
private:
	INoCorsFileDownloaderEvents* m_pEvents;

	std::wstring m_sUrl;
	std::wstring m_sPath;
	std::wstring m_sReferrer;

	NSFile::CFileBinary m_oFile;
	bool m_bIsFileOpened;
	bool m_bIsError;

	CefRefPtr<CefURLRequest> m_request;

public:
	CNoCorsFileDownloader(INoCorsFileDownloaderEvents* pEvents,
						  const std::wstring& sUrl,
						  const std::wstring& sPath,
						  const std::wstring& sReferrer = L"")
	{
		m_pEvents = pEvents;
		m_sUrl = sUrl;
		m_sPath = sPath;
		m_sReferrer = sReferrer;

		m_bIsFileOpened = false;
		m_bIsError = false;
	}

	virtual ~CNoCorsFileDownloader()
	{
		CloseFile();
	}

	const std::wstring& GetUrl() const { return m_sUrl; }
	const std::wstring& GetPath() const { return m_sPath; }
	void Start()
	{
		CefRefPtr<CefRequest> request = CefRequest::Create();
		request->SetURL(m_sUrl);
		request->SetMethod("GET");

#ifdef CEF_2623
		request->SetFlags(UR_FLAG_ALLOW_CACHED_CREDENTIALS);
#else
		request->SetFlags(UR_FLAG_ALLOW_STORED_CREDENTIALS);
#endif

		request->SetFirstPartyForCookies(m_sReferrer.empty() ? m_sUrl : m_sReferrer);

		if (!m_sReferrer.empty())
			request->SetReferrer(m_sReferrer, REFERRER_POLICY_DEFAULT);

		m_request = CefURLRequest::Create(request, this, CefRequestContext::GetGlobalContext());

		if (!m_request)
			Complete(false);
	}

	void Stop()
	{
		m_pEvents = NULL;

		CefRefPtr<CefURLRequest> request = m_request;
		m_request = nullptr;

		if (request)
			request->Cancel();

		CloseFile();
	}

public:
	void OnRequestComplete(CefRefPtr<CefURLRequest> request) OVERRIDE
	{
		bool bIsSuccess = !m_bIsError && (UR_SUCCESS == request->GetRequestStatus());

		if (bIsSuccess && !m_bIsFileOpened)
		{
			bIsSuccess = OpenFile(request);
		}

		Complete(bIsSuccess);
	}

	void OnDownloadData(CefRefPtr<CefURLRequest> request,
						const void* data,
						size_t data_length) OVERRIDE
	{
		if (m_bIsError)
			return;

		if (!m_bIsFileOpened && !OpenFile(request))
			return;

		if (0 < data_length)
		{
			if (!m_oFile.WriteFile(data, (DWORD)data_length))
				m_bIsError = true;
		}
	}

	void OnUploadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE {}
	void OnDownloadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE {}

	bool GetAuthCredentials(bool isProxy,
							const CefString& host,
							int port,
							const CefString& realm,
							const CefString& scheme,
							CefRefPtr<CefAuthCallback> callback) OVERRIDE
	{
		return false;
	}

private:
	bool OpenFile(CefRefPtr<CefURLRequest> request)
	{
		CefRefPtr<CefResponse> response = request->GetResponse();
		if (response)
		{
			int nStatus = response->GetStatus();
			if (0 != nStatus && (nStatus < 200 || nStatus >= 300))
			{
				m_bIsError = true;
				return false;
			}
		}

		if (NSFile::CFileBinary::Exists(m_sPath))
			NSFile::CFileBinary::Remove(m_sPath);

		if (!m_oFile.CreateFileW(m_sPath))
		{
			m_bIsError = true;
			return false;
		}

		m_bIsFileOpened = true;
		return true;
	}

	void CloseFile()
	{
		if (m_bIsFileOpened)
		{
			m_oFile.CloseFile();
			m_bIsFileOpened = false;
		}
	}

	void Complete(bool bIsSuccess)
	{
		CefRefPtr<CNoCorsFileDownloader> self(this);

		CloseFile();
		m_request = nullptr;

		if (!bIsSuccess && NSFile::CFileBinary::Exists(m_sPath))
			NSFile::CFileBinary::Remove(m_sPath);

		INoCorsFileDownloaderEvents* pEvents = m_pEvents;
		m_pEvents = NULL;

		if (pEvents)
			pEvents->OnNoCorsFileDownloaded(m_sUrl, m_sPath, bIsSuccess);
	}

	IMPLEMENT_REFCOUNTING(CNoCorsFileDownloader);
};

#endif // NOCORS_FILE_DOWNLOADER_H
