/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./lib_loader.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include "../../../../../core/DesktopEditor/common/File.h"
#endif

namespace NSNativePlugin
{
	CLibLoader::CLibLoader() : m_handle(NULL)
	{
	}

	CLibLoader::~CLibLoader()
	{
		Unload();
	}

	bool CLibLoader::IsLoaded() const
	{
		return NULL != m_handle;
	}

	bool CLibLoader::Load(const std::wstring& sAbsPath)
	{
		Unload();
#ifdef _WIN32
		m_handle = (void*)::LoadLibraryW(sAbsPath.c_str());
#else
		std::string sPathA = U_TO_UTF8(sAbsPath);
		m_handle = dlopen(sPathA.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
		return NULL != m_handle;
	}

	void* CLibLoader::Sym(const char* sName)
	{
		if (NULL == m_handle)
			return NULL;
#ifdef _WIN32
		return (void*)::GetProcAddress((HMODULE)m_handle, sName);
#else
		return dlsym(m_handle, sName);
#endif
	}

	void CLibLoader::Unload()
	{
		if (NULL == m_handle)
			return;
#ifdef _WIN32
		::FreeLibrary((HMODULE)m_handle);
#else
		dlclose(m_handle);
#endif
		m_handle = NULL;
	}
}
