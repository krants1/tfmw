#pragma once;
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <stack>
#include <iostream>

namespace T {
	template<typename T>
	using BufferPtr = std::shared_ptr<T>;

	template<typename T>
	struct BufferDebt;

	template<typename T>
	using BufferDebtPtr = std::shared_ptr<BufferDebt<T>>;

	template<typename T>
	struct BuffersStack {
		void enqueue(BufferPtr<T> s) {
			auto l = _lock();
			_stack.push(s);
		}
		BufferPtr<T> dequeue() {
			auto l = _lock();

			if (_stack.empty()) {
				if (_autoSize) {
					enqueue(BufferPtr<T>(new T()));
				} else {
					int32_t i(0);
					while (_stack.empty()) {
						i++;
						l.unlock();
						std::cout << "WARNING: BuffersStack is Empty. Wait(" << i << ")..\n";
						boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
						l.lock();
					}
				}
			}

			BufferPtr<T> s(_stack.top());
			_stack.pop();
			return s;
		}
		bool empty() {
			auto l = _lock();
			return _stack.empty();
		}
		void setAutoSize(bool autoSize) {
			_autoSize = autoSize;
		}
		void init(int count) {
			clear();
			for (int i = 0; i < count; i++)
				enqueue(BufferPtr<T>(new T()));
		}
		size_t size() {
			auto l = _lock();
			return _stack.size();
		}
		void clear() {
			auto l = _lock();
			while (!_stack.empty())
				_stack.pop();
		}

		std::unique_ptr<BufferDebt<T>> lend() {
			auto s = dequeue();
			std::unique_ptr<BufferDebt<T>> sr(new BufferDebt<T>((*this), s));
			return sr;
		}


	private:
		bool  _autoSize{false};
		boost::recursive_mutex _mutex;
		boost::unique_lock<boost::recursive_mutex> _lock() {
			boost::unique_lock<boost::recursive_mutex> l(_mutex);
			return l;
		}
		std::stack<BufferPtr<T>> _stack;
	};

	template<typename T>
	struct BufferDebt {
		BufferDebt(BuffersStack<T>& buffers, BufferPtr<T> bufferPtr) :_bufferPtr(bufferPtr), _buffers(buffers) {}

		~BufferDebt() noexcept {
			_buffers.enqueue(_bufferPtr);
		}
		BufferPtr<T> getPtrInstance() {
			return (_bufferPtr);
		}
		T & getRefInstance() {
			T * b(_bufferPtr.get());
			return (*b);
		}

	private:
		BufferPtr<T> _bufferPtr;
		BuffersStack<T>& _buffers;
	};
}