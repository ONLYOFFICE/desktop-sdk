#include "./internal/base.h"

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
		return "\n\
 - form_field_filler\n\
Description: \n\
Use this function if you are asked to fill out a document or form.\n\
Use only those keys that were obtained during the analysis of the document or form, unless a specific key was specified in the request.\n\
Parameters:\n\
  - document (string): path or link value to the document you were asked to fill\n\
  - fields (object): path or link value to the document you were asked to fill (\
If you are asked for a date, then give it in the format dd.mm.yyyy. \
Convert any given date range or time period (e.g. \"summer 2025\", \"fall 2024\", \"January 2023\", \"second half 2026\", \"first quarter 2022\") to two dates: \
start and end, represented as timestamp in format dd.mm.yyyy. Use exact calendar dates to define period boundaries (taking into account leap years, number of days in months, etc.)).\n\
Examples:\n\
[functionCalling (form_field_filler)] { \"document\" : \"/home/user/document.docx\", \"fields\" : { \"key1\" : \"value1\", \"key2\" : \"value2\" } }\n";
	}

	std::string main(const std::string& arg)
	{
		json param = json::parse(arg);
		if (!param.contains("document") || !param["document"].is_string())
			return "";

		if (!param.contains("fields") || !param["fields"].is_object())
			return "";

		std::string pathA = param["document"];
		std::wstring path = UTF8_TO_U(pathA);

		CDocBuilder builder;
		initBuilder(&builder);
		int res = builder.OpenFile(path.c_str(), L"");
		if (0 != res)
			return "";

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

		return "[file]" + U_TO_UTF8(filePath);
	}
}
