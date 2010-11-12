
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "testoscarsocket.h"
#include "oscarsocket.h"

#include "tests/mocks/mockasyncsocket.h"

namespace Helper
{
    void SignalSpy::provokeSignal()
    {
        emit justSignal();
    }

    void SignalSpy::packet(const QByteArray& arr)
    {
        receivedPacket = arr;
    }

    void SignalSpy::error(const QString& str)
    {
        errorString = str;
    }
}

namespace
{
    using namespace Helper;
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

        QByteArray makeValidPacket(int type, int subtype, int requestId, const QByteArray& snacData)
        {
            QByteArray packet = QByteArray("\x2a\x02\x00\x00\x00\x00", 6);
            packet[2] = (StartingFlapSqNum >> 8) & 0xff;
            packet[3] = (StartingFlapSqNum >> 0) & 0xff;
            packet.append(type >> 8);
            packet.append(type & 0xff);
            packet.append(subtype >> 8);
            packet.append(subtype & 0xff);
            packet.append((char)0);
            packet.append((char)0);
            packet.append((requestId >> 24) & 0xff);
            packet.append((requestId >> 16) & 0xff);
            packet.append((requestId >> 8) & 0xff);
            packet.append((requestId >> 0) & 0xff);
            packet[4] = ((SizeOfSnacHeader + snacData.size()) >> 8) & 0xff;
            packet[5] = (SizeOfSnacHeader + snacData.size()) & 0xff;
            packet.append(snacData);
            return packet;
        }

        static const int SizeOfSnacHeader = 10;

        static const int StartingFlapSqNum = 8984;

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
        expected[2] = (StartingFlapSqNum >> 8) & 0xff;
        expected[3] = (StartingFlapSqNum >> 0) & 0xff;
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, flap_sequenceNumberIncrement)
    {
        socket->flap(0x02, 12);
        asyncSocket->resetWriteBuffer();

        socket->flap(0x02, 12);

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x01\x00\x0c", 6);
        expected[2] = ((StartingFlapSqNum + 1) >> 8) & 0xff;
        expected[3] = ((StartingFlapSqNum + 1) >> 0) & 0xff;
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, snac_emptySnacData)
    {
        socket->snac(0x11, 0x22, 0x12345678, QByteArray());

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x00\x00\x0a\x00\x11\x00\x22\x00\x00\x12\x34\x56\x78", 16);
        expected[2] = (StartingFlapSqNum >> 8) & 0xff;
        expected[3] = (StartingFlapSqNum >> 0) & 0xff;
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, snac_withSnacData)
    {
        QByteArray snacData = QByteArray("datadatadata");
        socket->snac(0x11, 0x22, 0x12345678, snacData);

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = makeValidPacket(0x11, 0x22, 0x12345678, snacData);
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, readyRead_validPacket)
    {
        SignalSpy spy;
        spy.connect(&spy, SIGNAL(justSignal()), socket, SLOT(readyRead()));
        spy.connect(socket, SIGNAL(packet(QByteArray)), &spy, SLOT(packet(QByteArray)));
        QByteArray packet = makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("datadatadata"));
        asyncSocket->setReadBuffer(packet);

        spy.provokeSignal();

        ASSERT_EQ(packet, spy.receivedPacket);
    }

    TEST_F(TestOscarSocket, readyRead_twoValidPackets)
    {
        SignalSpy spy;
        spy.connect(&spy, SIGNAL(justSignal()), socket, SLOT(readyRead()));
        spy.connect(socket, SIGNAL(packet(QByteArray)), &spy, SLOT(packet(QByteArray)));
        asyncSocket->setReadBuffer(makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("first packet")));
        spy.provokeSignal();
        QByteArray packet = makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("second packet"));
        asyncSocket->setReadBuffer(packet);

        spy.provokeSignal();

        ASSERT_EQ(packet, spy.receivedPacket);
    }

    TEST_F(TestOscarSocket, readyRead_invalidPacket)
    {
        SignalSpy spy;
        spy.connect(&spy, SIGNAL(justSignal()), socket, SLOT(readyRead()));
        spy.connect(socket, SIGNAL(packet(QByteArray)), &spy, SLOT(packet(QByteArray)));
        spy.connect(socket, SIGNAL(error(QString)), &spy, SLOT(error(QString)));
        QByteArray packet = makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("datadatadata"));
        packet[0] = 12;
        asyncSocket->setReadBuffer(packet);
        ASSERT_TRUE(spy.errorString.isNull());

        spy.provokeSignal();

        ASSERT_TRUE(!spy.errorString.isNull());
    }
}
