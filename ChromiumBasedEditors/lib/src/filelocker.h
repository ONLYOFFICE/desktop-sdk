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

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_MAC)
#include <fcntl.h>
#endif

#if defined(LINUX) && !defined(_MAC)
#define USE_GIO_FILE
#include <gio/gio.h>
#endif

#include <iostream>
#include "../../../core/DesktopEditor/common/File.h"
#include "../../../core/DesktopEditor/common/Types.h"

namespace NSSystem
{
class CFileLocker
{
public:
	enum LockType {
		ltNone     = 0x00,
		ltReadOnly = 0x01,
		ltLocked   = 0x02,
		ltNosafe   = 0x04
	};

protected:
	std::wstring m_sFile;

#if defined(_WIN32)
	HANDLE m_nDescriptor;
#endif

#if defined(_MAC)
	int m_nDescriptor;
#endif

#if defined(USE_GIO_FILE)
	GFile* m_pFile;
	GError* m_pError;
	char* m_pFileUri;
	GOutputStream* m_pOutputStream;
#endif

public:
	CFileLocker(std::wstring sFile)
	{
		m_sFile = sFile;

#if defined(_WIN32)
		m_nDescriptor = INVALID_HANDLE_VALUE;
#endif

#if defined(_MAC)
		m_nDescriptor = -1;
#endif

#if defined(USE_GIO_FILE)
		m_pFile = NULL;
		m_pError = NULL;
		m_pFileUri = NULL;
		m_pOutputStream = NULL;
#endif
	}

	bool Lock()
	{
		bool bResult = true;
		std::string sFileA = U_TO_UTF8(m_sFile);

#if defined(_WIN32)
		std::wstring sFileFull = CorrectPathW(m_sFile);
		DWORD dwFileAttributes = 0; //GetFileAttributesW(sFileFull.c_str());
		m_nDescriptor = CreateFileW(sFileFull.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFileAttributes, NULL);
		if (m_nDescriptor != NULL && m_nDescriptor != INVALID_HANDLE_VALUE)
		{
			// TODO: проверить/удалить этот код, лок файла работает
			//LARGE_INTEGER lFileSize;
			//GetFileSizeEx(m_nDescriptor, &lFileSize);
			//LockFile(m_nDescriptor, 0, 0, lFileSize.LowPart, (DWORD)lFileSize.HighPart);
		}
		else
		{
			m_nDescriptor = INVALID_HANDLE_VALUE;
			bResult = false;
		}
#endif

#if defined(_MAC)
		m_nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
		if (-1 == m_nDescriptor)
			return true;

		struct flock _lock;
		memset(&_lock, 0, sizeof(_lock));
		_lock.l_type   = F_WRLCK;
		_lock.l_whence = SEEK_SET;
		_lock.l_start  = 0;
		_lock.l_len    = 0;
		_lock.l_pid    = getpid();

		fcntl(m_nDescriptor, F_SETLKW, &_lock);

		if (ltNone == IsLocked(m_sFile))
		{
			close(m_nDescriptor);
			m_nDescriptor = -1;
			bResult = false;
		}
#endif

#if defined(USE_GIO_FILE)
		m_pFile = g_file_new_for_commandline_arg(sFileA.c_str());
		if ( m_pFile )
		{
			m_pFileUri = g_file_get_uri(m_pFile);
			m_pOutputStream = G_OUTPUT_STREAM(g_file_append_to(m_pFile, G_FILE_CREATE_PRIVATE, NULL, &m_pError));
			bResult = !m_pError;
		}
#endif

		return  bResult;
	}

	bool Unlock()
	{
		bool bResult = false;
		std::string sFileA = U_TO_UTF8(m_sFile);

#if defined(_WIN32)
		if (INVALID_HANDLE_VALUE != m_nDescriptor)
		{
			LARGE_INTEGER lFileSize;
			GetFileSizeEx(m_nDescriptor, &lFileSize);
			UnlockFile(m_nDescriptor, 0, 0, lFileSize.LowPart, (DWORD)lFileSize.HighPart);

			CloseHandle(m_nDescriptor);
			m_nDescriptor = INVALID_HANDLE_VALUE;
			bResult = true;
		}
#endif

#if defined(_MAC)
		if (-1 == m_nDescriptor)
			return true;

		struct flock _lock;
		memset(&_lock, 0, sizeof(_lock));
		_lock.l_type   = F_UNLCK;
		_lock.l_whence = SEEK_SET;
		_lock.l_start  = 0;
		_lock.l_len    = 0;
		_lock.l_pid    = getpid();

		fcntl(m_nDescriptor, F_SETLKW, &_lock);
		close(m_nDescriptor);
		m_nDescriptor = -1;
		bResult = true;
#endif

#if defined(USE_GIO_FILE)
		bResult = ReleaseGio();
#endif

		return  bResult;
	}

	void CheckDescriptor()
	{
		std::string sFileA = U_TO_UTF8(m_sFile);

#if defined(_WIN32)
#endif

#if defined(_MAC)
		if (-1 == m_nDescriptor)
		{
			m_nDescriptor = open(sFileA.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if (-1 == m_nDescriptor)
				m_nDescriptor = open(sFileA.c_str(), O_CREAT | O_WRONLY, 0666);
		}
#endif

#if defined(USE_GIO_FILE)
#endif
	}

	static LockType IsLocked(const::std::wstring& sFile)
	{
		LockType isLocked = ltNone;
		std::string sFileA = U_TO_UTF8(sFile);

#if defined(_WIN32)
		HANDLE hFile = CreateFileW(sFile.c_str(),                  // открываемый файл
								   GENERIC_READ | GENERIC_WRITE,   // открываем для чтения и записи
								   0,                              // для совместного чтения
								   NULL,                           // защита по умолчанию
								   OPEN_EXISTING,                  // только существующий файл
								   FILE_ATTRIBUTE_NORMAL,          // обычный файл
								   NULL);                          // атрибутов шаблона нет
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD fileAttr = GetFileAttributesW(sFile.c_str());
			if (INVALID_FILE_ATTRIBUTES != fileAttr)
			{
				if (fileAttr & FILE_ATTRIBUTE_READONLY)
					isLocked = ltReadOnly;
				else
					isLocked = ltLocked;
			}
		}
		CloseHandle(hFile);
#endif

#if defined(_MAC)
		if (0 != access(sFileA.c_str(), W_OK) && 0 == access(sFileA.c_str(), R_OK))
		{
			isLocked = ltReadOnly;
			return isLocked;
		}

		int nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
		if (-1 == nDescriptor)
			return ltNone;

		struct flock _lock;
		memset(&_lock, 0, sizeof(_lock));
		fcntl(nDescriptor, F_GETLK, &_lock);
		if (F_WRLCK == (_lock.l_type & F_WRLCK))
			isLocked = ltLocked;
		close(nDescriptor);
#endif

#if defined(USE_GIO_FILE)
		CCrossPlatformFileLocker* pLocker = new CCrossPlatformFileLocker(sFile);
		if ( !pLocker->Lock() )
			isLocked = ltReadOnly;	// ltLocked

		pLocker->ReleaseGio();
		delete pLocker;
		pLocker = NULL;
#endif

		return isLocked;
	}

	bool SeekFile(DWORD dwPosition)
	{
		bool bResult = false;

#if defined(_WIN32)
		if ( m_nDescriptor != INVALID_HANDLE_VALUE )
			bResult = SetFilePointer(m_nDescriptor, (LONG)dwPosition, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
#endif

#if defined(_MAC)
		if ( m_nDescriptor != -1 )
			bResult = lseek(m_nDescriptor, dwPosition, SEEK_SET) != -1;
#endif

#if defined(USE_GIO_FILE)
		if ( m_pFile )
		{
			if ( m_pOutputStream )
			{
				if ( g_seekable_can_seek((GSeekable*)m_pOutputStream) )
				{
					if ( g_seekable_seek((GSeekable*)m_pOutputStream, dwPosition, G_SEEK_SET, NULL, &m_pError) )
						bResult = !m_pError;
				}
			}
		}
#endif

		return  bResult;
	}

	bool Truncate(DWORD dwPosition)
	{
		bool bResult = false;

#if defined(_WIN32)
		if ( m_nDescriptor != INVALID_HANDLE_VALUE )
			bResult = SetEndOfFile(m_nDescriptor) == TRUE;
#endif

#if defined(_MAC)
		if ( m_nDescriptor != -1 )
			bResult = ftruncate(nDescriptor, nFileSize) != -1;
#endif

#if defined(USE_GIO_FILE)
		if ( m_pFile )
		{
			if ( m_pOutputStream )
			{
				if ( g_seekable_can_truncate((GSeekable*)m_pOutputStream) )
				{
					if ( g_seekable_truncate((GSeekable*)m_pOutputStream, dwPosition, NULL, &m_pError) )
						bResult = !m_pError;
				}
			}
		}
#endif

		return  bResult;
	}

	bool WriteFile(const void* pData, DWORD dwBytesToWrite, DWORD& dwSizeWrite)
	{
		bool bResult = false;

#if defined(_WIN32)
		if ( m_nDescriptor != INVALID_HANDLE_VALUE )
			bResult = ::WriteFile(m_nDescriptor, pData, dwBytesToWrite, &dwSizeWrite, NULL) == TRUE;
#endif

#if defined(_MAC)
		if ( m_nDescriptor != -1 )
		{
			dwSizeWrite = (DWORD)write(m_nDescriptor, pData, dwBytesToWrite);
			bResult = dwSizeWrite != -1;
		}
#endif

#if defined(USE_GIO_FILE)
		if ( m_pFile && pData && m_pOutputStream )
			bResult = g_output_stream_write_all(m_pOutputStream, pData, dwBytesToWrite, &dwSizeWrite, NULL, &m_pError);
#endif

		return  bResult;
	}

	bool ReleaseGio()
	{
		bool bResult = false;

#if defined(USE_GIO_FILE)
		if ( m_pFile )
		{
			if ( m_pOutputStream )
			{
				g_output_stream_close(m_pOutputStream, NULL, &m_pError);
				g_object_unref(m_pOutputStream);
			}
			if ( m_pFileUri )
			{
				g_free(m_pFileUri);
				m_pFileUri = NULL;
			}
			if ( m_pFile )
			{
				g_object_unref(m_pFile);
				m_pFile = NULL;
			}
			bResult = true;
		}
#endif
		return bResult;
	}
};
}

#endif // FILELOCKER_H
