
#ifndef JABBERSTATUS_H
#define JABBERSTATUS_H

#include "contacts/imstatus.h"

class JabberStatus : public SIM::IMStatus
{
public:
    JabberStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QIcon& icon,
                 const QString& show, const QString& type);
    virtual ~JabberStatus();

    virtual QString id() const;
    virtual QString name() const;
    virtual bool hasText() const;
    virtual void setText(const QString& t);
    virtual QString text() const;
    virtual QIcon icon() const;

    virtual QStringList substatuses();
    virtual SIM::IMStatusPtr substatus(const QString& id);
    virtual SIM::IMStatusPtr clone();

    QString show();
    QString type();

private:
    QString m_id;
    QString m_name;
    bool m_hasText;
    QString m_text;
    QIcon m_icon;
    QString m_show;
    QString m_type;

};

typedef QSharedPointer<JabberStatus> JabberStatusPtr;

#endif

// vim: set expandtab:

