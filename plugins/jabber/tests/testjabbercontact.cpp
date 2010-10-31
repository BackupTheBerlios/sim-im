
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "jabberclient.h"
#include "jabbercontact.h"

namespace
{
    class TestJabberContact : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            client = new JabberClient(0, "Jabber.test@jabber.test");
        }

        virtual void TearDown()
        {
            delete client;
        }

        JabberClient* client;
    };

    TEST_F(TestJabberContact, Creation)
    {
        JabberContact contact(client);

        ASSERT_TRUE(contact.status());
        ASSERT_TRUE(contact.status()->flag(SIM::IMStatus::flOffline));
    }
}
