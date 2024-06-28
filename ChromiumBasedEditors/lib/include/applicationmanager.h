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

#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "./applicationmanager_events.h"

#include "./keyboardchecker.h"
#include "./spellchecker.h"
#include "./cefview.h"
#include "./cefapplication.h"
#include "./keychain.h"

#ifdef WIN32
DESKTOP_DECL int Core_SetProcessDpiAwareness(void);
DESKTOP_DECL int Core_GetMonitorRawDpi(WindowHandleId, unsigned int*, unsigned int*);
DESKTOP_DECL int Core_GetMonitorRawDpiByIndex(int, unsigned int*, unsigned int*);
#endif

DESKTOP_DECL double Core_GetMonitorScale(const unsigned int& x, const unsigned int& y);

class DESKTOP_DECL CAscApplicationSettings
{
public:
	std::wstring                    spell_dictionaries_path;
	std::wstring                    cache_path;
	std::wstring                    cookie_path;

	bool                            use_system_fonts;
	std::vector<std::wstring>       additional_fonts_folder;
	std::wstring                    fonts_cache_info_path;
	std::wstring                    system_plugins_path;
	std::wstring                    user_plugins_path;
	std::wstring                    user_dictionaries_path;

	std::wstring                    app_data_path;

	std::wstring                    local_editors_path;
	std::wstring                    connection_error_path;
	std::wstring                    file_converter_path;
	std::wstring                    recover_path;
	std::string                     country;

	bool                            sign_support;
	bool                            pass_support;
	bool                            protect_support;

	std::string                     converter_application_name;
	std::string                     converter_application_company;

	std::wstring                    user_providers_path;

	bool                            macroses_support;
	bool                            plugins_support;

	std::map<std::string, std::string> converter_environments;

public:
	CAscApplicationSettings();
	~CAscApplicationSettings();
	void SetUserDataPath(std::wstring sPath);
};

class CUserSettings_Private;
class DESKTOP_DECL CUserSettings
{
public:
	CUserSettings();
	~CUserSettings();
	std::wstring Get(const std::wstring& name);
	void Set(const std::wstring& name, const std::wstring& value);

private:
	CUserSettings_Private* m_pInternal;
	friend class CAscApplicationManager;
};

class CAscPrintSettings
{
public:
	enum PrintMode
	{
		pmFit       = 0,    // вписывать по аспекту
		pmStretch   = 1,    // растягивать
		pm100       = 2     // печатать как есть
	};

public:
	PrintMode   Mode;
	int         WidthPix;
	int         HeightPix;
	bool        ZoomEnable;
	bool        RotateEnable;
	bool        PrintableArea;

public:

	CAscPrintSettings()
	{
		Mode = pm100;
		WidthPix = -1;
		HeightPix = -1;
		ZoomEnable = false;
		RotateEnable = true;
		PrintableArea = false;
	}
};

class CAscApplicationManager;
class DESKTOP_DECL CAscDpiChecker
{
public:
	CAscDpiChecker(CAscApplicationManager* pManager);
	virtual ~CAscDpiChecker();

	virtual int GetWindowDpi(WindowHandleId, unsigned int*, unsigned int*);
	virtual int GetMonitorDpi(int, unsigned int*, unsigned int*);
	virtual int GetWidgetImplDpi(CCefViewWidgetImpl*, unsigned int*, unsigned int*);

	virtual double GetScale(unsigned int, unsigned int);

	double GetForceScale(unsigned int*, unsigned int*);

private:
	CAscApplicationManager* m_pManager;
};

class IExternalMessageLoop
{
public:
	IExternalMessageLoop() {}
	virtual ~IExternalMessageLoop() {}
	virtual void Run() = 0;
	virtual void Execute(void* message) = 0;
	virtual void SetTimer(long delay) = 0;
	virtual void KillTimer() = 0;
	virtual void Exit() = 0;
};

enum class ErrorPageType
{
	Network   = 0,
	Crash     = 1,
	Undefined = 255
};

class CAscReporterData;
class CAscApplicationManager_Private;
namespace NSFonts { class IApplicationFonts; }
class DESKTOP_DECL CAscApplicationManager
{
public:
	CAscApplicationManager();
	virtual ~CAscApplicationManager();

public:
	CAscApplicationSettings m_oSettings;
	CAscPrintSettings m_oPrintSettings;

public:

	// spell check
	void StartSpellChecker();
	void StopSpellChecker();
	void SpellCheck(const int& nEditorId, const std::string& sTask, int_64_type nId);

	// keyboard layout check
	void StartKeyboardChecker();
	void CheckKeyboard();
	void SendKeyboardAttack();
	virtual void OnNeedCheckKeyboard();
	virtual bool IsPlatformKeyboardSupport();
	virtual int GetPlatformKeyboardLayout();

	// fonts
	bool IsInitFonts();
	void CheckFonts(bool bAsync = true);
	NSFonts::IApplicationFonts* GetApplicationFonts();

	// listener
	void SetEventListener(NSEditorApi::CAscCefMenuEventListener* );
	NSEditorApi::CAscCefMenuEventListener* GetEventListener();

	// events
	void Apply(NSEditorApi::CAscMenuEvent* );
	NSEditorApi::CAscMenuEvent* ApplySync(NSEditorApi::CAscMenuEvent* );

	// creators
	CCefView* CreateCefView(CCefViewWidgetImpl* parent);
	CCefViewEditor* CreateCefEditor(CCefViewWidgetImpl* parent);
	CCefViewEditor* CreateCefPresentationReporter(CCefViewWidgetImpl* parent, CAscReporterData* data);

	// new files path
	std::wstring GetNewFilePath(const AscEditorType& format);

	// <view> <-> <id>
	CCefView* GetViewById(int nId);
	CCefView* GetViewByUrl(const std::wstring& url);
	CCefView* GetViewByRecentId(int nId);
	std::vector<int> GetViewsId();

	// application
	CApplicationCEF* GetApplication();
	void SetApplication(CApplicationCEF* );
	void CloseApplication();

	// dialog events
	virtual void StartSaveDialog(const std::wstring& sName, unsigned int nId);
	virtual void EndSaveDialog(const std::wstring& sPath, unsigned int nId);

	// logout from portal
	void Logout(std::wstring strUrl);

	// cancel download item
	void CancelDownload(unsigned int nDownloadId);

	// lock/unlock CS by id
	void LockCS(int nId);
	void UnlockCS(int nId);

	// external message loop
	virtual bool IsExternalEventLoop();
	virtual IExternalMessageLoop* GetExternalMessageLoop();
	virtual void ExternalMessageLoop_OnExecute(void* message);
	virtual void ExternalMessageLoop_OnTimeout();
	bool IsExitMessageLoop();

	// parameters to local file url
	void InitAdditionalEditorParams(std::wstring& sParams);

	// dpi checker
	static CAscDpiChecker* GetDpiChecker();
	double GetMonitorScaleByIndex(const int& nIndex, unsigned int& nDpiX, unsigned int& nDpiY);
	double GetMonitorScaleByWindow(const WindowHandleId& nHandle, unsigned int& nDpiX, unsigned int& nDpiY);

	// send to all main pages
	void SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* pEvent);
	void SetEventToAllWindows(NSEditorApi::CAscMenuEvent* pEvent);

	// crypto modes <0 - none, 1 - simple, 2 - advanced>
	void SetCryptoMode(const std::string& sPassword, const int& nMode);
	int GetCryptoMode();
	std::vector<int> GetSupportCryptoModes();
	virtual NSAscCrypto::CAscKeychain* GetKeychainEngine();

	// destroy methods
	void OnDestroyWindow();
	void DestroyCefView(int nId, bool bIsSafe = true);

	// debug info
	void SetDebugInfoSupport(bool bIsSupport);
	bool GetDebugInfoSupport();

	// get params sended to renderer context (before all js context)
	std::vector<std::string> GetRendererStartupProperties();

	// get LD_LIBRARY_PATH (for linux only)
	std::string GetLibraryPathVariable();

	// get file format by extention
	static int GetFileFormatByExtentionForSave(const std::wstring& sFileName);

	// get user settings
	CUserSettings* GetUserSettings();

	// check local file
	bool IsResolveLocalFile(const std::wstring& sFile);
	void AddFileToLocalResolver(const std::wstring& sFile);

	void SetRecentPin(const int& nId, const bool& bIsPin);

	void SetRendererProcessVariable(const std::wstring& sVariable);

	static bool IsUseSystemScaling();

	std::wstring GetExternalSchemeName();

	static std::string GetErrorPageAddon(const ErrorPageType& type);

	bool InstallPluginFromStore(const std::wstring& sName);

protected:
	int GenerateNextViewId();
	virtual CAscDpiChecker* InitDpiChecker();

protected:
	CAscApplicationManager_Private* m_pInternal;

	friend class CCefView_Private;
	friend class CCefView;
	friend class CCefViewEditor;
	friend class CAscClientHandler;
	friend class CASCFileConverterFromEditor;
	friend class CASCFileConverterToEditor;
	friend class CApplicationCEF;
	friend class CUserSettings;
	friend class CAscDpiChecker;
	friend class ClientSchemeHandler;
};

#endif // APPLICATION_MANAGER_H
