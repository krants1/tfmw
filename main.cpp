#include <iostream>

#ifdef _USE_PROTOBUF_
#include "proto/examples/tcp.h"
#endif

#include "t/examples/all.h"

int main() {
	std::cout << "Hello, World!" << std::endl;

	example_ini_files();
	example_t_threads();
	example_t_time_utils();
	example_t_log();
	example_t_tcp();
	example_t_packet();
	example_t_counters();

#ifdef _USE_PROTOBUF_
	example_t_proto_tcp();
#endif

#ifdef _MSC_VER
	system("pause");
#endif

	return 0;
}

