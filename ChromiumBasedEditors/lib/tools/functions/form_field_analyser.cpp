#include "./internal/base.h"

namespace form_field_analyser
{
	std::string description()
	{
		return "\n\
- form_field_analyser\n\
Description: \n\
Use this function if you are asked to analyze document or forms.\n\
Parameters:\n\
  - document (string): path or link value to the document you were asked to analyze\n\
Examples:\n\
If you need to analyze document /home/user/document.docx, respond with: \
[functionCalling (form_field_analyser)]: { \"document\" : \"/home/user/document.docx\" }\n";
	}

	std::string main(const std::string& arg)
	{
		json param = json::parse(arg);
		if (!param.contains("document") || !param["document"].is_string())
			return "";

		std::string pathA = param["document"];
		std::wstring path = UTF8_TO_U(pathA);

		CDocBuilder builder;
		initBuilder(&builder);
		int res = builder.OpenFile(path.c_str(), L"");
		if (0 != res)
			return "";

		CContext context = builder.GetContext();
		CValue global = context.GetGlobal();

		CValue api = global["Api"];
		CValue document = api.Call("GetDocument");

		json keys = json::array();

		CValue allForms = document.Call("GetAllForms");
		int count = allForms.GetLength();
		for (int i = 0; i < count; i++)
		{
			CValue field = allForms.Get(i);

			std::wstring keyW(field.Call("GetFormKey").ToString().c_str());
			std::string key = U_TO_UTF8(keyW);

			keys.push_back(key);
		}

		std::string result = keys.dump();
		builder.CloseFile();

		std::string prompt = "[prompt]Match the document fields by meaning with the values in the array in javascript format. \
And when you are asked to fill out a form, send not invented field values to the function, but only those that are in the passed list: " + result;

		return prompt;
	}
}
