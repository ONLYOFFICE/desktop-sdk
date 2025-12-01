#include "./../internal/base.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

namespace form_field_filler
{
	long long parseDateToUTCMilliseconds(const std::string& date)
	{
		long long default_value = 946684800000;

		std::tm tm = {};
		std::istringstream ss(date);
		ss >> std::get_time(&tm, "%d.%m.%Y");

		if (ss.fail())
			return default_value;

		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;

		time_t timeUtc = timegm(&tm);
		if (timeUtc == -1) {
			return default_value;
		}

		return static_cast<long long>(timeUtc) * 1000;
	}

	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"form_field_filler\",\
  \"description\": \"Use this function if you are asked to fill out a document or form. Provide only those keys that were obtained during the document/form analysis, unless a specific key was explicitly requested.\\n\\nSpecial rules for dates:\\n- If you are asked for a date, then give it in the format dd.mm.yyyy.\\n- Convert any given date range or time period (e.g. \\\"summer 2025\\\", \\\"fall 2024\\\", \\\"January 2023\\\", \\\"second half 2026\\\", \\\"first quarter 2022\\\") to two dates: start and end, represented as timestamp in format dd.mm.yyyy. Use exact calendar dates to define period boundaries (taking into account leap years, number of days in months, etc.)).\\n\\nReturn value:\\nAn object describing the result of the operation.\\n{\\n\\\"role\\\": \\\"tool\\\", // Required constant field\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"document\\\": \\\"string\\\", // Path or link to the newly filled document\\n\\\"error_message\\\": \\\"string | null\\\" // Error details if status = 'error'\\n}\\n\\nExample function call:\\n{\\n\\\"document\\\": \\\"/home/user/document.docx\\\",\\n\\\"fields\\\": {\\n\\\"Full Name\\\": \\\"John Doe\\\",\\n\\\"Date of Birth\\\": \\\"15.03.1990\\\",\\n\\\"startDate\\\": \\\"01.06.2025\\\",\\n\\\"endDate\\\": \\\"31.08.2025\\\"\\n}\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"document\\\": \\\"/home/user/document_filled.docx\\\",\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"document\\\": null,\\n\\\"error_message\\\": \\\"One or more required fields are missing.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"document\": {\
        \"type\": \"string\",\
        \"description\": \"Path to the file or URL of the document to be filled.\"\
      },\
      \"fields\": {\
        \"type\": \"object\",\
        \"description\": \"Key-value pairs of fields to fill in the document. Dates must follow the dd.mm.yyyy format.\"\
      }\
    },\
    \"required\": [\"document\", \"fields\"]\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		json param = json::parse(arg);
		if (!param.contains("document") || !param["document"].is_string())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Required field ('document') not present";
			returnValue["document"] = "";
			return returnValue.dump();
		}

		if (!param.contains("fields") || !param["fields"].is_object())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Required field ('fields') not present";
			returnValue["document"] = "";
			return returnValue.dump();
		}

		std::string pathA = param["document"];
		std::wstring path = UTF8_TO_U(pathA);

		CDocBuilder builder;
		initBuilder(&builder);
		int res = builder.OpenFile(path.c_str(), L"");
		if (0 != res)
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Could not open document";
			returnValue["fields"] = json::array();
			return returnValue.dump();
		}

		json inputFields = param["fields"];

		CContext context = builder.GetContext();
		CValue global = context.GetGlobal();

		CValue api = global["Api"];
		CValue document = api.Call("GetDocument");

		CValue allForms = document.Call("GetAllForms");
		int count = allForms.GetLength();
		for (int i = 0; i < count; i++)
		{
			CValue field = allForms.Get(i);

			std::wstring keyW(field.Call("GetFormKey").ToString().c_str());
			std::string key = U_TO_UTF8(keyW);

			if (!inputFields.contains(key))
				continue;

			std::wstring typeW(field.Call("GetFormType").ToString().c_str());
			std::string type = U_TO_UTF8(typeW);

			if ("textForm" == type && inputFields[key].is_string())
			{
				std::string fieldValue = inputFields[key];
				field.Call("SetText", fieldValue.c_str());
			}
			if ("dateForm" == type && inputFields[key].is_string())
			{
				std::string fieldValue = inputFields[key];
				field.Call("SetTime", (double)parseDateToUTCMilliseconds(fieldValue));
			}
		}

		std::wstring filePath = CAITools::getInstance().getTempFile() + L".pdf";
		builder.SaveFile(AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF, filePath.c_str());

		builder.CloseFile();

		returnValue["status"] = "success";
		returnValue["document"] = U_TO_UTF8(filePath);

		returnValue["toolSystemType"] = "file";
		returnValue["toolSystemResult"] = U_TO_UTF8(filePath);

		return JSON_serialize(returnValue);
	}
}
