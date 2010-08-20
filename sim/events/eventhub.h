#ifndef EVENTHUB_H
#define EVENTHUB_H

#include <QObject>
#include <QString>
#include <QMap>
#include "ievent.h"
#include "eventdata.h"
#include "simapi.h"

namespace SIM
{

class EventHub : public QObject
{
    Q_OBJECT
public:
    EventHub();
    bool registerEvent(const IEventPtr& event);
    bool unregisterEvent(const QString& id);

    void triggerEvent(const QString& id, const EventDataPtr& data);
    IEventPtr event(const QString& id);
signals:

public slots:

private:
    QMap<QString, IEventPtr> m_events;

};

void createEventHub();
void destroyEventHub();
EventHub* EXPORT getEventHub();
}

#endif // EVENTHUB_H
