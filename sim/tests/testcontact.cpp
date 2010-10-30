#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QDomDocument>
#include <QDomElement>

#include "contacts/contact.h"
#include "contacts/client.h"
#include "stubs/stubimcontact.h"
#include "stubs/stubclient.h"
#include "mocks/mockimcontact.h"
#include "mocks/mockimstatus.h"
#include "clientmanager.h"
#include "messaging/genericmessage.h"

namespace
{
    using namespace SIM;
    using namespace MockObjects;
    using ::testing::Return;
    using ::testing::_;

    class TestContact : public ::testing::Test
    {
    protected:

        ClientPtr createStubClient(const QString& id)
        {
            return ClientPtr(new StubObjects::StubClient(0, id));
        }

        IMContactPtr createStubIMContact(const ClientPtr& client)
        {
            return IMContactPtr(new StubObjects::StubIMContact(client.data()));
        }

        MessagePtr createGenericMessage(const IMContactPtr& contact)
        {
            return MessagePtr(new GenericMessage(contact));
        }

        void fillContactData(Contact& c)
        {
            c.setName("Foo");
            c.setNotes("Bar");
            c.setGroupId(42);
            c.setFlag(Contact::flIgnore, true);
            c.setLastActive(112);
        }
    };

    TEST_F(TestContact, clientContact_IfContactIsAdded_ReturnsAddedContact)
    {
        ClientPtr client =  createStubClient("ICQ.123456");
        IMContactPtr imContact = createStubIMContact(client);
        Contact contact(1);
        contact.addClientContact(imContact);

        IMContactPtr returnedContact = contact.clientContact("ICQ.123456");
        EXPECT_TRUE(returnedContact == imContact);
    }

    TEST_F(TestContact, clientContact_IfContactIsntAdded_ReturnsNullPointer)
    {
        Contact contact(1);

        IMContactPtr returnedContact = contact.clientContact("XMPP.bad@motherfucker.com");
        EXPECT_TRUE(!returnedContact);
    }

    TEST_F(TestContact, clientContactNames_ReturnsNamesOfClients)
    {
        ClientPtr client =  createStubClient("ICQ.123456");
        IMContactPtr imContact = createStubIMContact(client);
        Contact contact(1);
        contact.addClientContact(imContact);

        EXPECT_EQ(1, contact.clientContactNames().size());
        EXPECT_TRUE(contact.clientContactNames().contains("ICQ.123456"));
    }

    TEST_F(TestContact, hasUnreadMessages)
    {
        ClientPtr client =  createStubClient("ICQ.123456");
        MockObjects::MockIMContactPtr imContact = MockObjects::MockIMContactPtr(new MockObjects::MockIMContact());
        EXPECT_CALL(*imContact.data(), hasUnreadMessages()).WillOnce(Return(true));
        Contact contact(1);
        contact.addClientContact(imContact);

        EXPECT_TRUE(contact.hasUnreadMessages());
    }

    TEST_F(TestContact, SerializationAndDeserialization)
    {
        ClientPtr client =  createStubClient("ICQ.123456");
        IMContactPtr imContact = createStubIMContact(client);
        Contact contact(1);
        contact.addClientContact(imContact);
        fillContactData(contact);
        QDomDocument doc;
        QDomElement el = doc.createElement("contact");
        SIM::createClientManager();

        contact.serialize(el);
        Contact deserializedContact(1);
        deserializedContact.deserialize(el);

        SIM::destroyClientManager();
        EXPECT_TRUE(deserializedContact.name() == "Foo");
        EXPECT_TRUE(deserializedContact.notes() == "Bar");
        EXPECT_TRUE(deserializedContact.groupId() == 42);
        EXPECT_TRUE(deserializedContact.lastActive() == 112);
        EXPECT_TRUE(deserializedContact.flag(Contact::flIgnore));
    }

    TEST_F(TestContact, isOnline_emptyContact_offline)
    {
        Contact contact(1);

        ASSERT_FALSE(contact.isOnline());
    }

    TEST_F(TestContact, isOnline_subcontactOnline)
    {
        MockIMStatusPtr imStatus = MockIMStatusPtr(new MockIMStatus());
        MockIMContactPtr imContact = MockIMContactPtr(new MockObjects::MockIMContact());
        ON_CALL(*imContact.data(), status()).WillByDefault(Return(imStatus));
        ON_CALL(*imStatus.data(), flag(IMStatus::flOffline)).WillByDefault(Return(false));
        Contact contact(1);
        contact.addClientContact(imContact);

        ASSERT_TRUE(contact.isOnline());
    }
}

// vim: set expandtab:

