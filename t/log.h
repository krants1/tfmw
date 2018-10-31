#ifndef __T_LOG_H__
#define __T_LOG_H__

#include <string>
#include <chrono>
#include <fstream>
#include <ctime>
#include "threads.h"

#ifdef __linux__

#include <unistd.h>
#include <linux/limits.h>

#else
#include <windows.h>
#endif

namespace T {
	enum class LogType {
		Info, Warning, Error
	};
	const std::string LogTypeStrings[] = {"Info", "Warning", "Error"};

	struct LogData {
	private:
		bool textOnly;
		std::string text;
		LogType type;
		std::chrono::high_resolution_clock::time_point timePoint;
		std::string timeToString() {
			using namespace std::chrono;
			milliseconds ms = duration_cast<milliseconds>(this->timePoint.time_since_epoch());
			seconds s = duration_cast<seconds>(ms);
			std::time_t t = s.count();
			std::size_t fractional_seconds = (std::size_t) ms.count() % 1000;
			char buffer[40];
			time(&t);
#ifdef _MSC_VER
			struct tm timeinfo;
			localtime_s(&timeinfo, &t);
			struct tm *p_timeinfo = &timeinfo;
#else
			struct tm *p_timeinfo;
			p_timeinfo = localtime(&t);
#endif		
			strftime(buffer, sizeof(buffer), "%d.%m.%y %H:%M:%S.", p_timeinfo);
			return std::string(buffer) + std::to_string(fractional_seconds);
		}
	public:
		explicit LogData(std::string text, LogType logType = LogType::Info, bool textOnly = false) {
			using namespace std::chrono;
			high_resolution_clock::time_point tp = high_resolution_clock::now();
			this->text = std::move(text);
			this->timePoint = tp;
			this->type = logType;
			this->textOnly = textOnly;
		}

		std::string toString() {
			if (textOnly)
				return text + "\r\n";
			std::string result = timeToString() + "\t|";
			result += LogTypeStrings[0] + "\t|";
			result += this->text + "\r\n";
			return result;
		}
	};

	struct PathHelper {
		static std::string getExePath() {
#ifdef __linux__
			char result[PATH_MAX];
			ssize_t len = readlink("/proc/self/exe", result, PATH_MAX);
			if (len != -1) {
				result[len] = '\0';
				return std::string(result);
			} else
				return "";
#else
			char result[MAX_PATH];
			DWORD size = GetModuleFileNameA(NULL, result, MAX_PATH);
			return (size) ? std::string(result) : "";
#endif
		}

		static void replaceExt(std::string &fileName, const std::string &newExt) {
			std::string::size_type i = fileName.rfind('.', fileName.length());
			if (i != std::string::npos)
				fileName.replace(i + 1, newExt.length(), newExt);
			else
				fileName += "." + newExt;
		}
	};

	class LogThreadWriter : public TasksThread<LogData> {
	private:
		bool consoleMode;
		std::string fileName;
	public:
		void doTasks(std::queue<LogData> &tasks) override {
			std::ofstream fo(fileName, std::ofstream::out | std::ofstream::app | std::ofstream::ate);
			std::string s;
			while (!tasks.empty()) {
				LogData *ld = &tasks.front();
				s = ld->toString();
				fo << s;
				//fo.write(s.c_str(), sizeof(char)*s.size());
				tasks.pop();
			}
		}

		explicit LogThreadWriter(bool consoleMode = true, std::string fileName = "") : consoleMode(consoleMode) {
			executeAfterTerminate = true;
			if (fileName.empty()) {
				fileName = T::PathHelper::getExePath();
				T::PathHelper::replaceExt(fileName, "log");
			}
			this->fileName = fileName;
		}

		~LogThreadWriter() override {
			stop();
		}

		void addTask(LogData logData) override {
			TasksThread::addTask(logData);
			if (consoleMode)
				std::cout << logData.toString();
		}

		inline void log(std::string text, LogType logType = LogType::Info) {
			addTask(LogData(std::move(text), logType));
		}
	};

	static LogThreadWriter *log_thread_writer_static;

	static inline void log(LogData ld) {
		if (log_thread_writer_static == nullptr)
			std::cout << ld.toString();
		else
			log_thread_writer_static->addTask(ld);
	}

	static inline void log(std::string text, LogType logType = LogType::Info) {
		log(LogData(std::move(text), logType));
	}

	static inline void slog(std::string text, LogType logType = LogType::Info) {
		log(LogData(std::move(text), logType, true));
	}

	class LogHelper {
	protected:
		std::string className;
	public:
		void init(std::string const &clsName) {
			this->className = clsName;
		}

		void log(const std::string &text, LogType logType = LogType::Info) {
			T::log(className + ": " + text, logType);
		}
	};
}
#endif //__T_LOG_H__
