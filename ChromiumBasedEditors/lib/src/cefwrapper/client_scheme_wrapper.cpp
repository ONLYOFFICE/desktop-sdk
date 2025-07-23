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

#include "./client_scheme.h"
#include "./client_app.h"

#include <algorithm>
#include <string>

#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/graphics/BaseThread.h"
#include "include/cef_parser.h"
#include "./client_resource_handler_async.h"
#include "../utils.h"

#if defined(_LINUX) && !defined(_MAC)
#ifdef Status
#undef Status
#endif
#ifdef Success
#undef Success
#endif
#ifdef None
#undef None
#endif
#ifdef Always
#undef Always
#endif
#endif

#include "../applicationmanager_p.h"

namespace asc_scheme
{

	std::string GetMimeTypeFromExt(const std::wstring& sFile)
	{
		std::wstring sExt = NSFile::GetFileExtention(sFile);

		if (sExt == L"html")
			return "text/html";
		if (sExt == L"js")
			return "text/javascript";
		if (sExt == L"css")
			return "text/css";
		if (sExt == L"json")
			return "application/json";
		if (sExt == L"png")
			return "image/png";
		if (sExt == L"jpg" || sExt == L"jpeg")
			return "image/jpeg";
		if (sExt == L"svg")
			return "image/svg+xml";

		return "*/*";
	}
	std::vector<std::wstring> get_url_params(std::wstring& sUrl)
	{
		std::vector<std::wstring> ret;
		std::wstring::size_type pos = sUrl.find(L"__ascdesktopeditor__param__");
		std::wstring::size_type posF = pos;
		while (pos != std::wstring::npos)
		{
			std::wstring::size_type posParam = pos + 27; // len(__ascdesktopeditor__param__) = 27
			pos = sUrl.find(L"__ascdesktopeditor__param__", posParam);

			std::wstring::size_type posEnd = (pos == std::wstring::npos) ? sUrl.length() : pos;
			if (posEnd > posParam)
				ret.push_back(sUrl.substr(posParam, posEnd - posParam));
		}
		if (posF != std::wstring::npos)
			sUrl = sUrl.substr(0, posF);
		return ret;
	}
	unsigned long read_file_with_urls(std::wstring& sUrl, unsigned char*& data)
	{
		data = NULL;

		std::vector<std::wstring> arParams = get_url_params(sUrl);

		unsigned long start = 0;
		unsigned long count = 0;
		if (arParams.size() > 0)
			start = std::stoul(arParams[0]);
		if (arParams.size() > 1)
			count = std::stoul(arParams[1]);

		DWORD dwSize = 0;
		NSFile::CFileBinary oFile;
		if (oFile.OpenFile(sUrl))
		{
			unsigned long fileSize = (unsigned long)oFile.GetFileSize();
			if (fileSize)
			{
				if (start > fileSize)
					start = fileSize - 1;
				if (count == 0)
					count = fileSize;
				if ((start + count) > fileSize)
					count = fileSize - start;

				if (start > 0)
					oFile.SeekFile((int)start);

				data = new unsigned char[count];
				oFile.ReadFile(data, (DWORD)count, dwSize);
			}
		}
		return (unsigned long)dwSize;
	}

} // asc_scheme

// Implementation of the schema handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler, public CResourceHandlerFileAsyncCallback
{
public:
	ClientSchemeHandler(CAscApplicationManager* pManager, const std::wstring& sMainUrl) : offset_(0)
	{
		data_binary_ = NULL;
		data_binary_len_ = 0;

		m_pManager = pManager;

		m_bIsLocal = false;
		if (!NSFileDownloader::IsNeedDownload(sMainUrl))
			m_bIsLocal = true;
	}
	virtual ~ClientSchemeHandler()
	{
		if (NULL != data_binary_)
			delete [] data_binary_;
	}

	virtual void OnAsyncComplete(const std::wstring& sFile)
	{
		std::wstring sCopy = sFile;
		read_binary_file(sCopy, false);
	}

	virtual void ReleaseWrapper()
	{
		this->Release();
	}

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
								CefRefPtr<CefCallback> callback) OVERRIDE
	{
		CEF_REQUIRE_IO_THREAD();

		std::string url = request->GetURL().ToString();

		// страница, которая показывается при падении процесса рендерера
		std::string::size_type posFind = url.find("ascdesktop://crash.html");
		if (posFind != std::string::npos)
		{
			mime_type_ = "text/html";
			NSFile::CFileBinary::ReadAllTextUtf8A(m_pManager->m_oSettings.connection_error_path, data_);

			callback->Continue();
			return true;
		}

		// страница, которая показывается при ошибке загрузки
		posFind = url.find("ascdesktop://loaderror.html");
		if (posFind != std::string::npos)
		{
			mime_type_ = "text/html";
			NSFile::CFileBinary::ReadAllTextUtf8A(m_pManager->m_oSettings.connection_error_path, data_);

			callback->Continue();
			return true;
		}

		posFind = url.find("ascdesktop://fonts/");
		if (posFind != std::string::npos)
		{
			std::wstring sFile = read_file_path(request).substr(19);
			bool bIsCheck = true;
			if (0 == sFile.find(L"fonts_thumbnail"))
			{
				bIsCheck = false;
				sFile = (m_pManager->m_oSettings.fonts_cache_info_path + L"/" + NSFile::GetFileName(sFile));
				if (!NSFile::CFileBinary::Exists(sFile))
				{
					this->AddRef();
					NSResourceHandlerFileAsyncManager::Create(this, sFile, callback);
					return true;
				}
			}

#ifndef WIN32
			if (!sFile.empty())
			{
				if ('/' != sFile.c_str()[0])
					sFile = L"/" + sFile;
			}
#endif

			if (bIsCheck)
			{
				std::wstring::size_type posNativeViewer = sFile.rfind(L"?asc_native_viewer=");
				if (std::wstring::npos != posNativeViewer)
					sFile = sFile.substr(0, posNativeViewer);
			}

			if (bIsCheck && !m_bIsLocal && !m_pManager->IsResolveLocalFile(sFile))
				return false;

			read_binary_file(sFile, false);

			callback->Continue();
			return true;
		}

		posFind = url.find("ascdesktop://plugin_content/");
		if (posFind != std::string::npos)
		{
			std::wstring sFile = read_file_path(request).substr(28);
			if (!m_bIsLocal && !m_pManager->IsResolveLocalFile(sFile))
				return false;

			std::wstring::size_type posSearch = sFile.find(L"?lang=");
			if (std::wstring::npos != posSearch)
				sFile = sFile.substr(0, posSearch);

			if (0 == sFile.find(L"file:///"))
			{
				sFile = sFile.substr(7);
				if (!NSFile::CFileBinary::Exists(sFile))
					sFile = sFile.substr(1);
			}

			read_binary_file(sFile, true);

			callback->Continue();
			return true;
		}

		return false;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
	                                int64_t& response_length,
									CefString& redirectUrl) OVERRIDE
	{
		CEF_REQUIRE_IO_THREAD();

		DCHECK(!data_.empty() || !data_binary_);

		response->SetMimeType(mime_type_);
		response->SetStatus(200);

		CefResponse::HeaderMap headers;
		response->GetHeaderMap(headers);
		headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
		response->SetHeaderMap(headers);

		// Set the resulting response length
		response_length = (NULL == data_binary_) ? data_.length() : data_binary_len_;
	}

	virtual void Cancel() OVERRIDE
	{
		CEF_REQUIRE_IO_THREAD();
	}

	virtual bool ReadResponse(void* data_out,
							  int bytes_to_read,
							  int& bytes_read,
							  CefRefPtr<CefCallback> callback)
	OVERRIDE
	{
		CEF_REQUIRE_IO_THREAD();

		bool has_data = false;
		bytes_read = 0;

		BYTE* read_data = (BYTE*)data_binary_;
		size_t read_data_len = data_binary_len_;

		if (!read_data)
		{
			read_data = (BYTE*)data_.c_str();
			read_data_len = data_.length();
		}

		if (offset_ < read_data_len)
		{
			// Copy the next block of data into the buffer.
			int transfer_size = std::min(bytes_to_read, static_cast<int>(read_data_len - offset_));
			memcpy(data_out, read_data + offset_, transfer_size);
			offset_ += transfer_size;

			bytes_read = transfer_size;
			return true;
		}

		return false;
	}

private:

	std::wstring read_file_path(CefRefPtr<CefRequest>& request)
	{
		int nFlag = UU_SPACES | UU_REPLACE_PLUS_WITH_SPACE;
#if defined (_LINUX) && !defined(_MAC)
		nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#else
#ifndef CEF_2623
		nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#endif
#endif

		CefString cefUrl = request->GetURL();
		CefString cefFile = CefURIDecode(cefUrl, false, static_cast<cef_uri_unescape_rule_t>(nFlag));

		return cefFile.ToWString();
	}

	void read_binary_file(std::wstring& sFile, const bool& isCheckExt = false)
	{
		data_binary_len_ = (size_t)asc_scheme::read_file_with_urls(sFile, data_binary_);

		if (isCheckExt)
			mime_type_ = asc_scheme::GetMimeTypeFromExt(sFile);

		if (mime_type_.empty())
			mime_type_ = "*/*";
	}

private:
	std::string data_;
	std::string mime_type_;
	size_t offset_;

	BYTE* data_binary_;
	size_t data_binary_len_;

	CAscApplicationManager* m_pManager;
	bool m_bIsLocal;

	IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
};

namespace asc_scheme
{

	// Implementation of the factory for for creating schema handlers.
	class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory
	{
	private:
		CAscApplicationManager* m_pManager;

	public:
		ClientSchemeHandlerFactory(CAscApplicationManager* pManager = NULL)
		{
			m_pManager = pManager;
		}

	public:
		// Return a new scheme handler instance to handle the request.
		virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
													 CefRefPtr<CefFrame> frame,
													 const CefString& scheme_name,
													 CefRefPtr<CefRequest> request)
		OVERRIDE
		{
			CEF_REQUIRE_IO_THREAD();
			std::wstring sMainUrl = L"";
			if (browser && browser->GetMainFrame())
				sMainUrl = browser->GetMainFrame()->GetURL().ToWString();
			return new ClientSchemeHandler(m_pManager, sMainUrl);
		}

		IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
	};

#ifdef CEF_2623
	void RegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar,
							   std::vector<CefString>& cookiable_schemes)
	{
		registrar->AddCustomScheme("ascdesktop", true, false, false);
	}
#else
	void RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar
                               #ifndef CEF_VERSION_ABOVE_102
							   , std::vector<CefString>& cookiable_schemes
							   #endif
							   )
	{
		registrar->AddCustomScheme("ascdesktop", CEF_SCHEME_OPTION_STANDARD | CEF_SCHEME_OPTION_SECURE | CEF_SCHEME_OPTION_CORS_ENABLED/* | CEF_SCHEME_OPTION_FETCH_ENABLED*/);	
	}
#endif

	bool InitScheme(CAscApplicationManager* pManager)
	{
		return CefRegisterSchemeHandlerFactory("ascdesktop", "", new ClientSchemeHandlerFactory(pManager));
	}

}  // namespace asc_scheme

void client::ClientApp::RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar
                                              #ifndef CEF_VERSION_ABOVE_102
											  , std::vector<CefString>& cookiable_schemes
											  #endif
											  )
{
	return asc_scheme::RegisterCustomSchemes(registrar
                                         #ifndef CEF_VERSION_ABOVE_102
										 , cookiable_schemes
										 #endif
											 );
}
