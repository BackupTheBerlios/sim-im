#include "jabbercontact.h"
#include "buffer.h"
#include "jabberclient.h"

JabberContact::JabberContact(JabberClient* cl) : SIM::IMContact(), m_client(cl)
{
    m_jabberStatus = cl->getDefaultStatus("offline");
}


void JabberContact::deserialize(QDomElement& element)
{
    // TODO
}

void JabberContact::serialize(QDomElement& element)
{
    // TODO
}

void JabberContact::deserializeLine(const QString& key, const QString& value)
{

    QString val = value;
    if(val.startsWith('\"') && val.endsWith('\"'))
        val = val.mid(1, val.length() - 2);
    if(key == "ID") {
        setId(val);
    }
    else if(key == "Node") {
        setNode(val);
    }
    else if(key == "Resource") {
        setResource(val);
    }
    else if(key == "Name") {
        setName(val);
    }
    else if(key == "FirstName") {
        setFirstName(val);
    }
    else if(key == "Nick") {
        setNick(val);
    }
    else if(key == "Desc") {
        setDesc(val);
    }
    else if(key == "BirthDay") {
        setBirthday(val);
    }
    else if(key == "Url") {
        setUrl(val);
    }
    else if(key == "OrgName") {
        setOrgName(val);
    }
    else if(key == "OrgUnit") {
        setOrgUnit(val);
    }
    else if(key == "Title") {
        setTitle(val);
    }
    else if(key == "Role") {
        setRole(val);
    }
    else if(key == "Street") {
        setStreet(val);
    }
    else if(key == "ExtAddr") {
        setExtAddr(val);
    }
    else if(key == "City") {
        setCity(val);
    }
    else if(key == "Region") {
        setRegion(val);
    }
    else if(key == "PCode") {
        setPCode(val);
    }
    else if(key == "Country") {
        setCountry(val);
    }
    else if(key == "EMail") {
        setEmail(val);
    }
    else if(key == "Phone") {
        setPhone(val);
    }
    else if(key == "StatusTime") {
        setStatusTime(val.toUInt());
    }
    else if(key == "OnlineTime") {
        setOnlineTime(val.toUInt());
    }
    else if(key == "Subscribe") {
        setSubscribe(val.toUInt());
    }
    else if(key == "Group") {
        setGroup(val);
    }
    else if(key == "PhotoWidth") {
        setPhotoWidth(val.toUInt());
    }
    else if(key == "PhotoHeight") {
        setPhotoHeight(val.toUInt());
    }
    else if(key == "LogoWidth") {
        setLogoWidth(val.toUInt());
    }
    else if(key == "LogoHeight") {
        setLogoHeight(val.toUInt());
    }
    else if(key == "AutoReply") {
        setAutoReply(val);
    }
}

bool JabberContact::deserialize(const QString& data)
{
    QStringList strings = data.split("\n");
    foreach(const QString& line, strings)
    {
        if (line.isEmpty())
            break;
        QStringList keyval = line.split('=');
        if(keyval.size() < 2)
            continue;
        deserializeLine(keyval.at(0), keyval.at(1));
    }
    return true;
}

SIM::Client* JabberContact::client()
{
    return m_client;
}

SIM::IMStatusPtr JabberContact::status() const
{
    return m_jabberStatus.staticCast<SIM::IMStatus>();
}

QString JabberContact::name() const
{
    return getNick();
}

bool JabberContact::sendMessage(const SIM::MessagePtr& message)
{
    return true;
}

bool JabberContact::hasUnreadMessages()
{
    return false;
}

SIM::MessagePtr JabberContact::dequeueUnreadMessage()
{
    return SIM::MessagePtr();
}

void JabberContact::enqueueUnreadMessage(const SIM::MessagePtr& message)
{

}

SIM::IMGroupWeakPtr JabberContact::group()
{
    return SIM::IMGroupWeakPtr();
}

QString JabberContact::makeToolTipText()
{
    return QString();
}
