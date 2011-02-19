#ifndef CONTACTEVENT_H
#define CONTACTEVENT_H

#include "ievent.h"
#include "simapi.h"

namespace SIM
{
    class EXPORT ContactEventData : public EventData
    {
    public:
        static EventDataPtr create(int contactId);
        virtual QString eventId() const;

        int contactId() const;

    private:
        ContactEventData(int contactId);
        int m_contactId;
    };

    class EXPORT ContactEvent : public IEvent
    {
        Q_OBJECT
    public:
        static IEventPtr create(const QString& eventId);

        virtual QString id();
        virtual bool connectTo(QObject* receiver, const char* receiverSlot);

    signals:
        void eventTriggered(int contactId);

    public slots:
        virtual void triggered(const EventDataPtr& data);

    private:
        Q_DISABLE_COPY(ContactEvent);
        ContactEvent(const QString& eventId);

        QString m_id;
    };
}

#endif // CONTACTEVENT_H
