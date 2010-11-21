
#ifndef ICQSTATUS_H
#define ICQSTATUS_H


#include "contacts/protocol.h"
#include "contacts/imstatus.h"
#include "icq_defines.h"

class ICQ_EXPORT ICQStatus : public SIM::IMStatus
{
public:
    ICQStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QPixmap& icon);
    virtual ~ICQStatus();

    virtual QString id() const;
    virtual QString name() const;
    virtual bool hasText() const;
    virtual void setText(const QString& t);
    virtual QString text() const;
    virtual QPixmap icon() const;
    virtual int group() const;

    virtual bool flag(Flag fl) const;
    virtual void setFlag(Flag fl, bool val);

    virtual SIM::IMStatusPtr clone();

private:
    QString m_id;
    QString m_name;
    bool m_hasText;
    QString m_text;
    QPixmap m_icon;
    int m_group;
    int m_flags;
};

typedef QSharedPointer<ICQStatus> ICQStatusPtr;

#endif

// vim: set expandtab:

