#ifndef COMMANDSET_H
#define COMMANDSET_H

#include "uicommand.h"
#include "simapi.h"

#include <QList>

class QAction;
namespace SIM {

class CommandHub;
class EXPORT CommandSet
{
public:
    CommandSet(CommandHub* hub);

    void appendCommand(const UiCommandPtr& cmd);
    void appendSeparator();
    int elementsCount() const;

    QList<QAction*> createActionList(QObject* parent) const;

private:
    CommandHub* m_hub;
    struct Entry
    {
        bool separator;
        UiCommandPtr cmd;
    };
    QList<Entry> m_entries;
};

} // namespace SIM

#endif // COMMANDSET_H
