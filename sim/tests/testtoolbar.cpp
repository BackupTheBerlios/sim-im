#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "simgui/toolbar.h"
#include "stubs/stubimagestorage.h"

namespace
{
    using namespace SIM;
    using ::testing::_;
    using ::testing::Return;

    class MockToolbarActionFactory : public ToolbarActionFactory
    {
    public:
        MOCK_METHOD2(createAction, QAction*(const UiCommandPtr& cmd, QWidget* parent));
        MOCK_METHOD2(createWidget, QWidget*(const UiCommandPtr& cmd, QWidget* parent));
        MOCK_METHOD1(createSeparator, QAction*(QWidget* parent));
    };

    class TestToolbar : public ::testing::Test
    {
    protected:
        StubObjects::StubImageStorage storage;
        virtual void SetUp()
        {
            setImageStorage(&storage);
            toolbar = new ToolBar("test toolbar");
            factory = new MockToolbarActionFactory();
            toolbar->setToolbarActionFactory(factory);
        }

        virtual void TearDown()
        {
            setImageStorage(NULL);
            delete toolbar;
        }

        ToolBar* toolbar;
        MockToolbarActionFactory* factory;
    };

    TEST_F(TestToolbar, addUiCommand_UsesToolbarActionFactory)
    {
        UiCommandPtr cmd = UiCommand::create("Test", QString(), "test_cmd");
        QWidget* w = new QWidget(toolbar);
        EXPECT_CALL(*factory, createWidget(_, toolbar)).WillOnce(Return(w));

        toolbar->addUiCommand(cmd);

        // `w' destroyed here, because it's owned by ToolBar
    }

    TEST_F(TestToolbar, addSeparator_UsesToolbarActionFactory)
    {
        QAction* act = new QAction(toolbar);
        act->setSeparator(true);
        EXPECT_CALL(*factory, createSeparator(toolbar)).WillOnce(Return(act));

        toolbar->addSeparator();
    }

    TEST_F(TestToolbar, addUiCommand_WhenAddingCommandWithSubcommands_CreatesActionsForSubcommands)
    {
        UiCommandPtr cmd = UiCommand::create("Test", QString(), "test_cmd");
        UiCommandPtr subcmd1 = UiCommand::create("Test", QString(), "test_subcmd1");
        UiCommandPtr subcmd2 = UiCommand::create("Test", QString(), "test_subcmd2");
        cmd->addSubCommand(subcmd1);
        cmd->addSubCommand(subcmd2);
        QWidget* w = new QWidget(toolbar);
        QAction* subcmd1Action = new QAction(toolbar);
        QAction* subcmd2Action = new QAction(toolbar);

        EXPECT_CALL(*factory, createWidget(_, toolbar)).WillOnce(Return(w));
        EXPECT_CALL(*factory, createAction(_, toolbar)).
                WillOnce(Return(subcmd1Action)).
                WillOnce(Return(subcmd2Action));

        toolbar->addUiCommand(cmd);
    }
}
