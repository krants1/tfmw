#include <string>
#include <t/threads.h>
#include <t/log.h>

int example_t_threads() {
	T::sinfo() << "MyThread";

	class MyThread : public T::Thread {
	public:
		explicit MyThread(std::string param) : param_(std::move(param)) {}
		void execute() final {
			for (int i = 0; i < 5; i++) {
				T::sinfo() << param_ + " " + std::to_string(i);
			}
		}
		~MyThread() override {
			stop();
		}
	private:
		const std::string param_;
	};

	MyThread *mt = new MyThread("test");
	mt->run();
	delete mt;

	class MyTasksThread : public T::TasksThread<int> {
	public:
		explicit MyTasksThread(bool doAllTasks) {
			executeAfterTerminate_ = doAllTasks;
		}
		~MyTasksThread() override {
			stop();
		}
		void doTasks(std::queue<int> &tasks) final {
			while (!tasks.empty()) {
				sleep(300);
				T::sinfo() << "proc: " << tasks.front();
				tasks.pop();
			}
		}
	};

	{
		T::sinfo() << "doTasksInBackround";
		MyTasksThread wt(false);
		wt.run();
		for (int i = 1; i <= 5; i++) {
			T::Thread::sleep(50);
			wt.addTask(i);
		}
	}

	{
		T::sinfo() << "doAllTasks";
		MyTasksThread wt(true);
		wt.run();
		for (int i = 1; i <= 5; i++) {
			T::Thread::sleep(50);
			wt.addTask(i);
		}
	}

	return 0;
}