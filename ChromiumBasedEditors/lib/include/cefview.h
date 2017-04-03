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
};

class Q_DECL_EXPORT CCefView
{
public:

    CCefView(CCefViewWidgetImpl* parent, int nId);
    virtual ~CCefView();

    void load(const std::wstring& url);
    std::wstring GetUrl();

    void focus(bool value = true);

    void resizeEvent(int width = 0, int height = 0);
    void moveEvent();
    bool nativeEvent(const char* data, const int& datalen, void *message, long *result);

    void Apply(NSEditorApi::CAscMenuEvent* );
    NSEditorApi::CAscMenuEvent* ApplySync(NSEditorApi::CAscMenuEvent* );
    
    bool StartDownload(const std::wstring& sUrl);

#if 0
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dragMoveEvent(QDragMoveEvent *);
    virtual void dragLeaveEvent(QDragLeaveEvent *);
    virtual void dropEvent(QDropEvent *);
#endif

    CAscApplicationManager* GetAppManager();
    void SetAppManager(CAscApplicationManager* );

    CCefViewWidgetImpl* GetWidgetImpl();
    void OnDestroyWidgetImpl();

    int GetId();
    CefViewWrapperType GetType();

    void SetParentCef(int nId);
    int GetParentCef();

    void SetModified(bool bIsModified);
    bool GetModified();

protected:
    int m_nId;
    CefViewWrapperType m_eWrapperType;
    CCefView_Private* m_pInternal;

    friend class CAscClientHandler;
    friend class CAscApplicationManager;
    friend class CASCFileConverterToEditor;
};

class Q_DECL_EXPORT CCefViewEditor : public CCefView
{
protected:
    AscEditorType m_eType;

public:
    CCefViewEditor(CCefViewWidgetImpl* parent, int nId);
    virtual ~CCefViewEditor();

    void SetEditorType(AscEditorType eType);
    AscEditorType GetEditorType();

    static int GetFileFormat(const std::wstring& sFilePath);
    void OpenLocalFile(const std::wstring& sFilePath, const int& nFileFormat);
    void CreateLocalFile(const int& nFileFormat, const std::wstring& sName = L""); // AscEditorType
    bool OpenRecoverFile(const int& nId);
    bool OpenRecentFile(const int& nId);
};

class IFileDownloaderEvents
{
public:
    virtual void OnProgress(int nProgress) = 0;
    virtual void OnDownload(bool bIsSuccess) = 0;
};

class CCefFileDownloader_Private;
class Q_DECL_EXPORT CCefFileDownloader
{
protected:
    CCefFileDownloader_Private* m_pInternal;

public:
    CCefFileDownloader(IFileDownloaderEvents* pEvents);
    ~CCefFileDownloader();

    void DownloadFile(CAscApplicationManager* pManager, const std::wstring& sUrl, const std::wstring& sDstFile);
};


#endif  // CEFCLIENT_CEFWEBVIEW_H
