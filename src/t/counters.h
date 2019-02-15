#pragma once

#include <atomic>
#include <vector>
#include <string>

namespace T {
	class AtomicCounters;

	enum class CounterType {
		Title, Info, Warning, Error, Bool
	};

	class Counter : public std::atomic<int64_t> {
	public:
		Counter(AtomicCounters& owner, std::string name, CounterType counterType);
		CounterType& getType() {
			return counterType_;
		}
		const std::string& getName() {
			return name_;
		}
		void turnOn(bool value) {
			this->store(value);
		}
	private:
		AtomicCounters & owner_;
		std::string name_;
		CounterType counterType_;
	};

	class AtomicCounters {
	public:
		void add(Counter& counter) {
			list_.push_back(&counter);
		}
		const std::string getStat() {
			std::string result;
			for (Counter *c : list_) {
				switch (c->getType()) {
				case CounterType::Title:
					result.append("[" + c->getName() + "]\n");
					break;
				default:
					result.append(c->getName() + ":\t");
					result.append(std::to_string((*c)) + "\n");
				}
			}
			return result;
		}
	private:
		std::vector<Counter*> list_;
	};

	Counter::Counter(AtomicCounters& owner, std::string name, CounterType counterType = CounterType::Info)
		: owner_(owner), name_(std::move(name)), counterType_(counterType) {
		this->store(0);
		owner_.add(*this);
	}
}
