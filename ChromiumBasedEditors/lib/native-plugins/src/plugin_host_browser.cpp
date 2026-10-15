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
#include <chrono>
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
		case AscEditorType::etPdf:                 return "pdf";
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

	// Minimal JSON string escape for the hand-built ListOpen array below
	// (this file deliberately has no JSON library dependency).
	std::string JsonEscape(const std::string& s)
	{
		std::string r;
		r.reserve(s.length() + 8);
		for (size_t i = 0; i < s.length(); ++i)
		{
			unsigned char c = (unsigned char)s[i];
			switch (c)
			{
			case '\"': r += "\\\""; break;
			case '\\': r += "\\\\"; break;
			case '\n': r += "\\n"; break;
			case '\r': r += "\\r"; break;
			case '\t': r += "\\t"; break;
			default:
				if (c < 0x20)
				{
					char buf[8];
					sprintf(buf, "\\u%04x", (int)c);
					r += buf;
				}
				else
					r += (char)c;
			}
		}
		return r;
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

	// False when the task could not be queued (the UI thread is going away).
	bool PostUI(const std::function<void()>& fn)
	{
		if (CefCurrentlyOn(TID_UI))
		{
			fn();
			return true;
		}
		return CefPostTask(TID_UI, new CUITask(fn));
	}

	// How long a worker waits for the UI thread to answer, checking every
	// c_nUiPollMs whether a plugin stop has begun meanwhile.
	const int c_nUiWaitMs = 30000;
	const int c_nUiPollMs = 50;

	// The UI thread's answer, or sFallback when it is not coming: a stop began
	// (the UI thread is the one stopping), the wait ran out, or the task was
	// dropped unrun (the promise is broken then, and get() throws).
	std::string WaitForUI(std::future<std::string>& oFuture, const std::atomic<int>& nStopping,
	                      const std::string& sFallback)
	{
		for (int nWaited = 0; nWaited < c_nUiWaitMs; nWaited += c_nUiPollMs)
		{
			if (std::future_status::ready == oFuture.wait_for(std::chrono::milliseconds(c_nUiPollMs)))
			{
				try
				{
					return oFuture.get();
				}
				catch (...)
				{
					return sFallback;
				}
			}
			if (nStopping > 0)
				return sFallback;
		}
		return sFallback;
	}
}

CBrowserPluginHost::CBrowserPluginHost(CAscApplicationManager* pManager) : m_pManager(pManager), m_nStopping(0)
{
}

void CBrowserPluginHost::BeginStop()
{
	++m_nStopping;
}

void CBrowserPluginHost::EndStop()
{
	--m_nStopping;
}

std::string CBrowserPluginHost::IsOpen(const std::wstring& sAbsPath)
{
	if (m_nStopping > 0)
		return "";

	// Synchronous to the caller (IPC worker); resolves on the UI thread.
	std::shared_ptr<std::promise<std::string> > pResult = std::make_shared<std::promise<std::string> >();
	std::future<std::string> oFuture = pResult->get_future();

	CAscApplicationManager* pManager = m_pManager;
	std::string sPath = U_TO_UTF8(sAbsPath); // FindEditorView compares in UTF-8
	if (!PostUI([pManager, sPath, pResult]()
	{
		CCefViewEditor* pEditor = FindEditorView(pManager, sPath);
		pResult->set_value(pEditor ? EditorTypeToString(pEditor->GetEditorType()) : std::string(""));
	}))
		return "";

	return WaitForUI(oFuture, m_nStopping, "");
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

std::string CBrowserPluginHost::ListOpen()
{
	if (m_nStopping > 0)
		return "[]";

	// Same walk as FindEditorView, minus the path comparison. Synchronous to
	// the caller (a plugin's IPC worker); collected on the UI thread.
	std::shared_ptr<std::promise<std::string> > pResult = std::make_shared<std::promise<std::string> >();
	std::future<std::string> oFuture = pResult->get_future();

	CAscApplicationManager* pManager = m_pManager;
	if (!PostUI([pManager, pResult]()
	{
		std::string sOut = "[";
		if (pManager)
		{
			std::vector<int> arIds = pManager->GetViewsId();
			for (size_t i = 0; i < arIds.size(); ++i)
			{
				CCefView* pView = pManager->GetViewById(arIds[i]);
				if (!pView || pView->GetType() != cvwtEditor)
					continue;
				CCefViewEditor* pEditor = (CCefViewEditor*)pView;
				// An empty path is a document not saved to disk yet: listed so
				// the caller can tell the user, but not addressable by path.
				std::string sPath = U_TO_UTF8(pEditor->GetLocalFilePath());
				if (sOut.length() > 1)
					sOut += ",";
				sOut += "{\"path\":\"" + JsonEscape(sPath) + "\",\"editor\":\"" +
					EditorTypeToString(pEditor->GetEditorType()) + "\",\"id\":" +
					std::to_string(pView->GetId()) + ",\"modified\":" +
					(pView->GetModified() ? "true" : "false") + "}";
			}
		}
		sOut += "]";
		pResult->set_value(sOut);
	}))
		return "[]";

	return WaitForUI(oFuture, m_nStopping, "[]");
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
