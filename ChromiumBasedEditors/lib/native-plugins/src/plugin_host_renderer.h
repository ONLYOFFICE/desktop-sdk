/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DESKTOP_SDK_PLUGIN_HOST_RENDERER_H
#define DESKTOP_SDK_PLUGIN_HOST_RENDERER_H

#include "../include/native_plugin.h"
#include "./plugin_manager.h"

#include <map>
#include <string>
#include <mutex>

#include "include/cef_v8.h"

#if !defined(CEF_2623) && !defined(CEF_VERSION_103)
#define NATIVE_PLUGIN_ASYNC_JS
#endif

// Renderer-process host (process singleton). Backs window.AscDesktopEditor plugin
// methods: sync (returns string) and async (returns a JS Promise resolved later
// from any thread via IJsCallResolver -> ResolveJsCall).
class CRendererPluginHost : public NSNativePlugin::IPluginHostRenderer
{
public:
	static CRendererPluginHost& Instance();

	// Loads plugins once (dirs from CAscRendererProcessParams: "native-plugins-dir-count"
	// + "native-plugins-dir-<i>"). Empty => nothing loaded.
	void Init();
	NSNativePlugin::CNativePluginManager& Manager();

	// Convert a V8 argument list to a JSON array string (the plugin arg contract).
	static std::string ArgsToJson(const CefV8ValueList& arguments);

#ifdef NATIVE_PLUGIN_ASYNC_JS
	// Called from Execute() on the V8 thread (context active): creates the pending
	// Promise, returns it; outCallId identifies it for the resolver.
	CefRefPtr<CefV8Value> CreatePendingPromise(std::string& sOutCallId);
#endif

	// IPluginHostRenderer
	virtual NSNativePlugin::EPluginRole Role() { return NSNativePlugin::EPluginRole::Renderer; }
	virtual void RegisterJsMethod(const std::string& sName, bool bAsync);
	virtual void ExecuteJS(const std::string& sCode);
	virtual void SendToBrowser(const std::string& sName, const std::string& sArgsJson);
	virtual std::string GetParam(const std::string& sKey);

#ifdef NATIVE_PLUGIN_ASYNC_JS
	// Called by CJsCallResolver from any thread; settles the Promise on TID_RENDERER.
	void ResolveJsCall(const std::string& sCallId, bool bOk, const std::string& sJson);
	void SettleOnRenderer(const std::string& sCallId, bool bOk, const std::string& sJson);
#endif

private:
	CRendererPluginHost();

	NSNativePlugin::CNativePluginManager m_oManager;
	bool m_bInited;

#ifdef NATIVE_PLUGIN_ASYNC_JS
	struct TPending
	{
		CefRefPtr<CefV8Value> promise;
		CefRefPtr<CefV8Context> context;
	};
	std::map<std::string, TPending> m_mapPending;
	std::mutex m_oMutex;
	unsigned long long m_nCounter;
#endif
};

#ifdef NATIVE_PLUGIN_ASYNC_JS
// IJsCallResolver handed to plugins for async methods.
class CJsCallResolver : public NSNativePlugin::IJsCallResolver
{
public:
	CJsCallResolver(const std::string& sCallId, CRendererPluginHost* pHost)
		: m_sCallId(sCallId), m_pHost(pHost) {}
	virtual void Resolve(const std::string& sJson) { m_pHost->ResolveJsCall(m_sCallId, true, sJson); }
	virtual void Reject(const std::string& sError) { m_pHost->ResolveJsCall(m_sCallId, false, sError); }
	virtual void Release() { delete this; }
private:
	std::string m_sCallId;
	CRendererPluginHost* m_pHost;
};
#endif

#endif // DESKTOP_SDK_PLUGIN_HOST_RENDERER_H
