#include <QToolButton>
#include <QMenu>

#include "toolbarbutton.h"
#include "toolbarcombobox.h"
#include "standardtoolbaractionfactory.h"

namespace SIM {

QWidget* StandardToolbarActionFactory::createButton(const UiCommandPtr& cmd, QWidget* parent)
{
    ToolbarButton* w = new ToolbarButton(cmd, parent);

    w->setText(cmd->text());
    w->setIcon(cmd->icon());
    w->setCheckable(cmd->isCheckable());
    w->setChecked(cmd->isChecked());
    w->setAutoExclusive(cmd->isAutoExclusive());
    w->setPopupMode(QToolButton::InstantPopup);

    w->connect(cmd.data(), SIGNAL(checked(bool)), w, SLOT(setChecked(bool)));
    w->connect(w, SIGNAL(toggled(bool)), cmd.data(), SLOT(setChecked(bool)));

    return w;
}

QWidget* StandardToolbarActionFactory::createCombobox(const UiCommandPtr& cmd, QWidget* parent)
{
    ToolbarComboBox* w = new ToolbarComboBox(cmd, parent);

    w->setText(cmd->text());
    w->setIcon(cmd->icon());
    w->setAutoExclusive(cmd->isAutoExclusive());
    return w;
}

QWidget* StandardToolbarActionFactory::createWidget(const UiCommandPtr& cmd, QWidget* parent)
{
    switch(cmd->widgetType())
    {
    case UiCommand::wtButton:
        return createButton(cmd, parent);
    case UiCommand::wtCombobox:
        return createCombobox(cmd, parent);
    case UiCommand::wtNone:
    default:
        return NULL;
    }
}

QAction* StandardToolbarActionFactory::createSeparator(QWidget* parent)
{
    QAction* separator = new QAction(parent);
    separator->setSeparator(true);
    return separator;
}

QAction* StandardToolbarActionFactory::createAction(const UiCommandPtr& cmd, QWidget* parent)
{
    QAction* action = new QAction(cmd->icon(), cmd->text(), parent);
    action->setCheckable(cmd->isCheckable());
    action->setChecked(cmd->isChecked());
    action->connect(action, SIGNAL(triggered()), cmd.data(), SLOT(trigger()));
    return action;
}

QMenu* StandardToolbarActionFactory::createMenuWithCommands(const QList<UiCommandPtr>& cmds, QWidget* parent)
{
    QMenu* menu = new QMenu(parent);
    QActionGroup* group = new QActionGroup(parent);
    foreach(const UiCommandPtr& cmd, cmds)
    {
        QAction* subaction = createAction(cmd, menu);
        menu->addAction(subaction);
        if(cmd->isAutoExclusive())
        {
            group->addAction(subaction);
        }
        subaction->connect(cmd.data(), SIGNAL(checked(bool)), subaction, SLOT(setChecked(bool)));
        subaction->connect(subaction, SIGNAL(toggled(bool)), cmd.data(), SLOT(setChecked(bool)));
        // TODO submenus
    }
    return menu;
}

} // namespace SIM
