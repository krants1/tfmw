#include "../time_utils.h"
#include "../log.h"

int example_t_time_utils() {
	T::ThreadPriority::setHighest(true);
	T::SimpleTimeProfiler sp;

	for (int i = 0; i < 10; i++) {
		sp.start();
		T::Thread::sleep(100);
		sp.finish();
	}

	std::cout << "Total duration: " << sp.duration() << std::endl;
	std::cout << "Count: " << sp.count() << std::endl;

	T::ThreadPriority::setNormal(true);

	T::TimeOperation to(301);

	while (true) {
		if (to.canDoNow()) {
			T::slog("DoNow!");
			to.endProcess();
			break;
		}
		T::slog("Wait..");
		T::Thread::sleep(100);
	}

	return 0;
}