#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/commandhub.h"
#include "commands/commandset.h"
#include "commands/uicommand.h"

namespace
{
    using namespace SIM;
    class TestCommandSet : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            hub = new CommandHub();
            hub->registerCommand(UiCommand::create("Test", "", "test_cmd1"));
            hub->registerCommand(UiCommand::create("Test", "", "test_cmd2"));
        }

        virtual void TearDown()
        {
            delete hub;
        }

        void addDefaultCommands(CommandSet& set)
        {
            set.appendCommand(hub->command("test_cmd1"));
            set.appendSeparator();
            set.appendCommand(hub->command("test_cmd2"));
        }

        CommandHub* hub;
    };

    TEST_F(TestCommandSet, Addition)
    {
        CommandSet set("cmdset", hub);
        set.appendCommand(hub->command("test_cmd1"));
        set.appendSeparator();
        set.appendCommand(hub->command("test_cmd2"));

        ASSERT_EQ(3, set.elementsCount());
    }

    TEST_F(TestCommandSet, createActionList)
    {
        CommandSet set("cmdset", hub);
        addDefaultCommands(set);

        QList<QAction*> actions = set.createActionList(0);
        EXPECT_EQ(3, actions.size());
        EXPECT_TRUE(actions.at(1)->isSeparator());

        qDeleteAll(actions);
    }
}
