#include "./../internal/base.h"

namespace generate_docx
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"generate_docx\",\
  \"description\": \"Use this function if you are asked to generate a textual document (report, article, letter, etc.) based on a description. Input: a detailed description of what needs to be generated.\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"description\": {\
        \"type\": \"string\",\
        \"description\": \"Detailed description of the document to generate, including its topic, structure, tone, and format.\"\
      }\
    },\
    \"required\": [\"description\"]\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();

		json param = json::parse(arg);
		if (!param.contains("description") || !param["description"].is_string())
		{
			returnValue["status"] = "error";
		}
		else
		{
			std::string description = param["description"];
			std::string code = "window.AscDesktopEditor.generateNew(\"docx\", \"ai-gen-docx\", " + json(description).dump() + ");";
			helper->ExecuteJS(code);

			returnValue["status"] = "success";
		}

		return returnValue.dump();
	}
}
