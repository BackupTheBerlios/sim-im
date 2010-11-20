
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "icqclient.h"
#include "icqstatuswidget.h"
#include "contacts/imstatus.h"
#include "testicqclient.h"
#include "tests/mocks/mockoscarsocket.h"

namespace
{
    using namespace SIM;
    using namespace MockObjects;
    using ::testing::_;
    using ::testing::InSequence;
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

    TEST_F(TestIcqClient, getDefaultStatus_online)
    {
        IMStatusPtr status = client->getDefaultStatus("online");

        ASSERT_FALSE(status.isNull());
        ASSERT_FALSE(status->flag(IMStatus::flOffline));
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
        MockOscarSocket* oscarSocket = new MockObjects::MockOscarSocket();
        client->setOscarSocket(oscarSocket);
        EXPECT_CALL(*oscarSocket, connectToHost(_, _));

        client->changeStatus(client->getDefaultStatus("online"));
    }

//    TEST_F(TestIcqClient, loginSequence)
//    {
//        Helper::SignalEmitter emitter;
//        emitter.connect(&emitter, SIGNAL(connected()), client, SLOT(oscarSocketConnected()));
//        MockOscarSocket* oscarSocket = new MockObjects::MockOscarSocket();
//        client->setOscarSocket(oscarSocket);

//        emitter.emitConnectedSignal();
//    }
}
