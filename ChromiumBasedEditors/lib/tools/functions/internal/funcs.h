#pragma once
#include "./base.h"
#include "../form_field_analyser/main.cpp"
#include "../form_field_filler/main.cpp"

struct TFuncInstance
{
	std::string name;
	std::function<std::string(std::string)> func;
	TFuncInstance(const std::string& n, std::function<std::string(const std::string&)> f)
		: name(n), func(f) {}
};

class CFunctions
{
public:
	std::map<std::string, TFuncInstance> m_funcs;
	CFunctions()
	{
		m_funcs.insert(std::make_pair("form_field_analyser", TFuncInstance(form_field_analyser::description(), form_field_analyser::main)));
		m_funcs.insert(std::make_pair("form_field_filler", TFuncInstance(form_field_filler::description(), form_field_filler::main)));
	}
};
