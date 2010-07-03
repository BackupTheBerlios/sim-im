
#include <vector>

#include "group.h"

#include "contacts.h"
#include "contact.h"

namespace SIM
{
    DataDef groupData[] =
    {
        { "Name", DATA_UTF, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

    Group::Group(unsigned long id, Buffer *cfg)
    {
        m_id = id;
        m_userData = UserData::create();
        userdata()->setValue("id", (uint)id);
    }

    Group::~Group()
    {
//        if (!getContacts()->p->m_bNoRemove){
//            Contact *contact;
//            ContactList::ContactIterator itc;
//            while ((contact = ++itc) != NULL){
//                if (contact->getGroup() != (int)id())
//                    continue;
//                contact->setGroup(0);
//                EventContact e(contact, EventContact::eChanged);
//                e.process();
//            }
//            EventGroup e(this, EventGroup::eDeleted);
//            e.process();
//        }
        getContacts()->removeGroup(id());

    }

    PropertyHubPtr Group::getUserData(const QString& id, bool bCreate)
    {
        PropertyHubPtr hub = m_userData->getUserData(id);
        if(!hub.isNull())
            return hub;
        if(bCreate)
            return m_userData->createUserData(id);
        return getContacts()->getUserData(id);
    }

    QString Group::getName()
    {
        if(id() == 0)
            return i18n("Not in list");
        return userdata()->value("Name").toString();
    }

    void Group::setName(const QString& name)
    {
        userdata()->setValue("Name", name);
    }

    ClientDataIterator Group::clientDataIterator(Client* client)
    {
        return ClientDataIterator(m_clientData, client);
    }

    QByteArray Group::saveUserData() const
    {
        return m_clientData.save();
    }

    void Group::loadUserData(Client *client, Buffer *cfg)
    {
        m_clientData.load(client, cfg);
    }

    IMContact* Group::createData(Client* client)
    {
        return m_clientData.createData(client);
    }

    IMContact* Group::getData(Client *client)
    {
        return m_clientData.getData(client);
    }

    IMContact* Group::getData(const QString& clientName)
    {
        return m_clientData.getData(clientName);
    }

    QStringList Group::clientNames()
    {
        return m_clientData.clientNames();
    }

    bool Group::have(IMContact* c)
    {
        return m_clientData.have(c);
    }

    void Group::sort()
    {
        m_clientData.sort();
    }

    void Group::join(Group* c)
    {
        m_clientData.join(c->m_clientData);
    }

    void Group::join(SIM::IMContact *cData, Group* c)
    {
        m_clientData.join(cData, c->m_clientData);
    }

    unsigned Group::size()
    {
        return m_clientData.size();
    }

    Client *Group::activeClient(void *&data, Client *client)
    {
        return m_clientData.activeClient(data, client);
    }

    void Group::freeData(SIM::IMContact* d)
    {
        m_clientData.freeData(d);
    }

    void Group::freeClientData(Client *client)
    {
        m_clientData.freeClientData(client);
    }
}

// vim: set expandtab:

