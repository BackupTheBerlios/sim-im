/***************************************************************************
contacts.cpp  -  description
-------------------
begin                : Sun Mar 10 2002
copyright            : (C) 2002 by Vladimir Shutoff
email                : vovan@shutoff.ru
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <algorithm>
#include <map>
#include <list>
#include <vector>

#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QRegExp>
#include <QByteArray>

#include "buffer.h"
#include "event.h"
#include "log.h"
#include "misc.h"
//#include "unquot.h"
#include "profilemanager.h"

#include "contacts.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"
#include "clientmanager.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace SIM
{
//using namespace std;

//#ifdef __OS2__
//char *getDefEncoding()
//{
//	// default charset is win1251 if cp866 used in OS/2
//    COUNTRYCODE ccode = {0};  // Country code info (0 = current country)
//    COUNTRYINFO cinfo = {0};  // Buffer for country-specific information
//    ULONG       ilen  = 0;

//    if ( DosQueryCtryInfo(sizeof(cinfo), &ccode, &cinfo, &ilen) == 0 ) {
//    	if ( cinfo.codepage == 866 ) {
//    		return "CP 1251";
//    	}
//    }
//    return NULL;
//}
//#endif

//struct STR_ITEM
//{
//    QString     value;
//    QStringList proto;
//};

//typedef list<STR_ITEM> STR_LIST;

///* adds a value / client pair to str_list, *avoids* doubled entries */
//static void add_str (STR_LIST & m, const QString & value, QString client)
//{
//    STR_LIST::iterator it;

//    if (client.isEmpty())
//        client = "-";

//    /* check if value is already in str_list */
//    for (it = m.begin (); it != m.end (); ++it) {
//        QString     v = it->value;
//        if (v == value)
//            break;
//    }
//    /* already there */
//    if (it != m.end ()) {
//        QStringList &proto = it->proto;
//        /* client == '-' --> ignore */
//        if (!proto.empty () && client == QLatin1String("-"))
//            return;
//        /* search if  already in list */
//        int itp = proto.indexOf(client);
//        if(itp == -1)
//            return;
//        /* search if "-" somewhere in list --> delete it */
//        itp = proto.indexOf("-");
//        if(itp != -1)
//            proto.removeAt(itp);
//        /* add new client */
//        proto.push_back (client);
//    }
//    else {
//        /* new entry */
//        STR_ITEM    item;
//        item.value = value;
//        item.proto.push_back (client);
//        m.push_back (item);
//    }
//}

//QString addStrings (const QString &old_value, const QString &values,
//                           const QString &client)
//{
//    STR_LIST    str_list;

//    /* split old_value */
//    QString     m = old_value;
//    while (m.length ()) {
//        /* str_item  == one telephone entry */
//        QString     str_item = getToken (m, ';', false);
//        /* str == telephone entry without clients
//         * str_item == clients
//         */
//        QString     str = getToken (str_item, '/');
//        /* now split clients */
//        while (str_item.length ()) {
//            /* split by '/' - this was inserted as this function
//             * doesn't worked correct
//             */
//            QString     proto = getToken (str_item, '/');
//            /* now split by ',' since this seems to be the correct separator */
//            while (proto.length ()) {
//                QString     proto2 = getToken (proto, ',');
//                /* and add */
//                add_str (str_list, str, proto2);
//            }
//        }
//    }

//    /* split values */
//    m = values;
//    while (m.length ()) {
//        QString     str_item = getToken (m, ';', false);
//        QString     str = getToken (str_item, '/');
//        while (str_item.length ()) {
//            QString     proto = getToken (str_item, '/');
//            while (proto.length ()) {
//                QString     proto2 = getToken (proto, ',');
//                add_str (str_list, str, proto2);
//            }
//        }
//    }
//    /* add new client if new client was given */
//    if (!client.isEmpty()) {
//        for (STR_LIST::iterator it = str_list.begin (); it != str_list.end (); ++it) {
//            add_str (str_list, it->value, client);
//        }
//    }

//    /* now build new string */
//    QString     res;
//    for (STR_LIST::iterator it = str_list.begin (); it != str_list.end (); ++it) {
//        QStringList &proto = it->proto;
//        if (proto.size() == 0)
//            continue;
//        if (res.length())
//            res += ';';
//        res += quoteChars (it->value, ";/");
//        res += '/';
//        QString     proto_str;
//        for (QStringList::iterator itp = proto.begin (); itp != proto.end (); ++itp) {
//            if (proto_str.length ())
//                proto_str += ',';
//            proto_str += quoteChars ((*itp), ",;/");
//        }
//        res += proto_str;
//    }
//    return res;
//}

//static QString packString(const QString &value, const QString &client)
//{
//    QString res = quoteChars(value, "/");
//    res += '/';
//    if (!client.isEmpty()){
//        res += client;
//    }else{
//        res += '-';
//    }
//    return res;
//}

//QString addString(const QString &oldValue, const QString &newValue, const QString &client)
//{
//    QString res;
//    if (oldValue.length() == 0){
//        if (newValue.length() == 0)
//            return res;
//        return packString(newValue, client);
//    }
//    QString value = oldValue;
//    getToken(value, '/');
//    if (value == client || client.isEmpty())
//        return packString(newValue, client);
//    return oldValue;
//}

//typedef std::map<unsigned, PacketType*>	PACKET_MAP;
//class ContactListPrivate
//{
//public:
//    ContactListPrivate();
//    ~ContactListPrivate();
//    void clear(bool bClearAll);
//    unsigned registerUserData(const QString &name, const DataDef *def);
//    void unregisterUserData(unsigned id);
//    void flush(Contact *c, Group *g, const QByteArray &section, Buffer *cfg);
//    void flush(Contact *c, Group *g);
//    std::map<unsigned long, UserDataDef> userDataDef;
//    Contact *m_owner;
//    std::map<unsigned long, Contact*>  contacts;
//    std::vector<Group*>  groups;
//    std::vector<Client*> clients;
//    QList<Protocol*>     protocols;
//    PACKET_MAP           packets;
//    bool                 m_bNoRemove;
//    Group				*m_notInList;
//};

//ContactListPrivate::ContactListPrivate()
//    :m_owner(new Contact(0))
//    , m_bNoRemove(false)
//    , m_notInList(new Group(0))
//{
//    groups.push_back(m_notInList);
//}

//ContactListPrivate::~ContactListPrivate()
//{
//    clear(true);
//    delete m_owner;
//}

//void ContactListPrivate::clear(bool bClearAll)
//{
//    m_bNoRemove = true;
//    while (contacts.size() != 0)
//        delete contacts.begin()->second;

//    for (std::vector<Group*>::iterator it_g = groups.begin(); it_g != groups.end();){
//        Group *group = *it_g;
//        if (!bClearAll && (group->id() == 0)){
//            ++it_g;
//            continue;
//        }
//        delete group;
//        it_g = groups.begin();
//    }
//    m_bNoRemove = false;
//}

//unsigned ContactListPrivate::registerUserData(const QString &name, const DataDef *def)
//{
//    unsigned id = 0x1000;   // must be unique...
//    for (std::map<unsigned long, UserDataDef>::iterator it = userDataDef.begin(); it != userDataDef.end(); ++it){
//        if (id <= it->first)
//            id = it->first + 1;
//    }
//    UserDataDef d;
//    d.id = id;
//    d.name = name;
//    d.def = def;
//    userDataDef.insert(std::pair<unsigned long, UserDataDef>(id, d));
//    return id;
//}

//void ContactListPrivate::unregisterUserData(unsigned /*id*/)
//{

//}


//extern DataDef groupData[];

//ContactList::ContactList()
//{
//    m_userData = UserData::create();
//}

//ContactList::~ContactList()
//{

//}

//unsigned ContactList::registerUserData(const QString &name, const DataDef *def)
//{
//    return p->registerUserData(name, def);
//}

//void ContactList::unregisterUserData(unsigned id)
//{
//    p->unregisterUserData(id);
//}

//Contact *ContactList::owner()
//{
//    return p->m_owner;
//}

//bool ContactList::groupExists(unsigned long id)
//{
//    vector<Group*>::iterator it;
//    for(it = p->groups.begin(); it != p->groups.end(); ++it)
//    {
//        if ((*it)->id() == id)
//            return true;
//    }
//    return false;
//}

//bool ContactList::contactExists(unsigned long id)
//{
//    map<unsigned long, Contact*>::iterator it = p->contacts.find(id);
//    if(it != p->contacts.end())
//        return true;
//    return false;
//}

//Contact *ContactList::contact(unsigned long id, bool isNew)
//{
//    map<unsigned long, Contact*>::iterator it = p->contacts.find(id);
//    if (it != p->contacts.end())
//        return it->second;
//    if (!isNew)
//        return NULL;
//    if (id == 0){
//        id = 1;
//        for (it = p->contacts.begin(); it != p->contacts.end(); ++it){
//            if (id <= it->second->id())
//                id = it->second->id() + 1;
//        }
//    }
//    Contact *res = new Contact(id);
//    p->contacts.insert(std::pair<unsigned long, Contact*>(id, res));
//    EventContact e(res, EventContact::eCreated);
//    e.process();
//    return res;
//}

//void ContactList::addContact(Contact *contact)
//{
////    if (contact->id())
////        return;
////    unsigned long id = 1;
////    map<unsigned long, Contact*>::iterator it;
////    for (it = p->contacts.begin(); it != p->contacts.end(); ++it){
////        if (id <= it->second->id())
////            id = it->second->id() + 1;
////    }
////    contact->m_id = id;
////    p->contacts.insert(std::pair<unsigned long, Contact*>(id, contact));
////    EventContact e(contact, EventContact::eCreated);
////    e.process();
//}

//Group *ContactList::group(unsigned long id, bool isNew)
//{
//    vector<Group*>::iterator it;
//    if (id || !isNew)
//    {
//        for (it = p->groups.begin(); it != p->groups.end(); ++it) //Search group with given id and return group obj
//        {
//            if ((*it)->id() == id)
//                return (*it);
//        }
//    }
//    if (!isNew)
//        return NULL;
//    if (id == 0) //calculate new group id and create a new group
//        for (it = p->groups.begin(); it != p->groups.end(); ++it)
//            if (id <= (*it)->id())
//                id = (*it)->id() + 1;
//    Group *res = new Group(id);
//    p->groups.push_back(res);
//    EventGroup e(res, EventGroup::eAdded);
//    e.process();
//    return res;
//}

//int ContactList::groupIndex(unsigned long id)
//{
//    for (unsigned i = 0; i < p->groups.size(); i++)
//        if (p->groups[i]->id() == id)
//            return i;
//    return -1;
//}

//int ContactList::groupCount()
//{
//    return p->groups.size();
//}

//bool ContactList::moveGroup(unsigned long id, bool bUp)
//{
//    if (id == 0)
//        return false;
//    for (unsigned i = 1; i < p->groups.size(); i++)
//        if (p->groups[i]->id() == id)
//        {
//            if (!bUp && i == p->groups.size() - 1)
//                return false;
//            if (bUp) i--;
//            if (i == 0)
//                return false;
//            Group *g = p->groups[i];
//            p->groups[i] = p->groups[i+1];
//            p->groups[i+1] = g;
//            EventGroup e1(p->groups[i], EventGroup::eChanged);
//            EventGroup e2(p->groups[i+1], EventGroup::eChanged);
//            e1.process();
//            e2.process();
//            return true;
//        }
//    return false;
//}

//class GroupIteratorPrivate
//{
//public:
//    GroupIteratorPrivate();
//    Group *operator++();
//    vector<Group*>::iterator it;
//};

//GroupIteratorPrivate::GroupIteratorPrivate()
//{
//    it = getContacts()->p->groups.begin();
//}

//Group *GroupIteratorPrivate::operator++()
//{
//    if (it != getContacts()->p->groups.end())
//    {
//        Group *res = (*it);
//        ++it;
//        return res;
//    }
//    return NULL;
//}

//ContactList::GroupIterator::GroupIterator()
//{
//    p = new GroupIteratorPrivate;
//}

//void ContactList::GroupIterator::reset()
//{
//    p->it = getContacts()->p->groups.begin();
//}

//ContactList::GroupIterator::~GroupIterator()
//{
//    delete p;
//}

//Group *ContactList::GroupIterator::operator ++()
//{
//    return ++(*p);
//}

//class ContactIteratorPrivate
//{
//public:
//    ContactIteratorPrivate();
//    Contact *operator++();
//    map<unsigned long, Contact*>::iterator it;
//};

//ContactIteratorPrivate::ContactIteratorPrivate()
//{
//    it = getContacts()->p->contacts.begin();
//}

//Contact *ContactIteratorPrivate::operator++()
//{
//    if (it != getContacts()->p->contacts.end()){
//        Contact *res = it->second;
//        ++it;
//        return res;
//    }
//    return NULL;
//}

//ContactList::ContactIterator::ContactIterator()
//{
//    p = new ContactIteratorPrivate;
//}

//void ContactList::ContactIterator::reset()
//{
//    p->it = getContacts()->p->contacts.begin();
//}

//ContactList::ContactIterator::~ContactIterator()
//{
//    delete p;
//}

//Contact *ContactList::ContactIterator::operator ++()
//{
//    return ++(*p);
//}

//class PacketIteratorPrivate
//{
//public:
//    PacketIteratorPrivate();
//    PacketType *operator++();
//    PACKET_MAP::iterator it;
//};

//PacketIteratorPrivate::PacketIteratorPrivate()
//{
//    it = getContacts()->p->packets.begin();
//}

//PacketType *PacketIteratorPrivate::operator++()
//{
//    if (it != getContacts()->p->packets.end()){
//        PacketType *res = it->second;
//        ++it;
//        return res;
//    }
//    return NULL;
//}

//ContactList::PacketIterator::PacketIterator()
//{
//    p = new PacketIteratorPrivate;
//}

//void ContactList::PacketIterator::reset()
//{
//    p->it = getContacts()->p->packets.begin();
//}

//ContactList::PacketIterator::~PacketIterator()
//{
//    delete p;
//}

//PacketType *ContactList::PacketIterator::operator ++()
//{
//    return ++(*p);
//}


//void ContactList::addPacketType(unsigned id, const QString &name, bool bText)
//{
//    PACKET_MAP::iterator it = p->packets.find(id);
//    if (it != p->packets.end())
//        return;
//    p->packets.insert(PACKET_MAP::value_type(id, new PacketType(id, name, bText)));
//}

//void ContactList::removePacketType(unsigned id)
//{
//    PACKET_MAP::iterator it = p->packets.find(id);
//    if (it != p->packets.end()){
//        delete it->second;
//        p->packets.erase(it);
//    }
//}

//PacketType *ContactList::getPacketType(unsigned id)
//{
//    PACKET_MAP::iterator it = p->packets.find(id);
//    if (it == p->packets.end())
//        return NULL;
//    return it->second;
//}

//bool ContactList::moveClient(Client *client, bool bUp)
//{
////    unsigned i;
////    for (i = 0; i < p->clients.size(); i++)
////        if (p->clients[i] == client)
////            break;
////    if (!bUp)
////        i++;
////    if ((i == 0) && (i >= p->clients.size()))
////        return false;
////    Client *c = p->clients[i];
////    p->clients[i] = p->clients[i-1];
////    p->clients[i-1] = c;
////    EventClientsChanged e;
////    e.process();
////    Contact *contact;
////    ContactList::ContactIterator it;
////    while ((contact = ++it) != NULL){
////        contact->sort();
////        EventContact e(contact, EventContact::eChanged);
////        e.process();
////    }
//    return true;
//}

//void ContactList::addClient(Client *client)
//{
//    p->clients.push_back(client);
//    EventClientsChanged e;
//    e.process();
//}

//static const char CONTACTS_CONF[] = "contacts.conf";
//static const char CONTACT[] = "Contact=";
//static const char GROUP[] = "Group=";
//static const char OWNER[] = "Owner";
//static const char BACKUP_SUFFIX[] = "~";
//extern DataDef contactData[];

//void ContactList::save()
//{
//	save_new();
//}

//void ContactList::clear()
//{
//    p->clear(false);
//}

//void ContactList::load()
//{
//    clear();
//    if(!load_new())
//    {
//        if(!load_old())
//        {
//            log(L_ERROR, "Unable to load contacts");
//        }
//    }
//    for (unsigned i = 0; i < nClients(); i++){
//        Client *client = getClient(i);
//    }
//}

//bool ContactList::load_old()
//{
////    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.conf";
////    QFile f(cfgName);
////    if (!f.open(QIODevice::ReadOnly)){
////        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
////        return false;
////    }

////    PropertyHubPtr currenthub;
////    IMContact* imcontact = 0;
////    Contact* c = 0;
////    Group* gr = 0;
////    UserDataPtr currentUserData = getUserData();
////    while(!f.atEnd())
////    {
////        QString line = QString::fromLocal8Bit( f.readLine() );
////        line = line.trimmed();
////        //log(L_DEBUG, "Line: %s", qPrintable(line));
////        if(line.startsWith("[Group="))
////        {
////            imcontact = 0;
////            c = 0;
////            int id = line.mid(7, line.length() - 8).toInt();
////            gr = group(id, id != 0);
////            currenthub = gr->userdata();
////            currentUserData = gr->getUserData();
////        }
////        else if(line.startsWith("[Contact="))
////        {
////            imcontact = 0;
////            gr = 0;
////            int id = line.mid(9, line.length() - 10).toInt();
////            c = contact(id, true);
////            //log(L_DEBUG, "Contact: %d", id);
////            currenthub = c->getUserData()->root();
////            currentUserData = c->getUserData();
////        }
////        else if(line.startsWith("["))
////        {
////            QString dataname = line.mid(1, line.length() - 2);
////            int dotindex = line.indexOf(".");
////            if(dotindex > 0 && (c != 0 || gr != 0))
////            {
////                ClientPtr client = getClientManager()->client(dataname);
////                if(client)
////                {
////                    if(c) {
////                        imcontact = c->getData(client.data());
////                        if(!imcontact)
////                            imcontact = c->createData(client.data());
////                    }
////                    else if(gr) {
////                        imcontact = gr->getData(client.data());
////                        if(!imcontact)
////                            imcontact = gr->createData(client.data());
////                    }
////                    currenthub.clear();
////                }
////            }
////            else
////            {
////                if(!currentUserData.isNull())
////                    currenthub = currentUserData->createUserData(dataname);
////                imcontact = 0;
////            }
////        }
////        else
////        {
////            QStringList keyval = line.split("=");
////            QString val = keyval.at(1);
////            if(val.startsWith('"') && val.endsWith('"'))
////                val = val.mid(1, val.length() - 2);
////            if(!currenthub.isNull())
////            {
////                currenthub->setValue(keyval.at(0), val);
////                //log(L_DEBUG, "Userdata deserialization: %s=%s", qPrintable(keyval.at(0)), qPrintable(val));
////            }
////            if(imcontact)
////            {
////                imcontact->deserializeLine(keyval.at(0), val);
////                //log(L_DEBUG, "IMContact deserialization: %s=%s", qPrintable(keyval.at(0)), qPrintable(val));
////            }
////        }
////    }
////    return true;
//}

//void ContactList::save_new()
//{
//    if(!ProfileManager::instance()->profilePath().isEmpty())
//    {
//        QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.xml";
//        ProfileManager::instance()->sync();
//        QDomDocument doc;
//        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
//        QDomElement root = doc.createElement("contactlist");
//        QDomElement global = doc.createElement("global");
//        getUserData()->serialize(global);
//        root.appendChild(global);

//        QDomElement owner = doc.createElement("owner");
//        save_owner(owner);
//        root.appendChild(owner);

//        QDomElement groups = doc.createElement("groups");
//        if(save_groups(groups))
//            root.appendChild(groups);
//        QDomElement contacts = doc.createElement("contacts");
//        if(save_contacts(contacts))
//            root.appendChild(contacts);
//        doc.appendChild(root);
//        QFile f(cfgName);
//        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
//        f.write(doc.toByteArray());
//        f.close();
//    }
//}

//bool ContactList::save_owner(QDomElement element)
//{
//    owner()->serialize(element);
//    QStringList clients = getClientManager()->clientList();
//    foreach(const QString& clname, clients) {
//        ClientPtr client = getClientManager()->client(clname);
//        IMContactPtr imc = client->ownerContact();
//        if(!imc)
//            return false;
//        QDomElement el = element.ownerDocument().createElement("clientdata");
//        el.setAttribute("clientname", client->name());
//        imc->serialize(el);
//        element.appendChild(el);
//    }
//    return true;
//}

//bool ContactList::save_groups( QDomElement element )
//{
//    if( p->groups.size() == 0 )
//        return false;

//    vector<Group*>::iterator it;
//    for( it = p->groups.begin(); it != p->groups.end(); ++it ) {
//        QDomElement group =  element.ownerDocument().createElement( "group" );
//        group.setAttribute( "id", QString::number( (*it)->id() ) );
//        if( (*it)->getUserData()->serialize( group ) )
//            element.appendChild( group );
//    }

//    return true;
//}

//bool ContactList::save_contacts( QDomElement element ) {
//    if( p->groups.size() == 0 )
//        return false;

//    map<unsigned long, Contact*>::iterator it;
//    for(it = p->contacts.begin(); it != p->contacts.end(); ++it) {
//        QDomElement contact = element.ownerDocument().createElement("contact");
//        contact.setAttribute("id", QString::number(it->first));
//        it->second->serialize(contact);
//        element.appendChild(contact);
//    }

//    return true;
//}

//bool ContactList::load_new()
//{
//    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "contacts.xml";
//    QFile f(cfgName);
//    if(!f.open(QIODevice::ReadOnly))
//        return false;
//    QDomDocument doc;
//    doc.setContent(f.readAll());
//    QDomElement el = doc.elementsByTagName("global").at(0).toElement();
//    if(!getUserData()->deserialize(el))
//        return false;

//    QDomElement owner = doc.elementsByTagName("owner").at(0).toElement();
//    this->load_owner(owner);

//    QDomElement groups = doc.elementsByTagName("groups").at(0).toElement();
//    if(!load_groups(groups))
//        return false;


//    QDomElement contacts = doc.elementsByTagName("contacts").at(0).toElement();
//    if(!load_contacts(contacts))
//        return false;


//    return true;
//}

//bool ContactList::load_owner(const QDomElement& o)
//{
//    this->owner()->deserialize(o);
//    QDomNodeList list = o.elementsByTagName("clientdata");
//    for(int i = 0; i < list.size(); i++) {
//        QDomElement el = list.at(i).toElement();
//        ClientPtr client = getClientManager()->client(el.attribute("clientname"));
//        if(!client)
//            continue;
//        IMContactPtr c = client->ownerContact();
//        if(!c) {
//            c = client->protocol()->createIMContact(client);
//        }
//        c->deserialize(el);
//    }
//    return true;
//}

//bool ContactList::load_groups(const QDomElement& groups)
//{
//    QDomNodeList list = groups.elementsByTagName("group");
//    for(int i = 0; i < list.size(); i++) {
//        QDomElement el = list.at(i).toElement();
//        int id = el.attribute("id").toInt();
//        Group* gr = group(id, id != 0);
//        if(!gr->getUserData()->deserialize(el))
//            return false;
//    }
//    return true;
//}

//bool ContactList::load_contacts(const QDomElement& contacts)
//{
//    QDomNodeList list = contacts.elementsByTagName("contact");
//    for(int i = 0; i < list.size(); i++) {
//        QDomElement el = list.at(i).toElement();
//        int id = el.attribute("id").toInt();
//        Contact* c = contact(id, true);

//        c->deserialize(el);
////        c->getUserData()->deserialize(el);
////        QDomNodeList cldatalist = el.elementsByTagName("clientdata");
////        for(int j = 0; j < cldatalist.size(); j++) {
////            QDomElement clientElement = cldatalist.at(j).toElement();
////            ClientPtr client = getClientManager()->client(clientElement.attribute("clientname"));
////            IMContact* imc = c->getData(client->name());
////            if(!imc)
////                imc = c->createData(client.data());
////            imc->deserialize(clientElement);
////        }

//    }
//    return true;
//}

//void ContactListPrivate::flush(Contact *c, Group *g)
//{
//    ClientUserData *data = NULL;
////    if (c)
////        data = &c->clientData;
//    //c->sort();
////    if (g)
////        data = &g->clientData;
////    if (data)
////        data->sort();
//}

//void ContactListPrivate::flush(Contact *c, Group *g, const QByteArray &section, Buffer *cfg)
//{
//    if (cfg == NULL)
//        return;
////    if (section.isEmpty()){
////        if (c){
////            free_data(contactData, &c->data);
////            load_data(contactData, &c->data, cfg);
////            return;
////        }
////        if (g){
////            free_data(groupData, &g->data);
////            load_data(groupData, &g->data, cfg);
////            return;
////        }
////        return;
////    }
////    map<unsigned long, UserDataDef>::iterator it;
////    for (it = userDataDef.begin(); it != userDataDef.end(); ++it){
////        if (section != it->second.name)
////            continue;
////        UserData_old *data = &userData;
////        if (c)
////            data = &c->userData;
////        if (g)
////            data = &g->userData;
////        data->load(it->second.id, it->second.def, cfg);
////        return;
////    }
//    for (unsigned i = 0; i < getContacts()->nClients(); i++){
////        Client *client = getContacts()->getClient(i);
////        if (client->name() != section)
////            continue;
////        ClientUserData *data = NULL;
////        if (c && !g)
////            c->loadUserData(client, cfg);
////        if (g)
////            data = &g->clientData;
////        if (data)
////            data->load(client, cfg);
//        return;
//    }
//}

//unsigned ContactList::nClients()
//{
//    return p->clients.size();
//}

//Client *ContactList::getClient(unsigned n)
//{
//    if (n >= p->clients.size())
//        return NULL;
//    return p->clients[n];
//}

//void ContactList::clearClients()
//{
//    p->m_bNoRemove = true;
//    while (!p->clients.empty())
//        delete p->clients[0];
//    p->m_bNoRemove = false;
//    EventClientsChanged eClients;
//    eClients.process();
//}

//PropertyHubPtr ContactList::getUserData(const QString& id)
//{
//    PropertyHubPtr hub = m_userData->getUserData(id);
//    if(hub.isNull())
//        hub = m_userData->createUserData(id);
//    return hub;
//}

//static QString stripPhone(const QString &phone)
//{
//    QString res;
//    if (phone.isEmpty())
//        return res;
//    for (int i = 0; i < phone.length(); i++){
//        const QChar &c = phone[i];
//        if ((c < '0') || (c > '9'))
//            continue;
//        res += c;
//    }
//    if (res.length() > 7)
//        res = res.mid(res.length() - 7);
//    return res;
//}

//bool ContactList::cmpPhone(const QString &phone1, const QString &phone2)
//{
//    return stripPhone(phone1) == stripPhone(phone2);
//}

//Contact *ContactList::contactByPhone(const QString &_phone)
//{
////    QString phone = stripPhone(_phone);
////    if (phone.isEmpty())
////        return NULL;
////    Contact *c;
////    ContactIterator it;
////    while ((c = ++it) != NULL){
////        QString phones = c->getPhones();
////        while (!phones.isEmpty()){
////            QString phoneItem = getToken(phones, ';', false);
////            if (cmpPhone(getToken(phoneItem, ','), _phone))
////                return c;
////        }
////    }
////    c = contact(0, true);
////    c->setFlags(CONTACT_TEMP);
////    c->setName(_phone);
////    EventContact e(c, EventContact::eChanged);
////    e.process();
////    return c;
//}

//Contact *ContactList::contactByMail(const QString &_mail, const QString &_name)
//{
////    QString name = _name;
////    Contact *c;
////    ContactIterator it;
////    if (_mail.isEmpty()){
////        while ((c = ++it) != NULL){
////            if (c->getName().toLower() == name.toLower())
////                return c;
////        }
////        c = contact(0, true);
////        c->setFlags(CONTACT_TEMP);
////        c->setName(name);
////        EventContact e(c, EventContact::eChanged);
////        e.process();
////        return c;
////    }
////    if (name.isEmpty())
////        name = _mail;
////    while ((c = ++it) != NULL){
////        QString mails = c->getEMails();
////        while (mails.length()){
////            QString mail = getToken(mails, ';', false);
////            mail = getToken(mail, '/');
////            if (mail.toLower() == _mail.toLower())
////                return c;
////        }
////    }
////    c = contact(0, true);
////    c->setFlags(CONTACT_TEMP);
////    c->setName(name);
////    c->setEMails(_mail + "/-");
////    EventContact e(c, EventContact::eChanged);
////    e.process();
////    return c;
//}

//static ContactList* gs_contactList = 0;

//EXPORT void createContactList()
//{
//	if(gs_contactList)
//		return;
//	gs_contactList = new ContactList();
//}

//EXPORT void destroyContactList()
//{
//	if(!gs_contactList)
//		return;
//	delete gs_contactList;
//	gs_contactList = 0;
//}

//EXPORT ContactList *getContacts()
//{
//    return gs_contactList;
//}

//// see also http://www.iana.org/assignments/character-sets
//static ENCODING encodings[] =
//    {
//        { I18N_NOOP("Unicode"),  "UTF-8",  106, 0, 65001, true },

//        { I18N_NOOP("Arabic"), "ISO 8859-6", 82, 180, 28596, false },
//        { I18N_NOOP("Arabic"), "CP 1256", 2256, 180, 1256, true },

//        { I18N_NOOP("Baltic"), "ISO 8859-13", 109, 186, 28594, false },
//        { I18N_NOOP("Baltic"), "CP 1257", 2257, 186, 1257, true },

//        { I18N_NOOP("Central European"), "ISO 8859-2", 5, 238, 28592, false },
//        { I18N_NOOP("Esperanto"), "ISO 8859-3", 6, 238, 28593, false },
//        { I18N_NOOP("Central European"), "CP 1250", 2250, 238, 1250, true },

//        { I18N_NOOP("Chinese "), "GBK", 2025, 134, 0, false },
//        { I18N_NOOP("Chinese Simplified"), "gbk2312",2312, 134, 0, true },
//        { I18N_NOOP("Chinese Traditional"), "Big5",2026, 136, 0, true },

//        { I18N_NOOP("Cyrillic"), "ISO 8859-5", 8, 204, 28595, false },
//        { I18N_NOOP("Cyrillic"), "KOI8-R", 2084, 204, 1251, false },
//        { I18N_NOOP("Ukrainian"), "KOI8-U", 2088, 204, 1251, false },
//        { I18N_NOOP("Cyrillic"), "CP 1251", 2251, 204, 1251, true },

//        { I18N_NOOP("Greek"), "ISO 8859-7", 10, 161, 28597, false },
//        { I18N_NOOP("Greek"), "CP 1253", 2253, 161, 1253, true },

//        { I18N_NOOP("Hebrew"), "ISO 8859-8-I", 85, 177, 28598,  false },
//        { I18N_NOOP("Hebrew"), "CP 1255", 2255, 177, 1255, true },

//        { I18N_NOOP("Japanese"), "JIS7", 16, 128, 0, false },
//        { I18N_NOOP("Japanese"), "eucJP", 18, 128, 0, false },
//        { I18N_NOOP("Japanese"), "Shift-JIS", 17, 128, 0, true },

//        { I18N_NOOP("Korean"), "eucKR", 38, 0, 0, true },

//        { I18N_NOOP("Western European"), "ISO 8859-1", 4, 0, 28591, false },
//        { I18N_NOOP("Western European"), "ISO 8859-15", 111, 0, 28605, false },
//        { I18N_NOOP("Western European"), "CP 1252", 2252, 0, 1252, true },

//        { I18N_NOOP("Tamil"), "TSCII", 2107, 0, 0, true },

//        { I18N_NOOP("Thai"), "TIS-620", 2259, 222, 0, true },

//        { I18N_NOOP("Turkish"), "ISO 8859-9", 12, 162, 28599, false },
//        { I18N_NOOP("Turkish"), "CP 1254", 2254, 162, 1254, true },

//        { NULL, NULL, 0, 0, 0, false }
//    };

//const ENCODING *ContactList::getEncodings()
//{
//    return encodings;
//}

//QTextCodec *ContactList::getCodecByName(const QString &encoding)
//{
//    if (!encoding.isEmpty()){
//        QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
//        if (codec)
//            return codec;
//    }
//    QTextCodec *codec = QTextCodec::codecForLocale();
//    const ENCODING *e;
//    const char *codecName = codec->name();
//    for (e = encodings; e->language; e++){
//        if (!strcmp(codecName, e->codec))
//            break;
//    }
//    if (e->language && !e->bMain){
//        for (e++; e->language; e++){
//            if (e->bMain){
//                codec = QTextCodec::codecForName(e->codec);
//                break;
//            }
//        }
//    }
//    if (codec == NULL)
//        codec= QTextCodec::codecForLocale();
//    return codec;
//}

//QTextCodec *ContactList::getCodecByCodePage(const int iCP)
//{
//    const ENCODING *e;
//    for (e = encodings; e->language; e++){
//        if ( iCP == e->cp_code )
//            return ContactList::getCodecByName(e->codec);
//    }

//    return NULL;
//}

//const ENCODING *ContactList::getEncoding(Contact *contact)
//{
////    if( NULL == contact )
////        return NULL;

////    QString sEncoding = contact->getEncoding();
////    if( sEncoding.isEmpty() )
////        sEncoding = owner()->getEncoding();

////    const ENCODING *e = NULL;

////    if( sEncoding.isEmpty() ) {
////        QTextCodec *codec = QTextCodec::codecForLocale();
////        int mib = codec->mibEnum();
////        if( mib != 0 ) {
////            for (e = encodings; e->language; e++){
////                if ( mib == e->mib )
////                    break;
////            }
////        }
////        else {
////#ifdef WIN32
////            int iSize = ::GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, NULL, 0 );
////            char *sTemp = new char[iSize];
////            ::GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, sTemp, iSize );
////            QString sAnsiCP = QString::fromLatin1( sTemp );
////            int iCP = sAnsiCP.toUInt();
////            for (e = encodings; e->language; e++){
////                if ( iCP == e->cp_code )
////                    break;
////            }
////#endif
////        }
////    }
////    else {
////        for (e = encodings; e->language; e++){
////            if ( sEncoding == e->codec )
////                break;
////        }
////        if (e->language && !e->bMain){
////            for (e++; e->language; e++){
////                if (e->bMain){
////                    break;
////                }
////            }
////        }

////        if( e->language == NULL )
////            e = NULL;
////    }

////    return e;
//}

//QTextCodec *ContactList::getCodec(Contact *contact)
//{
//    QTextCodec *codec = NULL;
////    if (contact && !contact->getEncoding().isEmpty()){
////        codec = getCodecByName(contact->getEncoding());
////        if (codec)
////            return codec;
////    }
////    return getCodecByName(owner()->getEncoding());
//}

//QString ContactList::toUnicode(Contact *contact, const QByteArray &str)
//{
//    if (!str.isEmpty()){
//        QString res = getCodec(contact)->toUnicode(str);
//        return res.remove('\r');
//    }
//    return QString();
//}

//QByteArray ContactList::fromUnicode(Contact *contact, const QString &str)
//{
//    if (str.isEmpty())
//        return QByteArray();
//    QString s = str;
//    s = s.replace(QRegExp("\r?\n"), "\r\n");
//    return getCodec(contact)->fromUnicode(s);
//}

//void ContactList::removeGroup(unsigned long id)
//{
//    Group* gr = group(id, false);
//    if(!gr)
//        return;
//    if(!p->m_bNoRemove){
//        Contact *contact;
//        ContactList::ContactIterator itc;
//        while ((contact = ++itc) != NULL){
//            if (contact->getGroup() != (int)id)
//                continue;
//            contact->setGroup(0);
//            EventContact e(contact, EventContact::eChanged);
//            e.process();
//        }
//        EventGroup e(gr, EventGroup::eDeleted);
//        e.process();
//    }
//    std::vector<Group*> &groups = p->groups;
//    for (std::vector<Group*>::iterator it = groups.begin(); it != groups.end(); ++it) {
//        if ((*it) == gr) {
//            groups.erase(it);
//            break;
//        }
//    }
//}

//void ContactList::removeClient(Client* cl)
//{
//    for(std::vector<Client*>::iterator it = p->clients.begin(); it != p->clients.end(); ++it)
//    {
//        if ((*it) != cl)
//            continue;
//        p->clients.erase(it);
//        if (!getContacts()->p->m_bNoRemove)
//        {
//            EventClientsChanged e;
//            e.process();
//        }
//        break;
//    }
//}

//void ContactList::removeContact(unsigned long id)
//{
//    if (!p->m_bNoRemove) {
//        Contact* c = contact(id, false);
//        if(c) {
//            EventContact e(c, EventContact::eDeleted);
//            e.process();
//        }
//    }
//    p->contacts.erase(id);
//}

//QList<Group*> ContactList::allGroups()
//{
//    QList<Group*> groupList;
//    for(std::vector<Group*>::iterator it = p->groups.begin(); it != p->groups.end(); ++it)
//    {
//        groupList.append(*it);
//    }
//    return groupList;
//}

//QList<Contact*> ContactList::contactsInGroup(Group* gr)
//{
//    QList<Contact*> contactList;
//    for(std::map<unsigned long, Contact*>::iterator it = p->contacts.begin(); it != p->contacts.end(); ++it)
//    {
//        if((*it).second->getGroup() == gr->id())
//            contactList.append((*it).second);
//    }
//    return contactList;
//}

}   // namespace sim
// vim: set expandtab:

