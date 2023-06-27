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

#ifndef CROSSPLATFORMFILE_H
#define CROSSPLATFORMFILE_H


#if defined(LINUX) && !defined(_MAC)
#define USE_GIO_FILE
#include <gio/gio.h>
#include "../../../core/DesktopEditor/common/Types.h"

namespace NSSystem
{
class CFileGio
{
protected:
	GFile* m_pFile;
	GError* m_pError;
	char* m_pFileUri;

	GInputStream* m_pInputStream;
	GOutputStream* m_pOutputStream;

	DWORD m_lFileSize;
	DWORD m_lFilePosition;

public:
	CFileGio()
	{
		m_pFile = NULL;
		m_pError = NULL;
		m_pFileUri = NULL;

		m_pInputStream = NULL;
		m_pOutputStream = NULL;

		m_lFileSize = 0;
		m_lFilePosition = 0;
	}

	bool OpenFile(const char* sFileName, bool bReadOnly = true)
	{
		// Одна функция для открытия файла, типа open в C
		// Если только чтение, то g_file_read
		// Иначе g_file_append_to - в этом случае получаем поток для записи и не пересоздаём файл, что может быть критично.
		// Указывая G_FILE_CREATE_PRIVATE, обеспечиваем блокировку файла текущим пользователем

		bool bResult = false;
		m_pFile = g_file_new_for_commandline_arg(sFileName);

		if ( m_pFile )
		{
			m_pFileUri = g_file_get_uri(m_pFile);

			if ( bReadOnly )
				m_pInputStream = G_INPUT_STREAM(g_file_read(m_pFile, NULL, &m_pError));
			else
				m_pOutputStream = G_OUTPUT_STREAM(g_file_append_to(m_pFile, G_FILE_CREATE_PRIVATE, NULL, &m_pError));

			bResult = !m_pError;
		}

		return  bResult;
	}

	DWORD GetFileSize()
	{
		if ( m_pFile )
		{
			GFileInfo* pFileInfo = g_file_query_info(m_pFile, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, &m_pError);
			if ( pFileInfo )
			{
				m_lFileSize = g_file_info_get_size(pFileInfo);
				g_object_unref(pFileInfo);
			}
		}

		return m_lFileSize;
	}

	long GetFilePosition()
	{
		return m_lFilePosition;
	}

	bool SeekFile(DWORD lPosition, GSeekType seekType = G_SEEK_SET)
	{
		bool bResult = false;

		if ( m_pFile )
		{
			if ( m_pInputStream )
			{
				if ( g_seekable_can_seek((GSeekable*)m_pInputStream) )
				{
					if ( g_seekable_seek((GSeekable*)m_pInputStream, lPosition, seekType, NULL, &m_pError) )
					{
						m_lFilePosition = lPosition;
						bResult = !m_pError;
					}
				}
			}
			if ( m_pOutputStream )
			{
				if ( g_seekable_can_seek((GSeekable*)m_pOutputStream) )
				{
					if ( g_seekable_seek((GSeekable*)m_pOutputStream, lPosition, seekType, NULL, &m_pError) )
					{
						m_lFilePosition = lPosition;
						bResult = !m_pError;
					}
				}
			}
		}

		return  bResult;
	}

	bool Truncate(DWORD lPosition)
	{
		bool bResult = false;

		if ( m_pFile )
		{
			if ( m_pOutputStream )
			{
				if ( g_seekable_can_truncate((GSeekable*)m_pOutputStream) )
				{
					if ( g_seekable_truncate((GSeekable*)m_pOutputStream, lPosition, NULL, &m_pError) )
					{
						m_lFilePosition = lPosition;
						bResult = !m_pError;
					}
				}
			}
		}

		return  bResult;
	}

	bool ReadFile(BYTE* pData, DWORD nBytesToRead, DWORD& nSizeRead)
	{
		bool bResult = false;

		if ( m_pFile && m_pInputStream )
		{
			bResult = g_input_stream_read_all(m_pInputStream, pData, nBytesToRead, &nSizeRead, NULL, &m_pError);
		}

		return  bResult;
	}

	bool WriteFile(const void* pData, DWORD nBytesToWrite, DWORD& nSizeWrite)
	{
		DWORD bResult = false;

		if ( m_pFile && pData && m_pOutputStream )
		{
			bResult = g_output_stream_write_all(m_pOutputStream, pData, nBytesToWrite, &nSizeWrite, NULL, &m_pError);
		}

		return  bResult;
	}

	bool Close()
	{
		bool bResult = false;

		if ( m_pFile )
		{
			if ( m_pInputStream )
			{
				g_input_stream_close(m_pInputStream, NULL, &m_pError);
				g_object_unref(m_pInputStream);
			}
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

		return  bResult;
	}
};
}
#endif

#endif // CROSSPLATFORMFILE_H
