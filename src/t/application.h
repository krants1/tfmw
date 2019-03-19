#pragma once

#include "log.h"

#ifdef _MSC_VER
#include <consoleapi.h>
#include "utils/crash_dump.h"
#endif
#ifdef __linux__
#include <csignal>
#endif

namespace T {
	static void setSignalHundler();

	namespace statics {
		static bool applicationRun = false;
		static bool applicationTerminated = false;
	}

	class BaseApplication {
	public:
		static void Terminate() {
			statics::applicationTerminated = true;
		}
		static bool Terminated() {
			return statics::applicationTerminated;
		}
		static bool Running() {
			return statics::applicationRun;
		}
		BaseApplication() {
			statics::applicationRun = true;
		}
		~BaseApplication() {
			statics::applicationRun = false;
		}
	};

	class Application : public LogHelper, public BaseApplication {
	public:
		Application() {
#ifdef _MSC_VER
			SetUnhandledExceptionFilter(UnhandledException);
#endif
			T::LogHelper::init(__func__);
			log("Create");
			setSignalHundler();
		}
		virtual void setUp() {}
		virtual int run() {
			setUp();
			log("Run");
			while (!Terminated()) {
				try {
					execute();
				}
				catch (std::exception& e) {
					log(e.what(), LogType::Error);
				}
				T::Thread::sleep(executeDelay_);
			}
			return exitCode_;
		}
		virtual void execute() {}

		~Application() {
			log("Destroy");
		}
	protected:
		int executeDelay_ = 250;
		int exitCode_ = 0;
	private:
		LogThreadWriter & ltw = LogThreadWriter::getInstance();
	};

#ifdef __linux__
	static void signalHandler(int) {
		Application::Terminate();
	}
	static void setSignalHundler() {
		signal(SIGINT, signalHandler);
	}
#else
	static BOOL WINAPI consoleHandler(DWORD dwType) {
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
	static void setSignalHundler() {
		SetConsoleCtrlHandler((PHANDLER_ROUTINE) consoleHandler, TRUE);
	}
#endif
}