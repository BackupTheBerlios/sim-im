#include <gtest/gtest.h>

#include "tlvlist.h"
#include "icqclient.h"
#include "ssisnachandler.h"
#include "mocks/mockoscarsocket.h"

namespace
{
    class TestSsiSnacHandler : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<ServiceSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_SSI));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        static const int MaxContacts = 0x1000;
        static const int MaxGroups = 0x1001;
        static const int MaxVisibleContacts = 0x1002;
        static const int MaxInvisibleContacts = 0x1003;
        static const int MaxBitmasks = 0x1004;
        static const int MaxPresenceInfo = 0x1005;
        static const int MaxIgnore = 0x1006;

        QByteArray makeRightsInfoPacket()
        {
            ByteArrayBuilder builder;
            builder.appendWord(MaxContacts);
            builder.appendWord(MaxGroups);
            builder.appendWord(MaxVisibleContacts);
            builder.appendWord(MaxInvisibleContacts);
            builder.appendWord(MaxBitmasks);
            builder.appendWord(MaxPresenceInfo);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(MaxIgnore);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            builder.appendWord(0);
            TlvList list;
            list.append(Tlv(SsiSnacHandler::TlvIdMaximumItems, builder.getArray()));
            return list.toByteArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        SsiSnacHandler* handler;
    };

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxContacts)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket, 0, 0);

        EXPECT_EQ(MaxContacts, handler->maxContacts());
    }
}
