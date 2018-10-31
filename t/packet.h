#ifndef __T_PACKET_H__
#define __T_PACKET_H__

#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>

#include "../t/tcp.h"

namespace T {
	struct Packet {
		typedef std::function<void(Packet &packet)> reply;
		virtual unsigned int getNumber() { return 0; }
		virtual void parse(std::istream &) {};
		virtual void serialize(std::ostream &) {};
		virtual void hundle(reply) {}
		virtual ~Packet() = default;
		bool closeSocket = false;
	};

	class Controller {
	private:
		std::vector<Packet *> list;
		std::unordered_map<unsigned int, Packet *> mapPackets;
		std::unordered_map<std::type_index, unsigned int> mapNubmers;
	public:
		void reg(Packet *b) {
			reg(b->getNumber(), b);
		}
		void reg(unsigned int number, Packet *b) {
			list.push_back(b);
			mapPackets[number] = b;
			mapNubmers[std::type_index(typeid(*b))] = number;
			if (mapPackets.size() != mapNubmers.size())
				throw std::runtime_error("different maps sizes");
		};
		Packet *getPacket(unsigned int &number) {
			return mapPackets[number];
		}
		unsigned int getNumber(Packet *b) {
			return mapNubmers[std::type_index(typeid(*b))];
		}
		void test() {
			for (auto i = 0; (size_t) i < list.size(); i++) {
				Packet *b = list[i];
				std::vector<Packet *> *l = &list;
				Packet::reply funcReply = [&l, &i](Packet packet) {
					std::cout << "answer" << std::endl;
					Packet *b1 = (*l)[i];
					//Packet b2(*b1);
					typeid(*b1).name();
					typeid(packet).name();
				};
				b->hundle(funcReply);
			}
		}
		~Controller() {
			for (auto &i : list)
				delete i;
			list.clear();
		}
	};

	class PacketsSocketHelper : public SocketHelper {
	protected:
		Controller controller;
		// ToDo: crutch, take out of class
		boost::mutex bufMutex;
		boost::asio::streambuf buf;
		std::ostream outStream{&buf};
		std::istream inStream{&buf};
	protected:
		void sendPacket(socket &s, unsigned int number) {
			sendInt(s, number);
			sendInt(s, (int) buf.size());
			boost::asio::write(s, buf);
		}
		void readPacket(socket &s, unsigned int &number) {
			number = (unsigned int) readInt(s);
			int size = readInt(s);
			boost::asio::read(s, buf, boost::asio::transfer_exactly((size_t) size));
		};
	public:
		void sendPacket(socket &s, Packet &packet) {
			unsigned int number = controller.getNumber(&packet);
			packet.serialize(outStream);
			sendPacket(s, number);
		};

		void managePackets(socket &s) {
			boost::unique_lock<boost::mutex> l(bufMutex);
			unsigned int number;
			readPacket(s, number);

			Packet *p = controller.getPacket(number);

			if (p == nullptr)
				throw std::runtime_error("Unknow packet");

			Packet clone((*p));
			*p = clone;

			p->parse(inStream);
			PacketsSocketHelper *ths = this;

			Packet::reply funcReply = [&ths, &s](Packet &packet) {
				ths->sendPacket(s, packet);
			};

			p->hundle(funcReply);
			if (p->closeSocket)
				s.close();
		}
	};

	class TCPServiceP : public TCPService, public PacketsSocketHelper {
	public:
		explicit TCPServiceP(unsigned short port) : TCPService(port) {
			LogHelper::init(__FUNCTION__);
		}
		void handle(T::SocketHelper::socket &sock) override {
			if (sock.available()) {
				managePackets(sock);
			} else
				sleep(5);
		}
	};

	class TCPClientP : public TCPClient, public PacketsSocketHelper {
	public:
		TCPClientP(std::string host, short port) : TCPClient(std::move(host), port) {
			LogHelper::init(__FUNCTION__);
		}
	};

	template<unsigned int N, class M>
	struct StructPacket : public T::Packet {
	public:
		M message{};
		unsigned int getNumber() override {
			return number;
		}
	protected:
		unsigned int number{N};
		char buff[sizeof(M)]{};
		void parse(std::istream &ins) override {
			ins.read(buff, sizeof(message));
			std::memcpy(&message, buff, sizeof(message));
		};
		void serialize(std::ostream &out) override {
			std::memcpy(buff, &message, sizeof(message));
			out.write(buff, sizeof(buff));
		};
	};

}

#endif //__T_PACKET_H__
