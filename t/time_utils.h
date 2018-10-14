#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#ifdef __linux__
#define _USE_CHRONO_
#include <chrono>
#include <sched.h>
#else

#include <Windows.h>

#endif

#include "log.h"

namespace T {
	class ThreadPriority {
	public:
#ifdef __linux__
		static void display() {
			T::slog("__linux__ ThreadPriority::display() not supported yet");
		}
		static void setHighest(bool = false) {
			T::slog("__linux__ ThreadPriority::setHighest() not supported yet");
		}
		static void setCritical(bool  = false) {
			T::slog("__linux__ ThreadPriority::setCritical() not supported yet");
		}
		static void setNormal(bool  = false) {
			T::slog("__linux__ ThreadPriority::setNormal() not supported yet");
		}
#else
		static void display() {
			int dwThreadPri = GetThreadPriority(GetCurrentThread());
			printf("Current thread priority: %d\n", dwThreadPri);
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

	struct SimpleTimeProfiler {
	private:
#ifdef _USE_CHRONO_
		typedef std::chrono::high_resolution_clock clock_t;
		clock_t::time_point startClock, finishClock;
		clock_t::duration _duration;
#else
		LARGE_INTEGER _frequency;
		LARGE_INTEGER _counter, _counter2;
		int64_t _duration;
#endif

		int64_t _count = 0;
	public:
		SimpleTimeProfiler() {
#ifndef _USE_CHRONO_
			::QueryPerformanceFrequency(&_frequency);
#endif
			reset();
		}
		inline void reset() {
#ifdef _USE_CHRONO_
			_duration = std::chrono::nanoseconds::zero();
#else
			_duration = 0;
#endif
			_count = 0;
		}

		inline void start() {
#ifdef _USE_CHRONO_
			startClock = std::chrono::high_resolution_clock::now();
#else
			::QueryPerformanceCounter(&_counter);
#endif
		};

		inline void finish() {
#ifdef _USE_CHRONO_
			finishClock = std::chrono::high_resolution_clock::now();

			_duration += (finishClock - startClock);
#else
			::QueryPerformanceCounter(&_counter2);
			_duration += _counter2.QuadPart - _counter.QuadPart;
#endif
			_count++;
		};

		double duration() {
#ifdef _USE_CHRONO_
			return (_duration) / std::chrono::milliseconds(1);
#else
			return (_duration) / (double) _frequency.QuadPart * 1000;
#endif
		}

		int64_t count() {
			return _count;
		}
	};

	struct TimeOperation {
	private:
		int period;
		SimpleTimeProfiler rp;
	public:
		explicit TimeOperation(int period) : period(period) { rp.start(); }
		bool canDoNow() {
			rp.finish();
			rp.start();
			return (rp.duration() >= period);
		}
		void endProcess() {
			rp.reset();
			rp.start();
		};
	};
}
#endif //__TIME_UTILS_H__