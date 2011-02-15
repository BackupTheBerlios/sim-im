#ifndef ICQCONTACTLIST_H
#define ICQCONTACTLIST_H

#include <QMap>

#include "icqcontact.h"
#include "icqgroup.h"
#include "icq_defines.h"

class ICQClient;

class ICQ_EXPORT ICQContactList
{
public:
    ICQContactList(ICQClient* client);

    void addContact(const ICQContactPtr& contact);
    ICQContactPtr contact(int icqContactId);
    int contactCount() const;

    void addGroup(const ICQGroupPtr& group);
    ICQGroupPtr group(int icqGroupId);
    int groupCount() const;

private:
    ICQClient* m_client;
    QMap<int, ICQContactPtr> m_contacts;
    QMap<int, ICQGroupPtr> m_groups;
};

#endif // ICQCONTACTLIST_H
