#ifndef LOGEVENT_H
#define LOGEVENT_H

#include "ievent.h"
#include "eventdata.h"

namespace SIM {

class EXPORT LogEventData : public EventData
{
public:
    LogEventData(const QString& msg, int level);
    virtual QString eventId() const;

    QString message() const { return m_msg; }
    int level() const { return m_level; }

private:
    QString m_msg;
    int m_level;
};

class EXPORT LogEvent : public IEvent
{
    Q_OBJECT
public:
    static IEventPtr create();

    virtual QString id();
    virtual bool connectTo(QObject* receiver, const char* receiverSlot);

signals:
    void eventTriggered(const QString& msg, int level);

public slots:
    virtual void triggered(const EventDataPtr& data);
private:
    LogEvent();
    Q_DISABLE_COPY(LogEvent);
};

} // namespace SIM

#endif // LOGEVENT_H
