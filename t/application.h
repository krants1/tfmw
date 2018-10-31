#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <string>
#include "log.h"
#include <exception>

#ifdef _MSC_VER
#include <consoleapi.h>
#endif
#ifdef __linux__

#include <csignal>

#endif

namespace T {
	void SetSignalHundler();

	static bool application_run = false;
	static bool application_terminated = false;

	class Application : public LogHelper {
	private:
		LogThreadWriter ltw;
	public:
		Application() {
			log_thread_writer_static = &ltw;
			log_thread_writer_static->run();
			T::LogHelper::init(__func__);			
			SetSignalHundler();
			application_run = true;
		}
		virtual void setUp() {}
		virtual void run() {
			this->setUp();
			log("Run");
			try {
				while (!application_terminated) {
					execute();
					T::Thread::sleep(250);
				}
			}
			catch (std::exception &e) {
				log(e.what(), LogType::Error);
			}
		}
		virtual void execute() {}

		~Application() {
			log("Destroy");
			application_run = false;
		}
		static void Terminate() {
			application_terminated = true;
		}
		static bool Terminated() {
			return application_terminated;
		}
		static bool Running() {
			return application_run;
		}
	};

#ifdef __linux__
	void signalHandler(int signnum) {
		slog("\nSignal: " + std::to_string(signnum));
		Application::Terminate();
	}
	void SetSignalHundler() {
		signal(SIGINT, signalHandler);
	}
#else
	BOOL WINAPI ConsoleHandler(DWORD dwType) {
		switch (dwType) {
			case CTRL_C_EVENT:
				Application::Terminate();
				return TRUE;
			case CTRL_CLOSE_EVENT:
			case CTRL_LOGOFF_EVENT:
			case CTRL_SHUTDOWN_EVENT:
				Application::Terminate();
				while (Application::Running())
					T::Thread::sleep(10);
				return TRUE;
			default:
				return FALSE;
		}
	}
	void SetSignalHundler() {
		SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE);
	}
#endif

}
#endif //__T_APPLICATION_H__