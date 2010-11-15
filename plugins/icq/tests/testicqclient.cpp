
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "icqclient.h"
#include "icqstatuswidget.h"
#include "contacts/imstatus.h"
#include "tests/mocks/mockoscarsocket.h"

namespace
{
    using namespace SIM;
    class TestIcqClient : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.123456", false);
        }

        virtual void TearDown()
        {
            delete client;
        }

        ICQClient* client;
    };

    TEST_F(TestIcqClient, getDefaultStatus_offline)
    {
        IMStatusPtr status = client->getDefaultStatus("offline");

        ASSERT_FALSE(status.isNull());
        ASSERT_TRUE(status->flag(IMStatus::flOffline));
    }

    TEST_F(TestIcqClient, onCreation_statusIsOffline)
    {
        IMStatusPtr status = client->currentStatus();

        ASSERT_FALSE(status.isNull());
        ASSERT_TRUE(status->flag(IMStatus::flOffline));
    }

    TEST_F(TestIcqClient, changeStatus_fromOfflineToOnline_causesStatusWidgetBlink)
    {
        StatusWidget* widget = qobject_cast<StatusWidget*>(client->createStatusWidget());
        ASSERT_TRUE(widget);

        client->changeStatus(client->getDefaultStatus("online"));
        ASSERT_TRUE(widget->isBlinking());
    }

    TEST_F(TestIcqClient, changeStatus_fromOfflineToOnline)
    {
        OscarSocket* oscarSocket = new MockObjects::MockOscarSocket();
        client->setOscarSocket(oscarSocket);
        EXPECT_CALL(*oscarSocket, );
    }
}
