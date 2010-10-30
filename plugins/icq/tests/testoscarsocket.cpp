
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "oscarsocket.h"

#include "tests/mocks/mockasyncsocket.h"

namespace
{
#include "tests/gtest-qt.h"

    class TestOscarSocket : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new OscarSocket();
            asyncSocket = new MockObjects::MockAsyncSocket();
            socket->setSocket(asyncSocket);
        }

        virtual void TearDown()
        {
            delete socket;
        }

        OscarSocket* socket;
        MockObjects::MockAsyncSocket* asyncSocket;
    };

    TEST_F(TestOscarSocket, connectToHost)
    {
        EXPECT_CALL(*asyncSocket, connectToHost(QString("testhost"), 1234));

        socket->connectToHost("testhost", 1234);
    }
}
