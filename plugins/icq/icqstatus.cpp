
#include "icqstatus.h"

ICQStatus::ICQStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QIcon& icon) : IMStatus(),
    m_id(id),
    m_name(name),
    m_hasText(hasText),
    m_text(defaultText),
    m_icon(icon)
{
}

ICQStatus::~ICQStatus()
{
}

QString ICQStatus::id() const
{
    return m_id;
}

QString ICQStatus::name() const
{
    return m_name;
}

void ICQStatus::setText(const QString& t)
{
    m_text = t;
}

QString ICQStatus::text() const
{
    return m_text;
}

QIcon ICQStatus::icon() const
{
    return m_icon;
}

QStringList ICQStatus::substatuses()
{
    return QStringList();
}

SIM::IMStatusPtr ICQStatus::substatus(const QString& id)
{
    return SIM::IMStatusPtr();
}

SIM::IMStatusPtr ICQStatus::clone()
{
    return SIM::IMStatusPtr(new ICQStatus(m_id, m_name, m_hasText, m_text, m_icon));
}

bool ICQStatus::hasText() const
{
    return m_hasText;
}

// vim: set expandtab:

