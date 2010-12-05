
#include <gtest/gtest.h>
#include "mocks/mockoscarsocket.h"
#include "icqclient.h"

namespace
{
    using ::testing::_;
    using ::testing::AnyNumber;
    class TestLoginSequenceStep2 : public ::testing::Test
    {
    protected:

        virtual void SetUp()
        {
            socket = new MockObjects::MockOscarSocket();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
        }

        virtual void TearDown()
        {
            delete client;
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

        ICQClient* client;
        MockObjects::MockOscarSocket* socket;
    };

    TEST_F(TestLoginSequenceStep2, rateInfoReply_requestsSelfInfo)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceSelfInfoRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_ssiRequestsRights)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SSI, SsiSnacHandler::SnacSsiRightsRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_locationRequestsRights)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_LOCATION, LocationSnacHandler::SnacLocationRightsRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_buddyRequestsRights)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_BUDDY, BuddySnacHandler::SnacBuddyRightsRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_privacyRequestsRights)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_BOS, PrivacySnacHandler::SnacPrivacyRightsRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_icbmRequestsParametersInfo)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmParametersInfoRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }

    TEST_F(TestLoginSequenceStep2, rateInfoReply_ssiRequestList)
    {
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SSI, SsiSnacHandler::SnacSsiContactListRequest, _, _));

        socket->provokePacketSignal(0x02, makeRateInfoReply());
    }
}
