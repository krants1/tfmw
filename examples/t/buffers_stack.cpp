#include <t/buffers_stack.h>
#include <t/log.h>
#include <t/time_utils.h>

int example_t_buffers_stack() {

	T::slog("BuffersStack Start");
	{
		struct MyBuffer {
			char *buffer;
			MyBuffer() {
				buffer = new char[100 * 1024 * 1024];
				T::log("MyBuffer create");
			}
			~MyBuffer() {
				delete[] buffer;
				T::log("MyBuffer destroy");
			}
		};

		T::BuffersStack<MyBuffer> bs;
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
					auto b = bd->getPtrInstance();
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


		T::BuffersStack<MyBuffer> bs2;
		T::TimeProfiler tp;
		tp.start();
		while (!bs.empty()) {
			auto s = bs.dequeue();
			bs2.enqueue(s);
		}
		tp.finish();
		T::log("Duration: " + tp.durationStr());

		tp.reset();
		tp.start();
		int64_t sizeChange = bs2.size();
		{
			auto f2 = [](MyBuffer  & buf) {
			};
			auto bd = bs2.lend();
			f2(bd->getRefInstance());
			f2(bs2.lend()->getRefInstance());
			f2(bs2.lend()->getRefInstance());
			f2(bs2.lend()->getRefInstance());			
			sizeChange -= bs2.size();			
		}
		tp.finish();
		T::log("Size change: " + std::to_string(sizeChange));
		T::log("Duration: " + tp.durationStr());
	}

	T::slog("BuffersStack End");
	return 0;
}