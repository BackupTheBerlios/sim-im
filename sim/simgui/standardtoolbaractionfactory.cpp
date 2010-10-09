#include <QToolButton>
#include <QMenu>

#include "standardtoolbaractionfactory.h"

namespace SIM {

QWidget* StandardToolbarActionFactory::createButton(const UiCommandPtr& cmd, QWidget* parent)
{
    QToolButton* w = new QToolButton(parent);
    w->setText(cmd->text());
    w->setIcon(cmd->icon());
    w->setCheckable(cmd->isCheckable());
    w->setChecked(cmd->isChecked());
    w->setPopupMode(QToolButton::InstantPopup);

    QList<UiCommandPtr> subcmds = cmd->subCommands();
    if(subcmds.size() > 0)
    {
        QMenu* menu = createMenuWithCommands(subcmds, parent);
        w->setMenu(menu);
    }

    return w;
}

QWidget* StandardToolbarActionFactory::createWidget(const UiCommandPtr& cmd, QWidget* parent)
{
    switch(cmd->widgetType())
    {
    case UiCommand::wtButton:
        return createButton(cmd, parent);
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
    return action;
}

QMenu* StandardToolbarActionFactory::createMenuWithCommands(const QList<UiCommandPtr>& cmds, QWidget* parent)
{
    QMenu* menu = new QMenu(parent);
    foreach(const UiCommandPtr& cmd, cmds)
    {
        QAction* subaction = createAction(cmd, menu);
        menu->addAction(subaction);
        // TODO submenus
    }
    return menu;
}

} // namespace SIM
