#ifndef STANDARDEVENT_H
#define STANDARDEVENT_H

#include <QObject>

#include "ievent.h"

namespace SIM {

class EXPORT StandardEvent : public IEvent
{
    Q_OBJECT
public:

    QString id();
    bool connectTo(QObject* receiver, const char* receiverSlot);

    static IEventPtr create(const QString& eventId);

signals:
    void eventTriggered();

public slots:
    virtual void triggered(const EventDataPtr& data);

private:
    StandardEvent(const QString& eventId);
    Q_DISABLE_COPY(StandardEvent);
    QString m_id;
};

} // namespace SIM

#endif // STANDARDEVENT_H
