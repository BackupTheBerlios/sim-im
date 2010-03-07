
#ifndef ICQSTATUS_H
#define ICQSTATUS_H

#include "contacts/imstatus.h"

class ICQStatus : public SIM::IMStatus
{
public:
    ICQStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QIcon& icon);
    virtual ~ICQStatus();

    virtual QString id() const;
    virtual QString name() const;
    virtual bool hasText() const;
    virtual void setText(const QString& t);
    virtual QString text() const;
    virtual QIcon icon() const;

    virtual QStringList substatuses();
    virtual SIM::IMStatusPtr substatus(const QString& id);
    virtual SIM::IMStatusPtr clone();


private:
    QString m_id;
    QString m_name;
    bool m_hasText;
    QString m_text;
    QIcon m_icon;
};

typedef QSharedPointer<ICQStatus> ICQStatusPtr;

#endif

// vim: set expandtab:

