/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
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

enum class AscEditorType
{
	etDocument              = 0,
	etPresentation          = 1,
	etSpreadsheet           = 2,
	etDocumentMasterForm    = 3,
	etDocumentMasterOForm   = 4,
	etPdf                   = 5,
	etDraw                  = 6,
	etUndefined             = 255
};

class CCefView_Private;
class CAscApplicationManager;

class CCefViewWidgetImpl
{
public:
	WindowHandleId cef_handle;
	unsigned int cef_ex_style;
	unsigned int cef_style;

	unsigned int cef_x;
	unsigned int cef_y;
	unsigned int cef_width;
	unsigned int cef_height;

	unsigned char backgroundR;
	unsigned char backgroundG;
	unsigned char backgroundB;

public:
	CCefViewWidgetImpl()
	{
		cef_handle = 0;
		cef_ex_style = 0;
		cef_style = 0;

		cef_x = 0;
		cef_y = 0;
		cef_width = 0;
		cef_height = 0;

		backgroundR = 255;
		backgroundG = 255;
		backgroundB = 255;
	}

	virtual ~CCefViewWidgetImpl() {}

public:
	virtual void UpdateSize() {}
	virtual void AfterCreate() {}
	virtual void OnLoaded() {}
	virtual void OnRelease() {}

	static void SetParentNull(WindowHandleId handle);
};

class DESKTOP_DECL CCefView
{
public:
	CCefView(CCefViewWidgetImpl* parent, int nId);
	virtual ~CCefView();

	void load(const std::wstring& url);
	void reload();
	std::wstring GetUrl();
	std::wstring GetOriginalUrl();
	std::wstring GetUrlAsLocal();

	void focus(bool value = true);

	void resizeEvent();
	void moveEvent();
	bool isDoubleResizeEvent();

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
	void LoadReporter(void* reporter_data);

	double GetDeviceScale();

	int GetPrintPageOrientation(const int& nPage);

	bool IsDestroy();

	void SetParentWidgetInfo(const std::wstring& json);

	int GetRecentId();

	void ExecuteInAllFrames(const std::string& sCode, const bool& isMain = true);

	// Methods for browser-host plugins
	void ExecuteEditorScript(const std::string& sCode);
	void SendToRenderer(const std::string& sName, const std::string& sArgsJson);

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

	void OpenLocalFile(const std::wstring& sFilePath, const int& nFileFormat, const std::wstring& params = L"");
	void CreateLocalFile(const AscEditorType& nFileFormat, const std::wstring& sName = L"", const std::wstring& sTemplatePath = L"");
	void CreateLocalFile(const AscEditorType& nFileFormat, const int& nTemplateId, const std::wstring& sName = L"");
	bool OpenCopyAsRecoverFile(const int& nIdSrc);
	bool OpenRecoverFile(const int& nId);
	bool OpenRecentFile(const int& nId);
	bool OpenReporter(const std::wstring& sFolder);

	bool CheckCloudCryptoNeedBuild();
	bool IsBuilding();
	bool IsSaveLocked();

	std::wstring GetLocalFilePath();
	std::wstring GetRecoveryDir();

	static int GetFileFormat(const std::wstring& sFilePath);

	void UpdatePlugins();
};

#if defined(_LINUX) && !defined(_MAC)
DESKTOP_DECL void* CefGetXDisplay(void);
#endif

#endif  // CEFCLIENT_CEFWEBVIEW_H
