#pragma once

#include <chrono>
#include <iostream>
#include <string>

#ifdef __linux__
#else
#include <windows.h>
#endif

namespace T {
	struct ThreadPriority {
#ifdef __linux__
		static void display() {
			std::cout << "__linux__ ThreadPriority::display() not supported yet" << std::endl;
		}
		static void setHighest(bool = false) {
			std::cout << "__linux__ ThreadPriority::setHighest() not supported yet" << std::endl;
		}
		static void setCritical(bool  = false) {
			std::cout << "__linux__ ThreadPriority::setCritical() not supported yet" << std::endl;
		}
		static void setNormal(bool  = false) {
			std::cout << "__linux__ ThreadPriority::setNormal() not supported yet" << std::endl;
		}
#else
		static void display() {
			int dwThreadPri = GetThreadPriority(GetCurrentThread());
			std::cout << "Current thread priority: " + std::to_string(dwThreadPri) << std::endl;
		}

		static void setHighest(bool display = false) {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			if (display) ThreadPriority::display();
		}

		static void setCritical(bool display = false) {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
			if (display) ThreadPriority::display();
		}

		static void setNormal(bool display = false) {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
			if (display) ThreadPriority::display();
		}
#endif
	};

	struct TimeProfiler {
		TimeProfiler() {
			reset();
		}
		inline void reset() {
			duration_ = std::chrono::nanoseconds::zero();
			count_ = 0;
		}
		inline void start() {
			startClock_ = clock_t::now();
		};
		inline void finish() {
			finishClock_ = clock_t::now();
			duration_ += (finishClock_ - startClock_);
			count_++;
		};
		double duration() {
			return duration_.count();
		}
		int64_t count() {
			return count_;
		}
	private:
		typedef std::chrono::high_resolution_clock clock_t;
		clock_t::time_point startClock_, finishClock_;
		std::chrono::duration<double, std::milli> duration_{};
		int64_t count_ = 0;
	};

	struct TimeOperation {
		explicit TimeOperation(int period) : period_(period) { tp_.start(); }
		bool canDoNow() {
			tp_.finish();
			tp_.start();
			return (tp_.duration() >= period_);
		}
		void endProcess() {
			tp_.reset();
			tp_.start();
		};
	private:
		int period_;
		TimeProfiler tp_;
	};
}