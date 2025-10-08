#pragma once
#include "./base.h"
#include "../file_content_reader/main.cpp"
#include "../file_opener/main.cpp"
#include "../folder_content_reader/main.cpp"
#include "../form_field_analyser/main.cpp"
#include "../form_field_filler/main.cpp"
#include "../recent_files_reader/main.cpp"

struct TFuncInstance
{
	std::string name;
	std::function<std::string(std::string, CAIToolsHelper*)> func;
	TFuncInstance(const std::string& n, std::function<std::string(const std::string&, CAIToolsHelper*)> f)
		: name(n), func(f) {}
};

class CFunctions
{
public:
	std::map<std::string, TFuncInstance> m_funcs;
	CFunctions()
	{
		m_funcs.insert(std::make_pair("file_content_reader", TFuncInstance(file_content_reader::description(), file_content_reader::main)));
		m_funcs.insert(std::make_pair("file_opener", TFuncInstance(file_opener::description(), file_opener::main)));
		m_funcs.insert(std::make_pair("folder_content_reader", TFuncInstance(folder_content_reader::description(), folder_content_reader::main)));
		m_funcs.insert(std::make_pair("form_field_analyser", TFuncInstance(form_field_analyser::description(), form_field_analyser::main)));
		m_funcs.insert(std::make_pair("form_field_filler", TFuncInstance(form_field_filler::description(), form_field_filler::main)));
		m_funcs.insert(std::make_pair("recent_files_reader", TFuncInstance(recent_files_reader::description(), recent_files_reader::main)));
	}
};
