
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "oscarsocket.h"
#include "buddysnachandler.h"
#include "mocks/mockoscarsocket.h"
#include "icqstatus.h"
#include "events/eventhub.h"
#include "contacts/contactlist.h"
#include "contacts/contact.h"
#include "log.h"

namespace
{
    using ::testing::NiceMock;

    static const QByteArray ContactTextUin("123456789", 9);
    static const int ContactUin = 123456789;
    static const int ContactId = 42;
    static const int MetaContactId = 21;

    class TestBuddySnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            metaContact = SIM::getContactList()->createContact(MetaContactId);
            SIM::getContactList()->addContact(metaContact);

            ICQContactList* list = client->contactList();
            ICQContactPtr contact = client->createIMContact().dynamicCast<ICQContact>();
            contact->setIcqID(ContactId);
            contact->setUin(ContactUin);
            list->addContact(contact);

            metaContact->addClientContact(contact);
            contact->setMetaContact(metaContact.data());

            handler = static_cast<BuddySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BUDDY));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            SIM::getContactList()->removeContact(MetaContactId);
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

        SIM::ContactPtr metaContact;
        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        BuddySnacHandler* handler;
    };

    TEST_F(TestBuddySnacHandler, onlineNotificationProcessing)
    {
        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactPtr contact = client->contactList()->contact(ContactId);
        ASSERT_TRUE(contact->status());
        ASSERT_EQ("online", contact->status()->id());
    }

    TEST_F(TestBuddySnacHandler, onContactOnline_emitsEvent)
    {
        Helper::SignalSpy spy;
        SIM::getEventHub()->getEvent("contact_change_status")->connectTo(&spy, SLOT(intSlot(int)));
        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.intSlotCalls);
        ASSERT_EQ(MetaContactId, spy.intarg);
    }
}
