
#ifndef JABBERSTATUS_H
#define JABBERSTATUS_H

#include "contacts/imstatus.h"
#include "jabber_api.h"

class JABBER_EXPORT JabberStatus : public SIM::IMStatus
{
public:
    JabberStatus(const QString& id, const QString& name, bool hasText, const QString& defaultText, const QPixmap& icon,
                 const QString& show, const QString& type);
    virtual ~JabberStatus();

    virtual QString id() const;
    virtual QString name() const;
    virtual bool hasText() const;
    virtual void setText(const QString& t);
    virtual QString text() const;
    virtual QPixmap icon() const;

    virtual bool flag(Flag fl) const;
    virtual void setFlag(Flag fl, bool val);

    virtual SIM::IMStatusPtr clone();

    QString show();
    QString type();

private:
    QString m_id;
    QString m_name;
    bool m_hasText;
    QString m_text;
    QPixmap m_icon;
    QString m_show;
    QString m_type;
    int m_flags;
};

typedef QSharedPointer<JabberStatus> JabberStatusPtr;

#endif

// vim: set expandtab:

