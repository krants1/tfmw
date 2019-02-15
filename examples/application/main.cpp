#include <boost/thread.hpp>
#include <t/application.h>
#include <t/ini_files.h>

class Settings : public T::IniFile {
public:
	T::Param<std::string> host{*this, "Host", "localhost"};
	T::Param<std::string> port{*this, "Port", "5555"};
	T::Param<long> launchCount{*this, "LaunchCount"};
};

class MyApplication : public T::Application {
public:
	MyApplication() {
		settings_.launchCount = settings_.launchCount + 1;
	}
	~MyApplication() {
		doSomeThing.join();
		settings_.save();
	}
	void setUp() override {
		log("Settings: " + (std::string) settings_.host + ":" + (std::string) settings_.port);
	}
	void execute() override {
		std::cout << ".";
		std::cout.flush();
	}
private:
	Settings settings_;
	boost::thread doSomeThing{[]() {
		while (!Terminated()) {
			std::cout << ",";
			std::cout.flush();
			T::Thread::sleep(250);
		}
	}};
};

int main() {
	std::cout << "Hello, App World!" << std::endl;

	MyApplication ma;
	ma.run();

	return 0;
}