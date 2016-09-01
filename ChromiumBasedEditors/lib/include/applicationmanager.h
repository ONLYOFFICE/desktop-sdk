#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <string>
#include <vector>
#include "./applicationmanager_events.h"

#include "./keyboardchecker.h"
#include "./spellchecker.h"
#include "./cefview.h"
#include "./cefapplication.h"

class Q_DECL_EXPORT CAscApplicationSettings
{
public:
    std::wstring                    spell_dictionaries_path;
    std::wstring                    cache_path;
    std::wstring                    cookie_path;

    bool                            use_system_fonts;
    std::vector<std::wstring>       additional_fonts_folder;
    std::wstring                    fonts_cache_info_path;

    std::wstring                    app_data_path;

    std::wstring                    local_editors_path;
    std::wstring                    file_converter_path;
    std::wstring                    recover_path;
    std::string                     country;

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

    void SetEventListener(NSEditorApi::CAscMenuEventListener* );
    NSEditorApi::CAscMenuEventListener* GetEventListener();

    void Apply(NSEditorApi::CAscMenuEvent* );
    NSEditorApi::CAscMenuEvent* ApplySync(NSEditorApi::CAscMenuEvent* );

    CCefView* CreateCefView(CCefViewWidgetImpl* parent);
    CCefViewEditor* CreateCefEditor(CCefViewWidgetImpl* parent);

    CCefView* GetViewById(int nId);
    CCefView* GetViewByUrl(const std::wstring& url);

    void Logout(std::wstring strUrl);
    void CancelDownload(unsigned int nDownloadId);

    void DestroyCefView(int nId, bool bIsSafe = true);

    void LockCS(int nId);
    void UnlockCS(int nId);

    bool IsInitFonts();

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

protected:
    CAscApplicationManager_Private* m_pInternal;
    
    friend class CCefView_Private;
    friend class CCefView;
    friend class CCefViewEditor;
    friend class CAscClientHandler;
    friend class CASCFileConverterFromEditor;
};

#endif // APPLICATION_MANAGER_H
