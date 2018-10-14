#include "../threads.h"

#include <string>

int example_t_threads() {

	std::cout << "MyThread" << std::endl;

	class MyThread : public T::Thread {
	private:
		const std::string par;
	public:
		MyThread(const std::string par) : par(par) {}
		void execute() {
			for (int i = 0; i < 5; i++) {
				std::cout << par + " " + std::to_string(i) << std::endl;
			}
		}
		~MyThread() {
			stop();
		}
	};

	MyThread *mt = new MyThread("test");
	mt->run();
	delete mt;

	class MyTasksThread : public T::TasksThread<int> {
	public:
		void doTasks(std::queue<int> &tasks) {
			while (!tasks.empty()) {
				sleep(300);
				std::cout << "proc: " << tasks.front() << std::endl;
				tasks.pop();
			}
		}
		MyTasksThread(bool doAllTasks) {
			executeAfterTerminate = doAllTasks;
		}
		~MyTasksThread() {
			stop();
		}
	};

	std::cout << std::endl << "doTasksInBackround" << std::endl;
	MyTasksThread *wt = new MyTasksThread(false);
	wt->run();
	for (int i = 1; i <= 5; i++) {
		T::Thread::sleep(100);
		wt->addTask(i);
	}
	delete wt;

	std::cout << std::endl << "doAllTasks" << std::endl;

	MyTasksThread *wt2 = new MyTasksThread(true);
	wt2->run();
	for (int i = 1; i <= 5; i++) {
		T::Thread::sleep(100);
		wt2->addTask(i);
	}
	delete wt2;

	return 0;
}