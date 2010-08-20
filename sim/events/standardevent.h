#ifndef STANDARDEVENT_H
#define STANDARDEVENT_H

#include <QObject>

#include "ievent.h"

namespace SIM {

class StandardEvent : public IEvent
{
    Q_OBJECT
public:
    StandardEvent(const QString& eventId);

    QString id();
    bool connectTo(QObject* receiver, const char* receiverSlot);

signals:
    void eventTriggered();

public slots:
    virtual void triggered(const EventData& data);

private:
    QString m_id;
};

} // namespace SIM

#endif // STANDARDEVENT_H
