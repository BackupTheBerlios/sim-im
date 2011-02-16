#include <gtest/gtest.h>
#include <cstdio>
#include <QSignalSpy>

#include "events/eventhub.h"
#include "events/ievent.h"
#include "contacts/contactlist.h"
#include "tlvlist.h"
#include "icqclient.h"
#include "ssisnachandler.h"
#include "mocks/mockoscarsocket.h"
#include "signalspy.h"
#include "qt-gtest.h"

namespace
{
    using ::testing::NiceMock;
    using ::testing::_;

    static const int MaxContacts = 0x1000;
    static const int MaxGroups = 0x1001;
    static const int MaxVisibleContacts = 0x1002;
    static const int MaxInvisibleContacts = 0x1003;
    static const int MaxBitmasks = 0x1004;
    static const int MaxPresenceInfo = 0x1005;
    static const int MaxIgnore = 0x1006;

    static const QString BuddyName = "TestBuddy";
    static const QString BuddyUin = "123456789";
    static const int ContactId = 12;
    static const int GroupId = 42;
    static const QString GroupName = "TestGroup";


    class TestSsiSnacHandler : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            //SIM::createEventHub();
            //SIM::createContactList();
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<SsiSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_LISTS));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
            //SIM::destroyContactList();
            //SIM::destroyEventHub();
        }

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

        QByteArray makeContactListPacketWithContactEntry()
        {
            ByteArrayBuilder builder;
            builder.appendByte(0x00); // Version
            builder.appendWord(0x01); // Objects count

            builder.appendWord(BuddyUin.length());
            builder.appendBytes(BuddyUin.toAscii());
            builder.appendWord(GroupId); // Group Id
            builder.appendWord(ContactId); // Contact Id
            builder.appendWord(SsiSnacHandler::EntryContact); // Unknown
            builder.appendWord(0x00); // No Tlvs
            return builder.getArray();
        }

        QByteArray makeContactListPacketWithContactEntry_withNameTlv()
        {
            ByteArrayBuilder builder;
            builder.appendByte(0x00); // Version
            builder.appendWord(0x01); // Objects count

            builder.appendWord(BuddyUin.length());
            builder.appendBytes(BuddyUin.toAscii());
            builder.appendWord(GroupId); // Group Id
            builder.appendWord(ContactId); // Contact Id
            builder.appendWord(SsiSnacHandler::EntryContact);
            TlvList tlvs;
            tlvs.append(Tlv(SsiSnacHandler::TlvAlias, BuddyName.toUtf8()));
            QByteArray tlvsArray = tlvs.toByteArray();
            builder.appendWord(tlvsArray.size());
            builder.appendBytes(tlvsArray);

            return builder.getArray();
        }

        QByteArray makeContactListPacketWithGroupEntry()
        {
            ByteArrayBuilder builder;
            builder.appendByte(0x00); // Version
            builder.appendWord(0x01); // Objects count

            builder.appendWord(GroupName.length());
            builder.appendBytes(GroupName.toAscii());
            builder.appendWord(GroupId); // Group Id
            builder.appendWord(0x00); // Contact Id
            builder.appendWord(SsiSnacHandler::EntryGroup);

            TlvList tlvs;
            QByteArray data;
            data.append((char)((ContactId >> 8) & 0xff));
            data.append((char)((ContactId >> 0) & 0xff));
            tlvs.append(Tlv(SsiSnacHandler::TlvSubitems, data));
            QByteArray tlvdata = tlvs.toByteArray();
            builder.appendWord(tlvdata.size()); // No Tlvs
            builder.appendBytes(tlvdata);
            return builder.getArray();
        }


        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        SsiSnacHandler* handler;
    };

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxContacts)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxContacts, handler->maxContacts());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxGroups)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxGroups, handler->maxGroups());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxVisibleContacts)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxVisibleContacts, handler->maxVisibleContacts());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxInvisibleContacts)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxInvisibleContacts, handler->maxInvisibleContacts());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxBitmasks)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxBitmasks, handler->maxBitmasks());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxPresenceInfoFields)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxPresenceInfo, handler->maxPresenceInfoFields());
    }

    TEST_F(TestSsiSnacHandler, rightsInfoParsing_MaxIgnore)
    {
        handler->process(SsiSnacHandler::SnacSsiRightsInfo, makeRightsInfoPacket(), 0, 0);

        ASSERT_EQ(MaxIgnore, handler->maxIgnore());
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_contactEntry)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithContactEntry(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->contactCount());
        ASSERT_EQ(1, SIM::getContactList()->contactIds().count());
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_contactEntry_groupId)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithContactEntry(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->contactCount());
        ICQContactPtr contact = contactList->contact(ContactId);
        ASSERT_EQ(GroupId, contact->getGrpID());
    }


    TEST_F(TestSsiSnacHandler, ssiListParsing_contactEntry_withAliasTlv)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithContactEntry_withNameTlv(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->contactCount());
        ICQContactPtr contact = contactList->contact(ContactId);
        ASSERT_EQ(BuddyName, contact->getAlias());
    }


    TEST_F(TestSsiSnacHandler, ssiListParsing_groupEntry)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithGroupEntry(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->groupCount());
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_groupEntry_Alias)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithGroupEntry(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->groupCount());
        ICQGroupPtr group = contactList->group(GroupId);
        ASSERT_FALSE(group.isNull());
        ASSERT_EQ(GroupName, group->name());
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_groupEntry_GroupMembers)
    {
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithGroupEntry(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactList* contactList = client->contactList();
        ASSERT_EQ(1, contactList->groupCount());
        ICQGroupPtr group = contactList->group(GroupId);
        ASSERT_FALSE(group.isNull());
        ASSERT_TRUE(group->hasContactId(ContactId));
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_emitsEvent)
    {
        Helper::SignalSpy spy;
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("contact_list_updated");
        ASSERT_TRUE(event);

        event->connectTo(&spy, SLOT(justSlot()));
        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithContactEntry(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.justSlotCalls);
    }

    TEST_F(TestSsiSnacHandler, ssiListParsing_sendsActivatePacket)
    {
        EXPECT_CALL(*socket, snac(handler->getType(), SsiSnacHandler::SnacSsiActivate, _, _));

        bool success = handler->process(SsiSnacHandler::SnacSsiContactList, makeContactListPacketWithContactEntry(), 0, 0);
        ASSERT_TRUE(success);

    }
}
