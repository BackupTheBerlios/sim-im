#include "icqgroup.h"

ICQGroup::ICQGroup(const SIM::ClientPtr& cl) : m_client(cl)
{
}

SIM::ClientWeakPtr ICQGroup::client()
{
    return m_client;
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
