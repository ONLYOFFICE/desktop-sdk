#include "./../internal/base.h"

namespace generate_pptx
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"generate_pptx\",\
  \"description\": \"Use this function if you are asked to generate a complete presentation with a custom theme, fonts, and streaming content. Input: a detailed description of the presentation to create, including topic, number of slides, and style.\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"topic\": {\
        \"type\": \"string\",\
        \"description\": \"Presentation topic.\"\
      },\
      \"slideCount\": {\
        \"type\": \"string\",\
        \"description\": \"Number of slides to generate.\"\
      },\
      \"style\": {\
        \"type\": \"string\",\
        \"description\": \"Visual style of the presentation: modern, classic, minimal, or corporate.\"\
      }\
    },\
    \"required\": []\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();

		json param = json::parse(arg);
		if (!param.is_object())
		{
			returnValue["status"] = "error";
		}
		else
		{
			std::string value = json(param).dump();
			std::string code = "window.AscDesktopEditor.generateNew(\"pptx\", \"ai-gen-pptx\", " + json(value).dump() + ");";
			helper->ExecuteJS(code);

			returnValue["status"] = "success";
		}

		return returnValue.dump();
	}
}
