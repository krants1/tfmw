#include "../counters.h"
#include <iostream>
#include <boost/thread.hpp>

struct MyCounter : T::AtomicCounters {
	T::Counter header{*this, "MyClass", T::CounterType::Title};
	T::Counter active{*this, "Active", T::CounterType::Bool};
	T::Counter operation1{*this, "Operation1"};
	T::Counter operation2{*this, "Operation2"};
	T::Counter operation3{*this, "Operation3"};
};

int example_t_counters() {

	auto f = [](MyCounter &myCounter, int count) {
		for (int i = 0; i < abs(count); i++) {
			if (count < 0)
				myCounter.operation1--;
			else
				myCounter.operation1++;
		}
	};

	MyCounter mc;
	mc.active.turnOn(true);
	mc.operation1++;

	boost::thread
			t1(f, std::ref(mc), 700000),
			t2(f, std::ref(mc), 300000),
			t3(f, std::ref(mc), -1000000);
	t1.join();
	t2.join();
	t3.join();

	std::cout << mc.getStat() << std::endl;

	return 0;
}