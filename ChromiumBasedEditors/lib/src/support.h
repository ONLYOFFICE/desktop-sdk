#pragma once

#include "include/cef_browser.h"
#include "include/cef_base.h"
#include "include/cef_frame.h"
#include "include/cef_values.h"

namespace NSSupport
{

#ifdef CEF_VERSION_138
	using frame_id_t = CefString;
#else
	using frame_id_t = int64_t;
#endif // CEF_VERSION_138

#ifdef CEF_VERSION_138
	static CefRefPtr<CefFrame> GetFrameByName(CefRefPtr<CefBrowser> browser, const CefString& name)
	{
		return browser->GetFrameByName(name);
	}
	static CefRefPtr<CefFrame> GetFrameByIdentifier(CefRefPtr<CefBrowser> browser, const CefString& id)
	{
		return browser->GetFrameByIdentifier(id);
	}
# else
	static CefRefPtr<CefFrame> GetFrameByName(CefRefPtr<CefBrowser> browser, const CefString& name)
	{
		return browser->GetFrame(name);
	}
	static CefRefPtr<CefFrame> GetFrameByIdentifier(CefRefPtr<CefBrowser> browser, const int64_t id)
	{
		return browser->GetFrame(id);
	}
#endif // CEF_VERSION_138

	static std::vector<frame_id_t> GetFrameIdentifiers(CefRefPtr<CefBrowser> browser)
	{
		std::vector<frame_id_t> arIds;
		browser->GetFrameIdentifiers(arIds);

		// copy elision
		return arIds;
	}

	/**
	 * Use this class if you want to create/declare/store
	 * an id for a frame. Due backward compatibility you should use this class,
	 * since in new versions the id is a СefString, in previous ones - int64_t.
	 * Use ToString() to fill it in the args (message out).
	 * Use operator= from args->GetString to get frame id.
	 * To pass it to NSSupport::GetFrame use GetId().
	 */
	template <class FrameIdType = frame_id_t>
	class CFrameIdImpl
	{
	protected:
		FrameIdType id {};

	public:
		CFrameIdImpl()
		{
#ifndef CEF_VERSION_138
			id = -1;
#endif // CEF_VERSION_138
		}
		CFrameIdImpl(const CFrameIdImpl&) = default;
		CFrameIdImpl(CFrameIdImpl&&) noexcept = default;
		virtual ~CFrameIdImpl() = default;

		CFrameIdImpl& operator=(const CFrameIdImpl&) = default;
		CFrameIdImpl& operator=(CFrameIdImpl&&) noexcept = default;

		CFrameIdImpl(const FrameIdType& other) :
		    id(other) // id is int64_t or CefString
		{
		}
		CFrameIdImpl& operator=(const FrameIdType& other)
		{
			id = other; // id is int64_t or CefString
			return *this;
		}

		FrameIdType& GetId()
		{
			return id; // id is int64_t or CefString
		}

		const FrameIdType& GetId() const
		{
			return id; // id is int64_t or CefString
		}

		std::string ToString() const
		{
#ifdef CEF_VERSION_138
			return id;
#else
			return std::to_string(id);
#endif // CEF_VERSION_138
		}

		bool IsEmpty() const
		{
#ifdef CEF_VERSION_138
			return id.empty();
#else
			return id == -1;
#endif // CEF_VERSION_138
		}

		void Clear()
		{
#ifdef CEF_VERSION_138
			id.clear();
#else
			id = -1;
#endif // CEF_VERSION_138
		}

#ifndef CEF_VERSION_138
		CFrameIdImpl(const CefString& other)
		{
			// just in case
			static_assert(std::is_same<FrameIdType, int64_t>::value,
			        "call CFrameId::CFrameId(const CefString& other) with FrameIdType is not int64_t");

			std::string tmp = other.ToString();
			id = static_cast<int64_t>(std::stoll(tmp));
		}
		CFrameIdImpl& operator=(const CefString& other)
		{
			// just in case
			static_assert(std::is_same<FrameIdType, int64_t>::value,
			        "call CFrameId::CFrameId(const CefString& other) with FrameIdType is not int64_t");

			std::string tmp = other.ToString();
			id = static_cast<int64_t>(std::stoll(tmp));
			return *this;
		}
#endif // CEF_VERSION_138
	};

	using CFrameId = CFrameIdImpl<frame_id_t>;
} // namespace NSSupport

