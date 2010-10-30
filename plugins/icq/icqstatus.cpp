
#include "icqstatus.h"

ICQStatus::ICQStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QPixmap& icon) : IMStatus(),
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

QPixmap ICQStatus::icon() const
{
    return m_icon;
}

int ICQStatus::group() const
{
    return m_group;
}

SIM::IMStatusPtr ICQStatus::clone()
{
    ICQStatus* status = new ICQStatus(m_id, m_name, m_hasText, m_text, m_icon);
    status->setFlag(flOffline, flag(flOffline));
    status->setFlag(flInvisible, flag(flInvisible));
    return SIM::IMStatusPtr(status);
}

bool ICQStatus::hasText() const
{
    return m_hasText;
}


bool ICQStatus::flag(Flag fl) const
{
    return (m_flags & (1 << fl)) == (1 << fl);
}

void ICQStatus::setFlag(Flag fl, bool val)
{
    if(val)
        m_flags |= (1 << fl);
    else
        m_flags &= ~(1 << fl);
}


// vim: set expandtab:

