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

#include "./client_renderer.h"

#include <sstream>
#include <algorithm>
#include <string>

#include "include/cef_dom.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_message_router.h"

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../../core/DesktopEditor/raster/ImageFileFormatChecker.h"
#include "../../src/applicationmanager_p.h"

#include "../../../../../core/DesktopEditor/raster/BgraFrame.h"
#include "../../../../../core/DesktopEditor/graphics/pro/Image.h"

#include "../../../../../core/HtmlRenderer/include/ASCSVGWriter.h"

#include "../../src/nativeviewer.h"
#include "../../src/plugins.h"
#include "../../src/providers.h"

#include "../../src/additional/renderer.h"
#include "../crypto_mode.h"
#include "../../../../../core/DesktopEditor/common/CalculatorCRC32.h"
#include "./client_renderer_params.h"
#include "../../../../../core/Common/3dParty/openssl/common/common_openssl.h"
#include "../../../../../core/OfficeCryptReader/source/ECMACryptFile.h"

#ifndef CEF_2623
#define CEF_V8_SUPPORT_TYPED_ARRAYS

class CAscCefV8ArrayBufferReleaseCallback : public CefV8ArrayBufferReleaseCallback
{
public:
	virtual void ReleaseBuffer(void* buffer) OVERRIDE
	{
#if 0
		FILE* f = fopen("release_log.txt", "a+");
		fprintf(f, "release\n");
		fclose(f);
#endif
		delete [] ((BYTE*)buffer);
	}

	IMPLEMENT_REFCOUNTING(CAscCefV8ArrayBufferReleaseCallback);
};
#endif

namespace NSCommon
{
	template <typename Type>
	class smart_ptr
	{
	protected:
		Type*			m_pData;
		mutable LONG*	m_pCountRef;

	public:
		smart_ptr()
		{
			m_pData		= NULL;
			m_pCountRef = NULL;
		}
		smart_ptr(Type* pPointer)
		{
			m_pData		= pPointer;
			m_pCountRef = new LONG(1);
		}
		smart_ptr(const smart_ptr<Type>& pPointer)
		{
			m_pData		= NULL;
			m_pCountRef	= NULL;
			*this = pPointer;
		}
		~smart_ptr()
		{
			Release();
		}

		inline void Release()
		{
			if (!IsInit() || (NULL == m_pCountRef))
				return;

			*m_pCountRef -= 1;
			if (0 >= *m_pCountRef)
			{
				delete m_pData;
				delete m_pCountRef;
			}
			m_pData		= NULL;
			m_pCountRef	= NULL;
		}
		inline void AddRef()
		{
			if (!IsInit() || (NULL == m_pCountRef))
				return;
			*m_pCountRef += 1;
		}

	public:
		smart_ptr<Type>& operator=(const Type& oSrc)
		{
			Release();

			m_pData		= new Type(oSrc);
			m_pCountRef = new LONG(1);

			return *this;
		}
		smart_ptr<Type>& operator=(Type* pType)
		{
			Release();

			m_pData		= pType;
			m_pCountRef = new LONG(1);

			return *this;
		}
		smart_ptr<Type>& operator=(const smart_ptr<Type>& oSrc)
		{
			Release();

			if ((NULL == oSrc.m_pData) || (NULL == oSrc.m_pCountRef))
				return *this;

			*oSrc.m_pCountRef += 1;
			Attach(oSrc.m_pData, oSrc.m_pCountRef);
			return *this;
		}

	public:

		inline bool IsInit() const
		{
			return (NULL != m_pData);
		}
		inline bool is_init() const
		{
			return IsInit();
		}

		template<class T> inline const bool is()const
		{
			if (!IsInit())
				return false;
			T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
			return (NULL != pResult);
		}
		template<class T> inline const T& as()const
		{
			T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
			return *pResult;
		}
		template<class T> inline T& as()
		{
			T* pResult = dynamic_cast<T*>(const_cast<Type*>(m_pData));
			return *pResult;
		}

		template <typename T>
		inline void Attach(T* pCast, const LONG* pCountRef)
		{
			m_pData		= pCast;
			m_pCountRef	= const_cast<LONG*>(pCountRef);
		}

		template<typename T>
		inline smart_ptr<T> smart_dynamic_cast()const
		{
			smart_ptr<T> new_type;

			if ((NULL == m_pData) || (NULL == m_pCountRef))
				return new_type;

			T* pCast = dynamic_cast<T*>(m_pData);

			if (NULL == pCast)
				return new_type;

			*m_pCountRef += 1;

			new_type.Attach(pCast, m_pCountRef);

			return new_type;
		}

		inline Type& operator*()  { return *m_pData; }
		inline Type* operator->() { return  m_pData; }

		inline const Type& operator*()  const { return *m_pData; }
		inline const Type* operator->() const { return  m_pData; }

		inline const Type& get() { return  *m_pData; } const

		inline void reset(Type* pPointer = NULL)
		{
			*this = pPointer;
		}
	};

	int GetEncryptedVersion() { return 2; }

	int CheckEncryptedVersion(const std::string& input, int& offset)
	{
		offset = 0;
		// VER%NUMBER%; version from 100 to 999
		if (input.length() > 7)
		{
			const char* input_ptr = input.c_str();
			if (input_ptr[0] == 'V' && input_ptr[1] == 'E' && input_ptr[2] == 'R')
			{
				input_ptr += 3;
				int nVersion = 0;
				for (int i = 0; i < 4; ++i)
				{
					if (*input_ptr == ';')
					{
						offset = 3 + i + 1;
						return nVersion;
					}

					nVersion *= 10;
					nVersion += (*input_ptr - '0');
					++input_ptr;
				}
				return nVersion;
			}
		}
		return 1;
	}
	int CheckEncryptedVersion(const std::wstring& input, int& offset)
	{
		offset = 0;
		// VER%NUMBER%; version from 100 to 999
		if (input.length() > 7)
		{
			const wchar_t* input_ptr = input.c_str();
			if (input_ptr[0] == 'V' && input_ptr[1] == 'E' && input_ptr[2] == 'R')
			{
				input_ptr += 3;
				int nVersion = 0;
				for (int i = 0; i < 4; ++i)
				{
					if (*input_ptr == ';')
					{
						offset = 3 + i + 1;
						return nVersion;
					}

					nVersion *= 10;
					nVersion += (*input_ptr - '0');
					++input_ptr;
				}
				return nVersion;
			}
		}
		return 1;
	}

	std::string ConvertToCharSimple(const std::wstring& sValue)
	{
		std::string sRes;
		sRes.reserve(sValue.length());
		for (std::wstring::size_type pos = 0, len = sValue.length(); pos < len; ++pos)
			sRes.append(1, (char)sValue[pos]);
		return sRes;
	}

	std::string GetFilePathBase64(const std::wstring& sPath)
	{
		std::string sPathA = U_TO_UTF8(sPath);

		char* pDataDst = NULL;
		int nDataDst = 0;
		NSFile::CBase64Converter::Encode((BYTE*)sPathA.c_str(), (int)sPathA.length(), pDataDst, nDataDst, NSBase64::B64_BASE64_FLAG_NOCRLF);

		std::string sResult = "binary_content://" + std::string(pDataDst, (size_t)nDataDst);
		RELEASEARRAYOBJECTS(pDataDst);
		return sResult;
	}

	std::wstring GetFilePathFromBase64(const std::string& sPath)
	{
		int nPrefixLen = 17; // binary_content://

		BYTE* pDataDst = NULL;
		int nLenDst = 0;
		NSFile::CBase64Converter::Decode(sPath.c_str() + nPrefixLen, (int)sPath.length() - nPrefixLen, pDataDst, nLenDst);

		std::string sResultA((char*)pDataDst, (size_t)nLenDst);
		RELEASEARRAYOBJECTS(pDataDst);

		return UTF8_TO_U(sResultA);
	}
}

NSCommon::smart_ptr<NSSystem::CLocalFilesResolver> g_pLocalResolver = NULL;

namespace asc_client_renderer
{

inline void ExportMethod(const std::string& name, CefRefPtr<CefV8Value>& obj, CefRefPtr<CefV8Handler>& handler)
{
	CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(name, handler);
	obj->SetValue(name, func, V8_PROPERTY_ATTRIBUTE_NONE);
}

void ExtendObject(CefRefPtr<CefV8Value>& obj, CefRefPtr<CefV8Handler>& handler, const char** methods)
{
	const char** cur = methods;
	while (*cur != NULL)
	{
		CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(*cur, handler);
		obj->SetValue(*cur, func, V8_PROPERTY_ATTRIBUTE_NONE);
		++cur;
	}
}

std::string GetFileBase64(const std::wstring& sFile, int* outSize = NULL)
{
	if (outSize)
		*outSize = 0;

	NSFile::CFileBinary oFile;
	if (!oFile.OpenFile(sFile))
		return "";

	int nSize = (int)oFile.GetFileSize();
	DWORD dwSize = 0;
	BYTE* pData = new BYTE[nSize];
	oFile.ReadFile(pData, (DWORD)nSize, dwSize);
	oFile.CloseFile();

	char* pDataDst = NULL;
	int nDataDst = 0;
	NSFile::CBase64Converter::Encode(pData, nSize, pDataDst, nDataDst, NSBase64::B64_BASE64_FLAG_NOCRLF);

	std::string sBase64(pDataDst, nDataDst);
	RELEASEARRAYOBJECTS(pDataDst);

	if (outSize)
		*outSize = nSize;

	return sBase64;
}

class CLocalFileConvertV8Handler : public CefV8Handler
{
private:
	std::wstring m_sDirectory;

public:
	CLocalFileConvertV8Handler(const std::wstring& sDir = L"")
	{
		m_sDirectory = sDir;
	}

	void Close()
	{
		if (!m_sDirectory.empty())
			NSDirectory::DeleteDirectory(m_sDirectory);
	}

	~CLocalFileConvertV8Handler()
	{
		Close();
	}

	virtual bool Execute(const CefString& sMessageName,
					   CefRefPtr<CefV8Value> object,
					   const CefV8ValueList& arguments,
					   CefRefPtr<CefV8Value>& retval,
					   CefString& exception) OVERRIDE
	{
		std::string name = sMessageName.ToString();

		if (name == "close")
		{
			Close();
			return true;
		}
		else if (name == "get")
		{
			std::wstring sPath = L"";
			if (!arguments.empty())
				sPath = arguments[0]->GetStringValue().ToWString();
			else
				sPath = L"Editor.bin";

			std::wstring sFullPath = m_sDirectory + L"/" + sPath;

//#ifndef CEF_V8_SUPPORT_TYPED_ARRAYS
//			retval = CefV8Value::CreateUndefined();
//#else
			if (g_pLocalResolver->CheckNoFont(sFullPath))
			{
				BYTE* pData = NULL;
				DWORD dwFileLen = 0;
				NSFile::CFileBinary::ReadAllBytes(sFullPath, &pData, dwFileLen);

				if (0 != dwFileLen)
				{
#ifndef CEF_V8_SUPPORT_TYPED_ARRAYS
					retval = CefV8Value::CreateArray(dwFileLen);
					for (DWORD i = 0; i < dwFileLen; ++i)
						retval->SetValue(i, CefV8Value::CreateInt(pData[i]));
#else
					retval = CefV8Value::CreateArrayBuffer((void*)pData, (size_t)dwFileLen, new CAscCefV8ArrayBufferReleaseCallback());
#endif
				}
				else
					retval = CefV8Value::CreateUndefined();
			}
			else
			{
				retval = CefV8Value::CreateUndefined();
			}
//#endif
			return true;
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(CLocalFileConvertV8Handler);
};

class CAscEditorNativeV8Handler : public CefV8Handler, public INativeViewer_Events
{
	class CSavedPageInfo
	{
	public:
		std::wstring Url;
		int Page;
		int W;
		int H;

	public:
		CSavedPageInfo()
		{
			Page = -1;
			W = 0;
			H = 0;
		}
	};
	class CSavedPageTextInfo
	{
	public:
		int Page;
		std::string Info;
		int Paragraphs;
		int Words;
		int Spaces;
		int Symbols;

	public:
		CSavedPageTextInfo(const int& page, const int& paragraphs, const int& words, const int& symbols, const int& spaces, const std::string& sBase64Data)
		{
			Page = page;
			Info = sBase64Data;
			Paragraphs = paragraphs;
			Words = words;
			Spaces = spaces;
			Symbols = symbols;
		}
	};

public:
	AscEditorType m_etType;
	int           m_nEditorId;
	bool*         sync_command_check;

	// версия редактора
	std::string m_sVersion;

	// пути к папкам юзера/редактора
	std::wstring m_sAppData;
	std::wstring m_sFontsData;
	std::wstring m_sSystemPlugins;
	std::wstring m_sUserPlugins;
	std::wstring m_sCryptDocumentFolder; // recover
	std::wstring m_sRecoversFolder; // recover

	std::wstring m_sCookiesPath;

	std::wstring m_sLocalFileFolderWithoutFile;
	std::wstring m_sLocalFileFolder;
	std::wstring m_sLocalFileChanges;
	std::wstring m_sLocalFileSrc;
	bool m_bLocalIsSaved;
	int m_nCurrentChangesIndex; // текущий индекс изменения, чтобы не парсить файл. а понять, надо ли удалять или нет быстро.
	int m_nOpenChangesIndex; // количество изменений, при открытии

	// печать
	int m_nCurrentPrintIndex;
	bool m_bIsPrinting;

	// мап картинок по именам файлов
	int m_nLocalImagesNextIndex;
	std::map<std::wstring, std::wstring> m_mapLocalAddImages;

	// мап картинок по чексумме
	CCalculatorCRC32 m_oCRC32;
	std::map<unsigned int, std::wstring> m_mapLocalAddImagesCRC;

	NSFonts::IApplicationFonts* m_pLocalApplicationFonts;

	// файлы в дроп
	std::vector<std::wstring> m_arDropFiles;

	// открытие pdf, djvu, xps
	CNativeViewer m_oNativeViewer;
	int m_nNativeOpenFileTimerID;
	int m_bIsNativeViewerMode;
	std::list<CSavedPageInfo> m_arCompleteTasks;
	std::list<CSavedPageTextInfo> m_arCompleteTextTasks;

	bool m_bIsSupportSigs;
	bool m_bIsSupportOnlyPass;
	bool m_bIsSupportProtect;
	int m_nCryptoMode;

	bool m_bIsDebugMode;

	bool m_bIsEnableUploadCrypto;

	NSCriticalSection::CRITICAL_SECTION m_oCompleteTasksCS;

	// AES key
	BYTE* m_pAES_Key;

	int m_nIsCryptoModeProperty;

	std::string m_sEditorsCloudFeatures;
	bool m_bEditorsCloudFeaturesCheck;
	std::vector<std::string> m_arCloudFeaturesBlackList;

	std::wstring m_sAppTmpFolder;

	std::wstring m_sRendererProcessVariable;

	// для преобразования внутренняя ссылка => внешняя при скачивании
	std::wstring m_sEditorPageDomain;
	std::wstring m_sInternalEditorPageDomain;

	// для печати облачных файлов (pdf/xps/djvu)
	std::wstring m_sCloudNativePrintFile;

	bool m_bIsMacrosesSupport;
	bool m_bIsPluginsSupport;

	CAscEditorNativeV8Handler(const std::wstring& sUrl)
	{
		m_etType = AscEditorType::etUndefined;
		m_nEditorId = -1;
		sync_command_check = NULL;

		m_nCurrentPrintIndex = 0;

		m_nCurrentChangesIndex = 0;
		m_nOpenChangesIndex = 0;

		m_bLocalIsSaved = false;

		m_nLocalImagesNextIndex = 0;
		m_pLocalApplicationFonts = NULL;

		m_nNativeOpenFileTimerID = -1;
		m_bIsNativeViewerMode = false;

		m_bIsPrinting = false;

		m_bIsSupportSigs = true;
		m_bIsDebugMode = false;
		m_bIsSupportOnlyPass = true;
		m_bIsSupportProtect = true;

		m_nCryptoMode = 0;

		m_bIsEnableUploadCrypto = false;
		m_oCompleteTasksCS.InitializeCriticalSection();

		m_sRendererProcessVariable = L"";

		m_bIsMacrosesSupport = true;
		m_bIsPluginsSupport = true;

		CheckDefaults();

		if (!g_pLocalResolver.is_init())
		{
			g_pLocalResolver = new NSSystem::CLocalFilesResolver();
			g_pLocalResolver->CheckUrl(sUrl);
			g_pLocalResolver->Init(m_sFontsData, m_sRecoversFolder);
		}

		m_pAES_Key = NULL;
		m_nIsCryptoModeProperty = 0;

		m_bEditorsCloudFeaturesCheck = false;
		m_arCloudFeaturesBlackList.push_back("personal.onlyoffice.com");
	}

	void CheckDefaults()
	{
		CAscRendererProcessParams& default_params = CAscRendererProcessParams::getInstance();

		int nFlags = default_params.GetValueInt("onlypass", 3);

		m_bIsSupportOnlyPass = ((nFlags & 0x01) == 0x01) ? true : false;
		m_bIsSupportProtect = ((nFlags & 0x02) == 0x02) ? true : false;

		m_nCryptoMode = default_params.GetValueInt("cryptomode", m_nCryptoMode);

		m_sSystemPlugins = default_params.GetValueW("system_plugins_path");
		m_sUserPlugins = default_params.GetValueW("user_plugins_path");
		m_sCookiesPath = default_params.GetValueW("cookie_path");

		m_bIsDebugMode = (default_params.GetValue("debug_mode") == "true") ? true : false;

		m_sFontsData = default_params.GetValueW("fonts_cache_path");
		m_sAppData = default_params.GetValueW("app_data_path");

		m_sAppTmpFolder = default_params.GetValueW("tmp_folder");

		if (!m_sAppTmpFolder.empty())
			NSFile::CFileBinary::SetTempPath(m_sAppTmpFolder);

		m_sRecoversFolder = default_params.GetValueW("recovers_folder");
		m_sRendererProcessVariable = default_params.GetValueW("renderer_process_variable");

		if ("false" == default_params.GetValue("macroses_support"))
			m_bIsMacrosesSupport = false;
		if ("false" == default_params.GetValue("plugins_support"))
			m_bIsPluginsSupport = false;

#if 0
		default_params.Print();
#endif
	}

	virtual ~CAscEditorNativeV8Handler()
	{
		if (m_pAES_Key)
			NSOpenSSL::openssl_free(m_pAES_Key);
		NSBase::Release(m_pLocalApplicationFonts);
		m_oCompleteTasksCS.DeleteCriticalSection();
	}

	bool IsPropertyCryptoMode()
	{
		if (m_nCryptoMode > 0)
			return true;
		if (!CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId").empty())
			return true;
		return false;
	}

	virtual void OnDocumentOpened(const std::string& sBase64)
	{
	}

	virtual void OnPageSaved(const std::wstring& sUrl, const int& nPageNum, const int& nPageW, const int& nPageH)
	{
		CTemporaryCS oCS(&m_oCompleteTasksCS);
		CSavedPageInfo oInfo;
		oInfo.Url = sUrl;
		oInfo.Page = nPageNum;
		oInfo.W = nPageW;
		oInfo.H = nPageH;
		m_arCompleteTasks.push_back(oInfo);
	}
	virtual void OnPageText(const int& page, const int& paragraphs, const int& words, const int& symbols, const int& spaces, const std::string& sBase64Data)
	{
		CTemporaryCS oCS(&m_oCompleteTasksCS);
		CSavedPageTextInfo oInfo(page, paragraphs, words, symbols, spaces, sBase64Data);
		m_arCompleteTextTasks.push_back(oInfo);
	}

	inline void CallInEditorFrame(CefRefPtr<CefBrowser>& browser, const std::string& sCode)
	{
		CefRefPtr<CefFrame> frame = browser->GetFrame("frameEditor");
		if (!frame)
			frame = browser->GetMainFrame();

		frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
	}

	bool IsLocalFile(bool isUrlCheck)
	{
		if (isUrlCheck && m_sLocalFileFolderWithoutFile.empty())
		{
			std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
			if (0 == sUrl.find("file:/"))
				return true;
		}
		return m_sLocalFileFolderWithoutFile.empty() ? false : true;
	}

	bool CheckSW()
	{
		// функция, которая работает при включенном service worker
		// не подменился require.js и так далее
		if (!m_sVersion.empty() || m_etType != AscEditorType::etUndefined)
			return false;

		CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

		if (0 == browser->GetMainFrame()->GetURL().ToString().find("file://"))
			return false;

		if (true)
		{
			CefRefPtr<CefV8Value> retval;
			CefRefPtr<CefV8Exception> exception;

			bool bIsVersion = browser->GetMainFrame()->GetV8Context()->Eval(
"(function() { \n\
//console.log(\"!!! service worker !!!\");\n\
if (window.DocsAPI && window.DocsAPI.DocEditor) \n\
return window.DocsAPI.DocEditor.version(); \n\
else \n\
return undefined; \n\
})();",
#ifndef CEF_2623
		"", 0,
#endif
retval, exception);

			if (bIsVersion && retval->IsString())
				m_sVersion = retval->GetStringValue().ToString();

			if (m_sVersion.empty())
				m_sVersion = "undefined";

			CefRefPtr<CefProcessMessage> messageVersion = CefProcessMessage::Create("set_editors_version");
			messageVersion->GetArgumentList()->SetString(0, m_sVersion);
			SEND_MESSAGE_TO_BROWSER_PROCESS(messageVersion);
		}

		std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
		if (m_sVersion == "undefined" && sUrl.find("index.reporter.html") != std::string::npos)
		{
			m_etType = AscEditorType::etPresentation;
		}
		else
		{
			// сначала определим тип редактора
			if (sUrl.find("documenteditor") != std::wstring::npos)
				m_etType = AscEditorType::etDocument;
			else if (sUrl.find("presentationeditor") != std::wstring::npos)
				m_etType = AscEditorType::etPresentation;
			else if (sUrl.find("spreadsheeteditor") != std::wstring::npos)
				m_etType = AscEditorType::etSpreadsheet;
			else if (sUrl.find("pdfeditor") != std::wstring::npos)
				m_etType = AscEditorType::etPdf;
		}

		CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
		message->GetArgumentList()->SetInt(0, (int)m_etType);
		SEND_MESSAGE_TO_BROWSER_PROCESS(message);
		return true;
	}

	virtual bool Execute(const CefString& sMessageName,
					   CefRefPtr<CefV8Value> object,
					   const CefV8ValueList& arguments,
					   CefRefPtr<CefV8Value>& retval,
					   CefString& exception) OVERRIDE
	{
		std::string name = sMessageName.ToString();
		if (name == "Copy")
		{
			CefV8Context::GetCurrentContext()->GetFrame()->Copy();
			return  true;
		}
		else if (name == "Paste")
		{
			CefV8Context::GetCurrentContext()->GetFrame()->Paste();
			return  true;
		}
		else if (name == "Cut")
		{
			CefV8Context::GetCurrentContext()->GetFrame()->Cut();
			return  true;
		}
		else if (name == "SetEditorId")
		{
			CefRefPtr<CefV8Value> val = *arguments.begin();
			if (val->IsValid() && val->IsInt())
			{
				m_nEditorId = val->GetIntValue();
			}
			return true;
		}
		else if (name == "GetEditorId")
		{
			retval = CefV8Value::CreateInt(m_nEditorId);
			return true;
		}
		else if (name == "LoadJS")
		{
			bool bIsLocal = false;
			CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

			if (0 == browser->GetMainFrame()->GetURL().ToString().find("file://"))
				bIsLocal = true;

			if (m_sVersion.empty())
			{
				CefRefPtr<CefV8Value> retval;
				CefRefPtr<CefV8Exception> exception;

				bool bIsVersion = browser->GetMainFrame()->GetV8Context()->Eval(
"(function() { \n\
if (window.DocsAPI && window.DocsAPI.DocEditor) \n\
	return window.DocsAPI.DocEditor.version(); \n\
else \n\
	return undefined; \n\
})();",
#ifndef CEF_2623
			"", 0,
#endif
retval, exception);

				if (bIsVersion && retval->IsString())
					m_sVersion = retval->GetStringValue().ToString();

				if (m_sVersion.empty())
					m_sVersion = "undefined";

				CefRefPtr<CefProcessMessage> messageVersion = CefProcessMessage::Create("set_editors_version");
				messageVersion->GetArgumentList()->SetString(0, m_sVersion);
				SEND_MESSAGE_TO_BROWSER_PROCESS(messageVersion);
			}

			CefRefPtr<CefV8Value> val = *arguments.begin();
			if (!val->IsValid() || !val->IsString())
			{
				retval = CefV8Value::CreateInt(0);
				return true;
			}

			CefString scriptUrl = val->GetStringValue();
			std::wstring strUrl = scriptUrl.ToWString();

#ifdef NO_CACHE_WEB_CLOUD_SCRIPTS
			std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
			if (m_sVersion == "undefined" || m_sVersion == "reporter_cloud" || bIsLocal)
			{
				if ((m_sVersion == "undefined") && (sUrl.find("index.reporter.html") != std::string::npos))
				{
					m_sVersion = "reporter_cloud";
					if (m_etType != AscEditorType::etPresentation)
					{
						m_etType = AscEditorType::etPresentation;
						CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
						message->GetArgumentList()->SetInt(0, (int)m_etType);
						SEND_MESSAGE_TO_BROWSER_PROCESS(message);
					}
				}
			}
			if (strUrl.find(L"app.js") != std::wstring::npos)
			{
				// сначала определим тип редактора
				if (sUrl.find("documenteditor") != std::wstring::npos)
					m_etType = AscEditorType::etDocument;
				else if (sUrl.find("presentationeditor") != std::wstring::npos)
					m_etType = AscEditorType::etPresentation;
				else if (sUrl.find("spreadsheeteditor") != std::wstring::npos)
					m_etType = AscEditorType::etSpreadsheet;
				else if (sUrl.find("pdfeditor") != std::wstring::npos)
					m_etType = AscEditorType::etPdf;

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
				message->GetArgumentList()->SetInt(0, (int)m_etType);
				SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			}
			retval = CefV8Value::CreateInt(0);
			return true;
#else
			// 0 - грузить из облака
			// 1 - загружен и исполнен
			// 2 - ждать ответа
			int nResult = 0;
			std::wstring strPath = L"";

			if (strUrl.find(L"sdk/Common/AllFonts.js") != std::wstring::npos ||
				strUrl.find(L"sdkjs/common/AllFonts.js") != std::wstring::npos)
			{
				strPath = m_sFontsData + L"/AllFonts.js";
				nResult = 2;
			}
			else if (m_sVersion == "undefined" || m_sVersion == "reporter_cloud" || m_bIsDebugMode || bIsLocal)
			{
				std::string sUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
				if ((m_sVersion == "undefined") && (sUrl.find("index.reporter.html") != std::string::npos))
				{
					m_sVersion = "reporter_cloud";
					if (m_etType != Presentation)
					{
						m_etType = Presentation;
						CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
						CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
						message->GetArgumentList()->SetInt(0, (int)m_etType);
						browser->SendProcessMessage(PID_BROWSER, message);
					}
				}
				else if (strUrl.find(L"app.js") != std::wstring::npos)
				{
					// сначала определим тип редактора
					if (sUrl.find("documenteditor") != std::wstring::npos)
						m_etType = Document;
					else if (sUrl.find("presentationeditor") != std::wstring::npos)
						m_etType = Presentation;
					else if (sUrl.find("spreadsheeteditor") != std::wstring::npos)
						m_etType = Spreadsheet;

					CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
					CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
					message->GetArgumentList()->SetInt(0, (int)m_etType);
					browser->SendProcessMessage(PID_BROWSER, message);
				}
				retval = CefV8Value::CreateInt(0);
				return true;
			}

			if (0 == nResult)
			{
				std::wstring strAppPath = m_sAppData + L"/webdata/cloud";
				std::wstring strAppPathEditors = strAppPath + L"/" + NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)m_sVersion.c_str(), (LONG)m_sVersion.length());
				if (!NSDirectory::Exists(strAppPathEditors))
				{
					NSDirectory::CreateDirectory(strAppPathEditors);
					NSDirectory::CreateDirectory(strAppPathEditors + L"/word");
					NSDirectory::CreateDirectory(strAppPathEditors + L"/cell");
					NSDirectory::CreateDirectory(strAppPathEditors + L"/slide");
				}

				if (strUrl.find(L"sdk-all.js") != std::wstring::npos)
				{
					if (m_etType == Document)
						strPath = strAppPathEditors + L"/word/sdk-all.js";
					else if (m_etType == Presentation)
						strPath = strAppPathEditors + L"/slide/sdk-all.js";
					else if (m_etType == Spreadsheet)
						strPath = strAppPathEditors + L"/cell/sdk-all.js";

					nResult = 2;
				}
				else if (strUrl.find(L"sdk-all-min.js") != std::wstring::npos)
				{
					if (m_etType == Document)
						strPath = strAppPathEditors + L"/word/sdk-all-min.js";
					else if (m_etType == Presentation)
						strPath = strAppPathEditors + L"/slide/sdk-all-min.js";
					else if (m_etType == Spreadsheet)
						strPath = strAppPathEditors + L"/cell/sdk-all-min.js";

					nResult = 2;
				}
				else if (strUrl.find(L"app.js") != std::wstring::npos)
				{
					std::wstring sStringUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToWString();

					// сначала определим тип редактора
					if (sStringUrl.find(L"documenteditor") != std::wstring::npos)
						m_etType = Document;
					else if (sStringUrl.find(L"presentationeditor") != std::wstring::npos)
						m_etType = Presentation;
					else if (sStringUrl.find(L"spreadsheeteditor") != std::wstring::npos)
						m_etType = Spreadsheet;

					CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
					CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("EditorType");
					message->GetArgumentList()->SetInt(0, (int)m_etType);
					browser->SendProcessMessage(PID_BROWSER, message);

					if (m_etType == Document)
						strPath = strAppPathEditors + L"/word/app.js";
					else if (m_etType == Presentation)
						strPath = strAppPathEditors + L"/slide/app.js";
					else if (m_etType == Spreadsheet)
						strPath = strAppPathEditors + L"/cell/app.js";

					nResult = 2;
				}
			}

			if (strPath != L"" && nResult != 0)
			{
				if (nResult == 1)
				{
					NSFile::CFileBinary oFile;
					if (oFile.OpenFile(strPath))
					{
						int nSize = (int)oFile.GetFileSize();
						BYTE* scriptData = new BYTE[nSize];
						DWORD dwReadSize = 0;
						oFile.ReadFile(scriptData, (DWORD)nSize, dwReadSize);

						std::string strUTF8((char*)scriptData, (LONG)nSize);

						delete [] scriptData;

						CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(strUTF8, "", 0);
						retval = CefV8Value::CreateInt(1);
					}
					else
					{
						retval = CefV8Value::CreateInt(0);
					}
				}
				else
				{
					CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
					CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("load_js");
					int64 frameId = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
					message->GetArgumentList()->SetString(0, GetFullUrl2(strUrl, CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToWString()));
					message->GetArgumentList()->SetString(1, strPath);
					NSArgumentList::SetInt64(message->GetArgumentList(), 2, frameId);
					browser->SendProcessMessage(PID_BROWSER, message);

					retval = CefV8Value::CreateInt(2);
				}
			}
			else
			{
				retval = CefV8Value::CreateInt(0);
			}
#endif
			return true;
		}
		else if (name == "LoadFontBase64")
		{
			CefRefPtr<CefV8Value> val = *arguments.begin();
			CefString fileUrl = val->GetStringValue();
			std::wstring strUrl = fileUrl.ToWString();

			if (0 != strUrl.find(L"embedded"))
			{
				int nSize = 0;
				std::string sFontData = "";
				if (g_pLocalResolver->CheckFont(strUrl))
					sFontData = GetFileBase64(strUrl, &nSize);
				std::string sName = U_TO_UTF8(strUrl);
				sName = "window[\"" + sName + "\"] = \"" + std::to_string(nSize) + ";" + sFontData + "\";";
				CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(sName, "", 0);
			}
			else
			{
				std::wstring sFileFont = m_sLocalFileFolder + L"/fonts/" + strUrl + L".js";

				if (0 == sFileFont.find(L"file:///"))
				{
					sFileFont = sFileFont.substr(7);
					if (!NSFile::CFileBinary::Exists(sFileFont))
						sFileFont = sFileFont.substr(1);
				}

				std::string sFileCodeJS;
				NSFile::CFileBinary::ReadAllTextUtf8A(sFileFont, sFileCodeJS);

				CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(sFileCodeJS, "", 0);
			}

			return true;
		}
		else if (name == "getFontsSprite")
		{
			std::wstring sPath = L"";
			if (arguments.size() > 0)
			{
				CefRefPtr<CefV8Value> val = *arguments.begin();
				if (val->IsBool() && val->GetBoolValue())
					sPath = L"@2x";
				if (val->IsString())
					sPath = val->GetStringValue().ToWString();
			}

			if (std::wstring::npos == sPath.find(L"_ea"))
			{
				std::string sMainUrl = CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToString();
				std::string::size_type posParams = sMainUrl.find("?");
				if (posParams != std::string::npos)
				{
					std::string::size_type posLang = sMainUrl.find("lang=", posParams);
					if (posLang != std::string::npos)
					{
						posLang += 5;
						std::string::size_type posLang2 = sMainUrl.find("&", posLang);
						if (posLang2 == std::string::npos)
							posLang2 = sMainUrl.length();
						std::string sLang = sMainUrl.substr(posLang, posLang2 - posLang);

						std::string::size_type posSubLang = sLang.find("-");
						if (posSubLang != std::string::npos)
							sLang = sLang.substr(0, posSubLang);

						posSubLang = sLang.find("_");
						if (posSubLang != std::string::npos)
							sLang = sLang.substr(0, posSubLang);

						if ("zh" == sLang || "ja" == sLang || "ko" == sLang)
						{
							sPath = L"_ea" + sPath;
						}
					}
				}
			}

			std::wstring sUrl = L"ascdesktop://fonts/fonts_thumbnail" + sPath + L".png";
			retval = CefV8Value::CreateString(sUrl);
			return true;
		}
		else if (name == "SpellCheck")
		{
			int64 frameId = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("spell_check_task");
			message->GetArgumentList()->SetInt(0, (int)m_nEditorId);
			message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());
			NSArgumentList::SetInt64(message->GetArgumentList(), 2, frameId);
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_CreateEditorApi")
		{
			volatile bool* pChecker = this->sync_command_check;
			*pChecker = true;

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("create_editor_api");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			if (IsLocalFile(true))
			{
				CSdkjsAddons oAddonsChecker(m_sUserPlugins);
				oAddonsChecker.CheckLocal(CefV8Context::GetCurrentContext());
			}
			else
			{
				// ждем asc_onGetEditorPermissions

				// формат AllFonts.js изменен!!!
				// смотрим на версию и если она старая - берем нужную версию файла

				int nMajorVersion = NSVersion::GetMajorVersion(m_sVersion);
				std::wstring sAllFontsVersion = L"";
				if (nMajorVersion != 0 && nMajorVersion < 6)
					sAllFontsVersion = L"/AllFonts.js.1";
				else if (nMajorVersion != 0 && nMajorVersion < 7)
					sAllFontsVersion = L"/AllFonts.js.2";
				else if (CheckSW())
					sAllFontsVersion = L"/AllFonts.js";

				if (!sAllFontsVersion.empty())
				{
					// берем нужную версию
					std::string sCode;
					NSFile::CFileBinary::ReadAllTextUtf8A(m_sFontsData + sAllFontsVersion, sCode);
					if (!sCode.empty())
					{
						CefRefPtr<CefV8Value> retval;
						CefRefPtr<CefV8Exception> exception;
						CefV8Context::GetCurrentContext()->Eval(sCode,
						#ifndef CEF_2623
									"", 0,
						#endif
						retval, exception);
					}
				}
			}
			return true;
		}
		else if (name == "ConsoleLog")
		{
			LOGGER_STRING2(arguments[0]->GetStringValue().ToString());
			return true;
		}
		else if (name == "setCookie")
		{
			if (arguments.size() != 5)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_cookie");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(2, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(3, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(4, (*iter)->GetStringValue()); ++iter;
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			return true;
		}
		else if (name == "setAuth")
		{
			if (arguments.size() != 4)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_cookie");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(2, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(3, "asc_auth_key");
			message->GetArgumentList()->SetString(4, (*iter)->GetStringValue()); ++iter;
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "getCookiePresent")
		{
			if (arguments.size() != 2)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("is_cookie_present");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "getAuth")
		{
			if (arguments.size() != 1)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("is_cookie_present");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(1, "asc_auth_key");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			return true;
		}
		else if (name == "checkAuth")
		{
			if (arguments.size() != 1)
				return true;

			CefRefPtr<CefV8Value> _param = *arguments.begin();

			std::vector<CefString> arrKeys;
			_param->GetKeys(arrKeys);

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_check_auth");
			message->GetArgumentList()->SetInt(0, (int)arrKeys.size());

			int nCurrent = 1;
			for (std::vector<CefString>::iterator i = arrKeys.begin(); i != arrKeys.end(); i++)
				message->GetArgumentList()->SetString(nCurrent++, *i);

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "onDocumentModifiedChanged")
		{
			if (arguments.size() != 1)
				return true;

			if (IsChartEditor())
				return true;

			CefRefPtr<CefV8Value> val = *arguments.begin();
			bool bValue = val->GetBoolValue();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onDocumentModifiedChanged");
			message->GetArgumentList()->SetBool(0, bValue);
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SetDocumentName")
		{
			if (IsChartEditor())
				return true;

			CefRefPtr<CefV8Value> val = *arguments.begin();
			CefString sName = val->GetStringValue();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_document_name");
			message->GetArgumentList()->SetString(0, sName);
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			// HACK!!!
			if (IsPropertyCryptoMode())
			{
				CefRefPtr<CefFrame> _frame =  CefV8Context::GetCurrentContext()->GetFrame();
				_frame->ExecuteJavaScript("(function() { try { \
DE.controllers.Main.DisableMailMerge(); \
DE.controllers.Main.DisableVersionHistory(); \
} catch(err){} })();", _frame->GetURL(), 0);
			}

			return true;
		}
		else if (name == "OnSave")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_document_save");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "js_message")
		{
			if (arguments.size() != 2)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("js_message");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetString(1, (*iter)->GetStringValue()); ++iter;
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "Print_Start")
		{
			if (arguments.size() != 4)
			{
				m_bIsPrinting = true;
				return true;
			}

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_start");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetInt(1, (*iter)->GetIntValue()); ++iter;
			message->GetArgumentList()->SetString(2, CefV8Context::GetCurrentContext()->GetBrowser()->GetFocusedFrame()->GetURL());
			message->GetArgumentList()->SetString(3, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetInt(4, (*iter)->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			m_nCurrentPrintIndex = 0;
			m_bIsPrinting = true;
			return true;
		}
		else if (name == "Print_Page")
		{
			if (arguments.size() != 3)
				return true;

			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_page");
			message->GetArgumentList()->SetString(0, (*iter)->GetStringValue()); ++iter;
			message->GetArgumentList()->SetInt(1, m_nCurrentPrintIndex);
			message->GetArgumentList()->SetDouble(2, (*iter)->GetDoubleValue()); ++iter;
			message->GetArgumentList()->SetDouble(3, (*iter)->GetDoubleValue()); ++iter;
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			m_nCurrentPrintIndex++;
			return true;
		}
		else if (name == "Print_End")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print_end");

			if (arguments.size() > 0)
				message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			m_nCurrentPrintIndex = 0;
			m_bIsPrinting = false;
			return true;
		}
		else if (name == "Print")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("print");

			CefString params = "";
			CefString password = "";

			if (arguments.size() > 0)
				params = arguments[0]->GetStringValue();
			if (arguments.size() > 1)
				password = arguments[1]->GetStringValue();

			message->GetArgumentList()->SetString(0, params);
			message->GetArgumentList()->SetString(1, password);

			if (!m_sCloudNativePrintFile.empty())
				message->GetArgumentList()->SetString(2, m_sCloudNativePrintFile);

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "IsSupportNativePrint")
		{
			if (m_sLocalFileFolder.empty())
				retval = CefV8Value::CreateBool(false);
			else
				retval = CefV8Value::CreateBool(true);
			return true;
		}
		else if (name == "CheckNeedWheel")
		{
			// это код, когда под виндоус приходило два раза. Теперь поправили
			// для совместимости метод оставляем
			retval = CefV8Value::CreateBool(true);
			return true;
		}
		else if (name == "GetImageBase64")
		{
			if (arguments.size() < 1)
			{
				retval = CefV8Value::CreateString("");
				return true;
			}

			std::wstring sFileUrl = arguments[0]->GetStringValue().ToWString();

			if (sFileUrl.find(L"file://") == 0)
			{
				if (NSFile::CFileBinary::Exists(sFileUrl.substr(7)))
					sFileUrl = sFileUrl.substr(7);
				else if (NSFile::CFileBinary::Exists(sFileUrl.substr(8)))
					sFileUrl = sFileUrl.substr(8);
			}

			std::string sHeader = "";

			int nSize = 0;
			std::string sImageData = "";

			if (g_pLocalResolver->Check(sFileUrl))
				sImageData = GetFileBase64(sFileUrl, &nSize);

			#define IMAGE_CHECKER_SIZE 50
			if (IMAGE_CHECKER_SIZE > nSize)
			{
				retval = CefV8Value::CreateString("");
				return true;
			}

			bool bIsNoHeader = false;
			if (arguments.size() >= 2)
				bIsNoHeader = arguments[1]->GetBoolValue();

			if (!bIsNoHeader)
			{
				BYTE pData[IMAGE_CHECKER_SIZE];
				memset(pData, 0, IMAGE_CHECKER_SIZE);
				DWORD dwSize = 0;
				NSFile::CFileBinary oFile;
				oFile.OpenFile(sFileUrl);
				oFile.ReadFile(pData, IMAGE_CHECKER_SIZE, dwSize);
				oFile.CloseFile();

				CImageFileFormatChecker _checker;

				if (_checker.isBmpFile(pData, IMAGE_CHECKER_SIZE))
					sHeader = "data:image/bmp;base64,";
				else if (_checker.isJpgFile(pData, IMAGE_CHECKER_SIZE))
					sHeader = "data:image/jpeg;base64,";
				else if (_checker.isPngFile(pData, IMAGE_CHECKER_SIZE))
					sHeader = "data:image/png;base64,";
				else if (_checker.isGifFile(pData, IMAGE_CHECKER_SIZE))
					sHeader = "data:image/gif;base64,";
				else if (_checker.isTiffFile(pData, IMAGE_CHECKER_SIZE))
					sHeader = "data:image/tiff;base64,";
			}

			if (sHeader.empty() && !bIsNoHeader)
			{
				retval = CefV8Value::CreateString("");
				return true;
			}

			if (!sHeader.empty())
				sImageData = sHeader + sImageData;

			retval = CefV8Value::CreateString(sImageData.c_str());
			return true;
		}
		else if (name == "SetFullscreen")
		{
			bool bIsFullScreen = false;
			if (arguments.size() > 0)
				bIsFullScreen = arguments[0]->GetBoolValue();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(bIsFullScreen ? "onfullscreenenter" : "onfullscreenleave");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalStartOpen")
		{
			// редактор загрузился и готов к файлу
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_loadstart");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileOpen")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_open");
			if (arguments.size() == 1)
				message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileCreate")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_create");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRecoverFolder")
		{
			m_sLocalFileFolderWithoutFile = arguments[0]->GetStringValue().ToWString();

			m_sLocalFileChanges = m_sLocalFileFolderWithoutFile + L"/changes/changes0.json";
			if (!NSDirectory::Exists(m_sLocalFileFolderWithoutFile + L"/changes"))
				NSDirectory::CreateDirectory(m_sLocalFileFolderWithoutFile + L"/changes");

			std::vector<std::wstring> arMedia = NSDirectory::GetFiles(m_sLocalFileFolderWithoutFile + L"/media");
			m_nLocalImagesNextIndex = (int)arMedia.size() + 1;

			if (0 == m_sLocalFileFolderWithoutFile.find('/'))
				m_sLocalFileFolder = L"file://" + m_sLocalFileFolderWithoutFile;
			else
				m_sLocalFileFolder = L"file:///" + m_sLocalFileFolderWithoutFile;

			return true;
		}
		else if (name == "CheckUserId")
		{
			std::wstring sUserPath = m_sLocalFileFolderWithoutFile + L"/user_name.log";
			int nUserIndex = 1;
			std::string sUserLog = "";

			if (NSFile::CFileBinary::ReadAllTextUtf8A(sUserPath, sUserLog))
			{
				NSFile::CFileBinary::Remove(sUserPath);
				nUserIndex = std::stoi(sUserLog);
				nUserIndex++;
			}

			std::wstring sUserLogW = std::to_wstring(nUserIndex);
			NSFile::CFileBinary::SaveToFile(sUserPath, sUserLogW);

			retval = CefV8Value::CreateString(sUserLogW);
			return true;
		}
		else if (name == "LocalFileRecents")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecents");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileOpenRecent")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_openrecents");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRemoveRecent")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removerecents");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRemoveAllRecents")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removeallrecents");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRecovers")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_sendrecovers");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileOpenRecover")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_openrecovers");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRemoveRecover")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removerecovers");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileRemoveAllRecovers")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_removeallrecovers");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileSaveChanges")
		{
			std::vector<CefRefPtr<CefV8Value> >::const_iterator iter = arguments.begin();

			CefRefPtr<CefV8Value> param = arguments[0];
			int nDeleteIndex = arguments[1]->GetIntValue();
			int nCount = arguments[2]->GetIntValue();

			if (nDeleteIndex < m_nCurrentChangesIndex)
			{
				// нужно удалить изменения
				RemoveChanges(nDeleteIndex);
			}
			m_nCurrentChangesIndex = nDeleteIndex + nCount;

			if (nCount != 0)
			{
				FILE* _file = NSFile::CFileBinary::OpenFileNative(m_sLocalFileChanges, L"a+");
				if (NULL != _file)
				{
					if (!param->IsArray())
					{
						fprintf(_file, "\"");
						fprintf(_file, param->GetStringValue().ToString().c_str());
						fprintf(_file, "\",");
					}
					else
					{
						for (int i = 0; i < nCount; ++i)
						{
							fprintf(_file, "\"");
							fprintf(_file, param->GetValue(i)->GetStringValue().ToString().c_str());
							fprintf(_file, "\",");
						}
					}
					fclose(_file);
				}
			}
			return true;
		}
		else if (name == "LocalFileGetOpenChangesCount")
		{
			retval = CefV8Value::CreateInt(m_nOpenChangesIndex);
			return true;
		}
		else if (name == "LocalFileSetOpenChangesCount")
		{
			m_nOpenChangesIndex = arguments[0]->GetIntValue();
			return true;
		}
		else if (name == "LocalFileGetCurrentChangesIndex")
		{
			retval = CefV8Value::CreateInt(m_nCurrentChangesIndex);
			return true;
		}
		else if (name == "LocalFileSave")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onsavestart");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, ((arguments.size() > 1) && arguments[1]->IsString()) ? arguments[1]->GetStringValue() : "");
			message->GetArgumentList()->SetString(2, ((arguments.size() > 2) && arguments[2]->IsString()) ? arguments[2]->GetStringValue() : "");
			message->GetArgumentList()->SetInt(3, (arguments.size() > 3) ? arguments[3]->GetIntValue() : 0);
			message->GetArgumentList()->SetString(4, ((arguments.size() > 4) && arguments[4]->IsString()) ? arguments[4]->GetStringValue() : "");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "LocalFileGetSourcePath")
		{
			retval = CefV8Value::CreateString(m_sLocalFileSrc);
			return true;
		}
		else if (name == "LocalFileSetSourcePath")
		{
			m_sLocalFileSrc = arguments[0]->GetStringValue().ToWString();
			return true;
		}
		else if (name == "LocalFileGetSaved")
		{
			retval = CefV8Value::CreateBool(m_bLocalIsSaved);
			return true;
		}
		else if (name == "LocalFileSetSaved")
		{
			// saved -> unsaved перейти не может
			if (!m_bLocalIsSaved)
				m_bLocalIsSaved = arguments[0]->GetBoolValue();
			return true;
		}
		else if (name == "LocalFileGetImageUrl")
		{
			std::wstring sUrl = GetLocalImageUrl(arguments[0]->GetStringValue().ToWString());
			retval = CefV8Value::CreateString(sUrl);
			return true;
		}
		else if (name == "LocalFileGetImageUrlFromOpenFileDialog")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("onlocaldocument_onaddimage");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "execCommand")
		{
			std::string sCommand = arguments[0]->GetStringValue().ToString();
			std::string sArg = "";
			if (2 == arguments.size())
				sArg = arguments[1]->GetStringValue().ToString();

			if ("portal:cryptoinfo" == sCommand)
			{
				CAscRendererProcessParams::getInstance().SetProperty("cryptoEngineId", CPluginsManager::GetStringValue(sArg, "cryptoEngineId"));
				CAscRendererProcessParams::getInstance().SetProperty("user", CPluginsManager::GetStringValue(sArg, "userId"));
				return true;
			}
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_exec_command");

			message->GetArgumentList()->SetString(0, sCommand);
			if (!sArg.empty())
				message->GetArgumentList()->SetString(1, sArg);

			if ("portal:login" == sCommand)
			{
				std::wstring sCloudCryptoGuid = CPluginsManager::GetStringValueW(sArg, "cryptoEngineId");

				if (!sCloudCryptoGuid.empty())
				{
					CCloudCryptoDesktop info;
					info.GUID = sCloudCryptoGuid;
					info.User = CPluginsManager::GetStringValueW(sArg, "userId");
					info.Portal = CPluginsManager::GetStringValueW(sArg, "domain");
					info.Email = CPluginsManager::GetStringValueW(sArg, "email");
					info.PublicKey = CPluginsManager::GetStringValueW(sArg, "publicKey");
					NSStringUtils::string_replace(info.PublicKey, L"&#xA", L"\n");
					info.PrivateKeyEnc = CPluginsManager::GetStringValueW(sArg, "privateKeyEnc");

					CAscRendererProcessParams::getInstance().SetProperty("cryptoEngineId", U_TO_UTF8(sCloudCryptoGuid));
					CAscRendererProcessParams::getInstance().SetProperty("user", U_TO_UTF8(info.User));

					CCloudCryptoApp oApp(m_sUserPlugins + L"/cloud_crypto.xml");
					CCloudCryptoTmpInfoApp oAppTmp(m_sUserPlugins + L"/cloud_crypto_tmp.xml");

					CCloudCryptoDesktop* savedInfo = oApp.GetInfo(info);
					CCloudCryptoTmpInfo* tmpInfo = oAppTmp.getInfo(/*info.Email*/L"", info.Portal);

					int nServerPrivateKeyVersion = 0;
					int nServerPrivateKeyVersionOffset = 0;
					bool bIsServerPrivateKeyExist = (info.PrivateKeyEnc.empty() && info.PublicKey.empty()) ? false : true;

					if (bIsServerPrivateKeyExist)
					{
						nServerPrivateKeyVersion = NSCommon::CheckEncryptedVersion(info.PrivateKeyEnc, nServerPrivateKeyVersionOffset);

						if (nServerPrivateKeyVersion > NSCommon::GetEncryptedVersion())
						{
							// нужно сказать, что нужно обновить десктоп
							CefRefPtr<CefProcessMessage> messageUpdateApp = CefProcessMessage::Create("on_need_update_app");
							SEND_MESSAGE_TO_BROWSER_PROCESS(messageUpdateApp);
						}
					}

					bool bIsNeedRelogin = false;
					if (NULL == savedInfo && tmpInfo)
					{
						// ничего не сохранено. значит это первый логин
						if (!bIsServerPrivateKeyExist)
						{
							// генерируем ключи!
							unsigned char* publicKey = NULL;
							unsigned char* privateKey = NULL;
							NSOpenSSL::RSA_GenerateKeys(publicKey, privateKey);

							std::string sPublic((char*)publicKey);
							std::string sPrivate((char*)privateKey);

							NSOpenSSL::openssl_free(publicKey);
							NSOpenSSL::openssl_free(privateKey);

							info.PublicKey = NSFile::CUtf8Converter::GetUnicodeFromCharPtr(sPublic);
							info.PrivateKey = NSFile::CUtf8Converter::GetUnicodeFromCharPtr(sPrivate);

							std::string privateEnc;
							NSOpenSSL::AES_Encrypt_desktop_GCM(U_TO_UTF8(tmpInfo->m_sPassword), sPrivate, privateEnc, CAscRendererProcessParams::getInstance().GetProperty("user"));
							info.PrivateKeyEnc = NSFile::CUtf8Converter::GetUnicodeFromCharPtr(privateEnc);

							oApp.AddInfo(info);

							// отсылаем ключи
							NSStringUtils::string_replaceA(sPublic, "\n", "&#xA");
							std::string sCode = ("setTimeout(function() { window.cloudCryptoCommand && window.cloudCryptoCommand(\"encryptionKeys\", { publicKey : \"" + sPublic + "\", privateKeyEnc : \"" + privateEnc + "\" }); }, 10);");
							CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
							_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
						}
						else
						{
							// декодируем ключ
							std::string privateKeyEnc = U_TO_UTF8(info.PrivateKeyEnc);
							std::string privateKey;

							if (nServerPrivateKeyVersion == 2)
								NSOpenSSL::AES_Decrypt_desktop_GCM(U_TO_UTF8(tmpInfo->m_sPassword), privateKeyEnc, privateKey, CAscRendererProcessParams::getInstance().GetProperty("user"), nServerPrivateKeyVersionOffset);
							else
								NSOpenSSL::AES_Decrypt_desktop(U_TO_UTF8(tmpInfo->m_sPassword), privateKeyEnc, privateKey, CAscRendererProcessParams::getInstance().GetProperty("user"));

							info.PrivateKey = NSFile::CUtf8Converter::GetUnicodeFromCharPtr(privateKey);
							oApp.AddInfo(info);

							// теперь проверим, нужно ли обновить ключ, ведь пароль у нас есть
							if (nServerPrivateKeyVersion < NSCommon::GetEncryptedVersion())
							{
								// обновим ключ в новом формате
								std::string privateEnc;
								NSOpenSSL::AES_Encrypt_desktop_GCM(U_TO_UTF8(tmpInfo->m_sPassword), privateKey, privateEnc, CAscRendererProcessParams::getInstance().GetProperty("user"));
								info.PrivateKeyEnc = NSFile::CUtf8Converter::GetUnicodeFromCharPtr(privateEnc);
								std::string publicKey = NSCommon::ConvertToCharSimple(info.PublicKey);

								std::string sCode = ("setTimeout(function() { window.cloudCryptoCommand && window.cloudCryptoCommand(\"updateEncryptionKeys\", { publicKey : \"" + publicKey + "\", privateKeyEnc : \"" + privateEnc + "\" }); }, 10);");
								CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
								_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
							}
						}
					}
					else if (!savedInfo)
					{
						bIsNeedRelogin = true;
					}

					if (bIsServerPrivateKeyExist && nServerPrivateKeyVersion < NSCommon::GetEncryptedVersion())
					{
						// нужно удалить локальную запись и перелогиниться
						// но пока так сделать не могу, так как порталы не готовы к updateEncryptionKeys

						// TODO:
						// oApp.RemoveInfo(info.User);
						// bIsNeedRelogin = true;
					}

					if (tmpInfo)
						oAppTmp.removeInfo(L""/*info.Email*/, info.Portal);

					if (bIsNeedRelogin)
					{
						// перелогиньтесь!!!
						std::string sCode = ("setTimeout(function() { window.cloudCryptoCommand && window.cloudCryptoCommand(\"relogin\"); }, 10);");
						CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetBrowser()->GetMainFrame();
						_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
					}
				}
			}
			else if ("portal:checkpwd" == sCommand)
			{
				std::string emailId = CPluginsManager::GetStringValue(sArg, "emailInput");
				std::string passwordId = CPluginsManager::GetStringValue(sArg, "pwdInput");

				std::wstring sEmail;
				std::wstring sPassword;

				CefRefPtr<CefV8Value> retval;
				CefRefPtr<CefV8Exception> exception;
				bool bValid = CefV8Context::GetCurrentContext()->Eval("(function() { return document.getElementById(\"" + emailId + "\").value; })();",
#ifndef CEF_2623
			"", 0,
#endif
retval, exception);

				if (bValid && retval && retval->IsString())
					sEmail = retval->GetStringValue().ToWString();

				bValid = CefV8Context::GetCurrentContext()->Eval("(function() { return document.getElementById(\"" + passwordId + "\").value; })();",
#ifndef CEF_2623
			"", 0,
#endif
retval, exception);

				if (bValid && retval && retval->IsString())
					sPassword = retval->GetStringValue().ToWString();

				CCloudCryptoTmpInfoApp oAppTmp(m_sUserPlugins + L"/cloud_crypto_tmp.xml");
				oAppTmp.addInfo(L"", /*sEmail*/L"", sPassword, CPluginsManager::GetStringValueW(sArg, "domain"));
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "Logout")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_logout");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SetDropFiles")
		{
			// разрешения на файлы даются выше
			CefRefPtr<CefV8Value> val = *arguments.begin();
			int nCount = val->GetArrayLength();

			m_arDropFiles.clear();
			for (int i = 0; i < nCount; ++i)
			{
				std::wstring sValue = val->GetValue(i)->GetStringValue().ToWString();
				m_arDropFiles.push_back(sValue);
			}
			return true;
		}
		else if (name == "ClearDropFiles")
		{
			m_arDropFiles.clear();
			return true;
		}
		else if (name == "IsImageFile")
		{
			CImageFileFormatChecker oChecker;
			std::wstring sImageFile = arguments[0]->GetStringValue().ToWString();
			bool bIsImageFile = oChecker.isImageFile(sImageFile);
			retval = CefV8Value::CreateBool(bIsImageFile);
			return true;
		}
		else if (name == "GetDropFiles")
		{
			int nCount = (int)m_arDropFiles.size();
			retval = CefV8Value::CreateArray(nCount);
			int nCurrent = 0;
			for (std::vector<std::wstring>::iterator i = m_arDropFiles.begin(); i != m_arDropFiles.end(); i++)
				retval->SetValue(nCurrent++, CefV8Value::CreateString(*i));

			// Отдали список, чистим. Иначе всегда будем вставлять картинки при добавлении текста
			m_arDropFiles.clear();

			return true;
		}
		else if (name == "DropOfficeFiles")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("DropOfficeFiles");

			int nCurrent = 0;
			for (std::vector<std::wstring>::iterator i = m_arDropFiles.begin(); i != m_arDropFiles.end(); i++)
				 message->GetArgumentList()->SetString(nCurrent++, *i);

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SetAdvancedOptions")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_setadvancedoptions");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "ApplyAction")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_core_check_info");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "InitJSContext")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_init_js_context");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
			if (_frame)
			{
				std::string sCodeInitJS = "\
window.AscDesktopEditor.CreateEditorApi = function(api) {\n\
api && api.asc_registerCallback('asc_onGetEditorPermissions', function(e) { window.AscDesktopEditor.CheckCloudFeatures(e.asc_getLicenseType()); });\n\
window.AscDesktopEditor._CreateEditorApi();\n\
};\n\
window.AscDesktopEditor.sendSystemMessage = function(arg) {\n\
  window.AscDesktopEditor.isSendSystemMessage = true;\n\
  // expand system message\n\
  arg.url = window.AscDesktopEditor._getMainUrl();\n\
  window.AscDesktopEditor._sendSystemMessage(JSON.stringify(arg));\n\
};\n\
window.AscDesktopEditor.GetHash = function(arg, callback) {\n\
  window.AscDesktopEditor.getHashCallback = callback;\n\
  window.AscDesktopEditor._GetHash(arg);\n\
};\n\
window.AscDesktopEditor.CallInAllWindows = function(arg) {\n\
  window.AscDesktopEditor._CallInAllWindows(\"(\" + arg.toString() + \")();\");\n\
};\n\
window.AscDesktopEditor.OpenFileCrypt = function(name, url, callback) {\n\
  window.AscDesktopEditor.openFileCryptCallback = callback;\n\
  window.AscDesktopEditor._OpenFileCrypt(name, url);\n\
};\n\
window.AscDesktopEditor.OpenFilenameDialog = function(filter, ismulti, callback) {\n\
  if (window.on_native_open_filename_dialog) return;\n\
  window.on_native_open_filename_dialog = callback;\n\
  window.AscDesktopEditor._OpenFilenameDialog(filter, ismulti);\n\
};\n\
window.AscDesktopEditor.SaveFilenameDialog = function(filter, callback) {\n\
  window.on_native_save_filename_dialog = callback;\n\
  window.AscDesktopEditor._SaveFilenameDialog(filter);\n\
};\n\
window.AscDesktopEditor.DownloadFiles = function(filesSrc, filesDst, callback, params) {\n\
  if (filesSrc.length == 0) return callback({});\n\
  window.on_native_download_files = callback;\n\
  window.AscDesktopEditor._DownloadFiles(filesSrc, filesDst, params);\n\
};\n\
window.AscDesktopEditor.SetCryptoMode = function(password, mode, callback) {\n\
  window.on_set_crypto_mode = callback;\n\
  window.AscDesktopEditor._SetCryptoMode(password, mode, callback ? true : false);\n\
};\n\
window.AscDesktopEditor.GetAdvancedEncryptedData = function(password, callback) {\n\
  window.on_get_advanced_encrypted_data = callback;\n\
  window.AscDesktopEditor._GetAdvancedEncryptedData(password);\n\
};\n\
window.AscDesktopEditor.SetAdvancedEncryptedData = function(password, data, callback) {\n\
  window.on_set_advanced_encrypted_data = callback;\n\
  window.AscDesktopEditor._SetAdvancedEncryptedData(password, data);\n\
};\n\
window.AscDesktopEditor.AddVideo = function(file, callback) {\n\
  window.on_add_multimedia_local = callback;\n\
  window.AscDesktopEditor._AddVideo(file);\n\
};\n\
window.AscDesktopEditor.AddAudio = function(file, callback) {\n\
  window.on_add_multimedia_local = callback;\n\
  window.AscDesktopEditor._AddAudio(file);\n\
};\n\
window.AscDesktopEditor.ImportAdvancedEncryptedData = function(callback) {\n\
  window.AscDesktopEditor.OpenFilenameDialog('Key File (*docx);;All files (*.*)', false, function(files) {\n\
	var file = Array.isArray(files) ? files[0] : files;\n\
	if (file)\n\
	{\n\
	  var ret = window.AscDesktopEditor._ImportAdvancedEncryptedData(file);\n\
	  if (callback) callback(ret);\n\
	}\n\
  });\n\
};\n\
window.AscDesktopEditor.ExportAdvancedEncryptedData = function() {\n\
  window.AscDesktopEditor.SaveFilenameDialog('privateKey.docx', function(file) {\n\
	if (file)\n\
	{\n\
	  window.AscDesktopEditor._ExportAdvancedEncryptedData(file);\n\
	}\n\
  });\n\
};\n\
window.AscDesktopEditor.CloudCryptFile = function(url, callback) {\n\
  if (window.on_cloud_crypto_upload) { console.log('CloudCryptFile: waiting...'); return; }\n\
  window.AscDesktopEditor.DownloadFiles([url], [], function(files) {\n\
	var _files = [];\n\
	for (var elem in files)\n\
	  _files.push(files[elem]);\n\
	window.on_cloud_crypto_upload = undefined;\n\
	if (_files && 1 == _files.length)\n\
	{\n\
	  window.on_cloud_crypto_upload = callback;\n\
	  window.AscDesktopEditor._CloudCryptoUpload([_files[0]], true);\n\
	}\n\
  }, 1);\n\
};\n\
window.AscDesktopEditor.CloudCryptUpload = function(filter, callback) {\n\
  if (window.on_cloud_crypto_upload) { console.log('CloudCryptUpload: waiting...'); return; }\n\
  var filterOut = filter || \"\"; if (filterOut == \"\") filterOut = \"any\";\n\
  window.AscDesktopEditor.OpenFilenameDialog(filterOut, true, function(files) {\n\
	window.on_cloud_crypto_upload = undefined;\n\
	if (files && 0 < files.length)\n\
	{\n\
	  window.on_cloud_crypto_upload = callback;\n\
	  window.AscDesktopEditor._CloudCryptoUpload(files);\n\
	}\n\
  });\n\
};\n\
window.AscDesktopEditor.loadLocalFile = function(url, callback, start, len) {\n\
  var xhr = new XMLHttpRequest();\n\
  var loadUrl = url;\n\
  if (start !== undefined) loadUrl += (\"__ascdesktopeditor__param__\" + start);\n\
  if (len !== undefined)\n\
  {\n\
	if (undefined === start) loadUrl += \"__ascdesktopeditor__param__0\";\n\
	loadUrl += (\"__ascdesktopeditor__param__\" + len);\n\
  }\n\
  xhr.open(\"GET\", \"ascdesktop://fonts/\" + loadUrl, true);\n\
  xhr.responseType = \"arraybuffer\";\n\
  if (xhr.overrideMimeType)\n\
	xhr.overrideMimeType('text/plain; charset=x-user-defined');\n\
  else\n\
	xhr.setRequestHeader('Accept-Charset', 'x-user-defined');\n\
  xhr.onload = function() {\n\
	callback(new Uint8Array(xhr.response));\n\
  };\n\
  xhr.onerror = function() {\n\
	callback(null);\n\
  };\n\
  xhr.send(null);\n\
};\n\
Object.defineProperty(window.AscDesktopEditor, 'CryptoMode', {\n\
get: function() { return window.AscDesktopEditor.Property_GetCryptoMode(); },\n\
set: function(value) { window.AscDesktopEditor.Property_SetCryptoMode(value); }\n\
});\n\
window.AscDesktopEditor.isSupportBinaryFontsSprite = true;\n\
window.AscDesktopEditor.cloudCryptoCommandMainFrame=function(a,b){window.cloudCryptoCommandMainFrame_callback=b,window.AscDesktopEditor._cloudCryptoCommandMainFrame(window.AscDesktopEditor.GetFrameId(),JSON.stringify(a))},window.AscDesktopEditor.cloudCryptoCommand=function(a,b,c){switch(window.AscDesktopEditor.initCryptoWorker(b.cryptoEngineId),window.cloudCryptoCommandCounter=0,window.cloudCryptoCommandCount=0,window.cloudCryptoCommandParam=b,window.cloudCryptoCommandCallback=c,a){case\"share\":{var d=Array.isArray(b.file)?b.file:[b.file];window.cloudCryptoCommandCount=d.length,window.AscDesktopEditor.DownloadFiles(d,[],function(a){for(var b in a){let c=a[b],d=window.AscDesktopEditor.isFileSupportCloudCrypt(c,true),e=!1;if(d){let a=window.AscDesktopEditor.getDocumentInfo(c),b=window.cloudCryptoCommandParam;if(\"\"==a){let d=window.AscCrypto.CryptoWorker.createPassword();a=window.AscCrypto.CryptoWorker.generateDocInfo(b.keys,d),e=window.AscDesktopEditor.setDocumentInfo(c,d,a)}else{let d=window.AscCrypto.CryptoWorker.readPassword(a);a=window.AscCrypto.CryptoWorker.generateDocInfo(b.keys,d),e=window.AscDesktopEditor.setDocumentInfo(c,d,a)}}window.AscDesktopEditor.loadLocalFile(c,function(a){window.cloudCryptoCommandCallback({bytes:a,isCrypto:e,url:b}),window.AscDesktopEditor.RemoveFile(c),window.cloudCryptoCommandCounter++,window.cloudCryptoCommandCounter==window.cloudCryptoCommandCount&&(window.cloudCryptoCommandCount=0,delete window.cloudCryptoCommandParam,delete window.cloudCryptoCommandCallback)})}},1);break}case\"upload\":{var e=b.filter||\"any\",f=b.keys||[],g=window.AscCrypto.CryptoWorker.User;f.push({userId:g[2],publicKey:g[1]}),window.AscDesktopEditor.OpenFilenameDialog(e,!0,function(a){Array.isArray(a)||(a=[a]),window.cloudCryptoCommandCount=a.length;for(var b=0;b<a.length;b++){let c=a[b],d=window.AscDesktopEditor.isFileSupportCloudCrypt(c),e=window.AscDesktopEditor.isFileCrypt(c),g=\"\";if(d&&!e){let a=window.AscCrypto.CryptoWorker.createPassword();docinfo=window.AscCrypto.CryptoWorker.generateDocInfo(f,a),g=window.AscDesktopEditor.setDocumentInfo(c,a,docinfo,!0)}let h=\"\"!=g,i=b;window.AscDesktopEditor.loadLocalFile(h?g:c,function(a){var b=c,d=b.lastIndexOf(\"/\");-1!=d&&(b=b.substring(d+1)),d=b.lastIndexOf(\"\\\\\"),-1!=d&&(b=b.substring(d+1)),window.cloudCryptoCommandCallback({bytes:a,isCrypto:h,name:b,index:i,count:window.cloudCryptoCommandCount}),h&&window.AscDesktopEditor.RemoveFile(g),window.cloudCryptoCommandCounter++,window.cloudCryptoCommandCounter==window.cloudCryptoCommandCount&&(window.cloudCryptoCommandCount=0,delete window.cloudCryptoCommandParam,delete window.cloudCryptoCommandCallback)})}});break}default:{c(null),delete window.cloudCryptoCommandParam,delete window.cloudCryptoCommandCallback;break}}};\n\
window.AscDesktopEditor.convertFile = function(path, format, callback) {\n\
window.on_convert_file_callback = function(folder) { callback(window.AscDesktopEditor._onConvertFile(folder)); };\n\
if (path && path.indexOf && (0 === path.indexOf('https://') || 0 === path.indexOf('http://') || 0 === path.indexOf('www.'))) {\n\
window.AscDesktopEditor.DownloadFiles([path], [], function(_files) {\n\
var files = []; for (var _elem in _files) { files.push(_files[_elem]); }\n\
window.AscDesktopEditor._convertFile((files && files[0]) ? files[0] : '', format);\n\
});\n\
} else {\n\
window.AscDesktopEditor._convertFile(path, format);\n\
}\n\
};\n\
window.AscDesktopEditor.getPortalsList = function() { debugger;var ret = []; try { var portals = JSON.parse(localStorage.getItem(\"portals\")); for (var i = 0, len = portals.length; i < len; i++) { ret.push(portals[i].portal); ret.push(portals[i].provider); } } catch(err) { ret = []; } console.log(ret);window.AscDesktopEditor.setPortalsList(ret); };\n\
";
#ifdef CEF_VERSION_ABOVE_102
				sCodeInitJS += "!function(){window.AscSimpleRequest=window.AscSimpleRequest||{};var r=0,o={};window.AscSimpleRequest.createRequest=function(e){var t;o[++r]={id:r,complete:e.complete,error:e.error},e.timeout&&(o[t=r].timer=setTimeout(function(){o[t]&&(o[t].error&&o[t].error({status:\"error\",statusCode:404},\"error\"),delete o[t])},e.timeout)),window.AscDesktopEditor.sendSimpleRequest(r,e)},window.AscSimpleRequest._onSuccess=function(e,t){let r=o[e];r&&(r.timer&&clearTimeout(r.timer),r.complete&&r.complete(t,t.status),delete o[e])},window.AscSimpleRequest._onError=function(e,t){let r=o[e];r&&(r.timer&&clearTimeout(r.timer),r.error&&r.error(t,t.status),delete o[e])}}();\n";
#endif

				_frame->ExecuteJavaScript(sCodeInitJS, _frame->GetURL(), 0);
			}

			return true;
		}
		else if (name == "NativeViewerOpen")
		{
			std::vector<CefRefPtr<CefV8Value>>::const_iterator iter = arguments.begin();

			if (arguments.size() == 1) // password
			{
				m_oNativeViewer.SetPassword((*iter)->GetStringValue().ToWString());
				return true;
			}

			std::wstring sOpeningFilePath = (*iter)->GetStringValue().ToWString(); iter++;
			std::wstring sFontsDir = (*iter)->GetStringValue().ToWString(); iter++;
			std::wstring sFileDir = (*iter)->GetStringValue().ToWString(); ++iter;

			m_oNativeViewer.Init(sFileDir, sFontsDir, sOpeningFilePath, this);
			m_bIsNativeViewerMode = true;

			CefRefPtr<CefV8Value> _timerID;
			CefRefPtr<CefV8Exception> _exception;
			if (CefV8Context::GetCurrentContext()->Eval("(function(){ var intervalID = setInterval(function(){ if (!window.NativeFileOpen_error) { return; } window.AscDesktopEditor.NativeFunctionTimer(intervalID); }, 100); return intervalID; })();",
														#ifndef CEF_2623
																	"", 0,
														#endif
														_timerID, _exception))
			{
				m_nNativeOpenFileTimerID = _timerID->GetIntValue();
				//LOGGER_STRING2("timer created: " + std::to_string(m_nNativeOpenFileTimerID));
			}

			m_oNativeViewer.Start(0);

			return true;
		}
		else if (name == "NativeViewerClose")
		{
			m_oNativeViewer.CloseFile();
			return true;
		}
		else if (name == "NativeFunctionTimer")
		{
			int nIntervalID = arguments[0]->GetIntValue();
			//LOGGER_STRING2("NativeFunctionTimer called: " + std::to_string(nIntervalID));

			if (nIntervalID == m_nNativeOpenFileTimerID)
			{
				std::string sBase64File = m_oNativeViewer.GetBase64File();

				if (!sBase64File.empty())
				{
					CefRefPtr<CefV8Value> _timerID;
					CefRefPtr<CefV8Exception> _exception;

					if (sBase64File == "error" || sBase64File == "password")
					{
						m_oNativeViewer.Stop();
						m_oNativeViewer.ClearBase64();

						std::string sCode = "window.NativeFileOpen_error(\"" +
								sBase64File + "\", \"" + m_oNativeViewer.GetHash() + "\", \"" +
								m_oNativeViewer.GetDocInfo() + "\");";

						CefV8Context::GetCurrentContext()->Eval(sCode,
																#ifndef CEF_2623
																			"", 0,
																#endif
																_timerID, _exception);
						return true;
					}

					std::string sCode = "clearTimeout(" + std::to_string(m_nNativeOpenFileTimerID) + ");";
					if (CefV8Context::GetCurrentContext()->Eval(sCode,
																#ifndef CEF_2623
																			"", 0,
																#endif
																_timerID, _exception))
					{
						//LOGGER_STRING2("timer stoped: " + std::to_string(m_nNativeOpenFileTimerID));
					}
					m_nNativeOpenFileTimerID = -1;

					CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("native_viewer_onopened");
					message->GetArgumentList()->SetString(0, (sBase64File == "error") ? "" : sBase64File);
					message->GetArgumentList()->SetString(1, m_oNativeViewer.GetPassword());
					SEND_MESSAGE_TO_BROWSER_PROCESS(message);
				}
			}

			return true;
		}
		else if (name == "NativeViewerGetPageUrl")
		{
			CNativeViewerPageInfo oInfo;
			oInfo.Page = arguments[0]->GetIntValue();
			oInfo.W = arguments[1]->GetIntValue();
			oInfo.H = arguments[2]->GetIntValue();

			int nPageStart = arguments[3]->GetIntValue();
			int nPageEnd = arguments[4]->GetIntValue();

			if (arguments.size() > 5)
			{
				bool bIsDarkMode = arguments[5]->GetBoolValue();
				m_oNativeViewer.CheckDarkMode(bIsDarkMode);
			}

			std::wstring sUrl = m_oNativeViewer.GetPathPageImage(oInfo);
			if (NSFile::CFileBinary::Exists(sUrl))
				retval = CefV8Value::CreateString(sUrl + m_oNativeViewer.GetUrlAddon());
			else
			{
				m_oNativeViewer.AddTask(oInfo, nPageStart, nPageEnd);
				retval = CefV8Value::CreateString(L"");
			}

			return true;
		}
		else if (name == "NativeViewerGetCompleteTasks")
		{
			CTemporaryCS oCS(&m_oCompleteTasksCS);

			int nCount1 = (int)m_arCompleteTasks.size();
			int nCount2 = (int)m_arCompleteTextTasks.size();

			retval = CefV8Value::CreateArray(2 + nCount1 * 4 + nCount2 * 6);

			int nCurrent = 0;
			retval->SetValue(nCurrent++, CefV8Value::CreateInt(nCount1));
			retval->SetValue(nCurrent++, CefV8Value::CreateInt(nCount2));

			for (std::list<CSavedPageInfo>::iterator i = m_arCompleteTasks.begin(); i != m_arCompleteTasks.end(); i++)
			{
				retval->SetValue(nCurrent++, CefV8Value::CreateString(i->Url));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Page));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->W));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->H));
			}
			m_arCompleteTasks.clear();

			for (std::list<CSavedPageTextInfo>::iterator i = m_arCompleteTextTasks.begin(); i != m_arCompleteTextTasks.end(); i++)
			{
				retval->SetValue(nCurrent++, CefV8Value::CreateString(i->Info));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Page));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Paragraphs));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Words));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Spaces));
				retval->SetValue(nCurrent++, CefV8Value::CreateInt(i->Symbols));
			}
			m_arCompleteTextTasks.clear();

			return true;
		}
		else if (name == "GetInstallPlugins")
		{
			CPluginsManager oPlugins;
			oPlugins.m_strDirectory = m_sSystemPlugins;
			oPlugins.m_strUserDirectory = m_sUserPlugins;
			oPlugins.m_nCryptoMode = m_nCryptoMode;

			// TODO: пока нет платных фич во всех редакторах - это неправильно
			//if (!m_bEditorsCloudFeaturesCheck || m_sEditorsCloudFeatures.length() > 1)
			//    oPlugins.m_strCryptoPluginAttack = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");
			oPlugins.m_strCryptoPluginAttack = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");

			std::string sData = oPlugins.GetPluginsJson(true, true, m_bIsMacrosesSupport, m_bIsPluginsSupport);
			retval = CefV8Value::CreateString(sData);
			return true;
		}
		else if (name == "GetBackupPlugins")
		{
			CPluginsManager oPlugins;
			oPlugins.m_strDirectory = m_sSystemPlugins;
			oPlugins.m_strUserDirectory = m_sUserPlugins;
			oPlugins.m_nCryptoMode = m_nCryptoMode;

			oPlugins.m_strCryptoPluginAttack = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");

			std::string sData = oPlugins.GetPluginsJson(false, true);
			retval = CefV8Value::CreateString(sData);
			return true;
		}
		else if (name == "PluginInstall")
		{
			bool bResult = false;

			CPluginsManager oPlugins;
			oPlugins.m_strDirectory = m_sSystemPlugins;
			oPlugins.m_strUserDirectory = m_sUserPlugins;

			std::wstring sData = ((*arguments.begin())->GetStringValue()).ToWString();

			if (NSFile::CFileBinary::Exists(sData))
			{
				bResult = oPlugins.AddPlugin(sData);
			}
			else
			{
				// Парсим json конфиг
				std::wstring sBaseUrl, sPluginName;
				std::wstring sBackupStart = L"backup\":true";
				std::wstring sBaseUrlStart = L"baseUrl\":\"";
				std::wstring sBaseUrlEnd = L"\"";

				std::string::size_type pos = sData.find(sBackupStart);
				std::string::size_type pos1 = sData.find(sBaseUrlStart);
				std::string::size_type pos2 = sData.find(sBaseUrlEnd, pos1 + sBaseUrlStart.length());

				// Нужно восстановить плагин из папки backup
				if (pos != std::string::npos)
				{
					pos1 = sData.find(L"asc.{");
					pos2 = sData.find(L'}', pos1);

					if (pos1 != std::string::npos &&
						pos2 != std::string::npos &&
						pos2 > pos1)
					{
						std::wstring sGuid = sData.substr(pos1, pos2 - pos1 + 1);
						bResult = oPlugins.RestorePlugin(sGuid);
					}
				}
				// Установка из стора по ссылке
				else if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
				{
					sBaseUrl = sData.substr(pos1 + sBaseUrlStart.length(),
											pos2 - pos1 - sBaseUrlStart.length());

					auto pos = sBaseUrl.rfind(L"/", sBaseUrl.length() - 2);
					sPluginName = sBaseUrl.substr(pos);
					sPluginName.erase(std::remove(sPluginName.begin(), sPluginName.end(), L'/'), sPluginName.end());

					std::wstring sPackageUrl = sBaseUrl + L"/deploy/" + sPluginName + L".plugin";

					std::wstring sTmpFile = NSFile::CFileBinary::GetTempPath() + L"/temp_asc_plugin.plugin";
					if (NSFile::CFileBinary::Exists(sTmpFile))
						NSFile::CFileBinary::Remove(sTmpFile);

					CFileDownloaderWrapper oDownloader(sPackageUrl, sTmpFile);
					oDownloader.DownloadSync();

					if (NSFile::CFileBinary::Exists(sTmpFile))
					{
						bResult = oPlugins.AddPlugin(sTmpFile);
						NSFile::CFileBinary::Remove(sTmpFile);
					}
				}
			}

			// send to editor
			CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
			CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
			if (!_frame)
				_frame = browser->GetMainFrame();

			std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

			retval = CefV8Value::CreateBool(bResult);

			return true;
		}
		else if (name == "PluginUninstall")
		{
			bool bResult = false;

			CPluginsManager oPlugins;
			oPlugins.m_strDirectory = m_sSystemPlugins;
			oPlugins.m_strUserDirectory = m_sUserPlugins;

			std::wstring sGuid = arguments[0]->GetStringValue().ToWString();
			bool bBackup = (arguments.size() > 1 && arguments[1]->IsBool()) ? arguments[1]->GetBoolValue() : false;

			bResult = oPlugins.RemovePlugin(sGuid, bBackup);

			// send to editor
			CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
			CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
			if (!_frame)
				_frame = browser->GetMainFrame();

			std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

			retval = CefV8Value::CreateBool(bResult);

			return true;
		}
		else if (name == "IsLocalFile")
		{
			bool isUrlCheck = false;
			if (arguments.size() > 0)
				isUrlCheck = (*arguments.begin())->GetBoolValue();

			retval = CefV8Value::CreateBool(IsLocalFile(isUrlCheck));
			return true;
		}
		else if (name == "IsFilePrinting")
		{
			retval = CefV8Value::CreateBool(m_bIsPrinting);
			return true;
		}
		else if (name == "Sign")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_sign");

			int nSize = (int)arguments.size();
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, (nSize > 1) ? arguments[1]->GetStringValue() : "");
			message->GetArgumentList()->SetString(2, (nSize > 2) ? arguments[2]->GetStringValue() : "");
			message->GetArgumentList()->SetString(3, (nSize > 3) ? arguments[3]->GetStringValue() : "");

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "RemoveSignature")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_remove");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "RemoveAllSignatures")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_remove");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "ViewCertificate")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_viewcertificate");
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SelectCertificate")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_selectsertificate");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "GetDefaultCertificate")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_signature_defaultcertificate");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_OpenFilenameDialog")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_open_filename_dialog");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, std::to_string(CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier()));
			if (arguments.size() > 1)
				message->GetArgumentList()->SetBool(2, arguments[1]->GetBoolValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SaveQuestion")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_file_save_question");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "startReporter")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_start_reporter");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, m_sLocalFileFolder);
			message->GetArgumentList()->SetString(2, m_sVersion);

			// проверяекм на cloudCrypto
			std::string sCloudCryptoId = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");
			if (!sCloudCryptoId.empty())
			{
				std::string sJson = "{ \"cryptoEngineId\" : \"" + sCloudCryptoId + "\", \"userId\" : \"";
				sJson += CAscRendererProcessParams::getInstance().GetProperty("user");
				sJson += "\" }";

				message->GetArgumentList()->SetString(3, sJson);
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "endReporter")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_end_reporter");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "sendToReporter")
		{
			// сообщение докладчику
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_to_reporter");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "sendFromReporter")
		{
			// сообщение от докладчика
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_from_reporter");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "IsSignaturesSupport")
		{
			retval = CefV8Value::CreateBool(m_bIsSupportSigs);
			return true;
		}
		else if (name == "IsProtectionSupport")
		{
			bool bIsProtect = m_bIsSupportProtect;
			if (bIsProtect)
			{
				if (m_bIsSupportOnlyPass && IsPropertyCryptoMode())
					bIsProtect = false;
			}
			retval = CefV8Value::CreateBool(bIsProtect);
			return true;
		}
		else if (name == "SetSupportSign")
		{
			m_bIsSupportSigs = arguments[0]->GetBoolValue();
			return true;
		}
		else if (name == "SetInitFlags")
		{
			int nFlags = arguments[0]->GetIntValue();
			m_bIsSupportOnlyPass = ((nFlags & 0x01) == 0x01) ? true : false;
			m_bIsSupportProtect = ((nFlags & 0x02) == 0x02) ? true : false;

			if (1 < arguments.size())
				m_nCryptoMode = arguments[1]->GetIntValue();

			return true;
		}
		else if (name == "isBlockchainSupport")
		{
			retval = CefV8Value::CreateBool(m_bIsSupportOnlyPass && IsPropertyCryptoMode());
			return true;
		}
		else if (name == "_sendSystemMessage")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_system_message");

			int64 frameID = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();

			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, std::to_string(frameID));

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_GetHash")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("file_get_hash");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, (arguments.size() > 1) ? arguments[1]->GetStringValue() : "sha-256");
			message->GetArgumentList()->SetString(2, std::to_string(CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier()));
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_CallInAllWindows")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("call_in_all_windows");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_OpenFileCrypt")
		{
			m_sEditorPageDomain = NSCommon::GetBaseDomain(CefV8Context::GetCurrentContext()->GetFrame()->GetURL().ToWString(), true);
			m_sInternalEditorPageDomain = NSCommon::GetBaseDomain(arguments[1]->GetStringValue().ToWString(), true);

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("open_file_crypt");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "buildCryptedStart")
		{
			if (arguments.size() == 0)
			{
				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted");
				SEND_MESSAGE_TO_BROWSER_PROCESS(message);
				return true;
			}

			std::string sContent = arguments[0]->GetStringValue().ToString();
			BYTE* pDataDst = NULL;
			int nLenDst = 0;

			NSFile::CBase64Converter::Decode(sContent.c_str(), sContent.length(), pDataDst, nLenDst);

			NSFile::CFileBinary oFileWithChanges;
			oFileWithChanges.CreateFileW(m_sCryptDocumentFolder + L"/EditorWithChanges.bin");
			oFileWithChanges.WriteFile(pDataDst, nLenDst);
			oFileWithChanges.CloseFile();

			RELEASEARRAYOBJECTS(pDataDst);

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted");
			message->GetArgumentList()->SetString(0, arguments[2]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[3]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "buildCryptedEnd")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("build_crypted_end");
			message->GetArgumentList()->SetBool(0, arguments[0]->GetBoolValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "SetCryptDocumentFolder")
		{
			m_sCryptDocumentFolder = arguments[0]->GetStringValue().ToWString();
			return true;
		}
		else if (name == "PreloadCryptoImage")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("preload_crypto_image");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
			NSArgumentList::SetInt64(message->GetArgumentList(), 2, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "ResaveFile")
		{
			std::wstring sFile = /*m_sCryptDocumentFolder + */arguments[0]->GetStringValue().ToWString();
			std::string sData = arguments[1]->GetStringValue().ToString();

			std::wstring sFileHeader = L"ascdesktop://fonts/";
			if (0 == sFile.find(sFileHeader))
			{
				sFile = sFile.substr(sFileHeader.length());

#ifdef WIN32
				NSStringUtils::string_replace(sFile, L"/", L"\\");
#endif
			}

			BYTE* pDataDst = NULL;
			int nSizeDst = 0;
			NSFile::CBase64Converter::Decode(sData.c_str(), (int)sData.length(), pDataDst, nSizeDst);

			if (NSFile::CFileBinary::Exists(sFile))
				NSFile::CFileBinary::Remove(sFile);

			NSFile::CFileBinary oFile;
			oFile.CreateFileW(sFile);
			oFile.WriteFile(pDataDst, (DWORD)nSizeDst);
			oFile.CloseFile();

			RELEASEARRAYOBJECTS(pDataDst);

			return true;
		}
		else if (name == "_DownloadFiles")
		{
			std::wstring sDirTmp = m_sCryptDocumentFolder;
			if (sDirTmp.empty())
				sDirTmp = m_sLocalFileFolderWithoutFile;
			if (sDirTmp.empty())
				sDirTmp = m_sAppTmpFolder;
			if (sDirTmp.empty())
				sDirTmp = NSFile::CFileBinary::GetTempPath();

			CefRefPtr<CefV8Value> val = arguments[0];
			int nCount = val->GetArrayLength();

            int nCount2 = 0;
			CefRefPtr<CefV8Value> val2 = nullptr;
            if (arguments.size() > 1)
            {
                val2 = arguments[1];
                nCount2 = val2->GetArrayLength();
            }

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("download_files");

			int nParams = 0;
			if (arguments.size() > 2 && arguments[2]->IsInt())
				nParams = arguments[2]->GetIntValue();

			int nIndex = 0;

			message->GetArgumentList()->SetInt(nIndex++, nParams);
			NSArgumentList::SetInt64(message->GetArgumentList(), nIndex++, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());

			for (int i = 0; i < nCount; ++i)
			{
				std::wstring sUrlCurrent = val->GetValue(i)->GetStringValue().ToWString();
				if (!m_sEditorPageDomain.empty() && !m_sInternalEditorPageDomain.empty() && 0 == sUrlCurrent.find(m_sInternalEditorPageDomain))
				{
					sUrlCurrent = m_sEditorPageDomain + sUrlCurrent.substr(m_sInternalEditorPageDomain.length());
				}

				message->GetArgumentList()->SetString(nIndex++, sUrlCurrent);

				if (i < nCount2)
				{
					message->GetArgumentList()->SetString(nIndex++, val2->GetValue(i)->GetStringValue());
				}
				else
				{
					std::wstring sFileTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(sDirTmp, L"IMG");
					if (NSFile::CFileBinary::Exists(sFileTmp))
						NSFile::CFileBinary::Remove(sFileTmp);
					message->GetArgumentList()->SetString(nIndex++, sFileTmp);
				}
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			return true;
		}
		else if (name == "RemoveFile")
		{
			std::wstring sFile = arguments[0]->GetStringValue();
			if (NSFile::CFileBinary::Exists(sFile))
				NSFile::CFileBinary::Remove(sFile);
			return true;
		}
		else if (name == "_SetCryptoMode")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_crypto_mode");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetInt(1, arguments[1]->GetIntValue());
			message->GetArgumentList()->SetBool(2, (arguments.size() > 2) ? arguments[2]->GetBoolValue() : false);
			NSArgumentList::SetInt64(message->GetArgumentList(), 3, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "GetImageFormat")
		{
			std::wstring sFile = arguments[0]->GetStringValue();
			std::string sExt = "jpg";

			CImageFileFormatChecker _checker;
			if (_checker.isPngFile(sFile))
				sExt = "png";

			retval = CefV8Value::CreateString(sExt);
			return true;
		}
		else if (name == "GetEncryptedHeader")
		{
			retval = CefV8Value::CreateString("ENCRYPTED;");
			return true;
		}
		else if (name == "GetCryptoMode")
		{
			retval = CefV8Value::CreateInt(m_nCryptoMode);
			return true;
		}
		else if (name == "GetSupportCryptoModes")
		{
			NSAscCrypto::CCryptoMode oCryptoMode;
			NSAscCrypto::CCryptoKey keyEnc, keyDec;
			oCryptoMode.Load(keyEnc, keyDec, m_sCookiesPath + L"/user.data");

			CPluginsManager oPlugins;
			oPlugins.m_strDirectory = m_sSystemPlugins;
			oPlugins.m_strUserDirectory = m_sUserPlugins;
			oPlugins.GetInstalledPlugins();

			int nCount = (int)oPlugins.m_arCryptoModes.size();
			retval = CefV8Value::CreateArray(nCount);
			int nCurIndex = 0;
			for (std::map<int, std::string>::iterator iter = oPlugins.m_arCryptoModes.begin(); iter != oPlugins.m_arCryptoModes.end(); iter++)
			{
				int nMode = iter->first;
#ifdef CEF_2623
				CefRefPtr<CefV8Value> val = CefV8Value::CreateObject(NULL);
#else
				CefRefPtr<CefV8Value> val = CefV8Value::CreateObject(nullptr, nullptr);
#endif
				val->SetValue("type", CefV8Value::CreateInt(nMode), V8_PROPERTY_ATTRIBUTE_NONE);
				val->SetValue("info_presented", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);

				retval->SetValue(nCurIndex++, val);
			}
			return true;
		}
		else if (name == "_GetAdvancedEncryptedData")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("get_advanced_encrypted_data");
			NSArgumentList::SetInt64(message->GetArgumentList(), 0, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_SetAdvancedEncryptedData")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_advanced_encrypted_data");
			NSArgumentList::SetInt64(message->GetArgumentList(), 0, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(2, arguments[1]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "GetExternalClouds")
		{
			CExternalClouds oClouds;
#ifdef _MAC
			oClouds.m_sSystemDirectory = NSFile::GetProcessDirectory() + L"/../../../../Resources/providers";
#else
			oClouds.m_sSystemDirectory = NSFile::GetProcessDirectory() + L"/providers";
#endif

			std::string sClouds = oClouds.GetAllJSON();
			if ("[]" != sClouds)
			{
				CefRefPtr<CefV8Exception> exception1;
				CefV8Context::GetCurrentContext()->Eval("(function(){ return " + sClouds + "; })();",
																		  #ifndef CEF_2623
																					  "", 0,
																		  #endif
																		  retval, exception1);
			}
			else
			{
				retval = CefV8Value::CreateNull();
			}
			return true;
		}
		else if (name == "IsNativeViewer")
		{
			retval = CefV8Value::CreateBool(m_bIsNativeViewerMode);
			return true;
		}
		else if (name == "CryptoDownloadAs")
		{
			std::string fileData = arguments[0]->GetStringValue().ToString();

			NSFile::CFileBinary oFileWithChanges;
			oFileWithChanges.CreateFileW(m_sCryptDocumentFolder + L"/EditorWithChanges.bin");
			oFileWithChanges.WriteFile((BYTE*)fileData.c_str(), (DWORD)fileData.length());
			oFileWithChanges.CloseFile();

			int nFormat = arguments[1]->GetIntValue();
			nFormat = NSCommon::CorrectSaveFormat(nFormat);

			std::string sParams = "";
			if (arguments.size() > 2)
				sParams = arguments[2]->GetStringValue();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("crypto_download_as");
			message->GetArgumentList()->SetInt(0, nFormat);
			message->GetArgumentList()->SetString(1, sParams);
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);

			return true;
		}
		else if (name == "MediaStart")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("media_start");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetInt(1, arguments[1]->GetIntValue());
			message->GetArgumentList()->SetInt(2, arguments[2]->GetIntValue());
			message->GetArgumentList()->SetDouble(3, arguments[3]->GetDoubleValue());
			message->GetArgumentList()->SetDouble(4, arguments[4]->GetDoubleValue());
			message->GetArgumentList()->SetDouble(5, arguments[5]->GetDoubleValue());

			if (arguments.size() > 6)
			{
				message->GetArgumentList()->SetDouble(6, arguments[6]->GetDoubleValue());
				message->GetArgumentList()->SetDouble(7, arguments[7]->GetDoubleValue());
				message->GetArgumentList()->SetDouble(8, arguments[8]->GetDoubleValue());
				message->GetArgumentList()->SetDouble(9, arguments[9]->GetDoubleValue());
				message->GetArgumentList()->SetDouble(10, arguments[10]->GetDoubleValue());
				message->GetArgumentList()->SetDouble(11, arguments[11]->GetDoubleValue());
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "MediaEnd")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("media_end");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}


		if (name == "GetImageOriginalSize")
		{
#ifdef CEF_2623
			retval = CefV8Value::CreateObject(NULL);
#else
			retval = CefV8Value::CreateObject(nullptr, nullptr);
#endif
			int nW = 0;
			int nH = 0;

			std::wstring sUrl = arguments[0]->GetStringValue().ToWString();

			std::wstring sUrlFile = sUrl;
			if (sUrlFile.find(L"file://") == 0)
			{
				sUrlFile = sUrlFile.substr(7);

				// MS Word copy image with url "file://localhost/..." on mac
				if (0 == sUrlFile.find(L"localhost"))
					sUrlFile = sUrlFile.substr(9);

				NSStringUtils::string_replace(sUrlFile, L"%20", L" ");

				if (!NSFile::CFileBinary::Exists(sUrlFile))
					sUrlFile = sUrlFile.substr(1);
			}

			if (NSFile::CFileBinary::Exists(sUrlFile))
			{
				CBgraFrame oFrame;
				if (oFrame.OpenFile(sUrlFile))
				{
					nW = oFrame.get_Width();
					nH = oFrame.get_Height();
				}
			}
			else if (IsNeedDownload(sUrl))
			{
				std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
				if (NSFile::CFileBinary::Exists(sTmpFile))
					NSFile::CFileBinary::Remove(sTmpFile);

				CFileDownloaderWrapper oDownloader(sUrl, sTmpFile);
				oDownloader.DownloadSync();

				CBgraFrame oFrame;
				if (oFrame.OpenFile(sTmpFile))
				{
					nW = oFrame.get_Width();
					nH = oFrame.get_Height();
				}

				if (NSFile::CFileBinary::Exists(sTmpFile))
					NSFile::CFileBinary::Remove(sTmpFile);
			}
			else if (0 == sUrl.find(L"data:"))
			{
				std::wstring::size_type nBase64Start = sUrl.find(L"base64,");
				if (nBase64Start != std::wstring::npos)
				{
					int nStartIndex = (int)nBase64Start + 7;
					int nCount = (int)sUrl.length() - nStartIndex;
					char* pData = new char[nCount];
					const wchar_t* pDataSrc = sUrl.c_str();
					for (int i = 0; i < nCount; ++i)
						pData[i] = (char)pDataSrc[i + nStartIndex];

					BYTE* pDataDecode = NULL;
					int nLenDecode = 0;
					NSFile::CBase64Converter::Decode(pData, nCount, pDataDecode, nLenDecode);

					RELEASEARRAYOBJECTS(pData);

					std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
					if (NSFile::CFileBinary::Exists(sTmpFile))
						NSFile::CFileBinary::Remove(sTmpFile);

					NSFile::CFileBinary oFile;
					if (oFile.CreateFileW(sTmpFile))
					{
						oFile.WriteFile(pDataDecode, (DWORD)nLenDecode);
						oFile.CloseFile();
					}

					RELEASEARRAYOBJECTS(pDataDecode);

					CBgraFrame oFrame;
					if (oFrame.OpenFile(sTmpFile))
					{
						nW = oFrame.get_Width();
						nH = oFrame.get_Height();
					}

					if (NSFile::CFileBinary::Exists(sTmpFile))
						NSFile::CFileBinary::Remove(sTmpFile);
				}
			}
			else if (sUrl.find(L"image") == 0)
			{
				sUrl = m_sLocalFileFolderWithoutFile + L"/media/" + sUrl;

				CBgraFrame oFrame;
				if (oFrame.OpenFile(sUrl))
				{
					nW = oFrame.get_Width();
					nH = oFrame.get_Height();
				}
			}

			retval->SetValue("W", CefV8Value::CreateInt(nW), V8_PROPERTY_ATTRIBUTE_NONE);
			retval->SetValue("H", CefV8Value::CreateInt(nH), V8_PROPERTY_ATTRIBUTE_NONE);
			return true;
		}
		else if (name == "_AddAudio")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			std::wstring sExt = NSFile::GetFileExtention(sFile);
			NSCommon::makeLowerW(sExt);
			std::wstring sImage = L"display8image" + std::to_wstring(m_nLocalImagesNextIndex++);
			std::wstring sDstMain = m_sLocalFileFolderWithoutFile + L"/media/" + sImage + L".";
			std::wstring sDst = sDstMain + sExt;

			NSFile::CFileBinary::Copy(sFile, sDst);

			std::wstring sSrc = m_sSystemPlugins + L"/../sdkjs/common/Images/local/audio/image";

			//NSFile::CFileBinary::Copy(sSrc + L".svg", sDstMain + L"svg");
			//NSFile::CFileBinary::Copy(sSrc + L".emf", sDstMain + L"emf");
			NSFile::CFileBinary::Copy(sSrc + L".png", sDstMain + L"png");

			std::wstring sCode = L"(function(){ window.on_add_multimedia_local(\"" + sImage + L".png\", \"" + sImage + L"." + sExt + L"\"); window.on_add_multimedia_local = undefined; })();"; // .svg
			CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			return true;
		}
		else if (name == "_AddVideo")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			std::wstring sExt = NSFile::GetFileExtention(sFile);
			NSCommon::makeLowerW(sExt);
			std::wstring sImage = L"display8image" + std::to_wstring(m_nLocalImagesNextIndex++);
			std::wstring sDstMain = m_sLocalFileFolderWithoutFile + L"/media/" + sImage + L".";
			std::wstring sDst = sDstMain + sExt;

			NSFile::CFileBinary::Copy(sFile, sDst);

			std::wstring sSrc = m_sSystemPlugins + L"/../sdkjs/common/Images/local/video/image";

			NSFile::CFileBinary::Copy(sSrc + L".png", sDstMain + L"png");

			std::wstring sCode = L"(function(){ window.on_add_multimedia_local(\"" + sImage + L".png\", \"" + sImage + L"." + sExt + L"\"); window.on_add_multimedia_local = undefined; })();"; // .svg
			CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			return true;
		}
		else if (name == "SendByMail")
		{
			CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetBrowser()->GetFrame("frameEditor");
			if (!_frame)
				return true;

			if (CefV8Context::GetCurrentContext()->GetFrame()->GetName() != "frameEditor")
			{
				std::string sCode = "window.AscDesktopEditor.SendByMail();";
				_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
				return true;
			}

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_by_mail");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "IsLocalFileExist")
		{
			if (arguments.size() != 1)
			{
				retval = CefV8Value::CreateBool(false);
				return true;
			}

			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			if (sFile.find(L"file://") == 0)
			{
				if (NSFile::CFileBinary::Exists(sFile.substr(7)))
					sFile = sFile.substr(7);
				else if (NSFile::CFileBinary::Exists(sFile.substr(8)))
					sFile = sFile.substr(8);
			}

			retval = CefV8Value::CreateBool(NSFile::CFileBinary::Exists(sFile));
			return true;
		}
		else if (name == "_CloudCryptoUpload")
		{
			if (!m_bIsEnableUploadCrypto)
			{
				m_bIsEnableUploadCrypto = true;
				// расширяем функцию работой с генерацией паролей и сохранением их
				CefRefPtr<CefFrame> curFrame = CefV8Context::GetCurrentContext()->GetFrame();
				if (curFrame)
				{
					curFrame->ExecuteJavaScript("\
window.onSystemMessage2 = window.onSystemMessage;\n\
window.onSystemMessage = function(e) {\n\
switch (e.type)\n\
{\n\
  case \"generatePassword\":\n\
  {\n\
	window.AscDesktopEditor._CloudCryptoUploadPass(e.password, e.docinfo);\n\
	break;\n\
  }\n\
  case \"setPasswordByFile\":\n\
  {\n\
	if (e.isNeedMessage)\n\
	{\n\
	  var messageText = e.message;\n\
	  delete e.isNeedMessage;\n\
	  delete e.message;\n\
	  window.onSystemMessage({ type : \"operation\", block : true, opType : 2, opMessage : messageText });\n\
	  window.AscDesktopEditor.sendSystemMessage(e);\n\
	}\n\
	else\n\
	{\n\
	  window.AscDesktopEditor._CloudCryptoUploadSave();\n\
	}\n\
	break;\n\
  }\n\
  default:\n\
	break;\n\
}\n\
if (window.onSystemMessage2) window.onSystemMessage2(e);\n\
};", curFrame->GetURL(), 0);
				}
			}

			int nCount = arguments[0]->GetArrayLength();
			bool bIsNeedRemoveAfterUse = false;
			if (arguments.size() > 1)
				bIsNeedRemoveAfterUse = arguments[1]->GetBoolValue();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cloud_crypto_upload");
			message->GetArgumentList()->SetBool(0, bIsNeedRemoveAfterUse);
			NSArgumentList::SetInt64(message->GetArgumentList(), 1, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			message->GetArgumentList()->SetInt(2, nCount);
			for (int i = 0; i < nCount; ++i)
				message->GetArgumentList()->SetString(3 + i, arguments[0]->GetValue(i)->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_CloudCryptoUploadPass")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cloud_crypto_upload_pass");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_CloudCryptoUploadSave")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cloud_crypto_upload_save");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "CloudCryptUploadEnd")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cloud_crypto_upload_end");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "getLocalFileSize")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			long lSize = 0;

			NSFile::CFileBinary oFile;
			if (oFile.OpenFile(sFile))
				lSize = oFile.GetFileSize();

            retval = CefV8Value::CreateInt((int)lSize);
            return true;
        }
        else if (name == "_getMainUrl")
        {
            CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
			CefRefPtr<CefFrame> frame = browser ? browser->GetMainFrame() : nullptr;
            retval = CefV8Value::CreateString(frame ? frame->GetURL() : "");
            return true;
        }
        else if (name == "_getCurrentUrl")
        {
            CefRefPtr<CefFrame> frame = CefV8Context::GetCurrentContext()->GetFrame();
            retval = CefV8Value::CreateString(frame ? frame->GetURL() : "");
            return true;
        }
        else if (name == "_SaveFilenameDialog")
        {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_save_filename_dialog");
            message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
            message->GetArgumentList()->SetString(1, std::to_string(CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier()));
            SEND_MESSAGE_TO_BROWSER_PROCESS(message);
            return true;
        }
        else if (name == "_ImportAdvancedEncryptedData")
        {
            std::wstring sFile = arguments[0]->GetStringValue().ToWString();

			COfficeFileFormatChecker oChecker;
			bool bIsOfficeFile = oChecker.isOfficeFile(sFile);
			if (bIsOfficeFile && oChecker.nFileType == AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO)
			{
				NSFile::CFileBinary::Copy(sFile, m_sUserPlugins + L"/advanced_crypto_data.docx");
				retval = CefV8Value::CreateBool(true);
			}
			else
			{
				retval = CefV8Value::CreateBool(false);
			}
			return true;
		}
		else if (name == "_ExportAdvancedEncryptedData")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			NSFile::CFileBinary::Copy(m_sUserPlugins + L"/advanced_crypto_data.docx", sFile);
			return true;
		}
		else if (name == "CompareDocumentUrl" ||
				 name == "CompareDocumentFile" ||
				 name == "MergeDocumentUrl" ||
				 name == "MergeDocumentFile")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_compare_document");

			std::string sFileOrUrlType = "url";
			if (name == "CompareDocumentFile" || name == "MergeDocumentFile")
				sFileOrUrlType = "file";

			message->GetArgumentList()->SetString(0, sFileOrUrlType);
			message->GetArgumentList()->SetString(1, arguments[0]->GetStringValue());

			std::string sType = "compare";
			if (name == "MergeDocumentUrl" || name == "MergeDocumentFile")
				sType = "merge";

			message->GetArgumentList()->SetString(2, sType);

			if (arguments.size() == 4)
			{
				std::wstring sLocalDir = m_sCryptDocumentFolder;
				if (!sLocalDir.empty())
				{
					std::string sContent = arguments[2]->GetStringValue().ToString();
					BYTE* pDataDst = NULL;
					int nLenDst = 0;

					NSFile::CBase64Converter::Decode(sContent.c_str(), sContent.length(), pDataDst, nLenDst);

					NSFile::CFileBinary oFileWithChanges;
					oFileWithChanges.CreateFileW(sLocalDir + L"/EditorForCompare.bin");
					oFileWithChanges.WriteFile(pDataDst, nLenDst);
					oFileWithChanges.CloseFile();

					RELEASEARRAYOBJECTS(pDataDst);
				}
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}		
		else if (name == "onDocumentContentReady")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_document_content_ready");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "IsSupportMedia")
		{
			bool bIsLocal = IsLocalFile(true);
#ifdef _MAC
			bIsLocal = false;
#endif
			retval = CefV8Value::CreateBool(bIsLocal);
			return true;
		}
		else if (name == "SetIsReadOnly")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_set_is_readonly");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "CryptoAES_Init")
		{
			std::string sPassword = arguments[0]->GetStringValue().ToString();
			std::string sSalt = "";
			if (arguments.size() > 1)
				sSalt = arguments[0]->GetStringValue().ToString();

			if (NULL != m_pAES_Key)
				NSOpenSSL::openssl_free(m_pAES_Key);
			m_pAES_Key = NSOpenSSL::PBKDF2_desktop_GCM(sPassword, sSalt);
			return true;
		}
		else if (name == "CryptoAES_Clean")
		{
			if (NULL != m_pAES_Key)
				NSOpenSSL::openssl_free(m_pAES_Key);
			return true;
		}
		else if (name == "CryptoAES_Encrypt")
		{
			std::string sMessage = arguments[0]->GetStringValue().ToString();
			std::string sOut;
			NSOpenSSL::AES_Encrypt_desktop_GCM(m_pAES_Key, sMessage, sOut);
			retval = CefV8Value::CreateString(sOut);
			return true;
		}
		else if (name == "CryptoAES_Decrypt")
		{
			std::string sMessage = arguments[0]->GetStringValue().ToString();
			std::string sOut;

			int nVersionOffset = 0;
			int nVersion = NSCommon::CheckEncryptedVersion(sMessage, nVersionOffset);

			if (nVersion == 2)
				NSOpenSSL::AES_Decrypt_desktop_GCM(m_pAES_Key, sMessage, sOut, nVersionOffset);
			else
				NSOpenSSL::AES_Decrypt_desktop_GCM(m_pAES_Key, sMessage, sOut);

			retval = CefV8Value::CreateString(sOut);
			return true;
		}
		else if (name == "CryproRSA_CreateKeys")
		{
			unsigned char* publicKey = NULL;
			unsigned char* privateKey = NULL;
			NSOpenSSL::RSA_GenerateKeys(publicKey, privateKey);

			std::string sPublic((char*)publicKey);
			std::string sPrivate((char*)privateKey);

			retval = CefV8Value::CreateArray(2);
			retval->SetValue(0, CefV8Value::CreateString(sPrivate));
			retval->SetValue(1, CefV8Value::CreateString(sPublic));

			NSOpenSSL::openssl_free(publicKey);
			NSOpenSSL::openssl_free(privateKey);
			return true;
		}
		else if (name == "CryproRSA_EncryptPublic")
		{
			std::string sKey = arguments[0]->GetStringValue().ToString();
			NSStringUtils::string_replaceA(sKey, "&#xA", "\n");
			std::string sMessage = arguments[1]->GetStringValue().ToString();
			std::string sOut;
			NSOpenSSL::RSA_EncryptPublic_desktop((unsigned char*)sKey.c_str(), sMessage, sOut);
			retval = CefV8Value::CreateString(sOut);
			return true;
		}
		else if (name == "CryproRSA_DecryptPrivate")
		{
			std::string sKey = arguments[0]->GetStringValue().ToString();
			std::string sMessage = arguments[1]->GetStringValue().ToString();
			std::string sOut;
			NSOpenSSL::RSA_DecryptPrivate_desktop((unsigned char*)sKey.c_str(), sMessage, sOut);
			retval = CefV8Value::CreateString(sOut);
			return true;
		}
		else if (name == "IsCloudCryptoSupport")
		{
			retval = CefV8Value::CreateBool(true);
			return true;
		}
		else if (name == "CryptoGetHash")
		{
			std::string sMessage = arguments[0]->GetStringValue().ToString();
			int alg = arguments[1]->GetIntValue();
			unsigned int data_len = 0;
			unsigned char* data = NSOpenSSL::GetHash((unsigned char*)sMessage.c_str(), (unsigned int)sMessage.length(), alg, data_len);
			std::string sResult = NSOpenSSL::Serialize(data, data_len, OPENSSL_SERIALIZE_TYPE_HEX);
			NSOpenSSL::openssl_free(data);
			retval = CefV8Value::CreateString(sResult);
			return true;
		}
		else if (name == "CryptoCloud_GetUserInfo")
		{
			std::string userA = CAscRendererProcessParams::getInstance().GetProperty("user");
			CCloudCryptoDesktop info;
			info.User = UTF8_TO_U(userA);

			CCloudCryptoApp oApp(m_sUserPlugins + L"/cloud_crypto.xml");
			CCloudCryptoDesktop* pInfo = oApp.GetInfo(info);

			if (!pInfo)
				return true;

			retval = CefV8Value::CreateArray(3);
			retval->SetValue(0, CefV8Value::CreateString(pInfo->PrivateKey));
			retval->SetValue(1, CefV8Value::CreateString(pInfo->PublicKey));
			retval->SetValue(2, CefV8Value::CreateString(pInfo->User));
			return true;
		}
		else if (name == "Property_GetCryptoMode")
		{
			int nMode = m_nIsCryptoModeProperty;

			std::string sPluginCryptoAttack = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");
			if (!sPluginCryptoAttack.empty()) // и существует!!!
				nMode = 2;

			retval = CefV8Value::CreateInt(nMode);
			return true;
		}
		else if (name == "Property_SetCryptoMode")
		{
			m_nIsCryptoModeProperty = arguments[0]->GetIntValue();
			return true;
		}
		else if (name == "GetFrameId")
		{
			int64 frameID = CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier();
			uint64 uframeID = (uint64)frameID;
			std::string sId = std::to_string(uframeID);
			retval = CefV8Value::CreateString(sId);
			return true;
		}
		else if (name == "CallInFrame")
		{
			std::string sId = arguments[0]->GetStringValue().ToString();
			std::string sCode = arguments[1]->GetStringValue().ToString();
			int64 frameId = (int64)(std::stoull(sId));
			CefRefPtr<CefFrame> frame = CefV8Context::GetCurrentContext()->GetBrowser()->GetFrame(frameId);
			if (frame)
				frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
			return true;
		}
		else if (name == "_cloudCryptoCommandMainFrame")
		{
			std::string sId = arguments[0]->GetStringValue().ToString();
			std::string sArg = arguments[1]->GetStringValue().ToString();

			NSStringUtils::string_replaceA(sArg, "\r", "");
			NSStringUtils::string_replaceA(sArg, "\n", "");
			NSStringUtils::string_replaceA(sArg, "\\", "\\\\");
			NSStringUtils::string_replaceA(sArg, "\"", "\\\"");

			CefRefPtr<CefFrame> mainFrame = CefV8Context::GetCurrentContext()->GetBrowser()->GetMainFrame();

			std::string sCode = "(function(){\n\
var obj = JSON.parse(\"" + sArg + "\");\n\
window.cloudCryptoCommand(obj.type, obj.param, function(param){\n\
window.AscDesktopEditor.CallInFrame(\"" + sId + "\", \
\"window.cloudCryptoCommandMainFrame_callback(\" + JSON.stringify(param || {}) + \");delete window.cloudCryptoCommandMainFrame_callback;\");\n\
});\n\
})();";
			if (mainFrame)
				mainFrame->ExecuteJavaScript(sCode, mainFrame->GetURL(), 0);
			return true;
		}
		else if (name == "initCryptoWorker")
		{
			std::wstring sId = arguments[0]->GetStringValue().ToWString();
			NSStringUtils::string_replace(sId, L"asc.", L"");
			std::wstring sFile = m_sSystemPlugins + L"/" + sId + L"/worker.js";
			std::string sContentWorker;
			if (NSFile::CFileBinary::Exists(sFile))
				NSFile::CFileBinary::ReadAllTextUtf8A(sFile, sContentWorker);
			else
			{
				sFile = m_sUserPlugins + L"/" + sId + L"/worker.js";
				if (NSFile::CFileBinary::Exists(sFile))
					NSFile::CFileBinary::ReadAllTextUtf8A(sFile, sContentWorker);
			}

			if (!sContentWorker.empty())
			{
				CefRefPtr<CefFrame> frame = CefV8Context::GetCurrentContext()->GetFrame();
				if (frame)
					frame->ExecuteJavaScript(sContentWorker, frame->GetURL(), 0);
			}
			return true;
		}
		else if (name == "getDocumentInfo")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();

			COfficeFileFormatChecker oChecker;
			oChecker.isOfficeFile(sFile);
			if (AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO != oChecker.nFileType)
			{
				retval = CefV8Value::CreateString("");
				return true;
			}

			ECMACryptFile file;
			std::string docinfo = file.ReadAdditional(sFile, L"DocumentID");
			if (!docinfo.empty())
				NSStringUtils::string_replaceA(docinfo, "\n", "<!--break-->");
			retval = CefV8Value::CreateString(docinfo);
			return true;
		}
		else if (name == "setDocumentInfo")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			std::wstring sPassword = arguments[1]->GetStringValue().ToWString();
			std::wstring sDocinfo = arguments[2]->GetStringValue().ToWString();
			bool bIsCopy = false;
			if (arguments.size() >= 4)
				bIsCopy = arguments[3]->GetBoolValue();

			std::wstring sTmpFile = L"";
			if (bIsCopy)
			{
				sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
				if (NSFile::CFileBinary::Exists(sTmpFile))
					NSFile::CFileBinary::Remove(sTmpFile);

				NSFile::CFileBinary::Copy(sFile, sTmpFile);
				sFile = sTmpFile;
			}

			bool isCrypt = false;

			COfficeFileFormatChecker oChecker;
			oChecker.isOfficeFile(sFile);
			if (AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO != oChecker.nFileType)
			{
				ECMACryptFile file;
				if (file.EncryptOfficeFile(sFile, sFile, sPassword))
				{
					file.WriteAdditional(sFile, L"DocumentID", U_TO_UTF8(sDocinfo));
					isCrypt = true;
				}
			}
			else
			{
				ECMACryptFile file;
				isCrypt = file.WriteAdditional(sFile, L"DocumentID", U_TO_UTF8(sDocinfo));
			}

			if (!bIsCopy)
				retval = CefV8Value::CreateBool(isCrypt);
			else
				retval = CefV8Value::CreateString(sTmpFile);
			return true;
		}
		else if (name == "isFileSupportCloudCrypt")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			bool isSupportCrypt = false;

			bool isUseCrypto = false;
			if (arguments.size() >= 2)
				isUseCrypto = arguments[1]->GetBoolValue();

			COfficeFileFormatChecker oChecker;
			if (oChecker.isOfficeFile(sFile))
			{
				switch (oChecker.nFileType)
				{
					case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:
					case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:
					case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:
					case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF:
					case AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM:
					{
						isSupportCrypt = true;
						break;
					}
					case AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO:
					{
						isSupportCrypt = isUseCrypto;
						break;
					}
					default:
						break;
				}
			}
			retval = CefV8Value::CreateBool(isSupportCrypt);
			return true;
		}
		else if (name == "isFileCrypt")
		{
			std::wstring sFile = arguments[0]->GetStringValue().ToWString();
			bool isCrypt = false;
			COfficeFileFormatChecker oChecker;
			if (oChecker.isOfficeFile(sFile))
			{
				isCrypt = (AVS_OFFICESTUDIO_FILE_OTHER_MS_OFFCRYPTO == oChecker.nFileType) ? true : false;
			}
			retval = CefV8Value::CreateBool(isCrypt);
			return true;
		}
		else if (name == "Crypto_GetLocalImageBase64")
		{
			// из локальной ссылки делаем base64!!! (для копирования из зашифрованного файла)
			std::wstring sInput = arguments[0]->GetStringValue().ToWString();

			if (IsNeedDownload(sInput))
			{
				retval = CefV8Value::CreateString(sInput);
				return true;
			}
			if (0 == sInput.find(L"data:"))
			{
				retval = CefV8Value::CreateString(sInput);
				return true;
			}

			std::wstring sOutput = NSFile::GetFileName(sInput);
			bool isExist = false;
			if (NSFile::CFileBinary::Exists(m_sCryptDocumentFolder + L"/media/" + sOutput))
			{
				sOutput = m_sCryptDocumentFolder + L"/media/" + sOutput;
				isExist = true;
			}
			else if (NSFile::CFileBinary::Exists(m_sLocalFileFolderWithoutFile + L"/media/" + sOutput))
			{
				sOutput = m_sLocalFileFolderWithoutFile + L"/media/" + sOutput;
				isExist = true;
			}

			if (isExist)
			{
				CImageFileFormatChecker checker;
				if (checker.isImageFile(sOutput))
				{
					std::string sImageData = GetFileBase64(sOutput);
					switch (checker.eFileType)
					{
					case _CXIMAGE_FORMAT_BMP:
						sImageData = "data:image/bmp;base64," + sImageData;
						break;
					case _CXIMAGE_FORMAT_JPG:
					case _CXIMAGE_FORMAT_JP2:
					case _CXIMAGE_FORMAT_JPC:
						sImageData = "data:image/jpeg;base64," + sImageData;
						break;
					case _CXIMAGE_FORMAT_PNG:
						sImageData = "data:image/png;base64," + sImageData;
						break;
					case _CXIMAGE_FORMAT_GIF:
						sImageData = "data:image/gif;base64," + sImageData;
						break;
					case _CXIMAGE_FORMAT_TIF:
						sImageData = "data:image/tiff;base64," + sImageData;
						break;
					default:
						sImageData = "data:image/png;base64," + sImageData;
						break;
					}

					retval = CefV8Value::CreateString(sImageData);
					return true;
				}
			}

			retval = CefV8Value::CreateString(sInput);
			return true;
		}
		else if (name == "CheckCloudFeatures")
		{
			bool bIsStopPaydDesktopFeatures = false;

			// проверяем облачность...
			if (IsLocalFile(true))
			{
				bIsStopPaydDesktopFeatures = true;
			}

			// проверяем лицензию...
			if (!bIsStopPaydDesktopFeatures)
			{
				int nLicenceType = arguments[0]->GetIntValue();
				if (nLicenceType != 3) // c_oLicenseResult.Success
					bIsStopPaydDesktopFeatures = true;
			}

			// проверяем черный список...
			if (!bIsStopPaydDesktopFeatures)
			{
				// check on blacklist
				std::string sMainUrl = "";
				CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
				if (browser && browser->GetMainFrame())
					sMainUrl = browser->GetMainFrame()->GetURL().ToString();

				bool bIsInBlackList = false;
				for (std::vector<std::string>::const_iterator iter = m_arCloudFeaturesBlackList.begin(); iter != m_arCloudFeaturesBlackList.end(); iter++)
				{
					std::string::size_type pos = sMainUrl.find(*iter);
					if (pos != std::string::npos && pos < 10)
					{
						bIsInBlackList = true;
						break;
					}
				}

				if (bIsInBlackList)
					bIsStopPaydDesktopFeatures = true;
			}

			if (!bIsStopPaydDesktopFeatures)
			{
				CSdkjsAddons oAddonsChecker(m_sUserPlugins);
				bool bIsChanged = false;
				m_sEditorsCloudFeatures = oAddonsChecker.CheckCloud(CefV8Context::GetCurrentContext(), bIsChanged);

				if (bIsChanged)
				{
					CefRefPtr<CefV8Value> retval;
					CefRefPtr<CefV8Exception> exception;
					bool bValid = CefV8Context::GetCurrentContext()->Eval("window.AscDesktopEditor.CallInAllWindows(\
\"function(){ window.onfeaturesavailable && window.onfeaturesavailable(" + oAddonsChecker.GetFeaturesJSArray() + "); }\"\
);",
			#ifndef CEF_2623
				"", 0,
			#endif
			retval, exception);
				}
			}

			m_bEditorsCloudFeaturesCheck = true;

			if (m_sEditorsCloudFeatures.empty())
			{
				// этот метод может вызваться ПОСЛЕ вызова GetInstallPlugins (если права пришли позже sdk-all)
				// и может быть нудно застопить зря запущенный плагин

				std::string sCloudCryptoId = CAscRendererProcessParams::getInstance().GetProperty("cryptoEngineId");

				if (!sCloudCryptoId.empty())
				{
					CefRefPtr<CefV8Value> retval;
					CefRefPtr<CefV8Exception> exception;
					bool bValid = CefV8Context::GetCurrentContext()->Eval("(function(){var _editor = window.Asc.editor ? window.Asc.editor : window.editor; _editor && _editor.asc_pluginStop && _editor.asc_pluginStop(\"" + sCloudCryptoId + "\");})();",
			#ifndef CEF_2623
				"", 0,
			#endif
			retval, exception);
				}
			}

			return true;
		}
		else if (name == "GetLocalFeatures")
		{
			CSdkjsAddons oAddonsChecker(m_sUserPlugins);
			std::vector<std::string> arrFeatures = oAddonsChecker.GetFeaturesArray();
			int nCount = (int)arrFeatures.size();
			retval = CefV8Value::CreateArray(nCount);
			int nCurrent = 0;
			for (std::vector<std::string>::iterator i = arrFeatures.begin(); i != arrFeatures.end(); i++)
				retval->SetValue(nCurrent++, CefV8Value::CreateString(*i));
			return true;
		}
		else if (name == "cloudCryptoCommand")
		{
			// empty method
			retval = CefV8Value::CreateString("empty");
			return true;
		}
		else if (name == "GetSupportedScaleValues")
		{
			#define SCALES_COUNT 13
			const double scales[SCALES_COUNT] = {1, 1.25, 1.5, 1.75, 2, 2.25, 2.5, 2.75, 3, 3.5, 4, 4.5, 5};
			retval = CefV8Value::CreateArray(SCALES_COUNT);
			for (int i = 0; i < SCALES_COUNT; ++i)
				retval->SetValue(i, CefV8Value::CreateDouble(scales[i]));

			return true;
		}
		else if (name == "GetFontThumbnailHeight")
		{
			retval = CefV8Value::CreateInt(28);
			return true;
		}
		else if (name == "GetOpenedFile")
		{
#ifndef CEF_V8_SUPPORT_TYPED_ARRAYS
			retval = CefV8Value::CreateUndefined();
#else
			std::string sPath = arguments[0]->GetStringValue().ToString();
			std::wstring sFilePath = NSCommon::GetFilePathFromBase64(sPath);

			if (g_pLocalResolver->CheckNoFont(sFilePath))
			{
				BYTE* pData = NULL;
				DWORD dwFileLen = 0;
				NSFile::CFileBinary::ReadAllBytes(sFilePath, &pData, dwFileLen);

				if (0 != dwFileLen)
					retval = CefV8Value::CreateArrayBuffer((void*)pData, (size_t)dwFileLen, new CAscCefV8ArrayBufferReleaseCallback());
				else
					retval = CefV8Value::CreateUndefined();
			}
			else
			{
				retval = CefV8Value::CreateUndefined();
			}
#endif
			return true;
		}
		else if (name == "GetCurrentWindowInfo")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("get_current_window_info");
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "openTemplate")
		{
			if (arguments.empty())
				return true;

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("open_template");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());

			if (arguments.size() > 1)
				message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_convertFile")
		{
			if (arguments.size() < 2)
				return true;

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("convert_file");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetInt(1, arguments[1]->GetIntValue());
			NSArgumentList::SetInt64(message->GetArgumentList(), 2, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "_onConvertFile")
		{
			std::wstring sFolder = arguments[0]->GetStringValue().ToWString();

			if (sFolder.empty())
			{
				retval = CefV8Value::CreateUndefined();
				return true;
			}

#ifdef CEF_2623
			retval = CefV8Value::CreateObject(NULL);
#else
			retval = CefV8Value::CreateObject(nullptr, nullptr);
#endif

			CefRefPtr<CefV8Handler> handler = new CLocalFileConvertV8Handler(sFolder);
			retval->SetValue("get", CefV8Value::CreateFunction("get", handler), V8_PROPERTY_ATTRIBUTE_NONE);
			retval->SetValue("close", CefV8Value::CreateFunction("close", handler), V8_PROPERTY_ATTRIBUTE_NONE);

			return true;
		}
		else if (name == "SetPdfCloudPrintFileInfo")
		{
			std::string sBase64File = arguments[0]->GetStringValue().ToString();

			BYTE* pData = NULL;
			int nDataLen = 0;
			NSFile::CBase64Converter::Decode(sBase64File.c_str(), (int)sBase64File.length(), pData, nDataLen);

			std::wstring sDirTmp = m_sAppTmpFolder;
			if (sDirTmp.empty())
				sDirTmp = NSFile::CFileBinary::GetTempPath();

			std::wstring sFileTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(sDirTmp, L"IMG");
			if (NSFile::CFileBinary::Exists(sFileTmp))
				NSFile::CFileBinary::Remove(sFileTmp);

			NSFile::CFileBinary oFile;
			if (oFile.CreateFile(sFileTmp))
			{
				oFile.WriteFile(pData, (DWORD)nDataLen);
				oFile.CloseFile();

				m_sCloudNativePrintFile = sFileTmp;
			}

			RELEASEARRAYOBJECTS(pData);
			return true;
		}
		else if (name == "IsCachedPdfCloudPrintFileInfo")
		{
			retval = CefV8Value::CreateBool(!m_sCloudNativePrintFile.empty());
			return true;
		}
		else if (name == "sendSimpleRequest")
		{
			if (2 != arguments.size())
				return true;

			int nCounter = arguments[0]->GetIntValue();
			std::wstring sUrl = L"";

			if (arguments[1]->GetValue("url") && arguments[1]->GetValue("url")->IsString())
				sUrl = arguments[1]->GetValue("url")->GetStringValue().ToWString();

			if (nCounter < 0 || sUrl.empty())
				return true;

			std::string sMethod = "GET";
			if (arguments[1]->GetValue("method") && arguments[1]->GetValue("method")->IsString())
				sMethod = arguments[1]->GetValue("method")->GetStringValue().ToString();

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("send_simple_request");

			NSArgumentList::SetInt64(message->GetArgumentList(), 0, CefV8Context::GetCurrentContext()->GetFrame()->GetIdentifier());
			message->GetArgumentList()->SetInt(1, nCounter);
			message->GetArgumentList()->SetString(2, sUrl);
			message->GetArgumentList()->SetString(3, sMethod);

			CefRefPtr<CefV8Value> headers = arguments[1]->GetValue("headers");
			if (headers && headers->IsObject())
			{
				std::vector<CefString> arKeys;
				int nArgIndex = 4;
				if (headers->GetKeys(arKeys))
				{
					for (int i = 0, len = arKeys.size(); i < len; ++i)
					{
						message->GetArgumentList()->SetString(nArgIndex++, arKeys[i]);
						message->GetArgumentList()->SetString(nArgIndex++, headers->GetValue(arKeys[i])->GetStringValue());
					}
				}
			}

			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "isSupportMacroses")
		{
			retval = CefV8Value::CreateBool(m_bIsMacrosesSupport);
			return true;
		}
		else if (name == "isSupportPlugins")
		{
			retval = CefV8Value::CreateBool(m_bIsPluginsSupport);
			return true;
		}
		else if (name == "setPortalsList")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("set_portals_list");
			int nCount = arguments[0]->GetArrayLength();
			for (int i = 0; i < nCount; ++i)
				message->GetArgumentList()->SetString(i, arguments[0]->GetValue(i)->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "localSaveToDrawingFormat")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("local_save_to_drawing");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
			message->GetArgumentList()->SetString(2, CefV8Context::GetCurrentContext()->GetBrowser()->GetFocusedFrame()->GetURL());
			message->GetArgumentList()->SetString(3, arguments[2]->GetStringValue());
			message->GetArgumentList()->SetString(4, arguments[3]->GetStringValue());
			message->GetArgumentList()->SetInt(5, arguments[4]->GetIntValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}
		else if (name == "emulateCloudPrinting")
		{
			m_bIsPrinting = arguments[0]->GetBoolValue();
			return true;
		}
		else if (name == "isSupportNetworkFunctionality")
		{
#ifdef OLD_MACOS_SYSTEM
			retval = CefV8Value::CreateBool(false);
#else
			retval = CefV8Value::CreateBool(true);
#endif
			return true;
		}
		else if (name == "startExternalConvertation")
		{
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("external_convertation");
			message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
			message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
			SEND_MESSAGE_TO_BROWSER_PROCESS(message);
			return true;
		}

		// Function does not exist.
		return false;
	}

	void RemoveChanges(const int& nDeleteIndex)
	{
		int nNaturalIndex = m_nOpenChangesIndex + nDeleteIndex;

		// на каждое изменение две кавычки)
		nNaturalIndex <<= 1;

		// not cool realize
		BYTE* pData = NULL;
		DWORD dwSize = 0;
		NSFile::CFileBinary::ReadAllBytes(m_sLocalFileChanges, &pData, dwSize);
		int nCounter = 0;

		int nSize = (int)dwSize;
		int nIndex = -1;
		for (int i = 0; i < nSize; i++)
		{
			if ('\"' == pData[i])
			{
				if (nCounter == nNaturalIndex)
				{
					nIndex = i;
					break;
				}
				++nCounter;
			}
		}
		RELEASEARRAYOBJECTS(pData);

		if (-1 != nIndex)
		{
			NSFile::CFileBinary::Truncate(m_sLocalFileChanges, nIndex);
		}
	}

	bool IsNeedDownload(const std::wstring& FilePath)
	{
		std::wstring sFile = (FilePath.length() > 10) ? FilePath.substr(0, 10) : FilePath;

		if (std::wstring::npos != sFile.find(L"www."))
			return true;
		if (std::wstring::npos != sFile.find(L"http://"))
			return true;
		if (std::wstring::npos != sFile.find(L"ftp://"))
			return true;
		if (std::wstring::npos != sFile.find(L"https://"))
			return true;

		return false;
	}

	std::wstring GetLocalImageUrl(const std::wstring& sUrl)
	{
		std::wstring sUrlFile = sUrl;
		if (sUrlFile.find(L"file://") == 0)
		{
			sUrlFile = sUrlFile.substr(7);

			// MS Word copy image with url "file://localhost/..." on mac
			if (sUrlFile.find(L"localhost") == 0)
				sUrlFile = sUrlFile.substr(9);

			NSStringUtils::string_replace(sUrlFile, L"%20", L" ");

			if (!NSFile::CFileBinary::Exists(sUrlFile))
				sUrlFile = sUrlFile.substr(1);
		}

		if (NSFile::CFileBinary::Exists(sUrlFile))
		{
			if (!g_pLocalResolver->Check(sUrlFile))
				return L"error";

			BYTE* pLocalFileData = NULL;
			DWORD dwLocalFileSize = 0;
			unsigned int nCRC32 = 0;
			if (NSFile::CFileBinary::ReadAllBytes(sUrlFile, &pLocalFileData, dwLocalFileSize))
			{
				nCRC32 = m_oCRC32.Calc(pLocalFileData, (unsigned int)dwLocalFileSize);
				RELEASEARRAYOBJECTS(pLocalFileData);

				std::map<unsigned int, std::wstring>::iterator _findCRC32 = m_mapLocalAddImagesCRC.find(nCRC32);

				if (_findCRC32 != m_mapLocalAddImagesCRC.end())
					return _findCRC32->second;
			}

			return GetLocalImageUrlLocal(sUrlFile, sUrl, nCRC32);
		}

		std::map<std::wstring, std::wstring>::iterator _find = m_mapLocalAddImages.find(sUrl);
		if (_find != m_mapLocalAddImages.end())
			return _find->second;

		if (IsNeedDownload(sUrl))
		{
			std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
			if (NSFile::CFileBinary::Exists(sTmpFile))
				NSFile::CFileBinary::Remove(sTmpFile);

			CFileDownloaderWrapper oDownloader(sUrl, sTmpFile);
			oDownloader.DownloadSync();

			std::wstring sRet = GetLocalImageUrlLocal(sTmpFile, sUrl);

			if (NSFile::CFileBinary::Exists(sTmpFile))
				NSFile::CFileBinary::Remove(sTmpFile);

			return sRet;
		}
		if (0 == sUrl.find(L"data:"))
		{
			std::wstring::size_type nBase64Start = sUrl.find(L"base64,");
			if (nBase64Start != std::wstring::npos)
			{
				int nStartIndex = (int)nBase64Start + 7;
				int nCount = (int)sUrl.length() - nStartIndex;
				char* pData = new char[nCount];
				const wchar_t* pDataSrc = sUrl.c_str();
				for (int i = 0; i < nCount; ++i)
					pData[i] = (char)pDataSrc[i + nStartIndex];

				BYTE* pDataDecode = NULL;
				int nLenDecode = 0;
				NSFile::CBase64Converter::Decode(pData, nCount, pDataDecode, nLenDecode);

				RELEASEARRAYOBJECTS(pData);

				std::wstring sTmpFile = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"IMG");
				if (NSFile::CFileBinary::Exists(sTmpFile))
					NSFile::CFileBinary::Remove(sTmpFile);

				NSFile::CFileBinary oFile;
				if (oFile.CreateFileW(sTmpFile))
				{
					oFile.WriteFile(pDataDecode, (DWORD)nLenDecode);
					oFile.CloseFile();
				}

				RELEASEARRAYOBJECTS(pDataDecode);

				std::wstring sRet = GetLocalImageUrlLocal(sTmpFile, sUrl);

				if (NSFile::CFileBinary::Exists(sTmpFile))
					NSFile::CFileBinary::Remove(sTmpFile);

				return sRet;
			}
		}
		if (sUrl.find(L"image") == 0 || sUrl.find(L"display") == 0)
			return sUrl;
		return L"error";
	}

	bool CheckAnotherLocalFile(const std::wstring& sUrl)
	{
		// проверяем - может картинка из соседнего файла?
		std::wstring sLocalFile = sUrl;
		if (sLocalFile.find(L"file://") == 0)
		{
#ifdef _WIN32
			sLocalFile = sLocalFile.substr(8);
#else
			sLocalFile = sLocalFile.substr(7);
#endif
		}

		std::wstring sLocaDirRecover = NSFile::GetDirectoryName(m_sLocalFileFolderWithoutFile);
		NSStringUtils::string_replace(sLocaDirRecover, L"\\", L"/");

		NSStringUtils::string_replace(sLocalFile, L"\\", L"/");

		if (0 != sLocalFile.find(sLocaDirRecover))
			return false;

		return true;
	}

	std::wstring GetLocalImageUrlLocal(const std::wstring& sUrl, const std::wstring& sUrlMap, unsigned int nCRC32 = 0)
	{
		std::wstring sUrlTmp = sUrl;
		CImageFileFormatChecker oChecker;
		if (!oChecker.isImageFile(sUrlTmp))
			return L"error";

		if (oChecker.eFileType == _CXIMAGE_FORMAT_PNG)
		{
			std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".png";
			NSFile::CFileBinary::Copy(sUrl, m_sLocalFileFolderWithoutFile + L"/media/" + sRet);
			m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
			if (0 != nCRC32)
				m_mapLocalAddImagesCRC.insert(std::pair<unsigned int, std::wstring>(nCRC32, sRet));
			return sRet;
		}
		if (oChecker.eFileType == _CXIMAGE_FORMAT_JPG)
		{
			std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".jpg";
			NSFile::CFileBinary::Copy(sUrl, m_sLocalFileFolderWithoutFile + L"/media/" + sRet);
			m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
			if (0 != nCRC32)
				m_mapLocalAddImagesCRC.insert(std::pair<unsigned int, std::wstring>(nCRC32, sRet));
			return sRet;
		}

		CBgraFrame oFrame;
		if (oFrame.OpenFile(sUrl))
		{
			std::wstring sRet = L"image" + std::to_wstring(m_nLocalImagesNextIndex++) + L".png";
			oFrame.SaveFile(m_sLocalFileFolderWithoutFile + L"/media/" + sRet, _CXIMAGE_FORMAT_PNG);
			m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sRet));
			if (0 != nCRC32)
				m_mapLocalAddImagesCRC.insert(std::pair<unsigned int, std::wstring>(nCRC32, sRet));
			return sRet;
		}

		if (NULL == m_pLocalApplicationFonts)
		{
			m_pLocalApplicationFonts = NSFonts::NSApplication::Create();
			m_pLocalApplicationFonts->InitializeFromFolder(m_sFontsData);
		}

		MetaFile::IMetaFile* pMetafile = MetaFile::Create(m_pLocalApplicationFonts);
		pMetafile->LoadFromFile(sUrl.c_str());

		if (pMetafile->GetType() == MetaFile::c_lMetaEmf || pMetafile->GetType() == MetaFile::c_lMetaWmf)
		{
			std::wstring sMeta = L"";
			std::wstring sSvg = L"";
			std::wstring sIndex = std::to_wstring(m_nLocalImagesNextIndex++);
			std::wstring sOutDir = m_sLocalFileFolderWithoutFile + L"/media/";

			// копируем метафайл и генерируем путь для svg
			if (pMetafile->GetType() == MetaFile::c_lMetaWmf)
			{
				sMeta = L"display1image" + sIndex + L".wmf";
				sSvg  = L"display1image" + sIndex + L".svg";
				NSFile::CFileBinary::Copy(sUrl, sOutDir + sMeta);
			}
			else
			{
				sMeta = L"display2image" + sIndex + L".emf";
				sSvg  = L"display2image" + sIndex + L".svg";
				NSFile::CFileBinary::Copy(sUrl, sOutDir + sMeta);
			}

			bool bIsLocalAnother = CheckAnotherLocalFile(sUrl);
			if (bIsLocalAnother)
			{
				std::wstring sUrlFileName = NSFile::GetFileName(sUrl);
				if (0 == sUrlFileName.find(L"display"))
				{
					std::wstring::size_type nLen = sUrlFileName.length();
					if (nLen > 4 && '.' == sUrlFileName[nLen - 4])
					{
						std::wstring sSvgSrc = sUrl.substr(0, nLen - 3) + L"svg";
						if (NSFile::CFileBinary::Exists(sSvgSrc))
						{
							NSFile::CFileBinary::Copy(sSvgSrc, sOutDir + sSvg);
						}
					}
				}
			}

			// не из соседнего файла или нет свг - надо ее сделать!
			if (!NSFile::CFileBinary::Exists(sOutDir + sSvg))
			{
				std::wstring sInternalSvg = pMetafile->ConvertToSvg();
				if (!sInternalSvg.empty())
				{
					NSFile::CFileBinary::SaveToFile(sOutDir + sSvg, sInternalSvg);
				}
				else
				{
					double x = 0, y = 0, w = 0, h = 0;
					pMetafile->GetBounds(&x, &y, &w, &h);

					double _max = (w >= h) ? w : h;
					double dKoef = 100000.0 / _max;

					int WW = (int)(dKoef * w + 0.5);
					int HH = (int)(dKoef * h + 0.5);

					// TODO: заменить на новую конвертацию
					NSHtmlRenderer::CASCSVGWriter oWriterSVG(false);
					oWriterSVG.SetFontManager(m_pLocalApplicationFonts->GenerateFontManager());
					oWriterSVG.put_Width(WW);
					oWriterSVG.put_Height(HH);
					pMetafile->DrawOnRenderer(&oWriterSVG, 0, 0, WW, HH);

					oWriterSVG.SaveFile(sOutDir + sSvg);
				}
			}

			m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sSvg));
			if (0 != nCRC32)
				m_mapLocalAddImagesCRC.insert(std::pair<unsigned int, std::wstring>(nCRC32, sSvg));

			RELEASEINTERFACE(pMetafile);
			return sSvg;
		}
		if (pMetafile->GetType() == MetaFile::c_lMetaSvg || pMetafile->GetType() == MetaFile::c_lMetaSvm)
		{
			std::wstring sMeta = L"";
			std::wstring sSvg = L"";
			std::wstring sIndex = std::to_wstring(m_nLocalImagesNextIndex++);
			std::wstring sOutDir = m_sLocalFileFolderWithoutFile + L"/media/";

			// нужно проверить - не лежит ли рядом метафайл
			bool bIsLocalAnother = CheckAnotherLocalFile(sUrl);
			if (bIsLocalAnother)
			{
				std::wstring sUrlFileName = NSFile::GetFileName(sUrl);
				if (0 == sUrlFileName.find(L"display"))
				{
					std::wstring::size_type nLen = sUrlFileName.length();
					if (nLen > 4 && '.' == sUrlFileName[nLen - 4])
					{
						std::wstring sUrlFileNameWithoutExt = sUrl.substr(0, sUrl.length() - 3);
						std::wstring sWmf = sUrlFileNameWithoutExt + L"wmf";
						std::wstring sEmf = sUrlFileNameWithoutExt + L"emf";

						if (NSFile::CFileBinary::Exists(sWmf))
						{
							sMeta = L"display1image" + sIndex + L".wmf";
							sSvg  = L"display1image" + sIndex + L".svg";
							NSFile::CFileBinary::Copy(sWmf, sOutDir + sMeta);
							NSFile::CFileBinary::Copy(sUrl, sOutDir + sSvg);
						}
						else if (NSFile::CFileBinary::Exists(sEmf))
						{
							sMeta = L"display2image" + sIndex + L".emf";
							sSvg  = L"display2image" + sIndex + L".svg";
							NSFile::CFileBinary::Copy(sEmf, sOutDir + sMeta);
							NSFile::CFileBinary::Copy(sUrl, sOutDir + sSvg);
						}
					}
				}
			}

			if (sSvg.empty())
			{
				// метафайла нет. свг пока не поддерживаем - конвертируем в растр
				sSvg = L"image" + sIndex + L".png";

				double x = 0, y = 0, w = 0, h = 0;
				pMetafile->GetBounds(&x, &y, &w, &h);

				double _max = (w >= h) ? w : h;
				double dKoef = 1000.0 / _max;

				int WW = (int)(dKoef * w + 0.5);
				int HH = (int)(dKoef * h + 0.5);

				std::wstring sOutFile = sOutDir + sSvg;
				pMetafile->ConvertToRaster(sOutFile.c_str(), _CXIMAGE_FORMAT_PNG, WW, HH);
			}

			m_mapLocalAddImages.insert(std::pair<std::wstring, std::wstring>(sUrlMap, sSvg));
			if (0 != nCRC32)
				m_mapLocalAddImagesCRC.insert(std::pair<unsigned int, std::wstring>(nCRC32, sSvg));

			RELEASEINTERFACE(pMetafile);
			return sSvg;
		}

		RELEASEINTERFACE(pMetafile);

		return L"error";
	}

	std::wstring GetFullUrl(const std::wstring& sUrl, const std::wstring& sBaseUrl)
	{
		std::wstring sUrlSrc = L"";
		if (IsNeedDownload(sUrl))
		{
			sUrlSrc = sUrl;
		}
		else
		{
			if (0 == sUrl.find(wchar_t('/')))
			{
				// нужно брать корень сайта
				int nPos = sBaseUrl.find(L"//");
				if (nPos != std::wstring::npos)
				{
					nPos = sBaseUrl.find(wchar_t('/'), nPos + 3);
					if (nPos != std::wstring::npos)
					{
						sUrlSrc = sBaseUrl.substr(0, nPos);
						sUrlSrc += sUrl;
					}
				}
				if (sUrlSrc.empty())
				{
					sUrlSrc = sBaseUrl;
					sUrlSrc += (L"/" + sUrl);
				}
			}
			else
			{
				// брать место урла
				int nPos = sBaseUrl.find_last_of(wchar_t('/'));
				if (std::wstring::npos != nPos)
				{
					sUrlSrc = sBaseUrl.substr(0, nPos);
				}
				else
				{
					sUrlSrc = sBaseUrl;
				}
				sUrlSrc += (L"/" + sUrl);
			}
		}
		NSCommon::url_correct(sUrlSrc);
		return sUrlSrc;
	}

	std::wstring GetFullUrl2(const std::wstring& sUrl, const std::wstring& sBaseUrlSrc)
	{
		std::wstring sBaseUrl = sBaseUrlSrc;
		std::wstring::size_type sPosQuest = sBaseUrl.find(L"/index.html?");
		if (sPosQuest != std::wstring::npos)
		{
			sBaseUrl = sBaseUrl.substr(0, sPosQuest + 10);
		}

		return GetFullUrl(sUrl, sBaseUrl);
	}

	bool IsChartEditor()
	{
		if (!CefV8Context::GetCurrentContext())
			return false;

		CefRefPtr<CefFrame> _frame = CefV8Context::GetCurrentContext()->GetFrame();
		if (!_frame)
			return false;

		CefRefPtr<CefFrame> _frameParent = _frame->GetParent();
		if (!_frameParent)
			return false;

		std::string sName = _frameParent->GetName().ToString();
		if (sName != "frameEditor")
			return false;

		return true;
	}

	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(CAscEditorNativeV8Handler);
};

// Класс для создания dnd-событий по типу
// Есть проблема с перетаскиванием теста в инпут портала, возможно, проблема с dataTransfer.effectAllowed, которое read-only
class CAscDragDrop
{
private:
	CefRefPtr<CefProcessMessage> m_pCefMessage;

public:
	CAscDragDrop(CefRefPtr<CefProcessMessage> message)
	{
		m_pCefMessage = message->Copy();
	}

	std::wstring CreateEvent(const std::wstring& type)
	{
		std::wstring sCode = L"";

		if (m_pCefMessage.get() && type.length())
		{
			CefRefPtr<CefListValue> argList = m_pCefMessage->GetArgumentList();
			int nX = m_pCefMessage->GetArgumentList()->GetInt(0);
			int nY = m_pCefMessage->GetArgumentList()->GetInt(1);
			int nCursorX = m_pCefMessage->GetArgumentList()->GetInt(2);
			int nCursorY = m_pCefMessage->GetArgumentList()->GetInt(3);
			std::wstring sText = m_pCefMessage->GetArgumentList()->GetString(4).ToWString();
			std::wstring sHtml = m_pCefMessage->GetArgumentList()->GetString(5).ToWString();

			std::vector<std::wstring> arParts;
			arParts.push_back(sText);
			arParts.push_back(sHtml);
			for (size_t i = 0; i < arParts.size(); i++)
			{
				NSStringUtils::string_replace(arParts[i], L"\\", L"\\\\");
				NSStringUtils::string_replace(arParts[i], L"\"", L"\\\"");
				NSStringUtils::string_replace(arParts[i], L"\r", L"");
				NSStringUtils::string_replace(arParts[i], L"\n", L"");
			}

			// Поиск нужного элемента по координатам и вызов события
			sCode = L"(function(){\
/* Create custom data transfer item */\
function createDataTransferItem(kind, type, data) {\
let item = { kind: kind, type: type, data: data,\
getAsFile: function() { return this.data; },\
getAsString: function(fcall) { if (fcall) { fcall(this.data); } } };\
return item; }\
/* Create custom event */\
function createCustomEvent(type, x, y, c_x, c_y) {\
let event = new Event(type, { bubbles: true, cancelable: true, composed: true });\
event.dataTransfer = { dropEffect: \"none\", effectAllowed: \"all\", files: [], items: [], types: [], data: {},\
setData: function(_type, _value) { this.effectAllowed = \"copyMove\"; this.data[_type] = _value; this.types.push(_type);\
let dtItem = createDataTransferItem(\"string\", _type, _value); this.items.push(dtItem); },\
getData: function(_type) { console.log('getData()'); console.log(_type); return this.data[_type]; } };\
event.x = event.pageX = event.clientX = x; event.y = event.pageY = event.clientY = y; event.screenX = c_x; event.screenY = c_y;\
/*event.view = window*/;\
return event; }\
/* Add file to dataTransfer */\
function addFileToDataTransfer(ev, dataBase64, fileName) {\
let byteCharacters = atob(dataBase64); let byteNumbers = new Array(byteCharacters.length);\
for (let i = 0; i < byteCharacters.length; i++) { byteNumbers[i] = byteCharacters.charCodeAt(i); }\
let byteArray = new Uint8Array(byteNumbers); let oFile = new File([byteArray], fileName); ev.dataTransfer.files.push(oFile);\
let dtItem = createDataTransferItem(\"file\", \"\", oFile); ev.dataTransfer.items.push(dtItem);}\
/* Code */\
let event = createCustomEvent(\"" + type + L"\"," + std::to_wstring(nX) + L"," + std::to_wstring(nY) + L"," + std::to_wstring(nCursorX) + L"," + std::to_wstring(nCursorY) + L");";

			if (arParts[0].length())
				sCode += L"event.dataTransfer.setData(\"text/plain\", \"" + arParts[0] + L"\");";
			if (arParts[1].length())
				sCode += L"event.dataTransfer.setData(\"text/html\", \"" + arParts[1] + L"\");";

			// Читаем файлы и добавляем в dataTransfer
			std::wstring sDataTransferFiles = L"";
			if (argList->GetSize() > 6)
			{
				sDataTransferFiles = L"";
				for (uint i = 0; i < argList->GetSize() - 6; i++)
				{
					std::wstring sFilePath = m_pCefMessage->GetArgumentList()->GetString(6 + i);

					int nSize = 0;
					std::wstring sFileName = NSFile::GetFileName(sFilePath);
					std::string sImageBase64 = GetFileBase64(sFilePath, &nSize);

					if (sImageBase64.length())
						sDataTransferFiles += L"addFileToDataTransfer(event, \"" + UTF8_TO_U(sImageBase64) + L"\", \"" + sFileName + L"\");";
				}
			}

			sCode += sDataTransferFiles + L"let targetElem = document.elementFromPoint(" + std::to_wstring(nX) + L", " + std::to_wstring(nY) + L");\
if (targetElem) { targetElem.dispatchEvent(event);\
if (targetElem.nodeName === \"INPUT\") { targetElem.value = event.dataTransfer.getData(\"text/plain\"); } }\
console.log(targetElem); console.log(\"" + type + L"\"); console.log(event); })();";
		}

		return sCode;
	}
};

class ClientRenderDelegate : public client::ClientAppRenderer::Delegate {
 public:
  ClientRenderDelegate()
	: last_node_is_editable_(false)
  {
	m_pAdditional = Create_ApplicationRendererAdditional();
	sync_command_check = false;
	m_bIsNativeViewer = false;
  }

  virtual void OnWebKitInitialized(CefRefPtr<client::ClientAppRenderer> app) OVERRIDE {
	// Create the renderer-side router for query handling.
	CefMessageRouterConfig config;
	message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  virtual void OnContextCreated(CefRefPtr<client::ClientAppRenderer> app,
								CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								CefRefPtr<CefV8Context> context) OVERRIDE {
	message_router_->OnContextCreated(browser,  frame, context);

	// add AscEditorNative
	CefRefPtr<CefV8Value> object = context->GetGlobal();

#ifdef CEF_2623
	CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(NULL);
#else
	CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);
#endif

	std::wstring sMainUrl = L"";
	if (browser && browser->GetMainFrame())
		sMainUrl = browser->GetMainFrame()->GetURL().ToWString();

	CAscEditorNativeV8Handler* pWrapper = new CAscEditorNativeV8Handler(sMainUrl);
	pWrapper->sync_command_check = &sync_command_check;

	CefRefPtr<CefV8Handler> handler = pWrapper;

	#define EXTEND_METHODS_COUNT 177
	const char* methods[EXTEND_METHODS_COUNT] = {
		"Copy",
		"Paste",
		"Cut",
		"LoadJS",
		"LoadFontBase64",
		"getFontsSprite",
		"SetEditorId",
		"GetEditorId",
		"SpellCheck",
		"_CreateEditorApi",
		"ConsoleLog",
		"CheckCloudFeatures",

		"setCookie",
		"setAuth",
		"getCookiePresent",
		"getAuth",
		"Logout",

		"onDocumentModifiedChanged",
		"onDocumentContentReady",

		"GetImageBase64",

		"SetDocumentName",

		"OnSave",

		"js_message",

		"CheckNeedWheel",
		"SetFullscreen",

		"Print_Start",
		"Print_Page",
		"Print_End",
		"Print",
		"IsSupportNativePrint",

		"LocalStartOpen",
		"LocalFileOpen",
		"LocalFileRecoverFolder",

		"LocalFileRecents",
		"LocalFileOpenRecent",
		"LocalFileRemoveRecent",
		"GetLocalFeatures",

		"LocalFileRecovers",
		"LocalFileOpenRecover",
		"LocalFileRemoveRecover",

		"LocalFileSaveChanges",

		"LocalFileCreate",

		"LocalFileGetOpenChangesCount",
		"LocalFileSetOpenChangesCount",
		"LocalFileGetCurrentChangesIndex",

		"LocalFileSave",

		"LocalFileGetSourcePath",
		"LocalFileSetSourcePath",

		"LocalFileGetSaved",
		"LocalFileSetSaved",

		"LocalFileGetImageUrl",
		"LocalFileGetImageUrlFromOpenFileDialog",

		"checkAuth",

		"execCommand",

		"SetDropFiles",
		"ClearDropFiles",
		"IsImageFile",
		"GetDropFiles",
		"DropOfficeFiles",

		"SetAdvancedOptions",

		"LocalFileRemoveAllRecents",
		"LocalFileRemoveAllRecovers",

		"CheckUserId",

		"ApplyAction",

		"InitJSContext",

		"NativeViewerOpen",
		"NativeViewerClose",
		"NativeFunctionTimer",
		"NativeViewerGetPageUrl",
		"NativeViewerGetCompleteTasks",

		"GetInstallPlugins",
		"GetBackupPlugins",

		"IsLocalFile",
		"IsFilePrinting",

		"Sign",
		"ViewCertificate",
		"SelectCertificate",
		"GetDefaultCertificate",
		"_OpenFilenameDialog",

		"SaveQuestion",

		"startReporter",
		"endReporter",
		"sendToReporter",
		"sendFromReporter",

		"IsSignaturesSupport",

		"RemoveSignature",
		"RemoveAllSignatures",

		"SetSupportSign",

		"PluginInstall",
		"PluginUninstall",

		"SetInitFlags",

		"isBlockchainSupport",
		"_sendSystemMessage",
		"_GetHash",
		"_CallInAllWindows",
		"_OpenFileCrypt",
		"buildCryptedStart",
		"buildCryptedEnd",
		"SetCryptDocumentFolder",
		"PreloadCryptoImage",
		"ResaveFile",
		"_DownloadFiles",
		"RemoveFile",
		"GetImageFormat",
		"_SetCryptoMode",
		"GetEncryptedHeader",
		"GetCryptoMode",
		"GetSupportCryptoModes",
		"IsProtectionSupport",
		"_GetAdvancedEncryptedData",
		"_SetAdvancedEncryptedData",
		"GetExternalClouds",
		"IsNativeViewer",
		"CryptoDownloadAs",
		"MediaStart",
		"GetImageOriginalSize",
		"MediaEnd",
		"_AddAudio",
		"_AddVideo",
		"SendByMail",
		"IsLocalFileExist",

		"_CloudCryptoUpload",
		"_CloudCryptoUploadPass",
		"_CloudCryptoUploadSave",
		"CloudCryptUploadEnd",

		"getLocalFileSize",

		"_getMainUrl",
		"_getCurrentUrl",

		"_SaveFilenameDialog",

		"_ImportAdvancedEncryptedData",
		"_ExportAdvancedEncryptedData",

		"CompareDocumentUrl",
		"CompareDocumentFile",

		"MergeDocumentUrl",
		"MergeDocumentFile",

		"IsSupportMedia",

		"SetIsReadOnly",

		"CryptoCloud_GetUserInfo",

		"CryptoAES_Init",
		"CryptoAES_Clean",
		"CryptoAES_Encrypt",
		"CryptoAES_Decrypt",

		"CryproRSA_CreateKeys",
		"CryproRSA_EncryptPublic",
		"CryproRSA_DecryptPrivate",
		"IsCloudCryptoSupport",

		"CryptoGetHash",

		"Property_GetCryptoMode",
		"Property_SetCryptoMode",

		"_cloudCryptoCommandMainFrame",
		"GetFrameId",
		"CallInFrame",

		"initCryptoWorker",
		"getDocumentInfo",
		"setDocumentInfo",
		"isFileSupportCloudCrypt",
		"isFileCrypt",

		"Crypto_GetLocalImageBase64",

		"cloudCryptoCommand",

		"GetSupportedScaleValues",
		"GetFontThumbnailHeight",

		"GetOpenedFile",

		"GetCurrentWindowInfo",

		"openTemplate",

		"_convertFile",
		"_onConvertFile",

		"SetPdfCloudPrintFileInfo",
		"IsCachedPdfCloudPrintFileInfo",

		"sendSimpleRequest",

		"isSupportMacroses",
		"isSupportPlugins",

		"setPortalsList",
		"localSaveToDrawingFormat",
		"emulateCloudPrinting",

		"isSupportNetworkFunctionality",
		"startExternalConvertation",

		NULL
	};

	ExtendObject(obj, handler, methods);

	object->SetValue("AscDesktopEditor", obj, V8_PROPERTY_ATTRIBUTE_NONE);

	CefRefPtr<CefFrame> curFrame = context->GetFrame();
	if (curFrame)
	{
#if 0
		std::string sDisableSW = "\
navigator.serviceWorker.register2 = navigator.serviceWorker.register;\n\
navigator.serviceWorker.register = function() { \n\
var path = arguments[0]; var pathEnd = path.lastIndexOf('/');\n\
arguments[0] = (pathEnd === -1) ? (\"ascdesktop_\" + path) : (path.substr(0, pathEnd) + \"/ascdesktop_\" + path.substr(pathEnd + 1));;\n\
return navigator.serviceWorker.register2.apply(this, arguments);\n\
};\n\"";
#endif

		curFrame->ExecuteJavaScript("\
window.addEventListener(\"DOMContentLoaded\", function(){\n\
//if (window && window.AscCommon && window.AscCommon.checkDeviceScale) return;\n\
if (window && window.Asc && window.Asc.plugin) return;\n\
var _style = document.createElement(\"style\");\n\
_style.type = \"text/css\";\n\
_style.innerHTML = \"\
::-webkit-scrollbar { background: transparent; width: 16px; height: 16px; } \
::-webkit-scrollbar-button { width: 5px; height:5px; } \
::-webkit-scrollbar-track {	background:#F5F5F5; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-thumb { background:#BFBFBF; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-thumb:hover { background:#A7A7A7; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
::-webkit-scrollbar-corner { background:inherit; }\
.webkit-scrollbar::-webkit-scrollbar { background: transparent; width: 16px; height: 16px; } \
.webkit-scrollbar::-webkit-scrollbar-button { width: 5px; height:5px; } \
.webkit-scrollbar::-webkit-scrollbar-track {	background:#F5F5F5; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
.webkit-scrollbar::-webkit-scrollbar-thumb { background:#BFBFBF; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
.webkit-scrollbar::-webkit-scrollbar-thumb:hover { background:#A7A7A7; border: 4px solid transparent; border-radius:7px; background-clip: content-box; } \
.webkit-scrollbar::-webkit-scrollbar-corner { background:inherit; }\";\n\
document.getElementsByTagName(\"head\")[0].appendChild(_style);\n\
}, false);\n\
\n\
window.AscDesktopEditor.InitJSContext();", curFrame->GetURL(), 0);

#ifndef CEF_VERSION_ABOVE_86
		curFrame->ExecuteJavaScript("\
if (!String.prototype.replaceAll) {\
String.prototype.replaceAll = function (str, newStr) {\
if (Object.prototype.toString.call(str).toLowerCase() === '[object regexp]')\
	return this.replace(str, newStr);\
return this.split(str).join(newStr);\
};\
}", curFrame->GetURL(), 0);
#endif

		std::string sVar = pWrapper->m_sRendererProcessVariable.empty() ? "{}" : U_TO_UTF8(pWrapper->m_sRendererProcessVariable);
		NSStringUtils::string_replaceA(sVar, "\n", "");
		curFrame->ExecuteJavaScript("window.RendererProcessVariable = " + sVar + ";", curFrame->GetURL(), 0);
	}

	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("on_js_context_created");
#ifndef MESSAGE_IN_BROWSER
	curFrame->SendProcessMessage(PID_BROWSER, message);
#else
	browser->SendProcessMessage(PID_BROWSER, message);
#endif
  }

  virtual void OnContextReleased(CefRefPtr<client::ClientAppRenderer> app,
								 CefRefPtr<CefBrowser> browser,
								 CefRefPtr<CefFrame> frame,
								 CefRefPtr<CefV8Context> context) OVERRIDE {
	message_router_->OnContextReleased(browser,  frame, context);
  }

  virtual void OnFocusedNodeChanged(CefRefPtr<client::ClientAppRenderer> app,
									CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefDOMNode> node) OVERRIDE {
	bool is_editable = (node.get() && node->IsEditable());
	if (is_editable != last_node_is_editable_)
	{
	  // Notify the browser of the change in focused element type.
	  last_node_is_editable_ = is_editable;
#if 0
	  CefRefPtr<CefProcessMessage> message =
		  CefProcessMessage::Create(kFocusedNodeChangedMessage);
	  message->GetArgumentList()->SetBool(0, is_editable);
	  browser->SendProcessMessage(PID_BROWSER, message);
#endif
	}
  }

  virtual bool OnProcessMessageReceived(
	  CefRefPtr<client::ClientAppRenderer> app,
	  CefRefPtr<CefBrowser> browser,
#ifndef MESSAGE_IN_BROWSER
	  CefRefPtr<CefFrame> messageFrame,
#endif
	  CefProcessId source_process,
	  CefRefPtr<CefProcessMessage> message) OVERRIDE
{

	std::string sMessageName = message->GetName().ToString();

	if (sMessageName == "keyboard_layout")
	{
		int nKeyboardLayout = message->GetArgumentList()->GetInt(0);
		std::string sLayout = std::to_string(nKeyboardLayout);
		std::string sCode = "window[\"asc_current_keyboard_layout\"] = " + sLayout + ";";

		std::vector<int64> ids;
		browser->GetFrameIdentifiers(ids);

		for (std::vector<int64>::iterator i = ids.begin(); i != ids.end(); i++)
		{
			CefRefPtr<CefFrame> _frame = browser->GetFrame(*i);
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "cef_control_id")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			int nControlId = message->GetArgumentList()->GetInt(0);
			bool isSupportSign = message->GetArgumentList()->GetBool(1);
			int nParams = message->GetArgumentList()->GetInt(2);
			std::string sCode = "window[\"AscDesktopEditor\"][\"SetEditorId\"](" + std::to_string(nControlId) + ");";

			if (isSupportSign)
				sCode += "window[\"AscDesktopEditor\"][\"SetSupportSign\"](true);";
			else
				sCode += "window[\"AscDesktopEditor\"][\"SetSupportSign\"](false);";

			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "on_js_context_created_callback")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			int nParams = message->GetArgumentList()->GetInt(0);
			int nCryptoMode = message->GetArgumentList()->GetInt(1);

			std::string sCode = ("window[\"AscDesktopEditor\"][\"SetInitFlags\"](" + std::to_string(nParams) + ", " + std::to_string(nCryptoMode) + ");");

			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "spell_check_response")
	{
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 1);
		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (_frame)
		{
			std::string sCode = "window[\"asc_nativeOnSpellCheck\"](" + message->GetArgumentList()->GetString(0).ToString() + ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "sync_command_end")
	{
		sync_command_check = false;
		return true;
	}
	else if (sMessageName == "on_is_cookie_present")
	{
		CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
		bool bIsPresent = message->GetArgumentList()->GetBool(0);
		std::string sValue = message->GetArgumentList()->GetString(1).ToString();
		std::string sParam = bIsPresent ? "true" : "false";
		std::string sCode = "if (window[\"on_is_cookie_present\"]) { window[\"on_is_cookie_present\"](" + sParam + ", \"" + sValue + "\"); }";
		_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "on_check_auth")
	{
		CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
		int nCount = message->GetArgumentList()->GetInt(0);

		std::string sObject = "{";
		for (int i = 0; i < nCount; i++)
		{
			std::string sKey = message->GetArgumentList()->GetString(1 + i * 2);
			std::string sValue = message->GetArgumentList()->GetString(2 + i * 2);

			NSStringUtils::string_replaceA(sKey, "\"", "\\\"");
			NSStringUtils::string_replaceA(sValue, "\"", "\\\"");

			sObject += ("\""  + sKey + "\" : \"" + sValue + "\"");
			if (i != (nCount - 1))
				sObject += ",";
		}
		sObject += "}";

		std::string sCode = "if (window[\"on_check_auth\"]) { window[\"on_check_auth\"](" + sObject + "); }";

		_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "on_set_cookie")
	{
		CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
		_frame->ExecuteJavaScript("if (window[\"on_set_cookie\"]) { window[\"on_set_cookie\"](); }", _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "document_save")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
			_frame->ExecuteJavaScript("if (window[\"AscDesktopEditor_Save\"]) { window[\"AscDesktopEditor_Save\"](); }", _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "print")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::string sCode = "(function(){\
if (window.Asc && window.Asc.editor && !window.editor) window.Asc.editor.asc_nativePrint(undefined, undefined);\
else if (window.editor) window.editor.asc_nativePrint(undefined, undefined";

			if (message->GetArgumentList()->GetSize() == 1)
				sCode += (", " + (message->GetArgumentList()->GetString(0).ToString()));

			sCode += "); })();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_load_js")
	{
		int64 frameId = NSArgumentList::GetInt64(message->GetArgumentList(), 2);
		CefRefPtr<CefFrame> _frame = browser->GetFrame(frameId);
		if (_frame)
		{
			std::wstring sFilePath = message->GetArgumentList()->GetString(0).ToWString();
			std::wstring sUrl = message->GetArgumentList()->GetString(1).ToWString();

			std::string sScriptContent;
			if (NSFile::CFileBinary::ReadAllTextUtf8A(sFilePath, sScriptContent))
			{
				_frame->ExecuteJavaScript(sScriptContent, _frame->GetURL(), 0);
			}

			_frame->ExecuteJavaScript("window[\"asc_desktop_on_load_js\"](\"" + U_TO_UTF8(sUrl) + "\");", _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_loadend")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (!_frame)
			_frame = browser->GetMainFrame();

		if (_frame)
		{
			std::wstring sFolder = message->GetArgumentList()->GetString(0).ToWString();
			std::wstring sFileSrc = message->GetArgumentList()->GetString(1).ToWString();
			std::wstring sFolderJS = sFolder;
#ifndef _WIN32
			NSStringUtils::string_replace(sFolderJS, L"\\", L"\\\\");
			NSStringUtils::string_replace(sFolderJS, L"\"", L"\\\"");
			NSStringUtils::string_replace(sFileSrc, L"\\", L"\\\\");
			NSStringUtils::string_replace(sFileSrc, L"\"", L"\\\"");
#endif

			bool bIsSaved = message->GetArgumentList()->GetBool(2);

			std::wstring sSignatures = message->GetArgumentList()->GetString(3).ToWString();
			bool bIsLockedFile = message->GetArgumentList()->GetInt(4);
			NSStringUtils::string_replace(sSignatures, L"\"", L"\\\"");

			if (bIsSaved)
				_frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(true);", _frame->GetURL(), 0);
			else
				_frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(false);", _frame->GetURL(), 0);

			int nFileDataLen = 0;

			std::wstring sBinaryFile = sFolder + L"/Editor.bin";
			if (!NSFile::CFileBinary::Exists(sBinaryFile))
			{
				std::wstring sBinaryFileCrossPlatform = sFolder + L"/" + NSFile::GetFileName(sFileSrc);
				if (NSFile::CFileBinary::Exists(sBinaryFileCrossPlatform))
					sBinaryFile = sBinaryFileCrossPlatform;
			}

#ifdef CEF_V8_SUPPORT_TYPED_ARRAYS
			std::string sFileData = GetFileData2(sBinaryFile, nFileDataLen);
#else
			std::string sFileData = GetFileData(sBinaryFile, nFileDataLen);
#endif

			std::string sCode = "window.AscDesktopEditor.LocalFileRecoverFolder(\"" + U_TO_UTF8(sFolderJS) +
					"\");window.AscDesktopEditor.LocalFileSetSourcePath(\"" + U_TO_UTF8(sFileSrc) + "\");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

			if (NSFile::CFileBinary::Exists(sFolder + L"/changes/changes0.json"))
			{
				std::string sChanges;
				NSFile::CFileBinary::ReadAllTextUtf8A(sFolder + L"/changes/changes0.json", sChanges);
				if (0 < sChanges.length())
					sChanges[sChanges.length() - 1] = ']';

				const char* pDataCheck = (const char*)sChanges.c_str();
				const char* pDataCheckLimit = pDataCheck + sChanges.length();
				int nCounter = 0;
				while (pDataCheck != pDataCheckLimit)
				{
					if (*pDataCheck == '\"')
						++nCounter;
					++pDataCheck;
				}
				nCounter >>= 1;

				sChanges = "window.DesktopOfflineAppDocumentApplyChanges([" +
						sChanges + ");window.AscDesktopEditor.LocalFileSetOpenChangesCount(" +
						std::to_string(nCounter) + ");";
				_frame->ExecuteJavaScript(sChanges, _frame->GetURL(), 0);
			}

			if (bIsLockedFile)
			{
				_frame->ExecuteJavaScript("(function(){var _editor = window[\"editor\"]; if (!_editor && window[\"Asc\"]) _editor = window[\"Asc\"][\"editor\"]; if (_editor && _editor.asc_setLocalRestrictions) _editor.asc_setLocalRestrictions(" + std::to_string(bIsLockedFile) + ", true);})();",
										  _frame->GetURL(), 0);
			}

			sCode = "";
			sCode = "window.DesktopOfflineAppDocumentEndLoad(\"" + U_TO_UTF8(sFolder) + "\", \"" + sFileData + "\", " + std::to_string(nFileDataLen) + ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

			sCode = "window.DesktopOfflineAppDocumentSignatures(\"" + U_TO_UTF8(sSignatures) + "\");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "oncompare_loadend" ||
			 sMessageName == "onmerge_loadend")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (!_frame)
			_frame = browser->GetMainFrame();

		int nError = message->GetArgumentList()->GetInt(0);
		std::wstring sFolder = message->GetArgumentList()->GetString(1).ToWString();

		int nFileDataLen = 0;
		std::string sFileData = GetFileData(sFolder + L"/Editor.bin", nFileDataLen);

		// image_map
		std::vector<std::wstring> files = NSDirectory::GetFiles(sFolder + L"/media");
		std::string sImageMap = "{";
		for (std::vector<std::wstring>::iterator i = files.begin(); i != files.end(); i++)
		{
			std::wstring sFile = *i; NSStringUtils::string_replace(sFile, L"\\", L"/");
			NSStringUtils::string_replace(sFile, L"\"", L"'");
			std::wstring sName = L"media/" + NSFile::GetFileName(sFile);

			sImageMap += "\"";
			sImageMap += U_TO_UTF8(sName);
			sImageMap += "\":\"";
			sImageMap += U_TO_UTF8(sFile);
			sImageMap += "\",";
		}

		if (sImageMap[sImageMap.length() - 1] == ',')
			sImageMap[sImageMap.length() - 1] = '}';
		else
			sImageMap += "}";

		std::string sCodeFunc;
		if (sMessageName == "oncompare_loadend")
			sCodeFunc = "window.onDocumentCompare && window.onDocumentCompare(\"";
		else if (sMessageName == "onmerge_loadend")
			sCodeFunc = "window.onDocumentMerge && window.onDocumentMerge(\"";

		std::string sCode = sCodeFunc + U_TO_UTF8(sFolder) + "\", \"" + sFileData + "\", " + std::to_string(nFileDataLen) + ", " + sImageMap + ");";
		_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "onlocaldocument_onsaveend")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sFileSrc = message->GetArgumentList()->GetString(0).ToString();
			int nIsSaved = message->GetArgumentList()->GetInt(1);

			// 0 - ok
			// 1 - cancel
			// 2 - error

			if (0 == nIsSaved)
				_frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(true);", _frame->GetURL(), 0);
			else
				_frame->ExecuteJavaScript("window.AscDesktopEditor.LocalFileSetSaved(false);", _frame->GetURL(), 0);

			if (!sFileSrc.empty())
			{
#ifndef _WIN32
				NSStringUtils::string_replaceA(sFileSrc, "\\", "\\\\");
				NSStringUtils::string_replaceA(sFileSrc, "\"", "\\\"");
#endif
				std::string sCode = "window.AscDesktopEditor.LocalFileSetSourcePath(\"" + sFileSrc + "\");";
				_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			}

			std::string sCode = "window.DesktopOfflineAppDocumentEndSave(" + std::to_string(nIsSaved);

			std::string sHash = "";
			std::string sPass = "";
			if (4 <= message->GetArgumentList()->GetSize())
			{
				sPass = message->GetArgumentList()->GetString(2).ToString();
				sHash = message->GetArgumentList()->GetString(3).ToString();

				NSStringUtils::string_replaceA(sPass, "\\", "\\\\");
				NSStringUtils::string_replaceA(sPass, "\"", "\\\"");

				if (!sPass.empty() && !sHash.empty())
				{
					sCode += ", \"";
					sCode += sHash;
					sCode += "\", \"";
					sCode += sPass;
					sCode += "\"";
				}
			}

			sCode += ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_sendrecents")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::wstring sJSON = message->GetArgumentList()->GetString(0).ToWString();
			NSStringUtils::string_replace(sJSON, L"\\", L"\\\\");

			std::wstring sCode = L"if (window.onupdaterecents) {window.onupdaterecents(" + sJSON + L");}";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_sendrecovers")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::wstring sJSON = message->GetArgumentList()->GetString(0).ToWString();
			NSStringUtils::string_replace(sJSON, L"\\", L"\\\\");

			std::wstring sCode = L"if (window.onupdaterecovers) {window.onupdaterecovers(" + sJSON + L");}";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_onaddimage")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::wstring sPath = message->GetArgumentList()->GetString(0).ToWString();
			g_pLocalResolver->AddFile(sPath);

			NSStringUtils::string_replace(sPath, L"\\", L"\\\\");

			std::wstring sCode = L"window.DesktopOfflineAppDocumentAddImageEnd(\"" + sPath + L"\");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_ondragenter")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			/*CAscDragDrop oDnd(message);

			std::wstring sCode = oDnd.CreateEvent(L"dragenter");
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

			sCode = oDnd.CreateEvent(L"dragover");
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);*/
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_ondropdata")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			CAscDragDrop oDnd(message);

			std::wstring sCode = oDnd.CreateEvent(L"drop");
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_native_message")
	{
		std::string sCommand = message->GetArgumentList()->GetString(0).ToString();
		std::string sParam = message->GetArgumentList()->GetString(1).ToString();
		std::string sFrameName = message->GetArgumentList()->GetString(2).ToString();

		CefRefPtr<CefFrame> _frame = browser->GetMainFrame();
		if (!sFrameName.empty())
		{
			_frame = browser->GetFrame(sFrameName);
		}

		if (!_frame)
			return true;

		std::string sCode = "if (window.on_native_message) {window.on_native_message(\"" + sCommand + "\", \"" + sParam + "\");}";
		_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);

		return true;
	}
	else if (sMessageName == "on_editor_native_message")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sCommand = message->GetArgumentList()->GetString(0).ToString();
			std::string sParam = message->GetArgumentList()->GetString(1).ToString();

			std::string sCode = "if (window.on_editor_native_message) {window.on_editor_native_message(\"" + sCommand + "\", \"" + sParam + "\");}";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "onlocaldocument_additionalparams")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sParam = std::to_string(message->GetArgumentList()->GetInt(0));
			std::string sCode = "window.asc_initAdvancedOptions(" + sParam;

			if (2 <= message->GetArgumentList()->GetSize())
			{
				std::string sHash = message->GetArgumentList()->GetString(1).ToString();
				sCode += ",\"";
				sCode += sHash;
				sCode += "\"";
			}
			if (3 <= message->GetArgumentList()->GetSize())
			{
				std::string sDocInfo = message->GetArgumentList()->GetString(2).ToString();
				NSStringUtils::string_replaceA(sDocInfo, "\n", "<!--break-->");
				sCode += ",\"";
				sCode += sDocInfo;
				sCode += "\"";
			}

			sCode += ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "native_viewer_onopened")
	{
		m_bIsNativeViewer = true;
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::wstring s1 = message->GetArgumentList()->GetString(0).ToWString();
			std::wstring s2 = message->GetArgumentList()->GetString(1).ToWString();
			std::wstring s3 = message->GetArgumentList()->GetString(2).ToWString();

#ifdef _WIN32
			NSStringUtils::string_replace(s1, L"\\", L"/");
			NSStringUtils::string_replace(s2, L"\\", L"/");
			NSStringUtils::string_replace(s3, L"\\", L"/");
#else
			NSStringUtils::string_replace(s1, L"\\", L"\\\\");
			NSStringUtils::string_replace(s2, L"\\", L"\\\\");
			NSStringUtils::string_replace(s3, L"\\", L"\\\\");
			NSStringUtils::string_replace(s1, L"\"", L"\\\"");
			NSStringUtils::string_replace(s2, L"\"", L"\\\"");
			NSStringUtils::string_replace(s3, L"\"", L"\\\"");
#endif

			std::string sCode = "window.AscDesktopEditor.NativeViewerOpen(\"" + U_TO_UTF8(s1) +
					"\", \"" + U_TO_UTF8(s2) + "\", \"" + U_TO_UTF8(s3) + "\");";

			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "update_install_plugins")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
		{
			std::string sCode = "if (window.UpdateInstallPlugins) window.UpdateInstallPlugins();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_signature_defaultcertificate_ret" ||
			 sMessageName == "on_signature_selectsertificate_ret" ||
			 sMessageName == "on_signature_viewcertificate_ret")
	{
		std::string sParam = message->GetArgumentList()->GetString(0).ToString();

		std::string sCode = "window.OnNativeReturnCallback(\"" + sMessageName + "\", " + sParam + ");";

		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
	}
	else if (sMessageName == "on_open_filename_dialog")
	{
		CefRefPtr<CefFrame> _frame;

		std::string sId = message->GetArgumentList()->GetString(0).ToString();
		if (sId.empty())
			_frame = GetEditorFrame(browser);
		else
		{
			int64 nId = (int64)std::stoll(sId);
			_frame = browser->GetFrame(nId);
		}
		bool bIsMulti = message->GetArgumentList()->GetBool(1);

		std::wstring sParamCallback = L"";
		if (!bIsMulti)
		{
			std::wstring sPath = message->GetArgumentList()->GetString(2).ToWString();
			g_pLocalResolver->AddFile(sPath);
			NSStringUtils::string_replace(sPath, L"\\", L"\\\\");
			NSStringUtils::string_replace(sPath, L"\"", L"\\\"");
			sParamCallback = L"\"" + sPath + L"\"";
		}
		else
		{
			sParamCallback = L"[";

			int nCount = message->GetArgumentList()->GetSize();
			for (int nIndex = 2; nIndex < nCount; nIndex++)
			{
				std::wstring sPath = message->GetArgumentList()->GetString(nIndex).ToWString();
				g_pLocalResolver->AddFile(sPath);
				NSStringUtils::string_replace(sPath, L"\\", L"\\\\");
				NSStringUtils::string_replace(sPath, L"\"", L"\\\"");
				sParamCallback += (L"\"" + sPath + L"\"");

				if (nIndex < (nCount - 1))
					sParamCallback += L",";
			}

			sParamCallback += L"]";
		}

		if (_frame)
		{
			std::wstring sCode = L"(function() { window.on_native_open_filename_dialog(" + sParamCallback + L"); delete window.on_native_open_filename_dialog; })();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_save_filename_dialog")
	{
		CefRefPtr<CefFrame> _frame;

		std::string sId = message->GetArgumentList()->GetString(0).ToString();
		std::wstring sPath = message->GetArgumentList()->GetString(1).ToWString();
		if (sId.empty())
			_frame = GetEditorFrame(browser);
		else
		{
			int64 nId = (int64)std::stoll(sId);
			_frame = browser->GetFrame(nId);
		}

		if (_frame)
		{
			NSStringUtils::string_replace(sPath, L"\\", L"\\\\");

			std::wstring sCode = L"(function() { window.on_native_save_filename_dialog(\"" + sPath + L"\"); delete window.on_native_save_filename_dialog; })();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "on_signature_update_signatures")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::wstring sSignatures = message->GetArgumentList()->GetString(0).ToWString();
			NSStringUtils::string_replace(sSignatures, L"\"", L"\\\"");

			std::string sCode = "window.DesktopOfflineAppDocumentSignatures(\"" + U_TO_UTF8(sSignatures) + "\");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_editor_warning")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sWarning = message->GetArgumentList()->GetString(0).ToString();
			NSStringUtils::string_replaceA(sWarning, "\"", "\\\"");

			std::string sCode = "(function(){var _editor = window.Asc.editor ? window.Asc.editor : window.editor;\
_editor && _editor.local_sendEvent && _editor.local_sendEvent(\"asc_onError\", \"" + sWarning + "\", 0);})();";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_file_save_question")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			bool bValue = message->GetArgumentList()->GetBool(0);
			std::string sCode = "window.DesktopSaveQuestionReturn(";
			sCode += (bValue ? "true" : "false");
			sCode += ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "from_reporter_message")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sParam = message->GetArgumentList()->GetString(0);
			NSStringUtils::string_replaceA(sParam, "\\", "\\\\");
			NSStringUtils::string_replaceA(sParam, "\"", "\\\"");
			_frame->ExecuteJavaScript("window.editor && window.editor.DemonstrationReporterMessages({ data : \"" + sParam + "\" });", _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "to_reporter_message")
	{
		CefRefPtr<CefFrame> _frame = browser->GetMainFrame();

		if (_frame)
		{
			std::string sParam = message->GetArgumentList()->GetString(0);
			NSStringUtils::string_replaceA(sParam, "\\", "\\\\");
			NSStringUtils::string_replaceA(sParam, "\"", "\\\"");
			_frame->ExecuteJavaScript("window.editor && window.editor.DemonstrationToReporterMessages({ data : \"" + sParam + "\" });", _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "file_get_hash_callback")
	{
		int64 frameID = (int64)std::stoll(message->GetArgumentList()->GetString(1).ToString());
		CefRefPtr<CefFrame> _frame = browser->GetFrame(frameID);

		if (_frame)
		{
			std::string sCode = "if (window.AscDesktopEditor.getHashCallback) { window.AscDesktopEditor.getHashCallback(\"";
			sCode += message->GetArgumentList()->GetString(0).ToString();
			sCode += "\",";
			sCode += message->GetArgumentList()->GetString(2).ToString();
			sCode += "); window.AscDesktopEditor.getHashCallback = null; }";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "send_system_message")
	{
		std::string sArg = message->GetArgumentList()->GetString(0);
		NSStringUtils::string_replaceA(sArg, "\\", "\\\\");
		NSStringUtils::string_replaceA(sArg, "\"", "\\\"");

		if (message->GetArgumentList()->GetSize() == 1)
		{
			// main view
			std::vector<int64> identifiers;
			browser->GetFrameIdentifiers(identifiers);

			for (std::vector<int64>::iterator i = identifiers.begin(); i != identifiers.end(); i++)
			{
				int64 k = *i;
				CefRefPtr<CefFrame> frame = browser->GetFrame(k);

				if (frame && (frame->GetName().ToString().find("system_asc") == 0))
				{
					std::string sCode = "\
(function(){\n\
try {\n\
var _arg = JSON.parse(\"" + sArg + "\");\n\
window.AscDesktopEditor.isSendSystemMessage = false;\n\
if (window.onSystemMessage)\n\
{ window.onSystemMessage(_arg); } else if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
{ window.Asc.plugin.onSystemMessage(_arg); }\n\
}\n\
catch (err) {\n\
	if (!window.AscDesktopEditor.isSendSystemMessage) {\n\
		window.AscDesktopEditor.sendSystemMessage(_arg);\n\
	}\n\
}\n\
delete window.AscDesktopEditor.isSendSystemMessage;\n\
})();";

					frame->ExecuteJavaScript(sCode, frame->GetURL(), 0);
					break;
				}
			}
		}
		else
		{
			int64 frameID = (int64)std::stoll(message->GetArgumentList()->GetString(1).ToString());
			CefRefPtr<CefFrame> _frame = browser->GetFrame(frameID);

			if (_frame)
			{
				std::string sCode = "\
(function(){\n\
try {\n\
var _arg = JSON.parse(\"" + sArg + "\");\n\
if (window.onSystemMessage)\n\
{ window.onSystemMessage(_arg); } else if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
{ window.Asc.plugin.onSystemMessage(_arg); }\n\
}\n\
catch (err) {}\n\
})();";
				_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			}

			if (true)
			{
				std::vector<int64> identifiers;
				browser->GetFrameIdentifiers(identifiers);

				for (std::vector<int64>::iterator i = identifiers.begin(); i != identifiers.end(); i++)
				{
					int64 k = *i;
					CefRefPtr<CefFrame> _frameOP = browser->GetFrame(k);

					if (_frameOP && (k != frameID) && (_frameOP->GetName().ToString().find("iframe_asc.{") == 0))
					{
						std::string sCode = "\
(function(){\n\
try {\n\
var _arg = JSON.parse(\"" + sArg + "\");\n\
if (window.Asc && window.Asc.plugin && window.Asc.plugin.onSystemMessage)\n\
{ window.Asc.plugin.onSystemMessage(_arg); } \n\
}\n\
catch (err) {}\n\
})();";

						_frameOP->ExecuteJavaScript(sCode, _frameOP->GetURL(), 0);
						break;
					}
				}
			}
		}
		return true;
	}
	else if (sMessageName == "onload_crypt_document")
	{
		std::wstring sFilePathW = message->GetArgumentList()->GetString(0).ToWString();
		std::string sFilePath = U_TO_UTF8(sFilePathW);

		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::wstring sDirectoryRecover = NSFile::GetDirectoryName(sFilePathW);
			std::string sDirectoryRecoverA = U_TO_UTF8(sDirectoryRecover);

			std::string sCode = ("window.AscDesktopEditor.SetCryptDocumentFolder(\"" + sDirectoryRecoverA + "\");\n\
window.AscDesktopEditor.loadLocalFile(\"" + sFilePath + "\", function(data) {\n\
window.AscDesktopEditor.openFileCryptCallback(data);\n\
window.AscDesktopEditor.openFileCryptCallback = null;\n\
});");

			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "is_need_build_crypted_file")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
			_frame->ExecuteJavaScript("window.asc_IsNeedBuildCryptedFile();", _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "build_crypted_file")
	{
		std::string sFolderName = message->GetArgumentList()->GetString(0);
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
		if (_frame)
			_frame->ExecuteJavaScript("window.buildCryptoFile_Start();", _frame->GetURL(), 0);
		return true;
	}
	else if (sMessageName == "build_crypted_file_end")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sFileSrc = message->GetArgumentList()->GetString(0).ToString();
			int nIsSaved = message->GetArgumentList()->GetInt(1);

			// 0 - ok
			// 1 - cancel
			// 2 - error
			if (!sFileSrc.empty())
			{
				std::string sCode = "window.AscDesktopEditor.LocalFileSetSourcePath(\"" + sFileSrc + "\");";
				_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			}

			std::string sCode = "window.buildCryptoFile_End(\"" + sFileSrc + "\", " + std::to_string(nIsSaved);

			std::string sHash = "";
			std::string sPass = "";
			if (4 <= message->GetArgumentList()->GetSize())
			{
				sPass = message->GetArgumentList()->GetString(2).ToString();
				sHash = message->GetArgumentList()->GetString(3).ToString();

				NSStringUtils::string_replaceA(sPass, "\\", "\\\\");
				NSStringUtils::string_replaceA(sPass, "\"", "\\\"");

				if (!sPass.empty() && !sHash.empty())
				{
					sCode += ", \"";
					sCode += sHash;
					sCode += "\", \"";
					sCode += sPass;
					sCode += "\"";
				}
			}

			sCode += ");";
			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "on_preload_crypto_image")
	{
		CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);

		if (_frame)
		{
			int nType = message->GetArgumentList()->GetInt(0);
			int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 1);
			if (0 != nFrameId)
			{
				CefRefPtr<CefFrame> _frameID = browser->GetFrame(nFrameId);
				if (_frameID)
					_frame = _frameID;
			}

			std::wstring sFileSrc = message->GetArgumentList()->GetString(2).ToWString();

			std::string sUrl = U_TO_UTF8(sFileSrc);
			std::string sUrlNatural = sUrl;
			std::string sData = "";
			if (0 == nType)
			{
				sUrlNatural = message->GetArgumentList()->GetString(3).ToString();
				sUrl = "ascdesktop://fonts/" + sUrl;

				std::string sTest = "";
				NSFile::CFileBinary oFile;
				if (oFile.OpenFile(sFileSrc))
				{
					if (oFile.GetFileSize() > 11)
					{
						BYTE data[11];
						memset(data, 0, 11);
						DWORD dwSize = 0;
						oFile.ReadFile(data, 10, dwSize);
						sTest = std::string((char*)data);
					}
				}
				oFile.CloseFile();

				if (sTest == "ENCRYPTED;")
				{
					NSFile::CFileBinary::ReadAllTextUtf8A(sFileSrc, sData);

					// read extension
					std::string::size_type nPos = sData.find(';', 10);
					if (nPos != std::string::npos)
						sData = "ENCRYPTED;" + sData.substr(nPos + 1);
				}
			}
			else
			{
				// not local url
			}

			std::string sCode = "(function(){\n\
var _url = \"" + sUrlNatural + "\";\n\
var _images = window[\"crypto_images_map\"][_url];\n\
if (!_images) { return; }\n\
for (var i = 0; i < _images.length; i++) \n\
{\n\
_images[i].onload_crypto(\"" + sUrl + "\", \"" + sData + "\");\n\
}\n\
delete window[\"crypto_images_map\"][_url];\n\
})();";

			_frame->ExecuteJavaScript(sCode, _frame->GetURL(), 0);
			return true;
		}
	}
	else if (sMessageName == "on_download_files")
	{
		int nCount = message->GetArgumentList()->GetSize();
		int nIndex = 0;
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), nIndex++);

		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (!_frame)
			_frame = GetEditorFrame(browser);

		if (_frame)
		{
			std::string sParam = "{";
			while (nIndex < nCount)
			{
				sParam += ("\"" + message->GetArgumentList()->GetString(nIndex++).ToString() + "\" : \"");

				std::string sFile = message->GetArgumentList()->GetString(nIndex++).ToString();
				NSStringUtils::string_replaceA(sFile, "\\", "/");

				sParam += (sFile + "\"");

				if (nIndex < nCount)
					sParam += ", ";
			}
			sParam += "}";

			_frame->ExecuteJavaScript("(function() { window.on_native_download_files(" + sParam + "); delete window.on_native_download_files; })();", _frame->GetURL(), 0);
		}
		return true;
	}
	else if (sMessageName == "set_crypto_mode")
	{
		int nError = message->GetArgumentList()->GetInt(0);
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 1);

		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (_frame)
			_frame->ExecuteJavaScript("(function() { if (!window.on_set_crypto_mode) return; window.on_set_crypto_mode(" + std::to_string(nError) + "); delete window.on_set_crypto_mode; })();", _frame->GetURL(), 0);
	}
	else if (sMessageName == "get_advanced_encrypted_data")
	{
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 0);
		std::string sRet = message->GetArgumentList()->GetString(1);
		NSStringUtils::string_replaceA(sRet, "\\", "\\\\");

		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (_frame)
			  _frame->ExecuteJavaScript("(function() { if (!window.on_get_advanced_encrypted_data) return; window.on_get_advanced_encrypted_data(\"" + sRet + "\"); delete window.on_get_advanced_encrypted_data; })();", _frame->GetURL(), 0);
	}
	else if (sMessageName == "set_advanced_encrypted_data")
	{
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 0);
		std::string sRet = message->GetArgumentList()->GetString(1);
		NSStringUtils::string_replaceA(sRet, "\\", "\\\\");

		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (_frame)
			_frame->ExecuteJavaScript("(function() { if (!window.on_set_advanced_encrypted_data) return; window.on_set_advanced_encrypted_data(\"" + sRet + "\"); delete window.on_set_advanced_encrypted_data; })();", _frame->GetURL(), 0);
	}
	else if (sMessageName == "crypto_download_as_end")
	{
	  CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
	  if (_frame)
	  {
		  _frame->ExecuteJavaScript("(function() { window.CryptoDownloadAsEnd(); })();", _frame->GetURL(), 0);
	  }
	  return true;
	}
	else if (sMessageName == "set_editors_version_no_crypto")
	{
	  CefRefPtr<CefFrame> _frame = GetEditorFrame(browser);
	  if (_frame)
	  {
		  _frame->ExecuteJavaScript("(function() { window.AscDesktopEditor.CryptoMode = 0; })();", _frame->GetURL(), 0);
	  }
	  return true;
	}
	else if (sMessageName == "set_drop_files")
	{
		int nCount = message->GetArgumentList()->GetSize();
		int nIndex = 0;
		std::wstring sCode = L"[";
		while (nIndex < nCount)
		{
			std::wstring sFile = message->GetArgumentList()->GetString(nIndex++).ToWString();
			g_pLocalResolver->AddFile(sFile);

			sCode += L"\"";
			sCode += sFile;
			sCode += L"\",";
		}
		((wchar_t*)sCode.c_str())[sCode.length() - 1] = ']';

		std::vector<int64> arFramesIds;
		browser->GetFrameIdentifiers(arFramesIds);

		for (std::vector<int64>::iterator i = arFramesIds.begin(); i != arFramesIds.end(); i++)
		{
			CefRefPtr<CefFrame> _frame = browser->GetFrame(*i);
			if (_frame)
				_frame->ExecuteJavaScript(L"window.AscDesktopEditor.SetDropFiles(" + sCode + L");", _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "clear_drop_files")
	{
		std::vector<int64> arFramesIds;
		browser->GetFrameIdentifiers(arFramesIds);

		for (std::vector<int64>::iterator i = arFramesIds.begin(); i != arFramesIds.end(); i++)
		{
			CefRefPtr<CefFrame> _frame = browser->GetFrame(*i);
			if (_frame)
				_frame->ExecuteJavaScript(L"window.AscDesktopEditor.ClearDropFiles();", _frame->GetURL(), 0);
		}

		return true;
	}
	else if (sMessageName == "on_convert_local_file")
	{
		std::string sFolder = message->GetArgumentList()->GetString(0).ToString();
		int64 nFrameId = NSArgumentList::GetInt64(message->GetArgumentList(), 1);
		NSStringUtils::string_replaceA(sFolder, "\\", "\\\\");
		NSStringUtils::string_replaceA(sFolder, "\"", "\\\"");

		CefRefPtr<CefFrame> _frame = browser->GetFrame(nFrameId);
		if (!_frame)
			return true;

		_frame->ExecuteJavaScript("window.on_convert_file_callback(\"" + sFolder + "\");", _frame->GetURL(), 0);
		return true;
	}

	if (m_pAdditional.is_init() && m_pAdditional->OnProcessMessageReceived(app, browser, source_process, message))
		return true;

	return message_router_->OnProcessMessageReceived(
		browser,
		#ifndef MESSAGE_IN_BROWSER
		messageFrame,
		#endif
		source_process, message);
}

private:
  CefRefPtr<CefFrame> GetEditorFrame(CefRefPtr<CefBrowser> browser)
  {
	CefRefPtr<CefFrame> _frame = browser->GetFrame("frameEditor");
	if (_frame)
		return _frame;
	return browser->GetMainFrame();
  }

  std::string GetFileData(const std::wstring& sFile, int& nLen)
  {
	if (m_bIsNativeViewer)
	{
		std::string sFileData;
		NSFile::CFileBinary::ReadAllTextUtf8A(sFile, sFileData);

		std::string::size_type pos = sFileData.find(";");
		nLen = std::stoi(sFileData.substr(0, pos));
		sFileData = sFileData.substr(pos + 1);
		return sFileData;
	}

	BYTE* pData = NULL;
	DWORD dwFileLen = 0;
	NSFile::CFileBinary::ReadAllBytes(sFile, &pData, dwFileLen);

	char* pDataBase64 = NULL;
	int nDataBase64Len = 0;

	nLen = (int)dwFileLen;

	NSFile::CBase64Converter::Encode(pData, nLen, pDataBase64, nDataBase64Len, NSBase64::B64_BASE64_FLAG_NOCRLF);

	std::string sFileData(pDataBase64, nDataBase64Len);

	RELEASEARRAYOBJECTS(pDataBase64);
	RELEASEARRAYOBJECTS(pData);

	return sFileData;
  }

  std::string GetFileData2(const std::wstring& sFile, int& nLen)
  {
	if (m_bIsNativeViewer)
		return GetFileData(sFile, nLen);

	nLen = 0;
	return NSCommon::GetFilePathBase64(sFile);
  }

 private:
  bool last_node_is_editable_;
  bool sync_command_check;

  // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  NSCommon::smart_ptr<CApplicationRendererAdditionalBase> m_pAdditional;
  bool m_bIsNativeViewer;

  IMPLEMENT_REFCOUNTING(ClientRenderDelegate);
};

void CreateRenderDelegates(client::ClientAppRenderer::DelegateSet& delegates) {
  delegates.insert(new ClientRenderDelegate);
}

// static
}  // namespace client_renderer

void client::ClientAppRenderer::CreateDelegates(client::ClientAppRenderer::DelegateSet& delegates) {
	asc_client_renderer::CreateRenderDelegates(delegates);
}
