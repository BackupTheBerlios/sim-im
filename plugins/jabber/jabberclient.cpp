/***************************************************************************
                          jabberclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "simapi.h"

#ifndef WIN32
#include <ctype.h>
#endif

#include <algorithm>

#include "html.h"
#include "icons.h"
#include "log.h"
#include "unquot.h"
#include "core.h"
#include "core_events.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include "jabberclient.h"
#include "jabber.h"
#include "jabberconfig.h"
#include "jabber_ssl.h"
#include "jabberadd.h"
#include "jabberinfo.h"
#include "jabberhomeinfo.h"
#include "jabberworkinfo.h"
#include "jabberaboutinfo.h"
#include "jabberpicture.h"
#include "jabbermessage.h"
#include "jabberbrowser.h"
#include "infoproxy.h"

#include <QTimer>
#include <QRegExp>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QApplication>
#include <QWidget>
#include <QByteArray>
#include <QDateTime>
#include <QDomDocument>

using namespace std;
using namespace SIM;

#ifndef XML_STATUS_OK
#define XML_STATUS_OK    1
#define XML_STATUS_ERROR 0
#endif

unsigned PING_TIMEOUT = 50;

DataDef jabberUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(2) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "ID", DATA_UTF, 1, 0 },
        { "Node", DATA_UTF, 1, 0 },
        { "Resource", DATA_UTF, 1, 0 },
        { "Name", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(1) },		// Status
        { "FirstName", DATA_UTF, 1, 0 },
        { "Nick", DATA_UTF, 1, 0 },
        { "Desc", DATA_UTF, 1, 0 },
        { "BirthDay", DATA_UTF, 1, 0 },
        { "Url", DATA_UTF, 1, 0 },
        { "OrgName", DATA_UTF, 1, 0 },
        { "OrgUnit", DATA_UTF, 1, 0 },
        { "Role", DATA_UTF, 1, 0 },
        { "Title", DATA_UTF, 1, 0 },
        { "Street", DATA_UTF, 1, 0 },
        { "ExtAddr", DATA_UTF, 1, 0 },
        { "City", DATA_UTF, 1, 0 },
        { "Region", DATA_UTF, 1, 0 },
        { "PCode", DATA_UTF, 1, 0 },
        { "Country", DATA_UTF, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "Phone", DATA_UTF, 1, 0 },
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "Subscribe", DATA_ULONG, 1, 0 },
        { "Group", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, 0 },			// bChecked
        { "", DATA_STRING, 1, 0 },			// TypingId
        { "", DATA_BOOL, 1, 0 },			// SendTypingEvents
        { "", DATA_BOOL, 1, 0 },			// IsTyping
        { "", DATA_ULONG, 1, 0 },			// ComposeId
        { "", DATA_BOOL, 1, DATA(1) },			// richText
        { "", DATA_BOOL, 1, 0 },
        { "PhotoWidth", DATA_LONG, 1, 0 },
        { "PhotoHeight", DATA_LONG, 1, 0 },
        { "LogoWidth", DATA_LONG, 1, 0 },
        { "LogoHeight", DATA_LONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// nResources
        { "", DATA_STRLIST, 1, 0 },			// Resources
        { "", DATA_STRLIST, 1, 0 },			// ResourceStatus
        { "", DATA_STRLIST, 1, 0 },			// ResourceReply
        { "", DATA_STRLIST, 1, 0 },			// ResourceStatusTime
        { "", DATA_STRLIST, 1, 0 },			// ResourceOnlineTime
        { "AutoReply", DATA_UTF, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },			// ResourceClientName
        { "", DATA_STRLIST, 1, 0 },			// ResourceClientVersion
        { "", DATA_STRLIST, 1, 0 },			// ResourceClientOS
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static DataDef jabberClientData[] =
    {
        { "Server", DATA_STRING, 1, "jabber.org" },
        { "Port", DATA_ULONG, 1, DATA(5222) },
        { "UseSSL", DATA_BOOL, 1, 0 },
        { "UsePlain", DATA_BOOL, 1, 0 },
        { "UseVHost", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "Priority", DATA_ULONG, 1, DATA(5) },
        { "ListRequest", DATA_UTF, 1, 0 },
        { "VHost", DATA_UTF, 1, 0 },
        { "Typing", DATA_BOOL, 1, DATA(1) },
        { "RichText", DATA_BOOL, 1, DATA(1) },
        { "UseVersion", DATA_BOOL, 1, DATA(1) },
        { "ProtocolIcons", DATA_BOOL, 1, DATA(1) },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFF) },
        { "Photo", DATA_UTF, 1, 0 },
        { "Logo", DATA_UTF, 1, 0 },
        { "AutoSubscribe", DATA_BOOL, 1, DATA(1) },
        { "AutoAccept", DATA_BOOL, 1, DATA(1) },
        { "UseHTTP", DATA_BOOL, 1, 0 },
        { "URL", DATA_STRING, 1, 0 },
        { "InfoUpdated", DATA_BOOL, 1, 0 },
        { "", DATA_STRUCT, sizeof(JabberUserData) / sizeof(Data), DATA(jabberUserData) },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

JabberClient::JabberClient(JabberProtocol *protocol, Buffer *cfg) : TCPClient(protocol, cfg)
{
    load_data(jabberClientData, &data, cfg);
    QString jid = data.owner.ID.str();
	//log(L_DEBUG, "JID: %s", jid.toUtf8().data());

    //For old configs, where server part in own jid is missing
    if (!jid.isEmpty() && jid.indexOf('@')==-1)
	{
        jid += '@';
        if (getUseVHost())
		{
			jid += getVHost();
		}
		else
		{
			jid += getServer();
		}
        data.owner.ID.str()=jid;
    }
    if (data.owner.Resource.str().isEmpty())
	{
        QString resource = PACKAGE;
        data.owner.Resource.str() = resource.simplified();
    }

    TCPClient::changeStatus(this->protocol()->status("offline"));

    QString listRequests = getListRequest();
    while (!listRequests.isEmpty()){
        QString item = getToken(listRequests, ';', false);
        JabberListRequest lr;
        lr.bDelete = false;
        lr.jid = getToken(item, ',');
        lr.grp = getToken(item, ',');
        if (!item.isEmpty())
            lr.bDelete = true;
        m_listRequests.push_back(lr);
    }
    setListRequest(QString::null);

    m_bSSL		 = false;
    m_curRequest = NULL;
    m_msg_id	 = 0;
    m_bJoin		 = false;
    init();
}

JabberClient::~JabberClient()
{
    TCPClient::changeStatus(this->protocol()->status("offline"));
    //TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(jabberClientData, &data);
    freeData();
}

const DataDef *JabberProtocol::userDataDef()
{
    return jabberUserData;
}

bool JabberClient::compareData(void *d1, void *d2)
{
    JabberUserData *data1 = toJabberUserData((SIM::clientData*)d1); // FIXME unsafe type conversion
    JabberUserData *data2 = toJabberUserData((SIM::clientData*)d2); // FIXME unsafe type conversion
    return (data1->ID.str().toLower() == data2->ID.str().toLower());
}

void JabberClient::setID(const QString &id)
{
    data.owner.ID.str() = id;
}

QByteArray JabberClient::getConfig()
{
    QString lr;
    for (list<JabberListRequest>::iterator it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if (!lr.isEmpty())
            lr += ';';
        lr += quoteChars(it->jid, ",;");
        lr += ',';
        lr += quoteChars(it->grp, ",;");
        if (it->bDelete)
            lr += ",1";
    }
    setListRequest(lr);
    QByteArray res = Client::getConfig();
    if (res.length())
        res += '\n';
    return res += save_data(jabberClientData, &data);
}

QString JabberClient::name()
{
    QString res = "Jabber.";
    res += data.owner.ID.str();
    return res;
}

QWidget	*JabberClient::setupWnd()
{
    return new JabberConfig(NULL, this, false);
}

bool JabberClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign.toULong() != JABBER_SIGN)
        return false;
    QString resource;
    JabberUserData *data = toJabberUserData(_data);
    JabberUserData *my_data = findContact(data->ID.str(), QString::null, false, contact, resource);
    if (my_data){
        data = my_data;
    }else{
        contact = NULL;
    }
    return true;
}

bool JabberClient::createData(clientData *&_data, Contact *contact)
{
    JabberUserData *data = toJabberUserData(_data);
    JabberUserData *new_data = toJabberUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    new_data->ID.str() = data->ID.str();
    _data = (clientData*)new_data;
    return true;
}

void JabberClient::connect_ready()
{
    if (!getUseSSL() || m_bSSL){
        connected();
        return;
    }
    m_bSSL = true;
    // FIXME HACKHACKHACK!!!11 alarm
    SSLClient *ssl = new JabberSSL(socket()->socket(), (bool)!(getServer().compare("talk.google.com")));
    socket()->setSocket(ssl);
    ssl->startEncryption();
}

void JabberClient::connected()
{
    socket()->readBuffer().init(0);
    socket()->readBuffer().packetStart();
    socket()->setRaw(true);
    log(L_DEBUG, "Connect ready");
    startHandshake();
    TCPClient::connect_ready();
    reset();
}

void JabberClient::packet_ready()
{
    if (socket()->readBuffer().writePos() == 0)
        return;
    JabberPlugin *plugin = static_cast<JabberPlugin*>(protocol()->plugin());
    EventLog::log_packet(socket()->readBuffer(), false, plugin->JabberPacket);
	//log(L_DEBUG, "JABBER PACKET: %s\n", socket()->readBuffer().data());
    if (!parse(socket()->readBuffer(), true))
        socket()->error_state("XML parse error");
    socket()->readBuffer().init(0);
    socket()->readBuffer().packetStart();
}

bool JabberClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    switch (e->type()) {
    case eEventAddContact: {
        EventAddContact *ec = static_cast<EventAddContact*>(e);
        EventAddContact::AddContact *ac = ec->addContact();
        if (!ac->proto.isEmpty() && (protocol()->description()->text == ac->proto)){
            Contact *contact = NULL;
            QString resource;
            findContact(ac->addr, ac->nick, true, contact, resource);
            if (contact && contact->getGroup() != (int)ac->group){
                contact->setGroup(ac->group);
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }
            ec->setContact(contact);
            return true;
        }
        break;
    }
    case eEventDeleteContact: {
        EventDeleteContact *ec = static_cast<EventDeleteContact*>(e);
        QString addr = ec->alias();
        ContactList::ContactIterator it;
        Contact *contact;
        while ((contact = ++it) != NULL){
            JabberUserData *data;
            ClientDataIterator itc(contact->clientData, this);
            while ((data = toJabberUserData(++itc)) != NULL){
                if (data->ID.str() == addr){
                    contact->clientData.freeData(data);
                    ClientDataIterator itc(contact->clientData);
                    if (++itc == NULL)
                        delete contact;
                    return true;
                }
            }
        }
        break;
    }
    case eEventGoURL: {
        EventGoURL *u = static_cast<EventGoURL*>(e);
        QString url = u->url();
        QString proto;
        int n = url.indexOf(':');
        if (n < 0)
            return false;
        proto = url.left(n);
        if (proto != "jabber")
            return false;
        url = url.mid(n + 1);
        while (url.startsWith("/"))
            url = url.mid(1);
        QString s = unquoteString(url);
        QString jid = getToken(s, '/');
        if (!jid.isEmpty()){
            Contact *contact;
            QString resource;
            findContact(jid, s, true, contact, resource);
            Command cmd;
            cmd->id      = MessageGeneric;
            cmd->menu_id = MenuMessage;
            cmd->param	 = (void*)(contact->id());
            EventCommandExec(cmd).process();
            return true;
        }
        break;
    }
    case eEventTemplateExpanded: {
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        setStatus((unsigned long)(t->param), quoteString(t->tmpl, quoteNOBR, false));
        break;
    }
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                ClientDataIterator it(contact->clientData, this);
                JabberUserData *data;
                while ((data = toJabberUserData(++it)) != NULL){
                    listRequest(data, QString::null, QString::null, true);
                }
                break;
            }
            case EventContact::eChanged: {
                QString grpName;
                QString name;
                name = contact->getName();
                Group *grp = NULL;
                if (contact->getGroup())
                    grp = getContacts()->group(contact->getGroup());
                if (grp)
                    grpName = grp->getName();
                ClientDataIterator it(contact->clientData, this);
                JabberUserData *data;
                while ((data = toJabberUserData(++it)) != NULL){
                    if (grpName == data->Group.str()){
                        listRequest(data, name, grpName, false);
                        continue;
                    }
                    if (!data->Name.str().isEmpty()){
                        if (name == data->Name.str())
                            listRequest(data, name, grpName, false);
                        continue;
                    }
                    if (name == data->ID.str())
                        listRequest(data, name, grpName, false);
                }
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventGroup: {
        EventGroup *ev = static_cast<EventGroup*>(e);
        if (ev->action() != EventGroup::eChanged) 
            return false;
        Group *grp = ev->group();
        QString grpName = grp->getName();
        ContactList::ContactIterator itc;
        Contact *contact;
        while ((contact = ++itc) != NULL){
            if (contact->getGroup() != (int)grp->id())
                continue;
            ClientDataIterator it(contact->clientData, this);
            JabberUserData *data;
            while ((data = toJabberUserData(++it)) != NULL){
                if (grpName == data->Group.str())
                    listRequest(data, contact->getName(), grpName, false);
            }
        }
        break;
    }
    case eEventMessageCancel: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        for (list<Message*>::iterator it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
            if ((*it) == msg){
                m_waitMsg.erase(it);
                delete msg;
                return true;
            }
        }
        break;
    }
    case eEventMessageAccept: {
        EventMessageAccept *ema = static_cast<EventMessageAccept*>(e);
        for (list<Message*>::iterator it = m_ackMsg.begin(); it != m_ackMsg.end(); ++it){
            if ((*it)->id() == ema->msg()->id()){
                JabberFileMessage *msg = static_cast<JabberFileMessage*>(*it);
                m_ackMsg.erase(it);
                Contact *contact;
                QString resource;
                JabberUserData *data = findContact(msg->getFrom(), QString::null, false, contact, resource);
                if (data){
                    JabberFileTransfer *ft = new JabberFileTransfer(static_cast<FileMessage*>(msg), data, this);
                    ft->setDir(ema->dir());
                    ft->setOverwrite(ema->mode());
                    EventMessageAcked(msg).process();
                    ft->connect();
                }
                EventMessageDeleted(msg).process();
                if (data == NULL)
                    delete msg;
                return true;
            }
        }
        break;
    }
    case eEventMessageDecline: {
        EventMessageDecline *emd = static_cast<EventMessageDecline*>(e);
        for (list<Message*>::iterator it = m_ackMsg.begin(); it != m_ackMsg.end(); ++it){
            if ((*it)->id() == emd->msg()->id()){
                JabberFileMessage *msg = static_cast<JabberFileMessage*>(*it);
                m_ackMsg.erase(it);
                QString reason = emd->reason();
                if (reason.isEmpty())
                    reason = i18n("File transfer declined");
                ServerRequest req(this, "error", NULL, msg->getFrom(), msg->getID());
                req.start_element("error");
                req.add_attribute("code", "403");
                req.add_text(reason);
                req.send();
                EventMessageDeleted(msg).process();
                delete msg;
                return true;
            }
        }
        break;
    }
    case eEventClientVersion: {
        EventClientVersion *ecv = static_cast<EventClientVersion*>(e);
        ClientVersionInfo* info = ecv->info();
        if (!info->jid.isEmpty()){
            Contact *contact;
            QString resource;
            JabberUserData* data = findContact(info->jid, QString::null, false, contact, resource);
            if (!data)
                return false;
            unsigned i;
            for (i = 1; i <= data->nResources.toULong(); i++){
                if (resource == get_str(data->Resources, i))
                    break;
            }
            if (i <= data->nResources.toULong()){
                set_str(&data->ResourceClientName, i, info->name);
                set_str(&data->ResourceClientVersion, i, info->version);
                set_str(&data->ResourceClientOS, i, info->os);
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void JabberClient::changeStatus(const SIM::IMStatusPtr& status)
{
    QDateTime now = QDateTime::currentDateTime();
    data.owner.StatusTime.asULong() = now.toTime_t();
    if (currentStatus()->id() == "offline")
        data.owner.OnlineTime.asULong() = now.toTime_t();
    TCPClient::changeStatus(status);
    QSharedPointer<JabberStatus> jabberstatus = status.dynamicCast<JabberStatus>();
    socket()->writeBuffer().packetStart();
    QString priority = QString::number(getPriority());
    QString show = jabberstatus->show();
    QString type = jabberstatus->type();

    if (getInvisible()) {
        type = "invisible";
    }
    socket()->writeBuffer() << "<presence";
    if (!type.isEmpty())
        socket()->writeBuffer() << " type=\'" << type << "\'";
    socket()->writeBuffer() << ">\n";
    if (!show.isEmpty())
        socket()->writeBuffer() << "<show>" << show << "</show>\n";
    if (!status->text().isEmpty())
        socket()->writeBuffer() << "<status>" << status->text() << "</status>\n";
    if (!priority.isEmpty())
        socket()->writeBuffer() << "<priority>" << priority << "</priority>\n";
    socket()->writeBuffer() << "</presence>";
    sendPacket();
    EventClientChanged(this).process();
    if (status->id() == "offline") {
        if (socket()){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                    << "</stream:stream>\n";
            sendPacket();
        }
        // TODO
        /*
        Contact *contact;
        ContactList::ContactIterator it;
        QDateTime now(QDateTime::currentDateTime());
        data.owner.StatusTime.asULong() = now.toTime_t();
        while ((contact = ++it) != NULL) {
            JabberUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = toJabberUserData(++it)) != NULL){
                if (data->Status.toULong() == STATUS_OFFLINE)
                    continue;
                data->StatusTime.asULong() = now.toTime_t();
                setOffline(data);
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(dataName(data));
                m->setFlags(MESSAGE_RECEIVED);
                m->setStatus(STATUS_OFFLINE);
                EventMessageReceived e(m);
                if(!e.process())
                    delete m;
            }
        }
        */
    }
}

void JabberClient::setStatus(unsigned status)
{
    if (getInvisible() && (status != STATUS_OFFLINE))
	{
        if (m_status != status)
		{
            m_status = status;
            EventClientChanged(this).process();
        }
        return;
    }
    ARRequest ar;
    ar.contact  = NULL;
    ar.status   = status;
    ar.receiver = this;
    ar.param	= (void*)(long)status;
    EventARRequest(&ar).process();
}

void JabberClient::setStatus(unsigned status, const QString &ar)
{
    if (status  != m_status) {
        QDateTime now = QDateTime::currentDateTime();
        data.owner.StatusTime.asULong() = now.toTime_t();
        if (m_status == STATUS_OFFLINE)
            data.owner.OnlineTime.asULong() = now.toTime_t();
        m_status = status;
        socket()->writeBuffer().packetStart();
        QString priority = QString::number(getPriority());
        const char *show = NULL;
        const char *type = NULL;
        if (getInvisible()){
            type = "invisible";
        }else{
            switch (status){
            case STATUS_AWAY:
                show = "away";
                break;
            case STATUS_NA:
                show = "xa";
                break;
            case STATUS_DND:
                show = "dnd";
                break;
            case STATUS_OCCUPIED:
                show = "occupied";
                break;
            case STATUS_FFC:
                show = "chat";
                break;
            case STATUS_OFFLINE:
                priority = QString::null;
                type = "unavailable";
                break;
            }
        }
        socket()->writeBuffer() << "<presence";
        if (type)
            socket()->writeBuffer() << " type=\'" << type << "\'";
        socket()->writeBuffer() << ">\n";
        if (show)
            socket()->writeBuffer() << "<show>" << show << "</show>\n";
        if (!ar.isEmpty())
            socket()->writeBuffer() << "<status>" << ar << "</status>\n";
        if (!priority.isEmpty())
            socket()->writeBuffer() << "<priority>" << priority << "</priority>\n";
        socket()->writeBuffer() << "</presence>";
        sendPacket();
        EventClientChanged(this).process();
    }
    if (status == STATUS_OFFLINE){
        if (socket()){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
            << "</stream:stream>\n";
            sendPacket();
        }
        Contact *contact;
        ContactList::ContactIterator it;
        QDateTime now(QDateTime::currentDateTime());
        data.owner.StatusTime.asULong() = now.toTime_t();
        while ((contact = ++it) != NULL){
            JabberUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = toJabberUserData(++it)) != NULL){
                if (data->Status.toULong() == STATUS_OFFLINE)
                    continue;
                data->StatusTime.asULong() = now.toTime_t();
                setOffline(data);
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(dataName(data));
                m->setFlags(MESSAGE_RECEIVED);
                m->setStatus(STATUS_OFFLINE);
                EventMessageReceived e(m);
                if(!e.process())
                    delete m;
            }
        }
    }
}

void JabberClient::disconnected()
{
    for (list<ServerRequest*>::iterator it = m_requests.begin(); it != m_requests.end(); ++it)
        delete *it;
    m_requests.clear();
    if (m_curRequest){
        delete m_curRequest;
        m_curRequest = NULL;
    }
    list<Message*>::iterator itm;
    for (itm = m_ackMsg.begin(); itm != m_ackMsg.end(); ++itm){
        Message *msg = *itm;
        EventMessageDeleted(msg).process();
        delete msg;
    }
    for (itm = m_waitMsg.begin(); itm != m_waitMsg.end(); itm = m_waitMsg.begin()){
        Message *msg = *itm;
        msg->setError(I18N_NOOP("Client go offline"));
        EventMessageSent(msg).process();
        delete msg;
    }
    m_ackMsg.clear();
    init();
}

void JabberClient::init()
{
    m_id = QString::null;
    m_depth = 0;
    m_id_seed = 0xAAAA;
    m_bSSL = false;
}

void JabberClient::sendPacket()
{
    JabberPlugin *plugin = static_cast<JabberPlugin*>(protocol()->plugin());
    EventLog::log_packet(socket()->writeBuffer(), true, plugin->JabberPacket);
    socket()->write();
}

void JabberClient::element_start(const QString& el, const QXmlAttributes& attrs)
{
    QString element = el.toLower();
    QString id;
    if (m_depth)
	{
        if (m_curRequest)
		{
            m_curRequest->element_start(element, attrs);
        }
		else
		{
            if (element == "iq")
			{
                QString id = attrs.value("id");
                QString type = attrs.value("type");
				//log(L_DEBUG, "IQ: type = %s", type.toUtf8().data());
                if(id.isEmpty() || type == "set" || type == "get")
				{
                    m_curRequest = new IqRequest(this);
                    m_curRequest->element_start(element, attrs);
                }
				else
				{
                    list<ServerRequest*>::iterator it;
                    for (it = m_requests.begin(); it != m_requests.end(); ++it)
					{
                        if ((*it)->m_id == id)
                            break;
                    }
                    if (it != m_requests.end())
					{
                        m_curRequest = *it;
                        m_requests.erase(it);
                        m_curRequest->element_start(element, attrs);
                    }
					else
					{
                        log(L_WARN, "Packet %s not found", id.toLatin1().data());
                    }
                }
            }
			else if (element == "presence")
			{
                m_curRequest = new PresenceRequest(this);
                m_curRequest->element_start(element, attrs);
            }
			else if (element == "message")
			{
                m_curRequest = new MessageRequest(this);
                m_curRequest->element_start(element, attrs);
            }
			else if (element == "stream:error")
			{
                m_curRequest = new StreamErrorRequest(this);
                m_curRequest->element_start(element, attrs);
            }
			else if (element != "a")
			{
                log(L_DEBUG, "Bad tag %s", qPrintable(element));
            }
        }
    }else{
        if (element == "stream:stream"){
            id = attrs.value("id");
        }
        log(L_DEBUG, "Handshake %s (%s)", qPrintable(id), qPrintable(element));
        handshake(id);
    }
    m_depth++;
}

void JabberClient::element_end(const QString& el)
{
    m_depth--;
    if (m_curRequest){
        QString element = el.toLower();
        m_curRequest->element_end(element);
        if (m_depth == 1){
            delete m_curRequest;
            m_curRequest = NULL;
        }
    }
}

void JabberClient::char_data(const QString& str)
{
    if (m_curRequest)
        m_curRequest->char_data(str);
}

QString JabberClient::get_unique_id()
{
    QString s("a");
	s += QString::number(m_id_seed,16);
    m_id_seed += 0x10;
    return s;
}

JabberClient::ServerRequest::ServerRequest(JabberClient *client, const char *type,
        const QString &from, const QString &to, const QString &id)
{
    m_client = client;
    if (type == NULL)
        return;
    m_id = id.isEmpty() ? m_client->get_unique_id() : id;

    if (m_client->socket() == NULL)
        return;
    m_client->socket()->writeBuffer().packetStart();
    m_client->socket()->writeBuffer() << "<iq type=\'" << encodeXMLattr(type) << "\'";
    m_client->socket()->writeBuffer() <<" id=\'" << encodeXMLattr(m_id) << "\'";

    if (!from.isEmpty())
        m_client->socket()->writeBuffer() << " from=\'" << encodeXMLattr(from) << "\'";
    if (!to.isEmpty())
        m_client->socket()->writeBuffer() << " to=\'" << encodeXMLattr(to) << "\'";
    m_client->socket()->writeBuffer() << ">\n";
}

JabberClient::ServerRequest::~ServerRequest()
{
}

void JabberClient::ServerRequest::send()
{
    end_element(false);
    while (!m_els.isEmpty()){
        end_element(false);
    }
    m_client->socket()->writeBuffer()
    << "</iq>\n";
    m_client->sendPacket();
}

void JabberClient::ServerRequest::element_start(const QString&, const QXmlAttributes&)
{
}

void JabberClient::ServerRequest::element_end(const QString&)
{
}

void JabberClient::ServerRequest::char_data(const QString&)
{
}

void JabberClient::ServerRequest::start_element(const QString &name)
{
    end_element(true);
    m_client->socket()->writeBuffer() << "<" << name;
    m_element = name;
}

void JabberClient::ServerRequest::add_attribute(const QString &name, const QString &value)
{
    if(value.isEmpty())
        return;
    m_client->socket()->writeBuffer()
        << " " << name
        << "=\'" << JabberClient::encodeXMLattr(value) << "\'";
}

void JabberClient::ServerRequest::add_attribute(const QString &name, const char *value)
{
    if(value)
        add_attribute(name, QString::fromUtf8(value));
}

void JabberClient::ServerRequest::end_element(bool bNewLevel)
{
    if (bNewLevel){
        if (m_element.length()){
            m_client->socket()->writeBuffer() << ">\n";
            m_els.push(m_element);
        }
    }else{
        if (m_element.length()){
            m_client->socket()->writeBuffer() << "/>\n";
        }else if (m_els.count()){
            m_element = m_els.top();
            m_els.pop();
            m_client->socket()->writeBuffer() << "</" << m_element << ">\n";
        }
    }
    m_element = QString::null;
}

void JabberClient::ServerRequest::add_text(const QString &value)
{
    if (m_element.length()){
        m_client->socket()->writeBuffer() << ">";
        m_els.push(m_element);
        m_element = QString::null;
    }
    m_client->socket()->writeBuffer() << JabberClient::encodeXML(value);
}

void JabberClient::ServerRequest::text_tag(const QString &name, const QString &value)
{
    if (value.isEmpty())
        return;
    end_element(true);
    m_client->socket()->writeBuffer()
        << "<" << name << ">"
        << JabberClient::encodeXML(value)
        << "</" << name << ">\n";
}

void JabberClient::ServerRequest::add_condition(const QString &condition, bool bXData)
{
    QString cond = condition;
    while (cond.length()){
        QString item = getToken(cond, ';');
        if (item == "x:data"){
            bXData = true;
            start_element("x");
            add_attribute("xmlns", "jabber:x:data");
            add_attribute("type", "submit");
        }
        QString key = getToken(item, '=');
        if (bXData){
            start_element("field");
            add_attribute("var", key);
            text_tag("value", item);
            end_element();
        }else{
            text_tag(key, item);
        }
    }
}

const char *JabberClient::ServerRequest::_GET = "get";
const char *JabberClient::ServerRequest::_SET = "set";
const char *JabberClient::ServerRequest::_RESULT = "result";

void JabberClient::startHandshake()
{
    socket()->writeBuffer().packetStart();
    socket()->writeBuffer()
        << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<stream:stream to=\'"
        << encodeXML(VHost())
        << "\' xmlns=\'jabber:client\' xmlns:stream=\'http://etherx.jabber.org/streams\'>\n";
    sendPacket();
}

void JabberClient::handshake(const QString &id)
{
    if (id.isEmpty()){
        socket()->error_state("Bad session ID");
        return;
    }
    m_id = id;
    if (getRegister()){
        auth_register();
    }else{
        if (getUsePlain()){
            auth_plain();
        }else{
            auth_digest();
        }
    }
}

void JabberClient::auth_ok()
{
    if (getRegister()){
        setRegister(false);
        setClientStatus(STATUS_OFFLINE);
        TCPClient::setStatus(getManualStatus(), getCommonStatus());
        return;
    }
    setState(Connected);
    setPreviousPassword(QString::null);
    rosters_request();
    if (getInfoUpdated()){
        setClientInfo(&data.owner);
    }else{
        info_request(NULL, false);
    }
    setStatus(m_logonStatus);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

void JabberClient::auth_failed()
{
    m_reconnect = NO_RECONNECT;
    socket()->error_state(I18N_NOOP("Login failed"), AuthError);
}

QString JabberClient::encodeXML(const QString &str)
{
    return quoteString(str, quoteNOBR, false);
}
QString JabberClient::encodeXMLattr(const QString &str)
{
    return quoteString(str, quoteXMLattr, false);
}

JabberUserData *JabberClient::findContact(const QString &_jid, const QString &name, bool bCreate, Contact *&contact, QString &resource, bool bJoin)
{
    resource = QString::null;
    QString jid = _jid;
    int n = jid.indexOf('/');
    if (n >= 0){
        resource = jid.mid(n + 1);
        jid = jid.left(n);
    }
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        JabberUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toJabberUserData(++it)) != NULL){
          if (jid.toUpper() != data->ID.str().toUpper())
                continue;
            if (!resource.isEmpty())
                data->Resource.str() = resource;
            if (!name.isEmpty())
                data->Name.str() = name;
            return data;
        }
    }
    if (!bCreate)
        return NULL;
    it.reset();
    QString sname;
    if (!name.isEmpty()){
        sname = name;
    }else{
        sname = jid;
        int pos = sname.indexOf('@');
        if (pos > 0)
            sname = sname.left(pos);
    }
    if (bJoin){
        while ((contact = ++it) != NULL){
            if (contact->getName().toLower() == sname.toLower()){
                JabberUserData *data = toJabberUserData((SIM::clientData*) contact->clientData.createData(this)); // FIXME unsafe type conversion
                data->ID.str() = jid;
                if (!resource.isEmpty())
                    data->Resource.str() = resource;
                if (!name.isEmpty())
                    data->Name.str() = name;
                info_request(data, false);
                EventContact e(contact, EventContact::eChanged);
                e.process();
                m_bJoin = true;
                return data;
            }
        }
    }
    contact = getContacts()->contact(0, true);
    JabberUserData *data = toJabberUserData((SIM::clientData*) contact->clientData.createData(this)); // FIXME unsafe type conversion
    data->ID.str() = jid;
    if (!resource.isEmpty())
        data->Resource.str() = resource;
    if (!name.isEmpty())
        data->Name.str() = name;
    contact->setName(sname);
    info_request(data, false);
    EventContact e(contact, EventContact::eChanged);
    e.process();
    return data;
}

static void addIcon(QSet<QString> *s, const QString &icon, const QString &statusIcon)
{
    if (!s || statusIcon == icon)
        return;
    s->insert(icon);
}

QString JabberClient::get_icon(JabberUserData *data, unsigned status, bool invisible)
{
    const CommandDef *def = protocol()->statusList();
    for (; !def->text.isNull(); def++){
        if (def->id == status)
            break;
    }
    if ((def == NULL) || (def->text.isNull()))
        return "Jabber_offline";
    QString dicon = def->icon;
    if (invisible)
        dicon = "Jabber_invisible";
    if (getProtocolIcons()){
        QString id = data->ID.str();
        int host = id.indexOf( '@' );

        QString h;
        if (host != -1)
            h = id.mid(host + 1);
        else
            h = id;

            int p = h.indexOf( '.' );
            if (p)
                h = h.left( p );
            if (h == "icq"){
                if (invisible){
                    dicon = "ICQ_invisible";
                }else{
                    switch (status){
                    case STATUS_ONLINE:
                        dicon = "ICQ_online";
                        break;
                    case STATUS_OFFLINE:
                        dicon = "ICQ_offline";
                        break;
                    case STATUS_AWAY:
                        dicon = "ICQ_away";
                        break;
                    case STATUS_NA:
                        dicon = "ICQ_na";
                        break;
                    case STATUS_DND:
                        dicon = "ICQ_dnd";
                        break;
                    case STATUS_OCCUPIED:
                        dicon = "ICQ_occupied";
                        break;
                    case STATUS_FFC:
                        dicon = "ICQ_ffc";
                        break;
                    }
                }
            }else if (h == "aim"){
                switch (status){
                case STATUS_ONLINE:
                    dicon = "AIM_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "AIM_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "AIM_away";
                    break;
                }
            }else if (h == "msn"){
                if (invisible){
                    dicon = "MSN_invisible";
                }else{
                    switch (status){
                    case STATUS_ONLINE:
                        dicon = "MSN_online";
                        break;
                    case STATUS_OFFLINE:
                        dicon = "MSN_offline";
                        break;
                    case STATUS_AWAY:
                        dicon = "MSN_away";
                        break;
                    case STATUS_NA:
                        dicon = "MSN_na";
                        break;
                    case STATUS_DND:
                        dicon = "MSN_dnd";
                        break;
                    case STATUS_OCCUPIED:
                        dicon = "MSN_occupied";
                        break;
                    }
                }
            }else if (h == "yahoo"){
                switch (status){
                case STATUS_ONLINE:
                    dicon = "Yahoo!_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "Yahoo!_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "Yahoo!_away";
                    break;
                case STATUS_NA:
                    dicon = "Yahoo!_na";
                    break;
                case STATUS_DND:
                    dicon = "Yahoo!_dnd";
                    break;
                case STATUS_OCCUPIED:
                    dicon = "Yahoo!_occupied";
                    break;
                case STATUS_FFC:
                    dicon = "Yahoo!_ffc";
                    break;
                }
            }else if (h == "sms"){
                switch (status){
                case STATUS_ONLINE:
                    dicon = "sms_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "sms_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "sms_away";
                    break;
                case STATUS_NA:
                    dicon = "sms_na";
                    break;
                case STATUS_DND:
                    dicon = "sms_dnd";
                    break;
                case STATUS_OCCUPIED:
                    dicon = "sms_occupied";
                    break;
                case STATUS_FFC:
                    dicon = "sms_ffc";
                    break;
                }
            }else if ((h == "x-gadugadu") || (h == "gg")){
                switch (status){
                case STATUS_ONLINE:
                    dicon = "GG_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "GG_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "GG_away";
                    break;
                case STATUS_NA:
                    dicon = "GG_na";
                    break;
                case STATUS_DND:
                    dicon = "GG_dnd";
                    break;
                case STATUS_OCCUPIED:
                    dicon = "GG_occupied";
                    break;
                case STATUS_FFC:
                    dicon = "GG_ffc";
                    break;
                }
            }
    }
    return dicon;
}

void JabberClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QSet<QString> *icons)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString dicon = get_icon(data, data->Status.toULong(), data->invisible.toBool());
    if (data->Status.toULong() > curStatus)
	{
        curStatus = data->Status.toULong();
        if(!statusIcon.isEmpty() && icons)
        {
            icons->insert(statusIcon);
        }
        statusIcon = dicon;
    }
	else
	{
        if (!statusIcon.isEmpty())
		{
            addIcon(icons, dicon, statusIcon);
        }
		else
		{
            statusIcon = dicon;
        }
    }
    for (unsigned i = 1; i <= data->nResources.toULong(); i++){
        QString dicon = get_icon(data, get_str(data->ResourceStatus, i).toUInt(), false);
        addIcon(icons, dicon, statusIcon);
    }
    if (((data->Subscribe.toULong() & SUBSCRIBE_TO) == 0) && !isAgent(data->ID.str()))
        style |= CONTACT_UNDERLINE;
    if (icons && data->IsTyping.toBool())
        addIcon(icons, "typing", statusIcon);
}

QString JabberClient::buildId(JabberUserData *data)
{
    return data->ID.str();
}

QWidget *JabberClient::searchWindow(QWidget *parent)
{
    if (getState() != Connected)
        return NULL;
    return new JabberAdd(this, parent);
}

void JabberClient::ping()
{
    if (getState() != Connected)
        return;
    socket()->writeBuffer().packetStart();
    socket()->writeBuffer() << "\n";
    sendPacket();
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

QString JabberClient::contactName(void *clientData)
{
    QString res = Client::contactName(clientData);
    res += ": ";
    JabberUserData *data = toJabberUserData((SIM::clientData*)clientData); // FIXME unsafe type conversion
    QString name = data->ID.str();
    if (!data->Nick.str().isEmpty()){
        res += data->Nick.str();
        res += " (";
        res += name;
        res += ')';
    }else{
        res += name;
    }
    return res;
}


QString JabberClient::contactTip(void *_data)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString res;
    if (data->nResources.toULong() == 0){
        res = "<img src=\"sim:icons/";
        res += get_icon(data, STATUS_OFFLINE, data->invisible.toBool());
        res += "\">";
        res += i18n("Offline");
        res += "<br/>";
        res += "ID: <b>";
        res += data->ID.str();
        if (!data->Resource.str().isEmpty()){
            res += '/';
            res += data->Resource.str();
        }
        res += "</b>";

        if (data->StatusTime.toULong()){
            res += "<br/><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
        QString &reply = data->AutoReply.str();
        if (!reply.isEmpty()){
            res += "<br/>";
            res += reply.replace('\n', "<br/>");
        }
    }else{
        for (unsigned i = 1; i <= data->nResources.toULong(); i++){
            unsigned status = get_str(data->ResourceStatus, i).toUInt();
            res += "<img src=\"sim:icons/";
            res += get_icon(data, status, false);
            res += "\">";
            QString statusText;
            for (const CommandDef *cmd = protocol()->statusList(); !cmd->text.isEmpty(); cmd++){
                if (cmd->id == status){
                    statusText = i18n(cmd->text);
                    res += statusText;
                    break;
                }
            }
            res += "<br/>ID: <b>";
            res += data->ID.str();
            QString resource = get_str(data->Resources, i);
            if (!resource.isEmpty()){
                res += '/';
                res += resource;
            }
            res += "</b>";

            unsigned onlineTime = get_str(data->ResourceOnlineTime, i).toUInt();
            unsigned statusTime = get_str(data->ResourceStatusTime, i).toUInt();
            if (onlineTime){
                res += "<br/><font size=-1>";
                res += i18n("Online");
                res += ": </font>";
                res += formatDateTime(onlineTime);
            }
            if (statusTime != onlineTime){
                res += "<br/><font size=-1>";
                res += statusText;
                res += ": </font>";
                res += formatDateTime(statusTime);
            }

            QString clientName = get_str(data->ResourceClientName, i);
            QString clientVersion = get_str(data->ResourceClientVersion, i);
            QString clientOS = get_str(data->ResourceClientOS, i);
            if (!clientName.isEmpty()) {
                res += "<br/>" + clientName + ' ' + clientVersion;
                if (!clientOS.isEmpty())
                    res += " / " + clientOS;
            }

            const QString &reply = get_str(data->ResourceReply, i);
            if (!reply.isEmpty()){
                res += "<br/><br/>";
                QString r = reply;
                r = r.replace('\n', "<br/>");
                res += r;
            }
            if (i < data->nResources.toULong())
                res += "<br>_________<br>";
        }
    }

    if (data->LogoWidth.toLong() && data->LogoHeight.toLong()){
        QString logoFileName = logoFile(data);
        QImage img(logoFileName);
        if (!img.isNull()){
            QPixmap pict = QPixmap::fromImage(img);
            int w = pict.width();
            int h = pict.height();
            if (h > w){
                if (h > 60){
                    w = w * 60 / h;
                    h = 60;
                }
            }else{
                if (w > 60){
                    h = h * 60 / w;
                    w = 60;
                }
            }
            res += "<br/><img src=\"" + logoFileName + "\" width=\"";
            res += QString::number(w);
            res += "\" height=\"";
            res += QString::number(h);
            res += "\">";
        }
    }
    if (data->PhotoWidth.toLong() && data->PhotoHeight.toLong()){
       QString photoFileName = photoFile(data);
       QImage img(photoFileName);
        if (!img.isNull()){
            QPixmap pict = QPixmap::fromImage(img);
            int w = pict.width();
            int h = pict.height();
            if (h > w){
                if (h > 60){
                    w = w * 60 / h;
                    h = 60;
                }
            }else{
                if (w > 60){
                    h = h * 60 / w;
                    w = 60;
                }
            }
            res += "<br/><img src=\"" + photoFileName + "\" width=\"";
            res += QString::number(w);
            res += "\" height=\"";
            res += QString::number(h);
            res += "\">";
        }
    }
    return res;
}

void JabberClient::setOffline(JabberUserData *data)
{
    data->Status.asULong()    = STATUS_OFFLINE;
    data->composeId.asULong() = 0;
    data->Resources.clear();
    data->ResourceReply.clear();
    data->ResourceStatus.clear();
    data->ResourceStatusTime.clear();
    data->ResourceOnlineTime.clear();
    data->nResources.asULong() = 0;
    data->TypingId.str() = QString::null;
    if (data->IsTyping.toBool()){
        data->IsTyping.asBool() = false;
        Contact *contact;
        QString resource;
        if (findContact(data->ID.str(), QString::null, false, contact, resource)){
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
    }
}

const unsigned MAIN_INFO  = 1;
const unsigned HOME_INFO  = 2;
const unsigned WORK_INFO  = 3;
const unsigned ABOUT_INFO = 4;
const unsigned PHOTO_INFO = 5;
const unsigned LOGO_INFO  = 6;
const unsigned NETWORK	  = 7;

static CommandDef jabberWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "Jabber_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PHOTO_INFO,
            I18N_NOOP("Photo"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            LOGO_INFO,
            I18N_NOOP("Logo"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

static CommandDef cfgJabberWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "Jabber_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PHOTO_INFO,
            I18N_NOOP("Photo"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            LOGO_INFO,
            I18N_NOOP("Logo"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            NETWORK,
            I18N_NOOP("Network"),
            "network",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

CommandDef *JabberClient::infoWindows(Contact*, void *_data)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString name = i18n(protocol()->description()->text);
    name += ' ';
    name += data->ID.str();
    jabberWnd[0].text_wrk = name;
    return jabberWnd;
}

CommandDef *JabberClient::configWindows()
{
    QString title = name();
    int n = title.indexOf('.');
    if (n > 0)
        title = title.left(n) + ' ' + title.mid(n + 1);
    cfgJabberWnd[0].text_wrk = title;
    return cfgJabberWnd;
}

QWidget *JabberClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    switch (id){
    case MAIN_INFO:
        return new JabberInfo(parent, data, this);
    case HOME_INFO:
        return new InfoProxy(parent, new JabberHomeInfo(parent, data, this), i18n("Home info"));
    case WORK_INFO:
        return new InfoProxy(parent, new JabberWorkInfo(parent, data, this), i18n("Work info"));
    case ABOUT_INFO:
        return new InfoProxy(parent, new JabberAboutInfo(parent, data, this), i18n("About info"));
    case PHOTO_INFO:
        return new JabberPicture(parent, data, this, true);
    case LOGO_INFO:
        return new JabberPicture(parent, data, this, false);
    }
    return NULL;
}

QWidget *JabberClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new JabberInfo(parent, NULL, this);
    case HOME_INFO:
        return new InfoProxy(parent, new JabberHomeInfo(parent, NULL, this), i18n("Home info"));
    case WORK_INFO:
        return new InfoProxy(parent, new JabberWorkInfo(parent, NULL, this), i18n("Work info"));
    case ABOUT_INFO:
        return new InfoProxy(parent, new JabberAboutInfo(parent, NULL, this), i18n("About info"));
    case PHOTO_INFO:
        return new JabberPicture(parent, NULL, this, true);
    case LOGO_INFO:
        return new JabberPicture(parent, NULL, this, false);
    case NETWORK:
        return new JabberConfig(parent, this, true);
    }
    return NULL;
}

void JabberClient::updateInfo(Contact *contact, void *data)
{
    if (getState() != Connected){
        Client::updateInfo(contact, data);
        return;
    }
    if (data == NULL)
        data = &this->data.owner;
    info_request(toJabberUserData((SIM::clientData*)data), false); // FIXME unsafe type conversion
}

QString JabberClient::resources(void *_data)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString resource;
    if (data->nResources.toULong() > 1){
        for (unsigned i = 1; i <= data->nResources.toULong(); i++){
            if (!resource.isEmpty())
                resource += ';';
            QString dicon = get_icon(data, get_str(data->ResourceStatus, i).toUInt(), false);
            resource += dicon; //QString::number(dicon);
            resource += ',';
            resource += quoteChars(get_str(data->Resources, i), ";");
        }
    }
    return resource;
}

bool JabberClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.toULong() != JABBER_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    switch (type)
	{
    case MessageGeneric:
    case MessageFile:
    case MessageContacts:
    case MessageUrl:
        return true;
    case MessageAuthRequest:
        return ((data->Subscribe.toULong() & SUBSCRIBE_TO) == 0);
    case MessageAuthGranted:
        return ((data->Subscribe.toULong() & SUBSCRIBE_FROM) == 0);
    case MessageAuthRefused:
        return (data->Subscribe.toULong() & SUBSCRIBE_FROM);
    case MessageJabberOnline:
        return isAgent(data->ID.str()) && (data->Status.toULong() == STATUS_OFFLINE);
    case MessageJabberOffline:
        return isAgent(data->ID.str()) && (data->Status.toULong() != STATUS_OFFLINE);
    }
    return false;
}

class JabberImageParser : public HTMLParser
{
public:
    JabberImageParser(unsigned bgColor);
    QString parse(const QString &text);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    void startBody(const list<QString> &attrs);
    void endBody();
    QString res;
    bool		m_bPara;
    bool		m_bBody;
    unsigned	m_bgColor;
};

JabberImageParser::JabberImageParser(unsigned bgColor)
{
    m_bPara    = false;
    m_bBody    = true;
    m_bgColor  = bgColor;
}

QString JabberImageParser::parse(const QString &text)
{
    list<QString> attrs;
    startBody(attrs);
    HTMLParser::parse(text);
    endBody();
    return res;
}

void JabberImageParser::text(const QString &text)
{
    if (m_bBody)
        res += quoteString(text);
}

static const char *_tags[] =
    {
        "abbr",
        "acronym",
        "address",
        "blockquote",
        "cite",
        "code",
        "dfn",
        "div",
        "em",
        "h1",
        "h2",
        "h3",
        "h4",
        "h5",
        "h6",
        "kbd",
        "p",
        "pre",
        "q",
        "samp",
        "span",
        "strong",
        "var",
        "a",
        "dl",
        "dt",
        "dd",
        "ol",
        "ul",
        "li",
        NULL
    };

static const char *_styles[] =
    {
        "color",
        "background-color",
        "font-family",
        "font-size",
        "font-style",
        "font-weight",
        "text-align",
        "text-decoration",
        NULL
    };

void JabberImageParser::startBody(const list<QString> &attrs)
{
    m_bBody = true;
    res = QString::null;
    list<QString> newStyles;
    list<QString>::const_iterator it;
    for (it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        if (name == "style"){
            list<QString> styles = parseStyle(value);
            for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
                QString name = *it;
                ++it;
                QString value = *it;
                for (const char **s = _styles; *s; s++){
                    if (name == *s){
                        newStyles.push_back(name);
                        newStyles.push_back(value);
                        break;
                    }
                }
            }
        }
    }
    for (it = newStyles.begin(); it != newStyles.end(); ++it){
        QString name = *it;
        ++it;
        if (name == "background-color")
            break;
    }
    if (it == newStyles.end()){
        char b[15];
        sprintf(b, "#%06X", m_bgColor & 0xFFFFFF);
        newStyles.push_back("background-color");
        newStyles.push_back(b);
    }
    res += "<span style=\"";
    res += makeStyle(newStyles);
    res += "\">";
}

void JabberImageParser::endBody()
{
    if (m_bBody){
        res += "</span>";
        m_bBody = false;
    }
}

void JabberImageParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (tag == "html"){
        m_bBody = false;
        res = QString::null;
        return;
    }
    if (tag == "body"){
        startBody(attrs);
        return;
    }
    if (!m_bBody)
        return;
    if (tag == "img"){
        QString src;
        QString alt;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name == "src")
                src = value;
            if (name == "alt")
                alt = value;
        }
        if (!alt.isEmpty()){
            res += unquoteString(alt);
            return;
        }
        if (src.left(10) == "sim:icons/"){
            QStringList smiles = getIcons()->getSmile(src.mid(10));
            if (!smiles.empty()){
                res += smiles.front();
                return;
            }
        }
        text(alt);
        return;
    }
    if (tag == "p"){
        if (m_bPara){
            res += "<br/>";
            m_bPara = false;
        }
        return;
    }
    if (tag == "br"){
        res += "<br/>";
        return;
    }
    for (const char **t = _tags; *t; t++){
        if (tag == *t){
            res += '<';
            res += tag;
            for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
                QString name = *it;
                ++it;
                QString value = *it;
                if (name == "style"){
                    list<QString> styles = parseStyle(value);
                    list<QString> newStyles;
                    for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
                        QString name = *it;
                        ++it;
                        QString value = *it;
                        for (const char **s = _styles; *s; s++){
                            if (name == *s){
                                newStyles.push_back(name);
                                newStyles.push_back(value);
                                break;
                            }
                        }
                    }
                    value = makeStyle(newStyles);
                }
                if ((name != "style") && (name != "href"))
                    continue;
                res += ' ';
                res += name;
                if (!value.isEmpty()){
                    res += "=\'";
                    res += quoteString(value);
                    res += "\'";
                }
            }
            res += '>';
            return;
        }
    }
    if (tag == "b"){
        res += "<span style=\'font-weight:bold\'>";
        return;
    }
    if (tag == "i"){
        res += "<span style=\'font-style:italic\'>";
        return;
    }
    if (tag == "u"){
        res += "<span style=\'text-decoration:underline\'>";
        return;
    }
    if (tag == "font"){
        res += "<span";
        QString style;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name == "color"){
                if (!style.isEmpty())
                    style += ';';
                style += "color: ";
                style += value;
                continue;
            }
        }
        if (!style.isEmpty()){
            res += " style=\'";
            res += style;
            res += "\'";
        }
        res += '>';
        return;
    }
    return;
}

void JabberImageParser::tag_end(const QString &tag)
{
    if (tag == "body"){
        endBody();
        return;
    }
    if (!m_bBody)
        return;
    if (tag == "p"){
        m_bPara = true;
        return;
    }
    for (const char **t = _tags; *t; t++){
        if (tag == *t){
            res += "</";
            res += tag;
            res += '>';
            return;
        }
    }
    if ((tag == "b") || (tag == "i") || (tag == "u") || (tag == "font")){
        res += "</span>";
        return;
    }
}

static QString removeImages(const QString &text, unsigned bgColor)
{
    JabberImageParser p(bgColor);
    return p.parse(text);
}

bool JabberClient::send(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    switch (msg->type()){
    case MessageAuthRefused:{
            QString grp;
            Group *group = NULL;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact && contact->getGroup())
                group = getContacts()->group(contact->getGroup());
            if (group)
                grp = group->getName();
            listRequest(data, data->Name.str(), grp, false);
            if (data->Subscribe.toULong() & SUBSCRIBE_FROM){
                socket()->writeBuffer().packetStart();
                socket()->writeBuffer()
                    << "<presence to=\'"
                    << data->ID.str()
                    << "\' type=\'unsubscribed\'>\n<status>"
                    << encodeXML(msg->getPlainText())
                    << "</status>\n</presence>";
                sendPacket();
                if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                    msg->setClient(dataName(data));
                    EventSent(msg).process();
                }
                EventMessageSent(msg).process();
                delete msg;
                return true;
            }
        }
    case MessageGeneric:{
            Contact *contact = getContacts()->contact(msg->contact());
            if ((contact == NULL) || (data == NULL))
                return false;
            QString text = msg->getPlainText();
            EventSend e(msg, text.toUtf8());
            e.process();
            text = QString::fromUtf8( e.localeText() );
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<message type=\'chat\' to=\'"
                << data->ID.str();
            if(!msg->getResource().isEmpty())
			{
				socket()->writeBuffer() << QString("/") << encodeXMLattr(msg->getResource());
			}
            if (getTyping()){
                data->composeId.asULong() = ++m_msg_id;
                QString msg_id = "msg";
                msg_id += QString::number(data->composeId.asULong());
                socket()->writeBuffer()
                    << "\' id=\'" << msg_id;
            }
            if (text.startsWith("-----BEGIN PGP MESSAGE-----")){
				socket()->writeBuffer()
                    << "\'>\n<body>This message is encrypted.</body>\n";
			}else{
                socket()->writeBuffer()
                    << "\'>\n<body>" << encodeXML(text) << "</body>\n";
                if (data->richText.toBool() && getRichText() && (msg->getFlags() & MESSAGE_RICHTEXT)){
                    socket()->writeBuffer()
                        << "<html xmlns='http://jabber.org/protocol/xhtml-im'>\n<body>"
                        << removeImages(msg->getRichText(), msg->getBackground())
                        << "</body>\n</html>\n";
                }
            }
            if (getTyping()){
                socket()->writeBuffer()
					<< "<x xmlns='jabber:x:event'>\n"
					<< "<composing/>\n"
					<< "</x>\n";
            }
            if (text.startsWith("-----BEGIN PGP MESSAGE-----")){
                text.truncate(text.indexOf("\n-----END PGP MESSAGE-----"));
                socket()->writeBuffer()
                << "<x xmlns='jabber:x:encrypted'>"
                << text.remove(0, text.indexOf("\n\n") + 2)
                << "</x>\n";
            }
            socket()->writeBuffer()
            << "</message>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                if (data->richText.toBool()){
                    msg->setClient(dataName(data));
                    EventSent(msg).process();
                }else{
                    Message m(MessageGeneric);
                    m.setContact(msg->contact());
                    m.setClient(dataName(data));
                    m.setText(msg->getPlainText());
                    EventSent(msg).process();
                }
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageUrl:{
            Contact *contact = getContacts()->contact(msg->contact());
            if ((contact == NULL) || (data == NULL))
                return false;
            UrlMessage *m = static_cast<UrlMessage*>(msg);
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<message type=\'chat\' to=\'"
                << data->ID.str();
            if (!msg->getResource().isEmpty()){
                socket()->writeBuffer()
                    << QString("/") << encodeXMLattr(msg->getResource());
            }
            socket()->writeBuffer()
                << "\'>\n<body>" << encodeXML(m->getUrl());
            QString t = m->getPlainText();
            if (!t.isEmpty()){
                socket()->writeBuffer()
                    << "\n" << encodeXML(m->getPlainText());
            }
            socket()->writeBuffer()
            << "</body>\n";
            if (data->richText.toBool() && getRichText()){
                socket()->writeBuffer()
                    << "<html xmlns='http://jabber.org/protocol/xhtml-im'>\n<body>"
                    << "<a href=\'"
                    << encodeXML(m->getUrl()) << "\'>"
                    << encodeXML(m->getUrl()) << "</a>";
                if (!t.isEmpty()){
                    socket()->writeBuffer()
                    << "<br/>\n"
                    << removeImages(msg->getRichText(), msg->getBackground());
                }
                socket()->writeBuffer()
                << "</body>\n</html>\n";
            }
            socket()->writeBuffer()
            << "</message>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                if (data->richText.toBool()){
                    msg->setClient(dataName(data));
                    EventSent(msg).process();
                }else{
                    Message m(MessageGeneric);
                    m.setContact(msg->contact());
                    m.setClient(dataName(data));
                    m.setText(msg->getPlainText());
                    EventSent(msg).process();
                }
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageContacts:{
            Contact *contact = getContacts()->contact(msg->contact());
            if ((contact == NULL) || (data == NULL))
                return false;
            ContactsMessage *m = static_cast<ContactsMessage*>(msg);
            QStringList jids;
            QStringList names;
            QString contacts = m->getContacts();
            QString nc;
            while (!contacts.isEmpty()){
                QString item = getToken(contacts, ';');
                QString url = getToken(item, ',');
                QString proto = getToken(url, ':');
                if (proto == "sim"){
                    Contact *contact = getContacts()->contact(url.toLong());
                    if (contact){
                        clientData *data;
                        ClientDataIterator it(contact->clientData);
                        while ((data = ++it) != NULL){
                            Contact *c = contact;
                            if (!isMyData(data, c))
                                continue;
                            JabberUserData *d = toJabberUserData(data);
                            jids.append(d->ID.str());
                            names.append(c->getName());
                            if (!nc.isEmpty())
                                nc += ';';
                            nc += "jabber:";
                            nc += d->ID.str();
                            nc += ',';
                            if (c->getName() == d->ID.str()){
                                nc += d->ID.str();
                            }else{
                                nc += c->getName();
                                nc += " (";
                                nc += d->ID.str();
                                nc += ')';
                            }
                        }
                    }
                }
            }
            if (jids.isEmpty()){
                msg->setError(I18N_NOOP("No contacts for send"));
                EventMessageSent(msg).process();
                delete msg;
                return true;
            }
            m->setContacts(nc);
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<message type=\'chat\' to=\'"
                << data->ID.str();
            if (!msg->getResource().isEmpty()){
                socket()->writeBuffer()
                    << QString("/") << encodeXMLattr(msg->getResource());
            }
            socket()->writeBuffer()
                << "\'>\n<x xmlns='jabber:x:roster'>\n";
            QStringList::ConstIterator iti = jids.constBegin();
            QStringList::ConstIterator itn = names.constBegin();
            for (; iti != jids.constEnd(); ++iti, ++itn){
                socket()->writeBuffer()
                    << "<item name=\'"
                    << encodeXML(*itn)
                    << "\' jid=\'"
                    << encodeXML(*iti)
                    << "\'>\n<group/>\n</item>\n";
            }
            socket()->writeBuffer()
                << "</x>\n<body>";
            iti = jids.constBegin();
            for (; iti != jids.constEnd(); ++iti, ++itn){
                socket()->writeBuffer()
                    << encodeXML(*iti)
                    << "\n";
            }
            socket()->writeBuffer()
                << "</body>\n";
            if (data->richText.toBool() && getRichText()){
                socket()->writeBuffer()
                    << "<html xmlns='http://jabber.org/protocol/xhtml-im'>\n<body>";
                iti = jids.constBegin();
                for (; iti != jids.constEnd(); ++iti, ++itn){
                    socket()->writeBuffer()
                        << encodeXML(*iti)
                        << "<br/>\n";
                }
                socket()->writeBuffer()
                    << "</body>\n</html>\n";
            }
            socket()->writeBuffer()
                << "</message>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                if (data->richText.toBool()){
                    msg->setClient(dataName(data));
                    EventSent(msg).process();
                }else{
                    Message m(MessageGeneric);
                    m.setContact(msg->contact());
                    m.setClient(dataName(data));
                    m.setText(msg->getPlainText());
                    EventSent(&m).process();
                }
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageFile:{
            m_waitMsg.push_back(msg);
            JabberFileTransfer *ft = static_cast<JabberFileTransfer*>(static_cast<FileMessage*>(msg)->m_transfer);
            if (ft == NULL)
                ft = new JabberFileTransfer(static_cast<FileMessage*>(msg), data, this);
            ft->listen();
            return true;
        }
    case MessageAuthRequest:{
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<presence to=\'"
                << data->ID.str()
                << "\' type=\'subscribe\'>\n<status>"
                << encodeXML(msg->getPlainText())
                << "</status>\n</presence>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data));
                EventSent(msg).process();
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageAuthGranted:{
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<presence to=\'"
                << data->ID.str()
                << "\' type=\'subscribed\'></presence>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data));
                EventSent(msg).process();
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageJabberOnline:
        if (isAgent(data->ID.str()) && (data->Status.toULong() == STATUS_OFFLINE)){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<presence to=\'"
                << data->ID.str()
                << "\'></presence>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageJabberOffline:
        if (isAgent(data->ID.str()) && (data->Status.toULong() != STATUS_OFFLINE)){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<presence to=\'"
                << data->ID.str()
                << "\' type=\'unavailable\'></presence>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageTypingStart:
        if (getTyping()){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<message to=\'"
                << data->ID.str()
                << "\'>\n<x xmlns='jabber:x:event'>\n<composing/>\n<id>"
                << data->TypingId.str()
                << "</id>\n</x>\n</message>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageTypingStop:
        if (getTyping()){
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer()
                << "<message to=\'"
                << data->ID.str()
                << "\'>\n<x xmlns='jabber:x:event'>\n<id>"
                << data->TypingId.str()
                << "</id>\n</x>\n</message>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    }
    return false;
}

QString JabberClient::dataName(void *_data)
{
    QString res = name();
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    res += '+';
    res += data->ID.str();
    res = res.replace('/', '_');
    return res;
}

void JabberClient::listRequest(JabberUserData *data, const QString &name, const QString &grp, bool bDelete)
{
    QString jid = data->ID.str();
    list<JabberListRequest>::iterator it;
    for (it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if (jid == it->jid){
            m_listRequests.erase(it);
            break;
        }
    }
    JabberListRequest lr;
    lr.jid = jid;
    lr.name = name;
    lr.grp = grp;
    lr.bDelete = bDelete;
    m_listRequests.push_back(lr);
    processList();
}

JabberListRequest *JabberClient::findRequest(const QString &jid, bool bRemove)
{
    list<JabberListRequest>::iterator it;
    for (it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if (it->jid == jid){
            if (bRemove){
                m_listRequests.erase(it);
                return NULL;
            }
            return &(*it);
        }
    }
    return NULL;
}


bool JabberClient::isAgent(const QString &jid)
{
    if (jid.indexOf('@')==-1)
        return true;
    return false;
}

class JabberClient::JabberAuthMessage : public AuthMessage
{
public:
    JabberAuthMessage(std::vector<JabberAuthMessage*> &tempMessages, unsigned type, Buffer *cfg=NULL)
        : AuthMessage(type, cfg)
        , tempMessages(tempMessages)
    {
        tempMessages.push_back(this);
    }
    virtual ~JabberAuthMessage()
    {
        remove(tempMessages, this);
    }

    static bool remove(std::vector<JabberAuthMessage*>&messages, JabberAuthMessage *value)
    {
        std::vector<JabberAuthMessage*>::iterator it = find(messages.begin(), messages.end(), value);
        if (it != messages.end())
        {
            messages.erase(it);
            return true;
        }
        return false;
    }
    JabberAuthMessage & operator=( const JabberAuthMessage & ) { return *this; }

private:
    std::vector<JabberAuthMessage*> &tempMessages;
};

void JabberClient::auth_request(const QString &jid, unsigned type, const QString &text, bool bCreate)
{
    Contact *contact;
    QString resource;
    JabberUserData *data = findContact(jid, QString::null, false, contact, resource);
    if (isAgent(jid) || ((type == MessageAuthRequest) && getAutoAccept())){
        switch (type){
        case MessageAuthRequest:{
                if (data == NULL)
                    data = findContact(jid, QString::null, true, contact, resource);
                socket()->writeBuffer().packetStart();
                socket()->writeBuffer()
                    << "<presence to=\'"
                    << data->ID.str()
                    << "\' type=\'subscribed\'></presence>";
                sendPacket();
                socket()->writeBuffer().packetStart();
                socket()->writeBuffer()
                    << "<presence to=\'"
                    << data->ID.str()
                    << "\' type=\'subscribe\'>\n<status>"
                    << "</status>\n</presence>";
                sendPacket();
                EventContact e(contact, EventContact::eChanged);
                e.process();
                return;
            }
        case MessageAuthGranted:{
                if (data == NULL)
                    data = findContact(jid, QString::null, true, contact, resource);
                data->Subscribe.asULong() |= SUBSCRIBE_TO;
                EventContact e(contact, EventContact::eChanged);
                e.process();
                return;
            }

        }
    }
    if ((data == NULL) && bCreate){
        data = findContact(jid, QString::null, true, contact, resource);
        contact->setFlags(CONTACT_TEMP);
    }
    if (data == NULL)
        return;
    if (((type == MessageAuthGranted) || (type ==MessageAuthRefused)) &&
            (contact->getFlags() & CONTACT_TEMP)){
        contact->setFlags(contact->getFlags() & ~CONTACT_TEMP);
        EventContact e(contact, EventContact::eChanged);
        e.process();
        return;
    }
    JabberAuthMessage *msg = new JabberAuthMessage(tempAuthMessages, type);
    msg->setContact(contact->id());
    msg->setClient(dataName(data));
    msg->setFlags(MESSAGE_RECEIVED);
    if(!text.isEmpty())
        msg->setText(unquoteString(text));
    EventMessageReceived e(msg);
    e.process();
    if (JabberAuthMessage::remove(tempAuthMessages, msg))
    {
        delete msg;
    }
    if (type == MessageAuthGranted) {
        data->Subscribe.asULong() |= SUBSCRIBE_TO;
        EventContact e(contact, EventContact::eChanged);
        e.process();
    } else
    if (type == MessageAuthRefused) {
        data->Subscribe.asULong() &= ~SUBSCRIBE_TO;
        EventContact e(contact, EventContact::eChanged);
        e.process();
    }
}

void JabberClient::setInvisible(bool bState)
{
    if (getInvisible() == bState)
        return;
    TCPClient::setInvisible(bState);
    if (getStatus() == STATUS_OFFLINE)
        return;
    unsigned status = getStatus();
    m_status = STATUS_OFFLINE;
    if (getInvisible()){
        setStatus(status, NULL);
        return;
    }
    setStatus(status);
}

QString JabberClient::VHost()
{
    if (data.UseVHost.toBool() && !data.VHost.str().isEmpty())
        return data.VHost.str();
    return data.Server.str();
}

static char PICT_PATH[] = "pictures/";

QString JabberClient::photoFile(JabberUserData *data)
{
    QString f = PICT_PATH;
    f += "photo.";
    f += data->ID.str();
    f = user_file(f);
    return f;
}

QString JabberClient::logoFile(JabberUserData *data)
{
    QString f = PICT_PATH;
    f += "logo.";
    f += data->ID.str();
    f = user_file(f);
    return f;
}

void JabberClient::setupContact(Contact *contact, void *_data)
{
    JabberUserData *data = toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString mail = data->EMail.str();
    contact->setEMails(mail, name());
    QString phones;
    if (!data->Phone.str().isEmpty()){
        phones = data->Phone.str();
        phones += ",Home Phone,";
		phones += QString::number(PHONE);
    }
    contact->setPhones(phones, name());

    if (contact->getFirstName().isEmpty() && !data->FirstName.str().isEmpty())
        contact->setFirstName(data->FirstName.str(), name());

    if (contact->getName().isEmpty())
        contact->setName(data->ID.str());
}

QImage JabberClient::userPicture(JabberUserData *d)
{
    JabberUserData *_d = d ? d : &data.owner;
    QImage img;

    if (_d->PhotoWidth.toLong() && _d->PhotoHeight.toLong()) {
        img = QImage(photoFile(_d));
    } else if (_d->LogoWidth.toLong() && _d->LogoHeight.toLong()) {
        img = QImage(logoFile(_d));
    }
    if(img.isNull())
        return img;

    int w = img.width();
    int h = img.height();
    if (h > w){
        if (h > 60){
            w = w * 60 / h;
            h = 60;
        }
    }else{
        if (w > 60){
            h = h * 60 / w;
            w = 60;
        }
    }

    return img.scaled(w, h);
}

QImage JabberClient::userPicture(unsigned id)
{
    if (id==0)
        return QImage();
    Contact *contact = getContacts()->contact(id);
    if(!contact)
        return QImage();
    ClientDataIterator it(contact->clientData, this);

    JabberUserData *d;
    while ((d = toJabberUserData(++it)) != NULL){
        QImage img = userPicture(d);
        if(!img.isNull())
            return img;
    }
    return QImage();
}

JabberUserData* JabberClient::toJabberUserData(SIM::clientData * data)
{
   // This function is used to more safely preform type conversion from SIM::clientData* into JabberUserData*
   // It will at least warn if the content of the structure is not JabberUserData
   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
   if (! data) return NULL;
   if (data->Sign.asULong() != JABBER_SIGN)
   {
      QString Signs[] = {
        "Unknown(0)" ,     // 0x0000
        "ICQ_SIGN",        // 0x0001
        "JABBER_SIGN",     // 0x0002
        "MSN_SIGN",        // 0x0003
        "Unknown(4)"       // 0x0004
        "LIVEJOURNAL_SIGN",// 0x0005
        "SMS_SIGN",        // 0x0006
        "Unknown(7)",      // 0x0007
        "Unknown(8)",      // 0x0008
        "YAHOO_SIGN"       // 0x0009
      };
      QString Sign;
      if (data->Sign.toULong()<=9) // is always >=0 as it is unsigned int
        Sign = Signs[data->Sign.toULong()];
      else
        Sign = QString("Unknown(%1)").arg(Sign.toULong());

      log(L_ERROR,
        "ATTENTION!! Unsafly converting %s user data into JABBER_SIGN",
         Sign.toLatin1().data());
//      abort();
   }
   return (JabberUserData*) data;
}

// vim: set expandtab:

