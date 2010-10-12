#ifndef ICQGROUP_H
#define ICQGROUP_H

#include "contacts/imgroup.h"
#include "contacts/client.h"

class ICQGroup : public SIM::IMGroup
{
public:
    ICQGroup(const SIM::ClientPtr& cl);
    virtual SIM::ClientWeakPtr client();
    virtual QList<SIM::IMContactPtr> contacts();

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(const QString& data);

private:
    SIM::ClientWeakPtr m_client;
};

#endif // ICQGROUP_H
