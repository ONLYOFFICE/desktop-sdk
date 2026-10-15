/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./plugin_host_renderer.h"
#include "../../src/cefwrapper/client_renderer_params.h"

#include "include/cef_task.h"

#ifndef JSON_HAS_CPP_11
#define JSON_HAS_CPP_11
#endif

#include "../../tools/functions/internal/json.hpp"
using json = nlohmann::json;

namespace
{
	json V8ToJson(CefRefPtr<CefV8Value> v)
	{
		if (!v.get() || v->IsNull() || v->IsUndefined())
			return json(nullptr);
		if (v->IsBool())   return json(v->GetBoolValue());
		if (v->IsInt())    return json(v->GetIntValue());
		if (v->IsUInt())   return json(v->GetUIntValue());
		if (v->IsDouble()) return json(v->GetDoubleValue());
		if (v->IsString()) return json(v->GetStringValue().ToString());
		if (v->IsArray())
		{
			json a = json::array();
			int n = v->GetArrayLength();
			for (int i = 0; i < n; ++i)
				a.push_back(V8ToJson(v->GetValue(i)));
			return a;
		}
		if (v->IsObject())
		{
			json o = json::object();
			std::vector<CefString> keys;
			if (v->GetKeys(keys))
			{
				for (size_t i = 0; i < keys.size(); ++i)
					o[keys[i].ToString()] = V8ToJson(v->GetValue(keys[i]));
			}
			return o;
		}
		return json(nullptr);
	}

#ifdef NATIVE_PLUGIN_ASYNC_JS
	// Marshals promise resolution onto the renderer thread.
	class CSettleTask : public CefTask
	{
	public:
		CSettleTask(CRendererPluginHost* pHost, const std::string& sCallId, bool bOk, const std::string& sJson)
			: m_pHost(pHost), m_sCallId(sCallId), m_bOk(bOk), m_sJson(sJson) {}
		virtual void Execute() OVERRIDE { m_pHost->SettleOnRenderer(m_sCallId, m_bOk, m_sJson); }
	private:
		CRendererPluginHost* m_pHost;
		std::string m_sCallId;
		bool m_bOk;
		std::string m_sJson;
		IMPLEMENT_REFCOUNTING(CSettleTask);
	};
#endif
}

CRendererPluginHost::CRendererPluginHost()
	: m_bInited(false)
#ifdef NATIVE_PLUGIN_ASYNC_JS
	, m_nCounter(0)
#endif
{
}

CRendererPluginHost& CRendererPluginHost::Instance()
{
	static CRendererPluginHost oInstance;
	return oInstance;
}

NSNativePlugin::CNativePluginManager& CRendererPluginHost::Manager()
{
	return m_oManager;
}

void CRendererPluginHost::Init()
{
	if (m_bInited)
		return;
	m_bInited = true;

	CAscRendererProcessParams& oParams = CAscRendererProcessParams::getInstance();
	std::vector<std::wstring> arDirs;
	int nCount = oParams.GetValueInt("native-plugins-dir-count", 0);
	for (int i = 0; i < nCount; ++i)
	{
		std::wstring sDir = oParams.GetValueW("native-plugins-dir-" + std::to_string(i));
		if (!sDir.empty())
			arDirs.push_back(sDir);
	}

	m_oManager.SetHost(this);
	m_oManager.SetDirectories(arDirs);
	m_oManager.LoadAll(); // empty list -> nothing loaded
}

std::string CRendererPluginHost::ArgsToJson(const CefV8ValueList& arguments)
{
	json a = json::array();
	for (size_t i = 0; i < arguments.size(); ++i)
		a.push_back(V8ToJson(arguments[i]));
	return a.dump();
}

#ifdef NATIVE_PLUGIN_ASYNC_JS
CefRefPtr<CefV8Value> CRendererPluginHost::CreatePendingPromise(std::string& sOutCallId)
{
	CefRefPtr<CefV8Value> promise = CefV8Value::CreatePromise();
	std::lock_guard<std::mutex> oLock(m_oMutex);
	sOutCallId = std::to_string(++m_nCounter);
	TPending oPending;
	oPending.promise = promise;
	oPending.context = CefV8Context::GetCurrentContext();
	m_mapPending[sOutCallId] = oPending;
	return promise;
}
#endif

void CRendererPluginHost::RegisterJsMethod(const std::string& sName, bool bAsync)
{
	m_oManager.RegisterJsMethod(sName, bAsync);
}

void CRendererPluginHost::ExecuteJS(const std::string& sCode)
{
	CefRefPtr<CefV8Context> ctx = CefV8Context::GetCurrentContext();
	if (ctx.get() && ctx->GetFrame().get())
		ctx->GetFrame()->ExecuteJavaScript(sCode, ctx->GetFrame()->GetURL(), 0);
}

void CRendererPluginHost::SendToBrowser(const std::string& sName, const std::string& sArgsJson)
{
	CefRefPtr<CefV8Context> ctx = CefV8Context::GetCurrentContext();
	if (!ctx.get())
		return;
	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(sName);
	message->GetArgumentList()->SetString(0, sArgsJson);
#ifdef CEF_2623
	if (ctx->GetBrowser().get())
		ctx->GetBrowser()->SendProcessMessage(PID_BROWSER, message);
#else
	if (ctx->GetFrame().get())
		ctx->GetFrame()->SendProcessMessage(PID_BROWSER, message);
#endif
}

std::string CRendererPluginHost::GetParam(const std::string& sKey)
{
	return CAscRendererProcessParams::getInstance().GetValue(sKey);
}

#ifdef NATIVE_PLUGIN_ASYNC_JS
void CRendererPluginHost::ResolveJsCall(const std::string& sCallId, bool bOk, const std::string& sJson)
{
	if (CefCurrentlyOn(TID_RENDERER))
		SettleOnRenderer(sCallId, bOk, sJson);
	else
		CefPostTask(TID_RENDERER, new CSettleTask(this, sCallId, bOk, sJson));
}

void CRendererPluginHost::SettleOnRenderer(const std::string& sCallId, bool bOk, const std::string& sJson)
{
	TPending oPending;
	{
		std::lock_guard<std::mutex> oLock(m_oMutex);
		std::map<std::string, TPending>::iterator it = m_mapPending.find(sCallId);
		if (it == m_mapPending.end())
			return;
		oPending = it->second;
		m_mapPending.erase(it);
	}
	if (!oPending.context.get() || !oPending.context->IsValid())
		return;
	oPending.context->Enter();
	if (bOk)
		oPending.promise->ResolvePromise(CefV8Value::CreateString(sJson));
	else
		oPending.promise->RejectPromise(sJson);
	oPending.context->Exit();
}
#endif
