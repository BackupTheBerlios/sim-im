#include "commandhub.h"

namespace SIM
{
CommandHub::CommandHub(QObject *parent) :
    QObject(parent)
{
}

void CommandHub::registerCommand(const UiCommandPtr& cmd)
{
    m_commands.insert(cmd->id(), cmd);
}

void CommandHub::unregisterCommand(const QString& id)
{
    QMap<QString, UiCommandPtr>::iterator it = m_commands.find(id);
    if(it == m_commands.end())
        return;
    m_commands.erase(it);
}

UiCommandPtr CommandHub::command(const QString& id) const
{
    QMap<QString, UiCommandPtr>::const_iterator it = m_commands.find(id);
    if(it == m_commands.end())
        return UiCommandPtr();
    return it.value();
}

QStringList CommandHub::commandsForTag(const QString& tag) const
{
    QStringList ids;
    foreach(const UiCommandPtr& cmd, m_commands)
    {
        if(cmd->tags().contains(tag))
            ids.append(cmd->id());
    }
    return ids;
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
