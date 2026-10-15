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
#include "../../../../../../core/DesktopEditor/common/StringBuilder.h"

namespace recent_files_reader
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"recent_files_reader\",\
  \"description\": \"Use this function if you are asked to get a list of recently used files. Input: optional number of files to return. Output: an object describing the result of the operation. The files are sorted with the most recently used files appearing first.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\", // Required constant field\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"files\\\": [ { \\\"path\\\": \\\"string\\\", \\\"type\\\": \\\"integer\\\" } ], // Array of objects with file path and type\\n\\\"error_message\\\": \\\"string | null\\\" // Error details if status = 'error'\\n}\\n\\nExample function call parameter: { \\\"limit\\\": 5 }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"files\\\": [ { \\\"path\\\": \\\"/home/user/file1.txt\\\", \\\"type\\\": 0 }, { \\\"path\\\": \\\"/home/user/file2.docx\\\", \\\"type\\\": 1 } ],\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"files\\\": [],\\n\\\"error_message\\\": \\\"Recent files history not available.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"limit\": {\
        \"type\": \"integer\",\
        \"description\": \"Optional maximum number of recent files to return.\",\
        \"default\": 10\
      }\
    }\
  },\
  \"annotations\": {\
    \"readOnlyHint\": true,\
    \"destructiveHint\": false,\
    \"openWorldHint\": false\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		std::vector<CRecentFileInfo> files = helper->GetRecents();

		json jFiles = json::array();

		for (std::vector<CRecentFileInfo>::const_iterator i = files.begin(); i != files.end(); i++)
		{
			json jObj = json::object();
			jObj["path"] = U_TO_UTF8((i->Path));
			jObj["type"] = i->Type;
			if (!i->Url.empty())
				jObj["url"] = i->Url;
			jFiles.push_back(jObj);
		}

		returnValue["status"] = "success";
		returnValue["files"] = jFiles;

		return JSON_serialize(returnValue);
	}
}
