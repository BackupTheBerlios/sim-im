
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "icqclient.h"
#include "contacts/imstatus.h"

namespace
{
    using namespace SIM;
    class TestIcqClient : public ::testing::Test
    {
    protected:
    };

    TEST_F(TestIcqClient, getDefaultStatus_offline)
    {
        ICQClient client(0, "ICQ.123456", false);

        IMStatusPtr status = client.getDefaultStatus("offline");

        ASSERT_FALSE(status.isNull());
        ASSERT_TRUE(status->flag(IMStatus::flOffline));
    }
}
