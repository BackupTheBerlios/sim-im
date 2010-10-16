#include "commandhub.h"

namespace SIM
{
CommandHub::CommandHub(QObject *parent) :
    QObject(parent)
{
}

void CommandHub::registerCommand(const UiCommandPtr& cmd)
{
    m_commands.append(cmd);
}

void CommandHub::unregisterCommand(const QString& id)
{
    for(QList<UiCommandPtr>::iterator it = m_commands.begin(); it != m_commands.end(); ++it) {
        if((*it)->id() == id) {
            m_commands.erase(it);
            return;
        }
    }
}

UiCommandPtr CommandHub::command(const QString& id) const
{
    for(QList<UiCommandPtr>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it) {
        if((*it)->id() == id) {
            return *it;
        }
    }
    return UiCommandPtr();
}

QStringList CommandHub::commandsForTag(const QString& tag) const
{
    QStringList ids;
    foreach(const UiCommandPtr& cmd, m_commands) {
        if(cmd->tags().contains(tag))
            ids.append(cmd->id());
    }
    return ids;
}

CommandSetPtr CommandHub::createCommandSet(const QString& id)
{
    CommandSetPtr cmdset = CommandSetPtr(new CommandSet(id, this));
    m_commandSets.append(cmdset);
    return cmdset;
}

CommandSetPtr CommandHub::commandSet(const QString& id) const
{
    foreach(const CommandSetPtr& cmdset, m_commandSets)
    {
        if(cmdset->id() == id)
            return cmdset;
    }
    return CommandSetPtr();
}

void CommandHub::deleteCommandSet(const QString& id)
{
    for(QList<CommandSetPtr>::iterator it = m_commandSets.begin(); it != m_commandSets.end(); ++it)
    {
        if((*it)->id() == id)
        {
            m_commandSets.erase(it);
            return;
        }
    }
}

static CommandHub* g_commandHub = 0;

EXPORT CommandHub* getCommandHub()
{
    return g_commandHub;
}

void EXPORT createCommandHub()
{
    Q_ASSERT(!g_commandHub);
    g_commandHub = new CommandHub();
}

void EXPORT destroyCommandHub()
{
    Q_ASSERT(g_commandHub);
    delete g_commandHub;
    g_commandHub = 0;
}

}
