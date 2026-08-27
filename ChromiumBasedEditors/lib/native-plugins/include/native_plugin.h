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

#ifndef DESKTOP_SDK_NATIVE_PLUGIN_H
#define DESKTOP_SDK_NATIVE_PLUGIN_H

#include <string>

#define NATIVE_PLUGIN_API_VERSION 1

namespace NSNativePlugin
{
	enum class EPluginRole
	{
		Browser  = 0,
		Renderer = 1
	};

	// Bitmask returned by the optional NativePluginRoles() export: which
	// processes the plugin wants to run in. If the export is absent, the loader
	// assumes both (loads everywhere).
	enum ENativePluginRoleBits
	{
		NP_ROLE_BROWSER  = 1,
		NP_ROLE_RENDERER = 2
	};

	// Handed to the plugin for an ASYNC JS method. The plugin calls Resolve or
	// Reject exactly once (from any thread), then Release(). The framework
	// marshals to the V8 thread and settles the JS Promise / invokes the callback.
	class IJsCallResolver
	{
	public:
		virtual ~IJsCallResolver() {}
		virtual void Resolve(const std::string& sJson) = 0;
		virtual void Reject(const std::string& sError) = 0;
		virtual void Release() = 0;
	};

	class IPluginHost
	{
	public:
		virtual ~IPluginHost() {}
		virtual EPluginRole Role() = 0;
	};

	// Renderer-process host (V8 side).
	class IPluginHostRenderer : public IPluginHost
	{
	public:
		// Expose window.AscDesktopEditor.<name>. bAsync -> returns a Promise.
		virtual void RegisterJsMethod(const std::string& sName, bool bAsync) = 0;
		// Run JS in the current editor frame (renderer V8 thread).
		virtual void ExecuteJS(const std::string& sCode) = 0;
		// Send a message to the browser process.
		virtual void SendToBrowser(const std::string& sName, const std::string& sArgsJson) = 0;
		// Renderer command-line parameter (CAscRendererProcessParams).
		virtual std::string GetParam(const std::string& sKey) = 0;
	};

	// Browser-process host (views / UI thread side).
	class IPluginHostBrowser : public IPluginHost
	{
	public:
		// If sAbsPath is open in a live editor, return its family
		// (word|cell|slide|pdf); else "". Path is the SDK-native wide string.
		virtual std::string IsOpen(const std::wstring& sAbsPath) = 0;
		// Async: run JS in the "frameEditor" of the view holding sAbsPath.
		virtual void ExecuteEditorScript(const std::string& sAbsPath, const std::string& sScript) = 0;
		// Send a message to all renderer processes.
		virtual void SendToRenderers(const std::string& sName, const std::string& sArgsJson) = 0;
	};

	class INativePlugin
	{
	public:
		virtual ~INativePlugin() {}

		// The plugin's name == the folder it was loaded from. The framework
		// assigns it via SetName() right after creation (before Start); Name()
		// returns it. Inherit CNativePluginBase to get both for free instead of
		// hardcoding the name.
		virtual const char* Name() = 0;
		virtual void SetName(const std::string& /*sName*/) {}
		virtual int ApiVersion() = 0;

		// The plugin's OWN version as "MAJOR.MINOR" (e.g. "1.3") — not the host
		// API version above. When the same plugin folder exists in more than one
		// of the configured directories, the loader keeps the copy with the
		// higher version and never starts the other ones. A plugin that does not
		// override this stays at "0.0" and loses to any versioned copy; equal
		// versions are resolved by directory order (first configured wins).
		virtual const char* Version() { return "0.0"; }

		// host->Role() tells the plugin which process it is in; downcast to
		// IPluginHostRenderer* / IPluginHostBrowser* accordingly.
		virtual void Start(IPluginHost* pHost) = 0;
		// Must synchronously stop threads/servers before the DLL is unloaded.
		virtual void Stop() = 0;

		// --- renderer JS methods (return true if this plugin handles sName) ---
		// sArgsJson is a JSON array of the JS call arguments.
		// Sync returns via sOut; async via the resolver.
		virtual bool OnJsMethod(const std::string& /*sName*/, const std::string& /*sArgsJson*/, std::string& /*sOut*/) { return false; }
		virtual bool OnJsMethodAsync(const std::string& /*sName*/, const std::string& /*sArgsJson*/, IJsCallResolver* /*pResolver*/) { return false; }

		// --- cross-process / general messages (return true if handled) ---
		virtual bool OnMessage(const std::string& /*sName*/, const std::string& /*sArgsJson*/) { return false; }
	};

	// Convenience base: stores the framework-assigned name (== folder name) and
	// implements Name()/SetName(). Inherit this instead of INativePlugin to avoid
	// hardcoding the name; you still implement ApiVersion()/Start()/Stop().
	class CNativePluginBase : public INativePlugin
	{
	public:
		virtual const char* Name() { return m_sName.c_str(); }
		virtual void SetName(const std::string& sName) { m_sName = sName; }
	protected:
		std::string m_sName;
	};
}

#if defined(_WIN32)
#define NATIVE_PLUGIN_EXPORT __declspec(dllexport)
#else
#define NATIVE_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// C ABI exported by every plugin DLL (resolved by name via the SDK loader).
extern "C"
{
	typedef NSNativePlugin::INativePlugin* (*CreateNativePluginFn)(int nHostApiVersion);
	typedef void (*DestroyNativePluginFn)(NSNativePlugin::INativePlugin*);
	// OPTIONAL: return NP_ROLE_* bits (which processes this plugin runs in).
	// If not exported, the loader loads the plugin in every process.
	typedef int (*NativePluginRolesFn)();

	NATIVE_PLUGIN_EXPORT NSNativePlugin::INativePlugin* CreateNativePlugin(int nHostApiVersion);
	NATIVE_PLUGIN_EXPORT void DestroyNativePlugin(NSNativePlugin::INativePlugin* pPlugin);
	NATIVE_PLUGIN_EXPORT int NativePluginRoles();
}

#endif // DESKTOP_SDK_NATIVE_PLUGIN_H
