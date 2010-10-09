
#include <gtest/gtest.h>
#include "buffer.h"

class TestBuffer : public ::testing::Test
{
protected:

};

TEST_F(TestBuffer, testCommonTypes)
{
	char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
//	unsigned char d1 = 'e', d2 = 'f', d3 = 'g', d4 = 'h';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	EXPECT_EQ(4, buf.writePos()); // should take 4 bytes
	unsigned long l;
	buf >> l;
	EXPECT_EQ(0x61626364, l);
	EXPECT_EQ(4, buf.readPos());
	buf << "Test";
	buf >> c1 >> c2 >> c3 >> c4;
	EXPECT_TRUE(c1 == 'T' && c2 == 'e' && c3 == 's' && c4 == 't');
}

TEST_F(TestBuffer, testStdintTypes)
{
	uint8_t c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	EXPECT_EQ(4, buf.writePos()); // should take 4 bytes
	uint32_t l;
	buf >> l;
	EXPECT_EQ(0x61626364, l);
	EXPECT_EQ(4, buf.readPos());
}

TEST_F(TestBuffer, testScan)
{
	Buffer buf(128);
	buf << "AlphaBetaGammaDeltaEpsilonZeta";
	QByteArray arr;
	buf.scan("Gamma", arr);
	EXPECT_TRUE(QByteArray("AlphaBetaG") == arr);
	buf.scan("Epsilon", arr);
	EXPECT_TRUE( QByteArray("DeltaE") == arr);
	buf.scan("Notfound", arr);
	EXPECT_TRUE(QByteArray("DeltaE") == arr);
}

