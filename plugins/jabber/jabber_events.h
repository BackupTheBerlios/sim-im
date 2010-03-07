#ifndef _JABBER_EVENTS_H
#define _JABBER_EVENTS_H

#include <QString>

#include "event.h"

struct ClientLastInfo;
struct ClientTimeInfo;
struct ClientVersionInfo;
struct DiscoItem;
struct JabberAgentInfo;
struct JabberSearchData;
struct agentRegisterInfo;
struct JabberUserData;

class EventAgentInfo : public SIM::Event
{
public:
    EventAgentInfo(JabberAgentInfo *info)
        : Event(SIM::eEventAgentInfo), m_info(info) {}

    JabberAgentInfo *agentInfo() const { return m_info; }
protected:
    JabberAgentInfo *m_info;
};

class EventAgentRegister : public SIM::Event
{
public:
    EventAgentRegister(agentRegisterInfo *info)
        : Event(SIM::eEventAgentRegister), m_info(info) {}

    agentRegisterInfo *registerInfo() const { return m_info; }
protected:
    agentRegisterInfo *m_info;
};

class EventSearch : public SIM::Event
{
public:
    EventSearch(JabberSearchData *data)
        : Event(SIM::eEventJabberSearch), m_data(data) {}

    JabberSearchData *searchData() const { return m_data; }
protected:
    JabberSearchData *m_data;
};

class EventSearchDone : public SIM::Event
{
public:
    EventSearchDone(const QString &userID)
        : Event(SIM::eEventJabberSearchDone), m_id(userID) {}

    const QString &userID() const { return m_id; }
protected:
    QString m_id;
};

class EventDiscoItem : public SIM::Event
{
public:
    EventDiscoItem(DiscoItem *item)
        : Event(SIM::eEventDiscoItem), m_item(item) {}

    DiscoItem *item() const { return m_item; }
protected:
    DiscoItem *m_item;
};

class EventVCard : public SIM::Event
{
public:
    EventVCard(JabberUserData *data)
        : Event(SIM::eEventVCard), m_data(data) {}

    JabberUserData *data() const { return m_data; }
protected:
    JabberUserData *m_data;
};

class EventClientVersion : public SIM::Event
{
public:
    EventClientVersion(ClientVersionInfo *info)
        : Event(SIM::eEventClientVersion), m_info(info) {}

    ClientVersionInfo *info() const { return m_info; }
protected:
    ClientVersionInfo *m_info;
};

class EventClientLastInfo : public SIM::Event
{
public:
    EventClientLastInfo(ClientLastInfo *info)
        : Event(SIM::eEventClientLastInfo), m_info(info) {}

    ClientLastInfo *info() const { return m_info; }
protected:
    ClientLastInfo *m_info;
};

class EventClientTimeInfo : public SIM::Event
{
public:
    EventClientTimeInfo(ClientTimeInfo *info)
        : Event(SIM::eEventClientTimeInfo), m_info(info) {}

    ClientTimeInfo *info() const { return m_info; }
protected:
    ClientTimeInfo *m_info;
};

#endif  // _JABBER_EVENTS_H
