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
