#include "./../internal/base.h"

namespace file_content_reader
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"file_content_reader\",\
  \"description\": \"Use this function if you are asked to return the content of a file in a specific format (txt, html). Input: file path and optional desired format (default is 'txt'). Output: an object describing the result of the operation.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"content\\\": \\\"string\\\", // Converted file content\\n\\\"error_message\\\": \\\"string | null\\\" // Error details if status = 'error'\\n}\\n\\nExample function call parameter: { \\\"path\\\": \\\"/home/user/document.docx\\\" }\\nExample function call parameter with format: { \\\"path\\\": \\\"/home/user/document.docx\\\", \\\"format\\\": \\\"html\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"content\\\": \\\"File content in txt format...\\\",\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"content\\\": \\\"\\\",\\n\\\"error_message\\\": \\\"Unsupported format requested.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"path\": {\
        \"type\": \"string\",\
        \"description\": \"Path to the file to be converted.\"\
      },\
      \"format\": {\
        \"type\": \"string\",\
        \"enum\": [\"txt\", \"html\"],\
        \"description\": \"Desired output format. Default is 'txt'.\",\
        \"default\": \"txt\"\
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
		returnValue["status"] = "error";
		returnValue["error_message"] = "TODO: method no realized";
		returnValue["content"] = "";
		return returnValue.dump();
	}
}
