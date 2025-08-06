/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#ifndef FILELOCKER_H
#define FILELOCKER_H

#include <iostream>
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/DesktopEditor/common/Types.h"

namespace NSSystem
{
	enum class LockType
	{
		ltNone     = 0x00,
		ltReadOnly = 0x01,
		ltLocked   = 0x02,
		ltNosafe   = 0x04
	};

	class CLockFileTemp
	{
	private:
		std::string m_user;
		std::string m_host;
		std::string m_date;
		std::string m_user_dir;

		std::wstring m_file;

	public:
		CLockFileTemp(const std::wstring& file);
		~CLockFileTemp();
		CLockFileTemp(const CLockFileTemp& file);
		CLockFileTemp& operator=(const CLockFileTemp& file);

	public:
		void Generate();
		void Save(int type = 0);
		void Load();
		bool IsEqual(const CLockFileTemp& lock);

		std::wstring GetPath();
	};

	class CFileLocker
	{
	public:
		std::wstring m_sFile;
		std::wstring m_sLockFilePath;

		CFileLocker(const std::wstring& file);
		virtual ~CFileLocker();

		static CLockFileTemp CheckLockFilePath(const std::wstring& file, const int& flags = 0);
		virtual void DeleteLockFile();

		virtual bool Lock()   = 0;
		virtual bool Unlock() = 0;

		virtual bool StartWrite();
		virtual bool SeekFile(DWORD dwPosition) = 0;
		virtual bool Truncate(DWORD dwPosition) = 0;
		virtual bool WriteFile(const void* pData, DWORD dwBytesToWrite, DWORD& dwSizeWrite) = 0;

		virtual bool IsEmpty();

		static LockType IsLocked(const std::wstring& file);
		static CFileLocker* Create(const std::wstring& file);

		static bool IsHandled(const std::wstring& file);

		static bool RemoveRestrictionFlags(const std::wstring& file);
	};
}

#endif // FILELOCKER_H
