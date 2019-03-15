#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include "threads.h"
#include "paths.h"
#include <boost/lexical_cast.hpp>

#define T_LOG_EXCEPTION(ex) T::log(std::string(ex.what()) + "\n" + std::to_string(__LINE__) + ":" + __FILE__, T::LogType::Error); 	

namespace T {
	enum LogType {
		Info, Warning, Error
	};
	const std::string LogTypeStrings[] = {"Info", "Warning", "Error"};

	struct LogData {
	public:
		explicit LogData(std::string& text, LogType logType = LogType::Info, bool textOnly = false) {
			using namespace std::chrono;
			high_resolution_clock::time_point tp = high_resolution_clock::now();
			text_ = text;
			timePoint_ = tp;
			type_ = logType;
			textOnly_ = textOnly;
		}
		std::string toString() {
			if (textOnly_)
				return text_ + "\r\n";
			std::string result = timeToString() + "\t|";
			result += LogTypeStrings[type_] + "\t|";
			result += this->text_ + "\r\n";
			return result;
		}
	private:
		bool textOnly_;
		std::string text_;
		LogType type_;
		std::chrono::high_resolution_clock::time_point timePoint_;

		std::string timeToString() {
			using namespace std::chrono;
			milliseconds ms = duration_cast<milliseconds>(this->timePoint_.time_since_epoch());
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
			struct tm* p_timeinfo;
			p_timeinfo = localtime(&t);
#endif
			strftime(buffer, sizeof(buffer), "%d.%m.%y %H:%M:%S.", p_timeinfo);
			return std::string(buffer) + std::to_string(fractional_seconds);
		}
	};

	namespace statics {
		static bool logThreadWriterSingltonAssigned = false;
	}

	class LogThreadWriter : public TasksThread<LogData> {
	public:
		explicit LogThreadWriter(bool consoleMode = true, std::string fileName = "") : consoleMode_(consoleMode) {
			executeAfterTerminate_ = true;
			if (fileName.empty()) {
				fileName = T::PathHelper::getExePath();
				T::PathHelper::replaceExt(fileName, "log");
			}
			this->fileName_ = fileName;
		}

		~LogThreadWriter() override {
			stop();
		}

		void doTasks(std::queue<LogData>& tasks) override {
			std::ofstream fo(fileName_, std::ofstream::out | std::ofstream::app | std::ofstream::ate);
			std::string s;
			while (!tasks.empty()) {
				LogData* ld = &tasks.front();
				s = ld->toString();
				fo << s;
				tasks.pop();
			}
		}

		void addTask(LogData& logData) override {
			TasksThread::addTask(logData);
			if (consoleMode_)
				std::cout << logData.toString();
		}

		inline void log(std::string text, LogType logType = LogType::Info) {
			LogData ld(text, logType);
			addTask(ld);
		}
	private:
		bool consoleMode_;
		std::string fileName_;
	public: //Singlton		
		static LogThreadWriter& getInstance() {
			static LogThreadWriter instance;
			if (!statics::logThreadWriterSingltonAssigned) {
				instance.run();
				statics::logThreadWriterSingltonAssigned = true;
			}
			return instance;
		}
		static bool isSingltonAssigned() {
			return statics::logThreadWriterSingltonAssigned;
		}
	};

	static inline void log(LogData ld) {
		if (LogThreadWriter::isSingltonAssigned())
			LogThreadWriter::getInstance().addTask(ld);
		else
			std::cout << ld.toString();
	}

	static inline void log(std::string text, LogType logType = LogType::Info) {
		log(LogData(text, logType));
	}

	static inline void slog(std::string text, LogType logType = LogType::Info) {
		log(LogData(text, logType, true));
	}

	class LogHelper {
	public:
		void init(std::string const& className) {
			this->className_ = className;
		}
		void log(const std::string& text, LogType logType = LogType::Info) {
			T::log(className_ + ": " + text, logType);
		}
	private:
		std::string className_;
	};

	template<LogType LT, bool Simple >
	struct LogCast {
		std::string mess;
		template<typename T>
		LogCast &operator<<(const T &t) {
			mess += boost::lexical_cast<std::string>(t);
			return *this;
		}

		virtual ~LogCast() {
			Simple ? slog(mess, LT) : log(mess, LT);
		}

	};


	struct info : LogCast<LogType::Info, false> {};
	struct warning : LogCast<LogType::Warning, false> {};
	struct error : LogCast<LogType::Error, false> {};

	struct sinfo : LogCast<LogType::Info, true> {};
	struct swarning : LogCast<LogType::Warning, true> {};
	struct serror : LogCast<LogType::Error, true> {};
}