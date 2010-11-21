
#include <map>
#include <QTextStream>
#include "contacts.h"
#include "userdata.h"

namespace SIM
{
    UserData::UserData()
    {
        m_root = PropertyHub::create();
    }

    UserData::~UserData()
    {
    }

    PropertyHubPtr UserData::getUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        DataMap::const_iterator it = m_data.find(id);
        if(it != m_data.end())
            return it.value();
        return PropertyHubPtr();
    }

    PropertyHubPtr UserData::createUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        PropertyHubPtr hub = getUserData(id);
        if(hub.isNull())
        {
            hub = PropertyHub::create(id);
            m_data.insert(id, hub);
        }
        return hub;
    }

    void UserData::destroyUserData(const QString& id)
    {
        DataMap::iterator it = m_data.find(id);
        if(it != m_data.end())
            m_data.erase(it);
    }

    PropertyHubPtr UserData::root()
    {
        return m_root;
    }

    bool UserData::serialize(QDomElement el)
    {
        QDomElement root = el.ownerDocument().createElement("userdata");
        el.appendChild(root);
        QDomElement roothub = el.ownerDocument().createElement("propertyhub");
        m_root->serialize(roothub);
        root.appendChild(roothub);
        foreach(PropertyHubPtr hub, m_data)
        {
            QDomElement hubelement = el.ownerDocument().createElement("propertyhub");
            hubelement.setAttribute("name", hub->getNamespace());
            hub->serialize(hubelement);
            root.appendChild(hubelement);
        }
        return true;
    }

    bool UserData::deserialize(QDomElement el)
    {
        QDomElement root = el.elementsByTagName("userdata").at(0).toElement();
        if(root.isNull())
            return false;
        QDomNodeList list = root.elementsByTagName("propertyhub");
        for(int i = 0; i < list.size(); i++)
        {
            QByteArray tmp_array;
            QTextStream stream(&tmp_array);
            QDomNode child = list.at(i);
            QDomElement el = child.firstChildElement("root");
            el.save(stream, 1);
            QString hubname = child.toElement().attribute("name");
            PropertyHubPtr hub;
            hub = createUserData(hubname);
            if(!hub->deserialize(el))
                return false;
        }
        return true;
    }

    UserDataPtr UserData::create()
    {
        return UserDataPtr(new UserData());
    }

    QStringList UserData::userDataIds() const
    {
        return m_data.keys();
    }

}

// vim: set expandtab:

