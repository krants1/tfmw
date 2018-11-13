#include <gtest/gtest.h>
#include "../t/packet.h"

struct TestMessage {
	int vI;
	float vF;
	char vCA[50];
	int64_t vI64;
};

const int pNumber = 123;

struct TestPacket : T::StructPacket<pNumber, TestMessage> {};

TEST(StructPacket, GetNumber) {
	TestPacket t;
	ASSERT_EQ(t.getNumber(), pNumber);
}

TEST(StructPacket, ParseSerialize) {
	TestMessage tm;
	tm.vI = 456456;
	tm.vF = (float) 546.423;
	tm.vI64 = 54546564546;
	strcpy_s(tm.vCA, sizeof(tm.vCA), "Hellow World\n\t\r!!!");

	T::Packet * p;
	p = new TestPacket();
	TestPacket * tp1 = dynamic_cast<TestPacket*>(p);
	tp1->message = tm;

	boost::asio::streambuf buf;
	std::ostream outStream{&buf};
	std::istream inStream{&buf};

	p->serialize(outStream);
	ASSERT_EQ(buf.size(), sizeof(TestMessage));

	p = new TestPacket();
	TestPacket * tp2 = dynamic_cast<TestPacket*>(p);
	p->parse(inStream);

	ASSERT_EQ(buf.size(), 0);
	ASSERT_EQ(tp1->message.vI, tp2->message.vI);
	ASSERT_EQ(tp1->message.vF, tp2->message.vF);
	ASSERT_STREQ(tp1->message.vCA, tp2->message.vCA);
	ASSERT_EQ(tp1->message.vI64, tp2->message.vI64);

	delete tp1, tp2;
}

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
#ifdef _MSC_VER
	system("pause");
#endif
	return 0;
}