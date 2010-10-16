#include "commandset.h"

#include <QActionGroup>
#include <QMenu>

namespace SIM {

CommandSet::CommandSet(const QString& id, CommandHub* hub) : m_hub(hub), m_id(id)
{
}

QString CommandSet::id() const
{
    return m_id;
}

void CommandSet::appendCommand(const UiCommandPtr& cmd)
{
    Entry entry;
    entry.type = Entry::Command;
    entry.cmd = cmd;
    m_entries.append(entry);
}

void CommandSet::appendSeparator()
{
    Entry entry;
    entry.type = Entry::Separator;
    m_entries.append(entry);
}

int CommandSet::elementsCount() const
{
    return m_entries.size();
}

QList<QAction*> CommandSet::createActionList(QObject* parent) const
{
    QList<QAction*> list;
    QActionGroup* group = new QActionGroup(parent);
    group->setExclusive(true);
    foreach(const Entry& entry, m_entries)
    {
        QAction* action = new QAction(parent);
        if(entry.type == Entry::Separator)
        {
            action->setSeparator(true);
        }
        else if(entry.type == Entry::Command)
        {
            action->setText(entry.cmd->text());
            action->setIcon(entry.cmd->icon());
            action->setCheckable(entry.cmd->isCheckable());
            action->setChecked(entry.cmd->isChecked());
            if(entry.cmd->isAutoExclusive())
                group->addAction(action);

            if(entry.cmd->subcommands())
            {
                QMenu* m = new QMenu(0);
                QList<QAction*> actionslist = entry.cmd->subcommands()->createActionList(m);
                m->addActions(actionslist);
                action->setMenu(m);
            }

            action->connect(entry.cmd.data(), SIGNAL(checked(bool)), SLOT(setChecked(bool)));
            entry.cmd.data()->connect(action, SIGNAL(toggled(bool)), SLOT(setChecked(bool)));
        }
        // TODO submenus
        list.append(action);
    }
    return list;
}

void CommandSet::setText(const QString& text)
{
    m_text = text;
}

QString CommandSet::text() const
{
    return m_text;
}

} // namespace SIM
