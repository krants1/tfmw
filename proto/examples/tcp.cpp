#include "../scr/api.pb.h"
#include "../scr/api.pb.cc"
#include "../../t/packet.h"
#include "../../t/time_utils.h"

namespace T {
    template<class T>
    struct ProtoPacket : public Packet {
    public:
        T message;
    protected:
        void parse(std::istream &ins) override {
            message.ParseFromIstream(&ins);
        }
        void serialize(std::ostream &outs) override {
            message.SerializeToOstream(&outs);
        }
    };
}

int example_t_proto_tcp() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    class PongPacket : public T::ProtoPacket<protos::Pong> {
        void hundle(reply) override {
            T::slog("hundle Pong");
        }
    };
    class PingPacket : public T::ProtoPacket<protos::Ping> {
        void hundle(reply reply) override {
            T::slog("hundle Ping");
            PongPacket p;
            reply(p);
        }
    };
    class WelcomePacket : public T::ProtoPacket<protos::Welcome> {
        void hundle(reply) override {
            T::slog("successful login(" + message.application() + ")");
        }
    };
    class LoginPacket : public T::ProtoPacket<protos::Login> {
        void hundle(reply reply) override {
            if (message.name() == "Admin" && message.secret() == "secret") {
                T::slog("authorized: " + message.name() + "(" + message.application() + ")");
                WelcomePacket wp;
                wp.message.set_application("TCPServer.exe");
                reply(wp);
            } else {
                T::slog("Unknow user: " + message.name());
                abort();
            }
        }
    };

    class MyProtoTCPService : public T::TCPServiceP {
    public:
        explicit MyProtoTCPService(unsigned short port) : TCPServiceP(port) {
            LogHelper::init(__FUNCTION__);
            packets.reg(1, new LoginPacket());
            packets.reg(2, new WelcomePacket());
            packets.reg(5, new PingPacket());
            packets.reg(6, new PongPacket());
        }

        ~MyProtoTCPService() override { stop(); };
    };

    class MyTCPClientThread : public T::TCPClientP {
    public:
        MyTCPClientThread(std::string host, short port) : TCPClientP(std::move(host), port) {
            LogHelper::init(__FUNCTION__);
            packets.reg(1, new LoginPacket());
            packets.reg(2, new WelcomePacket());
            packets.reg(5, new PingPacket());
            packets.reg(6, new PongPacket());
        }

        void execute() override {
            try {
                open();
//Login
                LoginPacket lp;
                lp.message.set_name("Admin");
                lp.message.set_secret("secret");
                lp.message.set_application("TCPClient.exe");
                sendPacket(sock, lp);

                managePackets(sock);
//Execute
                T::TimeOperation toPing(1000);
                T::TimeOperation toDisconnect(3100);
                while (!toDisconnect.canDoNow()) {
                    if (sock.available())
                        managePackets(sock);
                    sleep(1);
                    if (toPing.canDoNow()) {
                        PingPacket p;
                        sendPacket(sock, p);
                        toPing.endProcess();
                    }
                }
                sock.close();
            }
            catch (std::exception &e) {
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