#include <QDir>
#include <QStringList>
#include "standardcontactlist.h"
#include "profilemanager.h"
#include "contacts/client.h"
#include "clientmanager.h"
#include "log.h"

namespace SIM {

StandardContactList::StandardContactList()
{
}


void StandardContactList::clear()
{
}

bool StandardContactList::load()
{
    if(!load_new())
        return load_old();
    return true;
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
        m_userData->serialize(global);
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

QList<int> StandardContactList::contactIds() const
{
    QList<int> result;
    foreach(const ContactPtr& contact, m_contacts)
    {
        if(contact->flag(Contact::flIgnore))
            continue;
        result.append(contact->id());
    }
    return result;
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

ContactPtr StandardContactList::ownerContact()
{
    return m_owner;
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

bool StandardContactList::save_owner(QDomElement /*element*/)
{
    return true;
}

bool StandardContactList::save_groups(QDomElement element)
{
    if(m_groups.isEmpty())
        return false;

    for(QMap<int, GroupPtr>::iterator it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        QDomElement group =  element.ownerDocument().createElement("group");
        group.setAttribute("id", QString::number(it.value()->id()));
        if(it.value()->serialize(group))
            element.appendChild(group);
    }
    return true;
}

bool StandardContactList::save_contacts(QDomElement element)
{
    if(m_contacts.isEmpty())
        return false;

    for(QMap<int, ContactPtr>::iterator it = m_contacts.begin(); it != m_contacts.end(); ++it)
    {
        QDomElement contact =  element.ownerDocument().createElement("contact");
        contact.setAttribute("id", QString::number(it.value()->id()));
        if(it.value()->serialize(contact))
            element.appendChild(contact);
    }
    return true;
}

bool StandardContactList::load_new()
{
    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.xml";
    QFile f(cfgName);
    if(!f.open(QIODevice::ReadOnly))
        return false;

    QDomDocument doc;
    doc.setContent(f.readAll());
    QDomElement el = doc.elementsByTagName("global").at(0).toElement();
    if(!m_userData->deserialize(el))
        return false;

    QDomElement owner = doc.elementsByTagName("owner").at(0).toElement();
    load_owner(owner);

    QDomElement groups = doc.elementsByTagName("groups").at(0).toElement();
    if(!load_groups(groups))
        return false;


    QDomElement contacts = doc.elementsByTagName("contacts").at(0).toElement();
    if(!load_contacts(contacts))
        return false;


    return true;
}

bool StandardContactList::load_owner(const QDomElement& /*owner*/)
{
    return true;
}

bool StandardContactList::load_groups(const QDomElement& groups)
{
    QDomNodeList list = groups.elementsByTagName("group");
    for(int i = 0; i < list.size(); i++)
    {
        QDomElement el = list.at(i).toElement();
        int id = el.attribute("id").toInt();
        GroupPtr gr = createGroup(id);
        if(!gr->deserialize(el))
            return false;
        addGroup(gr);
    }
    return true;
}

bool StandardContactList::load_contacts(const QDomElement& contacts)
{
    QDomNodeList list = contacts.elementsByTagName("contact");
    for(int i = 0; i < list.size(); i++) {
        QDomElement el = list.at(i).toElement();
        int id = el.attribute("id").toInt();
        ContactPtr c = createContact(id);
        if(!c->deserialize(el))
            return false;
        addContact(c);
    }
    return true;
}

bool StandardContactList::load_old()
{
    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.conf";
    QFile f(cfgName);
    if (!f.open(QIODevice::ReadOnly)){
        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
        return false;
    }

    ParserState state;
    while(!f.atEnd())
    {
        QString line = QString::fromLocal8Bit(f.readLine());
        line = line.trimmed();
        //log(L_DEBUG, "Line: %s", qPrintable(line));
        if(line.startsWith("[Group="))
        {
            state.nextSection = ParserState::Group;
            load_old_dispatch(state);
            resetState(state);
            state.groupId = line.mid(7, line.length() - 8).toInt();
            if(state.groupId > 0)
                addGroup(createGroup(state.groupId));
        }
        else if(line.startsWith("[Contact="))
        {
            state.nextSection = ParserState::Contact;
            load_old_dispatch(state);
            resetState(state);
            state.contactId = line.mid(9, line.length() - 10).toInt();
            if(state.contactId > 0)
                addContact(createContact(state.contactId));
        }
        else if(line.startsWith("["))
        {
            state.nextSection = ParserState::Client;
            load_old_dispatch(state);
            state.dataname = line.mid(1, line.length() - 2);
        }
        else
        {
            state.data.append(line + "\n");
        }
    }
    return true;
}

bool StandardContactList::load_old_dispatch(ParserState& state)
{
    if(state.contactId != -1)
    {
        ContactPtr c = contact(state.contactId);
        if(!c)
            return false;
        if(state.dataname.isEmpty())
        {
            c->deserialize(state.data);
        }
        else
        {
            if(state.dataname.indexOf('.') >= 0)
            {
                ClientPtr client = getClientManager()->client(state.dataname);
                if(!client)
                    return false;
                IMContactPtr imcontact = client->createIMContact();
                imcontact->deserialize(state.data);
                c->addClientContact(imcontact);
                if(c->name().isEmpty())
                    c->setName(imcontact->name());
            }
            else if(!state.dataname.isEmpty())
            {
                deserializeLines(c->userdata(), state.dataname, state.data);
            }
            else
            {
                c->deserialize(state.data);
            }
        }
    }
    else if(state.groupId != -1)
    {
        GroupPtr gr = group(state.groupId);
        if(!gr)
            return false;
        if(state.dataname.isEmpty())
        {
            gr->deserialize(state.data);
        }
        else
        {
            if(state.dataname.indexOf('.') >= 0)
            {
                ClientPtr client = getClientManager()->client(state.dataname);
                if(!client)
                    return false;
                IMGroupPtr imgroup = client->createIMGroup();
                imgroup->deserialize(state.data);
                gr->addClientGroup(imgroup);
            }
            else
            {
                deserializeLines(gr->userdata(), state.dataname, state.data);
            }
        }
    }

    return true;
}

void StandardContactList::resetState(ParserState& state)
{
    state.contactId = -1;
    state.groupId = -1;
    state.dataname = QString();
    state.data.clear();
}

bool StandardContactList::deserializeLines(const UserDataPtr& ud, const QString& dataname, const QString& data)
{
    PropertyHubPtr hub = ud->getUserData(dataname);
    if(!hub)
        hub = ud->createUserData(dataname);
    QStringList list = data.split('\n');
    foreach(const QString& s, list)
    {
        QStringList keyval = s.split('=');
        if(keyval.size() != 2)
            continue;
        if(keyval.at(1).startsWith('\"') && keyval.at(1).endsWith('\"') )
            hub->setValue(keyval.at(0), keyval.at(1).mid(1, keyval.at(1).size() - 2));
        else
            hub->setValue(keyval.at(0), keyval.at(1));
    }

    return true;
}

} // namespace SIM
