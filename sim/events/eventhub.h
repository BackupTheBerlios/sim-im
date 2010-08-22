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

class EXPORT EventHub : public QObject
{
    Q_OBJECT
public:
    EventHub();
    bool registerEvent(const IEventPtr& event);
    bool unregisterEvent(const QString& id);

    void triggerEvent(const QString& id, const EventDataPtr& data = EventDataPtr());
    IEventPtr getEvent(const QString& id);
signals:

public slots:

private:
    QMap<QString, IEventPtr> m_events;

};

void EXPORT createEventHub();
void EXPORT destroyEventHub();
EXPORT EventHub* getEventHub();

}

#endif // EVENTHUB_H
