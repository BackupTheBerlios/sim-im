#include "clientcapability.h"

ClientCapability::ClientCapability(const QByteArray& id, int shortId, const QString& name, const QString& desc) : m_guid(id), m_shortId(shortId), m_name(name), m_description(desc)
{
}

QByteArray ClientCapability::guid() const
{
    return m_guid;
}

int ClientCapability::shortId() const
{
    return m_shortId;
}

QString ClientCapability::name() const
{
    return m_name;
}

QString ClientCapability::description() const
{
    return m_description;
}
