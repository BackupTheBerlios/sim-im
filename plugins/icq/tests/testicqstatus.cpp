
#include <gtest/gtest.h>

#include "icqstatus.h"

namespace
{
    class TestIcqStatus : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
        }

        virtual void TearDown()
        {
        }
    };

    TEST_F(TestIcqStatus, clone_savesFlags)
    {
        ICQStatus status("offline", "Offline", false, QString(), QPixmap());
        status.setFlag(SIM::IMStatus::flOffline, true);
        status.setFlag(SIM::IMStatus::flInvisible, true);

        SIM::IMStatusPtr cloned = status.clone();

        ASSERT_TRUE(cloned->flag(SIM::IMStatus::flOffline));
        ASSERT_TRUE(cloned->flag(SIM::IMStatus::flInvisible));
    }
}
