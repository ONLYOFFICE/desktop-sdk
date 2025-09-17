#pragma once
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
#else
				if (m_pid > 0)
				{
					killpg(m_pid, SIGTERM);
					m_pid = -1;
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
			while (waitpid(-pid, &status, 0) > 0)
			{
				NSThreads::Sleep(500);
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
					m_callback->process_callback(m_id, type, lineBuf.substr(0, pos + 1));
					lineBuf.erase(0, pos + 1);
				}

				NSThreads::Sleep(500);
			}
			if (!lineBuf.empty())
				m_callback->process_callback(m_id, type, lineBuf);
		}

		void run()
		{
#ifdef _WIN32
			SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};

			CreatePipe(&m_hStdOutRd, &m_hStdOutWr, &sa, 0);
			SetHandleInformation(m_hStdOutRd, HANDLE_FLAG_INHERIT, 0);

			CreatePipe(&m_hStdErrRd, &m_hStdErrWr, &sa, 0);
			SetHandleInformation(m_hStdErrRd, HANDLE_FLAG_INHERIT, 0);

			STARTUPINFOW si{};
			memset(&si, 0, sizeof(si));
			si.cb = sizeof(si);
			si.hStdOutput = m_hStdOutWr;
			si.hStdError = m_hStdErrWr;
			si.dwFlags |= STARTF_USESTDHANDLES;

			std::wstring envBlock;
			for (auto& kv : m_env)
			{
				envBlock += (UTF8_TO_U((kv.first)) + L"=" + UTF8_TO_U((kv.second)));
				envBlock.push_back(L'\0');
			}
			envBlock.push_back(L'\0');

			std::wstring commandW = UTF8_TO_U(m_command);
			if (!CreateProcessW(nullptr, (LPWSTR)commandW.c_str(), nullptr, nullptr, TRUE, CREATE_SUSPENDED | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
								m_env.empty() ? nullptr : (LPVOID)(envBlock.c_str()), nullptr, &si, &m_pi))
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
			m_hStdOutWr = nullptr;
			m_hStdErrWr = nullptr;

			std::thread t_out = std::thread([this]()
				{
					readOutLoop(m_hStdOutRd, StreamType::StdOut);
				});
			std::thread t_err = std::thread([this]()
				{
					readOutLoop(m_hStdErrRd, StreamType::StdErr);
				});
#else
			if (pipe(stdoutPipe) < 0 || pipe(stderrPipe) < 0)
			{
				m_callback->process_callback(m_id, StreamType::Terminate, "");
				return;
			}

			pid = fork();
			if (pid == 0)
			{
				// child
				setsid();

				dup2(m_stdoutPipe[1], STDOUT_FILENO);
				dup2(m_stderrPipe[1], STDERR_FILENO);
				close(m_stdoutPipe[0]);
				close(m_stdoutPipe[1]);
				close(m_stderrPipe[0]);
				close(m_stderrPipe[1]);

				std::vector<std::string> envStrings;
				std::vector<char*> envp;
				for (auto& kv : m_env)
				{
					envStrings.push_back(kv.first + "=" + kv.second);
				}
				for (auto& s : envStrings)
					envp.push_back(const_cast<char*>(s.c_str()));
				envp.push_back(nullptr);

				execlpe("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr, envp.data());
				_exit(127);
			}
			else if (pid < 0)
			{
				m_callback->process_callback(m_id, StreamType::Stop, "");
				return;
			}

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
#else
		pid_t m_pid{-1};
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
