#include "commandset.h"

namespace SIM {

CommandSet::CommandSet(CommandHub* hub) : m_hub(hub)
{
}

void CommandSet::appendCommand(const UiCommandPtr& cmd)
{
    Entry entry;
    entry.separator = false;
    entry.cmd = cmd;
    m_entries.append(entry);
}

void CommandSet::appendSeparator()
{
    Entry entry;
    entry.separator = true;
    m_entries.append(entry);
}

int CommandSet::elementsCount() const
{
    return m_entries.size();
}

QList<QAction*> CommandSet::createActionList(QObject* parent) const
{
    QList<QAction*> list;
    foreach(const Entry& entry, m_entries)
    {
        QAction* action = new QAction(parent);
        if(entry.separator)
        {
            action->setSeparator(true);
        }
        else
        {
            action->setText(entry.cmd->text());
            action->setIcon(entry.cmd->icon());
            action->setCheckable(entry.cmd->isCheckable());
            action->setChecked(entry.cmd->isChecked());
        }
        list.append(action);
    }
    return list;
}

} // namespace SIM
