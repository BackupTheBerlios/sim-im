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
        }

        virtual void TearDown()
        {
            setImageStorage(NULL);
            delete toolbar;
        }

        UiCommandPtr createTestCommand(const QString& id, const QString& text = "Test")
        {
            return UiCommand::create(text, QString(), id);
        }

        ToolBar* toolbar;
    };

    TEST_F(TestToolbar, addUiCommand_UsesToolbarActionFactory)
    {
        MockToolbarActionFactory* factory = new MockToolbarActionFactory();
        toolbar->setToolbarActionFactory(factory);
        UiCommandPtr cmd = createTestCommand("test_cmd");
        QWidget* w = new QWidget(toolbar);
        EXPECT_CALL(*factory, createWidget(_, toolbar)).WillOnce(Return(w));

        toolbar->addUiCommand(cmd);

        // `w' destroyed here, because it's owned by ToolBar
    }

    TEST_F(TestToolbar, addSeparator_UsesToolbarActionFactory)
    {
        MockToolbarActionFactory* factory = new MockToolbarActionFactory();
        toolbar->setToolbarActionFactory(factory);
        QAction* act = new QAction(toolbar);
        act->setSeparator(true);
        EXPECT_CALL(*factory, createSeparator(toolbar)).WillOnce(Return(act));

        toolbar->addSeparator();
    }
}
