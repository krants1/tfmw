#include "../tcp.h"
#include "../time_utils.h"

#include <iostream>

int example_t_tcp() {

	class MyTCPService : public T::TCPService {
	public:
		using TCPService::TCPService;
		void handle(socket &sock) override {
			if (sock.available()) {
				std::string s = readLine(sock);
				sendLine(sock, s + " - Wuzzzzuuuup!!!");
			} else
				sleep(5);
		};
		~MyTCPService() override { stop(); }
	};

	class MyTCPClientThread : public T::TCPClient {
	private:
		int number;
	public:
		MyTCPClientThread(std::string host, short port, int number = 0) :
			TCPClient(std::move(host), port), number(number) {}
		void execute() override {
			int i = 0;
			try {
				open();
				T::SimpleTimeProfiler sp;
				sp.start();
				for (i = 1; i < 50; i++) {
					sendLine(sock, std::to_string(i) + ": Wazzup!");
					std::string s = readLine(sock);
					//log(s);
				}
				sp.finish();
				log("Done " + std::to_string(number) + ": " + std::to_string(sp.duration()));
				sock.close();
			}
			catch (std::exception &e) {
				log("[execute] " + std::string(e.what()), T::LogType::Error);
			}
		}
		~MyTCPClientThread() override { stop(); }
	};

	T::slog("TCP Service Start..");
	auto *ts = new MyTCPService(5555);
	ts->run();

	const int clientsCount = 5;
	std::vector<MyTCPClientThread *> cList;

	T::slog("TCP Clients Start..");
	for (int i = 1; i <= clientsCount; i++) {
		MyTCPClientThread *mct = new MyTCPClientThread("localhost", 5555, i);
		cList.push_back(mct);
		mct->run();
	}
	for (auto &i : cList)
		delete i;

	delete ts;
	T::slog("TCP End");

	return 0;

}