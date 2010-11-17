
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "qt-gtest.h"

#include "icqcontact.h"
#include "icqclient.h"

namespace
{
    class TestIcqContact : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_client = new ICQClient(0, "ICQ.123456", false);
        }

        virtual void TearDown()
        {
            delete m_client;
        }

        ICQClient* m_client;
    };

    TEST_F(TestIcqContact, Creation)
    {
        ICQContact contact(m_client);

        ASSERT_TRUE(contact.status());
        ASSERT_EQ(contact.status()->id(), "offline");
    }
}
