
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "oscarsocket.h"

#include "tests/mocks/mockasyncsocket.h"

namespace
{
#include "tests/gtest-qt.h"

    class TestOscarSocket : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new OscarSocket();
            asyncSocket = new MockObjects::MockAsyncSocket();
            socket->setSocket(asyncSocket);
        }

        virtual void TearDown()
        {
            delete socket;
        }

        static const int SizeOfSnacHeader = 10;

        OscarSocket* socket;
        MockObjects::MockAsyncSocket* asyncSocket;
    };

    TEST_F(TestOscarSocket, connectToHost)
    {
        EXPECT_CALL(*asyncSocket, connectToHost(QString("testhost"), 1234));

        socket->connectToHost("testhost", 1234);
    }

    TEST_F(TestOscarSocket, flap)
    {
        socket->flap(0x02, 12);

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x00\x00\x0c", 6);
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, snac_emptySnacData)
    {
        socket->snac(0x11, 0x22, 0x12345678, QByteArray());

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x00\x00\x0a\x00\x11\x00\x22\x00\x00\x12\x34\x56\x78", 16);
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, snac_withSnacData)
    {
        QByteArray snacData = QByteArray("datadatadata");
        socket->snac(0x11, 0x22, 0x12345678, snacData);

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x00\x00\x0a\x00\x11\x00\x22\x00\x00\x12\x34\x56\x78", 16);
        expected.append(snacData);
        expected[4] = ((SizeOfSnacHeader + snacData.size()) >> 8) & 0xff;
        expected[5] = (SizeOfSnacHeader + snacData.size()) & 0xff;
        ASSERT_EQ(expected, arr);
    }
}
