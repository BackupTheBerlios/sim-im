
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "contact.h"
#include "contacts.h"
#include "clientdataiterator.h"
#include "group.h"
#include "client.h"
#include "log.h"
#include "unquot.h"

namespace SIM
{

    DataDef contactData[] =
    {
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

    Contact::Contact(unsigned long id, Buffer * /*cfg*/)
        : m_id(id)
    {
        m_userData = UserData::create();
        getUserData()->root()->setValue("id", (uint)id);
    }

    Contact::~Contact()
    {
        if(!getContacts())
            return;
        getContacts()->removeContact(m_id);
    }

    const DataDef *Contact::dataDef()
    {
        return contactData;
    }

    PropertyHubPtr Contact::getUserData(const QString& id, bool bCreate)
    {
        PropertyHubPtr data = m_userData->getUserData(id);
        if(data)
            return data;
        if(bCreate)
            return m_userData->createUserData(id);
        Group *group = getContacts()->group(getGroup());
        if (group)
            return group->getUserData(id, false);
        return getContacts()->getUserData(id);
    }

    void Contact::setup()
    {
        QString str = getFirstName();
        getToken(str, '/');
        if (str != "-")
            setFirstName(QString());
        str = getLastName();
        getToken(str, '/');
        if(!str.contains('-'))
            setLastName(QString());
        QString res;
        str = getEMails();
        while (!str.isEmpty()){
            QString item = getToken(str, ';', false);
            QString value = getToken(item, '/', false);
            if (item != "-")
                continue;
            if (!res.isEmpty())
                res += ';';
            res += value;
            res += "/-";
        }
        setEMails(res);
        str = getPhones();
        while (!str.isEmpty()){
            QString item = getToken(str, ';', false);
            QString value = getToken(item, '/', false);
            if (item != "-")
                continue;
            if (!res.isEmpty())
                res += ';';
            res += value;
            res += "/-";
        }
        setPhones(res);
        ClientDataIterator it =  clientDataIterator();
        void *data;
        while ((data = ++it) != NULL)
            it.client()->setupContact(this, data);
    }

    struct sortClientData
    {
        void		*data;
        Client		*client;
        unsigned	nClient;
    };

    static bool cmp_sd(sortClientData p1, sortClientData p2)
    {
        if (((clientData*)(p1.data))->LastSend.asULong() > ((clientData*)(p2.data))->LastSend.asULong())
            return true;
        if (((clientData*)(p1.data))->LastSend.asULong() < ((clientData*)(p2.data))->LastSend.asULong())
            return false;
        return p1.nClient < p2.nClient;
    }

    unsigned long Contact::contactInfo(unsigned &style, QString &statusIcon, QSet<QString> *icons)
    {
        style = 0;
        statusIcon.clear();
        if (icons)
            icons->clear();
        unsigned long status = STATUS_UNKNOWN;
        void *data;
        ClientDataIterator it = clientDataIterator();
        std::vector<sortClientData> d;
        while ((data = ++it) != NULL){
            sortClientData sd;
            sd.data    = data;
            sd.client  = it.client();
            sd.nClient = 0;
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                if (getContacts()->getClient(i) == sd.client){
                    sd.nClient = i;
                    break;
                }
            }
            d.push_back(sd);
        }
        std::sort(d.begin(), d.end(), cmp_sd);
        for (unsigned i = 0; i < d.size(); i++){
            void *data = d[i].data;
            Client *client = m_clientData.activeClient(data, d[i].client);
            if (client == NULL)
                continue;
            client->contactInfo(data, status, style, statusIcon, icons);
        }
        QString phones = getPhones();
        bool bCell  = false;
        bool bPager = false;
        while (phones.length()){
            QString phoneItem = getToken(phones, ';', false);
            phoneItem = getToken(phoneItem, '/', false);
            getToken(phoneItem, ',');
            getToken(phoneItem, ',');
            unsigned n = phoneItem.toULong();
            if (n == CELLULAR)
                bCell = true;
            if (n == PAGER)
                bPager = true;
        }
        if (bCell){
            if(!statusIcon.isEmpty()){
                if(icons){
                    icons->insert("cell");
                }
            }else{
                statusIcon = "cell";
            }
        }
        if(bPager){
            if(!statusIcon.isEmpty()){
                if(icons){
                    icons->insert("pager");
                }
            }else{
                statusIcon = "pager";
            }
        }
        if (status == STATUS_UNKNOWN){
            if (statusIcon.isEmpty()){
                QString mails = getEMails();
                if (!mails.isEmpty())
                    statusIcon = "mail_generic";
            }
            if (statusIcon.isEmpty())
                statusIcon = "nonim";
            return STATUS_UNKNOWN;
        }
        if (statusIcon.isEmpty())
            statusIcon = "empty";
        return status;
    }

    static char tipDiv[] = "<br>__________<br>";

    QString Contact::tipText()
    {
        QString tip;
        tip += "<b>";
        tip += quoteString(getName());
        tip += "</b>";
        QString firstName = getFirstName();
        firstName = getToken(firstName, '/');
        firstName = quoteString(firstName);
        QString lastName = getLastName();
        lastName = getToken(lastName, '/');
        lastName = quoteString(lastName);
        if (firstName.length() || lastName.length()){
            tip += "<br>";
            if (firstName.length()){
                tip += firstName;
                tip += ' ';
            }
            tip += lastName;
        }
        bool bFirst = true;
        QString mails = getEMails();
        while (mails.length()){
            QString mail = getToken(mails, ';', false);
            mail = getToken(mail, '/');
            if (bFirst){
                tip += "<br>";
                bFirst = false;
            }else{
                tip += ", ";
            }
            tip += quoteString(mail);
        }
        void *data;
        ClientDataIterator it(m_clientData);
        while ((data = ++it) != NULL){
            Client *client = m_clientData.activeClient(data, it.client());
            if (client == NULL)
                continue;
            QString str = client->contactTip(data);
            if (str.length()){
                tip += tipDiv;
                tip += str;
            }
        }
        bFirst = true;
        QString phones = getPhones();
        while (phones.length()){
            if (bFirst){
                tip += tipDiv;
                bFirst = false;
            }else{
                tip += "<br>";
            }
            QString phone_item = getToken(phones, ';', false);
            phone_item = getToken(phone_item, '/', false);
            QString phone = getToken(phone_item, ',');
            getToken(phone_item, ',');
            unsigned phone_type = phone_item.toULong();
            QString icon;
            switch (phone_type){
            case PHONE:
                icon = "phone";
                break;
            case FAX:
                icon = "fax";
                break;
            case CELLULAR:
                icon = "cell";
                break;
            case PAGER:
                icon = "pager";
                break;
            }
            if (icon.length()){
                tip += "<img src=\"sim:icons/";
                tip += icon;
                tip += "\">";
            }
            tip += ' ';
            tip += quoteString(phone);
        }
        return tip;
    }

    bool Contact::setFirstName(const QString &name, const QString &client)
    {
        QString firstName = getFirstName();
        if(firstName == name)
            return false;
        setFirstName(addString(getFirstName(), name, client));
        return true;
    }

    bool Contact::setLastName(const QString &name, const QString &client)
    {
        QString lastName = getLastName();
        if(lastName == name)
            return false;
        setLastName(addString(getLastName(), name, client));
        return true;
    }


    bool Contact::setEMails(const QString &mail, const QString &client)
    {
        QString oldemail = getEMails();
        if(mail == oldemail)
            return false;
        setEMails(addStrings(getEMails(), mail, client));
        return true;
    }

    bool Contact::setPhones(const QString &phone, const QString &client)
    {
        QString oldphones = getPhones();
        if(phone == oldphones)
            return false;
        setPhones(addStrings(getPhones(), phone, client));
        return true;
    }

    int Contact::getGroup()
    {
        return getUserData()->root()->value("Group").toInt();
    }

    void Contact::setGroup(int g)
    {
        getUserData()->root()->setValue("Group", g);
    }

    QString Contact::getName()
    {
        return getUserData()->root()->value("Name").toString();
    }

    void Contact::setName(const QString& s)
    {
        getUserData()->root()->setValue("Name", s);
    }

    bool Contact::getIgnore()
    {
        return getUserData()->root()->value("Ignore").toBool();
    }

    void Contact::setIgnore(bool i)
    {
        getUserData()->root()->setValue("Ignore", i);
    }

    int Contact::getLastActive()
    {
        return getUserData()->root()->value("LastActive").toInt();
    }

    void Contact::setLastActive(int la)
    {
        getUserData()->root()->setValue("LastActive", la);
    }

    QString Contact::getEMails()
    {
        return getUserData()->root()->value("EMails").toString();
    }

    void Contact::setEMails(const QString& e)
    {
        getUserData()->root()->setValue("EMails", e);
    }

    void Contact::addPhone(const QString& p)
    {
        QStringList list = getUserData()->root()->value("PhonesList").toStringList();
        list.append(p);
        getUserData()->root()->setValue("PhonesList", list);
    }
//
//    QStringList Contact::getPhones()
//    {
//        return getUserData()->root()->value("PhonesList").toStringList();
//    }

    QString Contact::getPhones()
    {
        return getUserData()->root()->value("Phones").toString();
    }

    void Contact::setPhones(const QString& p)
    {
        getUserData()->root()->setValue("Phones", p);
    }

    int Contact::getPhoneStatus()
    {
        return getUserData()->root()->value("PhoneStatus").toInt();
    }

    void Contact::setPhoneStatus(int ps)
    {
        getUserData()->root()->setValue("PhoneStatus", ps);
    }

    QString Contact::getFirstName()
    {
        return getUserData()->root()->value("FirstName").toString();
    }

    void Contact::setFirstName(const QString& n)
    {
        getUserData()->root()->setValue("FirstName", n);
    }

    QString Contact::getLastName()
    {
        return getUserData()->root()->value("LastName").toString();
    }

    void Contact::setLastName(const QString& n)
    {
        getUserData()->root()->setValue("LastName", n);
    }

    QString Contact::getNotes()
    {
        return getUserData()->root()->value("Notes").toString();
    }
    
    void Contact::setNotes(const QString& n)
    {
        getUserData()->root()->setValue("Notes", n);
    }

    int Contact::getFlags()
    {
        return getUserData()->root()->value("Flags").toInt();
    }

    void Contact::setFlags(int flags)
    {
        getUserData()->root()->setValue("Flags", flags);
    }

    QString Contact::getEncoding()
    {
        return getUserData()->root()->value("Encoding").toString();
    }

    void Contact::setEncoding(const QString& enc)
    {
        getUserData()->root()->setValue("Encoding", enc);
    }

    void* Contact::createData(Client *client)
    {
        return m_clientData.createData(client);
    }

    void* Contact::getData(Client *client)
    {
        return m_clientData.getData(client);
    }

    bool Contact::have(void* d)
    {
        return m_clientData.have(d);
    }

    void Contact::sort()
    {
        m_clientData.sort();
    }

    void Contact::join(ClientUserData &data)
    {
        m_clientData.join(data);
    }

    void Contact::join(Contact* c)
    {
        m_clientData.join(c->m_clientData);
    }

    void Contact::join(SIM::clientData *cData, ClientUserData &data)
    {
        m_clientData.join(cData, data);
    }

    void Contact::join(SIM::clientData *cData, Contact* c)
    {
        m_clientData.join(cData, c->m_clientData);
    }

    unsigned Contact::size()
    {
        return m_clientData.size();
    }

    Client* Contact::activeClient(void *&data, Client *client)
    {
        return m_clientData.activeClient(data, client);
    }

    QString Contact::property(const char *name)
    {
        return m_clientData.property(name);
    }

    void Contact::freeData(void* d)
    {
        m_clientData.freeData(d);
    }

    void Contact::freeClientData(Client *client)
    {
        m_clientData.freeClientData(client);
    }

    ClientDataIterator Contact::clientDataIterator(Client* client)
    {
        return ClientDataIterator(m_clientData, client);
    }

    QByteArray Contact::saveUseData() const
    {
        return m_clientData.save();
    }

    void Contact::loadUserData(Client *client, Buffer *cfg)
    {
        m_clientData.load(client, cfg);
    }
}

// vim: set expandtab:

