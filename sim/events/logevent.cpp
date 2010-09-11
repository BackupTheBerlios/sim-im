#include <cstdio>
#include "logevent.h"

namespace SIM {

IEventPtr LogEvent::create()
{
    return IEventPtr(new LogEvent());
}

LogEvent::LogEvent()
{
}

QString LogEvent::id()
{
    return "log";
}

bool LogEvent::connectTo(QObject* receiver, const char* receiverSlot)
{
    return QObject::connect(this, SIGNAL(eventTriggered(QString, int)), receiver, receiverSlot);
}

void LogEvent::triggered(const EventDataPtr& data)
{
    if(data->eventId() != id())
    {
        printf("LogEvent data type mismatch\n");
        return;
    }

    QSharedPointer<LogEventData> d = data.staticCast<LogEventData>();
    emit eventTriggered(d->message(), d->level());
}

LogEventData::LogEventData(const QString& msg, int level) : EventData(),
    m_msg(msg), m_level(level)
{

}

QString LogEventData::eventId() const
{
    return "log";
}

} // namespace SIM
