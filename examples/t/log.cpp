#include <t/log.h>

int example_t_log() {
	T::slog("Logging");
	T::log("info");
	T::info() << "info v" << 2;
	T::log("warning", T::LogType::Warning);
	T::warning() << "warning v" << 2;
	T::log("error", T::LogType::Error);
	T::error() << "error v" << 2.0;

	T::slog("\nLogHelper");
	class MyClass : public T::LogHelper {
	public:
		MyClass() {
			T::LogHelper::init(__func__);
			log("Create");
		}

		~MyClass() {
			log("Destroy");
		}
	};
	{
		MyClass m;
		m.log("Do Somthing..");
	}

	T::slog("\nLogThreadFileWriter");
	T::LogThreadWriter ltw;
	ltw.run();
	for (int i = 0; i < 10; i++)
		ltw.log("Hello World " + std::to_string(i));

	T::slog("\nEnableThreadLogging");
	T::LogThreadWriter::getInstance();
	T::info() << "Ok!";

	try {
		throw std::runtime_error("Test Exception");
	}
	catch (std::exception &ex) {
		T_LOG_EXCEPTION(ex);
	}

	return 0;
}