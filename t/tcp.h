#pragma once

#include <boost/asio.hpp>
#include "threads.h"
#include "log.h"

namespace T {
	class SocketHelper {
	public:
		typedef boost::asio::ip::tcp btcp;
		typedef btcp::socket socket;
	protected:
		void send(socket &s, const std::string &message) {
			boost::asio::write(s, boost::asio::buffer(message));
		}
		void sendLine(socket &s, const std::string &message) {
			send(s, message + "\n");
		}
		std::string readLine(socket &s) {
			// boost 1.64
			boost::asio::streambuf b;
			boost::asio::read_until(s, b, '\n');
			std::istream is(&b);
			std::string line;
			std::getline(is, line);
			return line;
			// boost 1.66 +
			/*
			std::string data;
			std::size_t n = boost::asio::read_until(s, boost::asio::dynamic_buffer(data), '\n');
			return data.substr(0, n);
			 */
		}
		int readInt(socket &s) {
			char b[4];
			boost::asio::read(s, boost::asio::buffer(b), boost::asio::transfer_exactly(4));
			return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
		}
		void sendInt(socket &s, int i) {
			char b[4];
			b[0] = (char) (i & 0x000000ff);
			b[1] = (char) (i & 0x0000ff00) >> 8;
			b[2] = (char) (i & 0x00ff0000) >> 16;
			b[3] = (char) (i & 0xff000000) >> 24;

			s.send(boost::asio::buffer(b, 4));
		}
	};

	class TCPService : public Thread, public SocketHelper, public LogHelper {
	public:
		explicit TCPService(unsigned short port) : SocketHelper(), port_(port) {
			LogHelper::init(__func__);
			threadsCount_.store(0);
		}
		virtual void handle(socket &sock) = 0;
		void execute() override {
			try {
				const btcp::endpoint endpoint(btcp::v4(), port_);
				acceptor.open(endpoint.protocol());
				acceptor.bind(endpoint);
				acceptor.listen();
				log("Start listening..");
				while (acceptor.is_open()) {
					std::shared_ptr<socket> sock = std::make_shared<socket>(io_service_);
					boost::system::error_code ec;
					acceptor.accept(*sock, ec);
					if (!ec) doAccept(sock);
				}
			}
			catch (std::exception &e) {
				log("[execute] " + std::string(e.what()), LogType::Error);
			}
		}
		void stop() override {
			terminated_ = true;
			try {
				if (acceptor.is_open()) {
					log("End listening");
					acceptor.close();
				}
			}
			catch (std::exception &e) {
				log("[stop] " + std::string(e.what()), LogType::Error);
			}
			int64_t waitDuration = 50;
			while (this->threadsCount_ > 0) {
				sleep(waitDuration);
				if (this->threadsCount_ > 0) {
					log("wait loops..");
					waitDuration = 1000;
				}
			}
			Thread::stop();
		}
	protected:
		void loop(const std::shared_ptr<socket> &sock) {
			try {
				while ((*sock).is_open() && !terminated_)
					this->handle(*sock);
			}
			catch (std::exception &e) {
				log("[loop] " + std::string(e.what()), LogType::Error);
			}
			log("[disconnected]");
			this->threadsCount_--;
		}
		void doAccept(const std::shared_ptr<socket> &sock) {
			btcp::endpoint lep = sock->local_endpoint();
			log("[accept] " + lep.address().to_string() + ":" + std::to_string(lep.port()));
			// ToDo: create TCPSocketThread
			boost::thread(&TCPService::loop, this, sock).detach();
			// ToDo: create TCPSocketThreadsList
			this->threadsCount_++;
		}
	private:
		unsigned short port_;
		bool terminated_ = false;
		boost::asio::io_service io_service_;
		btcp::acceptor acceptor{io_service_};
		std::atomic<int32_t> threadsCount_{};
	};

	class TCPClient : public Thread, public SocketHelper, public LogHelper {
	public:
		TCPClient(std::string host, unsigned short port) : SocketHelper(), host_(std::move(host)), port_(port) {
			LogHelper::init(__func__);
		};
		virtual bool open() {
			btcp::resolver resolver(io_service_);
			boost::asio::connect(sock_, resolver.resolve({host_, std::to_string(port_)}));
			return sock_.is_open();
		}
		virtual void close() {
			try {
				if (sock_.is_open())
					sock_.close();
			}
			catch (std::exception &e) {
				log("[close] " + std::string(e.what()), LogType::Error);
			}
		}
	protected:
		void stop() override {
			Thread::stop();
			close();
		}
		boost::asio::io_service io_service_;
		socket sock_{io_service_};
		std::string host_;
		unsigned short port_;
	};

	class TCPAutoRestoreClient : public TCPClient {
	public:
		enum class Event { Open, Execute, Close };
		TCPAutoRestoreClient(std::string host, unsigned short port) : TCPClient(std::move(host), port) {
			LogHelper::init(__func__);
		};

		virtual void handle(Event event) = 0;

		void close() override {
			TCPClient::close();
			handle(Event::Close);
		}
		bool open() override {
			TCPClient::close();
			bool result = false;
			try {
				result = TCPClient::open();
			}
			catch (std::exception &e) {
				log("[open] " + std::string(e.what()), LogType::Error);
			}
			if (result) {
				needReconnect_ = false;
				handle(Event::Open);
			}
			return result;
		}

		bool isConnected() const {
			return sock_.is_open() && !needReconnect_;
		}
		void execute() override {
			open();
			while (!terminated_) {
				try {
					if (isConnected())
						handle(Event::Execute);
					else {
						sleep(1000);
						log("TryReConnect", LogType::Warning);
						open();
					}
				}
				catch (std::exception &e) {
					needReconnect_ = true;
					log("[execute] " + std::string(e.what()), LogType::Error);
				}
			};
		}
	protected:
		void stop() override {
			terminated_ = true;
			TCPClient::stop();
		}
		bool terminated_ = false;
		bool needReconnect_ = false;
	};
}