#include <string>
#include <t/threads.h>

int example_t_threads() {
	std::cout << "MyThread" << std::endl;

	class MyThread : public T::Thread {
	public:
		explicit MyThread(std::string param) : param_(std::move(param)) {}
		void execute() final {
			for (int i = 0; i < 5; i++) {
				std::cout << param_ + " " + std::to_string(i) << std::endl;
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
				std::cout << "proc: " << tasks.front() << std::endl;
				tasks.pop();
			}
		}
	};

	std::cout << "\ndoTasksInBackround" << std::endl;
	auto *wt = new MyTasksThread(false);
	wt->run();
	for (int i = 1; i <= 5; i++) {
		T::Thread::sleep(100);
		wt->addTask(i);
	}
	delete wt;

	std::cout << "\ndoAllTasks" << std::endl;

	auto *wt2 = new MyTasksThread(true);
	wt2->run();
	for (int i = 1; i <= 5; i++) {
		T::Thread::sleep(100);
		wt2->addTask(i);
	}
	delete wt2;

	return 0;
}