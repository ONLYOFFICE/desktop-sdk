/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./plugin_manager.h"

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/common/Directory.h"

namespace NSNativePlugin
{
	static const wchar_t* PlatformExt()
	{
#ifdef _WIN32
		return L".dll";
#elif defined(__APPLE__)
		return L".dylib";
#else
		return L".so";
#endif
	}

	CNativePluginManager::CNativePluginManager() : m_pHost(NULL), m_pStarting(NULL)
	{
	}

	CNativePluginManager::~CNativePluginManager()
	{
		StopAll();
	}

	void CNativePluginManager::SetHost(IPluginHost* pHost) { m_pHost = pHost; }
	void CNativePluginManager::SetDirectories(const std::vector<std::wstring>& arDirs) { m_arDirs = arDirs; }

	void CNativePluginManager::LoadAll()
	{
		std::wstring sExt = PlatformExt();
		for (size_t d = 0; d < m_arDirs.size(); ++d)
		{
			const std::wstring& sDir = m_arDirs[d];
			if (sDir.empty() || !NSDirectory::Exists(sDir))
				continue;

			// A plugin is a subfolder (one level down); its library is named
			// EXACTLY like the folder (+ platform ext). Everything else in the
			// folder is resources / dependent libraries.
			std::vector<std::wstring> arSubDirs = NSDirectory::GetDirectories(sDir);
			for (size_t i = 0; i < arSubDirs.size(); ++i)
			{
				std::wstring sName = NSFile::GetFileName(arSubDirs[i]); // folder name
				if (sName.empty())
					continue;
				std::wstring sAbsPath = arSubDirs[i] + L"/" + sName + sExt;
				if (NSFile::CFileBinary::Exists(sAbsPath))
					LoadFile(sAbsPath, U_TO_UTF8(sName));
			}
		}
	}

	bool CNativePluginManager::LoadFile(const std::wstring& sAbsPath, const std::string& sName)
	{
		CLibLoader* pLoader = new CLibLoader();
		if (!pLoader->Load(sAbsPath))
		{
			delete pLoader;
			return false;
		}

		// Role filter: if the plugin declares roles, skip it in a process it does
		// not want (avoids keeping the DLL loaded where it would only no-op).
		NativePluginRolesFn fnRoles = (NativePluginRolesFn)pLoader->Sym("NativePluginRoles");
		if (NULL != fnRoles && NULL != m_pHost)
		{
			int nMask = fnRoles();
			int nBit = (m_pHost->Role() == EPluginRole::Browser) ? NP_ROLE_BROWSER : NP_ROLE_RENDERER;
			if (0 == (nMask & nBit))
			{
				pLoader->Unload();
				delete pLoader;
				return false;
			}
		}

		CreateNativePluginFn fnCreate = (CreateNativePluginFn)pLoader->Sym("CreateNativePlugin");
		DestroyNativePluginFn fnDestroy = (DestroyNativePluginFn)pLoader->Sym("DestroyNativePlugin");
		if (NULL == fnCreate || NULL == fnDestroy)
		{
			pLoader->Unload();
			delete pLoader;
			return false;
		}

		INativePlugin* pPlugin = fnCreate(NATIVE_PLUGIN_API_VERSION);
		if (NULL == pPlugin || pPlugin->ApiVersion() != NATIVE_PLUGIN_API_VERSION)
		{
			if (pPlugin) fnDestroy(pPlugin);
			pLoader->Unload();
			delete pLoader;
			return false;
		}

		{
			std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
			pPlugin->SetName(sName); // plugin name == its folder name (the registry key)
			m_pStarting = pPlugin;
			pPlugin->Start(m_pHost); // may call host->RegisterJsMethod -> RegisterJsMethod()
			m_pStarting = NULL;

			TLoaded oEntry;
			oEntry.pLoader = pLoader;
			oEntry.pPlugin = pPlugin;
			oEntry.fnDestroy = fnDestroy;
			m_mapPlugins[sName] = oEntry;
		}
		return true;
	}

	void CNativePluginManager::StopAll()
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		while (!m_mapPlugins.empty())
			StopLocked(m_mapPlugins.begin()->first);
	}

	void CNativePluginManager::StopLocked(const std::string& sName)
	{
		std::map<std::string, TLoaded>::iterator it = m_mapPlugins.find(sName);
		if (it == m_mapPlugins.end())
			return;
		TLoaded oEntry = it->second;

		// drop this plugin's JS methods (stop dispatching to it)
		for (std::map<std::string, std::pair<INativePlugin*, bool> >::iterator jt = m_mapJsMethods.begin();
		     jt != m_mapJsMethods.end(); )
		{
			if (jt->second.first == oEntry.pPlugin)
				m_mapJsMethods.erase(jt++);
			else
				++jt;
		}

		// Stop() must synchronously join threads / stop servers before unload.
		if (oEntry.pPlugin)
		{
			oEntry.pPlugin->Stop();
			if (oEntry.fnDestroy)
				oEntry.fnDestroy(oEntry.pPlugin);
		}
		if (oEntry.pLoader)
		{
			oEntry.pLoader->Unload();
			delete oEntry.pLoader;
		}
		m_mapPlugins.erase(it);
	}

	// --- JS method registry ---

	void CNativePluginManager::RegisterJsMethod(const std::string& sName, bool bAsync)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		if (NULL != m_pStarting)
			m_mapJsMethods[sName] = std::make_pair(m_pStarting, bAsync);
	}

	bool CNativePluginManager::HasJsMethod(const std::string& sName)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		return m_mapJsMethods.find(sName) != m_mapJsMethods.end();
	}

	bool CNativePluginManager::IsJsMethodAsync(const std::string& sName)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		std::map<std::string, std::pair<INativePlugin*, bool> >::iterator it = m_mapJsMethods.find(sName);
		return it != m_mapJsMethods.end() && it->second.second;
	}

	void CNativePluginManager::GetJsMethodNames(std::vector<std::string>& arOut)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		for (std::map<std::string, std::pair<INativePlugin*, bool> >::iterator it = m_mapJsMethods.begin();
		     it != m_mapJsMethods.end(); ++it)
			arOut.push_back(it->first);
	}

	// --- dispatch ---

	bool CNativePluginManager::CallJsMethod(const std::string& sName, const std::string& sArgsJson, std::string& sOut)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		std::map<std::string, std::pair<INativePlugin*, bool> >::iterator it = m_mapJsMethods.find(sName);
		if (it == m_mapJsMethods.end())
			return false;
		return it->second.first->OnJsMethod(sName, sArgsJson, sOut);
	}

	bool CNativePluginManager::CallJsMethodAsync(const std::string& sName, const std::string& sArgsJson, IJsCallResolver* pResolver)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		std::map<std::string, std::pair<INativePlugin*, bool> >::iterator it = m_mapJsMethods.find(sName);
		if (it == m_mapJsMethods.end())
			return false;
		return it->second.first->OnJsMethodAsync(sName, sArgsJson, pResolver);
	}

	bool CNativePluginManager::DispatchPluginMessage(const std::string& sName, const std::string& sArgsJson)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		for (std::map<std::string, TLoaded>::iterator it = m_mapPlugins.begin(); it != m_mapPlugins.end(); ++it)
		{
			if (it->second.pPlugin && it->second.pPlugin->OnMessage(sName, sArgsJson))
				return true;
		}
		return false;
	}
}
