#ifndef _ICQ_EVENTS_H
#define _ICQ_EVENTS_H

#include "event.h"

struct SearchResult;

class EventSearchInternal : public SIM::Event
{
public:
    EventSearchInternal(SIM::SIMEvent e, SearchResult *res)
        : Event(e), m_res(res) {}

    SearchResult *searchResult() const { return m_res; }
protected:
    SearchResult *m_res;
};

class EventSearchDone : public EventSearchInternal
{
public:
    EventSearchDone(SearchResult *res)
        : EventSearchInternal(SIM::eEventICQSearchDone, res) {}
};

class EventSearch : public EventSearchInternal
{
public:
    EventSearch(SearchResult *res)
        : EventSearchInternal(SIM::eEventICQSearch, res) {}
};

#endif  // _ICQ_EVENTS_H
