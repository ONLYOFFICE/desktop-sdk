#include "./../internal/base.h"
#include "../../../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../../../core/DesktopEditor/common/Directory.h"

#if defined(_WIN32) && !defined(WIN_XP_OR_VISTA)
#include <windows.h>
#include <shlobj.h>
#include <comdef.h>
#endif

#ifdef _LINUX
#ifdef _MAC
#include <pwd.h>
#include <unistd.h>
#include <CoreServices/CoreServices.h>
#include <Foundation/Foundation.h>
#else
#include <pwd.h>
#include <unistd.h>
#include <cstdlib>
#endif
#endif

namespace folder_content_reader
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"folder_content_reader\",\
  \"description\": \"Use this function to list the contents of a folder. Input: folder name or path. Standard folder names include: 'Documents', 'Downloads', 'Desktop'. Output: an object describing the result of the operation.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"files\\\": [ \\\"array of file names\\\" ],\\n\\\"error_message\\\": \\\"string | null\\\"\\n}\\n\\nExample function call parameter: { \\\"folder\\\": \\\"Documents\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"files\\\": [ \\\"file1.txt\\\", \\\"file2.docx\\\" ],\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"files\\\": [],\\n\\\"error_message\\\": \\\"Folder not found.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"folder\": {\
        \"type\": \"string\",\
        \"description\": \"Name or path of the folder to be listed. Standard names: 'Documents', 'Downloads', 'Desktop'.\"\
      }\
    },\
    \"required\": [\"folder\"]\
  }\
}";
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		json returnValue = json::object();
		returnValue["role"] = "tool";

		json param = json::parse(arg);
		if (!param.contains("folder") || !param["folder"].is_string())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Folder not presented";
			return returnValue.dump();
		}

		std::string pathA = param["folder"];
		std::wstring path = UTF8_TO_U(pathA);

		NSStringUtils::string_replace(path, L"\\", L"/");

		if (std::wstring::npos == path.find(L"/"))
		{
			int nDirType = -1;
			if ("Documents" == pathA)
				nDirType = 0;
			else if ("Downloads" == pathA)
				nDirType = 1;
			else if ("Desktop" == pathA)
				nDirType = 2;

			if (nDirType == -1)
			{
				returnValue["status"] = "error";
				returnValue["error_message"] = "Folder not found";
				return returnValue.dump();
			}

#if defined(_WIN32) && !defined(WIN_XP_OR_VISTA)
			KNOWNFOLDERID folderId = FOLDERID_Documents;
			if (nDirType == 1)
				folderId = FOLDERID_Downloads;
			else if (nDirType == 2)
				folderId = FOLDERID_Desktop;

			PWSTR dirPath = NULL;
			HRESULT hr = SHGetKnownFolderPath(folderId, 0, NULL, &dirPath);

			if (SUCCEEDED(hr))
				path = std::wstring(dirPath);
			else
				path = L"";

			if (dirPath)
				CoTaskMemFree(dirPath);
#endif

#ifdef _LINUX
#ifdef _MAC
			NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

			NSSearchPathDirectory searchPath = NSDocumentDirectory;

			switch (nDirType) {
			case 1:
				searchPath = NSDownloadsDirectory;
				break;
			case 2:
				searchPath = NSDesktopDirectory;
				break;
			default:
				break;
			}

			NSArray* paths = NSSearchPathForDirectoriesInDomains(searchPath, NSUserDomainMask, YES);
			std::string result = "";

			if ([paths count] > 0) {
				NSString* path = [paths objectAtIndex:0];
				result = std::string([path UTF8String]);
			}

			[pool release];

			path = UTF8_TO_U(result);
#else
			std::wstring home_dir = NSSystemUtils::GetEnvVariable(L"HOME");
			path = home_dir + L"/Documents";

			switch (nDirType) {
			case 1:
				path = home_dir + L"/Downloads";
				break;
			case 2:
				path = home_dir + L"/Desktop";
				break;
			default:
				break;
			}
#endif
#endif
		}

		if (path.empty())
		{
			returnValue["status"] = "error";
			returnValue["error_message"] = "Folder not presented";
			return returnValue.dump();
		}


		returnValue["status"] = "success";

		std::vector<std::wstring> files = NSDirectory::GetFiles(path);

		json jFiles = json::array();

		for (std::vector<std::wstring>::iterator i = files.begin(); i != files.end(); i++)
		{
			jFiles.push_back(U_TO_UTF8((*i)));
		}

		returnValue["status"] = "success";
		returnValue["files"] = jFiles;

		return returnValue.dump();
	}
}
