
#include <vector>

#include "group.h"

#include "contacts/client.h"
#include "contacts/contactlist.h"
#include "contact.h"
#include "clientmanager.h"

namespace SIM
{
    Group::Group(int id) : m_id(id)
    {
        m_userData = UserData::create();
    }

    Group::~Group()
    {
    }

    QString Group::name() const
    {
        return m_name;
    }

    void Group::setName(const QString& name)
    {
        m_name = name;
    }

    void Group::addClientGroup(const IMGroupPtr& group)
    {
        m_imGroups.append(group);
    }

    IMGroupPtr Group::clientGroup(const QString& clientId) const
    {
        foreach(const IMGroupPtr& group, m_imGroups)
        {
            Client* client = group->client();
            if(!client)
                continue;
            if(client->name() == clientId)
                return group;
        }
        return IMGroupPtr();
    }

    QStringList Group::clientIds() const
    {
        QStringList result;
        foreach(const IMGroupPtr& group, m_imGroups)
        {
            Client* client = group->client();
            if(!client)
                continue;
            result.append(client->name());
        }
        return result;
    }

    bool Group::flag(Flag fl) const
    {
        return m_flags.at(fl);
    }

    void Group::setFlag(Flag fl, bool value)
    {
        m_flags.setBit(fl, value);
    }

    bool Group::serialize(QDomElement& element)
    {
        userdata()->serialize(element);
        QStringList clients = clientIds();
        QDomElement maininfo = element.ownerDocument().createElement("main");
        serializeMainInfo(maininfo);
        element.appendChild(maininfo);
        foreach(const QString& clname, clients) {
            IMGroupPtr imgr = clientGroup(clname);
            QDomElement clientElement = element.ownerDocument().createElement("clientdata");
            Client* client = imgr->client();
            clientElement.setAttribute("clientname", client->name());
            imgr->serialize(clientElement);
            element.appendChild(clientElement);
        }
        return true;
    }

    bool Group::deserialize(QDomElement& element)
    {
        userdata()->deserialize(element);
        QDomElement main = element.elementsByTagName("main").at(0).toElement();
        if(!main.isNull())
        {
            deserializeMainInfo(main);
        }
        QDomNodeList cldatalist = element.elementsByTagName("clientdata");
        for(int j = 0; j < cldatalist.size(); j++) {
            QDomElement clientElement = cldatalist.at(j).toElement();
            ClientPtr client = getClientManager()->client(clientElement.attribute("clientname"));
            if(!client)
                continue;
            IMGroupPtr imgr = clientGroup(client->name());
            if(!imgr)
                imgr = client->createIMGroup();
            imgr->deserialize(clientElement);
        }
        return true;
    }

    bool Group::serializeMainInfo(QDomElement& element)
    {
        PropertyHubPtr hub = PropertyHub::create();
        hub->setValue("Name", name());
        return hub->serialize(element);
    }

    bool Group::deserializeMainInfo(const QDomElement& element)
    {
        PropertyHubPtr hub = PropertyHub::create();
        if(!hub->deserialize(element))
            return false;
        setName(hub->value("Name").toString());
        return true;
    }

    bool Group::deserialize(const QString& /*data*/)
    {
        return true;
    }
}

// vim: set expandtab:

