#pragma once

#include "log.h"

#ifdef _MSC_VER
#include <consoleapi.h>
#endif
#ifdef __linux__
#include <csignal>
#endif

namespace T {
	void setSignalHundler();

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
			T::LogHelper::init(__func__);
			setSignalHundler();
		}
		virtual void setUp() {}
		virtual void run() {
			setUp();
			log("Run");
			try {
				while (!Terminated()) {
					execute();
					T::Thread::sleep(250);
				}
			}
			catch (std::exception& e) {
				log(e.what(), LogType::Error);
			}
		}
		virtual void execute() {}

		~Application() {
			log("Destroy");
		}
	private:
		LogThreadWriter & ltw = LogThreadWriter::getInstance();
	};

#ifdef __linux__
	void signalHandler(int) {
		Application::Terminate();
	}
	void setSignalHundler() {
		signal(SIGINT, signalHandler);
	}
#else
	BOOL WINAPI ñonsoleHandler(DWORD dwType) {
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
	void setSignalHundler() {
		SetConsoleCtrlHandler((PHANDLER_ROUTINE) ñonsoleHandler, TRUE);
	}
#endif
}