#ifndef __T_THREADS_H__
#define __T_THREADS_H__

#include <queue>
#include <iostream>

#include <boost/thread.hpp>

namespace T {
	class Thread {
	private:
		boost::once_flag runOnceFlag = boost::once_flag();
		void runOnce() {
			thr = new boost::thread(boost::bind(&Thread::execute, this));
		}
	protected:
		boost::thread *thr = nullptr;
		bool showStopAlarm = true;
		void checkStop() {
			if (showStopAlarm)
				std::cout << "\nWarning: Call @Stop in destructor of derived class\n";
			showStopAlarm = false;
		}
		virtual void stop() {
			if (thr != nullptr && thr->joinable())
				thr->join();
			showStopAlarm = false;
		}
	public:
		void run() {
			boost::call_once(runOnceFlag, boost::bind(&Thread::runOnce, this));
		}
		virtual ~Thread() {
			checkStop();
			stop();
			delete thr;
		}

		virtual void execute() = 0;

	public:
		static void sleep(int64_t duration) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(duration));
		}
	};

	template<typename T>
	class TasksThread : public Thread {
	protected:
		bool terminated = false;
		bool executeAfterTerminate = false;
		std::queue<T> newTasks;
		std::queue<T> execTasks;

		boost::condition_variable cv;
		boost::mutex cvMutex;

		void stop() override {
			terminated = true;
			push();
			Thread::stop();
		}
		bool waitAndGetNewTasks(bool getOnly = false) {
			boost::unique_lock<boost::mutex> l(cvMutex);
			if (!getOnly)
				cv.wait(l);
			while (!newTasks.empty()) {
				execTasks.push(newTasks.front());
				newTasks.pop();
			}
			return !execTasks.empty();
		}
		void push() {
			cv.notify_one();
		}
		void execute() override {
			while (!terminated)
				if (waitAndGetNewTasks())
					doTasks(execTasks);

			if (executeAfterTerminate)
				if (waitAndGetNewTasks(true))
					doTasks(execTasks);
		}
	public:
		virtual void addTask(T t) {
			boost::unique_lock<boost::mutex> l(cvMutex);
			newTasks.push(t);
			push();
		}

		virtual void doTasks(std::queue<T> &tasks) = 0;

		~TasksThread() override {
			stop();
		}
	};
}

#endif //__T_THREADS_H__