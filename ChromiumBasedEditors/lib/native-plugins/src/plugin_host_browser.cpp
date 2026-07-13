/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./plugin_host_browser.h"

#include "../../include/cefview.h"
#include "../../include/applicationmanager.h"

#include "include/cef_task.h"
#include "../../../../../core/DesktopEditor/common/File.h"

#include <vector>
#include <future>
#include <functional>

namespace
{
	std::string NormalizePathA(const std::string& s)
	{
		std::string r = s;
		for (size_t i = 0; i < r.length(); ++i)
		{
			if (r[i] == '\\') r[i] = '/';
			else if (r[i] >= 'A' && r[i] <= 'Z') r[i] = (char)(r[i] - 'A' + 'a');
		}
		return r;
	}
	std::string NormalizePathW(const std::wstring& s) { return NormalizePathA(U_TO_UTF8(s)); }

	std::string EditorTypeToString(AscEditorType eType)
	{
		switch (eType)
		{
		case AscEditorType::etPresentation:        return "slide";
		case AscEditorType::etSpreadsheet:         return "cell";
		case AscEditorType::etPdf:
		case AscEditorType::etDocumentMasterOForm: return "pdf";
		default:                                   return "word";
		}
	}

	// UI-thread only.
	CCefViewEditor* FindEditorView(CAscApplicationManager* pManager, const std::string& sAbsPath)
	{
		if (!pManager)
			return NULL;
		std::string sTarget = NormalizePathA(sAbsPath);
		std::vector<int> arIds = pManager->GetViewsId();
		for (size_t i = 0; i < arIds.size(); ++i)
		{
			CCefView* pView = pManager->GetViewById(arIds[i]);
			if (!pView || pView->GetType() != cvwtEditor)
				continue;
			CCefViewEditor* pEditor = (CCefViewEditor*)pView;
			if (NormalizePathW(pEditor->GetLocalFilePath()) == sTarget)
				return pEditor;
		}
		return NULL;
	}

	// Run a std::function on the CEF UI thread.
	class CUITask : public CefTask
	{
	public:
		CUITask(const std::function<void()>& fn) : m_fn(fn) {}
		virtual void Execute() OVERRIDE { if (m_fn) m_fn(); }
	private:
		std::function<void()> m_fn;
		IMPLEMENT_REFCOUNTING(CUITask);
	};

	void PostUI(const std::function<void()>& fn)
	{
		if (CefCurrentlyOn(TID_UI))
			fn();
		else
			CefPostTask(TID_UI, new CUITask(fn));
	}
}

CBrowserPluginHost::CBrowserPluginHost(CAscApplicationManager* pManager) : m_pManager(pManager)
{
}

std::string CBrowserPluginHost::IsOpen(const std::wstring& sAbsPath)
{
	// Synchronous to the caller (IPC worker); resolves on the UI thread.
	std::shared_ptr<std::promise<std::string> > pResult = std::make_shared<std::promise<std::string> >();
	std::future<std::string> oFuture = pResult->get_future();

	CAscApplicationManager* pManager = m_pManager;
	std::string sPath = U_TO_UTF8(sAbsPath); // FindEditorView compares in UTF-8
	PostUI([pManager, sPath, pResult]()
	{
		CCefViewEditor* pEditor = FindEditorView(pManager, sPath);
		pResult->set_value(pEditor ? EditorTypeToString(pEditor->GetEditorType()) : std::string(""));
	});

	return oFuture.get();
}

void CBrowserPluginHost::ExecuteEditorScript(const std::string& sAbsPath, const std::string& sScript)
{
	CAscApplicationManager* pManager = m_pManager;
	std::string sPath = sAbsPath;
	std::string sCode = sScript;
	PostUI([pManager, sPath, sCode]()
	{
		CCefViewEditor* pEditor = FindEditorView(pManager, sPath);
		if (pEditor)
			pEditor->ExecuteEditorScript(sCode);
	});
}

void CBrowserPluginHost::SendToRenderers(const std::string& sName, const std::string& sArgsJson)
{
	CAscApplicationManager* pManager = m_pManager;
	std::string sN = sName, sA = sArgsJson;
	PostUI([pManager, sN, sA]()
	{
		if (!pManager)
			return;
		std::vector<int> arIds = pManager->GetViewsId();
		for (size_t i = 0; i < arIds.size(); ++i)
		{
			CCefView* pView = pManager->GetViewById(arIds[i]);
			if (pView)
				pView->SendToRenderer(sN, sA);
		}
	});
}
