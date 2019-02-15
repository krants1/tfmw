#include <api.pb.h>
#include <t/packet.h>
#include <t/time_utils.h>

template<class M>
struct ProtoPacket : public T::Packet {
public:
	M message;
protected:
	void parse(std::istream& ins) override {
		message.ParseFromIstream(&ins);
	}
	void serialize(std::ostream& outs) override {
		message.SerializeToOstream(&outs);
	}
};

int example_t_proto_tcp() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	class PongPacket : public ProtoPacket<protos::Pong> {
		void handle(reply) override {
			T::slog("handle Pong");
		}
	};
	class PingPacket : public ProtoPacket<protos::Ping> {
		void handle(reply reply) override {
			T::slog("handle Ping");
			PongPacket p;
			reply(p);
		}
	};
	class WelcomePacket : public ProtoPacket<protos::Welcome> {
		void handle(reply) override {
			T::slog("Successful login(" + message.application() + ")");
		}
	};
	class LoginPacket : public ProtoPacket<protos::Login> {
		void handle(reply reply) override {
			if (message.name() == "Admin" && message.secret() == "secret") {
				T::slog("Authorized: " + message.name() + "(" + message.application() + ")");
				WelcomePacket wp;
				wp.message.set_application("TCPServer.exe");
				reply(wp);
			} else {
				T::slog("Unknow user: " + message.name());
				abort();
			}
		}
	};

	class MyProtoTCPService : public T::TCPPacketsService {
	public:
		explicit MyProtoTCPService(unsigned short port) : TCPPacketsService(port) {
			LogHelper::init(__func__);
			getController().add(1, new LoginPacket());
			getController().add(2, new WelcomePacket());
			getController().add(5, new PingPacket());
			getController().add(6, new PongPacket());
		}

		~MyProtoTCPService() override { stop(); };
	};

	class MyTCPClientThread : public T::TCPPacketsClient {
	public:
		MyTCPClientThread(std::string host, unsigned short port) : TCPPacketsClient(std::move(host), port) {
			LogHelper::init(__func__);
			getController().add(1, new LoginPacket());
			getController().add(2, new WelcomePacket());
			getController().add(5, new PingPacket());
			getController().add(6, new PongPacket());
		}

		void execute() override {
			try {
				open();
//Login
				LoginPacket lp;
				lp.message.set_name("Admin");
				lp.message.set_secret("secret");
				lp.message.set_application("TCPClient.exe");
				sendPacket(sock_, lp);

				managePackets(sock_);
//Execute
				T::TimeOperation toPing(1000);
				T::TimeOperation toDisconnect(3100);
				while (!toDisconnect.canDoNow()) {
					if (sock_.available())
						managePackets(sock_);
					sleep(1);
					if (toPing.canDoNow()) {
						PingPacket p;
						sendPacket(sock_, p);
						toPing.endProcess();
					}
				}
				sock_.close();
			}
			catch (std::exception& e) {
				log("[execute] " + std::string(e.what()), T::LogType::Error);
			}
		}

		~MyTCPClientThread() override { stop(); };
	};

	MyProtoTCPService ts(5556);
	ts.run();

	MyTCPClientThread tc("localhost", 5556);
	tc.run();

	return 0;
}