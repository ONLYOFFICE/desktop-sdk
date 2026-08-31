/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "./tools.h"
#include "../../../../core/DesktopEditor/doctrenderer/docbuilder.h"
#include "../../../../core/DesktopEditor/common/File.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "./functions/internal/funcs.h"

#ifdef GetTempPath
#undef GetTempPath
#endif

namespace
{
	// A tool description may carry {{PLATFORM}}, {{SHELL}} and {{MATH_HINT}};
	// all three are filled in here, on the way out to the model.
	//
	// This is the only spot where that can happen. The answer is known when
	// this file is compiled, not when info.json is written, and description()
	// inside each tool's main.cpp is regenerated from info.json by
	// functions/internal/funcs.py - anything hand-written there is overwritten
	// on the next run. tools.cpp the generator never touches.
	//
	// It matters because nothing else tells the model which machine it is on:
	// the agent's system prompt does not mention the platform, so without this
	// a script would be written for the wrong shell and fail on the first try.
	void SubstitutePlatform(std::string& sDescription)
	{
#if defined(_WIN32)
		NSStringUtils::string_replaceA(sDescription, "{{PLATFORM}}", "Windows");
		NSStringUtils::string_replaceA(sDescription, "{{SHELL}}", "PowerShell");
		NSStringUtils::string_replaceA(sDescription, "{{MATH_HINT}}",
			"PowerShell has the whole .NET library for arithmetic and dates: "
			"[math]::Round($x, 2), [datetime]::ParseExact(...), (Get-Date).AddDays(14)");
		NSStringUtils::string_replaceA(sDescription, "{{READ_HINT}}",
			"read it with [System.IO.File]::ReadAllText($path), which follows a byte "
			"order mark when the file has one and assumes UTF-8 when it does not. "
			"Get-Content without -Encoding reads ANSI and mangles UTF-8, and do not "
			"try to guess a code page");
#elif defined(_MAC)
		// _MAC comes first: macOS defines _LINUX as well.
		NSStringUtils::string_replaceA(sDescription, "{{PLATFORM}}", "macOS");
		NSStringUtils::string_replaceA(sDescription, "{{SHELL}}", "zsh");
		NSStringUtils::string_replaceA(sDescription, "{{MATH_HINT}}",
			"the shell itself does integer arithmetic only, so use awk for anything "
			"fractional: awk 'BEGIN{printf \"%.2f\", 250000/29.3*14}'");
		NSStringUtils::string_replaceA(sDescription, "{{READ_HINT}}",
			"cat passes the bytes through unchanged, so a UTF-8 file arrives intact; "
			"a UTF-16 one needs iconv -f UTF-16 -t UTF-8");
#else
		NSStringUtils::string_replaceA(sDescription, "{{PLATFORM}}", "Linux");
		NSStringUtils::string_replaceA(sDescription, "{{SHELL}}", "bash");
		NSStringUtils::string_replaceA(sDescription, "{{MATH_HINT}}",
			"the shell itself does integer arithmetic only, so use awk for anything "
			"fractional: awk 'BEGIN{printf \"%.2f\", 250000/29.3*14}'");
		NSStringUtils::string_replaceA(sDescription, "{{READ_HINT}}",
			"cat passes the bytes through unchanged, so a UTF-8 file arrives intact; "
			"a UTF-16 one needs iconv -f UTF-16 -t UTF-8");
#endif
	}
}

CAITools::CAITools()
{
	m_funcs = new CFunctions();
}

CAITools::~CAITools()
{
	delete m_funcs;
}

CAITools& CAITools::getInstance()
{
	static CAITools tools;
	return tools;
}

void CAITools::setWorkDirectory(const std::wstring& dir)
{
	m_workDirectory = dir;
	NSDoctRenderer::CDocBuilder::Initialize(m_workDirectory.c_str());
}
std::wstring CAITools::getWorkDirectory()
{
	return m_workDirectory;
}
void CAITools::setFontsDirectory(const std::wstring& dir)
{
	m_fontsDirectory = dir;
}
std::wstring CAITools::getFontsDirectory()
{
	return m_fontsDirectory;
}

std::wstring CAITools::getTempFile()
{
	std::wstring sDirTmp = NSFile::CFileBinary::GetTempPath();

	std::wstring sFileTmp = NSFile::CFileBinary::CreateTempFileWithUniqueName(sDirTmp, L"IMG");
	if (NSFile::CFileBinary::Exists(sFileTmp))
		NSFile::CFileBinary::Remove(sFileTmp);

	return sFileTmp;
}

std::string CAITools::getFunctions()
{
	if (m_funcs->m_funcs.empty())
		return "[]";

	std::string sResult = "[";

	for (std::map<std::string, TFuncInstance>::iterator iter = m_funcs->m_funcs.begin(); iter != m_funcs->m_funcs.end(); iter++)
	{
		std::string sDescription = iter->second.name;
		SubstitutePlatform(sDescription);
		sResult += sDescription;
		sResult += ",";
	}

	sResult[sResult.length() - 1] = ']';
	return sResult;
}

std::string CAITools::callFunc(const std::string& name, const std::string& arg, CAIToolsHelper* helper)
{
	auto find = m_funcs->m_funcs.find(name);
	if (find !=m_funcs->m_funcs.end())
		return find->second.func(arg, helper);
	return "";
}
