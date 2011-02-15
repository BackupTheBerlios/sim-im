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
