/*
 * (c) Copyright Ascensio System SIA 2010-2026
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DESKTOP_SDK_NATIVE_PLUGINS_LIB_LOADER_H
#define DESKTOP_SDK_NATIVE_PLUGINS_LIB_LOADER_H

#include <string>

namespace NSNativePlugin
{
	// Minimal cross-platform dynamic-library wrapper (no reusable one exists in
	// core/desktop-sdk). Windows: LoadLibraryW(full path)/GetProcAddress/FreeLibrary;
	// POSIX: dlopen(RTLD_NOW|RTLD_LOCAL)/dlsym/dlclose.
	class CLibLoader
	{
	public:
		CLibLoader();
		~CLibLoader();

		bool Load(const std::wstring& sAbsPath);
		void* Sym(const char* sName);
		void Unload();
		bool IsLoaded() const;

	private:
		CLibLoader(const CLibLoader&);
		CLibLoader& operator=(const CLibLoader&);

		void* m_handle;
	};
}

#endif // DESKTOP_SDK_NATIVE_PLUGINS_LIB_LOADER_H
