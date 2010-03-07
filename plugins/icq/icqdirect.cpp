/***************************************************************************
                          icqdirect.cpp  -  description
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


#include "icqclient.h"
#include "icqmessage.h"
#include "icqdirect.h"
#include "contacts/contact.h"

#include "core_events.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <QFile>
#include <QTimer>
#include <QRegExp>
#include <QList>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>

#include "log.h"

using namespace std;
using namespace SIM;

const unsigned short TCP_START  = 0x07EE;
const unsigned short TCP_ACK    = 0x07DA;
const unsigned short TCP_CANCEL	= 0x07D0;

const char FT_INIT		= 0;
const char FT_INIT_ACK	= 1;
const char FT_FILEINFO	= 2;
const char FT_START		= 3;
const char FT_SPEED		= 5;
const char FT_DATA		= 6;

const unsigned DIRECT_TIMEOUT	= 10;

ICQListener::ICQListener(ICQClient *client)
{
    m_client = client;
}

ICQListener::~ICQListener()
{
    if (m_client == NULL)
        return;
    m_client->m_listener = NULL;
    m_client->data.owner.Port.asULong() = 0;
}

bool ICQListener::accept(Socket *s, unsigned long ip)
{
    log(L_DEBUG, "Accept direct connection %s", qPrintable(QHostAddress(ip).toString()));
    m_client->m_sockets.push_back(new DirectClient(s, m_client, ip));
    return false;
}

void ICQListener::bind_ready(unsigned short port)
{
    m_client->data.owner.Port.asULong() = port;
}

bool ICQListener::error(const QString &err)
{
    log(L_WARN, "ICQListener error: %s", qPrintable(err));
    m_client->m_listener = NULL;
    m_client->data.owner.Port.asULong() = 0;
    m_client = NULL;
    return true;
}

// ___________________________________________________________________________________________

DirectSocket::DirectSocket(Socket *s, ICQClient *client, unsigned long ip)
{
    m_socket = new ICQClientSocket(this);
    m_socket->setSocket(s);
    m_bIncoming = true;
    m_client = client;
    m_state = WaitInit;
    m_version = 0;
    m_data	= NULL;
    m_port  = 0;
    m_ip    = ip;
    init();
}

DirectSocket::DirectSocket(ICQUserData *data, ICQClient *client)
{
    m_socket    = new ICQClientSocket(this);
    m_bIncoming = false;
    m_version   = (char)(data->Version.toULong());
    m_client    = client;
    m_state     = NotConnected;
    m_data		= data;
    m_port		= 0;
    m_localPort = 0;
    m_ip		= 0;
    init();
}

DirectSocket::~DirectSocket()
{
    if (m_socket)
        delete m_socket;
    removeFromClient();
}

void DirectSocket::timeout()
{
    if ((m_state != Logged) && m_socket)
        login_timeout();
}

void DirectSocket::login_timeout()
{
    m_socket->error_state("Timeout direct connection");
    if (m_data)
        m_data->bNoDirect.asBool() = true;
}

void DirectSocket::removeFromClient()
{
    for (list<DirectSocket*>::iterator it = m_client->m_sockets.begin(); it != m_client->m_sockets.end(); ++it){
        if (*it == this){
            m_client->m_sockets.erase(it);
            break;
        }
    }
}

void DirectSocket::init()
{
    if (!m_socket->created())
        m_socket->error_state("Connect error");
    m_nSequence = 0xFFFF;
    m_socket->writeBuffer().init(0);
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
}

unsigned long DirectSocket::Uin()
{
    if (m_data)
        return m_data->Uin.toULong();
    return 0;
}

unsigned short DirectSocket::localPort()
{
    return m_localPort;
}

unsigned short DirectSocket::remotePort()
{
    return m_port;
}

bool DirectSocket::error_state(const QString &error, unsigned)
{
    if ((m_state == ConnectIP1) || (m_state == ConnectIP2)){
        connect();
        return false;
    }
    if (!error.isEmpty())
        log(L_WARN, "Direct socket error %s", qPrintable(error));
    return true;
}

void DirectSocket::connect()
{
	log(L_DEBUG, "DirectSocket::connect()");
    m_socket->writeBuffer().init(0);
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
    if (m_port == 0){
        m_state = ConnectFail;
        m_socket->error_state(I18N_NOOP("Connect to unknown port"));
        return;
    }
    if (m_state == NotConnected){
        m_state = ConnectIP1;
        unsigned long ip = get_ip(m_data->RealIP);
        if (get_ip(m_data->IP) != get_ip(m_client->data.owner.IP))
            ip = 0;
        if (ip){
            m_socket->connect(QHostAddress(ip).toString(), m_port, NULL);
            return;
        }
    }
    if (m_state == ConnectIP1){
        m_state = ConnectIP2;
        unsigned long ip = get_ip(m_data->IP);
        if ((ip == get_ip(m_client->data.owner.IP)) && (ip == get_ip(m_data->RealIP)))
            ip = 0;
        if (ip){
            m_socket->connect(QHostAddress(ip).toString(), m_port, m_client);
            return;
        }
    }
    m_state = ConnectFail;
    m_socket->error_state(I18N_NOOP("Can't established direct connection"));
}

void DirectSocket::reverseConnect(unsigned long ip, unsigned short port)
{
    if (m_state != NotConnected){
        log(L_WARN, "Bad state for reverse connect");
        return;
    }
    m_bIncoming = true;
    m_state = ReverseConnect;
    m_socket->connect(QHostAddress(ip).toString(), port, NULL);
}

void DirectSocket::acceptReverse(Socket *s)
{
    if (m_state != WaitReverse){
        log(L_WARN, "Accept reverse in bad state");
        if (s)
            delete s;
        return;
    }
    if (s == NULL){
        m_socket->error_state("Reverse fail");
        return;
    }
    delete m_socket->socket();
    m_socket->setSocket(s);
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader   = true;
    m_state     = WaitInit;
    m_bIncoming = true;
}

void DirectSocket::packet_ready()
{
	log(L_DEBUG, "DirectSocket::packet_ready()");
    if (m_bHeader){
        unsigned short size;
        m_socket->readBuffer().unpack(size);
        if (size){
            m_socket->readBuffer().add(size);
            m_bHeader = false;
            return;
        }
    }
    if (m_state != Logged){
        ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
        EventLog::log_packet(m_socket->readBuffer(), false, plugin->ICQDirectPacket, QString::number((unsigned long)this));
    }
    switch (m_state){
    case Logged:{
            processPacket();
            break;
        }
    case WaitAck:{
            unsigned short s1, s2;
            m_socket->readBuffer().unpack(s1);
            m_socket->readBuffer().unpack(s2);
            if (s2 != 0){
                m_socket->error_state("Bad ack");
                return;
            }
            if (m_bIncoming){
                m_state = Logged;
                connect_ready();
            }else{
                m_state = WaitInit;
            }
            break;
        }
    case WaitInit:{
            char cmd;
            m_socket->readBuffer().unpack(cmd);
            if ((unsigned char)cmd != 0xFF){
                m_socket->error_state("Bad direct init command");
                return;
            }
            m_socket->readBuffer().unpack(m_version);
            if (m_version < 6){
                m_socket->error_state("Use old protocol");
                return;
            }
            m_socket->readBuffer().incReadPos(3);
            unsigned long my_uin;
            m_socket->readBuffer().unpack(my_uin);
            if (my_uin != m_client->data.owner.Uin.toULong()){
                m_socket->error_state("Bad owner UIN");
                return;
            }
            m_socket->readBuffer().incReadPos(6);
            unsigned long p_uin;
            m_socket->readBuffer().unpack(p_uin);
            if (m_data == NULL){
                Contact *contact;
                m_data = m_client->findContact(p_uin, NULL, false, contact);
                if ((m_data == NULL) || contact->getIgnore()){
                    m_socket->error_state("User not found");
                    return;
                }
                if ((m_client->getInvisible() && (m_data->VisibleId.toULong() == 0)) ||
                        (!m_client->getInvisible() && m_data->InvisibleId.toULong())){
                    m_socket->error_state("User not found");
                    return;
                }
            }
            if (p_uin != m_data->Uin.toULong()){
                m_socket->error_state("Bad sender UIN");
                return;
            }
            if (get_ip(m_data->RealIP) == 0)
                set_ip(&m_data->RealIP, m_ip);
            m_socket->readBuffer().incReadPos(13);
            unsigned long sessionId;
            m_socket->readBuffer().unpack(sessionId);
            if (m_bIncoming){
                m_nSessionId = sessionId;
                sendInitAck();
                sendInit();
                m_state = WaitAck;
            }else{
                if (sessionId != m_nSessionId){
                    m_socket->error_state("Bad session ID");
                    return;
                }
                sendInitAck();
                m_state = Logged;
                connect_ready();
            }
            break;
        }
    default:
        m_socket->error_state("Bad session ID");
        return;
    }
    if (m_socket == NULL){
        delete this;
        return;
    }
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
}

void DirectSocket::sendInit()
{
	log(L_DEBUG, "DirectSocket::sendInit()");
    if (!m_bIncoming && (m_state != ReverseConnect)){
        if (m_data->DCcookie.toULong() == 0){
            m_socket->error_state("No direct info");
            return;
        }
        m_nSessionId = m_data->DCcookie.toULong();
    }

    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer().pack((unsigned short)((m_version >= 7) ? 0x0030 : 0x002c));
    m_socket->writeBuffer().pack('\xFF');
    m_socket->writeBuffer().pack((unsigned short)m_version);
    m_socket->writeBuffer().pack((unsigned short)((m_version >= 7) ? 0x002b : 0x0027));
    m_socket->writeBuffer().pack(m_data->Uin.toULong());
    m_socket->writeBuffer().pack((unsigned short)0x0000);
    m_socket->writeBuffer().pack(m_data->Port.toULong());
    m_socket->writeBuffer().pack(m_client->data.owner.Uin.toULong());
    m_socket->writeBuffer().pack(get_ip(m_client->data.owner.IP));
    m_socket->writeBuffer().pack(get_ip(m_client->data.owner.RealIP));
    m_socket->writeBuffer().pack((char)0x04);
    m_socket->writeBuffer().pack(m_data->Port.toULong());
    m_socket->writeBuffer().pack(m_nSessionId);
    m_socket->writeBuffer().pack(0x00000050L);
    m_socket->writeBuffer().pack(0x00000003L);
    if (m_version >= 7)
        m_socket->writeBuffer().pack(0x00000000L);
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->ICQDirectPacket, QString::number((unsigned long)this));
    m_socket->write();
}

void DirectSocket::sendInitAck()
{
	log(L_DEBUG, "DirectSocket::sendInitAck()");
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer().pack((unsigned short)0x0004);
    m_socket->writeBuffer().pack((unsigned short)0x0001);
    m_socket->writeBuffer().pack((unsigned short)0x0000);
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->ICQDirectPacket, QString::number((unsigned long)this));
    m_socket->write();
}

void DirectSocket::connect_ready()
{
	log(L_DEBUG, "DirectSocket::connect_ready()");
    QTimer::singleShot(DIRECT_TIMEOUT * 1000, this, SLOT(timeout()));
    if (m_bIncoming){
        if (m_state == ReverseConnect)
            m_state = WaitInit;
    }else{
        sendInit();
        m_state = WaitAck;
    }
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
}

// ___________________________________________________________________________________________

static unsigned char client_check_data[] =
    {
        "As part of this software beta version Mirabilis is "
        "granting a limited access to the ICQ network, "
        "servers, directories, listings, information and databases (\""
        "ICQ Services and Information\"). The "
        "ICQ Service and Information may databases (\""
        "ICQ Services and Information\"). The "
        "ICQ Service and Information may\0"
    };

DirectClient::DirectClient(Socket *s, ICQClient *client, unsigned long ip)
        : DirectSocket(s, client, ip)
{
    m_channel = PLUGIN_NULL;
    m_state = WaitLogin;
    m_ssl = NULL;
}

DirectClient::DirectClient(ICQUserData *data, ICQClient *client, unsigned channel)
        : DirectSocket(data, client)
{
    m_state   = None;
    m_channel = channel;
    m_port    = (unsigned short)(data->Port.toULong());
    m_ssl = NULL;
}

DirectClient::~DirectClient()
{
    error_state(QString::null, 0);
    switch (m_channel){
    case PLUGIN_NULL:
        if (m_data && (m_data->Direct.object() == this))
            m_data->Direct.clear();
        break;
    case PLUGIN_INFOxMANAGER:
        if (m_data && (m_data->DirectPluginInfo.object() == this))
            m_data->DirectPluginInfo.clear();
        break;
    case PLUGIN_STATUSxMANAGER:
        if (m_data && (m_data->DirectPluginStatus.object() == this))
            m_data->DirectPluginStatus.clear();
        break;
    }
    secureStop(false);
}

bool DirectClient::isSecure()
{
    return m_ssl && m_ssl->connected();
}

void DirectClient::processPacket()
{
	log(L_DEBUG, "DirectSocket::processPacket()");
    switch (m_state){
    case None:
        m_socket->error_state("Bad state process packet");
        return;
    case WaitInit2:
        if (m_bIncoming){
            ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
            EventLog::log_packet(m_socket->readBuffer(), false, plugin->ICQDirectPacket, QString::number((unsigned long)this));
            if (m_version < 8){
                if (m_data->Direct.object()){
                    m_socket->error_state("Direct connection already established");
                    return;
                }
                m_state = Logged;
                processMsgQueue();
                break;
            }
            plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
            EventLog::log_packet(m_socket->readBuffer(), false, plugin->ICQDirectPacket, QString::number((unsigned long)this));
            m_socket->readBuffer().incReadPos(13);
            char p[16];
            m_socket->readBuffer().unpack(p, 16);
            for (m_channel = 0; m_channel <= PLUGIN_NULL; m_channel++){
                if (!memcmp(m_client->plugins[m_channel], p, 16))
                    break;
            }
            removeFromClient();
            switch (m_channel){
            case PLUGIN_INFOxMANAGER: {
                DirectClient *dc = dynamic_cast<DirectClient*>(m_data->DirectPluginInfo.object());
                if (dc){
                    if (dc->copyQueue(this)){
                        delete dc;
                        m_data->DirectPluginInfo.setObject(this);
                    }else{
                        m_socket->error_state("Plugin info connection already established");
                    }
                }else{
                    m_data->DirectPluginInfo.setObject(this);
                }
                break;
            }
            case PLUGIN_STATUSxMANAGER: {
                DirectClient *dc = dynamic_cast<DirectClient*>(m_data->DirectPluginStatus.object());
                if (dc){
                    if (dc->copyQueue(this)){
                        delete dc;
                        m_data->DirectPluginStatus.setObject(this);
                    }else{
                        m_socket->error_state("Plugin status connection already established");
                    }
                }else{
                    m_data->DirectPluginStatus.setObject(this);
                }
                break;
            }
            case PLUGIN_NULL: {
                DirectClient *dc = dynamic_cast<DirectClient*>(m_data->Direct.object());
                if (dc){
                    if (dc->copyQueue(this)){
                        delete dc;
                        m_data->Direct.setObject(this);
                    }else{
                        m_socket->error_state("Direct connection already established");
                    }
                }else{
                    m_data->Direct.setObject(this);
                }
                break;
            }
            default:
                m_socket->error_state("Unknown direct channel");
                return;
            }
            sendInit2();
        }
        m_state = Logged;
        processMsgQueue();
        return;
    default:
        break;
    }
    unsigned long hex, key, B1, M1;
    unsigned int i;
    unsigned char X1, X2, X3;

    unsigned int correction = 2;
    if (m_version >= 7)
        correction++;

    unsigned int size = m_socket->readBuffer().size() - correction;
    if (m_version >= 7) m_socket->readBuffer().incReadPos(1);

    unsigned long check;
    m_socket->readBuffer().unpack(check);

    // main XOR key
    key = 0x67657268 * size + check;

    unsigned char *p = (unsigned char*)m_socket->readBuffer().data(m_socket->readBuffer().readPos()-4);
    for(i=4; i<(size+3)/4; i+=4) {
        hex = key + client_check_data[i&0xFF];
        p[i] ^= hex&0xFF;
        p[i+1] ^= (hex>>8) & 0xFF;
        p[i+2] ^= (hex>>16) & 0xFF;
        p[i+3] ^= (hex>>24) & 0xFF;
    }

    B1 = (p[4] << 24) | (p[6] << 16) | (p[4] <<8) | (p[6]<<0);

    // special decryption
    B1 ^= check;

    // validate packet
    M1 = (B1 >> 24) & 0xFF;
    if(M1 < 10 || M1 >= size){
        m_socket->error_state("Decrypt packet failed");
        return;
    }

    X1 = (unsigned char)(p[M1] ^ 0xFF);
    if(((B1 >> 16) & 0xFF) != X1){
        m_socket->error_state("Decrypt packet failed");
        return;
    }

    X2 = (unsigned char)((B1 >> 8) & 0xFF);
    if(X2 < 220) {
        X3 = (unsigned char)(client_check_data[X2] ^ 0xFF);
        if((B1 & 0xFF) != X3){
            m_socket->error_state("Decrypt packet failed");
            return;
        }
    }
    ICQPlugin *icq_plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->readBuffer(), false, icq_plugin->ICQDirectPacket, name());

    m_socket->readBuffer().setReadPos(2);
    if (m_version >= 7){
        char startByte;
        m_socket->readBuffer().unpack(startByte);
        if (startByte != 0x02){
            m_socket->error_state("Bad start byte");
            return;
        }
    }
    unsigned long checksum;
    m_socket->readBuffer().unpack(checksum);
    unsigned short command;
    m_socket->readBuffer().unpack(command);
    m_socket->readBuffer().incReadPos(2);
    unsigned short seq;
    m_socket->readBuffer().unpack(seq);
    m_socket->readBuffer().incReadPos(12);

    unsigned short type, ackFlags, msgFlags;
    m_socket->readBuffer().unpack(type);
    m_socket->readBuffer().unpack(ackFlags);
    m_socket->readBuffer().unpack(msgFlags);
    QByteArray msg_str;
    m_socket->readBuffer() >> msg_str;
    Message *m;
    switch (command){
    case TCP_START:
        switch (type){
        case ICQ_MSGxAR_AWAY:
        case ICQ_MSGxAR_OCCUPIED:
        case ICQ_MSGxAR_NA:
        case ICQ_MSGxAR_DND:
        case ICQ_MSGxAR_FFC:{
                unsigned req_status = STATUS_AWAY;
                switch (type){
                case ICQ_MSGxAR_OCCUPIED:
                    req_status = STATUS_OCCUPIED;
                    break;
                case ICQ_MSGxAR_NA:
                    req_status = STATUS_NA;
                    break;
                case ICQ_MSGxAR_DND:
                    req_status = STATUS_DND;
                    break;
                case ICQ_MSGxAR_FFC:
                    req_status = STATUS_FFC;
                    break;
                }
                ar_request req;
                req.screen  = m_client->screen(m_data);
                req.type    = type;
                req.flags   = msgFlags;
                req.id.id_l = seq;
                req.id1     = 0;
                req.id2     = 0;
                req.bDirect = true;
                m_client->arRequests.push_back(req);

                Contact *contact = NULL;
                m_client->findContact(m_client->screen(m_data), NULL, false, contact);
                ARRequest ar;
                ar.contact  = contact;
                ar.param    = &m_client->arRequests.back();
                ar.receiver = m_client;
                ar.status   = req_status;
                EventARRequest(&ar).process();
                return;
            }
        case ICQ_MSGxSECURExOPEN:
        case ICQ_MSGxSECURExCLOSE:
            msg_str = NULL;
            msg_str = "1";
            sendAck(seq, type, msgFlags, msg_str);
            if (type == ICQ_MSGxSECURExOPEN){
                secureListen();
            }else{
                secureStop(true);
            }
            return;
        }
        if (m_channel == PLUGIN_NULL){
            MessageId id;
            id.id_l = seq;
            m = m_client->parseMessage(type, m_client->screen(m_data), msg_str, m_socket->readBuffer(), id, 0);
            if (m == NULL){
                m_socket->error_state("Start without message");
                return;
            }
            unsigned flags = m->getFlags() | MESSAGE_RECEIVED | MESSAGE_DIRECT;
            if (isSecure())
                flags |= MESSAGE_SECURE;
            m->setFlags(flags);
            bool bAccept = true;
            switch (m_client->getStatus()){
            case STATUS_DND:
                if (!m_client->getAcceptInDND())
                    bAccept = false;
                break;
            case STATUS_OCCUPIED:
                if (!m_client->getAcceptInOccupied())
                    bAccept = false;
                break;
            }
            if (msgFlags & (ICQ_TCPxMSG_URGENT | ICQ_TCPxMSG_LIST))
                bAccept = true;
            if (bAccept){
                if (msgFlags & ICQ_TCPxMSG_URGENT)
                    m->setFlags(m->getFlags() | MESSAGE_URGENT);
                if (msgFlags & ICQ_TCPxMSG_LIST)
                    m->setFlags(m->getFlags() | MESSAGE_LIST);
                if (m_client->messageReceived(m, m_client->screen(m_data)))
                    sendAck(seq, type, msgFlags);
            }else{
                sendAck(seq, type, ICQ_TCPxMSG_AUTOxREPLY);
                delete m;
            }
        }else{
            plugin p; //Fixme: Local declaration of 'p' hides declaration of the same name in outer scope, see previous declaration at line '609'
            m_socket->readBuffer().unpack((char*)p, sizeof(p));
            unsigned plugin_index;
            for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++){
                if (!memcmp(p, m_client->plugins[plugin_index], sizeof(p)))
                    break;
            }
            ICQBuffer info;
            unsigned short type = 1; //Fixme: Local declaration of 'type' hides declaration of the same name in outer scope, see previous declaration at line '665'
            switch (plugin_index){
            case PLUGIN_FILESERVER:
            case PLUGIN_FOLLOWME:
            case PLUGIN_ICQPHONE:
                type = 2;
            case PLUGIN_PHONEBOOK:
            case PLUGIN_PICTURE:
            case PLUGIN_QUERYxINFO:
            case PLUGIN_QUERYxSTATUS:
                m_client->pluginAnswer(plugin_index, m_data->Uin.toULong(), info);
                startPacket(TCP_ACK, seq);
                m_socket->writeBuffer().pack(type);
                m_socket->writeBuffer() << 0x00000000L
                << (char)1
                << type;
                m_socket->writeBuffer().pack(info.data(0), info.size());
                sendPacket();
                break;
            default:
                log(L_WARN, "Unknwon direct plugin request %u", plugin_index);
                break;
            }
        }
        break;
    case TCP_CANCEL:
	case TCP_ACK: {
        log(L_DEBUG, "Ack %X %X", ackFlags, msgFlags);
        bool itDeleted = false;
        QList<SendDirectMsg>::iterator it;
        for (it = m_queue.begin(); it != m_queue.end(); ++it){
            if (it->seq != seq)
                continue;
            if (it->msg == NULL){
                if (it->type == PLUGIN_AR){
                    Contact *contact = NULL;
                    m_client->findContact(m_client->screen(m_data), NULL, false, contact);
                    m_data->AutoReply.str() = getContacts()->toUnicode(contact,msg_str);
                    m_queue.erase(it);
                    itDeleted = true;
                    break;
                }
                unsigned plugin_index = it->type;
                switch (plugin_index){
                case PLUGIN_FILESERVER:
                case PLUGIN_FOLLOWME:
                case PLUGIN_ICQPHONE:
                    m_socket->readBuffer().incReadPos(-3);
                    break;
                case PLUGIN_QUERYxSTATUS:
                    m_socket->readBuffer().incReadPos(9);
                    break;
                }
                m_client->parsePluginPacket(m_socket->readBuffer(), plugin_index, m_data, m_data->Uin.toULong(), true);
                m_queue.erase(it);
				itDeleted = true;
				break;
            }
            Message *msg = it->msg;
            if (command == TCP_CANCEL){
                EventMessageCancel(msg).process();
                delete msg;
                break;
            }
            MessageId id;
            id.id_l = seq;
            Message *m = m_client->parseMessage(type, m_client->screen(m_data), msg_str, m_socket->readBuffer(), id, 0); //Fixme: Local declaration of 'm' hides declaration of the same name in outer scope, see previous declaration at line '671'
            switch (msg->type()){
            case MessageCloseSecure:
                secureStop(true);
                break;
            case MessageOpenSecure:
                if (msg_str.isEmpty()){
                    msg->setError(I18N_NOOP("Other side does not support the secure connection"));
                }else{
                    secureConnect();
                }
                return;
            case MessageFile:
                if (m == NULL){
                    m_socket->error_state("Ack without message");
                    return;
                }
                if (ackFlags){
                    if (msg_str.isEmpty()){
                        msg->setError(I18N_NOOP("Send message fail"));
                    }else{
                        QString err = getContacts()->toUnicode(m_client->getContact(m_data), msg_str);
                        msg->setError(err);
                    }
                    EventMessageSent(msg).process();
                    m_queue.erase(it);
                    delete msg;
                }else{
                    if (m->type() != MessageICQFile){
                        m_socket->error_state("Bad message type in ack file");
                        return;
                    }
                    ICQFileTransfer *ft = new ICQFileTransfer(static_cast<FileMessage*>(msg), m_data, m_client);
                    EventMessageAcked(msg).process();
                    m_queue.erase(it);
                    m_client->m_processMsg.push_back(msg);
                    ft->connect(static_cast<ICQFileMessage*>(m)->getPort());
                }
                return;
            }
            unsigned flags = msg->getFlags() | MESSAGE_DIRECT;
            if (isSecure())
                flags |= MESSAGE_SECURE;
            if (m_client->snacICBM()->ackMessage(msg, ackFlags, msg_str)){
                if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                    if (msg->type() == MessageGeneric){
                        Message m; //Fixme: Local declaration of 'm' hides declaration of the same name in outer scope, see previous declaration at line '842'
                        m.setContact(msg->contact());
                        m.setClient(msg->client());
                        if (it->type == CAP_RTF){
                            m.setText(m_client->removeImages(msg->getRichText(), true));
                            flags |= MESSAGE_RICHTEXT;
                        }else{
                            m.setText(msg->getPlainText());
                        }
                        m.setFlags(flags);
                        if (msg->getBackground() != msg->getForeground()){
                            m.setForeground(msg->getForeground());
                            m.setBackground(msg->getBackground());
                        }
                        EventSent(&m).process();
                    }else if ((msg->type() != MessageOpenSecure) && (msg->type() != MessageCloseSecure)){
                        msg->setFlags(flags);
                        EventSent(msg).process();
                    }
                }
            }
            EventMessageSent(msg).process();
            m_queue.erase(it);
            delete msg;
            break;
        }
        if (!itDeleted && (m_queue.size() == 0 || it == m_queue.end())){
            list<Message*>::iterator it; //Fixme: Local declaration of 'it' hides declaration of the same name in outer scope, see previous declaration at line '805'
            for (it = m_client->m_acceptMsg.begin(); it != m_client->m_acceptMsg.end(); ++it){
                QString name = m_client->dataName(m_data);
                Message *msg = *it;
                if ((msg->getFlags() & MESSAGE_DIRECT) && (!msg->client().isEmpty()) && (name == msg->client())){
                    bool bFound = false;
                    switch (msg->type()){
                    case MessageICQFile:
                        if (static_cast<ICQFileMessage*>(msg)->getID_L() == seq)
                            bFound = true;
                        break;
                    }
                  if (bFound){
                        m_client->m_acceptMsg.erase(it);
                        EventMessageDeleted(msg).process();
                        delete msg;
                        break;
                    }
                }
				if (it == m_client->m_acceptMsg.end())
					log(L_WARN, "Message for ACK not found??");
            }
        }
        break;
	}
    default:
        m_socket->error_state("Unknown TCP command");
    }
}

bool DirectClient::copyQueue(DirectClient *to)
{
    if (m_state == Logged)
        return false;
    to->m_queue = m_queue;
    m_queue.clear();
    return true;
}

void DirectClient::connect_ready()
{
	log(L_DEBUG, "DirectSocket::connect_ready()");
    if (m_state == None){
        m_state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (m_state == SSLconnect){
        for (QList<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
            SendDirectMsg &sm = *it;
            if ((sm.msg == NULL) || (sm.msg->type() != MessageOpenSecure))
                continue;
            EventMessageSent(sm.msg).process();
            delete sm.msg;
            m_queue.erase(it);
            break;
        }
        m_state = Logged;
        Contact *contact;
        if (m_client->findContact(m_client->screen(m_data), NULL, false, contact)){
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
        return;
    }
    if (m_state == SSLconnect){
        for (QList<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
            SendDirectMsg &sm = *it;
            if ((sm.msg == NULL) || (sm.msg->type() != MessageOpenSecure))
                continue;
            EventMessageSent(sm.msg).process();
            delete sm.msg;
            m_queue.erase(it);
            break;
        }
        m_state = Logged;
        Contact *contact;
        if (m_client->findContact(m_client->screen(m_data), NULL, false, contact)){
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
        return;
    }
    if (m_bIncoming){
        Contact *contact;
        m_data = m_client->findContact(m_client->screen(m_data), NULL, false, contact);
        if ((m_data == NULL) || contact->getIgnore()){
            m_socket->error_state("Connection from unknown user");
            return;
        }
        m_state = WaitInit2;
    }else{
        if (m_version >= 7){
            sendInit2();
            m_state = WaitInit2;
        }else{
            m_state = Logged;
            processMsgQueue();
        }
    }
}

void DirectClient::sendInit2()
{
	log(L_DEBUG, "DirectSocket::sendInit2()");
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer().pack((unsigned short)0x0021);
    m_socket->writeBuffer().pack((char) 0x03);
    m_socket->writeBuffer().pack(0x0000000AL);
    m_socket->writeBuffer().pack(0x00000001L);
    m_socket->writeBuffer().pack(m_bIncoming ? 0x00000001L : 0x00000000L);
    const plugin &p = m_client->plugins[m_channel];
    m_socket->writeBuffer().pack((const char*)p, 8);
    if (m_bIncoming) {
        m_socket->writeBuffer().pack(0x00040001L);
        m_socket->writeBuffer().pack((const char*)p + 8, 8);
    } else {
        m_socket->writeBuffer().pack((const char*)p + 8, 8);
        m_socket->writeBuffer().pack(0x00040001L);
    }
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->ICQDirectPacket, name());
    m_socket->write();
}

bool DirectClient::error_state(const QString &_err, unsigned code)
{
	//Fixme: Dereferencing NULL pointer 'm_data': Lines: 1045, 1046, 1048, 1058, 1059, 1060, 1061, 1062, 1069
    QString err = _err;
    if (!err.isEmpty() && !DirectSocket::error_state(err, code))
        return false;
    if (m_data && (m_port == m_data->Port.toULong())){
        switch (m_state){
        case ConnectIP1:
        case ConnectIP2:
            m_data->bNoDirect.asBool() = true;
            break;
        default:
            break;
        }
    }
    if (err.isEmpty())
        err = I18N_NOOP("Send message fail");
    for (QList<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
        SendDirectMsg &sm = *it;
        if (sm.msg){
            if (!m_client->snacICBM()->sendThruServer(sm.msg, m_data)){
                sm.msg->setError(err);
                EventMessageSent(sm.msg).process();
                delete sm.msg;
            }
        }else{
            m_client->addPluginInfoRequest(m_data->Uin.toULong(), sm.type);
        }
    }
    m_queue.clear();
    return true;
}

void DirectClient::sendAck(unsigned short seq, unsigned short type, unsigned short flags,
                           const char *msg, unsigned short status, Message *m)
{
	log(L_DEBUG, "DirectSocket::sendAck()");
    bool bAccept = true;
    if (status == ICQ_TCPxACK_ACCEPT){
        switch (m_client->getStatus()){
        case STATUS_AWAY:
            status = ICQ_TCPxACK_AWAY;
            break;
        case STATUS_OCCUPIED:
            bAccept = false;
            status = ICQ_TCPxACK_OCCUPIED;
            if (type == ICQ_MSGxAR_OCCUPIED){
                status = ICQ_TCPxACK_OCCUPIEDxCAR;
                bAccept = true;
            }
            break;
        case STATUS_NA:
            status = ICQ_TCPxACK_NA;
            break;
        case STATUS_DND:
            status = ICQ_TCPxACK_DND;
            bAccept = false;
            if (type == ICQ_MSGxAR_DND){
                status = ICQ_TCPxACK_DNDxCAR;
                bAccept = true;
            }
            break;
        default:
            break;
        }
    }
    if (!bAccept && (msg == NULL)){
        ar_request req;
        req.screen  = m_client->screen(m_data);
        req.type    = type;
        req.ack		= 0;
        req.flags   = flags;
        req.id.id_l = seq;
        req.id1     = 0;
        req.id2     = 0;
        req.bDirect = true;
        m_client->arRequests.push_back(req);

        unsigned short req_status = STATUS_ONLINE;
        if (m_data->Status.toULong() & ICQ_STATUS_DND){
            req_status = STATUS_DND;
        }else if (m_data->Status.toULong() & ICQ_STATUS_OCCUPIED){
            req_status = STATUS_OCCUPIED;
        }else if (m_data->Status.toULong() & ICQ_STATUS_NA){
            req_status = STATUS_NA;
        }else if (m_data->Status.toULong() & ICQ_STATUS_AWAY){
            req_status = STATUS_AWAY;
        }else if (m_data->Status.toULong() & ICQ_STATUS_FFC){
            req_status = STATUS_FFC;
        }

        Contact *contact = NULL;
        m_client->findContact(m_client->screen(m_data), NULL, false, contact);
        ARRequest ar;
        ar.contact  = contact;
        ar.param    = &m_client->arRequests.back();
        ar.receiver = m_client;
        ar.status   = req_status;
        EventARRequest(&ar).process();
        return;
    }

    QByteArray message;
    if (msg)
        message = msg;

    startPacket(TCP_ACK, seq);
    m_socket->writeBuffer().pack(type);
    m_socket->writeBuffer().pack(status);
    m_socket->writeBuffer().pack(flags);
    m_socket->writeBuffer() << message;
    bool bExt = false;
    if (m){
        switch (m->type()){
        case MessageICQFile:
            if (static_cast<ICQFileMessage*>(m)->getExtended()){
                bExt = true;
                ICQBuffer buf, msgBuf;
                ICQBuffer b;
                m_client->packExtendedMessage(m, buf, msgBuf, m_data);
                b.pack((unsigned short)buf.size());
                b.pack(buf.data(0), buf.size());
                b.pack32(msgBuf);
                m_socket->writeBuffer().pack(b.data(), b.size());
            }
            break;
        }
    }
    if (!bExt){
        m_socket->writeBuffer()
        << 0x00000000L
        << 0xFFFFFFFFL;
    }
    sendPacket();
}

void DirectClient::startPacket(unsigned short cmd, unsigned short seq)
{
	log(L_DEBUG, "DirectSocket::startPacket()");
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer()
    << (unsigned short)0;	// size
    if (m_version >= 7)
        m_socket->writeBuffer() << (char)0x02;
    if (seq == 0)
        seq = --m_nSequence;
    m_socket->writeBuffer()
    << (unsigned long)0;			// checkSum
    m_socket->writeBuffer().pack(cmd);
    m_socket->writeBuffer()
    << (char) ((m_channel == PLUGIN_NULL) ? 0x0E : 0x12)
    << (char) 0;
    m_socket->writeBuffer().pack(seq);
    m_socket->writeBuffer()
    << (unsigned long)0
    << (unsigned long)0
    << (unsigned long)0;
}

void DirectClient::sendPacket()
{
	log(L_DEBUG, "DirectSocket::sendPacket()");
    unsigned size = m_socket->writeBuffer().size() - m_socket->writeBuffer().packetStartPos() - 2;
    unsigned char *p = (unsigned char*)(m_socket->writeBuffer().data(m_socket->writeBuffer().packetStartPos()));
    p[0] = (unsigned char)(size & 0xFF);
    p[1] = (unsigned char)((size >> 8) & 0xFF);

    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->ICQDirectPacket, name());

    unsigned long hex, key, B1, M1;
    unsigned long i, check;
    unsigned char X1, X2, X3;

    p += 2;
    if (m_version >= 7){
        size--;
        p++;
    }

    // calculate verification data
    M1 = (rand() % ((size < 255 ? size : 255)-10))+10;
    X1 = (unsigned char)(p[M1] ^ 0xFF);
    X2 = (unsigned char)(rand() % 220);
    X3 = (unsigned char)(client_check_data[X2] ^ 0xFF);

    B1 = (p[4] << 24) | (p[6]<<16) | (p[4]<<8) | (p[6]);

    // calculate checkcode
    check = (M1 << 24) | (X1 << 16) | (X2 << 8) | X3;
    check ^= B1;

    *((unsigned long*)p) = check;
    // main XOR key
    key = 0x67657268 * size + check;

    // XORing the actual data
    for(i=4; i<(size+3)/4; i+=4){
        hex = key + client_check_data[i & 0xFF];
        p[i] ^= hex & 0xFF;
        p[i+1] ^= (hex>>8) & 0xFF;
        p[i+2] ^= (hex>>16) & 0xFF;
        p[i+3] ^= (hex>>24) & 0xFF;
    }
    m_socket->write();
}

void DirectClient::acceptMessage(Message *msg)
{
	log(L_DEBUG, "DirectSocket::acceptMessage()");
    unsigned short seq = 0;
    switch (msg->type()){
    case MessageICQFile:
        seq = (unsigned short)(static_cast<ICQFileMessage*>(msg)->getID_L());
        sendAck(seq, static_cast<ICQFileMessage*>(msg)->getExtended() ? ICQ_MSGxEXT : ICQ_MSGxFILE, 0, NULL, ICQ_TCPxACK_ACCEPT, msg);
        break;
    default:
        log(L_WARN, "Unknown type for direct decline");
    }
}

void DirectClient::declineMessage(Message *msg, const QString &reason)
{
    QByteArray r;
    r = getContacts()->fromUnicode(m_client->getContact(m_data), reason);
    unsigned short seq = 0;
    switch (msg->type()){
    case MessageICQFile:
        seq = (unsigned short)(static_cast<ICQFileMessage*>(msg)->getID_L());
        sendAck(seq, static_cast<ICQFileMessage*>(msg)->getExtended() ? ICQ_MSGxEXT : ICQ_MSGxFILE, 0, r, ICQ_TCPxACK_REFUSE, msg);
        break;
    default:
        log(L_WARN, "Unknown type for direct decline");
    }
}

bool DirectClient::sendMessage(Message *msg)
{
    SendDirectMsg sm;
    sm.msg	= msg;
    sm.seq	= 0;
    sm.type	= 0;
    sm.icq_type = 0;
    m_queue.push_back(sm);
    processMsgQueue();
    return true;
}

void packCap(ICQBuffer &b, const capability &c);

void DirectClient::processMsgQueue()
{
    if (m_state != Logged)
        return;
    for (QList<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end();){
        SendDirectMsg &sm = *it;
        if (sm.seq){
            ++it;
            continue;
        }
        if (sm.msg){
            QByteArray message;
            ICQBuffer &mb = m_socket->writeBuffer();
            unsigned short flags = ICQ_TCPxMSG_NORMAL;
            if (sm.msg->getFlags() & MESSAGE_URGENT)
                flags = ICQ_TCPxMSG_URGENT;
            if (sm.msg->getFlags() & MESSAGE_LIST)
                flags = ICQ_TCPxMSG_LIST;
            switch (sm.msg->type()){
            case MessageGeneric:
                startPacket(TCP_START, 0);
                mb.pack((unsigned short)ICQ_MSGxMSG);
                mb.pack(m_client->msgStatus());
                mb.pack(flags);
                if ((sm.msg->getFlags() & MESSAGE_RICHTEXT) &&
                        (m_client->getSendFormat() == 0) &&
                        (m_client->hasCap(m_data, CAP_RTF))){
                    QString text = sm.msg->getRichText();
                    QString part;
                    message = m_client->createRTF(text, part, sm.msg->getForeground(), m_client->getContact(m_data), 0xFFFFFFFF);
                    sm.type = CAP_RTF;
                }else if (m_client->hasCap(m_data, CAP_UTF) &&
                          (m_client->getSendFormat() <= 1) &&
                          ((sm.msg->getFlags() & MESSAGE_SECURE) == 0)){
                    message = ICQClient::addCRLF(sm.msg->getPlainText()).toUtf8();
                    sm.type = CAP_UTF;
                }else{
                    message = getContacts()->fromUnicode(m_client->getContact(m_data), sm.msg->getPlainText());
                    EventSend e(sm.msg, message);
                    e.process();
                    message = e.localeText();
                }
                mb << message;
                if (sm.msg->getBackground() == sm.msg->getForeground()){
                    mb << 0x00000000L << 0xFFFFFF00L;
                }else{
                    mb << (sm.msg->getForeground() << 8) << (sm.msg->getBackground() << 8);
                }
                if (sm.type){
                    mb << 0x26000000L;
                    packCap(mb, ICQClient::capabilities[sm.type]);
                }
                sendPacket();
                sm.seq = m_nSequence;
                sm.icq_type = ICQ_MSGxMSG;
                break;
            case MessageFile:
            case MessageUrl:
            case MessageContacts:
            case MessageOpenSecure:
            case MessageCloseSecure:
                startPacket(TCP_START, 0);
                m_client->packMessage(mb, sm.msg, m_data, sm.icq_type, true);
                sendPacket();
                sm.seq = m_nSequence;
                break;
            default:
                sm.msg->setError(I18N_NOOP("Unknown message type"));
                EventMessageSent(sm.msg).process();
                delete sm.msg;
                m_queue.erase(it);
                it = m_queue.begin();
                continue;
            }
        }else{
            if (sm.type == PLUGIN_AR){
                sm.icq_type = 0;
                unsigned s = m_data->Status.toULong();
                if (s != ICQ_STATUS_OFFLINE){
                    if (s & ICQ_STATUS_DND){
                        sm.icq_type = ICQ_MSGxAR_DND;
                    }else if (s & ICQ_STATUS_OCCUPIED){
                        sm.icq_type = ICQ_MSGxAR_OCCUPIED;
                    }else if (s & ICQ_STATUS_NA){
                        sm.icq_type = ICQ_MSGxAR_NA;
                    }else if (s & ICQ_STATUS_AWAY){
                        sm.icq_type = ICQ_MSGxAR_AWAY;
                    }else if (s & ICQ_STATUS_FFC){
                        sm.icq_type = ICQ_MSGxAR_FFC;
                    }
                }
                if (sm.type == 0){
                    m_queue.erase(it);
                    it = m_queue.begin();
                    continue;
                }
                ICQBuffer &mb = m_socket->writeBuffer();
                startPacket(TCP_START, 0);
                mb.pack(sm.icq_type);
                mb.pack(m_client->msgStatus());
                mb.pack(ICQ_TCPxMSG_AUTOxREPLY);
                mb << (char)1 << (unsigned short)0;
                sendPacket();
                sm.seq = m_nSequence;
            }else{
                ICQBuffer &mb = m_socket->writeBuffer();
                startPacket(TCP_START, 0);
                mb.pack((unsigned short)ICQ_MSGxMSG);
                mb.pack(m_client->msgStatus());
                mb.pack(ICQ_TCPxMSG_AUTOxREPLY);
                mb.pack((unsigned short)1);
                mb.pack((char)0);
                mb.pack((char*)m_client->plugins[sm.type], sizeof(plugin));
                mb.pack((unsigned long)0);
                sendPacket();
                sm.seq = m_nSequence;
            }
        }
        ++it;
    }
}

bool DirectClient::cancelMessage(Message *msg)
{
    for (QList<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
        if (it->msg == msg){
            if (it->seq){
                ICQBuffer &mb = m_socket->writeBuffer();
                startPacket(TCP_CANCEL, it->seq);
                mb.pack((unsigned short)it->icq_type);
                mb.pack((unsigned short)0);
                mb.pack((unsigned short)0);
                QByteArray message;
                mb << message;
                sendPacket();
            }
            m_queue.erase(it);
            return true;
        }
    }
    return false;
}

void DirectClient::addPluginInfoRequest(unsigned plugin_index)
{
    QList<SendDirectMsg>::ConstIterator it;
    for (it = m_queue.constBegin(); it != m_queue.constEnd(); ++it){
        const SendDirectMsg &sm = *it;
        if (sm.msg)
            continue;
        if (sm.type == plugin_index)
            return;
    }
    SendDirectMsg sm;
    sm.msg = NULL;
    sm.seq = 0;
    sm.type = plugin_index;
    sm.icq_type = 0;
    m_queue.push_back(sm);
    processMsgQueue();
}

class ICQ_SSLClient : public SSLClient
{
public:
    ICQ_SSLClient(Socket *s) : SSLClient(s) {}
};


void DirectClient::secureConnect()
{
    if (m_ssl != NULL) return;
    m_ssl = new ICQ_SSLClient(m_socket->socket());
    m_socket->setSocket(m_ssl);
    m_state = SSLconnect;
    m_ssl->startEncryption();
}

void DirectClient::secureListen()
{
    if (m_ssl != NULL)
        return;
    m_ssl = new ICQ_SSLClient(m_socket->socket());
    m_socket->setSocket(m_ssl);
    m_state = SSLconnect;
//    m_ssl->accept();
}

void DirectClient::secureStop(bool bShutdown)
{
    if (m_ssl){
        if (bShutdown){
            m_ssl->close();
        }
        m_socket->setSocket(m_ssl->socket(), false);
        m_ssl->setSocket(NULL);
        delete m_ssl;
        m_ssl = NULL;
        Contact *contact;
        if (m_client->findContact(m_client->screen(m_data), NULL, false, contact)){
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
    }
}

QString DirectClient::name()
{
    if (m_data == NULL)
        return QString::null;
    m_name = QString::null;
    switch (m_channel){
    case PLUGIN_NULL:
        break;
    case PLUGIN_INFOxMANAGER:
        m_name = "Info.";
        break;
    case PLUGIN_STATUSxMANAGER:
        m_name = "Status.";
        break;
    default:
        m_name = "Unknown.";
    }
    m_name += QString::number(m_data->Uin.toULong());
    m_name += '.';
    m_name += QString::number((unsigned long)this);
    return m_name;
}

ICQFileTransfer::ICQFileTransfer(FileMessage *msg, ICQUserData *data, ICQClient *client)
        : FileTransfer(msg), DirectSocket(data, client)
{
    m_state = None;
    FileMessage::Iterator it(*msg);
    m_nFiles     = it.count();
    m_totalSize = msg->getSize();
}

ICQFileTransfer::~ICQFileTransfer()
{
}

void ICQFileTransfer::connect(unsigned short port)
{
	log(L_DEBUG, "ICQFileTransfer::connect()");
    m_port = port;
    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();
    DirectSocket::connect();
}

void ICQFileTransfer::listen()
{
    FileTransfer::m_state = FileTransfer::Listen;
    if (m_notify)
        m_notify->process();
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
}

void ICQFileTransfer::processPacket()
{
	log(L_DEBUG, "ICQFileTransfer::processPacket()");
    char cmd;
    m_socket->readBuffer() >> cmd;
    if (cmd != FT_DATA){
        ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
        EventLog::log_packet(m_socket->readBuffer(), false, plugin->ICQDirectPacket, QByteArray("File transfer"));
    }
    if (cmd == FT_SPEED){
        char speed;
        m_socket->readBuffer().unpack(speed);
        m_speed = speed;
        return;
    }
    switch (m_state){
    case InitSend:
        switch (cmd){
        case FT_INIT_ACK:
            sendFileInfo();
            break;
        case FT_START:{
                unsigned long pos, empty, speed, curFile;
                m_socket->readBuffer().unpack(pos);
                m_socket->readBuffer().unpack(empty);
                m_socket->readBuffer().unpack(speed);
                m_socket->readBuffer().unpack(curFile);
                curFile--;
                log(L_DEBUG, "Start send at %lu %lu", pos, curFile);
                FileMessage::Iterator it(*m_msg);
                if (curFile >= it.count()){
                    m_socket->error_state("Bad file index");
                    return;
                }
                while (curFile != m_nFile){
                    if (!openFile()){
                        m_socket->error_state("Can't open file");
                        return;
                    }
                }
                if (m_file && !m_file->seek(pos)){
                    m_socket->error_state("Can't set transfer position");
                    return;
                }
                m_totalBytes += pos;
                m_bytes		  = pos;
                m_state       = Send;
                FileTransfer::m_state = FileTransfer::Write;
                if (m_notify){
                    m_notify->process();
                    m_notify->transfer(true);
                }
                write_ready();
                break;
            }
        default:
            log(L_WARN, "Bad init client command %X", cmd);
            m_socket->error_state("Bad packet");
        }
        break;
    case WaitInit:{
            if (cmd != FT_INIT){
                m_socket->error_state("No init command");
                return;
            }
            unsigned long n;
            m_socket->readBuffer().unpack(n);
            m_socket->readBuffer().unpack(n);
            m_nFiles = n;
            m_socket->readBuffer().unpack(n);
            m_totalSize = n;
            m_msg->setSize(m_totalSize);
            m_state = InitReceive;
            setSpeed(m_speed);
            startPacket(FT_INIT_ACK);
            m_socket->writeBuffer().pack((unsigned long)m_speed);
            QString uin = m_client->screen(&m_client->data.owner);
            m_socket->writeBuffer() << uin;
            sendPacket();
            FileTransfer::m_state = Negotiation;
            if (m_notify)
                m_notify->process();
        }
        break;
    case InitReceive:{
            initReceive(cmd);
            break;
        }
    case Receive:{
            if (m_bytes < m_fileSize){
                if (cmd != FT_DATA){
                    m_socket->error_state("Bad data command");
                    return;
                }
                unsigned short size = (unsigned short)(m_socket->readBuffer().size() - m_socket->readBuffer().readPos());
                m_bytes      += size;
                m_totalBytes += size;
                m_transferBytes += size;
                if (size){
                    if (m_file == NULL){
                        m_socket->error_state("Write without file");
                        return;
                    }
                    if (m_file->write(m_socket->readBuffer().data(m_socket->readBuffer().readPos()), size) != size){
                        m_socket->error_state("Error write file");
                        return;
                    }
                }
            }
            if (m_bytes >= m_fileSize){
                if (m_nFile + 1 >= m_nFiles){
                    log(L_DEBUG, "File transfer OK");
                    FileTransfer::m_state = FileTransfer::Done;
                    if (m_notify)
                        m_notify->process();
                    m_socket->error_state(QString::null);
                    return;
                }
                m_state = InitReceive;
            }
            if (m_notify)
                m_notify->process();
            if (cmd != FT_DATA)
                initReceive(cmd);
            break;
        }

    default:
        log(L_WARN, "Bad state in process packet %u", m_state);
    }
}

void ICQFileTransfer::initReceive(char cmd)
{
    if (cmd != FT_FILEINFO){
        m_socket->error_state("Bad command in init receive");
        return;
    }
    string stdStrFileName;
    char isDir;
    m_socket->readBuffer() >> isDir >> stdStrFileName;
	QByteArray qcfilename(stdStrFileName.c_str());
	QString fName = getContacts()->toUnicode(m_client->getContact(m_data), qcfilename);

	string stdStrDir;
    unsigned long n;
    m_socket->readBuffer() >> stdStrDir;
	QByteArray dir(stdStrDir.c_str());
    m_socket->readBuffer().unpack(n);
    if (m_notify)
        m_notify->transfer(false);
    if (!dir.isEmpty())
        fName = getContacts()->toUnicode(m_client->getContact(m_data), dir) + '/' + fName;
    if (isDir)
        fName += '/';
    m_state = Wait;
    FileTransfer::m_state = FileTransfer::Read;
    if (m_notify)
        m_notify->createFile(fName, n, true);
}

bool ICQFileTransfer::error(const QString &err)
{
    return error_state(err, 0);
}

bool ICQFileTransfer::accept(Socket *s, unsigned long)
{
    log(L_DEBUG, "Accept file transfer");
    if (m_state == WaitReverse){
        acceptReverse(s);
    }else{
        m_socket->setSocket(s);
        m_bIncoming = true;
        DirectSocket::m_state = DirectSocket::WaitInit;
        init();
    }
    return true;
}

void ICQFileTransfer::bind_ready(unsigned short port)
{
    m_localPort = port;
    if (m_state == WaitReverse){
        m_client->snacICBM()->requestReverseConnection(m_client->screen(m_data), this);
        return;
    }
    m_state = Listen;
    static_cast<ICQFileMessage*>(m_msg)->setPort(port);
    m_client->snacICBM()->accept(m_msg, m_data);
}

void ICQFileTransfer::login_timeout()
{
    if (ICQClient::hasCap(m_data, CAP_DIRECT)){
        DirectSocket::m_state = DirectSocket::WaitReverse;
        m_state = WaitReverse;
        bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
        return;
    }
    DirectSocket::login_timeout();
}

bool ICQFileTransfer::error_state(const QString &err, unsigned code)
{
    if (DirectSocket::m_state == DirectSocket::ConnectFail){
        if (ICQClient::hasCap(m_data, CAP_DIRECT)){
            login_timeout();
            return false;
        }
    }
    if (!DirectSocket::error_state(err, code))
        return false;
    if (FileTransfer::m_state != FileTransfer::Done){
        m_state = None;
        FileTransfer::m_state = FileTransfer::Error;
        m_msg->setError(err);
    }
    m_msg->m_transfer = NULL;
    m_msg->setFlags(m_msg->getFlags() & ~MESSAGE_TEMP);
    EventMessageSent(m_msg).process();
    return true;
}

void ICQFileTransfer::connect_ready()
{
	log(L_DEBUG, "ICQFileTransfer::connect_ready()");
    if (m_state == None){
        m_state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (m_state == WaitReverse){
        m_bIncoming = false;
        m_state = WaitReverseLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (m_state == WaitReverseLogin)
        m_bIncoming = true;
    m_file = 0;
    FileTransfer::m_state = FileTransfer::Negotiation;
    if (m_notify)
        m_notify->process();
    if (m_bIncoming){
        m_state = WaitInit;
    }else{
        m_state = InitSend;
        startPacket(FT_SPEED);
        m_socket->writeBuffer().pack((unsigned long)m_speed);
        sendPacket(true);
        sendInit();
    }
}

void ICQFileTransfer::sendInit()
{
    startPacket(FT_INIT);
    m_socket->writeBuffer().pack((unsigned long)0);
    m_socket->writeBuffer().pack((unsigned long)m_nFiles);			// nFiles
    m_socket->writeBuffer().pack((unsigned long)m_totalSize);		// Total size
    m_socket->writeBuffer().pack((unsigned long)m_speed);			// speed
	m_socket->writeBuffer() << QString::number(m_client->data.owner.Uin.toULong()).data();
    sendPacket();
    if ((m_nFiles == 0) || (m_totalSize == 0))
        m_socket->error_state(I18N_NOOP("No files for transfer"));
}

void ICQFileTransfer::startPacket(char cmd)
{
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer() << (unsigned short)0;
    m_socket->writeBuffer() << cmd;
}

void ICQFileTransfer::sendPacket(bool dump)
{
    unsigned long start_pos = m_socket->writeBuffer().packetStartPos();
    unsigned size = m_socket->writeBuffer().size() - start_pos - 2;
    unsigned char *p = (unsigned char*)(m_socket->writeBuffer().data(start_pos));
    p[0] = (unsigned char)(size & 0xFF);
    p[1] = (unsigned char)((size >> 8) & 0xFF);
    if (dump){
        ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
        QString name = "FileTranfer";
        if (m_data){
            name += '.';
            name += QString::number(m_data->Uin.toULong());
        }
        EventLog::log_packet(m_socket->writeBuffer(), true, plugin->ICQDirectPacket, name);
    }
    m_socket->write();
}

void ICQFileTransfer::setSpeed(unsigned speed)
{
    FileTransfer::setSpeed(speed);
    switch (m_state){
    case InitSend:
    case InitReceive:
    case Send:
    case Receive:
    case Wait:
        startPacket(FT_SPEED);
        m_socket->writeBuffer().pack((unsigned long)m_speed);
        sendPacket(true);
        break;
    default:
        break;
    }
}

void ICQFileTransfer::write_ready()
{
    if (m_state != Send){
        DirectSocket::write_ready();
        return;
    }
    if (m_transfer){
        m_transferBytes += m_transfer;
        m_transfer = 0;
        if (m_notify)
            m_notify->process();
    }
    if (m_bytes >= m_fileSize){
        m_state = None;
        m_state = InitSend;
        sendFileInfo();
        if (m_notify)
            m_notify->process();
        return;
    }
    QDateTime now(QDateTime::currentDateTime());
    if (now != m_sendTime){
        m_sendTime = now;
        m_sendSize = 0;
    }
    if (m_sendSize > (m_speed << 18)){
        m_socket->pause(1);
        return;
    }
    unsigned long tail = m_fileSize - m_bytes;
    if (tail > 2048) tail = 2048;
    startPacket(FT_DATA);
    char buf[2048];
    int readn = m_file->read(buf, tail);
    if (readn <= 0){
        m_socket->error_state("Read file error");
        return;
    }
    m_transfer   = readn;
    m_bytes      += readn;
    m_totalBytes += readn;
    m_sendSize   += readn;
    m_socket->writeBuffer().pack(buf, readn);
    sendPacket(false);
}

void ICQFileTransfer::sendFileInfo()
{
    if (!openFile()){
        if (FileTransfer::m_state == FileTransfer::Done)
            m_socket->error_state(QString::null);
        if (m_notify)
            m_notify->transfer(false);
        return;
    }
    if (m_notify)
        m_notify->transfer(false);
    startPacket(FT_FILEINFO);
    m_socket->writeBuffer().pack((char)(isDirectory() ? 1 : 0));
    QString fn  = filename();
    QString dir;
    int n = fn.lastIndexOf('/');
    if (n >= 0){
        dir = fn.left(n);
        dir = dir.replace('/', '\\');
        fn  = fn.mid(n);
    }
	QByteArray s1 = getContacts()->fromUnicode(m_client->getContact(m_data), fn);
    QByteArray s2="";
    if (!dir.isEmpty())
        s2 = getContacts()->fromUnicode(m_client->getContact(m_data), dir);
	string ssc1 = s1.data();
	string ssc2 = s2.data();
//#ifdef __OS2__  // to make it compileable under OS/2 (gcc 3.3.5)
    m_socket->writeBuffer() << ssc1.c_str() << ssc2.c_str();
//#else
//    m_socket->writeBuffer() << QString(ssc1) << QString(ssc2);
//#endif
    m_socket->writeBuffer().pack((unsigned long)m_fileSize);
    m_socket->writeBuffer().pack((unsigned long)0);
    m_socket->writeBuffer().pack((unsigned long)m_speed);
    sendPacket();
    if (m_notify)
        m_notify->process();
}

void ICQFileTransfer::setSocket(ICQClientSocket *socket)
{
    if (m_socket)
        delete m_socket;
    m_socket = socket;
    m_socket->setNotify(this);
    m_state  = WaitInit;
    processPacket();
    if ((m_msg->getFlags() & MESSAGE_RECEIVED) == 0){
        m_state = InitSend;
        sendInit();
    }
    m_socket->readBuffer().init(2);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
    DirectSocket::m_state = DirectSocket::Logged;
}

void ICQFileTransfer::startReceive(unsigned pos)
{
    if (m_state != Wait){
        log(L_WARN, "Start receive in bad state");
        return;
    }
    startPacket(FT_START);
    if (pos > m_fileSize)
        pos = m_fileSize;
    m_bytes = pos;
    m_totalBytes += pos;
    m_socket->writeBuffer().pack((unsigned long)pos);
    m_socket->writeBuffer().pack((unsigned long)0);
    m_socket->writeBuffer().pack((unsigned long)m_speed);
    m_socket->writeBuffer().pack((unsigned long)(m_nFile + 1));
    sendPacket();
    m_state = Receive;
    if (m_notify)
        m_notify->transfer(true);
}

///////////////////////////////////////////
//

AIMFileTransfer::AIMFileTransfer(FileMessage *msg, ICQUserData *data, ICQClient *client)
        : FileTransfer(msg)
{
    m_msg		= msg;
    m_client	= client;
	m_data = data;
	m_ip = 0;
	m_proxy = false;
	m_proxyActive = true;
	m_packetLength = 1000;
	m_socket = new ICQClientSocket(this);
	client->m_filetransfers.push_back(this);
	log(L_DEBUG, "AIMFileTransfer::AIMFileTransfer: %p", this);
}

AIMFileTransfer::~AIMFileTransfer()
{
	if(m_client)
	{
		for(std::list<AIMFileTransfer*>::iterator it = m_client->m_filetransfers.begin(); it != m_client->m_filetransfers.end(); ++it)
		{
			if((*it) == this) // FIXME make comparison by cookie
			{
				m_client->m_filetransfers.erase(it);
				break;
			}
		}
	}
	delete m_socket;
	log(L_DEBUG, "AIMFileTransfer::~AIMFileTransfer");
}

void AIMFileTransfer::requestFT()
{
	log(L_DEBUG, "AIMFileTransfer::requestFT m_stage = %d", m_stage);
        log(L_DEBUG, "Description: %s", qPrintable(m_msg->getDescription()));
        log(L_DEBUG, "filename: %s", qPrintable(filename()));
	ICQBuffer b;
	bool bWide = false;
	for(int i = 0; i < (int)(filename().length()); i++)
	{
		if (filename()[i].unicode() > 0x7F)
		{
			bWide = true;
			break;
		}
	}
	QString charset = bWide ? "utf-8" : "us-ascii";

	unsigned short this_port = (unsigned short)(m_proxy ? m_cookie2 : m_port);

    b << (unsigned short)0;
    b << m_cookie.id_l << m_cookie.id_h;
    b.pack((char*)m_client->capabilities[CAP_AIM_SENDFILE], sizeof(capability));
    b.tlv(0x0A, (unsigned short)m_stage);
	b.tlv(0x0F);
	b.tlv(0x03, (unsigned long)htonl(get_ip(m_client->data.owner.RealIP)));
	b.tlv(0x04, (unsigned long)htonl(get_ip(m_client->data.owner.IP)));
	b.tlv(0x05, this_port);

	this_port = ~(htons(m_port));
	b.tlv(0x17, this_port);

	unsigned long this_ip = m_ip;
	if(m_ip == 0)
		this_ip = htonl(get_ip(m_client->data.owner.RealIP));

	b.tlv(0x02, this_ip);
	this_ip = ~this_ip;
	b.tlv(0x16, this_ip);

	if(m_proxy)
	{
		b.tlv(0x10);
	}
	ICQBuffer buf;
	if(m_stage == 1)
	{
		if(files() == 1)
		{
			buf << ((unsigned short)0x0001) << ((unsigned short)0x0001);
		}
		else
		{
			buf << ((unsigned short)0x0002) << ((unsigned short)files());
		}
		buf << ((unsigned long)totalSize());

		if(!m_proxy && (files() == 1))
		{
			if(bWide)
			{
				QByteArray decodedfname = filename().toUtf8();
				buf.pack(decodedfname.data(), decodedfname.length() + 1);
			}
			else
			{
                                // FIXME: this does not compile and is wrong !
                                //buf.pack(filename(), filename().length() + 1);
                                // maybe this was meant?
                                buf.pack((const char*)filename().utf16(), filename().length() * 2 + 2);
			}
		}
		else
		{
			buf.pack((unsigned char)0);
		}
	}
	b.tlv(0x2711, buf);
	if(m_stage == 1)
	{
        b.tlv(0x2712, charset.toAscii(), charset.length());
	}
    m_client->snacICBM()->sendThroughServer(m_client->screen(m_data), 2, b, m_cookie, false, true);
}

void AIMFileTransfer::accept()
{
}

unsigned short AIMFileTransfer::remotePort()
{
    return m_port;
}

void AIMFileTransfer::processPacket()
{
	log(L_DEBUG, "AIMFileTransfer::processPacket");
}

void AIMFileTransfer::setICBMCookie(MessageId const& cookie)
{
	m_cookie = cookie;
}

void AIMFileTransfer::setICBMCookie2(unsigned short cookie2)
{
	m_cookie2 = cookie2;
}



bool AIMFileTransfer::readOFT(OftData* oft)
{
	log(L_DEBUG, "reading OFT");
	m_socket->readBuffer().unpack(oft->magic);

	if(oft->magic != OFT_magic)
	{
                log(L_DEBUG, "Invalid magic for OFT in stream %08x", (unsigned int)oft->magic);
		return false;
	}

	m_socket->readBuffer().unpack(oft->unknown);
	m_socket->readBuffer().unpack(oft->type);
	m_socket->readBuffer().unpack(oft->cookie, 8);
	m_socket->readBuffer().unpack(oft->encrypt);
	m_socket->readBuffer().unpack(oft->compress);

	m_socket->readBuffer().unpack(oft->total_files);
	oft->total_files = ntohs(oft->total_files);
	m_socket->readBuffer().unpack(oft->files_left);
	oft->files_left = ntohs(oft->files_left);
	m_socket->readBuffer().unpack(oft->total_parts);
	oft->total_parts = ntohs(oft->total_parts);
	m_socket->readBuffer().unpack(oft->parts_left);
	oft->parts_left = ntohs(oft->parts_left);
	m_socket->readBuffer().unpack(oft->total_size);
	oft->total_size = ntohl(oft->total_size);
	m_socket->readBuffer().unpack(oft->size);
	oft->size = ntohl(oft->size);

	m_socket->readBuffer().unpack(oft->mod_time);
	m_socket->readBuffer().unpack(oft->checksum);
	m_socket->readBuffer().unpack(oft->rfrcsum);
	m_socket->readBuffer().unpack(oft->rfsize);
	m_socket->readBuffer().unpack(oft->cretime);
	m_socket->readBuffer().unpack(oft->rfcsum);
	m_socket->readBuffer().unpack(oft->nrecvd);
	m_socket->readBuffer().unpack(oft->recvcsum);
	m_socket->readBuffer().unpack(oft->idstring, 32);
	m_socket->readBuffer().unpack(oft->flags);
	m_socket->readBuffer().unpack(oft->lnameoffset);
	m_socket->readBuffer().unpack(oft->lsizeoffset);
	m_socket->readBuffer().unpack(oft->dummy, 69);
	m_socket->readBuffer().unpack(oft->macfileinfo, 16);
	m_socket->readBuffer().unpack(oft->nencode);
	m_socket->readBuffer().unpack(oft->nlanguage);

	m_socket->readBuffer().unpack(oft->name, m_socket->readBuffer().size() - m_socket->readBuffer().readPos());

	if(oft->nencode == 0x0200) // Hack
	{
		for(unsigned int i = 0; i < (unsigned)oft->name.size() ; i++)
		{
			unsigned char tmp = oft->name.data()[i + 1];
			oft->name.data()[i + 1] = oft->name.data()[i];
			oft->name.data()[i] = tmp;
		}
	}
	oft->name.detach();
	return true;
}

bool AIMFileTransfer::writeOFT(OftData* oft)
{
	log(L_DEBUG, "writing OFT");
	m_socket->writeBuffer().pack(oft->magic);
	m_socket->writeBuffer().pack(oft->unknown);
	m_socket->writeBuffer().pack(oft->type);
	m_socket->writeBuffer().pack(oft->cookie, 8);
	m_socket->writeBuffer().pack(oft->encrypt);
	m_socket->writeBuffer().pack(oft->compress);
	m_socket->writeBuffer().pack((unsigned short)htons(oft->total_files));
	m_socket->writeBuffer().pack((unsigned short)htons(oft->files_left));
	m_socket->writeBuffer().pack((unsigned short)htons(oft->total_parts));
	m_socket->writeBuffer().pack((unsigned short)htons(oft->parts_left));
	m_socket->writeBuffer().pack((unsigned long)htonl(oft->total_size));
	m_socket->writeBuffer().pack((unsigned long)htonl(oft->size));
	m_socket->writeBuffer().pack(oft->mod_time);
	m_socket->writeBuffer().pack(oft->checksum);
	m_socket->writeBuffer().pack(oft->rfrcsum);
	m_socket->writeBuffer().pack(oft->rfsize);
	m_socket->writeBuffer().pack(oft->cretime);
	m_socket->writeBuffer().pack(oft->rfcsum);
	m_socket->writeBuffer().pack(oft->nrecvd);
	m_socket->writeBuffer().pack(oft->recvcsum);
	m_socket->writeBuffer().pack(oft->idstring, 32);
	m_socket->writeBuffer().pack(oft->flags);
	m_socket->writeBuffer().pack(oft->lnameoffset);
	m_socket->writeBuffer().pack(oft->lsizeoffset);
	m_socket->writeBuffer().pack(oft->dummy, 69);
	m_socket->writeBuffer().pack(oft->macfileinfo, 16);
	m_socket->writeBuffer().pack(oft->nencode);
	m_socket->writeBuffer().pack(oft->nlanguage);
	m_socket->writeBuffer().pack(oft->name.data(), oft->name.size() - 1);
	if(oft->name.size() - 1 <= 0x40)
	{
		for(unsigned int i = 0; i < 0x40 - (unsigned)oft->name.size() + 1; i++)
		{
			m_socket->writeBuffer().pack((unsigned char)0);
		}
	}
	else
	{
		m_socket->writeBuffer().pack((unsigned char)0);
	}
	return true;
}

unsigned long AIMFileTransfer::calculateChecksum()
{
	if(!m_file)
	{
		log(L_WARN, "No file for checksum calculation");
		return 0;
	}
	unsigned long checksum = 0xFFFF;
	//bool high = true;
	QByteArray chunk(1024, '\0');
	ulong bytesread = 0;
	long streamposition = 0;
	m_file->reset();
	do
    {
		bytesread = m_file->read(chunk.data(), chunk.size());
        checksum = checksumChunk(&chunk, (unsigned int)bytesread, checksum);
        streamposition += bytesread;

    }
	while (bytesread == (unsigned)chunk.size());

	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));
	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));

    log(L_WARN, "Calculating checksum: %s (%08x)", qPrintable(m_file->fileName()), (unsigned int)checksum);
	return checksum;
}

unsigned long AIMFileTransfer::checksumChunk(QByteArray* filechunk, unsigned int chunklength, unsigned int start)
{
	uint32_t checksum = start, prevchecksum;
	bool high = false;
	for (unsigned long i = 0; i < (unsigned long)filechunk->size() && i < (unsigned long)chunklength; i++)
    {
		prevchecksum = checksum;

		if(high)
		{
			checksum -= (((uint32_t)(filechunk->at(i)) & 0xff) << 8);
		}
		else
		{
			checksum -= ((uint32_t)(filechunk->at(i)) & 0xff);
		}
		high = !high;

		if(checksum > prevchecksum)
			checksum--;
	}

	return checksum;
}

void AIMFileTransfer::connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2)
{
	log(L_DEBUG, "Proxy connection, host = %s, port = %d", host.toLatin1().data(), port);
	m_proxy = true;
    m_port = port;
	m_cookie2 = cookie2;

    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();

	m_socket->connect(host, port, NULL);
    m_socket->writeBuffer().init(0);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
	m_socket->setRaw(true);

}

void AIMFileTransfer::negotiateWithProxy()
{
	if(m_proxyActive) // We initiated proxy transfer
	{
		unsigned char uin_length = m_client->getScreen().length();
		unsigned short packet_length = 0x26 + 1 + uin_length;
		m_socket->writeBuffer() << packet_length;
		m_socket->writeBuffer() << Chunk_status;
		// Status chunk is made of 6 bytes, first 2 are actually status and other 4 are zeroes
		m_socket->writeBuffer() << (unsigned short)0x0002 << (unsigned long) 0x00000000; // 0x0002 means FT request to send
		// Then, UIN chunk goes. First byte is length.
		m_socket->writeBuffer() << Chunk_uin << uin_length;
        m_socket->writeBuffer().pack(m_client->getScreen().toAscii(), uin_length);
		// Next chunk is cookie chunk
		m_socket->writeBuffer() << m_cookie.id_l << m_cookie.id_h;
		// And the last one is magic caps chunk
		m_socket->writeBuffer() << Chunk_cap << (unsigned short)0x0010;
		m_socket->writeBuffer().pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);
        //EventLog::log_packet(m_socket->writeBuffer(), true, ICQPlugin::icq_plugin->AIMDirectPacket); //commented out due to problems with netmon while transfer
		m_socket->write();
	}
	else // Remote host initiated proxy transfer
	{
		unsigned char uin_length = m_client->getScreen().length();
		unsigned short packet_length = 0x28 + 1 + uin_length;
		m_socket->writeBuffer() << packet_length;
		m_socket->writeBuffer() << Chunk_status;
		// Status chunk is made of 6 bytes, first 2 are actually status and other 4 are zeroes
		m_socket->writeBuffer() << (unsigned short)0x0004 << (unsigned long) 0x00000000; // 0x0004 means FT request to receive
		// Then, UIN chunk goes. First byte is length.
		m_socket->writeBuffer() << Chunk_uin << uin_length;
        m_socket->writeBuffer().pack(m_client->getScreen().toAscii(), uin_length);
		// Next chunk is cookie chunk
		m_socket->writeBuffer() << (unsigned short)m_cookie2 << m_cookie.id_l << m_cookie.id_h;
		// And the last one is magic caps chunk
		m_socket->writeBuffer() << Chunk_cap << (unsigned short)0x0010;
		m_socket->writeBuffer().pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);
        //EventLog::log_packet(m_socket->writeBuffer(), true, ICQPlugin::icq_plugin->AIMDirectPacket); //commented out due to problems with netmon while transfer
		m_socket->write();
	}
}

void AIMFileTransfer::resolve_ready(QHostAddress ip)
{
        m_ip = ip.toIPv4Address();
}

bool AIMFileTransfer::error_state(const QString &err, unsigned)
{
    m_msg->setError(err);
    EventMessageSent(m_msg).process();
    return true;
}


void AIMFileTransfer::startReceive(unsigned)
{
	log(L_DEBUG, "AIMFileTransfer::startReceive");
}

void AIMFileTransfer::bind_ready(unsigned short port)
{
	log(L_DEBUG, "AIMFileTransfer::bind_ready(%d)", port);
    for (list<Message*>::iterator it = m_client->m_processMsg.begin(); it != m_client->m_processMsg.end(); ++it){
        if ((*it) == m_msg){
            m_client->m_processMsg.erase(it);
            break;
        }
    }
    m_port = port;
}

bool AIMFileTransfer::error(const QString &err)
{
    error_state(err, 0);
    return true;
}

void AIMFileTransfer::connect(unsigned long ip, unsigned short port)
{
	log(L_DEBUG, "AIMFileTransfer::connect");

	m_socket->connect(ip, port, NULL);
    m_socket->writeBuffer().init(0);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
	m_socket->setRaw(true);
}


AIMIncomingFileTransfer::AIMIncomingFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client)
    : QObject(),
    AIMFileTransfer(msg, data, client),
    m_connectTimer(this)
{
	QObject::connect(&m_connectTimer, SIGNAL(timeout()), this, SLOT(connect_timeout()));
	m_totalBytes = 0;
}

AIMIncomingFileTransfer::~AIMIncomingFileTransfer()
{
	//m_client->deleteFileMessage(m_cookie);
}

bool AIMIncomingFileTransfer::error_state(const QString &err, unsigned code)
{
    log(L_DEBUG, "AIMFileTransfer::error_state: %s, %d", qPrintable(err), code);
	if(m_stage == 1)
	{
		// Well, this is hack, but, i think, it is not so ugly as it seems :)
		connect_timeout();
		return false;
	}
	else if(m_stage == 2)
	{
		// It can occur, so skip this stage, and wait for proxy FT request from the other peer
		return false;
	}
	return true;
}

bool AIMIncomingFileTransfer::accept(SIM::Socket* /*s*/, unsigned long /*ip*/)
{
	// TODO
	return false;
}

void AIMIncomingFileTransfer::accept()
{
	log(L_DEBUG, "AIMIncomingFileTransfer::accept");
    m_state = Connecting;
    m_connectTimer.setSingleShot( true );
    m_connectTimer.start( DIRECT_TIMEOUT * 1000 );
	FileTransfer::m_state = FileTransfer::Connect;
	if(m_notify)
		m_notify->process();

	unsigned long ip = get_ip(m_data->RealIP);
	if(!ip)
	{
		ip = get_ip(m_data->IP);
	}
	m_socket->connect(ip, m_port, NULL);
}

void AIMIncomingFileTransfer::connect_timeout()
{
	if(m_state == Connecting)
	{
		log(L_DEBUG, "Connecting timeout, trying reverse connection");
		FileMessage* msg = static_cast<FileMessage*>(m_msg);
		QString filename = msg->getDescription();
		m_stage++;
		requestFT();
		m_state = ProxyConnection;
		// TODO Here we should really open the socket and wait for incoming connection,
		// but we'll cheat for now - skip this step and wait for request for proxy transfer
	}
}

void AIMIncomingFileTransfer::connect_ready()
{
	log(L_DEBUG, "AIMIncomingFileTransfer::connect_ready()");
	m_connectTimer.stop();
	m_socket->writeBuffer().init(0);
	m_socket->readBuffer().init(0);
	m_socket->writeBuffer().packetStart();
	m_socket->readBuffer().packetStart();

	if(!m_proxy)
	{
		m_state = OFTNegotiation;

		ICQBuffer buf;
		buf << 0x0002 << m_cookie.id_l << m_cookie.id_h;
		buf.pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);
		m_client->snacICBM()->sendThroughServer(m_client->screen(m_data), 0x0002, buf, m_cookie, false, true);

		FileTransfer::m_state = FileTransfer::Negotiation;
		if(m_notify)
			m_notify->process();
	}
	else
	{
		m_state = ProxyNegotiation;
		negotiateWithProxy();
	}
	m_socket->setRaw(true);
}
void AIMIncomingFileTransfer::packet_ready()
{
	ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
	EventLog::log_packet(m_socket->readBuffer(), false, plugin->AIMDirectPacket);
	long size = (unsigned long)(m_socket->readBuffer().size() - m_socket->readBuffer().readPos());
	if(size <= 0)
	{
		log(L_DEBUG, "size <= 0");
		return;
	}
	switch(m_state)
	{
		case ProxyNegotiation:
			{
				unsigned short packet_length, chunk_id, status;
				m_socket->readBuffer() >> packet_length;
				m_socket->readBuffer() >> chunk_id;
				log(L_DEBUG, "[Input]Proxy packet, length = %d, chunk_id = %04x",packet_length, chunk_id);
				if(chunk_id == Chunk_status)
				{
					m_socket->readBuffer() >> status;
					log(L_DEBUG, "status = %04x", status);
					// TODO Handle errors
					if(status == 0x0003)
					{
						m_socket->readBuffer().incReadPos(6);
						m_socket->readBuffer() >> m_cookie2;
						m_socket->readBuffer() >> m_ip;
						FileMessage* msg = static_cast<FileMessage*>(m_msg);
						QString filename = msg->getDescription();
						m_stage++;
						requestFT();
					}
					if(status == 0x0005) // Everything is allright
					{
						log(L_DEBUG, "Connection accepted");
						// Read the rest of a packet:
						m_socket->readBuffer().incReadPos(packet_length - 4);
						ICQBuffer buf;
						buf << (unsigned short) 0x0002 << m_cookie.id_l << m_cookie.id_h;
						buf.pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);

						m_client->snacICBM()->sendThroughServer(m_client->screen(m_data), 0x0002, buf, m_cookie, false, true);
						FileTransfer::m_state = FileTransfer::Negotiation;
						if(m_notify)
							m_notify->process();
						m_state = OFTNegotiation;
					}
				}
				return;
			}
			break;
		case OFTNegotiation:
			if(readOFT(&m_oft))
			{
				if(m_oft.type == OFT_fileInfo)
					ackOFT();
				FileTransfer::m_state = FileTransfer::Read;
				if(m_notify)
				{
					m_notify->transfer(true);
					m_notify->process();
				}
				m_state = Reading;
			}
			break;

		case Reading:
			{
				if(m_bytes < m_fileSize)
				{
					long recvd_size = (unsigned long)(m_socket->readBuffer().size() - m_socket->readBuffer().readPos());
					if(size < 0)
					{
						return;
					}
					receiveNextBlock(recvd_size);
				}
				if(m_bytes >= m_fileSize)
				{
					/// TODO Calculate and verify checksum
					m_oft.type = OFT_success;
					writeOFT(&m_oft);
					//EventLog::log_packet(m_socket->writeBuffer(), true, ICQPlugin::icq_plugin->AIMDirectPacket); //commented out due to problems with netmon while transfer
					m_socket->write();

					if(m_totalBytes >= m_totalSize)
					{
						if(m_notify)
							m_notify->transfer(false);
						ICQBuffer buf;
						buf << (unsigned short) 0x0002 << m_cookie.id_l << m_cookie.id_h;
						buf.pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);
						if(m_file)
							m_file->flush();

						m_client->snacICBM()->sendThroughServer(m_client->screen(m_data), 0x0002, buf, m_cookie, false, true);
						m_state = Done;
					}
					else
					{
						m_state = OFTNegotiation;
					}
					return;
				}
			}
			break;
		default:
			break;
	}
}
void AIMIncomingFileTransfer::startReceive(unsigned /*pos*/)
{
	m_oft.type = OFT_answer;
	*((unsigned long*)&m_oft.cookie[0]) = htonl(m_cookie.id_l);
	*((unsigned long*)&m_oft.cookie[4]) = htonl(m_cookie.id_h);
	writeOFT(&m_oft);
	//EventLog::log_packet(m_socket->writeBuffer(), true, ICQPlugin::icq_plugin->AIMDirectPacket); //commented out due to problems with netmon while transfer
	m_socket->write();
	m_nFile = m_oft.total_files - m_oft.files_left + 1;
	m_nFiles = m_oft.total_files;
	m_fileSize = m_oft.size;
	m_totalSize = m_oft.total_size;
}

void AIMIncomingFileTransfer::ackOFT()
{
	log(L_DEBUG, "Sending file ack");
	if(m_notify)
	{
		m_notify->transfer(false);
		if(m_oft.nencode == 0x0200) // this is ucs2
		{
            m_notify->createFile(QString::fromUtf16((unsigned short*)m_oft.name.data()), m_fileSize, true);
		}
		else
		{
			m_notify->createFile(QString(m_oft.name), m_fileSize, true);
		}
	}
}

void AIMIncomingFileTransfer::receiveNextBlock(long size)
{
	m_totalBytes += size;
	m_bytes += size;
	m_transferBytes += size;
	if(size)
	{
		if(!m_file)
		{
			log(L_DEBUG, "Write without file");
			return;
		}
                long hret = m_file->write(m_socket->readBuffer().data(m_socket->readBuffer().readPos()), size);
		if(hret != size)
		{
			log(L_DEBUG, "Error while writing to file: %d", (int)hret);
			m_socket->error_state("Error write file");
			return;
		}
	}
	if (m_notify)
		m_notify->process();
	m_socket->readBuffer().incReadPos(size);
}

void AIMIncomingFileTransfer::write_ready()
{
	log(L_DEBUG, "AIMIncomingFileTransfer::write_ready()");
	if(m_state == Done)
	{
		FileTransfer::m_state = FileTransfer::Done;
		//m_client->deleteFileMessage(m_cookie);
		if(m_notify)
			m_notify->process();
		// I'm not sure who is responsible for connection closing in this case.
		// If sender is icq6, it closes this socket itself. Pidgin, however, does not.
		m_socket->close();
	}
}

void AIMIncomingFileTransfer::connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2)
{
	m_state = ProxyConnection;
	AIMFileTransfer::connectThroughProxy(host, port, cookie2);
}

AIMFileTransfer::tTransferDirection AIMIncomingFileTransfer::getDirection()
{
	return tdInput;
}


AIMOutcomingFileTransfer::AIMOutcomingFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client) : AIMFileTransfer(msg, data, client), m_connectTimer(this)
{
	QObject::connect(&m_connectTimer, SIGNAL(timeout()), this, SLOT(connect_timeout()));
	openFile();
	m_totalBytes = 0;
	EventMessageAcked(m_msg).process();
}

AIMOutcomingFileTransfer::~AIMOutcomingFileTransfer()
{
}

void AIMOutcomingFileTransfer::listen()
{
	log(L_DEBUG, "AIMFileTransfer::listen");
    m_state = Listen;
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
	FileTransfer::m_state = FileTransfer::Connect;
	if(m_notify)
		m_notify->process();
}

bool AIMOutcomingFileTransfer::accept(Socket *s, unsigned long)
{
    log(L_DEBUG, "Accept AIM file transfer");
    m_state = OFTNegotiation;

    m_socket->setSocket(s);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();

	FileTransfer::m_state = FileTransfer::Negotiation;
	if (m_notify)
		m_notify->process();

    m_socket->setRaw(true);
	initOFTSending();

    return true;
}

void AIMOutcomingFileTransfer::initOFTSending()
{
	int delta_length = filename().length() - 0x40;
	if(delta_length < 0)
		delta_length = 0;

	m_oft.magic = OFT_magic;
	m_oft.unknown = htons(256 + delta_length);
	m_oft.type = OFT_fileInfo;

	*((unsigned long*)&m_oft.cookie[0]) = htonl(m_cookie.id_l);
	*((unsigned long*)&m_oft.cookie[4]) = htonl(m_cookie.id_h);

	m_oft.encrypt = 0;
	m_oft.compress = 0;
	m_oft.total_files = files();
	m_oft.files_left = files() - file();
	m_oft.total_parts = 1; //FIXME if needed
	m_oft.parts_left = 1;
	m_oft.total_size = totalSize();
	m_oft.size = fileSize();
	m_oft.mod_time = QDateTime::currentDateTime().toTime_t(); //FIXME
	m_oft.checksum = calculateChecksum();
	m_oft.rfrcsum = 0x0000ffff;
	m_oft.rfsize = 0x0;
	m_oft.cretime = 0x0;
	m_oft.rfcsum = 0x0000ffff;
	m_oft.nrecvd = 0;
	m_oft.recvcsum = 0x0000ffff;
	memset(m_oft.idstring, 0, 32);
	strncpy((char*)m_oft.idstring, "Cool FileXfer", 31);
	m_oft.flags = 0x20; //FIXME magic
	m_oft.lnameoffset = 0x1c; // ???
	m_oft.lsizeoffset = 0x11;
	memset(m_oft.dummy, 0, 69);
	memset(m_oft.macfileinfo, 0, 16);
	//FileMessage* msg = static_cast<FileMessage*>(m_msg); //Fixme: msg is initialized, but not used.

//	QString filename = filename();
	bool bWide = false;
	for(int i = 0; i < (int)(filename().length() + 1); i++)
	{
		if (filename()[i].unicode() > 0x7F)
		{
			bWide = true;
			break;
		}
	}

	if(bWide)
	{
		m_oft.nencode = 0x0200;
		m_oft.nlanguage = 0;
		m_oft.name.resize((filename().length() + 1) * 2);
		for(int i = 0; i < (int)(filename().length() + 1); i++)
		{
			*((unsigned short*)(&m_oft.name.data()[i * 2])) = htons(filename()[i].unicode());
		}
	}
	else
	{
		m_oft.nencode = 0x0;
		m_oft.nlanguage = 0;
        m_oft.name = QByteArray( filename().toUtf8().data(), filename().length() + 1 );
	}
	writeOFT(&m_oft);
	//EventLog::log_packet(m_socket->writeBuffer(), true, ICQPlugin::icq_plugin->AIMDirectPacket); //commented out due to problems with netmon while transfer
	m_socket->write();
}

void AIMOutcomingFileTransfer::packet_ready()
{
	log(L_DEBUG, "AIMOutcomingFileTransfer::packet_ready %d", m_state);
	//ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin()); //commented out due to problems with netmon while transfer
	//EventLog::log_packet(m_socket->readBuffer(), false, plugin->AIMDirectPacket, m_client->screen(m_data)); //commented out due to problems with netmon while transfer
	switch(m_state)
	{
		case ProxyNegotiation:
			{
				unsigned short packet_length, chunk_id, status;
				m_socket->readBuffer() >> packet_length;
				m_socket->readBuffer() >> chunk_id;
				log(L_DEBUG, "[Output]Proxy packet, length = %d, chunk_id = %04x",packet_length, chunk_id);
				if(chunk_id == Chunk_status)
				{
					m_socket->readBuffer() >> status;
					log(L_DEBUG, "status = %04x", status);
					// TODO Handle errors
					if(status == 0x0003)
					{
						m_socket->readBuffer().incReadPos(6);
						m_socket->readBuffer() >> m_cookie2;
						m_socket->readBuffer() >> m_ip;
						FileMessage* msg = static_cast<FileMessage*>(m_msg);
						QString filename = msg->getDescription();
						m_stage++;
						requestFT();
					}
					if(status == 0x0005) // Everything is allright
					{
						log(L_DEBUG, "Connection accepted");
						// Read the rest of a packet:
						if(!m_proxyActive)
						{
							ICQBuffer buf;
							buf << (unsigned short) 0x0002 << m_cookie.id_l << m_cookie.id_h;
							buf.pack(m_client->capabilities[CAP_AIM_SENDFILE], 0x10);

							m_client->snacICBM()->sendThroughServer(m_client->screen(m_data), 0x0002, buf, m_cookie, false, true);
						}
						m_socket->readBuffer().incReadPos(packet_length - 4);
						FileTransfer::m_state = FileTransfer::Negotiation;
						if(m_notify)
							m_notify->process();
						m_state = OFTNegotiation;

						initOFTSending();
					}
				}
				return;
			}
			break;
		case OFTNegotiation:
			{
				log(L_DEBUG, "Output, negotiation");
				if(!m_notify)
				{
					log(L_DEBUG, "m_notify == NULL!!! Achtung!! Alarm!!11");
				}
				OftData this_oft;
				readOFT(&this_oft);
				if(this_oft.magic != OFT_magic)
				{
					log(L_WARN, "Invalid magic in OFT");
					// TODO cleanup
					return;
				}
				if(this_oft.type == OFT_success)
				{
					log(L_DEBUG, "File transfer OK(3)");
					FileTransfer::m_state = FileTransfer::Done;
					m_socket->close();
					if (m_notify)
						m_notify->process();
					m_socket->error_state(QString::null);
					return;
				}
				if(this_oft.type != OFT_answer)
				{
					log(L_WARN, "Error in OFT");
					// TODO cleanup
					return;
				}
				m_file->reset();
				m_state = Writing;
				// TODO Check other fields in this_oft
				FileTransfer::m_state = FileTransfer::Write;

				if(m_notify)
				{
					m_notify->transfer(true);
					m_notify->process();
				}

				log(L_DEBUG, "m_nFile = %d", file());
				sendNextBlock();
			}
			break;
		case Writing:
			{
				log(L_DEBUG, "Output, write");
				OftData this_oft;
				readOFT(&this_oft);
				if(this_oft.magic != OFT_magic)
				{
					log(L_WARN, "Invalid magic in OFT");
					// TODO cleanup
					return;
				}
				if(this_oft.type == OFT_success)
				{
					log(L_DEBUG, "File transfer OK(4)");
					if(totalBytes() >= totalSize())
					{
						FileTransfer::m_state = FileTransfer::Done;
						m_socket->close();
						m_socket->error_state(QString::null);
						EventSent(m_msg).process();
						if(m_notify)
						{
							m_notify->transfer(false);
							m_notify->process();
						}
					}
					else
					{
						m_state = OFTNegotiation;
						openFile();
						log(L_DEBUG, "m_nFile = %d", file());
						if(m_notify)
							m_notify->process();
						initOFTSending();
					}
					return;
				}
			}
			break;
		default:
			break;
	}
	if (m_socket->readBuffer().readPos() <= m_socket->readBuffer().writePos())
	{
		return;
	}
	//ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin()); //commented out due to problems with netmon while transfer
	//EventLog::log_packet(m_socket->readBuffer(), false, plugin->AIMDirectPacket, m_client->screen(m_data)); //commented out due to problems with netmon while transfer
	m_socket->readBuffer().init(0);
}

bool AIMOutcomingFileTransfer::sendNextBlock()
{
	if(!m_file)
	{
		log(L_DEBUG, "Read without file");
		m_socket->error_state("Read without file");
		return false;
	}

	char* buffer = new char[m_packetLength + 1]; // FIXME replace it with QByteArray

	int bytes_read = m_file->read(buffer, m_packetLength);
	if(bytes_read < 0)
	{
		log(L_DEBUG, "Error while reading file");
		m_socket->error_state("Error while reading file");
		delete [] buffer;
		return false;
	}
	if(bytes_read == 0)
	{
		delete [] buffer;
		return true;
	}
	m_socket->writeBuffer().pack(buffer, bytes_read);
	m_socket->write();
	m_totalBytes += bytes_read;
	m_bytes += bytes_read;
	m_transferBytes += bytes_read;

	if(m_notify)
		m_notify->process();

	delete [] buffer;
	return true;
}

void AIMOutcomingFileTransfer::connect_ready()
{
	log(L_DEBUG, "AIMOutcomingFileTransfer::connect_ready() %d %d", m_state, m_proxyActive);
	if(m_state == ProxyConnection)
	{
		negotiateWithProxy();
		m_state = ProxyNegotiation;
	}
}

void AIMOutcomingFileTransfer::write_ready()
{
	if(FileTransfer::m_state != FileTransfer::Connect)
	{
		if(totalBytes() < totalSize())
		{
			if(FileTransfer::m_state == FileTransfer::Write)
				sendNextBlock();
		}
		else
		{
			/// TODO Calculate and verify checksum
			log(L_DEBUG, "File transfer OK(6)");
		}
	}
}

void AIMOutcomingFileTransfer::connect_timeout()
{
	if(m_state == ReverseConnection)
	{
		FileMessage* msg = static_cast<FileMessage*>(m_msg);
		QString filename = msg->getDescription();
	}
}

AIMFileTransfer::tTransferDirection AIMOutcomingFileTransfer::getDirection()
{
	return tdOutput;
}

void AIMOutcomingFileTransfer::connectThroughProxy(const QString& host, uint16_t port, uint16_t cookie2)
{
	m_state = ProxyConnection;
	AIMFileTransfer::connectThroughProxy(host, port, cookie2);
}

void AIMOutcomingFileTransfer::connect(unsigned long ip, unsigned short port)
{
	log(L_DEBUG, "AIMOutcomingFileTransfer::connect");

	if(m_stage == 2)
	{
		m_port = port;
		FileTransfer::m_state = FileTransfer::Connect;
		if (m_notify)
			m_notify->process();

		m_state = ProxyConnection;
                connectThroughProxy(AOL_PROXY_HOST, AOL_PROXY_PORT, 0);
	}
	else
	{
		AIMFileTransfer::connect(ip, port);
	}
}

