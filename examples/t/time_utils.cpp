#include <t/time_utils.h>
#include <t/log.h>

int example_t_time_utils() {
	T::ThreadPriority::setHighest(true);
	T::TimeProfiler tp;
	for (int i = 0; i < 10; i++) {
		tp.start();
		T::Thread::sleep(100);
		tp.finish();
	}
	std::cout << "Total duration: " << tp.duration() << std::endl;
	std::cout << "Count: " << tp.count() << std::endl;

	T::ThreadPriority::setNormal(true);
	T::TimeOperation to(301);
	while (true) {
		if (to.canDoNow())
			break;
		T::slog("Wait..");
		T::Thread::sleep(100);
	}
	T::slog("DoProcess!");
	to.endProcess();
	return 0;
}