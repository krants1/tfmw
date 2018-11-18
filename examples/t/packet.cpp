#include <cstring>
#include <string>
#include <iostream>

#include "../../t/packet.h"

enum class TaskType {
	SUMM, MULT
};

struct Task {
	int value1;
	int value2;
	TaskType taskType;
};

struct DataResult {
	int value;
};

struct ResultPacket : T::StructPacket<1, DataResult> {
	void hundle(reply) override {
		T::slog("Result: " + std::to_string(message.value));
	}
};

struct TaskPacket : public T::StructPacket<2, Task> {
	void hundle(reply reply) override {
		std::string s = "Task: " + std::to_string(message.value1);
		s += (message.taskType == TaskType::SUMM) ? " + " : " * ";
		s += std::to_string(message.value2);
		T::slog(s);

		ResultPacket rp;
		rp.message.value = (message.taskType == TaskType::SUMM) ?
			message.value1 + message.value2 : message.value1 * message.value2;
		reply(rp);
	}
};

int example_t_packet() {

	class MyTCPServiceP : public T::TCPPacketsService {
	public:
		explicit MyTCPServiceP(unsigned short port) : TCPPacketsService(port) {
			LogHelper::init(__FUNCTION__);
			getController().add(new TaskPacket());
			getController().add(new ResultPacket());
		}

		~MyTCPServiceP() override { stop(); };
	};

	class MyTCPClientP : public T::TCPPacketsClient {
	public:
		MyTCPClientP(std::string host, short port) : TCPPacketsClient(std::move(host), port) {
			LogHelper::init(__FUNCTION__);
			getController().add(new TaskPacket());
			getController().add(new ResultPacket());
		}

		void execute() override {
			try {
				open();

				TaskPacket tp;
				tp.message.value1 = 5;
				tp.message.value2 = 7;
				tp.message.taskType = TaskType::SUMM;
				sendPacket(sock_, tp);
				managePackets(sock_);

				tp.message.taskType = TaskType::MULT;
				sendPacket(sock_, tp);
				managePackets(sock_);

				sock_.close();
			}
			catch (std::exception &e) {
				log("[execute] " + std::string(e.what()), T::LogType::Error);
			}
		}

		~MyTCPClientP() override { stop(); };
	};

	T::slog("TCP ServiceP Start..");
	auto *ts = new MyTCPServiceP(5555);
	ts->run();

	T::slog("TCP ClientP Start..");
	auto *mcp = new MyTCPClientP("localhost", 5555);
	mcp->run();

	delete mcp;
	delete ts;

	return 0;
}
