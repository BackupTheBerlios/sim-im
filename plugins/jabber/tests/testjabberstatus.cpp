#include <gtest/gtest.h>

#include "jabberstatus.h"

namespace
{
    class TestJabberStatus : public ::testing::Test
    {
    protected:
    };

    TEST_F(TestJabberStatus, clone_copiesFlags)
    {
        JabberStatus status("test", "test", false, QString(), QPixmap(), QString(), QString());
        status.setFlag(SIM::IMStatus::flOffline, true);
        status.setFlag(SIM::IMStatus::flInvisible, true);

        SIM::IMStatusPtr newStatus = status.clone();

        ASSERT_TRUE(newStatus->flag(SIM::IMStatus::flOffline));
        ASSERT_TRUE(newStatus->flag(SIM::IMStatus::flInvisible));
    }
}
