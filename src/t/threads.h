#pragma once

#include <queue>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

namespace T {
	class Thread {
	public:
		virtual ~Thread() {
			checkStop();
			stop();
			delete thread_;
		}
		virtual void execute() = 0;
		void run() {
			boost::call_once(runOnceFlag_, boost::bind(&Thread::runOnce, this));
		}
		static void sleep(int64_t duration) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(duration));
		}
	protected:
		virtual void stop() {
			if (thread_ != nullptr && thread_->joinable())
				thread_->join();
			showStopAlarm_ = false;
		}
		void checkStop() {
			if (showStopAlarm_) {
				std::cout << "\nWarning: Call @Stop in destructor of derived class!!!\n";
				showStopAlarm_ = false;
			}
		}
	private:
		boost::thread* thread_ = nullptr;
		bool showStopAlarm_ = true;
		boost::once_flag runOnceFlag_ = boost::once_flag();
		void runOnce() {
			thread_ = new boost::thread(boost::bind(&Thread::execute, this));
		}
	};

	template<typename T>
	class TasksThread : public Thread {
	public:
		~TasksThread() override {
			stop();
		}
		virtual void doTasks(std::queue<T>& tasks) = 0;
		virtual void addTask(T& t) {
			boost::unique_lock<boost::mutex> l(cvMutex);
			newTasks_.push(t);
			push();
		}
	protected:
		void stop() override {
			terminated_ = true;
			push();
			Thread::stop();
		}
		void execute() override {
			while (!terminated_)
				if (waitAndGetNewTasks())
					doTasks(executeTasks_);

			if (executeAfterTerminate_)
				if (waitAndGetNewTasks(true))
					doTasks(executeTasks_);
		}
		bool executeAfterTerminate_ = false;
	private:
		bool terminated_ = false;
		std::queue<T> newTasks_;
		std::queue<T> executeTasks_;
		boost::condition_variable cv_;
		boost::mutex cvMutex;
		bool waitAndGetNewTasks(bool getOnly = false) {
			boost::unique_lock<boost::mutex> l(cvMutex);
			if (!getOnly)
				cv_.wait(l);
			while (!newTasks_.empty()) {
				executeTasks_.push(newTasks_.front());
				newTasks_.pop();
			}
			return !executeTasks_.empty();
		}
		void push() {
			cv_.notify_one();
		}
	};
}