
#include <gtest/gtest.h>

#include "jabberclient.h"
#include "jabberstatus.h"

namespace
{
    class TestJabberClient : public ::testing::Test
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

    TEST_F(TestJabberClient, getDefaultStatus_offline_returnsStatusWithOfflineFlag)
    {
        JabberStatusPtr status = client->getDefaultStatus("offline");

        ASSERT_TRUE(status);
        ASSERT_TRUE(status->flag(SIM::IMStatus::flOffline));
    }
}
