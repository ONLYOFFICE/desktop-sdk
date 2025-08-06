/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "./tools.h"
#include "../../../../core/DesktopEditor/doctrenderer/docbuilder.h"
#include "../../../../core/DesktopEditor/common/File.h"
#include "./functions/internal/funcs.h"

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
	std::string sResult = "";

	for (std::map<std::string, TFuncInstance>::iterator iter = m_funcs->m_funcs.begin(); iter != m_funcs->m_funcs.end(); iter++)
	{
		sResult += iter->second.name;
		sResult += "\n";
	}

	return sResult;
}

std::string CAITools::callFunc(const std::string& name, const std::string& arg)
{
	auto find = m_funcs->m_funcs.find(name);
	if (find !=m_funcs->m_funcs.end())
		return find->second.func(arg);
	return "";
}
