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

#ifndef ASC_TEMPLATES_MANAGER_H
#define ASC_TEMPLATES_MANAGER_H

#include "./../include/applicationmanager.h"
#include "./../../../../core/DesktopEditor/graphics/Timer.h"
#include "./../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "./../../../../core/DesktopEditor/common/Directory.h"
#include "./../../../../core/Common/OfficeFileFormatChecker.h"
#include "./../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "./x2t.h"

class CTemplateRec
{
public:
	int          Id;
	std::wstring Name;
	std::wstring Path;
	std::wstring Icon;

	bool         IsPin;
	int          Format;

public:
	CTemplateRec()
	{
		Id = 0;
		IsPin = false;
		Format = 0;
	}
};

#define ONLYOFFICE_TEMPLATES_VERSION 1
class CTemplatesCache : public NSThreads::CBaseThread
{
private:
	CAscApplicationManager* m_pManager;
	NSCriticalSection::CRITICAL_SECTION m_oCS;

	std::wstring m_sLang;
	int m_nScale;

	std::vector<CTemplateRec> m_arTemplates;

	int m_nStartCounter;

public:
	CTemplatesCache()
	{
		m_oCS.InitializeCriticalSection();

		m_sLang = L"en";
		m_nScale = 100;
		m_nStartCounter = 0;
	}
	~CTemplatesCache()
	{
		Stop();
		m_oCS.DeleteCriticalSection();
	}

	void IncrementStartPointer()
	{
		++m_nStartCounter;
		if (2 == m_nStartCounter)
			Start(0);
	}

	void SetManager(CAscApplicationManager* pManager)
	{
		m_pManager = pManager;
	}

	void SetLang(const std::wstring& lang)
	{
		CTemporaryCS oCS(&m_oCS);
		if (m_sLang == lang)
			return;

		m_sLang = lang;

		if (IsRunned())
			Stop();
		Start(0);
	}
	void SetScale(const int& scale)
	{
		CTemporaryCS oCS(&m_oCS);
		if (m_nScale == scale)
			return;

		if (IsRunned())
			Stop();
		Start(0);
	}

	void SetPin(const int& nId, const bool& isPin)
	{
		CTemporaryCS oCS(&m_oCS);

		if (nId < 0 || nId >= (int)m_arTemplates.size())
			return;

		m_arTemplates[nId].IsPin = isPin;
		SaveTemplates();
	}

	void SetIcon(const int& nId, const std::wstring& sIcon)
	{
		CTemporaryCS oCS(&m_oCS);

		if (nId < 0 || nId >= (int)m_arTemplates.size())
			return;

		m_arTemplates[nId].Icon = sIcon;
	}

protected:

	void LoadTemplates()
	{
		CTemporaryCS oCS(&m_oCS);

		m_arTemplates.clear();

		XmlUtils::CXmlNode oNode;
		if (oNode.FromXmlFile(m_pManager->m_oSettings.recover_path + L"/../templates.xml"))
		{
			std::vector<XmlUtils::CXmlNode> nodes = oNode.GetNodes(L"template");

			for (std::vector<XmlUtils::CXmlNode>::iterator i = nodes.begin(); i != nodes.end(); i++)
			{
				CTemplateRec rec;
				rec.Id = i->GetAttributeInt("id");
				rec.Path = i->GetAttribute("path");
				rec.Name = i->GetAttribute("name");
				rec.IsPin = (i->GetAttributeInt("pin") == 1) ? true : false;
				rec.Format = i->GetAttributeInt("format");

				m_arTemplates.push_back(rec);
			}
		}
	}

	void SaveTemplates()
	{
		NSStringUtils::CStringBuilder oXmlWriter;
		oXmlWriter.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><templates>");

		for (std::vector<CTemplateRec>::iterator i = m_arTemplates.begin(); i != m_arTemplates.end(); i++)
		{
			oXmlWriter.WriteString(L"<template id=\"");
			oXmlWriter.AddInt(i->Id);
			oXmlWriter.WriteString(L"\" name=\"");
			oXmlWriter.WriteEncodeXmlString(i->Name);
			oXmlWriter.WriteString(L"\" path=\"");
			oXmlWriter.WriteEncodeXmlString(i->Path);
			oXmlWriter.WriteString(L"\" pin=\"");
			oXmlWriter.AddInt(i->IsPin ? 1 : 0);
			oXmlWriter.WriteString(L"\" format=\"");
			oXmlWriter.AddInt(i->Format);
			oXmlWriter.WriteString(L"\" />");
		}

		oXmlWriter.WriteString(L"</templates>");
		NSFile::CFileBinary::SaveToFile(m_pManager->m_oSettings.recover_path + L"/../templates.xml", oXmlWriter.GetData());
	}

	void DumpTemplates(std::vector<std::wstring>& templatePaths)
	{
		CTemporaryCS oCS(&m_oCS);

		m_arTemplates.clear();
		int nId = 0;
		for (std::vector<std::wstring>::iterator i = templatePaths.begin(); i != templatePaths.end(); i++)
		{
			CTemplateRec rec;
			rec.Id = nId++;
			rec.Path = *i;
			rec.Name = NSFile::GetFileName(rec.Path);
			rec.IsPin = false;
			rec.Format = 0;

			COfficeFileFormatChecker oChecker;
			if (oChecker.isOfficeFile(rec.Path))
				rec.Format = oChecker.nFileType;

			m_arTemplates.push_back(rec);
		}

		SaveTemplates();
	}

	void SendToInterface()
	{
		NSStringUtils::CStringBuilder oBuilder;
		oBuilder.WriteString(L"[");
		for (std::vector<CTemplateRec>::iterator i = m_arTemplates.begin(); i != m_arTemplates.end(); i++)
		{
			if (1 < oBuilder.GetCurSize())
				oBuilder.WriteString(L",");

			oBuilder.WriteString(L"{id:");
			oBuilder.AddInt(i->Id);
			oBuilder.WriteString(L",type:");
			oBuilder.AddInt(i->Format);
			oBuilder.WriteString(L",name:\"");
			oBuilder.WriteEncodeXmlString(i->Name);
			oBuilder.WriteString(L"\",path:\"");
			oBuilder.WriteEncodeXmlString(i->Path);
			oBuilder.WriteString(L"\",pin:\"");
			oBuilder.AddInt(i->IsPin ? 1 : 0);
			oBuilder.WriteString(L"\",icon:\"");
			oBuilder.WriteEncodeXmlString(i->Icon);
			oBuilder.WriteString(L"\"}");
		}
		oBuilder.WriteString(L"]");

		NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_TEMPLATES);
		NSEditorApi::CAscLocalRecentsAll* pData = new NSEditorApi::CAscLocalRecentsAll();
		pData->put_JSON(oBuilder.GetData());
		pEvent->m_pData = pData;

		m_pManager->SetEventToAllMainWindows(pEvent);
	}

public:
	virtual DWORD ThreadProc()
	{
		std::wstring sTemplateLog = m_pManager->m_oSettings.templates_cache_info_path + L"/templates.log";
		std::vector<std::string> arTemplates;

		// читаем "старый" набор шаблонов
		if (true)
		{
			NSFile::CFileBinary oFile;
			if (oFile.OpenFile(sTemplateLog))
			{
				int nSize = oFile.GetFileSize();
				char* pBuffer = new char[nSize];
				DWORD dwReaden = 0;
				oFile.ReadFile((BYTE*)pBuffer, nSize, dwReaden);
				oFile.CloseFile();

				int nStart = 0;
				int nCur = nStart;
				for (; nCur < nSize; ++nCur)
				{
					if (pBuffer[nCur] == '\n')
					{
						int nEnd = nCur - 1;
						if (nEnd > nStart)
						{
							std::string s(pBuffer + nStart, nEnd - nStart + 1);
							arTemplates.push_back(s);
						}
						nStart = nCur + 1;
					}
				}

				delete[] pBuffer;
			}

#ifdef ONLYOFFICE_TEMPLATES_VERSION
			if (0 != arTemplates.size())
			{
				// check version!!!
				std::string sOO_Version = arTemplates[0];
				if (0 != sOO_Version.find("ONLYOFFICE_TEMPLATES_VERSION_"))
				{
					arTemplates.clear();
				}
				else
				{
					std::string sVersion = sOO_Version.substr(29);
					int nVersion = std::stoi(sVersion);
					if (nVersion != ONLYOFFICE_TEMPLATES_VERSION)
						arTemplates.clear();
					else
						arTemplates.erase(arTemplates.begin());
				}
			}
#endif
		}

		// читаем "новый" набор шаблонов
		std::wstring sSystemTemplatesPath = m_pManager->m_oSettings.system_templates_path;
		if (NSDirectory::Exists(sSystemTemplatesPath + L"/" + m_sLang))
			sSystemTemplatesPath = sSystemTemplatesPath + L"/" + m_sLang;

		std::vector<std::wstring> arTemplatesPathsTmp = NSDirectory::GetFiles(sSystemTemplatesPath, true);
		if (!m_pManager->m_oSettings.user_templates_path.empty())
			NSDirectory::GetFiles2(m_pManager->m_oSettings.user_templates_path, arTemplatesPathsTmp, true);

		// удаляем папки, которые не нужно парсить
		std::vector<std::wstring> arTemplatesPaths;
		arTemplatesPaths.reserve(arTemplatesPathsTmp.size());
		for (std::vector<std::wstring>::iterator i = arTemplatesPathsTmp.begin(); i != arTemplatesPathsTmp.end(); i++)
		{
#ifdef _WIN32
			if (i->find(L"\\.git\\") == std::wstring::npos)
				arTemplatesPaths.push_back(*i);
#else
			if (i->find(L"/.git/") == std::wstring::npos)
				arTemplatesPaths.push_back(*i);
#endif
		}

		// сортируем (нужно для сравнения для старого набора)
		std::sort(arTemplatesPaths.begin(), arTemplatesPaths.end());

		bool bIsEqual = true;

		// если количество шрифтов в наборах не совпадают - то нужно перегенерировать
		if (arTemplates.size() != arTemplatesPaths.size())
			bIsEqual = false;

		if (bIsEqual)
		{
			// если наборы не совпадают - то нужно перегенерировать
			int nCount = (int)arTemplates.size();
			for (int i = 0; i < nCount; ++i)
			{
				if (arTemplates[i] != NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(arTemplatesPaths[i].c_str(), (LONG)arTemplatesPaths[i].length()))
				{
					bIsEqual = false;
					break;
				}
			}
		}

		std::wstring sOutputDir = m_pManager->m_oSettings.templates_cache_info_path;

		if (!bIsEqual)
		{
			if (NSDirectory::Exists(sOutputDir))
				NSDirectory::DeleteDirectory(sOutputDir);
			NSDirectory::CreateDirectory(sOutputDir);

			// формируем новый набор шаблонов
			NSStringUtils::CStringBuilder oTemplatesLog;
#ifdef ONLYOFFICE_TEMPLATES_VERSION
			oTemplatesLog.WriteString(L"ONLYOFFICE_TEMPLATES_VERSION_");
			oTemplatesLog.WriteString(std::to_wstring(ONLYOFFICE_TEMPLATES_VERSION));
			oTemplatesLog.WriteString(L"\n");
#endif
			int nCount = (int)arTemplatesPaths.size();
			for (int i = 0; i < nCount; ++i)
			{
				oTemplatesLog.WriteString(arTemplatesPaths[i]);
				oTemplatesLog.WriteString(L"\n");
			}

			NSFile::CFileBinary::SaveToFile(sTemplateLog, oTemplatesLog.GetData());

			DumpTemplates(arTemplatesPaths);
		}
		else
		{
			LoadTemplates();
		}

		sOutputDir = sOutputDir + L"/" + std::to_wstring(m_nScale);
		if (!NSDirectory::Exists(sOutputDir))
			NSDirectory::CreateDirectory(sOutputDir);

		DWORD dwTime = NSTimers::GetTickCount();
		DWORD dwTime2 = dwTime;
		DWORD dwTimeEps = 2000;

		bool bIsNeedConvert = false;
		for (std::vector<std::wstring>::iterator i = arTemplatesPaths.begin(); i != arTemplatesPaths.end(); i++)
		{
			std::wstring sOutputFile = sOutputDir + L"/" + NSFile::GetFileName(*i) + L".jpg";
			if (NSFile::CFileBinary::Exists(sOutputFile))
				continue;

			bIsNeedConvert = true;
		}

		if (bIsNeedConvert)
			SendToInterface();

		if (!m_bRunThread)
			return 0;

		int nCurrentId = 0;
		for (std::vector<std::wstring>::iterator i = arTemplatesPaths.begin(); i != arTemplatesPaths.end(); i++, ++nCurrentId)
		{
			if (!m_bRunThread)
				break;

			std::wstring sOutputFile = sOutputDir + L"/" + NSFile::GetFileName(*i) + L".jpg";
			if (NSFile::CFileBinary::Exists(sOutputFile))
			{
				SetIcon(nCurrentId, sOutputFile);
				continue;
			}

			std::wstring sTempDir = NSDirectory::GetTempPath();

			sTempDir = NSFile::CFileBinary::CreateTempFileWithUniqueName(sTempDir, L"T_");
			if (NSFile::CFileBinary::Exists(sTempDir))
				NSFile::CFileBinary::Remove(sTempDir);
			NSDirectory::CreateDirectory(sTempDir);

			int nW = (int)(m_nScale * m_pManager->m_oSettings.templates_image_width / 100);
			int nH = (int)(m_nScale * m_pManager->m_oSettings.templates_image_height / 100);

			NSStringUtils::CStringBuilder oBuilder;
			oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
			oBuilder.WriteEncodeXmlString(*i);
			oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
			oBuilder.WriteEncodeXmlString(sOutputFile);
			oBuilder.WriteString(L"</m_sFileTo><m_nFormatTo>1025</m_nFormatTo><m_sFontDir>");
			oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
			oBuilder.WriteString(L"</m_sFontDir><m_bIsNoBase64>false</m_bIsNoBase64><m_sAllFontsPath>");
			oBuilder.WriteEncodeXmlString(m_pManager->m_oSettings.fonts_cache_info_path);
			oBuilder.WriteString(L"/AllFonts.js</m_sAllFontsPath>");

			oBuilder.WriteString(L"<m_sTempDir>");
			oBuilder.WriteEncodeXmlString(sTempDir);
			oBuilder.WriteString(L"</m_sTempDir>");

			oBuilder.WriteString(L"<m_bDontSaveAdditional>true</m_bDontSaveAdditional>");
			oBuilder.WriteString(L"<m_bFromChanges>false</m_bFromChanges>");

			std::wstring json_params = L"{\
\"spreadsheetLayout\":{\
\"fitToWidth\":0,\"fitToHeight\":0,\"orientation\":\"landscape\",\
\"sheetsProps\":{\
\"0\":{\
\"headings\":false,\"printTitlesWidth\":null,\"printTitlesHeight\":null,\
\"pageMargins\":{\"bottom\":10,\"footer\":5,\"header\":5,\"left\":5,\"right\":5,\"top\":10},\
\"pageSetup\":{\"orientation\":1,\"width\":210,\"height\":297,\"paperUnits\":0,\"scale\":100,\"printArea\":false,\"horizontalDpi\":600,\"verticalDpi\":600,\"usePrinterDefaults\":true,\"fitToHeight\":0,\"fitToWidth\":0}\
}\
}\
},\
\"documentLayout\":{\"drawPlaceHolders\":true,\"drawFormHighlight\":true,\"isPrint\":true}\
}";

			oBuilder.WriteString(L"<m_sJsonParams>");
			oBuilder.WriteEncodeXmlString(json_params);
			oBuilder.WriteString(L"</m_sJsonParams>");

			// disable cache
			oBuilder.WriteString(L"<m_nDoctParams>1</m_nDoctParams>");

			oBuilder.WriteString(L"<m_oThumbnail><first>true</first><aspect>16</aspect><width>");
			oBuilder.WriteString(std::to_wstring(nW));
			oBuilder.WriteString(L"</width><height>");
			oBuilder.WriteString(std::to_wstring(nH));
			oBuilder.WriteString(L"</height></m_oThumbnail>");

			oBuilder.WriteString(L"</TaskQueueDataConvert>");

			std::wstring sTempFileForParams = sTempDir + L"/params.xml";
			NSFile::CFileBinary::SaveToFile(sTempFileForParams, oBuilder.GetData(), true);

			int nReturnCode = NSX2T::Convert(m_pManager->m_oSettings.file_converter_path + L"/x2t", sTempFileForParams, m_pManager);
			SetIcon(nCurrentId, sOutputFile);

			NSDirectory::DeleteDirectory(sTempDir);

			dwTime2 = NSTimers::GetTickCount();
			if (dwTime2 > (dwTime + dwTimeEps))
			{
				if (m_bRunThread)
					SendToInterface();
				dwTime = dwTime2;
			}
		}

		if (m_bRunThread)
		{
			SendToInterface();
		}

		m_bRunThread = FALSE;
		return 0;
	}
};

#endif // ASC_TEMPLATES_MANAGER_H
