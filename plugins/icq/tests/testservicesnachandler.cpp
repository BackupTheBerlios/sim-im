#include <gtest/gtest.h>
#include "mocks/mockoscarsocket.h"
#include "icqclient.h"
#include "servicesnachandler.h"
#include "rateinfo.h"

namespace
{
    using ::testing::_;
    using ::testing::AnyNumber;
    using ::testing::NiceMock;
    class TestServiceSnacHandler : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<ServiceSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_SERVICE));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeServerServicesPacket()
        {
            ByteArrayBuilder builder;
            // FIXME:
            builder.appendWord(0x0001);
            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceServerReady, 0, builder.getArray());
        }

        QByteArray makeServerCapabilitiesAckPacket()
        {
            ByteArrayBuilder builder;
            builder.appendDword(0x00010004);

            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceCapabilitiesAck, 0, builder.getArray());
        }

        QByteArray makeRateInfoReply()
        {
            ByteArrayBuilder builder;
            int rateGroups = 5;
            builder.appendWord(rateGroups);
            for(int group = 0; group < rateGroups; group++)
            {
                builder.appendWord(group + 1);
                // More or less arbitrary values:
                builder.appendDword(0x50);
                builder.appendDword(0x9c4);
                builder.appendDword(0x7d0);
                builder.appendDword(0x5dc);
                builder.appendDword(0x320);
                builder.appendDword(0x1600);
                builder.appendDword(0x1700);
                builder.appendDword(0);
                builder.appendByte(0);
            }

            for(int group = 0; group < rateGroups; group++)
            {
                builder.appendWord(group + 1);
                builder.appendWord(2); // Two entries below
                builder.appendDword(0x00010002);
                builder.appendDword(0x00010004);
            }
            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfo, 0, builder.getArray());
        }

        QByteArray makeAcknowledgedRateInfoClassesPacket()
        {
            return QByteArray("\x00\x01\x00\x02\x00\x03\x00\x04\x00\x05", 10);
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        ServiceSnacHandler* handler;
    };

    TEST_F(TestServiceSnacHandler, negotiation_serverReady)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceCapabilities, _, _));
        QByteArray serverServices = makeServerServicesPacket();
        socket->provokePacketSignal(0x02, serverServices);
    }

    TEST_F(TestServiceSnacHandler, negotiation_capabilitiesAck)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfoRequest, _, _));
        QByteArray capabilitiesAck = makeServerCapabilitiesAckPacket();
        socket->provokePacketSignal(0x02, capabilitiesAck);
    }

    TEST_F(TestServiceSnacHandler, negotiation_rateInfoReply)
    {
        QByteArray acknowledgedRateInfoClasses = makeAcknowledgedRateInfoClassesPacket();
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber()); // Lest the gmock fail the test
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfoAck, _, acknowledgedRateInfoClasses));
        QByteArray rateInfoReply = makeRateInfoReply();
        socket->provokePacketSignal(0x02, rateInfoReply);
    }

    TEST_F(TestServiceSnacHandler, negotiation_rateInfoReply_parsesRateInfoCorrectly)
    {
        QByteArray rateInfoReply = makeRateInfoReply();
        socket->provokePacketSignal(0x02, rateInfoReply);

        RateInfoPtr info = handler->rateInfo(1);
        ASSERT_TRUE(info);
        ASSERT_TRUE(info->hasSnac(0x0001, 0x0002));
    }
}
