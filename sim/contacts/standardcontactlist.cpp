#include <QDir>
#include "standardcontactlist.h"
#include "profilemanager.h"
#include "contacts/client.h"

namespace SIM {

StandardContactList::StandardContactList()
{
}


void StandardContactList::clear()
{
}

bool StandardContactList::load()
{
    return load_new();
}

bool StandardContactList::save()
{
    if(!ProfileManager::instance()->profilePath().isEmpty())
    {
        QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.xml";
        ProfileManager::instance()->sync();
        QDomDocument doc;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
        QDomElement root = doc.createElement("contactlist");
        QDomElement global = doc.createElement("global");
        getUserData()->serialize(global);
        root.appendChild(global);

        QDomElement owner = doc.createElement("owner");
        save_owner(owner);
        root.appendChild(owner);

        QDomElement groups = doc.createElement("groups");
        if(save_groups(groups))
            root.appendChild(groups);
        QDomElement contacts = doc.createElement("contacts");
        if(save_contacts(contacts))
            root.appendChild(contacts);
        doc.appendChild(root);
        QFile f(cfgName);
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(doc.toByteArray());
        f.close();
    }
    return true;
}

bool StandardContactList::addContact(const ContactPtr& newContact)
{
    ContactPtr c = contact(newContact->id());
    if(c)
        return false;
    m_contacts.insert(newContact->id(), newContact);
    return true;
}

ContactPtr StandardContactList::contact(int id) const
{
    QMap<int, ContactPtr>::const_iterator it = m_contacts.find(id);
    if(it == m_contacts.end())
        return ContactPtr();
    return it.value();
}

bool StandardContactList::contactExists(int id) const
{
    ContactPtr c = contact(id);
    if(c)
        return true;
    return false;
}

void StandardContactList::removeContact(int id)
{
    QMap<int, ContactPtr>::iterator it = m_contacts.find(id);
    if(it != m_contacts.end())
        m_contacts.erase(it);
}

ContactPtr StandardContactList::createContact(int id)
{
    return ContactPtr(new Contact(id));
}

bool StandardContactList::addGroup(const GroupPtr& newGroup)
{
    GroupPtr g = group(newGroup->id());
    if(g)
        return false;
    m_groups.insert(newGroup->id(), newGroup);
    return true;
}

GroupPtr StandardContactList::group(int id) const
{
    QMap<int, GroupPtr>::const_iterator it = m_groups.find(id);
    if(it == m_groups.end())
        return GroupPtr();
    return it.value();
}

void StandardContactList::removeGroup(int id)
{
    QMap<int, GroupPtr>::iterator it = m_groups.find(id);
    if(it != m_groups.end())
        m_groups.erase(it);
}

GroupPtr StandardContactList::createGroup(int id)
{
    return GroupPtr(new Group(id));
}

QList<GroupPtr> StandardContactList::allGroups() const
{
    return m_groups.values();
}

QList<ContactPtr> StandardContactList::contactsForGroup(int groupId)
{
    QList<ContactPtr> result;
    foreach(const ContactPtr& contact, m_contacts)
    {
        if(contact->groupId() == groupId)
            result.append(contact);
    }

    return result;
}

void StandardContactList::incomingMessage(const MessagePtr& message)
{
    Q_UNUSED(message);
}

UserDataPtr StandardContactList::userdata() const
{
    return UserDataPtr();
}

void StandardContactList::save_new()
{
}

bool StandardContactList::save_owner(QDomElement element)
{
    return true;
}

bool StandardContactList::save_groups(QDomElement element)
{
    return true;
}

bool StandardContactList::save_contacts(QDomElement element)
{
    return true;
}

bool StandardContactList::load_new()
{
    return true;
}

bool StandardContactList::load_owner(const QDomElement& owner)
{
    return true;
}

bool StandardContactList::load_groups(const QDomElement& groups)
{
    return true;
}

bool StandardContactList::load_contacts(const QDomElement& contacts)
{
    return true;
}

bool StandardContactList::load_old()
{
    return true;
}

} // namespace SIM
