#include "scr/tcp.h"
#include <iostream>

int main() {
	std::cout << "Hello, Protobuf!" << std::endl;

	example_t_proto_tcp();

#ifdef _MSC_VER
	system("pause");
#endif

	return 0;
}
