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

		return returnValue.dump();
	}
}
