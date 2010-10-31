#include "jabbergroup.h"
#include "jabberclient.h"

JabberGroup::JabberGroup(JabberClient* cl) : SIM::IMGroup(), m_client(cl)
{
}

SIM::Client* JabberGroup::client()
{
    return m_client;
}

QList<SIM::IMContactPtr> JabberGroup::contacts()
{
    return QList<SIM::IMContactPtr>();
}

bool JabberGroup::serialize(QDomElement& element)
{
    return true;
}

bool JabberGroup::deserialize(QDomElement& element)
{
    return true;
}

bool JabberGroup::deserialize(const QString& data)
{
    return true;
}
