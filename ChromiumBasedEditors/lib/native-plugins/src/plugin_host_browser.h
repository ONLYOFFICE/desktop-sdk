/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DESKTOP_SDK_PLUGIN_HOST_BROWSER_H
#define DESKTOP_SDK_PLUGIN_HOST_BROWSER_H

#include "../include/native_plugin.h"
#include <string>
#include <atomic>

class CAscApplicationManager;

// Browser-process host: view lookup + live-editor script + renderer broadcast.
// IsOpen/ExecuteEditorScript/SendToRenderers may be called off the UI thread
// (from a plugin's IPC worker) and marshal to TID_UI internally.
class CBrowserPluginHost : public NSNativePlugin::IPluginHostBrowser
{
public:
	CBrowserPluginHost(CAscApplicationManager* pManager);

	virtual NSNativePlugin::EPluginRole Role() { return NSNativePlugin::EPluginRole::Browser; }
	virtual std::string IsOpen(const std::wstring& sAbsPath);
	virtual void ExecuteEditorScript(const std::string& sAbsPath, const std::string& sScript);
	virtual void SendToRenderers(const std::string& sName, const std::string& sArgsJson);
	virtual std::string ListOpen();

	// Bracket every plugin Stop() made on the UI thread. A plugin's Stop() waits
	// for its own worker threads, and a worker may itself be waiting here for
	// the UI thread (IsOpen / ListOpen): in between, those calls answer "nothing
	// open" at once instead of deadlocking the stop.
	void BeginStop();
	void EndStop();

private:
	CAscApplicationManager* m_pManager;
	std::atomic<int> m_nStopping;
};

#endif // DESKTOP_SDK_PLUGIN_HOST_BROWSER_H
