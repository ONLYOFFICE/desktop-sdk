#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#endif

#include <boost/process.hpp>
#include <thread>
#include <atomic>
#include <string>
#include <iostream>
#include <map>

#include "../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../core/DesktopEditor/graphics/TemporaryCS.h"

namespace NSProcesses
{
	enum class StreamType
	{
		StdOut,
		StdErr,
		Stop
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
			: m_command(command), m_env(map), m_callback(cb), m_running(false), m_id(id)
		{
		}

		~CProcessRunner()
		{
			stop();
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
			if (!m_running)
				return;

			m_running = false;
			if (m_proc.valid() && m_proc.running())
				m_proc.terminate();

			if (m_worker.joinable())
				m_worker.join();
		}

		int get_id()
		{
			return m_id;
		}

	private:
		void run()
		{
			boost::process::ipstream out;
			boost::process::ipstream err;

			try
			{
				boost::process::environment env = boost::this_process::environment();
				for (auto& item : m_env)
					env[item.first] = item.second;

	#ifdef _WIN32
				std::wstring commandW = UTF8_TO_U(m_command);
				m_proc = boost::process::child(commandW, boost::process::std_out > out, boost::process::std_err > err, env);
	#else
				m_proc = boost::process::child(m_command, boost::process::std_out > out, boost::process::std_err > err, env);
	#endif
			}
			catch (const std::exception& ex)
			{
				m_callback->process_callback(m_id, StreamType::StdErr, std::string("Failed to start process: ") + ex.what() + "\n");
				return;
			}

			std::thread t_out([this, &out]() {
				std::string line;
				while (m_running && std::getline(out, line))
				{
					m_callback->process_callback(m_id, StreamType::StdOut, line);
				}
			});

			std::thread t_err([this, &err]() {
				std::string line;
				while (m_running && std::getline(err, line))
				{
					m_callback->process_callback(m_id, StreamType::StdErr, line);
				}
			});

			if (m_proc.running())
				m_proc.wait();

			m_callback->process_callback(m_id, StreamType::Stop, "");

			t_out.join();
			t_err.join();
		}

	private:
		std::string				m_command;
		std::map<std::string, std::string> m_env;

		CProcessRunnerCallback* m_callback;
		std::thread				m_worker;
		std::atomic<bool>		m_running;
		boost::process::child	m_proc;
		int						m_id;
	};

	class CProcessManager : public CProcessRunnerCallback
	{
	private:
		std::vector<CProcessRunner*> m_processes;
		int							 m_counter;
		bool						 m_enable_callback;
		CProcessRunnerCallback*		 m_callback;

		NSCriticalSection::CRITICAL_SECTION m_cs;

	public:
		CProcessManager(CProcessRunnerCallback* cb)
		{
			m_cs.InitializeCriticalSection();
			m_counter = 1;
			m_enable_callback = true;
			m_callback = cb;
		}
		~CProcessManager()
		{
			StopAll();
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
					delete runner;
					m_processes.erase(iter);
					return;
				}
			}
		}

		void StopAll()
		{
			CTemporaryCS oCS(&m_cs);
			m_enable_callback = false;
			for (std::vector<CProcessRunner*>::iterator iter = m_processes.begin(); iter != m_processes.end(); iter++)
			{
				CProcessRunner* runner = *iter;
				delete runner;
			}
			m_processes.clear();
		}

		void DisableCallbacks()
		{
			CTemporaryCS oCS(&m_cs);
			m_enable_callback = false;
		}

		virtual void process_callback(const int& id, const StreamType& type, const std::string& message)
		{
			CTemporaryCS oCS(&m_cs);

			if (!m_enable_callback)
				return;

			m_callback->process_callback(id, type, message);
		}
	};
}
