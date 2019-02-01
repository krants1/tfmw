#include "../../t/counters.h"
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

	boost::thread_group group;	
	group.add_thread(new boost::thread(f, std::ref(mc), 700000));
	group.add_thread(new boost::thread(f, std::ref(mc), 300000));
	group.add_thread(new boost::thread(f, std::ref(mc), -1000000));
	group.add_thread(new boost::thread(f, std::ref(mc), 777));
	group.join_all();

	std::cout << mc.getStat() << std::endl;

	return 0;
}