/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DESKTOP_SDK_NATIVE_PLUGINS_MANAGER_H
#define DESKTOP_SDK_NATIVE_PLUGINS_MANAGER_H

#include "../include/native_plugin.h"
#include "./lib_loader.h"

#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace NSNativePlugin
{
	// One per process (browser or renderer). Loads plugin DLLs from a directory,
	// starts them with the given host, dispatches JS-method / message calls, and
	// supports per-plugin Stop (for future runtime removal) + StopAll on shutdown.
	class CNativePluginManager
	{
	public:
		CNativePluginManager();
		~CNativePluginManager();

		void SetHost(IPluginHost* pHost);
		void SetDirectories(const std::vector<std::wstring>& arDirs);

		// empty list -> loads nothing. Each dir holds plugin subfolders; a plugin
		// is <dir>/<name>/<name><ext> (library named exactly like its folder).
		void LoadAll();
		void StopAll();

		// --- JS method registry (renderer); called from the host's RegisterJsMethod ---
		void RegisterJsMethod(const std::string& sName, bool bAsync);
		bool HasJsMethod(const std::string& sName);
		bool IsJsMethodAsync(const std::string& sName);
		void GetJsMethodNames(std::vector<std::string>& arOut);

		// --- dispatch ---
		bool CallJsMethod(const std::string& sName, const std::string& sArgsJson, std::string& sOut);
		bool CallJsMethodAsync(const std::string& sName, const std::string& sArgsJson, IJsCallResolver* pResolver);
		// NB: not "DispatchMessage" — that is a windows.h macro (DispatchMessageW).
		bool DispatchPluginMessage(const std::string& sName, const std::string& sArgsJson);

	private:
		struct TLoaded
		{
			CLibLoader* pLoader;
			INativePlugin* pPlugin;
			DestroyNativePluginFn fnDestroy;
			TLoaded() : pLoader(NULL), pPlugin(NULL), fnDestroy(NULL) {}
		};

		bool LoadFile(const std::wstring& sAbsPath, const std::string& sName);
		void StopLocked(const std::string& sName);

		IPluginHost* m_pHost;
		std::vector<std::wstring> m_arDirs;
		std::map<std::string, TLoaded> m_mapPlugins;
		std::map<std::string, std::pair<INativePlugin*, bool> > m_mapJsMethods; // name -> (plugin, async)
		INativePlugin* m_pStarting;   // plugin currently in Start() (for RegisterJsMethod)
		std::recursive_mutex m_oMutex;
	};
}

#endif // DESKTOP_SDK_NATIVE_PLUGINS_MANAGER_H
