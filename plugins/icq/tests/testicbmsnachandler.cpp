
#include "gtest/gtest.h"
#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "icbmsnachandler.h"
#include "oscarsocket.h"
#include "mocks/mockoscarsocket.h"


namespace
{
    using ::testing::_;
    using ::testing::NiceMock;
    static const int MinMessageInterval = 1000;

    class TestIcbmSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<IcbmSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_MESSAGE));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeParametersInfoPacket()
        {
            ByteArrayBuilder builder;
            builder.appendWord(0x0004); // Channel
            builder.appendDword(0x00000003); // Flags
            builder.appendWord(0x0200); // Max snac size
            builder.appendWord(0x0384); // Max sender warn level
            builder.appendWord(0x03e7); // Max receiver warn level
            builder.appendDword(MinMessageInterval);
            return builder.getArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        IcbmSnacHandler* handler;
    };

    TEST_F(TestIcbmSnacHandler, parametersPacket_processing)
    {
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(MinMessageInterval, handler->minMessageInterval());
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_sendNewParameters)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSetParameters, _, _)).Times(1);

        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(), 0, 0);
        ASSERT_TRUE(success);
    }
}
