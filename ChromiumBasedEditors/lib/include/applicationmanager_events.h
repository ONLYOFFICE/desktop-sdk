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

#ifndef APPLICATION_MANAGER_EVENTS_H
#define APPLICATION_MANAGER_EVENTS_H

#define ASC_MENU_EVENT_TYPE_CEF_CREATETAB       1001
#define ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE   1002
#define ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK      1003
#define ASC_MENU_EVENT_TYPE_CEF_KEYBOARDLAYOUT  1004
#define ASC_MENU_EVENT_TYPE_CEF_CONTROL_ID      1005
#define ASC_MENU_EVENT_TYPE_CEF_SYNC_COMMAND    1006
#define ASC_MENU_EVENT_TYPE_CEF_CLOSE           1007
#define ASC_MENU_EVENT_TYPE_CEF_DESTROY         1008
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD        1009
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START  1010
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_END    1011
#define ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED  1012

#define ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME   1013
#define ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT        1014

#define ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK      1015
#define ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE    1016
#define ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD  1017

#define ASC_MENU_EVENT_TYPE_CEF_DRAG_ENTER      1018
#define ASC_MENU_EVENT_TYPE_CEF_DRAG_LEAVE      1019
#define ASC_MENU_EVENT_TYPE_CEF_DROP            1020

#define ASC_MENU_EVENT_TYPE_CEF_ONCLOSE         2001
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE   2002

#define ASC_MENU_EVENT_TYPE_CEF_SAVE            3001
#define ASC_MENU_EVENT_TYPE_CEF_ONSAVE          3002

#define ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN      3100
#define ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER   3101
#define ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE   3102

#define ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE      4001
#define ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN     4002

#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_START        5001
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS     5002
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END          5003

#define ASC_MENU_EVENT_TYPE_CEF_PRINT_START                 5011
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE                  5012
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_END                   5013
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE_CHECK            5014

#define ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND             5015
#define ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS          5016
#define ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND      5017

#define ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION       6001

#define ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG              7001
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN              7002
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS           7003
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN        7004
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS          7005
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN       7006
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE            7007
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE              7008
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH         7009
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE          7010
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN             7011

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN      7013
#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_VIEW_DELAY      7014

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG 7015

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO     7016

#define ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO             7017
#define ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM           7018
#define ASC_MENU_EVENT_TYPE_REPORTER_CREATE                 7019
#define ASC_MENU_EVENT_TYPE_REPORTER_END                    7020

#define ASC_MENU_EVENT_TYPE_SSO_TOKEN                       7030

#define ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE               7031
#define ASC_MENU_EVENT_TYPE_PAGE_LOAD_START                 7032
#define ASC_MENU_EVENT_TYPE_PAGE_LOAD_END                   7033

#define ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE 7034

#define ASC_MENU_EVENT_TYPE_BINARY_FROM_RENDERER            7035

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG 7036

#define ASC_MENU_EVENT_TYPE_PAGE_GOT_FOCUS                  7050
#define ASC_MENU_EVENT_TYPE_PAGE_CRASH                      7051

#define ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW               8000

#define ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD           8001
#define ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END       8002
#define ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR 8003

#define ASC_MENU_EVENT_TYPE_EXECUTE_JS_CODE                 8004

#define ASC_MENU_EVENT_TYPE_ENCRYPT_PERSONAL_KEY_IMPORT     8005
#define ASC_MENU_EVENT_TYPE_ENCRYPT_PERSONAL_KEY_EXPORT     8006

#define ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS         8007
#define ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PROCESS         8008

#define ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START     8009
#define ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_END       8010

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVEFILENAME_DIALOG 8011

#define ASC_MENU_EVENT_TYPE_ON_NEED_UPDATE_APP              8012

#define ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE         9001

#define ASC_MENU_EVENT_TYPE_WINDOWS_MESSAGE_USER_COUNT      10

#include <string>
#include <vector>

namespace NSEditorApi
{
	class IMenuEventDataBase
	{
	protected:
		unsigned int m_lRef;

	public:
		IMenuEventDataBase()
		{
			m_lRef = 1;
		}

		virtual ~IMenuEventDataBase()
		{
		}

		virtual unsigned int AddRef()
		{
			++m_lRef;
			return m_lRef;
		}
		virtual unsigned int Release()
		{
			unsigned int ret = --m_lRef;
			if (0 == m_lRef)
				delete this;
			return ret;
		}
	};
}

namespace NSEditorApi
{
#define LINK_PROPERTY_INT(memberName)					\
	inline int get_##memberName()						\
	{													\
		return m_n##memberName;							\
	}													\
	inline void put_##memberName(const int& newVal)		\
	{													\
		m_n##memberName = newVal;						\
	}
#define LINK_PROPERTY_UINT(memberName)					\
	inline unsigned int get_##memberName()				\
	{													\
		return m_n##memberName;							\
	}													\
	inline void put_##memberName(const unsigned int& newVal)		\
	{													\
		m_n##memberName = newVal;						\
	}

#define LINK_PROPERTY_DOUBLE(memberName)				\
	inline double get_##memberName()					\
	{													\
		return m_d##memberName;							\
	}													\
	inline void put_##memberName(const double& newVal)	\
	{													\
		m_d##memberName = newVal;						\
	}

#define LINK_PROPERTY_BOOL(memberName)					\
	inline bool get_##memberName()						\
	{													\
		return m_b##memberName;							\
	}													\
	inline void put_##memberName(const bool& newVal)	\
	{													\
		m_b##memberName = newVal;						\
	}

#define LINK_PROPERTY_BYTE(memberName)					\
	inline unsigned char get_##memberName()				\
	{													\
		return m_n##memberName;							\
	}													\
	inline void put_##memberName(const unsigned char& newVal)	\
	{															\
		m_n##memberName = newVal;								\
	}

#define LINK_PROPERTY_STRING(memberName)						\
	inline std::wstring get_##memberName()						\
	{															\
		return m_s##memberName;									\
	}															\
	inline void put_##memberName(const std::wstring& newVal)	\
	{															\
		m_s##memberName = newVal;								\
	}
#define LINK_PROPERTY_STRINGA(memberName)						\
	inline std::string get_##memberName()						\
	{															\
		return m_s##memberName;									\
	}															\
	inline void put_##memberName(const std::string& newVal)		\
	{															\
		m_s##memberName = newVal;								\
	}

#define LINK_PROPERTY_OBJECT(objectType, memberName)			\
	inline objectType& get_##memberName()						\
	{                                                           \
		return m_o##memberName;                                 \
	}                                                           \
	inline void put_##memberName(const objectType& newVal)      \
	{                                                           \
		m_o##memberName = newVal;                               \
	}

// JS
#define LINK_PROPERTY_INT_JS(memberName)								\
	inline js_wrapper<int>& get_##memberName()							\
	{																	\
		return m_n##memberName;											\
	}																	\
	inline void put_##memberName(const int& newVal)						\
	{																	\
		m_n##memberName = newVal;										\
	}																	\
	inline void put_##memberName(const js_wrapper<int>& newVal)			\
	{																	\
		m_n##memberName = newVal;										\
	}

#define LINK_PROPERTY_DOUBLE_JS(memberName)								\
	inline js_wrapper<double>& get_##memberName()						\
	{																	\
		return m_d##memberName;											\
	}																	\
	inline void put_##memberName(const double& newVal)					\
	{																	\
		m_d##memberName = newVal;										\
	}																	\
	inline void put_##memberName(const js_wrapper<double>& newVal)		\
	{																	\
		m_d##memberName = newVal;										\
	}

#define LINK_PROPERTY_BOOL_JS(memberName)								\
	inline js_wrapper<bool>& get_##memberName()							\
	{																	\
		return m_b##memberName;											\
	}																	\
	inline void put_##memberName(const bool& newVal)					\
	{																	\
		m_b##memberName = newVal;										\
	}																	\
	inline void put_##memberName(const js_wrapper<bool>& newVal)		\
	{																	\
		m_b##memberName = newVal;										\
	}

#define LINK_PROPERTY_BYTE_JS(memberName)								\
	inline js_wrapper<unsigned char>& get_##memberName()				\
	{																	\
		return m_n##memberName;											\
	}																	\
	inline void put_##memberName(const unsigned char& newVal)			\
	{																	\
		m_n##memberName = newVal;										\
	}																	\
	inline void put_##memberName(const js_wrapper<unsigned char>& newVal)		\
	{																	\
		m_n##memberName = newVal;										\
	}

#define LINK_PROPERTY_STRING_JS(memberName)									\
	inline js_wrapper<std::wstring>& get_##memberName()						\
	{																		\
		return m_s##memberName;												\
	}																		\
	inline void put_##memberName(const std::wstring& newVal)				\
	{																		\
		m_s##memberName = newVal;											\
	}																		\
	inline void put_##memberName(const js_wrapper<std::wstring>& newVal)	\
	{																		\
		m_s##memberName = newVal;											\
	}

#define LINK_PROPERTY_STRINGA_JS(memberName)								\
	inline js_wrapper<std::string>& get_##memberName()						\
	{																		\
		return m_s##memberName;												\
	}																		\
	inline void put_##memberName(const std::string& newVal)					\
	{																		\
		m_s##memberName = newVal;											\
	}																		\
	inline void put_##memberName(const js_wrapper<std::string>& newVal)		\
	{																		\
		m_s##memberName = newVal;											\
	}

#define LINK_PROPERTY_OBJECT_JS(objectType, memberName)						\
	inline js_wrapper<objectType>& get_##memberName()						\
	{																		\
		return m_o##memberName;												\
	}																		\
	inline void put_##memberName(const js_wrapper<objectType>& newVal)		\
	{																		\
		m_o##memberName = newVal;											\
	}																		\
	inline void put_##memberName(objectType* newVal)						\
	{																		\
		m_o##memberName = newVal;											\
	}
}

namespace NSEditorApi
{
	class CAscMenuEvent : public IMenuEventDataBase
	{
	public:
		int					m_nType;
		IMenuEventDataBase*	m_pData;

	public:
		CAscMenuEvent(int nType = -1)
					: m_nType(nType)
					, m_pData(NULL)
		{
		}
		virtual ~CAscMenuEvent()
		{
			if (NULL != m_pData)
				m_pData->Release();
		}
	};

	class CAscCefMenuEvent : public CAscMenuEvent
	{
	public:
		int	m_nSenderId;

	public:
		CAscCefMenuEvent(int nType = -1) : CAscMenuEvent(nType)
		{
			m_nSenderId = -1;
		}
		virtual ~CAscCefMenuEvent()
		{
		}

		LINK_PROPERTY_INT(SenderId)
	};

	class CAscCefMenuEventListener
	{
	public:
		// memory release!!!
		virtual void OnEvent(CAscCefMenuEvent* pEvent)
		{
			if (NULL != pEvent)
				pEvent->Release();
		}
		virtual bool IsSupportEvent(int nEventType)
		{
			return true;
		}
	};
}

namespace NSEditorApi
{
	class CAscLocalRecentsAll : public IMenuEventDataBase
	{
	private:
		std::wstring m_sJSON;
		int m_nId;

	public:

		CAscLocalRecentsAll()
		{
			m_nId = -1;
		}
		virtual ~CAscLocalRecentsAll()
		{
		}

		LINK_PROPERTY_STRING(JSON)
		LINK_PROPERTY_INT(Id)
	};

	class CAscLocalOpenFileRecent_Recover : public IMenuEventDataBase
	{
	private:
		int m_nId;
		bool m_bIsRecover;
		std::wstring m_sPath;
		bool m_bIsRemove;

	public:

		CAscLocalOpenFileRecent_Recover()
		{
			m_bIsRecover = false;
			m_bIsRemove = false;
		}
		virtual ~CAscLocalOpenFileRecent_Recover()
		{
		}

		LINK_PROPERTY_BOOL(IsRecover)
		LINK_PROPERTY_BOOL(IsRemove)
		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_STRING(Path)
	};

	class CAscLocalFileOpen : public IMenuEventDataBase
	{
	private:
		std::wstring m_sDirectory;

	public:

		CAscLocalFileOpen()
		{
		}
		virtual ~CAscLocalFileOpen()
		{
		}

		LINK_PROPERTY_STRING(Directory)
	};

	class CAscLocalFileCreate : public IMenuEventDataBase
	{
	private:
		int m_nType;

	public:

		CAscLocalFileCreate()
		{
		}
		virtual ~CAscLocalFileCreate()
		{
		}

		LINK_PROPERTY_INT(Type)
	};
}

namespace NSEditorApi
{
	class CAscExecCommand : public IMenuEventDataBase
	{
	private:
		std::wstring m_sCommand;
		std::wstring m_sParam;

	public:

		CAscExecCommand()
		{
		}
		virtual ~CAscExecCommand()
		{
		}

		LINK_PROPERTY_STRING(Command)
		LINK_PROPERTY_STRING(Param)
	};

	class CAscExecCommandJS : public IMenuEventDataBase
	{
	private:
		std::wstring m_sCommand;
		std::wstring m_sParam;
		std::wstring m_sFrameName;

	public:

		CAscExecCommandJS()
		{
		}
		virtual ~CAscExecCommandJS()
		{
		}

		LINK_PROPERTY_STRING(Command)
		LINK_PROPERTY_STRING(Param)
		LINK_PROPERTY_STRING(FrameName)
	};
}

namespace NSEditorApi
{
	class CAscCreateTab : public IMenuEventDataBase
	{
	private:
		std::wstring m_sUrl;
		std::wstring m_sName;
		bool m_bActive;
		int m_nIdEqual;

	public:
		CAscCreateTab()
		{
			m_bActive = true;
			m_nIdEqual = -1;
		}
		virtual ~CAscCreateTab()
		{
		}

		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_BOOL(Active)
		LINK_PROPERTY_INT(IdEqual)
		LINK_PROPERTY_STRING(Name)
	};

	class CAscDownloadFileInfo : public IMenuEventDataBase
	{
	private:
		std::wstring    m_sUrl;
		std::wstring    m_sFilePath;

		int             m_nId;

		double          m_dSpeed;       // Kb/sec
		int             m_nPercent;
		bool            m_bIsComplete;
		bool            m_bIsCanceled;

		unsigned int    m_nIdDownload;

	public:
		CAscDownloadFileInfo()
		{
			m_nId           = -1;

			m_dSpeed        = 0;
			m_nPercent      = 0;
			m_bIsComplete   = false;
			m_bIsCanceled     = false;
		}
		virtual ~CAscDownloadFileInfo()
		{
		}
		CAscDownloadFileInfo* Copy()
		{
			CAscDownloadFileInfo* ret = new CAscDownloadFileInfo();
			ret->m_sUrl = m_sUrl;
			ret->m_sFilePath = m_sFilePath;

			ret->m_nId = m_nId;

			ret->m_dSpeed = m_dSpeed;
			ret->m_nPercent = m_nPercent;
			ret->m_bIsComplete = m_bIsComplete;
			ret->m_bIsCanceled = m_bIsCanceled;

			ret->m_nIdDownload = m_nIdDownload;

			return ret;
		}

		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_STRING(FilePath)
		LINK_PROPERTY_INT(Percent)
		LINK_PROPERTY_BOOL(IsComplete)
		LINK_PROPERTY_BOOL(IsCanceled)
		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_DOUBLE(Speed)
		LINK_PROPERTY_UINT(IdDownload)
	};

	class CAscOnOpenExternalLink : public IMenuEventDataBase
	{
	private:
		std::wstring    m_sUrl;
		int             m_nId;

	public:
		CAscOnOpenExternalLink()
		{
			m_nId           = -1;
		}
		virtual ~CAscOnOpenExternalLink()
		{
		}

		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_INT(Id)
	};

	class CAscTabEditorType : public IMenuEventDataBase
	{
	private:
		int m_nId;
		int m_nType;

	public:
		CAscTabEditorType()
		{
		}
		virtual ~CAscTabEditorType()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_INT(Type)
	};

	class CAscSpellCheckType : public IMenuEventDataBase
	{
	private:
		std::string m_sResult;
		int         m_nEditorId;
		long long   m_nFrameId;

	public:
		CAscSpellCheckType()
		{
			m_nEditorId = 0;
			m_nFrameId = 0;
		}
		virtual ~CAscSpellCheckType()
		{
		}

		LINK_PROPERTY_STRINGA(Result)
		LINK_PROPERTY_INT(EditorId)

		inline long long get_FrameId()
		{
			return m_nFrameId;
		}
		inline void put_FrameId(const long long& newVal)
		{
			m_nFrameId = newVal;
		}
	};

	class CAscKeyboardLayout : public IMenuEventDataBase
	{
	private:
		int         m_nLanguage;

	public:
		CAscKeyboardLayout()
		{
		}
		virtual ~CAscKeyboardLayout()
		{
		}

		LINK_PROPERTY_INT(Language)
	};

	class CAscTypeId : public IMenuEventDataBase
	{
	private:
		int m_nId;

	public:
		CAscTypeId()
		{
		}
		virtual ~CAscTypeId()
		{
		}

		LINK_PROPERTY_INT(Id)
	};

	class CAscDocumentOnSaveData : public IMenuEventDataBase
	{
	private:
		int m_nId;
		bool m_bIsCancel;

	public:
		CAscDocumentOnSaveData()
		{
			m_bIsCancel = false;
		}
		virtual ~CAscDocumentOnSaveData()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_BOOL(IsCancel)
	};

	class CAscDocumentModifyChanged : public IMenuEventDataBase
	{
	private:
		int m_nId;
		bool m_bChanged;

	public:
		CAscDocumentModifyChanged()
		{
		}
		virtual ~CAscDocumentModifyChanged()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_BOOL(Changed)
	};

	class CAscDocumentName : public IMenuEventDataBase
	{
	private:
		std::wstring m_sName;
		std::wstring m_sPath;
		std::wstring m_sUrl;
		int m_nId;

	public:
		CAscDocumentName()
		{
		}
		virtual ~CAscDocumentName()
		{
		}

		LINK_PROPERTY_STRING(Name)
		LINK_PROPERTY_STRING(Path)
		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_INT(Id)
	};

	class CAscJSMessage : public IMenuEventDataBase
	{
	private:
		std::wstring m_sName;
		std::wstring m_sValue;
		int m_nId;

	public:
		CAscJSMessage()
		{
		}
		virtual ~CAscJSMessage()
		{
		}

		LINK_PROPERTY_STRING(Name)
		LINK_PROPERTY_STRING(Value)
		LINK_PROPERTY_INT(Id)
	};

	class CAscPrintProgress : public IMenuEventDataBase
	{
	private:
		int m_nId;
		int m_nProgress;

	public:
		CAscPrintProgress()
		{
		}
		virtual ~CAscPrintProgress()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_INT(Progress)
	};

	class CAscPrintEnd : public IMenuEventDataBase
	{
	private:
		int m_nId;
		int m_nPagesCount;
		int m_nCurrentPage;
		std::wstring m_sOptions;

	public:
		CAscPrintEnd()
		{
		}
		virtual ~CAscPrintEnd()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_INT(PagesCount)
		LINK_PROPERTY_INT(CurrentPage)
		LINK_PROPERTY_STRING(Options)
	};

	class CAscPrinterContextBase : public IMenuEventDataBase
	{
	public:
		CAscPrinterContextBase() : IMenuEventDataBase() {}
		virtual ~CAscPrinterContextBase() {}

		virtual void GetLogicalDPI(int& nDpiX, int& nDpiY) = 0;
		virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH) = 0;
		virtual void GetPrintAreaSize(int& nW, int& nH) = 0;
		virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
							const double& x, const double& y, const double& w, const double& h, const double& dAngle) = 0;

		virtual void* GetNativeRenderer() { return NULL; }
		virtual void* GetNativeRendererUnsupportChecker() { return NULL; }
		virtual void NewPage() {}

		virtual void InitRenderer(void* pRenderer, void* pFontManager) {}

		virtual void PrepareBitBlt(void* pRenderer, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
								   const double& x, const double& y, const double& w, const double& h, const double& dAngle) {}

		virtual void SetPageOrientation(int nOrientaion) {}

		virtual void SaveState() {}
		virtual void RestoreState() {}
	};

	class CAscPrintPage : public IMenuEventDataBase
	{
	private:
		CAscPrinterContextBase* m_pContext;
		int                     m_nPage;
		bool                    m_bIsRotate;

	public:
		CAscPrintPage()
		{
			m_pContext = NULL;
			m_nPage = 0;
		}
		virtual ~CAscPrintPage()
		{
			if (NULL != m_pContext)
				m_pContext->Release();
		}

		LINK_PROPERTY_INT(Page)
		LINK_PROPERTY_BOOL(IsRotate)

		inline CAscPrinterContextBase* get_Context()
		{
			return m_pContext;
		}
		inline void put_Context(CAscPrinterContextBase* pContext)
		{
			m_pContext = pContext;
		}
	};

	class CAscEditorScript : public IMenuEventDataBase
	{
	private:
		std::wstring m_sUrl;
		std::wstring m_sDestination;
		int m_nId;
		long long m_nFrameId;

	public:
		CAscEditorScript()
		{
			m_nId = -1;
			m_nFrameId = -1;
		}
		virtual ~CAscEditorScript()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_STRING(Destination)

		inline long long get_FrameId()
		{
			return m_nFrameId;
		}
		inline void put_FrameId(const long long& newVal)
		{
			m_nFrameId = newVal;
		}
	};

	class CAscEditorSaveQuestion : public IMenuEventDataBase
	{
	private:
		int m_nId;
		bool m_bValue;

	public:
		CAscEditorSaveQuestion()
		{
			m_nId = -1;
			m_bValue = false;
		}
		virtual ~CAscEditorSaveQuestion()
		{
		}

		LINK_PROPERTY_BOOL(Value)
		LINK_PROPERTY_INT(Id)
	};
}

namespace NSEditorApi
{
	class CAscSaveDialog : public IMenuEventDataBase
	{
	private:
		std::wstring    m_sFilePath;
		int             m_nId;
		unsigned int    m_nIdDownload;

	public:
		CAscSaveDialog()
		{
			m_nId           = -1;
		}
		virtual ~CAscSaveDialog()
		{
		}

		LINK_PROPERTY_STRING(FilePath)
		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_UINT(IdDownload)
	};
}

namespace NSEditorApi
{
	class CAscKeyboardDown : public IMenuEventDataBase
	{
	private:
		int m_nKeyCode;
		bool m_bIsCtrl;
		bool m_bIsShift;
		bool m_bIsAlt;
		bool m_bIsCommandMac;

	public:
		CAscKeyboardDown()
		{
			m_nKeyCode  = -1;
			m_bIsCtrl   = false;
			m_bIsShift  = false;
			m_bIsAlt    = false;
			m_bIsCommandMac = false;
		}
		virtual ~CAscKeyboardDown()
		{
		}

		LINK_PROPERTY_INT(KeyCode)
		LINK_PROPERTY_BOOL(IsCtrl)
		LINK_PROPERTY_BOOL(IsShift)
		LINK_PROPERTY_BOOL(IsAlt)
		LINK_PROPERTY_BOOL(IsCommandMac)
	};
}

namespace NSEditorApi
{
	class CAscLocalSaveFileDialog : public IMenuEventDataBase
	{
	private:
		int m_nId;
		int m_nFileType;
		std::wstring m_sPath;
		std::vector<int> m_arSupportFormats;

	public:
		CAscLocalSaveFileDialog()
		{
			m_nId  = -1;
			m_nFileType = 0;
		}
		virtual ~CAscLocalSaveFileDialog()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_INT(FileType)
		LINK_PROPERTY_STRING(Path)

		std::vector<int>& get_SupportFormats()
		{
			return m_arSupportFormats;
		}
	};

	class CAscLocalDragDropData : public IMenuEventDataBase
	{
	private:
		int m_nX;
		int m_nY;
		std::wstring m_sText;
		std::wstring m_sHtml;
		std::vector<std::wstring> m_arFiles;

	public:
		CAscLocalDragDropData()
		{
			m_nX = 0;
			m_nY = 0;
			m_sText = L"";
			m_sHtml = L"";
		}
		virtual ~CAscLocalDragDropData()
		{
		}

		LINK_PROPERTY_INT(X)
		LINK_PROPERTY_INT(Y)
		LINK_PROPERTY_STRING(Text)
		LINK_PROPERTY_STRING(Html)

		void add_File(const std::wstring& sPath)
		{
			m_arFiles.push_back(sPath);
		}

		std::vector<std::wstring>& get_Files()
		{
			return m_arFiles;
		}
	};

	class CAscLocalOpenFileDialog : public IMenuEventDataBase
	{
	private:
		int m_nId;
		std::wstring m_sPath;
		std::wstring m_sFilter;

		bool m_bIsMultiselect;
		std::vector<std::wstring> m_arPaths;

	public:
		CAscLocalOpenFileDialog()
		{
			m_nId  = -1;
			m_bIsMultiselect = false;
		}
		virtual ~CAscLocalOpenFileDialog()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_STRING(Path)
		LINK_PROPERTY_STRING(Filter)

		LINK_PROPERTY_BOOL(IsMultiselect)

		std::vector<std::wstring>& get_Files()
		{
			return m_arPaths;
		}
	};

	class CAscLocalOpenDirectoryDialog : public IMenuEventDataBase
	{
	private:
		std::wstring m_sPath;

	public:
		CAscLocalOpenDirectoryDialog()
		{
		}
		virtual ~CAscLocalOpenDirectoryDialog()
		{
		}

		LINK_PROPERTY_STRING(Path)
	};

	class CAscLocalSaveFileNameDialog : public IMenuEventDataBase
	{
	private:
		int m_nId;
		std::wstring m_sPath;
		std::wstring m_sFilter;

	public:
		CAscLocalSaveFileNameDialog()
		{
			m_nId  = -1;
		}
		virtual ~CAscLocalSaveFileNameDialog()
		{
		}

		LINK_PROPERTY_INT(Id)
		LINK_PROPERTY_STRING(Path)
		LINK_PROPERTY_STRING(Filter)
	};

	class CAscLocalOpenFiles : public IMenuEventDataBase
	{
	private:
		std::vector<std::wstring> m_arFiles;

	public:
		CAscLocalOpenFiles()
		{
		}
		virtual ~CAscLocalOpenFiles()
		{
		}

		std::vector<std::wstring>& get_Files()
		{
			return m_arFiles;
		}
	};

	class CAscEditorExecuteCommand : public IMenuEventDataBase
	{
	private:
		std::wstring m_sCommand;
		std::wstring m_sParams;

	public:
		CAscEditorExecuteCommand()
		{
		}
		virtual ~CAscEditorExecuteCommand()
		{
		}

		LINK_PROPERTY_STRING(Command)
		LINK_PROPERTY_STRING(Params)
	};
}

namespace NSEditorApi
{
	class CAscAddPlugin : public IMenuEventDataBase
	{
	private:
		std::wstring m_sPath;

	public:
		CAscAddPlugin()
		{
		}
		virtual ~CAscAddPlugin()
		{
		}

		LINK_PROPERTY_STRING(Path)
	};
}

namespace NSEditorApi
{
	class CAscReporterMessage : public IMenuEventDataBase
	{
	private:
		std::wstring m_sMessage;
		int m_nReceiverId;

	public:
		CAscReporterMessage()
		{
			m_nReceiverId = -1;
		}
		virtual ~CAscReporterMessage()
		{
		}

		LINK_PROPERTY_STRING(Message)
		LINK_PROPERTY_INT(ReceiverId)
	};

	class CAscReporterCreate : public IMenuEventDataBase
	{
	private:
		void* m_pData;

	public:
		CAscReporterCreate()
		{
			m_pData = NULL;
		}
		virtual ~CAscReporterCreate()
		{
		}

		void* get_Data() { return m_pData; }
		void put_Data(void* data) { m_pData = data; }
	};
}

namespace NSEditorApi
{
	class CAscSSOToken : public IMenuEventDataBase
	{
	private:
		std::wstring m_sUrl;
		std::wstring m_sToken;

	public:
		CAscSSOToken()
		{
		}
		virtual ~CAscSSOToken()
		{
		}

		LINK_PROPERTY_STRING(Url)
		LINK_PROPERTY_STRING(Token)
	};
}

namespace NSEditorApi
{
	class CAscUIThreadMessage : public IMenuEventDataBase
	{
	private:
		int m_nType;
		std::vector<std::wstring> m_arArgs;

	public:
		CAscUIThreadMessage()
		{
		}
		virtual ~CAscUIThreadMessage()
		{
		}

		LINK_PROPERTY_INT(Type)
		std::vector<std::wstring>& GetArgs() { return m_arArgs; }
	};
}

namespace NSEditorApi
{
	class CAscOpenSslData : public IMenuEventDataBase
	{
	private:
		std::wstring m_sCertPath;
		std::wstring m_sCertPassword;

		std::wstring m_sKeyPath;
		std::wstring m_sKeyPassword;

	public:
		CAscOpenSslData()
		{
		}
		virtual ~CAscOpenSslData()
		{
		}

		LINK_PROPERTY_STRING(CertPath)
		LINK_PROPERTY_STRING(CertPassword)
		LINK_PROPERTY_STRING(KeyPath)
		LINK_PROPERTY_STRING(KeyPassword)
	};

	class CEncryptData : public IMenuEventDataBase
	{
	public:
		CEncryptData(){}
		virtual ~CEncryptData(){}

		LINK_PROPERTY_STRING(Path)
		LINK_PROPERTY_STRING(Value)
		private:
			std::wstring m_sPath;
		std::wstring m_sValue;
	};
}

namespace NSEditorApi
{
	class CAscBinaryMessage : public IMenuEventDataBase
	{
	private:
		std::string     m_sMessageName;
		unsigned char*  m_pData;
		unsigned int    m_nDataSize;

	public:
		CAscBinaryMessage()
		{
			m_pData = NULL;
			m_nDataSize = 0;
		}
		virtual ~CAscBinaryMessage()
		{
		}

		LINK_PROPERTY_STRINGA(MessageName)

		inline unsigned char* get_Data()
		{
			return m_pData;
		}
		inline void put_Data(unsigned char* pData)
		{
			m_pData = pData;
		}

		inline unsigned int get_DataSize()
		{
			return m_nDataSize;
		}
		inline void put_DataSize(unsigned int nSize)
		{
			m_nDataSize = nSize;
		}
	};
}

namespace NSEditorApi
{
	class CAscSystemExternalPlugins : public IMenuEventDataBase
	{
	public:
		class CItem
		{
		public:
			std::wstring name;
			std::wstring nameLocale;
			std::wstring id;
			std::wstring url;
		};

	private:
		std::vector<CItem> m_items;

	public:
		CAscSystemExternalPlugins()
		{
		}
		virtual ~CAscSystemExternalPlugins()
		{
		}

		std::vector<CItem>& get_Items()
		{
			return m_items;
		}

		void addItem(const std::wstring& name, const std::wstring& id, const std::wstring& url, const std::wstring& name_local)
		{
			CItem item;
			item.name = name;
			item.id = id;
			item.url = url;
			item.nameLocale = name_local;
			m_items.push_back(item);
		}
	};
}

namespace NSEditorApi
{
	class CAscExternalProcess : public IMenuEventDataBase
	{
	private:
		std::wstring                m_sProgram;
		std::vector<std::wstring>   m_arArguments;
		std::wstring                m_sWorkingDirectory;
		bool m_bDetached;

	public:
		CAscExternalProcess()
		{
		}
		virtual ~CAscExternalProcess()
		{
		}

		LINK_PROPERTY_STRING(Program)
		LINK_PROPERTY_STRING(WorkingDirectory)
		LINK_PROPERTY_BOOL(Detached)
		std::vector<std::wstring>& get_Arguments() { return m_arArguments; }
	};

	class CAscExternalMedia : public IMenuEventDataBase
	{
	private:
		int m_nX;
		int m_nY;
		double m_dW;
		double m_dH;

		int m_nBoundsX;
		int m_nBoundsY;
		int m_nBoundsW;
		int m_nBoundsH;

		double m_dsx;
		double m_dshy;
		double m_dshx;
		double m_dsy;
		double m_dtx;
		double m_dty;

		std::wstring m_sUrl;

	public:
		CAscExternalMedia()
		{
			m_nX = 0;
			m_nY = 0;
			m_dW = 0;
			m_dH = 0;

			m_nBoundsX = 0;
			m_nBoundsY = 0;
			m_nBoundsW = 0;
			m_nBoundsH = 0;

			m_dsx    = 1;
			m_dshy   = 0;
			m_dshx   = 0;
			m_dsy    = 1;
			m_dtx    = 0;
			m_dty    = 0;
		}
		virtual ~CAscExternalMedia()
		{
		}

		LINK_PROPERTY_STRING(Url)

		LINK_PROPERTY_INT(X)
		LINK_PROPERTY_INT(Y)
		LINK_PROPERTY_DOUBLE(W)
		LINK_PROPERTY_DOUBLE(H)

		LINK_PROPERTY_INT(BoundsX)
		LINK_PROPERTY_INT(BoundsY)
		LINK_PROPERTY_INT(BoundsW)
		LINK_PROPERTY_INT(BoundsH)

		LINK_PROPERTY_DOUBLE(sx)
		LINK_PROPERTY_DOUBLE(shy)
		LINK_PROPERTY_DOUBLE(shx)
		LINK_PROPERTY_DOUBLE(sy)
		LINK_PROPERTY_DOUBLE(tx)
		LINK_PROPERTY_DOUBLE(ty)
	};
}

namespace NSEditorApi
{
	class CAscX509CertificateData : public IMenuEventDataBase
	{
	private:
		std::wstring m_sData;
		std::wstring m_sFilePath; // used only on macOS

	public:
		CAscX509CertificateData()
		{
		}
		virtual ~CAscX509CertificateData()
		{
		}

		LINK_PROPERTY_STRING(Data)
		LINK_PROPERTY_STRING(FilePath)
	};
}

namespace NSEditorApi
{
	typedef enum {
		utNone = 0
	} AscUpdateAppType;

	class CAscUpdateApp : public IMenuEventDataBase
	{
	private:
		AscUpdateAppType m_nType;

	public:
		CAscUpdateApp()
		{
			m_nType = utNone;
		}
		virtual ~CAscUpdateApp()
		{
		}

		inline AscUpdateAppType get_Type()
		{
			return m_nType;
		}
		inline void put_Type(const AscUpdateAppType& newVal)
		{
			m_nType = newVal;
		}
	};
}

#endif // APPLICATION_MANAGER_EVENTS_H
