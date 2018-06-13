/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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
#include "./applicationmanager_events.h"

#include "./keyboardchecker.h"
#include "./spellchecker.h"
#include "./cefview.h"
#include "./cefapplication.h"

#ifdef WIN32
__declspec(dllexport) int __cdecl Core_SetProcessDpiAwareness(void);
__declspec(dllexport) int __cdecl Core_GetMonitorRawDpi(WindowHandleId, unsigned int*, unsigned int*);
__declspec(dllexport) int __cdecl Core_GetMonitorRawDpiByIndex(int, unsigned int*, unsigned int*);
#endif

class Q_DECL_EXPORT CAscApplicationSettings
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

    std::wstring                    app_data_path;

    std::wstring                    local_editors_path;
    std::wstring                    file_converter_path;
    std::wstring                    recover_path;
    std::string                     country;

    bool                            sign_support;
    bool                            pass_support;

public:
    CAscApplicationSettings();
    void SetUserDataPath(std::wstring sPath);
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

class CAscReporterData;
class CAscApplicationManager_Private;
class CApplicationFonts;
class Q_DECL_EXPORT CAscApplicationManager
{
public:
    CAscApplicationManager();
    ~CAscApplicationManager();

public:
    CAscApplicationSettings m_oSettings;
    CAscPrintSettings m_oPrintSettings;

public:

    void StartSpellChecker();
    void StopSpellChecker();
    void SpellCheck(const int& nEditorId, const std::string& sTask, int nId);

    void StartKeyboardChecker();
    virtual void OnNeedCheckKeyboard();
    void CheckKeyboard();
    void SendKeyboardAttack();

    void CheckFonts(bool bAsync = true);

    void SetEventListener(NSEditorApi::CAscCefMenuEventListener* );
    NSEditorApi::CAscCefMenuEventListener* GetEventListener();

    void Apply(NSEditorApi::CAscMenuEvent* );
    NSEditorApi::CAscMenuEvent* ApplySync(NSEditorApi::CAscMenuEvent* );

    CCefView* CreateCefView(CCefViewWidgetImpl* parent);
    CCefViewEditor* CreateCefEditor(CCefViewWidgetImpl* parent);
    CCefViewEditor* CreateCefPresentationReporter(CCefViewWidgetImpl* parent, CAscReporterData* data);

    CCefView* GetViewById(int nId);
    CCefView* GetViewByUrl(const std::wstring& url);
    CCefView* GetViewByRecentId(int nId);

    void Logout(std::wstring strUrl);
    void CancelDownload(unsigned int nDownloadId);

    void DestroyCefView(int nId, bool bIsSafe = true);

    void LockCS(int nId);
    void UnlockCS(int nId);

    bool IsInitFonts();

    std::string GetLibraryPathVariable();

    CApplicationCEF* GetApplication();
    void SetApplication(CApplicationCEF* );

    void SetDebugInfoSupport(bool bIsSupport);
    bool GetDebugInfoSupport();

    void CloseApplication();
    CApplicationFonts* GetApplicationFonts();

    virtual void StartSaveDialog(const std::wstring& sName, unsigned int nId);
    virtual void EndSaveDialog(const std::wstring& sPath, unsigned int nId);

    virtual bool IsPlatformKeyboardSupport();
    virtual int GetPlatformKeyboardLayout();
    
    void OnDestroyWindow();

    static int GetFileFormatByExtentionForSave(const std::wstring& sFileName);

    void InitAdditionalEditorParams(std::wstring& sParams);

    static void DoMessageLoopWork();

    virtual bool IsExternalEventLoop();
    virtual void ExitExternalEventLoop();

    int GetMonitorScaleByIndex(const int& nIndex, unsigned int& nDpiX, unsigned int& nDpiY);
    int GetMonitorScaleByWindow(const WindowHandleId& nHandle, unsigned int& nDpiX, unsigned int& nDpiY);

    void SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* pEvent);

    // 0 - none, 1 - simple, 2 - advanced
    void SetCryptoMode(const std::wstring& sPassword, const int& nMode);
    int GetCryptoMode();

protected:
    int GenerateNextViewId();

protected:
    CAscApplicationManager_Private* m_pInternal;
    
    friend class CCefView_Private;
    friend class CCefView;
    friend class CCefViewEditor;
    friend class CAscClientHandler;
    friend class CASCFileConverterFromEditor;
    friend class CApplicationCEF;
};

#endif // APPLICATION_MANAGER_H
