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

#include "./../internal/base.h"
#include "../../../../../../core/DesktopEditor/common/Directory.h"

namespace file_opener
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"file_opener\",\
  \"description\": \"Show a file to the USER by opening it in an editor window. Input: the FULL path of the file. Output: only whether the window opened.\\n\\nThe path must be complete, from the drive or the root folder down to the file name: C:\\\\Users\\\\name\\\\Documents\\\\report.docx on Windows, /Users/name/Documents/report.docx on macOS, /home/name/Documents/report.docx on Linux. A bare file name or a relative path is refused - there is no folder it could be looked for in - and so is a path where no file exists; either way nothing opens and the error says why. A tool that creates or saves a file reports the full path it wrote to: pass that path exactly as reported, never shortened to the file name.\\n\\nThis function does NOT read the file: none of its content comes back to you, so calling it teaches you nothing about what is inside. Use it only when the user asked to SEE a file.\\n\\nTo find out what is inside a file, do not call this one:\\n- a text file (.md, .txt, .csv, .json, .log, ...) -> read it with run_script (cat on macOS/Linux, Get-Content on Windows);\\n- a document (.docx, .xlsx, .pptx, .pdf, .odt, .ods, .odp, .rtf, .epub, .xps, .djvu, ...) -> open it with the document tools, which return its text, cells or form fields. They read far more formats than the four obvious ones, so hand the file over instead of assuming it is unsupported.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\", // Required constant field\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"error_message\\\": \\\"string | null\\\" // Error details if status = 'error'\\n}\\n\\nExample function call parameter: { \\\"path\\\": \\\"/home/user/document.txt\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"error_message\\\": \\\"File not found: /home/user/document.txt. Nothing was opened. Check the full path and call file_opener again with the path of a file that exists.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"path\": {\
        \"type\": \"string\",\
        \"description\": \"The FULL path of the file to open - not just its name.\"\
      }\
    },\
    \"required\": [\
      \"path\"\
    ]\
  },\
  \"annotations\": {\
    \"readOnlyHint\": true,\
    \"destructiveHint\": false,\
    \"openWorldHint\": false\
  }\
}";
	}

	// A complete path of the kind this machine uses, for the error messages.
#if defined(_WIN32)
	static const char* kPathExample = "C:\\Users\\name\\Documents\\report.docx";
#elif defined(_MAC)
	// _MAC comes first: macOS defines _LINUX as well.
	static const char* kPathExample = "/Users/name/Documents/report.docx";
#else
	static const char* kPathExample = "/home/name/Documents/report.docx";
#endif

	// A complete path: from a drive letter or a UNC share on Windows, from the
	// root folder on macOS and Linux. Anything else would be looked for
	// relative to the application's own working folder.
	static bool IsFullPath(const std::string& sPath)
	{
#if defined(_WIN32)
		char c0 = sPath.length() > 0 ? sPath[0] : 0;
		if (sPath.length() >= 3 && ((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z')) &&
		    ':' == sPath[1] && ('\\' == sPath[2] || '/' == sPath[2]))
			return true;
		return sPath.length() >= 2 && ('\\' == sPath[0] || '/' == sPath[0]) &&
		       ('\\' == sPath[1] || '/' == sPath[1]);
#else
		return !sPath.empty() && '/' == sPath[0];
#endif
	}

	static std::string Refuse(json& returnValue, const std::string& sMessage)
	{
		returnValue["status"] = "error";
		returnValue["error_message"] = sMessage;
		return returnValue.dump();
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		json param = json::parse(arg);
		if (!param.contains("path") || !param["path"].is_string() ||
		    param["path"].get<std::string>().empty())
			return Refuse(returnValue, std::string("No path was given. Nothing was opened. Pass the FULL "
			                                       "path of the file in \"path\", for example ") +
			                               kPathExample + ".");

		std::string pathA = param["path"];
		std::wstring path = UTF8_TO_U(pathA);

		// Opening is handed to the application and answers nothing back, so a
		// path it cannot open is caught here or not at all - and "success"
		// would be reported for a window that never appears.
		//
		// A name with no folder in it is what a model passes when it only saw
		// the file's name: there is nowhere it could be looked for.
		if (std::string::npos == pathA.find_first_of("/\\"))
			return Refuse(returnValue, "Only a file name was given: \"" + pathA + "\". file_opener "
			                           "needs the FULL path of the file, for example " +
			                           std::string(kPathExample) + ". Nothing was opened. Use the "
			                           "full path the tool that created or saved the file reported, "
			                           "or find where the file is first.");

		if (!IsFullPath(pathA))
			return Refuse(returnValue, "Not a full path: \"" + pathA + "\". file_opener needs the "
			                           "FULL path of the file, for example " +
			                           std::string(kPathExample) + ". Nothing was opened. Use the "
			                           "full path the tool that created or saved the file reported, "
			                           "or find where the file is first.");

		// Asked before the file test: on Linux a folder opens for reading too.
		if (NSDirectory::Exists(path))
			return Refuse(returnValue, pathA + " is a folder, not a file. Nothing was opened. Give the "
			                           "FULL path of a file.");

		if (!NSFile::CFileBinary::Exists(path))
			return Refuse(returnValue, "File not found: " + pathA + ". Nothing was opened. Check the "
			                           "full path and call file_opener again with the path of a file "
			                           "that exists.");

		returnValue["status"] = "success";

		helper->OpenFile(path);

		return returnValue.dump();
	}
}
