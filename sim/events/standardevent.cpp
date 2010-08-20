#include "standardevent.h"

namespace SIM {

StandardEvent::StandardEvent(const QString& eventId) : IEvent()
        , m_id(eventId)
{
}

QString StandardEvent::id()
{
    return m_id;
}

bool StandardEvent::connectTo(QObject* receiver, const char* receiverSlot)
{
    return QObject::connect(this, SIGNAL(eventTriggered()), receiver, receiverSlot);
}

void StandardEvent::triggered(const EventData& data)
{
    Q_UNUSED(data);
    emit eventTriggered();
}

} // namespace SIM
