#include "../../t/buffers_stack.h"
#include "../../t/log.h"

int example_t_buffers_stack() {

	T::slog("BuffersStack Start");
	{
		struct MyBuffer {
			MyBuffer() { T::log("MyBuffer create"); }
			~MyBuffer() { T::log("MyBuffer destroy"); }
		};

		T::BuffersStack<MyBuffer> bs, bs2;
		bs.setAutoSize(true);
		bs.enqueue(bs.dequeue());
		T::log("clear()");
		bs.clear();
		bs.setAutoSize(false);

		int count = 4;
		T::log("init()");
		bs.init(count);

		T::log("Size before: " + std::to_string(bs.size()));
		auto f = [](T::BuffersStack<MyBuffer> &bs) {
			{
				//like RAAI			
				{
					auto bd = bs.lend();
					auto b = bd->getInstance();
					T::log("Size: " + std::to_string(bs.size()));
					// do something
					boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
				}
			}
		};

		boost::thread_group group;
		for (auto i = 0; i < count + 1; i++) {
			group.add_thread(new boost::thread(f, std::ref(bs)));
			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
		}

		group.join_all();
		T::log("Size after: " + std::to_string(bs.size()));

		while (!bs.empty()) {
			auto s(bs.dequeue());
			bs2.enqueue(s);
		}
	}

	T::slog("BuffersStack End");
	return 0;
}