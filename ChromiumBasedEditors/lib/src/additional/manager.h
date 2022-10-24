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

#ifndef APPLICATION_MANAGER_ADDITIONAL_H
#define APPLICATION_MANAGER_ADDITIONAL_H

#include "../../include/applicationmanager_events.h"
#include "./renderer.h"
#include "../../../../../core/DesktopEditor/graphics/pro/Graphics.h"
#include "./../../include/applicationmanager.h"

class CApplicationManagerAdditionalBase
{
public:
	std::vector<NSEditorApi::CAscMenuEvent*>* m_arApplyEvents;

public:
	CApplicationManagerAdditionalBase(CAscApplicationManager* pManager)
	{
		m_arApplyEvents = NULL;
	}
	virtual ~CApplicationManagerAdditionalBase()
	{
	}

public:
	virtual bool GetEditorPermission() { return true; }

	virtual void Apply(NSEditorApi::CAscMenuEvent* pEvent) {}
	virtual void ApplyView(CCefView* pView, NSEditorApi::CAscMenuEvent* pEvent, CefRefPtr<CefBrowser> browser) {}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										  CefProcessId source_process,
										  CefRefPtr<CefProcessMessage> message,
										  CCefView* view)
	{
		return false;
	}

	virtual void CheckSaveStart(std::wstring sDirectory, int nFileType) {}
	virtual void CheckSaveEnd() {}

	virtual void Print_Start() {}
	virtual void Print_End() {}
	virtual void Check_Print(IRenderer* pRenderer, NSFonts::IFontManager* pFontManager, int nRasterW, int nRasterH, double fPageWidth, double fPageHeight) {}

	virtual void Local_Save_Start() {}
	virtual bool Local_Save_End(bool bIsNeedSaveDialog, int nId, CefRefPtr<CefBrowser> browser) { return false; }

	virtual std::string GetCrashPage() { return ""; }

	virtual std::wstring GetExternalSchemeName() { return L"oo-office"; }
};

CApplicationManagerAdditionalBase* Create_ApplicationManagerAdditional(CAscApplicationManager* pManager);

#endif // APPLICATION_MANAGER_ADDITIONAL_H
