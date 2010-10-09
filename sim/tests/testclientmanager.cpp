
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "stubs/stubclient.h"

#include "contacts/client.h"
#include "clientmanager.h"
#include "contacts.h"
#include "events/eventhub.h"

namespace
{
    using namespace SIM;
    class MockClient : public SIM::Client
    {
        QString name();
        QString dataName(void*);
        QWidget *setupWnd();
		IMContact* getOwnerContact();
        void setOwnerContact(IMContact* contact);
        QWidget *searchWindow(QWidget *parent);
        bool isMyData(IMContact*&, Contact*&);
        bool createData(IMContact*&, Contact*);
        void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons);
        void setupContact(Contact*, void *data);
        bool send(Message*, void *data);
        bool canSend(unsigned type, void *data);
    };

    class TestClientManager : public ::testing::Test
    {
    protected:
        void SetUp()
        {
            SIM::createEventHub();
            SIM::createClientManager();
            SIM::createContactList();
        }

        void TearDown()
        {
            SIM::destroyClientManager();
            SIM::destroyContactList();
            SIM::destroyEventHub();
        }

        void testClientManipulation();
    };

    TEST_F(TestClientManager, ClientManipulation)
    {
        SIM::ClientPtr icqclient = SIM::ClientPtr(new StubObjects::StubClient("ICQ.666666666"));
        SIM::ClientPtr jabberclient = SIM::ClientPtr(new StubObjects::StubClient("Jabber.loh@jabber.org"));
        EXPECT_TRUE(icqclient->name() == QString("ICQ.666666666"));
        SIM::getClientManager()->addClient(icqclient);
        SIM::getClientManager()->addClient(jabberclient);
        SIM::ClientPtr client2 = SIM::getClientManager()->client("ICQ.666666666");
        EXPECT_TRUE(client2->name() == QString("ICQ.666666666"));

        SIM::ClientPtr nonexistant = SIM::getClientManager()->client("Nothing");
        EXPECT_TRUE(nonexistant.isNull());
    }
}

// vim: set expandtab:

