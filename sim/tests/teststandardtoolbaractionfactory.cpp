#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QComboBox>
#include <QToolButton>

#include "stubs/stubimagestorage.h"
#include "standardtoolbaractionfactory.h"

namespace
{
    using namespace SIM;
    class TestStandardToolbarActionFactory : public ::testing::Test
    {
    protected:
        StubObjects::StubImageStorage storage;
        virtual void SetUp()
        {
            setImageStorage(&storage);
            factory = new StandardToolbarActionFactory();
        }

        virtual void TearDown()
        {
            delete factory;
            setImageStorage(NULL);
        }

        UiCommandPtr createButtonCmd()
        {
            UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");
            cmd->setWidgetType(UiCommand::wtButton);
            return cmd;
        }

        ToolbarActionFactory* factory;
    };

    TEST_F(TestStandardToolbarActionFactory, ButtonCreation)
    {
        UiCommandPtr cmd = createButtonCmd();

        QWidget* widget = factory->createWidget(cmd, 0);
        ASSERT_TRUE(widget != NULL);
        ASSERT_TRUE(widget->inherits("QAbstractButton"));

        delete widget;
    }

    TEST_F(TestStandardToolbarActionFactory, ActionCreation)
    {
        UiCommandPtr cmd = UiCommand::create("Test", "", "test_cmd");
        cmd->setWidgetType(UiCommand::wtButton);

        QAction* action = factory->createAction(cmd, 0);
        ASSERT_TRUE(action != NULL);
        EXPECT_TRUE(action->text() == cmd->text());
        EXPECT_TRUE(action->isCheckable() == cmd->isCheckable());
        EXPECT_TRUE(action->isChecked() == cmd->isChecked());

        delete action;
    }
}
