#include "./../internal/base.h"

namespace generate_form
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"generate_form\",\
  \"description\": \"Use this function if you are asked to generate a form or document template (contract, any document for filling) based on a description. Input: a detailed description of the desired form or template.\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"description\": {\
        \"type\": \"string\",\
        \"description\": \"Detailed description of the form or template to generate, including purpose, structure.\"\
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
			std::string code = "window.AscDesktopEditor.generateNew(\"pdf\", \"ai-gen-form\", " + json(description).dump() + ");";
			helper->ExecuteJS(code);

			returnValue["status"] = "success";
		}

		return returnValue.dump();
	}
}
