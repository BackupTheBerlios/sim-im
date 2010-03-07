
#include "testbuffer.h"

#include "buffer.h"

void TestBuffer::testCommonTypes()
{
	char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
//	unsigned char d1 = 'e', d2 = 'f', d3 = 'g', d4 = 'h';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	QVERIFY2(buf.writePos() == 4, "Unexpected 'char' size"); // should take 4 bytes
	unsigned long l;
	buf >> l; /* FIXED
        .\tests\testbuffer.cpp(14) : error C2248: 'QByteArray::operator QNoImplicitBoolCast' : cannot access private member declared in class 'QByteArray'
         \qt4\include\qtcore\../../src/corelib/tools/qbytearray.h(349) : see declaration of 'QByteArray::operator QNoImplicitBoolCast'
         \qt4\include\qtcore\../../src/corelib/tools/qbytearray.h(124) : see declaration of 'QByteArray'
        .\tests\testbuffer.cpp(14) : warning C4552: '>>' : operator has no effect; expected operator with side-effect

        because of uint32_t in win32, buffer.h line 82
        */

	QVERIFY2(l == 0x61626364, "Unexpected endianness");
	QVERIFY2(buf.readPos() == 4, "Unexpected 'unsigned long' size");
	buf << "Test";
	buf >> c1 >> c2 >> c3 >> c4;
	QVERIFY2(c1 == 'T' && c2 == 'e' && c3 == 's' && c4 == 't', "'char' unpacking failed");
}

void TestBuffer::testStdintTypes()
{
#ifndef WIN32
	uint8_t c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	QVERIFY2(buf.writePos() == 4, "Unexpected 'uint8_t' size"); // should take 4 bytes
	uint32_t l;
	buf >> l;
	QVERIFY2(l == 0x61626364, "Unexpected endianness");
	QVERIFY2(buf.readPos() == 4, "Unexpected 'uint32_t' size");
#endif
}

void TestBuffer::testScan()
{
	Buffer buf(128);
	buf << "AlphaBetaGammaDeltaEpsilonZeta";
	QByteArray arr;
	buf.scan("Gamma", arr);
	QCOMPARE(arr, QByteArray("AlphaBetaG"));
	buf.scan("Epsilon", arr);
	QCOMPARE(arr, QByteArray("DeltaE"));
	buf.scan("Notfound", arr);
	QCOMPARE(arr, QByteArray("DeltaE"));
}

