#include <boost/thread.hpp>
#include "../t/application.h"
#include "../t/ini_files.h"

class Settings : public T::IniFile {
public:
	T::Param<std::string> host{*this, "Host", "localhost"};
	T::Param<std::string> port{*this, "Port", "5555"};
	T::Param<long> launchCount{*this, "LaunchCount"};
};

class MyApplication : public T::Application {
private:
	Settings settings;
	boost::thread doSomeThing{[]() {
		while (!Terminated()) {
			std::cout << ",";
			std::cout.flush();
			T::Thread::sleep(250);
		}
	}};
public:
	MyApplication() {
		settings.launchCount = settings.launchCount + 1;
		log("Settings: " + (std::string) settings.host + ":" + (std::string) settings.port);
	}
	~MyApplication() {
		doSomeThing.join();
		settings.save();
	}
	void execute() override {
		std::cout << ".";
		std::cout.flush();
	}
};

int main() {
	std::cout << "Hello, App World!" << std::endl;

	MyApplication ma;
	ma.run();

	return 0;

}