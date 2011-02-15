#include "icqgroup.h"
#include "icqclient.h"

ICQGroup::ICQGroup(ICQClient* cl) : m_client(cl)
{
}

SIM::Client* ICQGroup::client()
{
    return m_client;
}

QString ICQGroup::name()
{
    return m_name;
}

void ICQGroup::setName(const QString& name)
{
    m_name = name;
}

QList<SIM::IMContactPtr> ICQGroup::contacts()
{
    return QList<SIM::IMContactPtr>();
}

bool ICQGroup::serialize(QDomElement& element)
{
    return true;
}

bool ICQGroup::deserialize(QDomElement& element)
{
    return true;
}

bool ICQGroup::deserialize(const QString& data)
{
    return true;
}

void ICQGroup::setIcqId(int id)
{
    m_icqId = id;
}

int ICQGroup::icqId() const
{
    return m_icqId;
}

void ICQGroup::addContactId(int contactId)
{
    if(hasContactId(contactId))
        return;
    m_contactIds.append(contactId);
}

void ICQGroup::deleteContactId(int contactId)
{
    m_contactIds.removeAll(contactId);
}

bool ICQGroup::hasContactId(int contactId)
{
    return m_contactIds.contains(contactId);
}
