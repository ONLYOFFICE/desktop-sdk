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

#ifndef CORS_RESOURCE_HANDLER_H
#define CORS_RESOURCE_HANDLER_H

#ifdef CEF_VERSION_ABOVE_102

#define CEF_SUPPORT_STREAMING_CORS_RESOURCE_HANDLER

#include "include/cef_resource_handler.h"
#include "include/cef_urlrequest.h"
#include "include/wrapper/cef_closure_task.h"
#include <vector>
#include <string>
#include <mutex>

#define CALL_AND_CLEAR_CALLBACK(callback_ptr, ...) \
do {                                               \
	if (callback_ptr) {                            \
		auto temp_callback = callback_ptr;         \
		callback_ptr = nullptr;                    \
		temp_callback->Continue(__VA_ARGS__);      \
	}                                              \
} while(0)

//#define CEF_SUPPORT_STREAMING_CORS_RESOURCE_HANDLER_LOG_DATA
#ifdef CEF_SUPPORT_STREAMING_CORS_RESOURCE_HANDLER_LOG_DATA
void LOG_DATA(void* data, int data_len)
{
	if (data == NULL || data_len <= 0)
		return;

	FILE* f = fopen("D:/stream_data_log.txt", "ab");
	if (!f) return;

	fwrite(data, 1, data_len, f);
	fwrite("\n\n", 1, 2, f);

	fclose(f);
}
#else
void LOG_DATA(void* data, int data_len) {}
#endif

class StreamingURLRequestClient;

class StreamingCORSResourceHandler : public CefResourceHandler
{
public:
	explicit StreamingCORSResourceHandler(CefRefPtr<CefRequest> request);

	bool Open(CefRefPtr<CefRequest> request,
			  bool& handle_request,
			  CefRefPtr<CefCallback> callback) override;

	void GetResponseHeaders(CefRefPtr<CefResponse> response,
							int64& response_length,
							CefString& redirectUrl) override;

	bool Read(void* data_out,
			  int bytes_to_read,
			  int& bytes_read,
			  CefRefPtr<CefResourceReadCallback> callback) override;

	void Cancel() override;

	void OnData(CefRefPtr<CefURLRequest> request, const void* data, size_t size);
	void OnComplete(CefRefPtr<CefURLRequest> request);

private:
	void ReadResponseInfo(CefRefPtr<CefURLRequest> request);

	friend class StreamingURLRequestClient;

	CefRefPtr<CefRequest> m_originalRequest;
	CefRefPtr<CefURLRequest> m_urlRequest;

	CefRefPtr<CefCallback> m_openCallback;

	std::recursive_mutex m_mutex;

	std::vector<char> m_buffer;
	void* m_pendingDataOut;
	int m_pendingBytesToRead;

	bool m_completed;
	bool m_responseInfoReceived;

	CefRefPtr<CefResourceReadCallback> m_pendingCallback;

	int m_status;
	CefString m_statusText;
	CefString m_mimeType;
	CefResponse::HeaderMap m_headers;

	IMPLEMENT_REFCOUNTING(StreamingCORSResourceHandler);
};

class StreamingURLRequestClient : public CefURLRequestClient
{
public:
	explicit StreamingURLRequestClient(CefRefPtr<StreamingCORSResourceHandler> handler)
		: m_handler(handler)
	{
	}

	~StreamingURLRequestClient()
	{
	}

	void OnDownloadData(CefRefPtr<CefURLRequest> request,
						const void* data,
						size_t size) override
	{
		m_handler->OnData(request, data, size);
	}

	void OnRequestComplete(CefRefPtr<CefURLRequest> request) override
	{
		m_handler->OnComplete(request);
	}

	void OnUploadProgress(CefRefPtr<CefURLRequest>, int64, int64) override {}
	void OnDownloadProgress(CefRefPtr<CefURLRequest>, int64, int64) override {}

	bool GetAuthCredentials(bool, const CefString&, int, const CefString&,
							const CefString&, CefRefPtr<CefAuthCallback>) override
	{
		return false;
	}

private:
	CefRefPtr<StreamingCORSResourceHandler> m_handler;
	IMPLEMENT_REFCOUNTING(StreamingURLRequestClient);
};

StreamingCORSResourceHandler::StreamingCORSResourceHandler(CefRefPtr<CefRequest> request)
	: m_originalRequest(request)
	  , m_pendingDataOut(nullptr)
	  , m_pendingBytesToRead(0)
	  , m_completed(false)
	  , m_responseInfoReceived(false)
	  , m_status(200)
{
	std::string url = request->GetURL().ToString();

	const std::string prefix = "onlyoffice-proxy://";
	if (url.find(prefix) == 0)
	{
		std::string path = url.substr(prefix.size());

		if (path.find("https/") == 0)
			path = "https://" + path.substr(6);
		else if (path.find("http/") == 0)
			path = "http://" + path.substr(5);

		m_originalRequest = CefRequest::Create();
		m_originalRequest->SetURL(path);
		m_originalRequest->SetMethod(request->GetMethod());
		m_originalRequest->SetReferrer(request->GetReferrerURL(),
									   request->GetReferrerPolicy());

		CefRequest::HeaderMap headers;
		request->GetHeaderMap(headers);
		m_originalRequest->SetHeaderMap(headers);

		if (CefRefPtr<CefPostData> post = request->GetPostData()) {
			m_originalRequest->SetPostData(post);
		}

		m_originalRequest->SetFlags(request->GetFlags());
		m_originalRequest->SetFirstPartyForCookies(request->GetFirstPartyForCookies());
	}
}

bool StreamingCORSResourceHandler::Open(CefRefPtr<CefRequest> request,
										bool& handle_request,
										CefRefPtr<CefCallback> callback) {
	handle_request = false;
	m_openCallback = callback;

	CefPostTask(TID_UI, base::BindOnce(
							[](CefRefPtr<StreamingCORSResourceHandler> self) {
								std::string url = self->m_originalRequest->GetURL().ToString();

								CefRefPtr<StreamingURLRequestClient> client = new StreamingURLRequestClient(self);

								self->m_urlRequest = CefURLRequest::Create(self->m_originalRequest, client.get(), CefRequestContext::GetGlobalContext());

								if (!self->m_urlRequest)
								{
									if (self->m_openCallback)
									{
										self->m_openCallback->Cancel();
										self->m_openCallback = nullptr;
									}
								}
							},
							CefRefPtr<StreamingCORSResourceHandler>(this)
							));

	return true;
}

static bool IsSkipProxyResponseHeader(const CefString& name)
{
	// header names are case-insensitive (and lowercase in HTTP/2)
	std::string sName = name.ToString();
	for (char& c : sName)
	{
		if (c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
	}

	// CORS headers are set by proxy. HeaderMap is a multimap, so the server ones
	// would be duplicated: "Access-Control-Allow-Origin: *, *" is rejected by CORS check
	if (0 == sName.find("access-control-"))
		return true;

	// CefURLRequest returns already decoded body with unknown length (response_length = -1)
	return (sName == "content-encoding" || sName == "content-length");
}

void StreamingCORSResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
													  int64& response_length,
													  CefString& redirectUrl)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	CefResponse::HeaderMap headers;
	for (const auto& header : m_headers)
	{
		if (!IsSkipProxyResponseHeader(header.first))
			headers.insert(header);
	}

	headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
	headers.insert(std::make_pair("Access-Control-Allow-Methods", "*"));
	headers.insert(std::make_pair("Access-Control-Allow-Headers", "*"));
	headers.insert(std::make_pair("Access-Control-Expose-Headers", "*"));

	response->SetHeaderMap(headers);
	response->SetStatus(m_status);
	response->SetStatusText(m_statusText);
	response->SetMimeType(m_mimeType);

	response_length = -1;
}

bool StreamingCORSResourceHandler::Read(void* data_out,
										int bytes_to_read,
										int& bytes_read,
										CefRefPtr<CefResourceReadCallback> callback)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	bytes_read = 0;

	// Try to read from buffer
	if (!m_buffer.empty())
	{
		int n = std::min(bytes_to_read, (int)m_buffer.size());
		memcpy(data_out, m_buffer.data(), n);
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + n);
		bytes_read = n;

		LOG_DATA(data_out, n);
		return true;
	}

	if (m_completed)
		return false;

	// Wait for data
	m_pendingCallback = callback;
	m_pendingBytesToRead = bytes_to_read;
	m_pendingDataOut = data_out;
	return true;
}

void StreamingCORSResourceHandler::ReadResponseInfo(CefRefPtr<CefURLRequest> request)
{
	// GetResponseHeaders is called right after Open callback,
	// so response info must be read BEFORE it is continued
	if (m_responseInfoReceived)
		return;

	CefRefPtr<CefResponse> resp = request->GetResponse();
	if (!resp)
		return;

	m_responseInfoReceived = true;
	m_status = resp->GetStatus();
	m_statusText = resp->GetStatusText();
	m_mimeType = resp->GetMimeType();
	resp->GetHeaderMap(m_headers);
}

void StreamingCORSResourceHandler::OnData(CefRefPtr<CefURLRequest> request, const void* data, size_t size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	// Add to buffer
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + size);

	ReadResponseInfo(request);

	// Call Open callback ONLY on first data
	CALL_AND_CLEAR_CALLBACK(m_openCallback);

	// Notify pending Read callback if exists
	if (m_pendingCallback && m_pendingDataOut)
	{
		int n = std::min(m_pendingBytesToRead, (int)m_buffer.size());
		memcpy(m_pendingDataOut, m_buffer.data(), n);
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + n);

		LOG_DATA(m_pendingDataOut, n);

		m_pendingDataOut = nullptr;
		m_pendingBytesToRead = 0;

		CALL_AND_CLEAR_CALLBACK(m_pendingCallback, n);
	}
}

void StreamingCORSResourceHandler::OnComplete(CefRefPtr<CefURLRequest> request)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	CefURLRequest::ErrorCode error = request->GetRequestError();

	// response without body (no OnData)
	ReadResponseInfo(request);

	m_completed = true;

	// If Open never got data
	CALL_AND_CLEAR_CALLBACK(m_openCallback);

	// Wake up pending Read
	CALL_AND_CLEAR_CALLBACK(m_pendingCallback, 0);
}

void StreamingCORSResourceHandler::Cancel()
{
	if (m_urlRequest)
		m_urlRequest->Cancel();	
}

#endif // CEF_VERSION_ABOVE_102

#endif // CORS_RESOURCE_HANDLER_H
