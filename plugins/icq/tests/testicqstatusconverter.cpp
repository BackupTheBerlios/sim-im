
#include "gtest/gtest.h"
#include "qt-gtest.h"
#include "icqstatusconverter.h"
#include "icqclient.h"

namespace
{
    class TestICQStatusConverter : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.123456", false);
            converter = new ICQStatusConverter(client);
        }

        virtual void TearDown()
        {
            delete converter;
            delete client;
        }

        ICQStatusConverter* converter;
        ICQClient* client;
    };

    TEST_F(TestICQStatusConverter, convertsOnline)
    {
        ICQStatusPtr status = converter->makeStatus(0x10000000);
        ASSERT_TRUE(status);
        ASSERT_EQ("online", status->id());
    }
}
