#include "../../t/ini_files.h"
#include <iostream>
#include <string>

int example_ini_files() {
	class Settings : public T::IniFile {
	public:
		T::Param<std::string> dbPath{*this, "DBPath", "c:\\database\\test.fdb"};
		T::Param<int> threadPoolCount{*this, "ThreadPoolCount"};
		T::Param<double> lastZoom{*this, "LastZoom", 0.5};
		T::Param<long> launchCount{*this, "LaunchCount"};
		T::Param<bool> debugMode{*this, "DebugMode", false};
	};

	Settings t;
	t.launchCount = t.launchCount + 1;
	std::cout << "LaunchCount: " << t.launchCount << std::endl;
	std::cout << "DBPath: " << (std::string) t.dbPath << std::endl;
	t.save();

	return 0;
}
