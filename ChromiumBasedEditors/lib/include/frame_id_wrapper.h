#ifndef FRAME_ID_WRAPPER_H
#define FRAME_ID_WRAPPER_H

#include <string>

#ifdef CEF_VERSION_138
    using frame_id_t = std::string;
#else
    using frame_id_t = int64_t;
#endif // CEF_VERSION_138

namespace NSSupport
{
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
		CFrameIdImpl(const std::string& other)
		{
			// just in case
			static_assert(std::is_same<FrameIdType, int64_t>::value,
			        "call CFrameId::CFrameId(const std::string& other) with FrameIdType is not int64_t");

			id = static_cast<int64_t>(std::stoll(other));
		}
		FrameIdType& operator=(const std::string& other)
		{
			// just in case
			static_assert(std::is_same<FrameIdType, int64_t>::value,
			        "call CFrameId::CFrameId(const std::string& other) with FrameIdType is not int64_t");

			id = static_cast<int64_t>(std::stoll(other));
			return *this;
		}
#endif // CEF_VERSION_138
	};

	using CFrameId = CFrameIdImpl<frame_id_t>;
} // namespace NSSupport
#endif // FRAME_ID_WRAPPER_H
