#ifndef ICQGROUP_H
#define ICQGROUP_H

#include "contacts/imgroup.h"
#include "contacts/client.h"
#include "icq_defines.h"

class ICQClient;
class ICQ_EXPORT ICQGroup : public SIM::IMGroup
{
public:
    ICQGroup(ICQClient* cl);
    virtual SIM::Client* client();
    virtual QList<SIM::IMContactPtr> contacts();

    virtual QString name();
    void setName(const QString& name);

    void setIcqId(int id);
    int icqId() const;

    void addContactId(int contactId);
    void deleteContactId(int contactId);
    bool hasContactId(int contactId);

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(const QString& data);

private:
    ICQClient* m_client;
    int m_icqId;
    QList<int> m_contactIds;
    QString m_name;
};
typedef QSharedPointer<ICQGroup> ICQGroupPtr;

#endif // ICQGROUP_H
