#include "all.h"

#include <iostream>

int main() {
	std::cout << "Hello, World!" << std::endl;

	example_ini_files();
	example_t_threads();
	example_t_time_utils();
	example_t_log();
	example_t_tcp();
	example_t_packet();
	example_t_counters();

#ifdef _MSC_VER
	system("pause");
#endif

	return 0;
}

