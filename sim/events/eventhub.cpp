#include "eventhub.h"

namespace SIM
{
static EventHub* g_eventHub;

EventHub::EventHub() : QObject()
{
}

bool EventHub::registerEvent(const IEventPtr& event)
{
    QMap<QString, IEventPtr>::iterator it = m_events.find(event->id());
    if(it != m_events.end())
        return false;
    m_events.insert(event->id(), event);
    return true;
}

bool EventHub::unregisterEvent(const QString& id)
{
    QMap<QString, IEventPtr>::iterator it = m_events.find(event->id());
    if(it != m_events.end())
        return false;
    m_events.erase(it);
    return true;
}

void EventHub::triggerEvent(const QString& id, const EventDataPtr& data)
{
    IEventPtr ev = event(id);
    if(!ev)
        return;
    ev->triggered(data);
}

IEventPtr EventHub::event(const QString& id)
{
    QMap<QString, IEventPtr>::iterator it = m_events.find(event->id());
    if(it != m_events.end())
        return IEventPtr();
    return it.value();
}

void createEventHub()
{
    if(g_eventHub)
        return;
    g_eventHub = new EventHub();
}

void destroyEventHub()
{
    if(!g_eventHub)
        return;
    delete g_eventHub;
    g_eventHub = 0;
}

EventHub* EXPORT getEventHub()
{
    return g_eventHub;
}

}
