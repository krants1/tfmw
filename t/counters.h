#ifndef __T_COUNTERS_H__
#define __T_COUNTERS_H__

#include <atomic>
#include <vector>
#include "string"

namespace T {
	class AtomicCounters;

	enum class CounterType {
		Title, Info, Warning, Error, Bool
	};

	struct Counter : std::atomic<int64_t> {
	protected:
		AtomicCounters &owner;
		std::string name;
		CounterType counterType;
	public:
		Counter(AtomicCounters &owner, std::string name, CounterType counterType);
		CounterType getType() {
			return counterType;
		}
		const std::string &getName() {
			return name;
		}
		void turnOn(bool value) {
			this->store(value);
		}
	};

	class AtomicCounters {
	private:
		std::vector<Counter *> list;
	public:
		void reg(Counter &counter) {
			list.push_back(&counter);
		}
		const std::string getStat() {
			std::string result;
			for (Counter *c : list) {
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
	};

	Counter::Counter(AtomicCounters &owner, std::string name, CounterType counterType = CounterType::Info)
			: owner(owner), name(std::move(name)), counterType(counterType) {
		this->store(0);
		owner.reg(*this);
	}
}

#endif //__T_COUNTERS_H__
