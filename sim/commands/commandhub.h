#ifndef COMMANDHUB_H
#define COMMANDHUB_H

#include <QObject>
#include <QMap>
#include "uicommand.h"
#include "simapi.h"
#include "commandset.h"

namespace SIM
{
class EXPORT CommandHub : public QObject
{
    Q_OBJECT
public:
    explicit CommandHub(QObject *parent = 0);

    void registerCommand(const UiCommandPtr& cmd);
    void unregisterCommand(const QString& id);
    UiCommandPtr command(const QString& id) const;
    QStringList commandsForTag(const QString& tag) const;

    CommandSetPtr createCommandSet(const QString& id);
    CommandSetPtr commandSet(const QString& id) const;
    void deleteCommandSet(const QString& id);
signals:

public slots:

private:
    QList<UiCommandPtr> m_commands;
    QList<CommandSetPtr> m_commandSets;

};

EXPORT CommandHub* getCommandHub();
void EXPORT createCommandHub();
void EXPORT destroyCommandHub();
}

#endif // COMMANDHUB_H
