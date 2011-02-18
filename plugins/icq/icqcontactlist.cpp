 #include "icqcontactlist.h"

ICQContactList::ICQContactList(ICQClient* client) : m_client(client)
{
}

void ICQContactList::addContact(const ICQContactPtr& contact)
{
    m_contacts.insert(contact->getIcqID(), contact);
}

ICQContactPtr ICQContactList::contact(int icqContactId)
{
    return m_contacts.value(icqContactId);
}

ICQContactPtr ICQContactList::contactByScreen(const QString& screen)
{
    for(QMap<int, ICQContactPtr>::iterator it = m_contacts.begin(); it != m_contacts.end(); ++it)
    {
        if(it.value()->getScreen() == screen)
            return it.value();
    }
    return ICQContactPtr();
}

int ICQContactList::contactCount() const
{
    return m_contacts.count();
}

void ICQContactList::addGroup(const ICQGroupPtr& group)
{
    m_groups.insert(group->icqId(), group);
}

ICQGroupPtr ICQContactList::group(int icqGroupId)
{
    return m_groups.value(icqGroupId);
}

int ICQContactList::groupCount() const
{
    return m_groups.count();
}
