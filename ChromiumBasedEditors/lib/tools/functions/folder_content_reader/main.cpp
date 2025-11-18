#include "./../internal/base.h"
#include "../../../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../../../core/DesktopEditor/common/SystemUtils.h"

namespace folder_content_reader
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"folder_content_reader\",\
  \"description\": \"Use this function to list the contents of a folder. Input: folder name or path. Standard folder names include: 'Documents', 'Downloads', 'Desktop'. Output: an object describing the result of the operation.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"files\\\": [ \\\"array of file names\\\" ],\\n\\\"error_message\\\": \\\"string | null\\\"\\n}\\n\\nExample function call parameter: { \\\"folder\\\": \\\"Documents\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"files\\\": [ \\\"file1.txt\\\", \\\"file2.docx\\\" ],\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"files\\\": [],\\n\\\"error_message\\\": \\\"Folder not found.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"folder\": {\
        \"type\": \"string\",\
        \"description\": \"Name or path of the folder to be listed. Standard names: 'Documents', 'Downloads', 'Desktop'.\"\
      }\
    },\
    \"required\": [\"folder\"]\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		json param = json::parse(arg);
		if (!param.contains("folder") || !param["folder"].is_string())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Folder not presented";
			return returnValue.dump();
		}

		std::string pathA = param["folder"];
		std::wstring path = UTF8_TO_U(pathA);

		NSStringUtils::string_replace(path, L"\\", L"/");

		if (std::wstring::npos == path.find(L"/"))
		{
			NSSystemUtils::SystemDirectoryType dirType = NSSystemUtils::SystemDirectoryType::Undefined;
			if ("Documents" == pathA)
				dirType = NSSystemUtils::SystemDirectoryType::Documents;
			else if ("Downloads" == pathA)
				dirType = NSSystemUtils::SystemDirectoryType::Downloads;
			else if ("Desktop" == pathA)
				dirType = NSSystemUtils::SystemDirectoryType::Desktop;

			if (dirType == NSSystemUtils::SystemDirectoryType::Undefined)
			{
				returnValue["status"] = "error";
				returnValue["error_message"] = "Folder not found";
				return returnValue.dump();
			}

			path = NSSystemUtils::GetSystemDirectory(dirType);
		}

		if (path.empty())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Folder not presented";
			return returnValue.dump();
		}


		returnValue["status"] = "success";

		std::vector<std::wstring> files = NSDirectory::GetFiles(path, true);

		json jFiles = json::array();

		for (std::vector<std::wstring>::iterator i = files.begin(); i != files.end(); i++)
		{
			jFiles.push_back(U_TO_UTF8((*i)));
		}

		returnValue["status"] = "success";
		returnValue["files"] = jFiles;

		return returnValue.dump();
	}
}
