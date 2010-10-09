#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/uicommand.h"

#include "testuicommand.h"
#include "imagestorage/imagestorage.h"
#include "stubs/stubimagestorage.h"

namespace
{
    using namespace SIM;
    class TestUiCommand : public ::testing::Test
    {
    protected:
        StubObjects::StubImageStorage storage;
        virtual void SetUp()
        {
            setImageStorage(&storage);
        }

        virtual void TearDown()
        {
            setImageStorage(NULL);
        }

    };

    TEST_F(TestUiCommand, SubscribeTo)
    {
        ::Test::TestObject obj;
        UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");

        cmd->subscribeTo(&obj, SLOT(cmdTriggered()));
        cmd->trigger();

        ASSERT_EQ(obj.calls, 1);
    }

    TEST_F(TestUiCommand, Unsubscribe)
    {
        ::Test::TestObject obj;
        UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");

        cmd->subscribeTo(&obj, SLOT(cmdTriggered()));
        cmd->unsubscribe(&obj, SLOT(cmdTriggered()));
        cmd->trigger();

        ASSERT_EQ(obj.calls, 0);
    }

    TEST_F(TestUiCommand, setChecked_ChangesState_emitsSignal)
    {
        ::Test::TestObject obj;
        UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");
        cmd->setChecked(false);

        QObject::connect(cmd.data(), SIGNAL(checked(bool)), &obj, SLOT(cmdTriggered()));
        cmd->setChecked(true);
\
        ASSERT_EQ(obj.calls, 1);
    }

    TEST_F(TestUiCommand, setChecked_SameState_doesntEmitSignal)
    {
        ::Test::TestObject obj;
        UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");

        cmd->setChecked(true);
        QObject::connect(cmd.data(), SIGNAL(checked(bool)), &obj, SLOT(cmdTriggered()));
        cmd->setChecked(true);
\
        ASSERT_EQ(obj.calls, 0);
    }
}
