
#include "gtest/gtest.h"

#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "locationsnachandler.h"
#include "oscarsocket.h"
#include "mocks/mockoscarsocket.h"

namespace
{
    using ::testing::NiceMock;
    using ::testing::_;

    static const int MaxProfileLength = 0x1000;
    static const int MaxCapabilities = 0x0012;

    class TestLocationSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<LocationSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_LOCATION));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeRightsInfoPacket()
        {
            TlvList list;
            list.append(Tlv::fromUint16(LocationSnacHandler::TlvMaxProfileLength, MaxProfileLength));
            list.append(Tlv::fromUint16(LocationSnacHandler::TlvMaxCapabilities, MaxCapabilities));
            return list.toByteArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        LocationSnacHandler* handler;
    };

    TEST_F(TestLocationSnacHandler, rightsPacket_processing)
    {
        bool success = handler->process(LocationSnacHandler::SnacLocationRightsInfo, makeRightsInfoPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(MaxCapabilities, handler->maxCapabilities());
    }

    TEST_F(TestLocationSnacHandler, rightsPacket_sendsSetUserInfoPacket)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_LOCATION, LocationSnacHandler::SnacSetUserInfo, _, _)).Times(1);
        bool success = handler->process(LocationSnacHandler::SnacLocationRightsInfo, makeRightsInfoPacket(), 0, 0);
        ASSERT_TRUE(success);
    }

}
