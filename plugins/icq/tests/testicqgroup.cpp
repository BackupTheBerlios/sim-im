
#include "gtest/gtest.h"

#include "icqclient.h"
#include "icqgroup.h"

namespace
{
    class TestIcqGroup : public ::testing::Test
    {
    public:
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

    TEST_F(TestIcqGroup, contactIdsAddition)
    {
        ICQGroup group(m_client);
        group.addContactId(123);

        ASSERT_TRUE(group.hasContactId(123));
    }

    TEST_F(TestIcqGroup, contactIdsDeletion)
    {
        ICQGroup group(m_client);
        group.addContactId(123);
        group.deleteContactId(123);

        ASSERT_FALSE(group.hasContactId(123));
    }
}
