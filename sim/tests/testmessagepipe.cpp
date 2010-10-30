#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "messaging/messagepipe.h"
#include "messaging/messageprocessor.h"
#include "messaging/message.h"
#include "mocks/mockcontactlist.h"

namespace
{
    using namespace SIM;
    using ::testing::_;
    using ::testing::Return;
    using ::testing::InSequence;
    using ::testing::DefaultValue;
    class MockMessageProcessor : public MessageProcessor
    {
    public:
        MOCK_CONST_METHOD0(id, QString());
        MOCK_METHOD1(process, MessageProcessor::ProcessResult(const MessagePtr& message));
    };

    class StubMessage : public Message
    {
    public:
        Client* client() { return 0; }
        IMContactWeakPtr contact() const { return IMContactWeakPtr(); }
        QIcon icon() { return QIcon(); }
        QDateTime timestamp() { return QDateTime(); }
        QString toHtml() { return QString(); }
        QString toPlainText() { return QString(); }
        QString originatingClientId() const { return QString(); }
        QStringList choices() { return QStringList(); }
    };

    class TestMessagePipe : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            SIM::createMessagePipe();
            pipe = SIM::getMessagePipe();
        }

        virtual void TearDown()
        {
            pipe = 0;
            SIM::destroyMessagePipe();
        }

        MessagePipe* pipe;
    };

    TEST_F(TestMessagePipe, pushMessage_CallsProcessors)
    {
        MockMessageProcessor* processor = new MockMessageProcessor();
        MessagePtr msg = MessagePtr(new StubMessage());
        EXPECT_CALL(*processor, process(_)).WillOnce(Return(MessageProcessor::Success));
        pipe->addMessageProcessor(processor);
        pipe->pushMessage(msg);
    }

    TEST_F(TestMessagePipe, pushMessage_BlockingWorks)
    {
        MockObjects::MockContactList cl;
        pipe->setContactList(&cl);
        MockMessageProcessor* processor = new MockMessageProcessor();
        MessagePtr msg = MessagePtr(new StubMessage());
        EXPECT_CALL(*processor, process(_)).WillOnce(Return(MessageProcessor::Block));
        EXPECT_CALL(cl, incomingMessage(_)).Times(0);
        pipe->addMessageProcessor(processor);
        pipe->pushMessage(msg);
    }

    TEST_F(TestMessagePipe, addMessageProcessor_PreservesOrder)
    {
        MockMessageProcessor* processor1 = new MockMessageProcessor();
        MockMessageProcessor* processor2 = new MockMessageProcessor();
        MessagePtr msg = MessagePtr(new StubMessage());

        DefaultValue<MessageProcessor::ProcessResult>::Set(MessageProcessor::Block);
        {
            InSequence seq;
            EXPECT_CALL(*processor1, process(_)).WillOnce(Return(MessageProcessor::Success));
            EXPECT_CALL(*processor2, process(_)).WillOnce(Return(MessageProcessor::Success));
        }
        pipe->addMessageProcessor(processor1);
        pipe->addMessageProcessor(processor2);
        pipe->pushMessage(msg);
    }
}
