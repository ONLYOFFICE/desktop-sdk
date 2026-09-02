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

// The process runner comes first on purpose: on Windows it pulls in
// <windows.h>, and File.h (which undefines the macros windows.h leaves behind)
// only does that the first time it is included. Letting the runner include it
// first means those undefs actually run for this translation unit.
#ifdef MAX_PATH
// File.h got here first and defined it; <windows.h> below defines it again to
// the same 260. Drop ours so the build stays quiet.
#undef MAX_PATH
#endif
#include "../../../src/cefwrapper/external_process_with_childs.h"

#include "./../internal/base.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

#ifdef _WIN32
// Belt and braces: if anything above re-defined them, put the real function
// names back before we call NSFile.
#undef GetTempPath
#undef CreateFile
#undef DeleteFile
#endif

namespace run_script
{
	std::string description()
	{
		return "\
{\
  \"type\": \"function\",\
  \"name\": \"run_script\",\
  \"description\": \"Run a shell script on the user's machine and return what it printed. Use this for anything about the file system - reading a text file (.md, .txt, .csv, .json, .log), listing a folder, finding files by name or date, checking sizes, copying, moving or renaming - by writing a small script instead of looking for a dedicated function. Reading a text file is what this tool is for: print it and it comes back to you in stdout.\\n\\nThis machine runs {{PLATFORM}}, so write the script for {{SHELL}} and use {{PLATFORM}} path syntax, unless you set `interpreter` to something else yourself. The body is written to a temporary file and handed to the interpreter, so several lines, quoting, pipes and redirection all behave normally. Whatever you need to read back, print on stdout; the output comes back as UTF-8, so accented and non-Latin text survives. To read a text file, {{READ_HINT}}. For calculations, {{MATH_HINT}}. There is no exit code in the answer, so report failures yourself by printing them.\\n\\nReturn value:\\n{\\n\\\"role\\\": \\\"tool\\\", // Required constant field\\n\\\"status\\\": \\\"success\\\" | \\\"error\\\",\\n\\\"stdout\\\": \\\"string\\\",\\n\\\"stderr\\\": \\\"string\\\",\\n\\\"timed_out\\\": false, // true when the script was still running at timeout_ms and was killed\\n\\\"truncated\\\": false, // true when the output was too long and was cut\\n\\\"error_message\\\": \\\"string | null\\\" // Set when the script could not be run at all\\n}\\n\\nstatus is 'success' when the script ran, whatever it printed on stderr. status is 'error' only when nothing could be started.\\n\\nExample function call parameter: { \\\"script\\\": \\\"Get-ChildItem -Name\\\", \\\"interpreter\\\": \\\"powershell\\\", \\\"cwd\\\": \\\"C:\\\\\\\\Users\\\\\\\\me\\\\\\\\Documents\\\" }\\n\\nExample successful response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"success\\\",\\n\\\"stdout\\\": \\\"report.docx\\\\nnotes.txt\\\\n\\\",\\n\\\"stderr\\\": \\\"\\\",\\n\\\"timed_out\\\": false,\\n\\\"truncated\\\": false,\\n\\\"error_message\\\": null\\n}\\n\\nExample error response:\\n{\\n\\\"role\\\": \\\"tool\\\",\\n\\\"status\\\": \\\"error\\\",\\n\\\"stdout\\\": \\\"\\\",\\n\\\"stderr\\\": \\\"\\\",\\n\\\"timed_out\\\": false,\\n\\\"truncated\\\": false,\\n\\\"error_message\\\": \\\"The script could not be written to a temporary file.\\\"\\n}\",\
  \"parameters\": {\
    \"type\": \"object\",\
    \"properties\": {\
      \"script\": {\
        \"type\": \"string\",\
        \"description\": \"The script body. It is written to a temporary file and run by the interpreter, so it may span many lines.\"\
      },\
      \"interpreter\": {\
        \"type\": \"string\",\
        \"enum\": [\
          \"auto\",\
          \"bash\",\
          \"sh\",\
          \"zsh\",\
          \"powershell\",\
          \"cmd\"\
        ],\
        \"description\": \"Which interpreter runs the script. Leave it out (or pass 'auto') to get {{SHELL}}, which is what this machine uses.\"\
      },\
      \"cwd\": {\
        \"type\": \"string\",\
        \"description\": \"Directory to run in. Defaults to wherever the editor was started.\"\
      },\
      \"timeout_ms\": {\
        \"type\": \"integer\",\
        \"description\": \"How long the script may run before it is killed, in milliseconds. Default 30000, maximum 600000.\"\
      }\
    },\
    \"required\": [\
      \"script\"\
    ]\
  }\
}";
	}

	// Output past this is dropped: a runaway script must not blow up the
	// conversation, and the model cannot read a megabyte of it anyway.
	static const size_t c_nMaxStream = 200 * 1024;

	// The runner hands us stdout/stderr a line at a time, then one terminal
	// signal: Stop when the child ran and exited, Terminate when it never
	// started (CreateProcessW or pipe() failed) or when we killed it ourselves.
	// So Terminate that we did not ask for means "the interpreter never ran" -
	// without it, a missing interpreter is indistinguishable from a script that
	// printed nothing.
	class CCollector : public NSProcesses::CProcessRunnerCallback
	{
	public:
		CCollector() : m_bDone(false), m_bTruncated(false), m_bTerminated(false) {}

		virtual void process_callback(const int& id, const NSProcesses::StreamType& type,
		                              const std::string& message)
		{
			(void)id;
			std::unique_lock<std::mutex> oLock(m_oMutex);
			if (NSProcesses::StreamType::StdOut == type)
				Append(m_sOut, message);
			else if (NSProcesses::StreamType::StdErr == type)
				Append(m_sErr, message);
			else
			{
				m_bTerminated = (NSProcesses::StreamType::Terminate == type);
				m_bDone = true;
				m_oCond.notify_all();
			}
		}

		// True when the script finished on its own, false on timeout.
		bool WaitFor(int nTimeoutMs)
		{
			std::unique_lock<std::mutex> oLock(m_oMutex);
			return m_oCond.wait_for(oLock, std::chrono::milliseconds(nTimeoutMs),
			                        [this]() { return m_bDone; });
		}

		bool Terminated() { std::unique_lock<std::mutex> oLock(m_oMutex); return m_bTerminated; }

		std::string Out()   { std::unique_lock<std::mutex> oLock(m_oMutex); return m_sOut; }
		std::string Err()   { std::unique_lock<std::mutex> oLock(m_oMutex); return m_sErr; }
		bool Truncated()    { std::unique_lock<std::mutex> oLock(m_oMutex); return m_bTruncated; }

	private:
		void Append(std::string& sTarget, const std::string& sLine)
		{
			if (sTarget.length() >= c_nMaxStream)
			{
				m_bTruncated = true;
				return;
			}
			sTarget += sLine;
			sTarget += "\n"; // the runner strips the newline off each line
			if (sTarget.length() > c_nMaxStream)
			{
				sTarget.resize(c_nMaxStream);
				m_bTruncated = true;
			}
		}

		std::mutex m_oMutex;
		std::condition_variable m_oCond;
		std::string m_sOut;
		std::string m_sErr;
		bool m_bDone;
		bool m_bTruncated;
		bool m_bTerminated;
	};

	static std::string Failure(json& oResult, const std::string& sMessage)
	{
		oResult["status"] = "error";
		oResult["error_message"] = sMessage;
		return JSON_serialize(oResult);
	}

	// 'it''s here' - the POSIX way to put a single quote inside a quoted word.
	static std::string QuoteSingle(const std::string& sValue)
	{
		std::string sOut = "'";
		for (size_t i = 0; i < sValue.length(); ++i)
		{
			if ('\'' == sValue[i])
				sOut += "'\\''";
			else
				sOut += sValue[i];
		}
		sOut += "'";
		return sOut;
	}

	// PowerShell single quotes: the only escape is doubling the quote.
	static std::string QuotePowerShell(const std::string& sValue)
	{
		std::string sOut = "'";
		for (size_t i = 0; i < sValue.length(); ++i)
		{
			if ('\'' == sValue[i])
				sOut += "''";
			else
				sOut += sValue[i];
		}
		sOut += "'";
		return sOut;
	}

	std::string main(const std::string& arg, CAIToolsHelper* helper)
	{
		(void)helper;

		json oResult = json::object();
		oResult["role"] = "tool";
		oResult["stdout"] = "";
		oResult["stderr"] = "";
		oResult["timed_out"] = false;
		oResult["truncated"] = false;
		oResult["error_message"] = nullptr;

		// The other tools parse unguarded; a malformed argument then throws all
		// the way out into V8. Answer it instead.
		json oParam;
		try
		{
			oParam = json::parse(arg);
		}
		catch (...)
		{
			return Failure(oResult, "The tool arguments are not valid JSON.");
		}

		if (!oParam.is_object() || !oParam.contains("script") ||
		    !oParam["script"].is_string() || oParam["script"].get<std::string>().empty())
		{
			return Failure(oResult, "'script' is required and must be a non-empty string.");
		}

		std::string sBody = oParam["script"].get<std::string>();
		std::string sInterpreter = (oParam.contains("interpreter") && oParam["interpreter"].is_string())
			? oParam["interpreter"].get<std::string>() : "auto";
		std::string sCwd = (oParam.contains("cwd") && oParam["cwd"].is_string())
			? oParam["cwd"].get<std::string>() : "";

		int nTimeoutMs = 30000;
		if (oParam.contains("timeout_ms") && oParam["timeout_ms"].is_number_integer())
			nTimeoutMs = oParam["timeout_ms"].get<int>();
		if (nTimeoutMs < 1000)
			nTimeoutMs = 1000;
		if (nTimeoutMs > 600000)
			nTimeoutMs = 600000;

		if ("auto" == sInterpreter)
		{
#ifdef _WIN32
			sInterpreter = "powershell";
#else
#ifdef _MAC
			sInterpreter = "zsh";
#else
			sInterpreter = "bash";
#endif
#endif
		}

		// The runner starts the interpreter directly - there is no shell in the
		// chain - so the working directory has to come from the script itself.
		std::string sPrefix;
		std::string sExtension;
		if ("powershell" == sInterpreter)
		{
			sExtension = ".ps1";
			// Without this every non-ASCII character of the output is destroyed.
			// A child PowerShell with a redirected stdout and no console gets
			// [Console]::OutputEncoding = 437 (OEM US), which has no Cyrillic and
			// no accents, so it writes '?' for each of them - measured, not
			// guessed. chcp does not help: there is no console to re-page. cmd
			// and the POSIX shells need nothing, they pass bytes through.
			sPrefix = "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8\n";
			if (!sCwd.empty())
				sPrefix += "Set-Location -LiteralPath " + QuotePowerShell(sCwd) + "\n";
		}
		else if ("cmd" == sInterpreter)
		{
			sExtension = ".cmd";
			if (!sCwd.empty())
				sPrefix = "cd /d \"" + sCwd + "\"\r\n";
		}
		else if ("bash" == sInterpreter || "sh" == sInterpreter || "zsh" == sInterpreter)
		{
			sExtension = ".sh";
			if (!sCwd.empty())
				sPrefix = "cd " + QuoteSingle(sCwd) + "\n";
		}
		else
		{
			return Failure(oResult, "Unknown interpreter '" + sInterpreter +
			               "'. Use one of: auto, bash, sh, zsh, powershell, cmd.");
		}

		std::wstring sScriptPath = NSFile::CFileBinary::CreateTempFileWithUniqueName(
			NSFile::CFileBinary::GetTempPath(), L"oo_script");
		if (NSFile::CFileBinary::Exists(sScriptPath))
			NSFile::CFileBinary::Remove(sScriptPath);
		sScriptPath += UTF8_TO_U(sExtension);

		std::string sFull = sPrefix + sBody;
		NSFile::CFileBinary oFile;
		if (!oFile.CreateFileW(sScriptPath))
			return Failure(oResult, "The script could not be written to a temporary file.");
		if (".ps1" == sExtension)
		{
			// Windows PowerShell 5.1 reads a BOM-less .ps1 as the ANSI codepage,
			// which mangles anything non-ASCII in the script. The runner already
			// runs the child under LANG=C.UTF-8; say UTF-8 here too.
			const BYTE arBOM[3] = { 0xEF, 0xBB, 0xBF };
			oFile.WriteFile((BYTE*)arBOM, 3);
		}
		oFile.WriteFile((BYTE*)sFull.c_str(), (DWORD)sFull.length());
		oFile.CloseFile();

		std::string sScriptPathA = U_TO_UTF8(sScriptPath);

		// The first word is what Windows resolves the program from - it cuts the
		// command at the first space - so the interpreter goes first, bare, and
		// the path follows quoted. On POSIX the argv split understands double
		// quotes, so the same shape works there. The name stays bare on both:
		// the runner looks it up on PATH itself, which a hardcoded /bin/<name>
		// would defeat wherever the shell lives elsewhere (Homebrew, Nix).
		std::string sCommand;
		if ("powershell" == sInterpreter)
			sCommand = "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" + sScriptPathA + "\"";
		else if ("cmd" == sInterpreter)
			sCommand = "cmd.exe /c \"" + sScriptPathA + "\"";
		else
			sCommand = sInterpreter + " \"" + sScriptPathA + "\"";

		// Declaration order is load-bearing: the runner is destroyed first, and
		// its stop() joins the worker thread, so the collector it calls back
		// into is guaranteed to outlive it.
		CCollector oCollector;
		std::map<std::string, std::string> mapEnv; // empty: inherit ours
		NSProcesses::CProcessRunner oRunner(1, sCommand, std::move(mapEnv), &oCollector);
		oRunner.start();

		bool bFinished = oCollector.WaitFor(nTimeoutMs);
		if (!bFinished)
		{
			// stop() takes the whole child tree with it: a job object on
			// Windows, the process group on POSIX.
			oRunner.set_ended();
			oRunner.stop();
			oResult["timed_out"] = true;
		}

		NSFile::CFileBinary::Remove(sScriptPath);

		oResult["stdout"] = oCollector.Out();
		oResult["stderr"] = oCollector.Err();
		oResult["truncated"] = oCollector.Truncated();

		if (bFinished && oCollector.Terminated())
		{
			// Terminate without us asking: the interpreter never started.
			return Failure(oResult, "The interpreter '" + sInterpreter +
			               "' could not be started. Check that it is installed and on PATH.");
		}

		oResult["status"] = "success";

		// Script output is whatever some other program wrote; invalid UTF-8 in
		// it must not take the whole answer down.
		return JSON_serialize(oResult);
	}
}
