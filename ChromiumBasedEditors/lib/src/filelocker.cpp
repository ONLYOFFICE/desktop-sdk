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

#include "./filelocker.h"
#include <set>

class CHandlesMonitor
{
private:
	std::set<std::wstring> m_arHandles;

public:
	CHandlesMonitor()
	{
	}
	~CHandlesMonitor()
	{
	}

	void Add(const std::wstring& file)
	{
		if (!IsExist(file))
			m_arHandles.insert(file);
	}

	void Remove(const std::wstring& file)
	{
		if (IsExist(file))
			m_arHandles.erase(file);
	}

	bool IsExist(const std::wstring& file)
	{
		return (m_arHandles.find(file) != m_arHandles.end()) ? true : false;
	}

	static CHandlesMonitor& Instance()
	{
		static CHandlesMonitor singleton;
		return singleton;
	}
};

#if defined(_WIN32)
#include <windows.h>

namespace NSSystem
{
	class CFileLockerWin : public CFileLocker
	{
		HANDLE m_nDescriptor;

	public:
		CFileLockerWin(const std::wstring& file) : CFileLocker(file)
		{
			m_nDescriptor = INVALID_HANDLE_VALUE;
		}
		virtual ~CFileLockerWin()
		{
			Unlock();
		}

		virtual bool Lock()
		{
			bool bResult = true;
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
			return bResult;
		}

		virtual bool Unlock()
		{
			bool bResult = false;

			if (INVALID_HANDLE_VALUE != m_nDescriptor)
			{
				LARGE_INTEGER lFileSize;
				GetFileSizeEx(m_nDescriptor, &lFileSize);
				UnlockFile(m_nDescriptor, 0, 0, lFileSize.LowPart, (DWORD)lFileSize.HighPart);

				CloseHandle(m_nDescriptor);
				m_nDescriptor = INVALID_HANDLE_VALUE;
				bResult = true;
			}

			return bResult;
		}

		virtual bool SeekFile(DWORD dwPosition)
		{
			bool bResult = false;
			if ( m_nDescriptor != INVALID_HANDLE_VALUE )
				bResult = SetFilePointer(m_nDescriptor, (LONG)dwPosition, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
			return bResult;
		}

		virtual bool Truncate(DWORD dwPosition)
		{
			bool bResult = false;
			if ( m_nDescriptor != INVALID_HANDLE_VALUE )
				bResult = SetEndOfFile(m_nDescriptor) == TRUE;
			return bResult;
		}

		virtual bool WriteFile(const void* pData, DWORD dwBytesToWrite, DWORD& dwSizeWrite)
		{
			bool bResult = false;
			if ( m_nDescriptor != INVALID_HANDLE_VALUE )
				bResult = ::WriteFile(m_nDescriptor, pData, dwBytesToWrite, &dwSizeWrite, NULL) == TRUE;
			return bResult;
		}

	public:
		static LockType IsLockedInternal(const std::wstring& file)
		{
			LockType lockType = LockType::ltNone;
			HANDLE hFile = CreateFileW(file.c_str(),                  // открываемый файл
			                           GENERIC_READ | GENERIC_WRITE,  // открываем для чтения и записи
			                           0,                             // для совместного чтения
			                           NULL,                          // защита по умолчанию
			                           OPEN_EXISTING,                 // только существующий файл
			                           FILE_ATTRIBUTE_NORMAL,         // обычный файл
			                           NULL);                         // атрибутов шаблона нет
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD fileAttr = GetFileAttributesW(file.c_str());
				if (INVALID_FILE_ATTRIBUTES != fileAttr)
				{
					if (fileAttr & FILE_ATTRIBUTE_READONLY)
						lockType = LockType::ltReadOnly;
					else
						lockType = LockType::ltLocked;
				}
			}
			CloseHandle(hFile);
			return lockType;
		}
	};
}

#endif

#ifdef _LINUX

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace NSSystem
{
	class CFileLockerEmpty : public CFileLocker
	{
	protected:
		int m_nDescriptor;

	public:
		CFileLockerEmpty(const std::wstring& file) : CFileLocker(file)
		{
			m_nDescriptor = -1;
		}
		virtual ~CFileLockerEmpty()
		{
			Unlock();
		}

		virtual bool Lock()
		{
			std::string sFileA = U_TO_UTF8(m_sFile);
			m_nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
			if (-1 == m_nDescriptor)
				return true;
			return true;
		}

		virtual bool Unlock()
		{
			if (-1 == m_nDescriptor)
				return true;
			close(m_nDescriptor);
			m_nDescriptor = -1;
			return true;
		}

		virtual bool SeekFile(DWORD dwPosition)
		{
			bool bResult = false;
			if ( m_nDescriptor != -1 )
				bResult = lseek(m_nDescriptor, dwPosition, SEEK_SET) != -1;
			return bResult;
		}

		virtual bool Truncate(DWORD dwPosition)
		{
			bool bResult = false;
			if ( m_nDescriptor != -1 )
				bResult = ftruncate(m_nDescriptor, (off_t)dwPosition) != -1;
			return bResult;
		}

		virtual bool WriteFile(const void* pData, DWORD dwBytesToWrite, DWORD& dwSizeWrite)
		{
			if (!NSFile::CFileBinary::Exists(m_sFile))
			{
				Unlock();
				std::wstring sDirectory = NSFile::GetDirectoryName(m_sFile);
				NSDirectory::CreateDirectories(sDirectory);

				NSFile::CFileBinary oFile;
				oFile.CreateFile(m_sFile);
				oFile.CloseFile();

				Lock();
			}
			bool bResult = false;
			if ( m_nDescriptor != -1 )
			{
				dwSizeWrite = (DWORD)write(m_nDescriptor, pData, dwBytesToWrite);
				bResult = dwSizeWrite != -1;
			}
			return bResult;
		}

		virtual bool IsEmpty()
		{
			return true;
		}
	};

	class CFileLockerFCNTL : public CFileLockerEmpty
	{
	public:
		CFileLockerFCNTL(const std::wstring& file) : CFileLockerEmpty(file)
		{
		}
		virtual ~CFileLockerFCNTL()
		{
			Unlock();
		}

		virtual bool Lock()
		{
			bool bResult = true;
			std::string sFileA = U_TO_UTF8(m_sFile);

			m_nDescriptor = open(sFileA.c_str(), O_WRONLY | O_EXCL);
			if (-1 == m_nDescriptor)
				return true;

			struct flock _lock;
			memset(&_lock, 0, sizeof(_lock));
			_lock.l_type   = F_WRLCK;
			_lock.l_whence = SEEK_SET;
			_lock.l_start  = 0;
			_lock.l_len    = 0;
			_lock.l_pid    = getpid();

			bResult = (0 == fcntl(m_nDescriptor, F_SETLKW, &_lock));

			CHandlesMonitor::Instance().Add(m_sFile);
			return  bResult;
		}

		virtual bool Unlock()
		{
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

			CHandlesMonitor::Instance().Remove(m_sFile);
			return true;
		}

	public:
		static LockType IsLockedInternal(const std::wstring& file)
		{
			LockType lockType = LockType::ltNone;
			std::string sFileA = U_TO_UTF8(file);

			if (0 != access(sFileA.c_str(), W_OK) && 0 == access(sFileA.c_str(), R_OK))
			{
				lockType = LockType::ltReadOnly;
				return lockType;
			}

			int nDescriptor = open(sFileA.c_str(), O_RDWR | O_EXCL);
			if (-1 == nDescriptor)
				return LockType::ltNone;

			struct flock _lock;
			memset(&_lock, 0, sizeof(_lock));
			fcntl(nDescriptor, F_GETLK, &_lock);
			if (F_WRLCK == (_lock.l_type & F_WRLCK))
				lockType = LockType::ltLocked;
			close(nDescriptor);
			return lockType;
		}
	};
}

#if defined(_LINUX) && !defined(_MAC)

#include <gio/gio.h>

namespace NSSystem
{
	class CFileLockerGIO : public CFileLocker
	{
		GFile* m_pFile;
		char* m_pFileUri;
		GOutputStream* m_pOutputStream;

	public:
		CFileLockerGIO(const std::wstring& file) : CFileLocker(file)
		{
			m_pFile = NULL;
			m_pFileUri = NULL;
			m_pOutputStream = NULL;
		}
		virtual ~CFileLockerGIO()
		{
			Unlock();
		}

		virtual bool Lock()
		{
			bool bResult = true;
			std::string sFileA = U_TO_UTF8(m_sFile);

			m_pFile = g_file_new_for_commandline_arg(sFileA.c_str());
			if ( m_pFile )
			{
				m_pFileUri = g_file_get_uri(m_pFile);
				GError* err = NULL;
				m_pOutputStream = G_OUTPUT_STREAM(g_file_append_to(m_pFile, G_FILE_CREATE_PRIVATE, NULL, &err));
				bResult = !err;

				if (err)
					g_error_free (err);
			}

			return  bResult;
		}

		virtual bool Unlock()
		{
			bool bResult = false;

			if (m_pFile)
			{
				if (m_pOutputStream)
				{
					GError* err = NULL;
					g_output_stream_close(m_pOutputStream, NULL, &err);
					g_object_unref(m_pOutputStream);

					if (err)
						g_error_free (err);
				}
				if (m_pFileUri)
				{
					g_free(m_pFileUri);
					m_pFileUri = NULL;
				}
				if (m_pFile)
				{
					g_object_unref(m_pFile);
					m_pFile = NULL;
				}
				bResult = true;
			}

			return bResult;
		}

		virtual bool SeekFile(DWORD dwPosition)
		{
			bool bResult = false;
			if (m_pFile && m_pOutputStream)
			{
				if (g_seekable_can_seek((GSeekable*)m_pOutputStream))
				{
					GError* err = NULL;
					if (g_seekable_seek((GSeekable*)m_pOutputStream, dwPosition, G_SEEK_SET, NULL, &err))
						bResult = !err;

					if (err)
						g_error_free (err);
				}
			}
			return bResult;
		}

		virtual bool Truncate(DWORD dwPosition)
		{
			bool bResult = false;
			if (m_pFile && m_pOutputStream)
			{
				if (g_seekable_can_truncate((GSeekable*)m_pOutputStream))
				{
					GError* err = NULL;
					if (g_seekable_truncate((GSeekable*)m_pOutputStream, dwPosition, NULL, &err))
						bResult = !err;

					if (err)
						g_error_free (err);
				}
			}
			return bResult;
		}

		virtual bool WriteFile(const void* pData, DWORD dwBytesToWrite, DWORD& dwSizeWrite)
		{
			bool bResult = false;
			if (m_pFile && pData && m_pOutputStream)
			{
				GError* err = NULL;
				bResult = g_output_stream_write_all(m_pOutputStream, pData, dwBytesToWrite, &dwSizeWrite, NULL, &err);
				if (err)
					g_error_free (err);
			}
			return bResult;
		}

		static LockType IsLockedInternal(const std::wstring& file)
		{
			LockType lockType = LockType::ltNone;
			CFileLockerGIO* pLocker = new CFileLockerGIO(file);
			if (!pLocker->Lock())
				lockType = LockType::ltReadOnly;	// ltLocked
			delete pLocker;
			return lockType;
		}
	};
}

namespace NSSystem
{
	bool IsLocalFile(const std::wstring& file)
	{
		bool bLocal = false;
		std::string sFileA = U_TO_UTF8(file);
		GFile* pFile = g_file_new_for_commandline_arg(sFileA.c_str());
		if (pFile)
		{
			char* pFileUri = g_file_get_uri(pFile);
			std::string sFileUri(pFileUri);
			if (pFileUri)
				g_free(pFileUri);

			std::string::size_type pos = sFileUri.find("file://", 0);
			bLocal = pos == 0;

			if (pFile)
				g_object_unref(pFile);
		}
		return bLocal;
	}
}

#endif

#endif

namespace NSSystem
{
	LockType CFileLocker::IsLocked(const std::wstring& file)
	{
		LockType lockType = LockType::ltNone;

#ifdef _WIN32
		lockType = NSSystem::CFileLockerWin::IsLockedInternal(file);
#endif

#ifdef _LINUX
#ifndef _MAC
		if (IsLocalFile(file))
			lockType = NSSystem::CFileLockerFCNTL::IsLockedInternal(file);
		else
			lockType = NSSystem::CFileLockerGIO::IsLockedInternal(file);
#endif
#endif
		return lockType;
	}

	CFileLocker* CFileLocker::Create(const std::wstring& file)
	{
	#ifdef _WIN32
		return new NSSystem::CFileLockerWin(file);
	#endif

	#ifdef _LINUX
	#ifndef _MAC
		if (IsLocalFile(file))
			return new NSSystem::CFileLockerFCNTL(file);
		else
			return new NSSystem::CFileLockerGIO(file);
	#else
		return new NSSystem::CFileLockerEmpty(file);
	#endif
    #endif

		// ERROR!!!
		return NULL;
	}

	CFileLocker::CFileLocker(const std::wstring& file)
	{
		m_sFile = file;
	}
	CFileLocker::~CFileLocker()
	{
	}

	bool CFileLocker::IsEmpty()
	{
		return false;
	}

	bool CFileLocker::IsHandled(const std::wstring& file)
	{
		return CHandlesMonitor::Instance().IsExist(file);
	}

	bool CFileLocker::RemoveRestrictionFlags(const std::wstring& file)
	{
#ifdef _LINUX
		std::string fileA = U_TO_UTF8(file);
		struct stat fileStat;
		if (stat(fileA.c_str, &fileStat) != 0)
		{
			return false;
		}

		if (fileStat.st_mode & S_IWUSR)
			return true;

		if (chmod(fileA.c_str(), fileStat.st_mode | S_IWUSR) != 0)
		{
			return false;
		}
		return true;
#else
		DWORD attrs = GetFileAttributesW(file.c_str());
		if (attrs == INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}

		if (0 == (attrs & FILE_ATTRIBUTE_READONLY))
			return true;

		if (!SetFileAttributesW(file.c_str(), attrs &= ~FILE_ATTRIBUTE_READONLY))
		{
			return false;
		}

		return true;
#endif

	}
}
