
#include <vector>
#include <list>
#include "client.h"
#include "group.h"
#include "misc.h"
#include "log.h"
#include "contacts/contact.h"
using namespace std;

namespace SIM
{
    Client::Client(Protocol* protocol) : m_ulStatus(1),
        m_commonStatus(true),
        m_invisible(false),
        m_savePassword(true)
    {
        m_protocol = protocol;
        m_status = STATUS_OFFLINE;
        m_state  = Offline;
        //m_currentStatus = IMStatusPtr(new InvalidStatus());
    }

    SIM_DEPRECATED Client::Client(Protocol *protocol, Buffer *cfg)
    {
        m_currentStatus = IMStatusPtr(/*new InvalidStatus()*/);
        m_data = PropertyHub::create();

        //now uncrypt password somehow
        QString pswd = getPassword();
        setPassword(uncryptPassword(pswd));

        m_protocol = protocol;
        m_status = STATUS_OFFLINE;
        m_state  = Offline;
    }

    SIM_DEPRECATED void Client::setStatus(unsigned status, bool bCommon)
    {
        setManualStatus(status);
        setCommonStatus(bCommon);
        EventClientChanged(this).process();
    }

    Client::~Client()
    {
        freeData();
    }

    void Client::changeStatus(const IMStatusPtr& status)
    {
        m_currentStatus = status;
        EventClientChanged(this).process();
    }

    IMStatusPtr Client::currentStatus()
    {
        return m_currentStatus;
    }

    void Client::setManualStatus(const IMStatusPtr& status)
    {
        m_manualStatus = status;
    }

    IMStatusPtr Client::manualStatus()
    {
        return m_manualStatus;
    }

    QString Client::resources(void*)
    {
        return QString::null;
    }

    QString Client::contactName(void*)
    {
        return i18n(protocol()->description()->text);
    }

    CommandDef *Client::infoWindows(Contact*, void*)
    {
        return NULL;
    }

    QWidget *Client::infoWindow(QWidget*, Contact*, void*, unsigned)
    {
        return NULL;
    }

    CommandDef *Client::configWindows()
    {
        return NULL;
    }

    QWidget *Client::configWindow(QWidget*, unsigned)
    {
        return NULL;
    }

    QString Client::contactTip(void*)
    {
        return QString::null;
    }

    void Client::updateInfo(Contact *contact, void *data)
    {
        if(data)
        {
            EventContact(contact, EventContact::eFetchInfoFailed).process();
        }
        else
        {
            EventClientChanged(this).process();
        }
    }

    void Client::freeData()
    {
        getContacts()->removeClient(this);
//        Group *grp;
//        ContactList::GroupIterator itg;
//        while ((grp = ++itg) != NULL)
//        {
//            if (grp->clientData.size() == 0)
//                continue;
//            grp->clientData.freeClientData(this);
//        }

        Contact *contact;
        ContactList::ContactIterator itc;
        while ((contact = ++itc) != NULL)
        {
            if (contact->size() == 0)
                continue;
            contact->freeClientData(this);
            if (contact->size())
            {
                getContacts()->removeContact(contact->id());
            }
        }
		//free_data(_clientData, &data);
    }

    QByteArray Client::getConfig()
    {
        QString real_pswd = getPassword();
        QString pswd = getPassword();

        setPassword(cryptPassword(pswd));
        QString prev = getPreviousPassword();
        if (!prev.isEmpty())
            setPassword(prev);
        if (!getSavePassword())
            setPassword(NULL);
//        QByteArray res = save_data(_clientData, &data);
        setPassword(real_pswd);
        return QByteArray();
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
            return(new_passwd);
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
        hub->setValue("ManualStatus", (unsigned int)getManualStatus());
        hub->setValue("CommonStatus", getCommonStatus());
        hub->setValue("Password", cryptPassword(getPassword()));
        hub->setValue("SavePassword", getSavePassword());
        hub->setValue("PreviousPassword", getPreviousPassword());
        hub->setValue("LastSend", m_lastSend);
        hub->setValue("Invisible", getInvisible());
        return hub->serialize(element);
    }

    bool Client::deserialize(QDomElement& element)
    {
        PropertyHubPtr hub = PropertyHub::create();
        if(!hub->deserialize(element))
            return false;
        setManualStatus(hub->value("ManualStatus").toUInt());
        setCommonStatus(hub->value("CommonStatus").toBool());
        setPassword(uncryptPassword(hub->value("Password").toString()));
        setSavePassword(hub->value("SavePassword").toBool());
        setPreviousPassword(hub->value("PreviousPassword").toString());
        m_lastSend = (hub->value("LastSend").toStringList());
        setInvisible(hub->value("Invisible").toBool());
        return true;
    }

    void Client::deserializeLine(const QString& key, const QString& value)
    {
        QString val = value;
        if(val.startsWith('\"') && val.endsWith('\"'))
            val = val.mid(1, val.length() - 2);
        //log(L_DEBUG, "Client::deserializeLine(): %s=%s", qPrintable(key), qPrintable(val));
        if(key == "ManualStatus") {
            setManualStatus(val.toUInt());
        }
        else if(key == "CommonStatus") {
            setCommonStatus(val == "true");
        }
        else if(key == "Password") {
            setPassword(uncryptPassword(val));
        }
        else if(key == "SavePassword") {
            setSavePassword(val == "true");
        }
        else if(key == "PreviousPassword") {
            setPreviousPassword(val);
        }
        else if(key == "Invisible") {
            setInvisible(val == "true");
        }
    }

    bool Client::deserialize(Buffer* cfg)
    {
        cfg->setWritePos(cfg->size() - 1);
        cfg->setReadPos(0);
        while(1) {
            const QString line = QString::fromUtf8(cfg->getLine());
            if (line.isEmpty())
                break;
            QStringList keyval = line.split('=');
            if(keyval.size() < 2)
                continue;
            deserializeLine(keyval.at(0), keyval.at(1));
        }
        return true;
    }

    void Client::setClientInfo(IMContact*)
    {
    }

    bool Client::compareData(void*, void*)
    {
        return false;
    }

    void Client::setState(State state, const QString &text, unsigned code)
    {
        m_state = state;
        EventClientChanged(this).process();
        if (state == Error){
            EventNotification::ClientNotificationData d;
            d.client  = this;
            d.text = text;
            d.code    = code;
            d.args    = QString::null;
            d.flags   = EventNotification::ClientNotificationData::E_ERROR;
            d.options = QString::null;
            d.id      = 0;
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                if (getContacts()->getClient(i) == this){
                    d.id = i + 1;
                    break;
                }
            }
            EventClientNotification e(d);
            e.process();
        }
    }

    void Client::contactsLoaded()
    {
    }

}

// vim: set expandtab:

