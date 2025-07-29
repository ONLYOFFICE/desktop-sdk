#pragma once

#include "include/cef_browser.h"
#include "include/cef_base.h"
#include "include/cef_frame.h"
#include "include/cef_values.h"

namespace NSSupport
{
	static CefString GetIdentifier(CefRefPtr<CefFrame> frame)
	{
#ifdef CEF_VERSION_138
		return frame->GetIdentifier();
#else
		return std::to_string(frame->GetIdentifier());
#endif // CEF_VERSION_138
	}

#ifdef CEF_VERSION_138
	static CefRefPtr<CefFrame> GetFrame(CefRefPtr<CefBrowser> browser, const CefString& name)
	{
		return browser->GetFrameByName(name);
	}
	static CefRefPtr<CefFrame> GetFrame(CefRefPtr<CefBrowser> browser, int64_t id)
	{
		return browser->GetFrameByIdentifier(std::to_string(id));
	}
	static std::vector<CefString> GetFrameIdentifiers(CefRefPtr<CefBrowser> browser)
	{
		std::vector<CefString> arIds;
		browser->GetFrameIdentifiers(arIds);

		// copy elision
		return arIds;
	}
# else
	template <class T>
	static CefRefPtr<CefFrame> GetFrame(CefRefPtr<CefBrowser> browser, T&& value)
	{
		return browser->GetFrame(std::forward<T>(value));
	}

	static std::vector<int64_t> GetFrameIdentifiers(CefRefPtr<CefBrowser> browser)
	{
		std::vector<int64_t> arIds;
		browser->GetFrameIdentifiers(arIds);

		// copy elision
		return arIds;
	}
#endif // CEF_VERSION_138
} // namespace NSSupport

