
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "qt-gtest.h"
#include "icqclient.h"
#include "oscarsocket.h"
#include "buddysnachandler.h"
#include "mocks/mockoscarsocket.h"
#include "icqstatus.h"

namespace
{
    using ::testing::NiceMock;

    static const QByteArray ContactTextUin = "123456789";
    static const int ContactUin = 123456789;
    static const int ContactId = 42;

    class TestBuddySnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            ICQContactList* list = client->contactList();
            ICQContactPtr contact = client->createIMContact().dynamicCast<ICQContact>();
            contact->setIcqID(ContactId);
            contact->setUin(ContactUin);
            list->addContact(contact);

            handler = static_cast<BuddySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BUDDY));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeOnlineBuddyPacket()
        {
            ByteArrayBuilder builder;

            builder.appendByte(ContactTextUin.length());
            builder.appendBytes(ContactTextUin);
            builder.appendWord(0); // Warning level

            ICQStatusPtr status = client->getDefaultStatus("online");
            TlvList list;
            list.append(Tlv::fromUint16(BuddySnacHandler::TlvUserClass, CLASS_FREE | CLASS_ICQ));
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvOnlineSince, 0)); // Yeah, online since 1 Jan 1970
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvOnlineStatus, status->icqId()));
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvUserIp, 0));

            builder.appendWord(list.tlvCount());
            builder.appendBytes(list.toByteArray());

            return builder.getArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        BuddySnacHandler* handler;
    };

    TEST_F(TestBuddySnacHandler, onlineNotificationProcessing)
    {
        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactPtr contact = client->contactList()->contact(ContactId);
        ASSERT_EQ("online", contact->status()->id());
    }
}
