
#include <vector>
#include <list>
#include "client.h"
#include "group.h"
#include "misc.h"
#include "log.h"
#include "contacts/contact.h"
#include "contacts/protocol.h"
using namespace std;

namespace SIM
{
    Client::Client(Protocol* protocol) : m_protocol(protocol)
    {
    }

    Client::~Client()
    {
    }

    QString Client::cryptPassword(const QString& passwd)
    {
        if (passwd.length()) {
            QString new_passwd;
            unsigned short temp = 0x4345;
            for (int i = 0; i < (int)(passwd.length()); i++) {
                temp ^= (passwd[i].unicode());
                new_passwd += '$';
                new_passwd += QString::number(temp,16);
            }
            return new_passwd;
        }
        return QString();
    }

    QString Client::uncryptPassword(const QString& passwd)
    {
        QString pswd = passwd;
        if (pswd.length() && (pswd[0] == '$'))
        {
            pswd = pswd.mid(1);
            QString new_pswd;
            unsigned short temp = 0x4345;
            QString tmp;
            do
            {
                QString sub_str = getToken(pswd, '$');
                temp ^= sub_str.toUShort(0,16);
                new_pswd += tmp.setUtf16(&temp,1);
                temp = sub_str.toUShort(0,16);
            }
            while (pswd.length());
            return new_pswd;
        }
        return QString();
    }

    bool Client::serialize(QDomElement& element)
    {
        PropertyHubPtr hub = PropertyHub::create();
        hub->setValue("Password", cryptPassword(password()));
        return hub->serialize(element);
    }

    bool Client::deserialize(QDomElement& element)
    {
        PropertyHubPtr hub = PropertyHub::create();
        if(!hub->deserialize(element))
            return false;
        setPassword(uncryptPassword(hub->value("Password").toString()));
        return true;
    }

    QString Client::password() const
    {
        return m_password;
    }

    void Client::setPassword(const QString& password)
    {
        m_password = password;
    }
}

// vim: set expandtab:

