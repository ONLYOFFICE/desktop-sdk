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

#ifndef CEFCLIENT_CEFWEBVIEW_H
#define CEFCLIENT_CEFWEBVIEW_H

#include "./base.h"
#include "./applicationmanager_events.h"

enum CefViewWrapperType
{
    cvwtSimple      = 0,
    cvwtEditor      = 1
};

enum AscEditorType
{
    etDocument      = 0,
    etPresentation  = 1,
    etSpreadsheet   = 2,
    etUndefined     = 255
};

class CCefView_Private;
class CAscApplicationManager;

class CCefViewWidgetImpl
{
public:
    virtual bool parent_window_is_empty() { return false; }
    virtual int parent_x() { return 0; }
    virtual int parent_y() { return 0; }
    virtual int parent_width() { return 0; }
    virtual int parent_height() { return 0; }
    virtual WindowHandleId parent_wid() { return 0; }
    virtual void child_loaded() { }
    virtual void releaseFromChild() { }
};

class DESKTOP_DECL CCefView
{
public:
    CCefView(CCefViewWidgetImpl* parent, int nId);
    virtual ~CCefView();

    void load(const std::wstring& url);
    std::wstring GetUrl();
    std::wstring GetOriginalUrl();
    std::wstring GetUrlAsLocal();

    void focus(bool value = true);

    void resizeEvent(int width = 0, int height = 0);
    void moveEvent();
    bool nativeEvent(const char* data, const int& datalen, void *message, long *result);

    void Apply(NSEditorApi::CAscMenuEvent* );
    NSEditorApi::CAscMenuEvent* ApplySync(NSEditorApi::CAscMenuEvent* );

    NSEditorApi::CAscCefMenuEvent* CreateCefEvent(int nType);
    
    bool StartDownload(const std::wstring& sUrl);

    void SetExternalCloud(const std::wstring& sProviderId);

    CAscApplicationManager* GetAppManager();
    void SetAppManager(CAscApplicationManager* );

    CCefViewWidgetImpl* GetWidgetImpl();
    void OnDestroyWidgetImpl();

    int GetId();
    CefViewWrapperType GetType();

    void SetModified(bool bIsModified);
    bool GetModified();

    bool IsPresentationReporter();
    void LoadReporter(int nParentId, std::wstring url);

    double GetDeviceScale();

protected:
    int m_nId;
    CefViewWrapperType m_eWrapperType;
    CCefView_Private* m_pInternal;

public:

    friend class CCefView_Private;
    friend class CAscClientHandler;
    friend class CAscApplicationManager;
    friend class CAscApplicationManager_Private;
    friend class CASCFileConverterToEditor;
    friend class CCefViewEditor;
};

class DESKTOP_DECL CCefViewEditor : public CCefView
{
protected:
    AscEditorType m_eType;

public:
    CCefViewEditor(CCefViewWidgetImpl* parent, int nId);
    virtual ~CCefViewEditor();

    void SetEditorType(AscEditorType eType);
    AscEditorType GetEditorType();

    void OpenLocalFile(const std::wstring& sFilePath, const int& nFileFormat);
    void CreateLocalFile(const int& nFileFormat, const std::wstring& sName = L"");
    bool OpenRecoverFile(const int& nId);
    bool OpenRecentFile(const int& nId);
    bool OpenReporter(const std::wstring& sFolder);

    bool CheckCloudCryptoNeedBuild();
    bool IsBuilding();

    std::wstring GetLocalFilePath();
    std::wstring GetRecoveryDir();

    static int GetFileFormat(const std::wstring& sFilePath);
};

#endif  // CEFCLIENT_CEFWEBVIEW_H
