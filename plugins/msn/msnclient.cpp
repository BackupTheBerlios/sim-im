/***************************************************************************
                          msn.cpp  -  description
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

#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>

#include <algorithm>

#include "log.h"
#include "core.h"
#include "core_events.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include "msnclient.h"
#include "msnconfig.h"
#include "msnpacket.h"
#include "msn.h"
#include "msninfo.h"
#include "msnsearch.h"

#include "socket/clientsocket.h"

#ifndef INADDR_NONE
#define INADDR_NONE     0xFFFFFFFF
#endif

using namespace std;
using namespace SIM;

const unsigned long PING_TIMEOUT	= 60;

const unsigned long TYPING_TIME		= 10;

static DataDef msnUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(3) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "Screen", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(1) },	// Status
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "PhoneHome", DATA_UTF, 1, 0 },
        { "PhoneWork", DATA_UTF, 1, 0 },
        { "PhoneMobile", DATA_UTF, 1, 0 },
        { "Mobile", DATA_BOOL, 1, 0 },
        { "Group", DATA_ULONG, 1, 0 },
        { "Flags", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// sFlags
        { "", DATA_ULONG, 1, 0 },
        { "IP", DATA_IP, 1, 0 },
        { "RealIP", DATA_IP, 1, 0 },
        { "Port", DATA_ULONG, 1, 0 },
        { "", DATA_OBJECT, 1, 0 },				// sb
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static DataDef msnClientData[] =
    {
        { "Server", DATA_STRING, 1, "messenger.hotmail.com" },
        { "Port", DATA_ULONG, 1, DATA(1863) },
        { "ListVer", DATA_ULONG, 1, 0 },
        { "ListRequests", DATA_UTF, 1, 0 },
        { "Version", DATA_STRING, 1, "5.0.0540" },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFF) },
        { "UseHTTP", DATA_BOOL, 1, 0 },
        { "AutoHTTP", DATA_BOOL, 1, DATA(1) },
        { "Deleted", DATA_STRLIST, 1, 0 },
        { "NDeleted", DATA_ULONG, 1, 0 },
        { "AutoAuth", DATA_BOOL, 1, DATA(1) },
        { "", DATA_STRUCT, sizeof(MSNUserData) / sizeof(Data), DATA(msnUserData) },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

MSNClient::MSNClient(Protocol *protocol, Buffer *cfg)
    : TCPClient   (protocol, cfg)
    , m_bJoin     (false)
    , m_packetId  (1)
    , m_msg       (NULL)
    , m_bFirstTry (false)
{
    load_data(msnClientData, &data, cfg);  
    
    m_bFirst  = (cfg == NULL);
    QString s = getListRequests();
    while (!s.isEmpty()){
        QString item = getToken(s, ';');
        MSNListRequest lr;
        lr.Type = getToken(item, ',').toUInt();
        lr.Name = item;
    }
    setListRequests(QString::null);

}

MSNClient::~MSNClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(msnClientData, &data);
    freeData();
}

QString MSNClient::name()
{
    return "MSN." + getLogin();
}

QWidget	*MSNClient::setupWnd()
{
    return new MSNConfig(NULL, this, false);
}

QByteArray MSNClient::getConfig()
{
    QString listRequests;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if (!listRequests.isEmpty())
            listRequests += ";";
        listRequests += QString::number(it->Type) + "," + it->Name;
    }
    setListRequests(listRequests);
    QByteArray res = Client::getConfig();
    if (res.length())
        res += "\n";
    res += save_data(msnClientData, &data);
    setListRequests(QString::null);
    return res;
}

const DataDef *MSNProtocol::userDataDef()
{
    return msnUserData;
}

void MSNClient::connect_ready()
{
    m_bFirstTry = false;
    socket()->readBuffer().init(0);
    socket()->readBuffer().packetStart();
    socket()->setRaw(true);
    log(L_DEBUG, "Connect ready");
    TCPClient::connect_ready();
    MSNPacket *packet = new VerPacket(this);
    packet->send();
}

void MSNClient::setStatus(unsigned status)
{
    if (status  == m_status)
        return;
    QDateTime now(QDateTime::currentDateTime());
    if (m_status == STATUS_OFFLINE)
        data.owner.OnlineTime.asULong() = now.toTime_t();
    data.owner.StatusTime.asULong() = now.toTime_t();
    m_status = status;
    data.owner.Status.asULong() = m_status;
    EventClientChanged(this).process();
    if (status == STATUS_OFFLINE)
    {
        if (m_status != STATUS_OFFLINE)
        {
            m_status = status;
            data.owner.Status.asULong() = status;
            data.owner.StatusTime.asULong() = now.toTime_t();
            MSNPacket *packet = new OutPacket(this);
            packet->send();
        }
        return;
    }
    if (Client::m_state != Connected)
    {
        m_logonStatus = status;
        return;
    }
    m_status = status;
    MSNPacket *packet = new ChgPacket(this);
    packet->send();
}

void MSNClient::connected()
{
    setState(Client::Connected);
    setStatus(m_logonStatus);
    processRequests();
}

void MSNClient::setInvisible(bool bState)
{
    if (bState == getInvisible())
        return;
    TCPClient::setInvisible(bState);
    if (getStatus() == STATUS_OFFLINE)
        return;
    MSNPacket *packet = new ChgPacket(this);
    packet->send();
}

void MSNClient::disconnected()
{
    stop();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL)
    {
        bool bChanged = false;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toMSNUserData(++it)) != NULL){
            if (data->Status.toULong() != STATUS_OFFLINE){
                data->Status.asULong() = STATUS_OFFLINE;
                QDateTime now(QDateTime::currentDateTime());
                data->StatusTime.asULong() = now.toTime_t();
                SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
                if (sock)
                {
                    delete sock;
                    data->sb.clear();
                }
                bChanged = true;
            }
            if (bChanged)
            {
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
    m_packetId = 0;
    m_pingTime = 0;
    m_state    = None;
    m_authChallenge = QString::null;
    clearPackets();
}

void MSNClient::clearPackets()
{
    if (m_msg)
    {
        delete m_msg;
        m_msg = NULL;
    }
    for (list<MSNPacket*>::iterator it = m_packets.begin(); it != m_packets.end(); ++it)
    {
        delete *it;
    }
    m_packets.clear();
}

void MSNClient::packet_ready()
{
    if (socket()->readBuffer().writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(protocol()->plugin());
    EventLog::log_packet(socket()->readBuffer(), false, plugin->MSNPacket);
    if (m_msg)
    {
        if (!m_msg->packet())
            return;
        delete m_msg;
        m_msg = NULL;
    }
    for (;;)
    {
        QByteArray s;
        if (!socket()->readBuffer().scan("\r\n", s))
            break;
        getLine(s);
    }
    if (socket()->readBuffer().readPos() == socket()->readBuffer().writePos())
        socket()->readBuffer().init(0);
}

struct statusText
{
    unsigned	status;
    const char	*name;
};

statusText st[] =
    {
        { STATUS_ONLINE, "NLN" },
        { STATUS_OFFLINE, "FLN" },
        { STATUS_OFFLINE, "HDN" },
        { STATUS_NA, "IDL" },
        { STATUS_AWAY, "AWY" },
        { STATUS_DND, "BSY" },
        { STATUS_BRB, "BRB" },
        { STATUS_PHONE, "PHN" },
        { STATUS_LUNCH, "LUN" },
        { 0, NULL }
    };

static unsigned str2status(const QString &str)
{
    for (const statusText *s = st; s->name; s++){
        if (str == s->name)
            return s->status;
    }
    return STATUS_OFFLINE;
}

void MSNClient::processLSG(unsigned id, const QString &name)
{
    if (id == 0)
        return;
    Group *grp;
    MSNListRequest *lr = findRequest(id, LR_GROUPxREMOVED);
    if (lr)
        return;
    MSNUserData *data = findGroup(id, QString::null, grp);
    if (data){
        lr = findRequest(grp->id(), LR_GROUPxCHANGED);
        if (lr){
            data->sFlags.asULong() |= MSN_CHECKED;
            return;
        }
    }
    data = findGroup(id, name, grp);
    data->sFlags.asULong() |= MSN_CHECKED;
}

void MSNClient::processLST(const QString &mail, const QString &name, unsigned state, unsigned grp)
{
    if ((state & MSN_FORWARD) == 0){
        for (unsigned i = 1; i <= getNDeleted(); i++){
            if (getDeleted(i) == mail)
                return;
        }
    }

    m_curBuddy = mail;
    Contact *contact;
    MSNListRequest *lr = findRequest(mail, LR_CONTACTxREMOVED);
    if (lr)
        return;
    bool bNew = false;
    MSNUserData *data = findContact(mail, contact);
    if (data == NULL){
        data = findContact(mail, name, contact);
        bNew = true;
    }else{
        data->EMail.str() = mail;
        data->ScreenName.str() = name;
        if (name != contact->getName())
            contact->setName(name);
    }
    data->sFlags.asULong() |= MSN_CHECKED;
    data->Flags.asULong() = state;
    if (state & MSN_BLOCKED)
        contact->setIgnore(true);

    lr = findRequest(mail, LR_CONTACTxCHANGED);
    data->Group.asULong() = grp;
    data->PhoneHome.clear();
    data->PhoneWork.clear();
    data->PhoneMobile.clear();
    data->Mobile.asBool() = false;
    Group *group = NULL;
    if ((grp == 0) || (grp == NO_GROUP)){
        group = getContacts()->group(0);
    }else{
        findGroup(grp, QString::null, group);
    }
    if (lr == NULL){
        bool bChanged = ((data->Flags.toULong() & MSN_FLAGS) != (data->sFlags.toULong() & MSN_FLAGS));
        if (getAutoAuth() && (data->Flags.toULong() & MSN_FORWARD) && ((data->Flags.toULong() & MSN_ACCEPT) == 0) && ((data->Flags.toULong() & MSN_BLOCKED) == 0))
            bChanged = true;
        int grp = 0;
        if (group)
            grp = group->id();
        if (grp != contact->getGroup())
            bChanged = true;
        if (bChanged){
            MSNListRequest lr;
            lr.Type = LR_CONTACTxCHANGED;
            lr.Name = data->EMail.str();
            m_requests.push_back(lr);
        }
        if (data->Flags.toULong() & MSN_FORWARD)
            contact->setGroup(grp);
    }
}

void MSNClient::checkEndSync()
{
    if (m_nBuddies || m_nGroups)
        return;
    ContactList::GroupIterator itg;
    Group *grp;
    list<Group*>	grpRemove;
    list<Contact*>	contactRemove;
    while ((grp = ++itg) != NULL){
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *data = toMSNUserData(++it);
        if (grp->id() && (data == NULL)){
            MSNListRequest lr;
            lr.Type = LR_GROUPxCHANGED;
            lr.Name = QString::number(grp->id());
            m_requests.push_back(lr);
            continue;
        }
        if (data == NULL)
            continue;
        if ((data->sFlags.toULong() & MSN_CHECKED) == 0)
            grpRemove.push_back(grp);
    }
    Contact *contact;
    ContactList::ContactIterator itc;
    while ((contact = ++itc) != NULL){
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        list<void*> forRemove;
        while ((data = toMSNUserData(++it)) != NULL){
            if (data->sFlags.toULong() & MSN_CHECKED){
                if ((data->sFlags.toULong() & MSN_REVERSE) && ((data->Flags.toULong() & MSN_REVERSE) == 0))
                    auth_message(contact, MessageRemoved, data);
                if (!m_bFirst && ((data->sFlags.toULong() & MSN_REVERSE) == 0) && (data->Flags.toULong() & MSN_REVERSE)){
                    if ((data->Flags.toULong() & MSN_ACCEPT) || getAutoAuth()){
                        auth_message(contact, MessageAdded, data);
                    }else{
                        auth_message(contact, MessageAuthRequest, data);
                    }
                }
                setupContact(contact, data);
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }else{
                forRemove.push_back(data);
            }
        }
        if (forRemove.empty())
            continue;
        for (list<void*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
            contact->clientData.freeData(*itr);
        if (contact->clientData.size() == 0)
            contactRemove.push_back(contact);
    }
    for (list<Contact*>::iterator rc = contactRemove.begin(); rc != contactRemove.end(); ++rc)
        delete *rc;
    for (list<Group*>::iterator rg = grpRemove.begin(); rg != grpRemove.end(); ++rg)
        delete *rg;
    if (m_bJoin){
        EventJoinAlert(this).process();
    }
    m_bFirst = false;
    connected();
}

void MSNClient::getLine(const QByteArray &line)
{
    QString l = QString::fromUtf8(line);
    l = l.remove('\r');
    log(L_DEBUG, "Get: %s", qPrintable(l));
    QString cmd = getToken(l, ' ');
	log(L_DEBUG, QString("Command: %1").arg(cmd));
    if ((cmd == "715") || (cmd == "228"))
        return;
    if (cmd == "XFR"){
        QString id   = getToken(l, ' ');	// ID
        QString type = getToken(l, ' ');	// NS
        if (type == "NS"){
            l = getToken(l, ' ');				// from
            QString host = getToken(l, ':');
            unsigned short port = l.toUShort();
            if (host.isEmpty() || (port == 0)){
                log(L_WARN, "Bad host on XFR");
                socket()->error_state(I18N_NOOP("MSN protocol error"));
                return;
            }
            clearPackets();
            socket()->close();
            socket()->readBuffer().init(0);
            socket()->connect(host, port, this);
            return;
        }
        l = id + " " + type + " " + l;
    }
    if (cmd == "MSG"){
        getToken(l, ' ');
        getToken(l, ' ');
        unsigned size = getToken(l, ' ').toUInt();
        if (size == 0){
            log(L_WARN, "Bad server message size");
            socket()->error_state(I18N_NOOP("MSN protocol error"));
            return;
        }
        m_msg = new MSNServerMessage(this, size);
        packet_ready();
        return;
    }
    if (cmd == "CHL"){
        getToken(l, ' ');
        MSNPacket *packet = new QryPacket(this, getToken(l, ' '));
        packet->send();
        return;
    }
    if (cmd == "QNG")
        return;
    if (cmd == "BPR"){
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' '), contact);
        if (data){
            QString info = getToken(l, ' ');
            QString type = getToken(l, ' ');
            bool bChanged = false;
            if (type == "PHH"){
                bChanged = data->PhoneHome.setStr(unquote(info));
            }else if (type == "PHW"){
                bChanged = data->PhoneWork.setStr(unquote(info));
            }else if (type == "PHM"){
                bChanged = data->PhoneMobile.setStr(unquote(info));
            }else if (type == "MOB"){
                data->Mobile.asBool() = ((info[0] == 'Y') != 0);
            }else{
                log(L_DEBUG, "Unknown BPR type %s", qPrintable(type));
            }
            if (bChanged){
                setupContact(contact, data);
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }
        }
        return;
    }
    if (cmd == "ILN"){
        getToken(l, ' ');
        unsigned status = str2status(getToken(l, ' '));
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' '), contact);
        QDateTime now(QDateTime::currentDateTime());
        if (data && (data->Status.toULong() != status))
        {
            data->Status.asULong() = status;
            if (data->Status.toULong() == STATUS_OFFLINE){
                data->OnlineTime.asULong() = now.toTime_t();
                set_ip(&data->IP, 0);
                set_ip(&data->RealIP, 0);
            }
            data->StatusTime.asULong() = now.toTime_t();
            StatusMessage *m = new StatusMessage();
            m->setContact(contact->id());
            m->setClient(dataName(data));
            m->setFlags(MESSAGE_RECEIVED);
            m->setStatus(status);
            EventMessageReceived e(m);
            if(!e.process())
                delete m;
        }
        return;
    }
    if (cmd == "NLN"){
        unsigned status = str2status(getToken(l, ' '));
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' '), contact);
        if (data && (data->Status.toULong() != status))
        {
            QDateTime now(QDateTime::currentDateTime());
            if (data->Status.toULong() == STATUS_OFFLINE){
                data->OnlineTime.asULong() = now.toTime_t();
                set_ip(&data->IP, 0);
                set_ip(&data->RealIP, 0);
            }
            data->StatusTime.asULong() = now.toTime_t();
            data->Status.asULong() = status;
            StatusMessage *m = new StatusMessage();
            m->setContact(contact->id());
            m->setClient(dataName(data));
            m->setFlags(MESSAGE_RECEIVED);
            m->setStatus(status);
            EventMessageReceived e(m);
            if(!e.process())
                delete m;
            if ((status == STATUS_ONLINE) && !contact->getIgnore()){
                EventContact e(contact, EventContact::eOnline);
                e.process();
            }
        }
        return;
    }
    if (cmd == "FLN"){
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' '), contact);
        if (data && (data->Status.toULong() != STATUS_OFFLINE)){
            QDateTime now(QDateTime::currentDateTime());
            data->StatusTime.asULong() = now.toTime_t();
            data->Status.asULong() = STATUS_OFFLINE;
            StatusMessage *m = new StatusMessage();
            m->setContact(contact->id());
            m->setClient(dataName(data));
            m->setFlags(MESSAGE_RECEIVED);
            m->setStatus(STATUS_OFFLINE);
            EventMessageReceived e(m);
            if(!e.process())
                delete m;
        }
        return;
    }
    if (cmd == "ADD"){
        getToken(l, ' ');
        if (getToken(l, ' ') == "RL"){
            setListVer(getToken(l, ' ').toUInt());
            Contact *contact;
            MSNUserData *data = findContact(getToken(l, ' '), getToken(l, ' '), contact);
            if (data){
                data->Flags.asULong() |= MSN_REVERSE;
                if ((data->Flags.toULong() & MSN_ACCEPT) || getAutoAuth()){
                    auth_message(contact, MessageAdded, data);
                }else{
                    auth_message(contact, MessageAuthRequest, data);
                }
            }
        }
        return;
    }
    if (cmd == "REM"){
        getToken(l, ' ');
        if (getToken(l, ' ') == "RL"){
            setListVer(getToken(l, ' ').toUInt());
            Contact *contact;
            MSNUserData *data = findContact(getToken(l, ' '), contact);
            if (data){
                data->Flags.asULong() &= ~MSN_REVERSE;
                auth_message(contact, MessageRemoved, data);
            }
        }
        return;
    }
    if (cmd == "RNG"){
        QString session = getToken(l, ' ');
        QString addr = getToken(l, ' ');
        getToken(l, ' ');
        QString cookie, email, nick;
        cookie = getToken(l, ' ');
        email  = getToken(l, ' ');
        nick   = getToken(l, ' ');
        Contact *contact;
        MSNUserData *data = findContact(email, contact);
        if (data == NULL){
            data = findContact(email, nick, contact);
            contact->setFlags(CONTACT_TEMP);
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
        if (sock){
            delete sock;
        }
        sock = new SBSocket(this, contact, data);
        sock->connect(addr, session, cookie, false);
        data->sb.setObject(sock);
        return;
    }
    if (cmd == "OUT"){
        m_reconnect = NO_RECONNECT;
        socket()->error_state(I18N_NOOP("Your account is being used from another location"));
        return;
    }
    if (cmd == "GTC")
        return;
    if (cmd == "BLP")
        return;
	    if (cmd == "UUX")
    {
		// personal message
        getToken(l, ' ');
        getToken(l, ' ');
        unsigned size = getToken(l, ' ').toUInt();
        if (size == 0){
            log(L_WARN, "Empty server personal message size");
            //return;
        }
        else
        {
            m_msg = new MSNServerMessage(this, size);
            // only we post the message in log now.....
            // ToDo: restore this
            // Error: m_msg is not a string
            //log(L_WARN, "Personal message: %s", m_msg);
            //packet_ready();
        }
        return;
    }
    if (cmd == "LSG"){
        unsigned id = getToken(l, ' ').toUInt();
        processLSG(id, unquote(getToken(l, ' ')));
        m_nGroups--;
        checkEndSync();
        return;
    }
    if (cmd == "LST"){
        QString mail = unquote(getToken(l, ' '));
        QString name = unquote(getToken(l, ' '));
        unsigned state = getToken(l, ' ').toUInt();
        unsigned grp   = getToken(l, ' ').toUInt();
        processLST(mail, name, state, grp);
        m_nBuddies--;
        checkEndSync();
        return;
    }
    if (cmd == "PRP"){
        QString cmd = getToken(l, ' ');
        if (cmd == "PHH")
            data.owner.PhoneHome.str() = unquote(getToken(l, ' '));
        if (cmd == "PHW")
            data.owner.PhoneWork.str() = unquote(getToken(l, ' '));
        if (cmd == "PHM")
            data.owner.PhoneMobile.str() = unquote(getToken(l, ' '));
        if (cmd == "MBE")
            data.owner.Mobile.asBool() = (getToken(l, ' ') == "Y");
        return;
    }
    if (cmd == "BPR"){
        Contact *contact;
        MSNUserData *data = findContact(m_curBuddy, contact);
        if (data == NULL)
            return;
         EventContact e(contact, EventContact::eChanged);
        e.process();
        QString cmd = getToken(l, ' ');
        if (cmd == "PHH")
            data->PhoneHome.str() = unquote(getToken(l, ' '));
        if (cmd == "PHW")
            data->PhoneWork.str() = unquote(getToken(l, ' '));
        if (cmd == "PHM")
            data->PhoneMobile.str() = unquote(getToken(l, ' '));
        if (cmd == "MBE")
            data->Mobile.asBool() = (getToken(l, ' ') == "Y");
        return;
    }
    unsigned code = cmd.toUInt();
    if (code){
        MSNPacket *packet = NULL;
        unsigned id = getToken(l, ' ').toUInt();
        list<MSNPacket*>::iterator it;
        for (it = m_packets.begin(); it != m_packets.end(); ++it){
            if ((*it)->id() == id){
                packet = *it;
                break;
            }
        }
        if (it == m_packets.end()){
            socket()->error_state("Bad packet id");
            return;
        }
        m_packets.erase(it);
        packet->error(code);
        delete packet;
        return;
    }
    if (m_packets.empty()){
        log(L_DEBUG, "Packet not found");
        return;
    }
    MSNPacket *packet = NULL;
    unsigned id = getToken(l, ' ').toUInt();
    list<MSNPacket*>::iterator it;
    for (it = m_packets.begin(); it != m_packets.end(); ++it){
        if ((*it)->id() == id){
            packet = *it;
            break;
        }
    }
    if (it == m_packets.end()){
        socket()->error_state("Bad packet id");
        return;
    }
    if (cmd != packet->cmd()){
        socket()->error_state("Bad answer cmd");
        return;
    }
    QStringList args;
    while (l.length())
        args.push_back(getToken(l, ' ', false));
    packet->answer(args);
    m_packets.erase(it);
    delete packet;
}

void MSNClient::sendLine(const QString &line, bool crlf)
{
    log(L_DEBUG, "Send: %s", qPrintable(line));
    socket()->writeBuffer().packetStart();
    socket()->writeBuffer() << (const char*)line.toUtf8();
    if (crlf)
        socket()->writeBuffer() << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(protocol()->plugin());
    EventLog::log_packet(socket()->writeBuffer(), true, plugin->MSNPacket);
    socket()->write();
}

void MSNClient::authFailed()
{
    m_reconnect = NO_RECONNECT;
    socket()->error_state(I18N_NOOP("Login failed"), AuthError);
}

void MSNClient::authOk()
{
    m_state    = None;
    m_authChallenge = QString::null;
    QDateTime now(QDateTime::currentDateTime());
    m_pingTime = now.toTime_t();
    QTimer::singleShot(TYPING_TIME * 1000, this, SLOT(ping()));
    setPreviousPassword(QString::null);
    MSNPacket *packet = new SynPacket(this);
    packet->send();
}

void MSNClient::ping()
{
    if (getState() != Connected)
        return;
    QDateTime now(QDateTime::currentDateTime());
    if (now.toTime_t() >= m_pingTime + PING_TIMEOUT){
        sendLine("PNG");
        m_pingTime = now.toTime_t();
    }
    for (list<SBSocket*>::iterator it = m_SBsockets.begin(); it != m_SBsockets.end(); ++it)
        (*it)->timer(now.toTime_t());
    QTimer::singleShot(TYPING_TIME * 1000, this, SLOT(ping()));
}

QString MSNClient::getLogin()
{
    return data.owner.EMail.str();
}

void MSNClient::setLogin(const QString &str)
{
    data.owner.EMail.str() = str;
}

const unsigned MAIN_INFO = 1;
const unsigned NETWORK	 = 2;

static CommandDef msnWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "MSN_online",
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

static CommandDef cfgMsnWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "MSN_online",
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

CommandDef *MSNClient::infoWindows(Contact*, void *_data)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += data->EMail.str();
    msnWnd[0].text_wrk = name;
    return msnWnd;
}

CommandDef *MSNClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += data.owner.EMail.str();
    cfgMsnWnd[0].text_wrk = name;
    return cfgMsnWnd;
}

QWidget *MSNClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    switch (id){
    case MAIN_INFO:
        return new MSNInfo(parent, data, this);
    }
    return NULL;
}

QWidget *MSNClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new MSNInfo(parent, NULL, this);
    case NETWORK:
        return new MSNConfig(parent, this, true);
    }
    return NULL;
}

bool MSNClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.toULong() != MSN_SIGN))
        return false;
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    if (getState() != Connected)
        return false;
    switch (type){
    case MessageGeneric:
    case MessageFile:
    case MessageUrl:
        if (getInvisible())
            return false;
        return true;
    case MessageAuthGranted:
    case MessageAuthRefused:
        return (data->Flags.toULong() & MSN_ACCEPT) == 0;
    }
    return false;
}

bool MSNClient::send(Message *msg, void *_data)
{
    if ((_data == NULL) || (getState() != Connected))
        return false;
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    MSNPacket *packet;
    switch (msg->type()){
    case MessageAuthGranted:
        if (data->Flags.toULong() & MSN_ACCEPT)
            return false;
        packet = new AddPacket(this, "AL", data->EMail.str(), quote(data->ScreenName.str()), 0);
        packet->send();
    case MessageAuthRefused:
        if (data->Flags.toULong() & MSN_ACCEPT)
            return false;
        if (msg->getText().isEmpty()){
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data));
                EventSent(msg).process();
            }
            EventMessageSent(msg).process();
            delete msg;
            return true;
        }
    case MessageGeneric:
    case MessageFile:
    case MessageUrl: {
            SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
            if (!sock){
                if (getInvisible())
                    return false;
                Contact *contact;
                findContact(data->EMail.str(), contact);
                sock = new SBSocket(this, contact, data);
                sock->connect();
                data->sb.setObject(sock);
            }
            return sock->send(msg);
        }
    case MessageTypingStart: {
            SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
            if (!sock){
                if (getInvisible())
                    return false;
                Contact *contact;
                findContact(data->EMail.str(), contact);
                sock = new SBSocket(this, contact, data);
                sock->connect();
                data->sb.setObject(sock);
            }
            sock->setTyping(true);
            delete msg;
            return true;
        }
    case MessageTypingStop: {
            SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
            if (!sock)
                return false;
            sock->setTyping(false);
            delete msg;
            return true;
        }
    }
    return false;
}

QString MSNClient::dataName(void *_data)
{
    QString res = name();
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    res += "+";
    res += data->EMail.str();
    return res;
}

bool MSNClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign.toULong() != MSN_SIGN)
        return false;
    MSNUserData *data = toMSNUserData(_data);
    if (data->EMail.str().toLower() == this->data.owner.EMail.str().toLower())
        return false;
    MSNUserData *my_data = findContact(data->EMail.str(), contact);
    if (my_data){
        data = my_data;
    }else{
        contact = NULL;
    }
    return true;
}

bool MSNClient::createData(clientData *&_data, Contact *contact)
{
    MSNUserData *data = toMSNUserData(_data);
    MSNUserData *new_data = toMSNUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    new_data->EMail.str() = data->EMail.str();
    _data = (clientData*)new_data;
    return true;
}

void MSNClient::setupContact(Contact *contact, void *_data)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString phones;
    if (!data->PhoneHome.str().isEmpty()){
        phones += data->PhoneHome.str();
        phones += ",Home Phone,1";
    }
    if (!data->PhoneWork.str().isEmpty()){
        if (!phones.isEmpty())
            phones += ";";
        phones += data->PhoneWork.str();
        phones += ",Work Phone,1";
    }
    if (!data->PhoneMobile.str().isEmpty()){
        if (!phones.isEmpty())
            phones += ";";
        phones += data->PhoneMobile.str();
        phones += ",Private Cellular,2";
    }
    bool bChanged = contact->setPhones(phones, name());
    bChanged |= contact->setEMails(data->EMail.str(), name());
    if (contact->getName().isEmpty()){
        QString name = data->ScreenName.str();
        if (name.isEmpty())
            name = data->EMail.str();
        int n = name.indexOf('@');
        if (n > 0)
            name = name.left(n);
        
        QString oldName(contact->getName());
        contact->setName(name);
        bChanged |= (oldName != name );
    }
    if (bChanged)
    {
        EventContact e(contact, EventContact::eChanged);
        e.process();
    }
}

QString MSNClient::unquote(const QString &s)
{
    QString res;
    for (int i = 0; i < (int)(s.length()); i++){
        QChar c = s[i];
        if (c != '%'){
            res += c;
            continue;
        }
        i++;
        if (i + 2 > (int)(s.length()))
            break;
        res += QChar((char)((fromHex(s[i].toAscii()) << 4) + fromHex(s[i+1].toAscii())));
        i++;
    }
    return res;
}

// FIXME: what's with non-latin1 characters here?
QString MSNClient::quote(const QString &s)
{
    QString res;
    for (int i = 0; i < (int)(s.length()); i++){
        QChar c = s[i];
        if ((c == '%') || (c == ' ') || (c == '+')){
            char b[4];
            sprintf(b, "%%%2X", c.toLatin1());
            res += b;
        }else{
            res += c;
        }
    }
    return res;
}

MSNUserData *MSNClient::findContact(const QString &mail, Contact *&contact)
{
    ContactList::ContactIterator itc;
    while ((contact = ++itc) != NULL){
        MSNUserData *res;
        ClientDataIterator it(contact->clientData, this);
        while ((res = toMSNUserData(++it)) != NULL){
            if (res->EMail.str() == mail)
                return res;
        }
    }
    return NULL;
}

QString MSNClient::contactName(void *clientData)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)clientData); // FIXME unsafe type conversion
    return "MSN: " + data->EMail.str();
}

MSNUserData *MSNClient::findContact(const QString &mail, const QString &name, Contact *&contact, bool bJoin)
{
    unsigned i;
    for (i = 1; i <= getNDeleted(); i++){
        if (getDeleted(i) == mail)
            break;
    }
    if (i <= getNDeleted()){
        QStringList deleted;
        for (i = 1; i <= getNDeleted(); i++){
            if (getDeleted(i) == mail)
                continue;
            deleted.push_back(getDeleted(i));
        }
        setNDeleted(0);
        for (QStringList::iterator it = deleted.begin(); it != deleted.end(); ++it){
            setNDeleted(getNDeleted() + 1);
            setDeleted(getNDeleted(), (*it));
        }
    }
    QString name_str = unquote(name);
    MSNUserData *data = findContact(mail, contact);
    if (data){
        data->ScreenName.str() = name;
        setupContact(contact, data);
        return data;
    }
    if (bJoin){
        ContactList::ContactIterator it;
        while ((contact = ++it) != NULL){
            if (contact->getName() == name_str){
                data = toMSNUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
                data->EMail.str() = mail;
                data->ScreenName.str() = name;
                setupContact(contact, data);
                EventContact e(contact, EventContact::eChanged);
                e.process();
                return data;
            }
        }
        it.reset();
        while ((contact = ++it) != NULL){
            if (contact->getName().toLower() == name_str.toLower()){
                data = toMSNUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
                data->EMail.str() = mail;
                data->ScreenName.str() = name;
                setupContact(contact, data);
                EventContact e(contact, EventContact::eChanged);
                e.process();
                m_bJoin = true;
                return data;
            }
        }
        int n = name_str.indexOf('@');
        if (n > 0){
            name_str = name_str.left(n);
            it.reset();
            while ((contact = ++it) != NULL){
                if (contact->getName().toLower() == name_str.toLower()){
                    data = toMSNUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
                    data->EMail.str() = mail;
                    data->ScreenName.str() = name;
                    setupContact(contact, data);
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                    m_bJoin = true;
                    return data;
                }
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = toMSNUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    data->EMail.str() = mail;
    data->ScreenName.str() = name;
    contact->setName(name_str);
    EventContact e(contact, EventContact::eChanged);
    e.process();
    return data;
}

MSNUserData *MSNClient::findGroup(unsigned long id, const QString &name, Group *&grp)
{
    ContactList::GroupIterator itg;
    while ((grp = ++itg) != NULL){
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *res = toMSNUserData(++it);
        if ((res == NULL) || (res->Group.toULong() != id))
            continue;
        if (!name.isEmpty() && res->ScreenName.setStr(name)){
            grp->setName(name);
            EventGroup e(grp, EventGroup::eChanged);
            e.process();
        }
        return res;
    }
    if (name.isEmpty())
        return NULL;
    QString grpName = name;
    itg.reset();
    while ((grp = ++itg) != NULL){
        if (grp->getName() != grpName)
            continue;
        MSNUserData *res = toMSNUserData((SIM::clientData*)grp->clientData.createData(this)); // FIXME unsafe type conversion
        res->Group.asULong() = id;
        res->ScreenName.str() = name;
        return res;
    }
    grp = getContacts()->group(0, true);
    MSNUserData *res = toMSNUserData((SIM::clientData*)grp->clientData.createData(this)); // FIXME unsafe type conversion
    res->Group.asULong() = id;
    res->ScreenName.str() = name;
    grp->setName(grpName);
    EventGroup e(grp, EventGroup::eChanged);
    e.process();
    return res;
}

void MSNClient::auth_message(Contact *contact, unsigned type, MSNUserData *data)
{
    AuthMessage *msg = new AuthMessage(type);
    msg->setClient(dataName(data));
    msg->setContact(contact->id());
    msg->setFlags(MESSAGE_RECEIVED);
    EventMessageReceived e(msg);
    if(!e.process())
        delete msg;
}

bool MSNClient::done(unsigned code, Buffer&, const QString &headers)
{
    switch (m_state){
    case LoginHost:
        if (code == 200){
            QString h = getHeader("PassportURLs", headers);
            if (h.isEmpty()){
                socket()->error_state("No PassportURLs answer");
                break;
            }
            QString loginHost = getValue("DALogin", h);
            if (loginHost.isEmpty()){
                socket()->error_state("No DALogin in PassportURLs answer");
                break;
            }
            QString loginUrl = "https://" + loginHost;
            requestTWN(loginUrl);
        }else{
            socket()->error_state("Bad answer code");
        }
        break;
    case TWN:
        if (code == 200){
            QString h = getHeader("Authentication-Info", headers);
            if (h.isEmpty()){
                socket()->error_state("No Authentication-Info answer");
                break;
            }
            QString twn = getValue("from-PP", h);
            if (twn.isEmpty()){
                socket()->error_state("No from-PP in Authentication-Info answer");
                break;
            }
            MSNPacket *packet = new UsrPacket(this, twn);
            packet->send();
        }else if (code == 401){
            authFailed();
        }else{
            socket()->error_state("Bad answer code");
        }
        break;
    default:
        log(L_WARN, "Fetch done in bad state");
    }
    return false;
}

bool MSNClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    switch(e->type()) {
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == static_cast<MSNPlugin*>(protocol()->plugin())->MSNInitMail){
            EventGoURL(m_init_mail).process();
            return true;
        }
        if (cmd->id == static_cast<MSNPlugin*>(protocol()->plugin())->MSNNewMail){
            EventGoURL(m_new_mail).process();
            return true;
        }
        break;
    }
    case eEventAddContact: {
        EventAddContact *ec = static_cast<EventAddContact*>(e);
        EventAddContact::AddContact *ac = ec->addContact();
        if (!ac->proto.isEmpty() && (protocol()->description()->text == ac->proto))
		{
            Contact *contact = NULL;
            findContact(ac->addr, ac->nick, contact);
            if (contact && contact->getGroup() != ac->group)
            {
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
            MSNUserData *data;
            ClientDataIterator itc(contact->clientData, this);
            while ((data = toMSNUserData(++itc)) != NULL){
                if (data->EMail.str() == addr){
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
    case eEventGetContactIP: {
        EventGetContactIP *ei = static_cast<EventGetContactIP*>(e);
        Contact *contact = ei->contact();
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toMSNUserData(++it)) != NULL){
            if (data->IP.ip()) {
                ei->setIP(data->IP.ip());
                return true;
            }
        }
        break;
    }
    case eEventMessageAccept: {
        EventMessageAccept *ema = static_cast<EventMessageAccept*>(e);
        Contact *contact = getContacts()->contact(ema->msg()->contact());
        if (contact == NULL)
            return false;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toMSNUserData(++it)) != NULL){
            if (dataName(data) == ema->msg()->client()){
                SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
                if (sock)
                    sock->acceptMessage(ema->msg(), ema->dir(), ema->mode());
                return true;
            }
        }
        break;
    }
    case eEventMessageDecline: {
        EventMessageDecline *emd = static_cast<EventMessageDecline*>(e);
        Contact *contact = getContacts()->contact(emd->msg()->contact());
        if (contact == NULL)
            return false;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toMSNUserData(++it)) != NULL){
            if (dataName(data) == emd->msg()->client()){
                SBSocket *sock = dynamic_cast<SBSocket*>(data->sb.object());
                if (sock)
                    sock->declineMessage(emd->msg(), emd->reason());
                return true;
            }
        }
        break;
    }
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                MSNUserData *data;
                ClientDataIterator it(contact->clientData, this);
                while ((data = toMSNUserData(++it)) != NULL){
                    findRequest(data->EMail.str(), LR_CONTACTxCHANGED, true);
                    MSNListRequest lr;
                    if (data->Group.toULong() != NO_GROUP){
                        lr.Type  = LR_CONTACTxREMOVED;
                        lr.Name  = data->EMail.str();
                        lr.Group = data->Group.toULong();
                        m_requests.push_back(lr);
                    }
                    if (data->Flags.toULong() & MSN_BLOCKED){
                        lr.Type = LR_CONTACTxREMOVED_BL;
                        lr.Name  = data->EMail.str();
                        m_requests.push_back(lr);
                    }
                }
                processRequests();
                break;
            }
            case EventContact::eChanged: {
                MSNUserData *data;
                ClientDataIterator it(contact->clientData, this);
                while ((data = toMSNUserData(++it)) != NULL){
                    bool bChanged = false;
                    if (contact->getIgnore() != ((data->Flags.toULong() & MSN_BLOCKED) != 0))
                        bChanged = true;
                    if (contact->getGroup() != (int)(data->Group.toULong()))
                        bChanged = true;
                    if (contact->getName() != data->ScreenName.str())
                        bChanged = true;
                    if (!bChanged)
                        continue;
                    findRequest(data->EMail.str(), LR_CONTACTxCHANGED, true);
                    MSNListRequest lr;
                    lr.Type = LR_CONTACTxCHANGED;
                    lr.Name = data->EMail.str();
                    m_requests.push_back(lr);
                }
                processRequests();
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventGroup: {
        EventGroup *ev = static_cast<EventGroup*>(e);
        Group *grp = ev->group();
        switch (ev->action()) {
        case EventGroup::eChanged: {
            ClientDataIterator it(grp->clientData, this);
            MSNUserData *data = toMSNUserData(++it);
            if ((data == NULL) || (grp->getName() != data->ScreenName.str())){
                findRequest(grp->id(), LR_GROUPxCHANGED, true);
                MSNListRequest lr;
                lr.Type = LR_GROUPxCHANGED;
                lr.Name = QString::number(grp->id());
                m_requests.push_back(lr);
                processRequests();
            }
            break;
        }
        case EventGroup::eDeleted: {
            ClientDataIterator it(grp->clientData, this);
            MSNUserData *data = toMSNUserData(++it);
            if (data){
                findRequest(grp->id(), LR_GROUPxCHANGED, true);
                MSNListRequest lr;
                lr.Type = LR_GROUPxREMOVED;
                lr.Name = QString::number(data->Group.toULong());
                m_requests.push_back(lr);
                processRequests();
            }
            break;
        }
        case EventGroup::eAdded:
            break;
        }
        break;
    }
    case eEventMessageCancel: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        for (list<SBSocket*>::iterator it = m_SBsockets.begin(); it != m_SBsockets.end(); ++it){
            if ((*it)->cancelMessage(msg))
                return msg;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void MSNClient::requestLoginHost(const QString &url)
{
    if (!isDone())
        return;
    m_state = LoginHost;
    fetch(url);
}

void MSNClient::requestTWN(const QString &url)
{
    if (!isDone())
        return;
    QString auth = "Authorization: Passport1.4 OrgVerb=GET,OrgURL=http%%3A%%2F%%2Fmessenger%%2Emsn%%2Ecom,sign-in=";
    auth += quote(getLogin());
    auth += ",pwd=";
    auth += quote(getPassword());
    auth += ",";
    auth += m_authChallenge;
    m_state = TWN;
    fetch(url, auth);
}

QString MSNClient::getValue(const QString &key, const QString &str)
{
    QString s = str;
    while (!s.isEmpty()){
        QString k = getToken(s, '=');
        QString v;
        if (s.startsWith("\'")){
            getToken(s, '\'');
            v = getToken(s, '\'');
            getToken(s, ',');
        }else{
            v = getToken(s, ',');
        }
        if (k == key)
            return v;
    }
    return QString::null;
}

QString MSNClient::getHeader(const QString &name, const QString &headers)
{
    int idx = headers.indexOf(name + ':');
    if(idx != -1) {
        int end = headers.indexOf('\n', idx);
        QString res;
        if(end == -1)
            res = headers.mid(idx);
        else
            res = headers.mid(idx, end - idx + 1);
        return res.trimmed();
    }
    return QString::null;
}

MSNListRequest *MSNClient::findRequest(unsigned long id, unsigned type, bool bDelete)
{
    if (m_requests.empty())
        return NULL;
    return findRequest(QString::number(id), type, bDelete);
}

MSNListRequest *MSNClient::findRequest(const QString &name, unsigned type, bool bDelete)
{
    if (m_requests.empty())
        return NULL;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if ((it->Type == type) && (it->Name == name)){
            if (bDelete){
                m_requests.erase(it);
                return NULL;
            }
            return &(*it);
        }
    }
    return NULL;
}

void MSNClient::processRequests()
{
    if (m_requests.empty() || (getState() != Connected))
        return;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        Group *grp;
        Contact *contact;
        MSNPacket *packet = NULL;
        MSNUserData *data;
        switch (it->Type){
        case LR_CONTACTxCHANGED:
            data = findContact(it->Name, contact);
            if (data){
                bool bBlock = (data->Flags.toULong() & MSN_BLOCKED) != 0;
                if (contact->getIgnore() != bBlock){
                    if (contact->getIgnore()){
                        if (data->Flags.toULong() & MSN_FORWARD)
                            packet = new RemPacket(this, "FL", it->Name);
                        if (data->Flags.toULong() & MSN_ACCEPT){
                            if (packet)
                                packet->send();
                            packet = new RemPacket(this, "AL", it->Name);
                        }
                        data->Flags.asULong() &= ~(MSN_FORWARD | MSN_ACCEPT);
                        if (packet)
                            packet->send();
                        packet = new AddPacket(this, "BL", data->EMail.str(), quote(contact->getName()));
                        data->ScreenName.str() = contact->getName();
                        data->Flags.asULong() |= MSN_BLOCKED;
                    }else{
                        packet = new RemPacket(this, "BL", data->EMail.str());
                        data->Flags.asULong() &= ~MSN_BLOCKED;
                    }
                }
                if (data->Flags.toULong() & MSN_BLOCKED)
                    break;
                unsigned grp_id = 0;
                if (contact->getGroup()){
                    Group *grp = getContacts()->group(contact->getGroup());
                    if(grp) {
                        ClientDataIterator it(grp->clientData, this);
                        MSNUserData *res = toMSNUserData(++it);
                        if (res)
                            grp_id = res->Group.toULong();
                    }
                }
                if (((data->Flags.toULong() & MSN_FORWARD) == 0) || (data->Group.toULong() == NO_GROUP)){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "FL", data->EMail.str(), quote(data->ScreenName.str()), grp_id);
                    data->Group.asULong() = grp_id;
                    data->Flags.asULong() |= MSN_FORWARD;
                }
                if (getAutoAuth() && (data->Flags.toULong() & MSN_FORWARD) && ((data->Flags.toULong() & MSN_ACCEPT) == 0) && ((data->Flags.toULong() & MSN_BLOCKED) == 0)){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "AL", data->EMail.str(), quote(data->ScreenName.str()), 0);
                    data->Group.asULong() = grp_id;
                    data->Flags.asULong() |= MSN_ACCEPT;
                }
                if (data->Group.toULong() != grp_id){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "FL", data->EMail.str(), quote(data->ScreenName.str()), grp_id);
                    packet->send();
                    packet = NULL;
                    packet = new RemPacket(this, "FL", data->EMail.str(), data->Group.toULong());
                    data->Group.asULong() = grp_id;
                }
                if (contact->getName() != data->ScreenName.str()){
                    if (packet)
                        packet->send();
                    packet = new ReaPacket(this, data->EMail.str(), quote(contact->getName()));
                    data->ScreenName.str() = contact->getName();
                }
            }
            break;
        case LR_CONTACTxREMOVED:
            packet = new RemPacket(this, "AL", it->Name);
            packet->send();
            packet = new RemPacket(this, "FL", it->Name);
            setNDeleted(getNDeleted() + 1);
            setDeleted(getNDeleted(), it->Name);
            break;
        case LR_CONTACTxREMOVED_BL:
            packet = new RemPacket(this, "BL", it->Name);
            break;
        case LR_GROUPxCHANGED:
            grp = getContacts()->group(it->Name.toULong());
            if (grp){
                ClientDataIterator it(grp->clientData, this);
                data = toMSNUserData(++it);
                if (data){
                    packet = new RegPacket(this, data->Group.toULong(), quote(grp->getName()));
                }else{
                    packet = new AdgPacket(this, grp->id(), quote(grp->getName()));
                    data = toMSNUserData((SIM::clientData*)grp->clientData.createData(this)); // FIXME unsafe type conversion
                }
                data->ScreenName.str() = grp->getName();
            }
            break;
        case LR_GROUPxREMOVED:
            packet = new RmgPacket(this, it->Name.toULong());
            break;
        }
        if (packet)
            packet->send();
    }
    m_requests.clear();
}

bool MSNClient::add(const QString &mail, const QString &name, unsigned grp)
{
    Contact *contact;
    MSNUserData *data = findContact(mail, contact);
    if (data)
    {
        if (contact->getGroup() != (int)grp)
        {
            contact->setGroup(grp);
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        return false;
    }
    data = findContact(mail, name, contact);
    if (!data)
        return false;
    contact->setGroup(grp);
    EventContact e(contact, EventContact::eChanged);
    e.process();
    return true;
}

bool MSNClient::compareData(void *d1, void *d2)
{
    return (toMSNUserData((SIM::clientData*)d1)->EMail.str() == (toMSNUserData((SIM::clientData*)d2)->EMail.str())); // FIXME unsafe type conversion
}

static void addIcon(QSet<QString> *s, const QString &icon, const QString &statusIcon)
{
    if (!s || statusIcon == icon)
        return;
    s->insert(icon);
}

void MSNClient::contactInfo(void *_data, unsigned long &curStatus, unsigned&, QString &statusIcon, QSet<QString> *icons)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    unsigned cmp_status = data->Status.toULong();
    const CommandDef *def;
    for (def = protocol()->statusList(); !def->text.isEmpty(); def++){
        if (def->id == cmp_status)
            break;
    }
    if ((cmp_status == STATUS_BRB) || (cmp_status == STATUS_PHONE) || (cmp_status == STATUS_LUNCH))
        cmp_status = STATUS_AWAY;
    if (data->Status.toULong() > curStatus){
        curStatus = data->Status.toULong();
        if (!statusIcon.isEmpty() && icons){
            icons->insert(statusIcon);
        }
        statusIcon = def->icon;
    }else{
        if (!statusIcon.isEmpty()){
            addIcon(icons, def->icon, statusIcon);
        }else{
            statusIcon = def->icon;
        }
    }
    if (icons && data->typing_time.toULong())
        addIcon(icons, "typing", statusIcon);
}

QString MSNClient::contactTip(void *_data)
{
    MSNUserData *data = toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    unsigned long status = STATUS_UNKNOWN;
    unsigned style  = 0;
    QString statusIcon;
    contactInfo(data, status, style, statusIcon);
    QString res;
    res += "<img src=\"sim:icons/";
    res += statusIcon;
    res += "\">";
    QString statusText;
    for (const CommandDef *cmd = protocol()->statusList(); !cmd->text.isEmpty(); cmd++){
        if (cmd->icon == statusIcon){
            res += " ";
            statusText = i18n(cmd->text);
            res += statusText;
            break;
        }
    }
    res += "<br>";
    res += data->EMail.str();
    res += "</b>";
    if (data->Status.toULong() == STATUS_OFFLINE){
        if (data->StatusTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
    }else{
        if (data->OnlineTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatDateTime(data->OnlineTime.toULong());
        }
        if (data->Status.toULong() != STATUS_ONLINE){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
    }
    if (data->IP.ip())
    {
        res += "<br>";
        res += formatAddr(data->IP, data->Port.toULong());
    }
    if (data->RealIP.ip() && ((data->IP.ip() == NULL) || (get_ip(data->IP) != get_ip(data->RealIP))))
    {
        res += "<br>";
        res += formatAddr(data->RealIP, data->Port.toULong());
    }
    return res;
}

QWidget *MSNClient::searchWindow(QWidget *parent)
{
    if (getState() != Connected)
        return NULL;
    return new MSNSearch(this, parent);
}


MSNUserData* MSNClient::toMSNUserData(SIM::clientData * data)
{
   // This function is used to more safely preform type conversion from SIM::clientData* into MSNUserData*
   // It will at least warn if the content of the structure is not MSNUserData
   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
   if (! data) return NULL;
   if (data->Sign.asULong() != MSN_SIGN)
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
        "ATTENTION!! Unsafly converting %s user data into MSN_SIGN",
         qPrintable(Sign));
//      abort();
   }
   return (MSNUserData*) data;
}


SBSocket::SBSocket(MSNClient *client, Contact *contact, MSNUserData *data)
{
    m_state		= Unknown;
    m_client	= client;
    m_contact	= contact;
    m_data		= data;
    m_socket	= new ClientSocket(this, client->createSBSocket());
    m_packet_id = 0;
    m_messageSize = 0;
    m_invite_cookie = get_random();
    m_bTyping	= false;
    m_client->m_SBsockets.push_back(this);
}

SBSocket::~SBSocket()
{
    if (m_packet)
        m_packet->clear();
    if (m_socket)
        delete m_socket;
    list<SBSocket*>::iterator it = find(m_client->m_SBsockets.begin(), m_client->m_SBsockets.end(), this);
    if (it != m_client->m_SBsockets.end())
        m_client->m_SBsockets.erase(it);
    if (m_data){
        m_data->sb.clear();
        if (m_data->typing_time.toULong()){
            m_data->typing_time.asULong() = 0;
            EventContact e(m_contact, EventContact::eStatus);;
            e.process();
        }
    }
    list<Message*>::iterator itm;
    for (itm = m_queue.begin(); itm != m_queue.end(); ++itm){
        Message *msg = (*itm);
        msg->setError(I18N_NOOP("Contact go offline"));
        EventMessageSent(msg).process();
        delete msg;
    }
    list<msgInvite>::iterator itw;
    for (itw = m_waitMsg.begin(); itw != m_waitMsg.end(); ++itw){
        Message *msg = itw->msg;
        msg->setError(I18N_NOOP("Contact go offline"));
        EventMessageSent(msg).process();
        delete msg;
    }
    for (itw = m_acceptMsg.begin(); itw != m_acceptMsg.end(); ++itw){
        Message *msg = itw->msg;
        EventMessageDeleted(msg).process();
        delete msg;
    }
}

void SBSocket::connect()
{
    m_packet = new XfrPacket(m_client, this);
    m_packet->send();
}

void SBSocket::connect(const QString &addr, const QString &session, const QString &cookie, bool bDirection)
{
    m_packet = NULL;
    if (m_state != Unknown){
        log(L_DEBUG, "Connect in bad state");
        return;
    }
    if (bDirection){
        m_state = ConnectingSend;
    }else{
        m_state = ConnectingReceive;
    }
    m_cookie = cookie;
    m_session = session;
    QString ip = addr;
    unsigned short port = 0;
    int n = ip.indexOf(':');
    if (n > 0){
        port = ip.mid(n + 1).toUShort();
        ip = ip.left(n);
    }
    if (port == 0){
        m_socket->error_state("Bad address");
        return;
    }
    m_socket->connect(ip, port, m_client);
}

bool SBSocket::send(Message *msg)
{

    m_bTyping = false;
    m_queue.push_back(msg);
    switch (m_state){
    case Unknown:
        connect();
        break;
    case Connected:
        process();
        break;
    default:
        break;
    }
    return true;
}

bool SBSocket::error_state(const QString&, unsigned)
{
    if (m_queue.size()){
        m_socket->close();
        connect();
        return false;
    }
    return true;
}

void SBSocket::connect_ready()
{
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
    QString args = m_client->data.owner.EMail.str();
    args += ' ';
    args += m_cookie;
    m_cookie = QString::null;
    switch (m_state){
    case ConnectingSend:
        send("USR", args);
        m_state = WaitJoin;
        break;
    case ConnectingReceive:
        args += " ";
        args += m_session;
        send("ANS", args);
        m_state = Connected;
        process();
        break;
    default:
        log(L_WARN, "Bad state for connect ready");
    }
}

void SBSocket::packet_ready()
{
    if (m_socket->readBuffer().writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->readBuffer(), false, plugin->MSNPacket);
    for (;;){
        if (m_messageSize && !getMessage())
            break;
        QByteArray s;
        if (!m_socket->readBuffer().scan("\r\n", s))
            break;
        getLine(s);
    }
    if (m_socket->readBuffer().readPos() == m_socket->readBuffer().writePos())
        m_socket->readBuffer().init(0);
}

void SBSocket::getMessage(unsigned size)
{
    m_messageSize = size;
    m_message = QString::null;
    getMessage();
}

bool SBSocket::getMessage()
{
    unsigned tail = m_socket->readBuffer().writePos() - m_socket->readBuffer().readPos();
    if (tail > m_messageSize)
        tail = m_messageSize;
    QString msg;
    m_socket->readBuffer().unpack(msg, tail);
    m_message += msg;
    m_messageSize -= tail;
    if (m_messageSize)
        return false;
    messageReady();
    return true;
}

void SBSocket::send(const QString &cmd, const QString &args)
{
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer()
    << (const char*)cmd.toUtf8()
    << " "
    << (const char*)QString::number(++m_packet_id).toUtf8();
    if (!args.isEmpty()){
        m_socket->writeBuffer()
        << " "
        << (const char*)args.toUtf8();
    }
    m_socket->writeBuffer() << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->MSNPacket);
    m_socket->write();
}

void SBSocket::getLine(const QByteArray &_line)
{
    QString line = QString::fromUtf8(_line);
    QString cmd = getToken(line, ' ');
    if (cmd == "BYE"){
        m_socket->error_state("");
        return;
    }
    if (cmd == "MSG"){
        QString email = getToken(line, ' ');
        getToken(line, ' ');
        unsigned size = line.toUInt();
        getMessage(size);
    }
    if (cmd == "JOI"){
        if (m_state != WaitJoin){
            log(L_WARN, "JOI in bad state");
            return;
        }
        m_state = Connected;
        process();
    }
    if (cmd == "USR")
        send("CAL", m_data->EMail.str());
    if ((cmd == "ACK") || (cmd == "NAK")){
        unsigned id = getToken(line, ' ').toUInt();
        if (id != m_msg_id){
            log(L_WARN, "Bad ACK id");
            return;
        }
        if (m_queue.empty())
            return;
        Message *msg = m_queue.front();
        if (cmd == "NAK"){
            m_msgText = QString::null;
            msg->setError(I18N_NOOP("Send message failed"));
            EventMessageSent(msg).process();
            delete msg;
            m_queue.erase(m_queue.begin());
            process(false);
            return;
        }
        if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
            Message m(MessageGeneric);
            m.setContact(m_contact->id());
            m.setClient(m_client->dataName(m_data));
            m.setText(m_msgPart);
            m.setForeground(msg->getForeground());
            m.setBackground(0xFFFFFF);
            m.setFont(msg->getFont());
            EventSent(&m).process();
        }
        if (m_msgText.isEmpty()){
            if (msg->type() == MessageFile){
                sendFile();
            }else{
                EventMessageSent(msg).process();
                delete msg;
                m_queue.erase(m_queue.begin());
            }
        }
        process();
    }
}

typedef map<QString, QString>   STR_VALUES;

static STR_VALUES parseValues(const QString &str)
{
    STR_VALUES res;
    QString s = str.trimmed();
    while (!s.isEmpty()){
        QString p = getToken(s, ';', false).trimmed();
        QString key = getToken(p, '=', false);
        STR_VALUES::iterator it = res.find(key);
        if (it == res.end()){
            res.insert(STR_VALUES::value_type(key, p));
        }else{
            it->second = p;
        }
        s = s.trimmed();
    }
    return res;
}

static char FT_GUID[] = "{5D3E02AB-6190-11d3-BBBB-00C04F795683}";

void SBSocket::messageReady()
{
    log(L_DEBUG, "MSG: [%s]", qPrintable(m_message));
    QString content_type;
    QString charset;
    QString font;
    QString typing;
    unsigned color = 0;
    bool bColor = false;
    while (!m_message.isEmpty()){
        int n = m_message.indexOf("\r\n");
        if (n < 0){
            log(L_DEBUG, "Error parse message");
            return;
        }
        if (n == 0){
            m_message = m_message.mid(2);
            break;
        }
        QString line = m_message.left(n);
        m_message = m_message.mid(n + 2);
        QString key = getToken(line, ':', false);
        if (key == "Content-Type"){
            line = line.trimmed();
            content_type = getToken(line, ';').trimmed();
            STR_VALUES v = parseValues(line.trimmed());
            STR_VALUES::iterator it = v.find("charset");
            if (it != v.end())
                charset = it->second;
            continue;
        }
        if (key == "X-MMS-IM-Format"){
            STR_VALUES v = parseValues(line.trimmed());
            STR_VALUES::iterator it = v.find("FN");
            if (it != v.end())
                font = m_client->unquote(it->second);
            it = v.find("EF");
            if (it != v.end()){
                QString effects = it->second;
                if (effects.indexOf('B') != -1)
                    font += ", bold";
                if (effects.indexOf('I') != -1)
                    font += ", italic";
                if (effects.indexOf('S') != -1)
                    font += ", strikeout";
                if (effects.indexOf('U') != -1)
                    font += ", underline";
            }
            it = v.find("CO");
            if (it != v.end())
                color = it->second.toULong(&bColor, 16);
            continue;
        }
        if (key == "TypingUser"){
            typing = line.trimmed();
            continue;
        }
    }
    if (content_type == "text/plain"){
        if (m_data->typing_time.toULong()){
            m_data->typing_time.asULong() = 0;
            EventContact e(m_contact, EventContact::eStatus);;
            e.process();
        }
        QString msg_text = m_message;
        msg_text = msg_text.remove('\r');
        Message *msg = new Message(MessageGeneric);
        msg->setFlags(MESSAGE_RECEIVED);
        if (bColor){
            msg->setBackground(0xFFFFFF);
            msg->setForeground(color);
        }
        msg->setFont(font);
        msg->setText(msg_text);
        msg->setContact(m_contact->id());
        msg->setClient(m_client->dataName(m_data));
        EventMessageReceived e(msg);
        if (!e.process())
            delete msg;
        return;
    }
    if (content_type == "text/x-msmsgscontrol"){
        if (typing.toLower() == m_data->EMail.str().toLower()){
            bool bEvent = (m_data->typing_time.toULong() == 0);
            QDateTime now(QDateTime::currentDateTime());
            m_data->typing_time.asULong() = now.toTime_t();
            if (bEvent)
            {
                EventContact e(m_contact, EventContact::eStatus);;
                e.process();
            }
        }
    }
    if (content_type == "text/x-msmsgsinvite"){
        QString file;
        QString command;
        QString guid;
        QString code;
        QString ip_address;
        QString ip_address_internal;
        unsigned short port = 0;
        unsigned short port_x = 0;
        unsigned cookie = 0;
        unsigned auth_cookie = 0;
        unsigned fileSize = 0;
        while (!m_message.isEmpty()){
            QString line;
            int n = m_message.indexOf("\r\n");
            if (n < 0){
                line = m_message;
                m_message = QString::null;
            }else{
                line = m_message.left(n);
                m_message = m_message.mid(n + 2);
            }
            QString key = getToken(line, ':', false);
            line = line.trimmed();
            if (key == "Application-GUID"){
                guid = line;
                continue;
            }
            if (key == "Invitation-Command"){
                command = line;
                continue;
            }
            if (key == "Invitation-Cookie"){
                cookie = line.toULong();
                continue;
            }
            if (key == "Application-File"){
                file = line;
                continue;
            }
            if (key == "Application-FileSize"){
                fileSize = line.toULong();
                continue;
            }
            if (key == "Cancel-Code"){
                code = line;
                continue;
            }
            if (key == "IP-Address"){
                ip_address = line;
                continue;
            }
            if (key == "IP-Address-Internal"){
                ip_address_internal = line;
                continue;
            }
            if (key == "Port"){
                port =  line.toUShort();
                continue;
            }
            if (key == "PortX"){
                port_x = line.toUShort();
                continue;
            }
            if (key == "AuthCookie"){
                auth_cookie = line.toULong();
                continue;
            }

        }
        if (cookie == 0){
            log(L_WARN, "No cookie in message");
            return;
        }
        if (command == "INVITE"){
            if (guid != FT_GUID){
                log(L_WARN, "Unknown GUID %s", qPrintable(guid));
                return;
            }
            if (file.isEmpty()){
                log(L_WARN, "No file in message");
                return;
            }
            FileMessage *msg = new FileMessage;
            msg->setDescription(m_client->unquote(file));
            msg->setSize(fileSize);
            msg->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
            msg->setContact(m_contact->id());
            msg->setClient(m_client->dataName(m_data));
            msgInvite m;
            m.msg    = msg;
            m.cookie = cookie;
            m_acceptMsg.push_back(m);
            EventMessageReceived e(msg);
            if (e.process()){
                for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                    if (it->msg == msg){
                        m_acceptMsg.erase(it);
                        break;
                    }
                }
            }
        }else if (command == "ACCEPT"){
            unsigned ip      = QHostAddress(ip_address).toIPv4Address();
            unsigned real_ip = QHostAddress(ip_address_internal).toIPv4Address();
            if (ip != INADDR_NONE)
                set_ip(&m_data->IP, ip);
            if (real_ip != INADDR_NONE)
                set_ip(&m_data->RealIP, real_ip);
            if (port)
                m_data->Port.asULong() = port;
            list<msgInvite>::iterator it;
            for (it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
                if (it->cookie == cookie){
                    Message *msg = it->msg;
                    if (msg->type() == MessageFile){
                        m_waitMsg.erase(it);
                        FileMessage *m = static_cast<FileMessage*>(msg);
                        MSNFileTransfer *ft;
                        bool bNew = false;
                        if (m->m_transfer){
                            ft = static_cast<MSNFileTransfer*>(m->m_transfer);
                        }else{
                            ft = new MSNFileTransfer(m, m_client, m_data);
                            bNew = true;
                        }
                        ft->ip1   = real_ip;
                        ft->port1 = port;
                        ft->ip2	  = ip;
                        ft->port2 = port_x;
                        ft->auth_cookie = auth_cookie;

                        if (bNew){
                            EventMessageAcked(msg).process();
                        }
                        ft->connect();
                        break;
                    }
                    msg->setError("Bad type");
                    EventMessageSent(msg).process();
                    delete msg;
                    return;
                }
            }
            if (it == m_waitMsg.end())
                log(L_WARN, "No message for accept");
            return;
        }else if (command == "CANCEL"){
            if (code == "REJECT"){
                list<msgInvite>::iterator it;
                for (it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
                    if (it->cookie == cookie){
                        Message *msg = it->msg;
                        msg->setError(I18N_NOOP("Message declined"));
                        EventMessageSent(msg).process();
                        delete msg;
                        m_waitMsg.erase(it);
                        break;
                    }
                }
                if (it == m_waitMsg.end())
                    log(L_WARN, "No message for cancel");
                return;
            }
            list<msgInvite>::iterator it;
            for (it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                if (it->cookie == cookie){
                    Message *msg = it->msg;
                    EventMessageDeleted(msg).process();
                    delete msg;
                    m_acceptMsg.erase(it);
                    break;
                }
            }
            if (it == m_acceptMsg.end())
                log(L_WARN, "No message for cancel");
        }else{
            log(L_WARN, "Unknown command %s", qPrintable(command));
            return;
        }
    }
}

void SBSocket::timer(unsigned now)
{
    if (m_data->typing_time.toULong()){
        if (now >= m_data->typing_time.toULong() + TYPING_TIME){
            m_data->typing_time.asULong() = 0;
            EventContact e(m_contact, EventContact::eStatus);;
            e.process();
        }
    }
    sendTyping();
}

void SBSocket::setTyping(bool bTyping)
{
    if (m_bTyping == bTyping)
        return;
    m_bTyping = bTyping;
    sendTyping();
}

void SBSocket::sendTyping()
{
    if (m_bTyping && (m_state == Connected)){
        QString message;
        message += "MIME-Version: 1.0\r\n";
        message += "Content-Type: text/x-msmsgcontrol\r\n";
        message += "TypingUser: ";
        message += m_client->data.owner.EMail.str();
        message += "\r\n";
        message += "\r\n";
        sendMessage(message, "U");
    }
}

void SBSocket::sendMessage(const QString &str, const char *type)
{
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer()
    << "MSG "
    << (const char*)QString::number(++m_packet_id).toUtf8()
    << " "
    << type
    << " "
    << (const char*)QString::number(str.toUtf8().length()).toUtf8()
    << "\r\n"
    << (const char*)str.toUtf8();
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->MSNPacket);
    m_socket->write();
}

bool SBSocket::cancelMessage(Message *msg)
{
    if (m_queue.empty())
        return false;
    if (m_queue.front() == msg){
        m_msgPart = QString::null;
        m_msgText = QString::null;
        m_msg_id = 0;
        m_queue.erase(m_queue.begin());
        process();
        return true;
    }
    list<Message*>::iterator it = find(m_queue.begin(), m_queue.end(), msg);
    if (it == m_queue.end())
        return false;
    m_queue.erase(it);
    delete msg;
    return true;
}

void SBSocket::sendFile()
{
    if (m_queue.empty())
        return;
    Message *msg = m_queue.front();
    if (msg->type() != MessageFile)
        return;
    m_queue.erase(m_queue.begin());
    FileMessage *m = static_cast<FileMessage*>(msg);
    if (++m_invite_cookie == 0)
        m_invite_cookie++;
    msgInvite mi;
    mi.msg    = msg;
    mi.cookie = m_invite_cookie;
    m_waitMsg.push_back(mi);
    QString message;
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "Application-Name: File Transfer\r\n"
               "Application-GUID: ";
    message += FT_GUID;
    message += "\r\n"
               "Invitation-Command: INVITE\r\n"
               "Invitation-Cookie: ";
    message += QString::number(m_invite_cookie);
    message += "\r\n"
               "Application-File: ";
    QString name;
    unsigned size;
    if (m->m_transfer){
        name = m->m_transfer->m_file->fileName();
        size = m->m_transfer->fileSize();
    }else{
        FileMessage::Iterator it(*m);
        if (it[0])
            name = *it[0];
        size = it.size();
    }
    name = name.replace('\\', '/');
    int n = name.lastIndexOf('/');
    if (n >= 0)
        name = name.mid(n + 1);
    message += m_client->quote(name);
    message += "\r\n"
               "Application-FileSize: ";
    message += QString::number(size);
    message += "\r\n"
               "Connectivity: N\r\n\r\n";
    sendMessage(message, "S");
}

void SBSocket::process(bool bTyping)
{
    if (bTyping)
        sendTyping();
    if (m_msgText.isEmpty() && !m_queue.empty()){
        Message *msg = m_queue.front();
        EventSend e(msg, msg->getPlainText().toUtf8());
        e.process();
        m_msgText = QString::fromUtf8( e.localeText() );
        if (msg->type() == MessageUrl){
            UrlMessage *m = static_cast<UrlMessage*>(msg);
            QString msgText = m->getUrl();
            msgText += "\r\n";
            msgText += m_msgText;
            m_msgText = msgText;
        }
        if ((msg->type() == MessageFile) && static_cast<FileMessage*>(msg)->m_transfer)
            m_msgText = QString::null;
        if (m_msgText.isEmpty()){
            if (msg->type() == MessageFile){
                sendFile();
                return;
            }
            EventMessageSent(msg).process();
            delete msg;
            m_queue.erase(m_queue.begin());
        }
        m_msgText = m_msgText.replace('\n', "\r\n");
    }
    if (m_msgText.isEmpty())
        return;
    m_msgPart = getPart(m_msgText, 1664);
    Message *msg = m_queue.front();
    char color[10];
    sprintf(color, "%06lX", msg->getBackground());
    QString message;
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/plain; charset=UTF-8\r\n";
    message += "X-MMS_IM-Format: ";
    if (!msg->getFont().isEmpty()){
        QString font = msg->getFont();
        if (!font.isEmpty()){
            QString font_type;
            int n = font.indexOf(", ");
            if (n > 0){
                font_type = font.mid(n + 2);
                font = font.left(n);
            }
            message += "FN=";
            message += m_client->quote(font);
            QString effect;
            while (!font_type.isEmpty()){
                QString type = font_type;
                int n = font_type.indexOf(", ");
                if (n > 0){
                    type = font_type.mid(n);
                    font_type = font_type.mid(n + 2);
                }else{
                    font_type = QString::null;
                }
                if (type == "bold")
                    effect += "B";
                if (type == "italic")
                    effect += "I";
                if (type == "strikeout")
                    effect += "S";
                if (type == "underline")
                    effect += "U";
            }
            if (!effect.isEmpty()){
                message += "; EF=";
                message += effect;
            }
        }
    }
    message += "; CO=";
    message += color;
    message += "; CS=0\r\n";
    message += "\r\n";
    message += m_msgPart;
    sendMessage(message, "A");
    m_msg_id = m_packet_id;
}

void SBSocket::acceptMessage(unsigned short port, unsigned cookie, unsigned auth_cookie)
{
    QString message;
    message += "MIME-Version: 1.0\r\n"
               "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "IP-Address: ";
    message += QHostAddress(get_ip(m_client->data.owner.IP)).toString();
    message += "\r\n"
               "IP-Address-Internal: ";
    message += QHostAddress(m_client->socket()->localHost()).toString();
    message += "\r\n"
               "Port: ";
    message += QString::number(port);
    message += "\r\n"
               "AuthCookie: ";
    message += QString::number(auth_cookie);
    message += "\r\n"
               "Sender-Connect: TRUE\r\n"
               "Invitation-Command: ACCEPT\r\n"
               "Invitation-Cookie: ";
    message += QString::number(cookie);
    message += "\r\n"
               "Launch-Application: FALSE\r\n"
               "Request-Data: IP-Address:\r\n\r\n";
    sendMessage(message, "N");
}

bool SBSocket::acceptMessage(Message *msg, const QString &dir, OverwriteMode mode)
{
    for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
        if (it->msg->id() != msg->id())
            continue;
        Message *msg = it->msg;
        unsigned cookie = it->cookie;
        m_acceptMsg.erase(it);
        MSNFileTransfer *ft = new MSNFileTransfer(static_cast<FileMessage*>(msg), m_client, m_data);
        ft->setDir(dir);
        ft->setOverwrite(mode);
        ft->auth_cookie = get_random();
        ft->cookie = cookie;
        EventMessageAcked(msg).process();
        ft->listen();
        EventMessageDeleted(msg).process();
        return true;
    }
    return false;
}

void SBSocket::declineMessage(unsigned cookie)
{
    QString message;
    message += "MIME-Version: 1.0\r\n"
               "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "Invitation-Command: CANCEL\r\n"
               "Invitation-Cookie: ";
    message += QString::number(cookie);
    message += "\r\n"
               "Cancel-Code: REJECT\r\n\r\n";
    sendMessage(message, "S");
}

bool SBSocket::declineMessage(Message *msg, const QString &reason)
{
    for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
        if (it->msg->id() != msg->id())
            continue;
        Message *msg = it->msg;
        unsigned cookie = it->cookie;
        m_acceptMsg.erase(it);
        declineMessage(cookie);
        if (!reason.isEmpty()){
            Message *msg = new Message(MessageGeneric);
            msg->setText(reason);
            msg->setFlags(MESSAGE_NOHISTORY);
            if (!m_client->send(msg, m_data))
                delete msg;
        }
        delete msg;
        return true;
    }
    return false;
}

