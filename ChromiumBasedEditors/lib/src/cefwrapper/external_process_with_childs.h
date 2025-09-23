#pragma once
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <atomic>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#endif

#ifdef _MAC
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif
#endif

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "../../../../../core/DesktopEditor/graphics/BaseThread.h"

namespace NSProcesses
{
	enum class StreamType
	{
		StdOut,
		StdErr,
		Stop,
		Terminate
	};

	class CProcessRunnerCallback
	{
	public:
		CProcessRunnerCallback(){}
		virtual ~CProcessRunnerCallback(){}

		virtual void process_callback(const int& id, const StreamType& type, const std::string& message) = 0;
	};

	class CProcessRunner
	{
	public:
		CProcessRunner(const int& id, const std::string& command, std::map<std::string, std::string>&& map, CProcessRunnerCallback* cb)
			: m_command(command), m_env(map), m_callback(cb), m_running(false), m_id(id), m_isEnded(false)
		{
		}

		~CProcessRunner()
		{
			stop();
		}

		int get_id()
		{
			return m_id;
		}

		void set_ended()
		{
			m_isEnded = true;
		}

		void start()
		{
			if (m_running)
				return;

			m_running = true;

			m_worker = std::thread([this]() {
				run();
			});
		}

		void stop()
		{
			if (m_running.load())
			{
#ifdef _WIN32
				if (m_hJob)
					CloseHandle(m_hJob);
				m_hJob = nullptr;

				if (m_hStdOutRd) { CloseHandle(m_hStdOutRd); m_hStdOutRd = nullptr; }
				if (m_hStdOutWr) { CloseHandle(m_hStdOutWr); m_hStdOutWr = nullptr; }
				if (m_hStdErrRd) { CloseHandle(m_hStdErrRd); m_hStdErrRd = nullptr; }
				if (m_hStdErrWr) { CloseHandle(m_hStdErrWr); m_hStdErrWr = nullptr; }
				if (m_hStdInRd)  { CloseHandle(m_hStdInRd);  m_hStdInRd  = nullptr; }
				if (m_hStdInWr)  { CloseHandle(m_hStdInWr);  m_hStdInWr  = nullptr; }
#else
				if (m_pid > 0)
				{
					killpg(m_pid, SIGTERM);
					m_pid = -1;

					if (m_stdinPipe[1] >= 0)
					{
						close(m_stdinPipe[1]);
						m_stdinPipe[1] = -1;
					}
				}
#endif
			}

			m_running.store(false);
			if (m_worker.joinable())
			{
				if (std::this_thread::get_id() != m_worker.get_id())
					m_worker.join();
				else
					m_worker.detach();
			}
		}

		void wait()
		{
#ifdef _WIN32
			WaitForSingleObject(m_pi.hProcess, INFINITE);
#else
			int status;
			pid_t wpid;

			while (true)
			{
				NSThreads::Sleep(500);
				wpid = waitpid(-m_pid, &status, 0);
				if (wpid > 0)
				{
					continue;
				}
				else if (wpid == -1)
				{
					if (errno == EINTR)
					{
						continue;
					}
					break;
				}
				else
				{
					break;
				}
			}
#endif
		}

		void write_stdin(const std::string& data)
		{
#ifdef _WIN32
			if (m_hStdInWr)
			{
				DWORD written = 0;
				WriteFile(m_hStdInWr, data.c_str(), static_cast<DWORD>(data.size()), &written, nullptr);
			}
#else
			if (m_stdinPipe[1] >= 0)
			{
				::write(m_stdinPipe[1], data.c_str(), data.size());
			}
#endif
		}

	private:

		void readOutLoop(
#ifdef _WIN32
			HANDLE handle
#else
			int fd
#endif
			, const StreamType& type
			)
		{
			std::string lineBuf;
			char buf[256]; DWORD n;
			while (m_running.load() &&
#ifdef _WIN32
				   ReadFile(handle, buf, sizeof(buf), &n, nullptr) && n > 0
#else
				   (n = read(fd, buf, sizeof(buf))) > 0
#endif
				   )
			{
				lineBuf.append(buf, n);
				size_t pos;
				while ((pos = lineBuf.find('\n')) != std::string::npos)
				{
					m_callback->process_callback(m_id, type, lineBuf.substr(0, pos));
					lineBuf.erase(0, pos + 1);
				}

				NSThreads::Sleep(500);
			}
			if (!lineBuf.empty())
				m_callback->process_callback(m_id, type, lineBuf);
		}

#ifdef _LINUX
		std::string findBinary(const std::string& cmd)
		{
			if (!cmd.empty() && access(cmd.c_str(), X_OK) == 0)
				return cmd;

			const char* pathEnv = std::getenv("PATH");
			if (!pathEnv)
				return cmd;

			std::istringstream iss(pathEnv);
			std::string dir;
			while (std::getline(iss, dir, ':'))
			{
				std::string full = dir + "/" + cmd;
				if (access(full.c_str(), X_OK) == 0)
					return full;
			}
			return cmd;
		}
#endif

		void run()
		{
#ifdef _WIN32
			SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};

			CreatePipe(&m_hStdOutRd, &m_hStdOutWr, &sa, 0);
			SetHandleInformation(m_hStdOutRd, HANDLE_FLAG_INHERIT, 0);

			CreatePipe(&m_hStdErrRd, &m_hStdErrWr, &sa, 0);
			SetHandleInformation(m_hStdErrRd, HANDLE_FLAG_INHERIT, 0);

			CreatePipe(&m_hStdInRd, &m_hStdInWr, &sa, 0);
			SetHandleInformation(m_hStdInWr, HANDLE_FLAG_INHERIT, 0);

			STARTUPINFOW si{};
			memset(&si, 0, sizeof(si));
			si.cb = sizeof(si);
			si.hStdInput  = m_hStdInRd;
			si.hStdOutput = m_hStdOutWr;
			si.hStdError = m_hStdErrWr;
			si.dwFlags |= STARTF_USESTDHANDLES;

			std::wstring envBlock;
			for (auto& kv : m_env)
			{
				envBlock += (UTF8_TO_U((kv.first)) + L"=" + UTF8_TO_U((kv.second)));
				envBlock.push_back(L'\0');
			}
			envBlock += L"LANG=C.UTF-8\0";
			envBlock.push_back(L'\0');

			std::wstring commandW = UTF8_TO_U(m_command);
			if (!CreateProcessW(nullptr, (LPWSTR)commandW.c_str(), nullptr, nullptr, TRUE, CREATE_SUSPENDED | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
								(LPVOID)(envBlock.c_str()), nullptr, &si, &m_pi))
			{
				DWORD dwError = GetLastError();

				m_callback->process_callback(m_id, StreamType::Terminate, "");
				return;
			}

			m_hJob = CreateJobObject(nullptr, nullptr);
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{};
			jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			SetInformationJobObject(m_hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
			AssignProcessToJobObject(m_hJob, m_pi.hProcess);

			ResumeThread(m_pi.hThread);

			CloseHandle(m_hStdOutWr);
			CloseHandle(m_hStdErrWr);
			CloseHandle(m_hStdInRd);
			m_hStdOutWr = nullptr;
			m_hStdErrWr = nullptr;
			m_hStdInRd = nullptr;

			std::thread t_out = std::thread([this]()
											{
												readOutLoop(m_hStdOutRd, StreamType::StdOut);
											});
			std::thread t_err = std::thread([this]()
											{
												readOutLoop(m_hStdErrRd, StreamType::StdErr);
											});
#else
			if (pipe(m_stdinPipe) < 0 || pipe(m_stdoutPipe) < 0 || pipe(m_stderrPipe) < 0)
			{
				m_callback->process_callback(m_id, StreamType::Terminate, "");
				return;
			}

			m_pid = fork();
			if (m_pid == 0)
			{
				setsid();
				setpgid(0, 0);

				dup2(m_stdinPipe[0], STDIN_FILENO);
				dup2(m_stdoutPipe[1], STDOUT_FILENO);
				dup2(m_stderrPipe[1], STDERR_FILENO);

				close(m_stdinPipe[0]); close(m_stdinPipe[1]);
				close(m_stdoutPipe[0]); close(m_stdoutPipe[1]);
				close(m_stderrPipe[0]); close(m_stderrPipe[1]);

				std::vector<std::string> args;
				{
					std::istringstream iss(m_command);
					std::string token;
					while (iss >> std::quoted(token)) // "..."
						args.push_back(token);
				}
				if (args.empty())
					_exit(127);

				std::vector<char*> argv;
				for (auto& s : args)
					argv.push_back(const_cast<char*>(s.c_str()));
				argv.push_back(nullptr);

				std::map<std::string, std::string> mergedEnv;
				for (char **env = environ; *env; ++env)
				{
					std::string s(*env);
					auto pos = s.find('=');
					if (pos != std::string::npos)
					{
						mergedEnv[s.substr(0, pos)] = s.substr(pos + 1);
					}
				}
				for (auto &kv : m_env)
				{
					mergedEnv[kv.first] = kv.second;
				}

				std::vector<std::string> envStrings;
				std::vector<char*> envp;
				for (auto &kv : mergedEnv)
				{
					envStrings.push_back(kv.first + "=" + kv.second);
				}
				for (auto &s : envStrings)
				{
					envp.push_back(const_cast<char*>(s.c_str()));
				}
				envp.push_back(nullptr);

				std::string sProgramm(argv[0]);
				sProgramm = findBinary(sProgramm);

				execve(sProgramm.c_str(), argv.data(), envp.data());
				_exit(127);
			}
			else if (m_pid < 0)
			{
				m_callback->process_callback(m_id, StreamType::Stop, "");
				return;
			}

			close(m_stdinPipe[0]);
			close(m_stdoutPipe[1]);
			close(m_stderrPipe[1]);

			std::thread t_out = std::thread([this]()
											{
				                                readOutLoop(m_stdoutPipe[0], StreamType::StdOut);
											});
			std::thread t_err = std::thread([this]()
											{
				                                readOutLoop(m_stderrPipe[0], StreamType::StdErr);
											});

#endif

			wait();

			m_running.store(false);

			if (t_out.joinable())
				t_out.join();
			if (t_err.joinable())
				t_err.join();

			m_callback->process_callback(m_id, m_isEnded ? StreamType::Terminate : StreamType::Stop, "");
		}

	private:
		std::string				m_command;
		std::map<std::string, std::string> m_env;

		CProcessRunnerCallback* m_callback;
		std::thread				m_worker;
		std::atomic<bool>		m_running;
		int						m_id;

		bool					m_isEnded;

#ifdef _WIN32
		PROCESS_INFORMATION m_pi{};
		HANDLE m_hJob{nullptr};
		HANDLE m_hStdOutRd{nullptr}, m_hStdOutWr{nullptr};
		HANDLE m_hStdErrRd{nullptr}, m_hStdErrWr{nullptr};
		HANDLE m_hStdInRd{ nullptr },  m_hStdInWr{ nullptr };
#else
		pid_t m_pid{-1};
		int m_stdinPipe[2];
		int m_stdoutPipe[2];
		int m_stderrPipe[2];
#endif
	};

	class CProcessManager : public CProcessRunnerCallback
	{
	private:
		std::vector<CProcessRunner*> m_processes;
		int							 m_counter;
		std::atomic<bool>			 m_enable_callback;
		CProcessRunnerCallback*		 m_callback;

		NSCriticalSection::CRITICAL_SECTION m_cs;
		NSCriticalSection::CRITICAL_SECTION m_cs_callback;

	public:
		CProcessManager(CProcessRunnerCallback* cb)
		{
			m_cs.InitializeCriticalSection();
			m_cs_callback.InitializeCriticalSection();
			m_counter = 1;
			m_enable_callback.store(true);
			m_callback = cb;
		}
		~CProcessManager()
		{
			StopAll();
			m_cs_callback.DeleteCriticalSection();
			m_cs.DeleteCriticalSection();
		}

		int Start(const std::string& command, std::map<std::string, std::string>&& env)
		{
			m_cs.Enter();
			int cur_id = m_counter++;
			CProcessRunner* runner = new CProcessRunner(cur_id, command, std::move(env), this);
			m_processes.push_back(runner);
			m_cs.Leave();
			runner->start();
			return cur_id;
		}

		void End(const int& id)
		{
			CTemporaryCS oCS(&m_cs);

			for (std::vector<CProcessRunner*>::iterator iter = m_processes.begin(); iter != m_processes.end(); iter++)
			{
				CProcessRunner* runner = *iter;
				if (runner->get_id() == id)
				{
					m_processes.erase(iter);
					runner->set_ended();
					delete runner;
					return;
				}
			}
		}

		void SendStdIn(const int& id, const std::string& data)
		{
			CTemporaryCS oCS(&m_cs);

			for (std::vector<CProcessRunner*>::iterator iter = m_processes.begin(); iter != m_processes.end(); iter++)
			{
				CProcessRunner* runner = *iter;
				if (runner->get_id() == id)
				{
					runner->write_stdin(data);
					return;
				}
			}
		}

		void StopAll()
		{
			m_enable_callback.store(false);

			CTemporaryCS oCS(&m_cs);
			for (std::vector<CProcessRunner*>::iterator iter = m_processes.begin(); iter != m_processes.end(); iter++)
			{
				CProcessRunner* runner = *iter;
				delete runner;
			}
			m_processes.clear();
		}

		virtual void process_callback(const int& id, const StreamType& type, const std::string& message)
		{
			CTemporaryCS oCS(&m_cs_callback);

			if (!m_enable_callback.load())
				return;

			m_callback->process_callback(id, type == StreamType::Terminate ? StreamType::Stop : type, message);

			if (type == StreamType::Stop)
				End(id);
		}
	};
}
