
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/commandhub.h"
#include "commands/uicommand.h"
#include "imagestorage/imagestorage.h"
#include "stubs/stubimagestorage.h"

namespace 
{
    using namespace SIM;
    class TestCommandHub : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            createStubImageStorage();
            SIM::createCommandHub();
        }

        virtual void TearDown()
        {
            SIM::destroyCommandHub();
            destroyStubImageStorage();
        }

        void createStubImageStorage()
        {
            imageStorage = new StubObjects::StubImageStorage();
            SIM::setImageStorage(imageStorage);
        }

        void destroyStubImageStorage()
        {
            delete imageStorage;
            SIM::setImageStorage(0);
        }

        ImageStorage* imageStorage;
    };

    TEST_F(TestCommandHub, Manipulation)
    {
        UiCommandPtr cmd = UiCommand::create("Test command", "test", "test_cmd");
        getCommandHub()->registerCommand(cmd);

        UiCommandPtr c = getCommandHub()->command("test_cmd");
        ASSERT_EQ(cmd, c);
    }

    TEST_F(TestCommandHub, CommandSetManipulation)
    {
        CommandSetPtr newcmdset = getCommandHub()->createCommandSet("test_command_set");

        CommandSetPtr cmdset = getCommandHub()->commandSet("test_command_set");

        ASSERT_TRUE(cmdset);
        ASSERT_TRUE(newcmdset == cmdset);
    }

    TEST_F(TestCommandHub, CommandSetDeletion)
    {
        CommandSetPtr newcmdset = getCommandHub()->createCommandSet("test_command_set");
		//getCommandHub()->createCommandSet("test_command_set");

        getCommandHub()->deleteCommandSet("test_command_set"); //crashing
        CommandSetPtr cmdset = getCommandHub()->commandSet("test_command_set");

        ASSERT_FALSE(cmdset);

    }
}
