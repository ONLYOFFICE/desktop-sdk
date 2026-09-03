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

	static std::wstring FindPluginLibrary(const std::wstring& sPluginDir, const std::wstring& sName)
	{
		std::wstring sPath = sPluginDir + L"/" + sName + PlatformExt();
		if (NSFile::CFileBinary::Exists(sPath))
			return sPath;

#ifndef _WIN32
		sPath = sPluginDir + L"/lib" + sName + PlatformExt();
		if (NSFile::CFileBinary::Exists(sPath))
			return sPath;
#endif

#ifdef _MAC
		std::wstring sBundle = sPluginDir + L"/" + sName + L".framework";
		const wchar_t* arRelative[] = { L"/", L"/Versions/Current/", L"/Versions/A/" };
		for (size_t i = 0; i < 3; ++i)
		{
			sPath = sBundle + arRelative[i] + sName;
			if (NSFile::CFileBinary::Exists(sPath))
				return sPath;
		}
#endif
		return L"";
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

	// A version part is capped instead of overflowing on absurd input; with this
	// bound the whole key stays far inside int (9999 * 10000 + 9999).
	static const int c_nVersionPartMax = 9999;

	static int ReadVersionPart(const char*& p)
	{
		int nPart = 0;
		while (*p >= '0' && *p <= '9')
		{
			nPart = nPart * 10 + (*p - '0');
			if (nPart > c_nVersionPartMax)
				nPart = c_nVersionPartMax;
			++p;
		}
		return nPart;
	}

	// "MAJOR.MINOR" (e.g. "1.3") -> one comparable number, MAJOR dominating.
	// Anything the plugin cannot spell properly reads as 0.0 and loses.
	int CNativePluginManager::VersionKey(const char* sVersion)
	{
		if (NULL == sVersion)
			return 0;
		const char* p = sVersion;
		while (' ' == *p || '\t' == *p)
			++p;
		int nMajor = ReadVersionPart(p);
		int nMinor = 0;
		if ('.' == *p)
		{
			++p;
			nMinor = ReadVersionPart(p);
		}
		return nMajor * (c_nVersionPartMax + 1) + nMinor;
	}

	void CNativePluginManager::LoadAll()
	{
		// Pass 1: collect the copies of every plugin. The same plugin folder may
		// sit in several configured directories (e.g. shipped with the app and
		// installed by the user); those copies compete for one slot.
		std::vector<std::string> arNames; // plugin names in discovery order
		std::map<std::string, std::vector<std::wstring> > mapCandidates;
		for (size_t d = 0; d < m_arDirs.size(); ++d)
		{
			const std::wstring& sDir = m_arDirs[d];
			if (sDir.empty() || !NSDirectory::Exists(sDir))
				continue;

			std::vector<std::wstring> arSubDirs = NSDirectory::GetDirectories(sDir);
			for (size_t i = 0; i < arSubDirs.size(); ++i)
			{
				std::wstring sName = NSFile::GetFileName(arSubDirs[i]); // folder name
				if (sName.empty())
					continue;
				std::wstring sAbsPath = FindPluginLibrary(arSubDirs[i], sName);
				if (sAbsPath.empty())
					continue;

				std::string sKey = U_TO_UTF8(sName);
				if (mapCandidates.find(sKey) == mapCandidates.end())
					arNames.push_back(sKey);
				mapCandidates[sKey].push_back(sAbsPath);
			}
		}

		// Pass 2: one plugin per name, the highest Version() winning.
		for (size_t i = 0; i < arNames.size(); ++i)
			LoadLatest(arNames[i], mapCandidates[arNames[i]]);
	}

	bool CNativePluginManager::LoadLatest(const std::string& sName, const std::vector<std::wstring>& arPaths)
	{
		TLoaded oLatest;
		int nLatestVersion = -1; // below any real version -> the first copy wins ties

		for (size_t i = 0; i < arPaths.size(); ++i)
		{
			TLoaded oCandidate;
			if (!OpenCandidate(arPaths[i], sName, oCandidate))
				continue;

			int nVersion = VersionKey(oCandidate.pPlugin->Version());
			if (nVersion > nLatestVersion)
			{
				if (NULL != oLatest.pPlugin)
					CloseCandidate(oLatest); // beaten, and it never ran
				oLatest = oCandidate;
				nLatestVersion = nVersion;
			}
			else
			{
				CloseCandidate(oCandidate);
			}
		}

		if (NULL == oLatest.pPlugin)
			return false;
		return StartCandidate(sName, oLatest);
	}

	bool CNativePluginManager::OpenCandidate(const std::wstring& sAbsPath, const std::string& sName, TLoaded& oOut)
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

		pPlugin->SetName(sName); // plugin name == its folder name (the registry key)

		oOut.pLoader = pLoader;
		oOut.pPlugin = pPlugin;
		oOut.fnDestroy = fnDestroy;
		return true;
	}

	void CNativePluginManager::CloseCandidate(TLoaded& oEntry)
	{
		// No Stop() here: only Start() spins up anything that would need it.
		if (NULL != oEntry.pPlugin && NULL != oEntry.fnDestroy)
			oEntry.fnDestroy(oEntry.pPlugin);
		if (NULL != oEntry.pLoader)
		{
			oEntry.pLoader->Unload();
			delete oEntry.pLoader;
		}
		oEntry = TLoaded();
	}

	bool CNativePluginManager::StartCandidate(const std::string& sName, TLoaded& oEntry)
	{
		std::lock_guard<std::recursive_mutex> oLock(m_oMutex);
		// A name already taken keeps its plugin (LoadAll called twice): starting a
		// second one would leak the first and double its threads.
		if (m_mapPlugins.find(sName) != m_mapPlugins.end())
		{
			CloseCandidate(oEntry);
			return false;
		}

		m_pStarting = oEntry.pPlugin;
		oEntry.pPlugin->Start(m_pHost); // may call host->RegisterJsMethod -> RegisterJsMethod()
		m_pStarting = NULL;

		m_mapPlugins[sName] = oEntry;
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
