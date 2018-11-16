#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include "tcp.h"

namespace T {
	struct Packet {
		typedef std::function<void(Packet& packet)> reply;
		virtual unsigned int getNumber() { return 0; }
		virtual void parse(std::istream&) {};
		virtual void serialize(std::ostream&) {};
		virtual void hundle(reply) {}
		virtual ~Packet() = default;
		bool closeSocket = false;
	};

	class Controller {
	public:
		void add(Packet* b) {
			add(b->getNumber(), b);
		}
		void add(unsigned int number, Packet* p) {
			UPacket up(p);
			list_.push_back(std::move(up));
			mapPackets_[number] = p;
			mapNubmers_[std::type_index(typeid(*p))] = number;
			if (mapPackets_.size() != mapNubmers_.size())
				throw std::runtime_error("different maps sizes");
		};
		Packet* getPacket(unsigned int& number) {
			return mapPackets_[number];
		}
		unsigned int getNumber(Packet* p) {
			return mapNubmers_[std::type_index(typeid(*p))];
		}
	private:
		typedef std::unique_ptr<Packet> UPacket;
		std::vector<UPacket> list_;
		std::unordered_map<unsigned int, Packet*> mapPackets_;
		std::unordered_map<std::type_index, unsigned int> mapNubmers_;
	};

	class PacketsSocketHelper : public SocketHelper {
	public:
		void sendPacket(socket& s, Packet& packet) {
			unsigned int number = controller_.getNumber(&packet);
			packet.serialize(outStream);
			sendPacket(s, number);
		};
		void managePackets(socket& s) {
			boost::unique_lock<boost::mutex> l(bufMutex_);
			unsigned int number;
			readPacket(s, number);

			Packet* p = controller_.getPacket(number);

			if (p == nullptr)
				throw std::runtime_error("Unknow packet");

			Packet clone((*p));
			*p = clone;

			p->parse(inStream);
			PacketsSocketHelper* ths = this;

			Packet::reply funcReply = [&ths, &s](Packet& packet) {
				ths->sendPacket(s, packet);
			};

			p->hundle(funcReply);
			if (p->closeSocket)
				s.close();
		}
	protected:
		Controller& getController() {
			return controller_;
		}
		void sendPacket(socket& s, unsigned int number) {
			sendInt(s, number);
			sendInt(s, (int) buf_.size());
			boost::asio::write(s, buf_);
		}
		void readPacket(socket& s, unsigned int& number) {
			number = (unsigned int) readInt(s);
			int size = readInt(s);
			boost::asio::read(s, buf_, boost::asio::transfer_exactly((size_t) size));
		};
	private:
		Controller controller_;
		// ToDo: crutch, take out of class
		boost::mutex bufMutex_;
		boost::asio::streambuf buf_;
		std::ostream outStream{&buf_};
		std::istream inStream{&buf_};
	};

	class TCPPacketsService : public TCPService, public PacketsSocketHelper {
	public:
		explicit TCPPacketsService(unsigned short port) : TCPService(port) {
			LogHelper::init(__func__);
		}
		void handle(T::SocketHelper::socket& sock) override {
			if (sock.available()) {
				managePackets(sock);
			} else
				sleep(5);
		}
	};

	class TCPPacketsClient : public TCPClient, public PacketsSocketHelper {
	public:
		TCPPacketsClient(std::string host, unsigned short port) : TCPClient(std::move(host), port) {
			LogHelper::init(__func__);
		}
	};

	template<unsigned int N, class M>
	struct StructPacket : public T::Packet {
	public:
		M message{};
		unsigned int getNumber() override {
			return number;
		}
	private:
		unsigned int number{N};
		char buff[sizeof(M)]{};
		void parse(std::istream& ins) override {
			ins.read(buff, sizeof(message));
			std::memcpy(&message, buff, sizeof(message));
		};
		void serialize(std::ostream& out) override {
			std::memcpy(buff, &message, sizeof(message));
			out.write(buff, sizeof(buff));
		};
	};

}
