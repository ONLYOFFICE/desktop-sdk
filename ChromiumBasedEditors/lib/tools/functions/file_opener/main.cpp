#include "./../internal/base.h"

namespace file_opener
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"file_opener\",\
  \"description\": \"Use this function if you are asked to open a file. Input: a file path. Output: an object describing the result of the operation.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\", // Required constant field\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"error_message\\\": \\\"string | null\\\" // Error details if status = 'error'\\n}\\n\\nExample function call parameter: { \\\"path\\\": \\\"/home/user/document.txt\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"error_message\\\": \\\"File not found.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"path\": {\
        \"type\": \"string\",\
        \"description\": \"Path to the file to be opened.\"\
      }\
    },\
    \"required\": [\"path\"]\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		json param = json::parse(arg);
		if (!param.contains("path") || !param["path"].is_string())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "File not opened";
			return returnValue.dump();
		}

		std::string pathA = param["path"];
		std::wstring path = UTF8_TO_U(pathA);

		returnValue["status"] = "success";

		helper->OpenFile(path);

		return returnValue.dump();
	}
}
