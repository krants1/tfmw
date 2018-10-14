#include "../log.h"

int example_t_log() {
	T::log("info");
	T::log("warning", T::LogType::Warning);
	T::log("error", T::LogType::Error);

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
	MyClass *m = new MyClass;
	m->log("Do Somthing..");
	delete m;

	T::slog("\nLogThreadWriter");
	T::LogThreadWriter ltw;
	ltw.run();
	for (int i = 0; i < 10; i++)
		ltw.log("Hello World " + std::to_string(i));

	return 0;
}