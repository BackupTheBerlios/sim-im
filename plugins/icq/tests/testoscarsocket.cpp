
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "testoscarsocket.h"
#include "standardoscarsocket.h"

#include "tests/mocks/mockasyncsocket.h"

#include "qt-gtest.h"

namespace Helper
{
    SignalSpy::SignalSpy() : connectedCalls(0)
    {

    }

    void SignalSpy::provokeSignal()
    {
        emit justSignal();
    }

    void SignalSpy::packet(int channel, const QByteArray& arr)
    {
        receivedPacket = arr;
        receivedChannel = channel;
    }

    void SignalSpy::error(const QString& str)
    {
        errorString = str;
    }

    void SignalSpy::connected()
    {
        connectedCalls++;
    }
}

namespace
{
    using namespace Helper;

    class TestOscarSocket : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new StandardOscarSocket();
            asyncSocket = new MockObjects::MockAsyncSocket();
            socket->setSocket(asyncSocket);
        }

        virtual void TearDown()
        {
            delete socket;
        }

        QByteArray makeEmptyFlapPacket(int channel)
        {
            QByteArray packet = QByteArray("\x2a\x00\x00\x00\x00\x00", 6);
            packet[1] = channel;
            return packet;
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

        StandardOscarSocket* socket;
        MockObjects::MockAsyncSocket* asyncSocket;
    };


    TEST_F(TestOscarSocket, connectToHost)
    {
        EXPECT_CALL(*asyncSocket, connectToHost(QString("testhost"), 1234));

        socket->connectToHost("testhost", 1234);
    }

    TEST_F(TestOscarSocket, signal_Connected)
    {
        SignalSpy spy;
        socket->connect(socket, SIGNAL(connected()), &spy, SLOT(connected()));

        asyncSocket->provokeConnectedSignal();

        ASSERT_EQ(1, spy.connectedCalls);
    }

    TEST_F(TestOscarSocket, flap)
    {
        socket->flap(0x02, QByteArray(2, 0x55));

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x00\x00\x02\x55\x55", 8);
        expected[2] = (StartingFlapSqNum >> 8) & 0xff;
        expected[3] = (StartingFlapSqNum >> 0) & 0xff;
        ASSERT_EQ(expected, arr);
    }

    TEST_F(TestOscarSocket, flap_sequenceNumberIncrement)
    {
        socket->flap(0x02, QByteArray());
        asyncSocket->resetWriteBuffer();

        socket->flap(0x02, QByteArray());

        QByteArray arr = asyncSocket->getWriteBuffer();
        QByteArray expected = QByteArray("\x2a\x02\x00\x01\x00\x00", 6);
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
        spy.connect(socket, SIGNAL(packet(int, QByteArray)), &spy, SLOT(packet(int, QByteArray)));
        QByteArray packet = makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("datadatadata"));
        asyncSocket->setReadBuffer(packet);

        spy.provokeSignal();

        ASSERT_EQ(packet, spy.receivedPacket);
    }

    TEST_F(TestOscarSocket, readyRead_twoValidPackets)
    {
        SignalSpy spy;
        spy.connect(&spy, SIGNAL(justSignal()), socket, SLOT(readyRead()));
        spy.connect(socket, SIGNAL(packet(int, QByteArray)), &spy, SLOT(packet(int, QByteArray)));
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
        spy.connect(socket, SIGNAL(packet(int, QByteArray)), &spy, SLOT(packet(int, QByteArray)));
        spy.connect(socket, SIGNAL(error(QString)), &spy, SLOT(error(QString)));
        QByteArray packet = makeValidPacket(0x11, 0x22, 0x12345678, QByteArray("datadatadata"));
        packet[0] = 12;
        asyncSocket->setReadBuffer(packet);
        ASSERT_TRUE(spy.errorString.isNull());

        spy.provokeSignal();

        ASSERT_TRUE(!spy.errorString.isNull());
    }

    TEST_F(TestOscarSocket, readyRead_channel)
    {
        SignalSpy spy;
        spy.connect(&spy, SIGNAL(justSignal()), socket, SLOT(readyRead()));
        spy.connect(socket, SIGNAL(packet(int, QByteArray)), &spy, SLOT(packet(int, QByteArray)));
        QByteArray packet = makeEmptyFlapPacket(0x02);
        asyncSocket->setReadBuffer(packet);

        spy.provokeSignal();

        ASSERT_EQ(0x02, spy.receivedChannel);
    }
}
