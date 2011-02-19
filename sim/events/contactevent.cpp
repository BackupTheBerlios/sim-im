#include "contactevent.h"

namespace SIM
{
    EventDataPtr ContactEventData::create(int contactId)
    {
        return QSharedPointer<ContactEventData>(new ContactEventData(contactId));
    }

    ContactEventData::ContactEventData(int contactId) : m_contactId(contactId)
    {
    }

    QString ContactEventData::eventId() const
    {
        return "contact";
    }

    int ContactEventData::contactId() const
    {
        return m_contactId;
    }

    ContactEvent::ContactEvent(const QString& eventId) : m_id(eventId)
    {
    }

    IEventPtr ContactEvent::create(const QString& eventId)
    {
        return IEventPtr(new ContactEvent(eventId));
    }

    QString ContactEvent::id()
    {
        return m_id;
    }

    bool ContactEvent::connectTo(QObject* receiver, const char* receiverSlot)
    {
        return QObject::connect(this, SIGNAL(eventTriggered(int)), receiver, receiverSlot);
    }

    void ContactEvent::triggered(const EventDataPtr& data)
    {
        if(data->eventId() != "contact")
            return;
        QSharedPointer<ContactEventData> d = data.dynamicCast<ContactEventData>();
        if(d.isNull())
            return;
        emit eventTriggered(d->contactId());
    }
}
