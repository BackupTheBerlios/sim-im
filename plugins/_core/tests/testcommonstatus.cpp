
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "contacts/client.h"
#include "tests/stubs/stubclient.h"
#include "clientmanager.h"
#include "contacts/protocol.h"
#include "standardclientmanager.h"
#include "commonstatus.h"

namespace
{
    using namespace SIM;
    using ::testing::_;
    using ::testing::Return;
    class MockProtocol : public SIM::Protocol
    {
    public:
        MOCK_METHOD0(name, QString());
        MOCK_METHOD0(iconId, QString());
        MOCK_METHOD0(helpLink, QString());
        MOCK_METHOD1(createClient, QSharedPointer<Client>(Buffer *cfg));
        MOCK_METHOD1(createClient, QSharedPointer<Client>(const QString& name));
        MOCK_METHOD0(states, QStringList());
        MOCK_METHOD1(status, IMStatusPtr(const QString& id));
    };

    class TestCommonStatus : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            SIM::createClientManager();
            manager = SIM::getClientManager();
        }

        virtual void TearDown()
        {
            SIM::destroyClientManager();
        }

        ClientPtr createClient(Protocol* proto)
        {
            return ClientPtr(new StubObjects::StubClient(proto, "test_client"));
        }

        QStringList fullStatusList()
        {
            return QStringList() << "online" << "free_for_chat" << "away" << "n/a" << "dnd" << "offline" << "invisible";
        }

        ClientManager* manager;
    };

//    TEST_F(TestCommonStatus, rebuildStatusList_OneClient)
//    {
//        MockProtocol proto;
//        CommonStatus status(manager);
//        manager->addClient(createClient(&proto));
//        EXPECT_CALL(proto, states()).WillOnce(Return(fullStatusList()));

//        status.rebuildStatusList();

//        ASSERT_EQ(7, status.stateCount());
//    }
}
