
#include "jabberstatus.h"


JabberStatus::JabberStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QPixmap& icon,
                           const QString& show, const QString& type) : IMStatus(),
    m_id(id),
    m_name(name),
    m_hasText(hasText),
    m_text(defaultText),
    m_icon(icon),
    m_show(show),
    m_type(type)
{
}

JabberStatus::~JabberStatus()
{
}


QString JabberStatus::id() const
{
    return m_id;
}

QString JabberStatus::name() const
{
    return m_name;
}

void JabberStatus::setText(const QString& t)
{
    m_text = t;
}

QString JabberStatus::text() const
{
    return m_text;
}

QPixmap JabberStatus::icon() const
{
    return m_icon;
}

SIM::IMStatusPtr JabberStatus::clone()
{
    JabberStatus* status = new JabberStatus(m_id, m_name, m_hasText, m_text, m_icon, m_show, m_type);
    status->setFlag(flOffline, flag(flOffline));
    status->setFlag(flInvisible, flag(flInvisible));
    return SIM::IMStatusPtr(status);
}

bool JabberStatus::hasText() const
{
    return m_hasText;
}

QString JabberStatus::show()
{
    return m_show;
}

QString JabberStatus::type()
{
    return m_type;
}

bool JabberStatus::flag(Flag fl) const
{
    return (m_flags & (1 << fl)) == (1 << fl);
}

void JabberStatus::setFlag(Flag fl, bool val)
{
    if(val)
        m_flags |= (1 << fl);
    else
        m_flags &= ~(1 << fl);
}

// vim: set expandtab:

