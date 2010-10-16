#ifndef COMMANDSET_H
#define COMMANDSET_H

#include "uicommand.h"
#include "simapi.h"

#include <QList>
#include <QSharedPointer>

class QAction;
namespace SIM {

class CommandHub;
class EXPORT CommandSet;
typedef QSharedPointer<CommandSet> CommandSetPtr;

class EXPORT CommandSet
{
public:
    CommandSet(const QString& id, CommandHub* hub);

    QString id() const;

    void setText(const QString& text);
    QString text() const;

    void appendCommand(const UiCommandPtr& cmd);
    void appendSeparator();
    int elementsCount() const;

    QList<QAction*> createActionList(QObject* parent) const;

private:
    CommandHub* m_hub;
    struct Entry
    {
        enum Type
        {
            Separator = 0,
            Command
        };

        Type type;
        UiCommandPtr cmd;
    };
    QList<Entry> m_entries;
    QString m_id;
    QString m_text;
};

} // namespace SIM

#endif // COMMANDSET_H
