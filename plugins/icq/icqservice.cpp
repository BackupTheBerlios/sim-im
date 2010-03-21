/***************************************************************************
                          icqservice.cpp  -  description
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

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <QTimer>
#include <QBuffer>
#include <QImage>
#include <QCryptographicHash>
#include <QDateTime>

#include "log.h"

#include "icqclient.h"
#include "icqservice.h"
#include "contacts/contact.h"

using namespace std;
using namespace SIM;

const unsigned short ICQ_SNACxSRV_ERROR         = 0x0001;
const unsigned short ICQ_SNACxSRV_READYxCLIENT  = 0x0002;
const unsigned short ICQ_SNACxSRV_READYxSERVER	= 0x0003;
const unsigned short ICQ_SNACxSRV_SERVICExREQ	= 0x0004;
const unsigned short ICQ_SNACxSRV_SERVICExRESP	= 0x0005;
const unsigned short ICQ_SNACxSRV_REQxRATExINFO = 0x0006;
const unsigned short ICQ_SNACxSRV_RATExINFO     = 0x0007;
const unsigned short ICQ_SNACxSRV_RATExACK      = 0x0008;
const unsigned short ICQ_SNACxSRV_RATExDEL      = 0x0009;   /* Not implemented */
const unsigned short ICQ_SNACxSRV_RATExCHANGE   = 0x000A;
const unsigned short ICQ_SNACxSRV_PAUSE         = 0x000B;
const unsigned short ICQ_SNACxSRV_PAUSExACK     = 0x000C;
const unsigned short ICQ_SNACxSRV_RESUME        = 0x000D;
const unsigned short ICQ_SNACxSRV_GETxUSERxINFO = 0x000E;
const unsigned short ICQ_SNACxSRV_NAMExINFO     = 0x000F;
const unsigned short ICQ_SNACxSRV_EVIL          = 0x0010;
const unsigned short ICQ_SNACxSRV_SETxIDLE      = 0x0011;
const unsigned short ICQ_SNACxSRV_MIGRATE       = 0x0012;
const unsigned short ICQ_SNACxSRV_MOTD          = 0x0013;
const unsigned short ICQ_SNACxSRV_PRIVATY_FLAGS = 0x0014;   /* Not implemented */
const unsigned short ICQ_SNACxSRV_IMxICQ        = 0x0017;
const unsigned short ICQ_SNACxSRV_ACKxIMxICQ    = 0x0018;
const unsigned short ICQ_SNACxSRV_SETxSTATUS    = 0x001E;
const unsigned short ICQ_SNACxSRV_EXT_STATUS    = 0x0021;

SnacIcqService::SnacIcqService(ICQClient* client) : SnacHandler(client, 0x0001)
{
}

SnacIcqService::~SnacIcqService()
{
}

bool SnacIcqService::process(unsigned short subtype, ICQBuffer* buf, unsigned short /*seq*/) //seq unused
{
	switch (subtype)
	{
		case ICQ_SNACxSRV_PAUSE:
			log(L_DEBUG, "Server pause");
			m_client->m_bNoSend = true;
			m_client->snac(ICQ_SNACxFOOD_SERVICE, ICQ_SNACxSRV_PAUSExACK);
			m_client->socket()->writeBuffer() << ICQ_SNACxFOOD_SERVICE
				<< ICQ_SNACxFOOD_LOCATION
				<< ICQ_SNACxFOOD_BUDDY
				<< ICQ_SNACxFOOD_MESSAGE
				<< ICQ_SNACxFOOD_BOS
				<< ICQ_SNACxFOOD_PING
				<< ICQ_SNACxFOOD_LISTS
				<< ICQ_SNACxFOOD_VARIOUS
				<< ICQ_SNACxFOOD_LOGIN;
			m_client->sendPacket(true);
			break;
		case ICQ_SNACxSRV_RESUME:
			log(L_DEBUG, "Server resume");
			m_client->m_bNoSend = false;
			m_client->snacICBM()->processSendQueue();
			break;
		case ICQ_SNACxSRV_MIGRATE:
			{
				log(L_DEBUG, "Server migrate");
				m_client->m_bNoSend = true;
				int i;
				unsigned short cnt;
				unsigned short food[0x17];

				*buf >> cnt;
				for(i = 0; i < cnt; i++)
				{
					*buf >> food[i];
				}
				TlvList tlv(*buf);
				Tlv *tlv_adr    = tlv(0x05);
				Tlv *tlv_cookie = tlv(0x06);
				for(; i >= 0; i--)
				{

					setServiceSocket(tlv_adr,tlv_cookie,food[i]);
				}
				break;
			}
		case ICQ_SNACxSRV_RATExCHANGE:
			{
				const char *msg_text = NULL;
				unsigned short msg_code;
				unsigned short class_id;
				unsigned long  window_size;
				unsigned long  clear_level;
				unsigned long  alert_level;
				unsigned long  limit_level;
				unsigned long  discon_level;
				unsigned long  current_level;
				unsigned long  max_level;
				unsigned long  last_send;
				char current_state;
				*buf >> msg_code
					>> class_id
					>> window_size
					>> clear_level
					>> alert_level
					>> limit_level
					>> discon_level
					>> current_level
					>> max_level
					>> last_send
					>> current_state;
				if(subtype == ICQ_SNACxSRV_RATExCHANGE)
				{
					switch (msg_code)
					{
						case 0x0001:
							msg_text = "Rate limits parameters changed";
							break;
						case 0x0002:
							msg_text = "Rate limits warning";
							break;
						case 0x0003:
							msg_text = "Rate limit hit";
							break;
						case 0x0004:
							msg_text = "Rate limit clear";
							break;
						default:
							msg_text = "Unknown";
					}
					log(L_DEBUG, "%s", msg_text);
				}
				log(L_DEBUG, "grp: %02X, ws: %04lX, cl %04lX, al %04lX, ll %04lX, dl: %04lX, cur %04lX, ml %04lX",
						class_id,window_size,clear_level,alert_level,limit_level,discon_level,
						current_level,max_level);
				if (--class_id < m_client->m_rates.size()){
					RateInfo &r = m_client->m_rates[class_id];
					r.m_winSize  = window_size;
					r.m_maxLevel = max_level;
					r.m_minLevel = alert_level;
					r.m_curLevel = current_level;
					r.m_lastSend = QDateTime::currentDateTime();
					m_client->snacICBM()->processSendQueue();
				}
				break;
			}
		case ICQ_SNACxSRV_RATExINFO:
			{
				bool bNew =  m_client->m_rates.size() == 0;
				m_client->m_rates.clear();
				unsigned short n_rates;
				*buf >> n_rates;
				unsigned n;
				for(n = 0; n < n_rates; n++)
				{
					unsigned short class_id;
					unsigned long  window_size;
					unsigned long  clear_level;
					unsigned long  alert_level;
					unsigned long  limit_level;
					unsigned long  discon_level;
					unsigned long  current_level;
					unsigned long  max_level;
					unsigned long  last_send;
					char current_state;
					*buf >> class_id
						>> window_size
						>> clear_level
						>> alert_level
						>> limit_level
						>> discon_level
						>> current_level
						>> max_level
						>> last_send
						>> current_state;
					log(L_DEBUG, "grp: %02X, ws: %04lX, cl %04lX, al %04lX, ll %04lX, dl: %04lX, cur %04lX, ml %04lX, cs: %d",
							class_id,window_size,clear_level,alert_level,limit_level,discon_level,
							current_level,max_level, current_state);
					RateInfo r;
					r.m_winSize		= window_size;
					r.m_minLevel	= alert_level;
					r.m_maxLevel	= max_level;
					r.m_curLevel	= current_level;
					r.m_lastSend	= QDateTime::currentDateTime();
					m_client->m_rates.push_back(r);
				}
				for(n = 0; n < n_rates; n++)
				{
					unsigned short class_id;
					unsigned short pairs;
					*buf >> class_id
						>> pairs;
					class_id--;
					for(unsigned i = 0; i < pairs; i++)
					{
						unsigned long snac;
						*buf >> snac;
						if(class_id >= m_client->m_rates.size())
							continue;
						RATE_MAP::iterator it = m_client->m_rate_grp.find(snac);
						if (it != m_client->m_rate_grp.end())
							continue;
						m_client->m_rate_grp.insert(RATE_MAP::value_type(snac, class_id));
					}
				}
				m_client->snac(ICQ_SNACxFOOD_SERVICE, ICQ_SNACxSRV_RATExACK);
				m_client->socket()->writeBuffer() << 0x00010002L << 0x00030004L << 0x0005;
				m_client->sendPacket(true);
				if(!bNew)
					break;
				m_client->snac(ICQ_SNACxFOOD_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
				m_client->sendPacket(true);
				m_client->listsRequest();
				m_client->locationRequest();
				log(L_DEBUG, "alpha");
				m_client->buddyRequest();
				log(L_DEBUG, "beta");
				m_client->snacICBM()->rightsRequest();
				log(L_DEBUG, "gamma");
				m_client->bosRequest();
				log(L_DEBUG, "delta");
			}
			break;
		case ICQ_SNACxSRV_MOTD:
			break;
		case ICQ_SNACxSRV_ACKxIMxICQ:
			requestRateInfo();
			break;
		case ICQ_SNACxSRV_EXT_STATUS:
			{
				QByteArray shash;
				unsigned short nType;
				char flags, size;

				*buf >> nType;
				if(nType == 0)  // SSBI ready
					break;
				if(nType == 2)  // iChat message
					break;

				*buf >> flags >> size;
				shash.resize(size);
				buf->unpack(shash.data(), shash.size());

				QImage img(m_client->getPicture());
				if(img.isNull())
					break;

				QByteArray ba;
				QBuffer l_buf(&ba);
				if(!l_buf.open(QIODevice::WriteOnly))
				{
					log(L_ERROR, "Can't open QByteArray for writing!");
					break;
				}
				if(!img.save(&l_buf, "JPEG")) {
					log(L_ERROR, "Can't save QImage to QBuffer");
					break;
				}
				l_buf.close();
				QByteArray hash = QCryptographicHash::hash(ba.data(), QCryptographicHash::Md5);

				if(hash != shash) {
					log(L_WARN, "The buddyIcon on server does not match the local one - updating");
					m_client->uploadBuddy(&m_client->data.owner);
					break;
				}
				log(L_DEBUG, "SRV_EXT_STATUS");
				if(flags & FirstSend)
				{
					m_client->uploadBuddyIcon(1, img);
				}
				else
				{
					ICQUserData* data = &m_client->data.owner;
                    data->setBuddyHash(hash);
					/*
					   m_client->sendCapability(QString::null);
					   m_client->requestBuddy(data);
					   */
				}

			}
			break;
		case ICQ_SNACxSRV_NAMExINFO:
			{
				QString screen = buf->unpackScreen();
				if(screen.length() == 0){
					char n;
					*buf >> n;
					buf->incReadPos(n);
					screen = buf->unpackScreen();
				}
                if (screen.toULong() != m_client->data.owner.getUin()){
                    log(L_WARN, "Not my name info (%s)", qPrintable(screen));
					break;
				}
				buf->incReadPos(4);
				TlvList tlv(*buf);
				Tlv *tlvIP = tlv(0x000A);
				if (tlvIP)
                    m_client->data.owner.setIP(htonl((uint32_t)(*tlvIP)));
				break;
			}
		case ICQ_SNACxSRV_SERVICExRESP:
			{
				TlvList tlv(*buf);
				Tlv *tlv_id = tlv(0x0D);
				if (!tlv_id){
					log(L_WARN, "No service id in response");
					break;
				}
				Tlv *tlv_adr    = tlv(0x05);
				Tlv *tlv_cookie = tlv(0x06);
				setServiceSocket(tlv_adr,tlv_cookie,(uint16_t)(*tlv_id));
				break;
			}
		case ICQ_SNACxSRV_READYxSERVER:
			m_client->snac(ICQ_SNACxFOOD_SERVICE, ICQ_SNACxSRV_IMxICQ);
			if (m_client->m_bAIM){
				m_client->socket()->writeBuffer()
					<< 0x00010003L
					<< 0x00130003L
					<< 0x00020001L
					<< 0x00030001L
					<< 0x00040001L
					<< 0x00060001L
					<< 0x00080001L
					<< 0x00090001L
					<< 0x000A0001L
					<< 0x000B0001L;
			}else{
				m_client->socket()->writeBuffer()
					<< 0x00010004L
					<< 0x00130004L
					<< 0x00020001L
					<< 0x00030001L
					<< 0x00150001L
					<< 0x00040001L
					<< 0x00060001L
					<< 0x00090001L
					<< 0x000A0001L
					<< 0x000B0001L;
			}
			m_client->sendPacket(true);
			break;
		case ICQ_SNACxSRV_ERROR:
			break;
		case ICQ_SNACxSRV_EVIL:
			{
				unsigned short level;
				buf->unpack(level);
				QString from = buf->unpackScreen();
				m_client->data.owner.setWarningLevel(level);
				if (from.isEmpty())
					from = i18n("anonymous");
				EventNotification::ClientNotificationData d;
				d.client  = m_client;
				d.code    = 0;
				d.text = I18N_NOOP("You've been warned by %1");
				d.args    = from;
				d.flags   = EventNotification::ClientNotificationData::E_INFO;
				d.options = QString::null;
				d.id      = CmdShowWarning;
				EventClientNotification e(d);
				e.process();
				break;
			}
		default:
			log(L_WARN, "Unknown service foodgroup type %04X", subtype);
	}
	return true;
}

void SnacIcqService::requestRateInfo()
{
    m_client->snac(ICQ_SNACxFOOD_SERVICE, ICQ_SNACxSRV_REQxRATExINFO);
    m_client->sendPacket(true);
}

void SnacIcqService::addService(ServiceSocket* s)
{
    m_services.push_back(s);
}

void SnacIcqService::deleteService(ServiceSocket* s)
{
	for(std::list<ServiceSocket*>::iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
		if((*it) == s)
		{
			m_services.erase(it);
			break;
		}
	}
}

ServiceSocket* SnacIcqService::getService(unsigned short id)
{
    for(std::list<ServiceSocket*>::iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
        if((*it)->id() == id)
		{
			return (*it);
        }
    }
	log(L_WARN, "Service not found");
	return NULL;
}

void SnacIcqService::clearServices()
{
    while(!m_services.empty())
	{
        ServiceSocket *s = m_services.front();
        delete s;
    }
}

void SnacIcqService::setServiceSocket(Tlv *tlv_addr, Tlv *tlv_cookie, unsigned short service)
{
    ServiceSocket *s = getService(service);
    if(!s)
	{
        return;
    }
    if (!tlv_addr){
        s->error_state("No address for service", 0);
        return;
    }
    if (!tlv_cookie){
        s->error_state("No cookie for service", 0);
        return;
    }
    unsigned short port = m_client->getPort();
    QByteArray addr(tlv_addr->byteArray());
    int idx = addr.indexOf(':');
    if(idx != -1) {
        port = addr.mid(idx + 1).toUShort();
        addr = addr.left(idx);
    }
    if (s->connected())
        s->close();
    QByteArray ba = tlv_cookie->byteArray();
    ba.resize(ba.size()-1);
    s->connect(addr, port, ba);
}

void SnacIcqService::sendClientReady()
{
    snac(ICQ_SNACxSRV_READYxCLIENT);
    m_client->socket()->writeBuffer()
    << 0x00010004L << 0x0110164FL
    << 0x00130004L << 0x0110164FL
    << 0x00020001L << 0x0110164FL
    << 0x00030001L << 0x0110164FL
    << 0x00150001L << 0x0110164FL
    << 0x00040001L << 0x0110164FL
    << 0x00060001L << 0x0110164FL
    << 0x00090001L << 0x0110164FL
    << 0x000A0001L << 0x0110164FL
    << 0x000B0001L << 0x0110164FL;

    m_client->sendPacket(true);
}

void SnacIcqService::sendLogonStatus()
{
    if (m_client->getInvisible())
        m_client->sendInvisible(false);
    m_client->sendContactList();

    QDateTime now(QDateTime::currentDateTime());
    if (m_client->data.owner.getPluginInfoTime() == 0)
        m_client->data.owner.setPluginInfoTime(now.toTime_t());
    if (m_client->data.owner.getPluginStatusTime() == 0)
        m_client->data.owner.setPluginStatusTime(now.toTime_t());
    if (m_client->data.owner.getInfoUpdateTime() == 0)
        m_client->data.owner.setInfoUpdateTime(now.toTime_t());
	m_client->data.owner.setOnlineTime(now.toTime_t());
    if (getContacts()->owner()->getPhones() != m_client->data.owner.getPhoneBook())
	{
        m_client->data.owner.setPhoneBook(getContacts()->owner()->getPhones());
        m_client->data.owner.setPluginInfoTime(now.toTime_t());
    }
    if (m_client->getPicture() != m_client->data.owner.getPicture()){
        m_client->data.owner.setPicture(m_client->getPicture());
        m_client->data.owner.setPluginInfoTime(now.toTime_t());
    }
    if (getContacts()->owner()->getPhoneStatus() != (int)m_client->data.owner.getFollowMe()){
        m_client->data.owner.setFollowMe(getContacts()->owner()->getPhoneStatus());
        m_client->data.owner.setPluginStatusTime(now.toTime_t());
    }

    ICQBuffer directInfo(25);
    fillDirectInfo(directInfo);

    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->getFullStatus());
    m_client->socket()->writeBuffer().tlv(0x0008, (unsigned short)0x0a06);
    m_client->socket()->writeBuffer().tlv(0x000C, directInfo);
	Buffer b;
	b << (unsigned short)0x000e
		<< (unsigned short)0x0000
		<< (unsigned short)0x0002
		<< (unsigned short)0x0000;

    m_client->socket()->writeBuffer().tlv(0x001d, b);
    m_client->socket()->writeBuffer().tlv(0x001f, (unsigned short)0);
    m_client->sendPacket(true);
    if (!m_client->getInvisible())
        m_client->sendInvisible(true);
    sendIdleTime();
    m_client->m_status = m_client->m_logonStatus;
}

void SnacIcqService::setInvisible()
{
	log(L_DEBUG, "SnacIcqService::setInvisible");
    if(m_client->getInvisible())
        m_client->sendInvisible(false);
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->getFullStatus());
    m_client->sendPacket(true);
    if (!m_client->getInvisible())
        m_client->sendInvisible(true);
}

void SnacIcqService::sendStatus(unsigned long statuscode)
{
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, statuscode);
    m_client->sendPacket(true);
    sendIdleTime();
}

void SnacIcqService::sendStatus()
{
	log(L_DEBUG, "SnacIcqService::sendStatus");
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->getFullStatus());
    m_client->sendPacket(true);
    sendIdleTime();
}

void SnacIcqService::sendPluginInfoUpdate(unsigned plugin_id)
{
	log(L_DEBUG, "SnacIcqService::sendPluginInfoUpdate");
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->getFullStatus());
    ICQBuffer directInfo(25);
    fillDirectInfo(directInfo);
    m_client->socket()->writeBuffer().tlv(0x000C, directInfo);
    ICQBuffer b;
    b << (char)2;
    b.pack(m_client->data.owner.getPluginInfoTime());
    b.pack((unsigned short)2);
    b.pack((unsigned short)1);
    b.pack((unsigned short)2);
    b.pack((char*)m_client->plugins[plugin_id], sizeof(plugin));
    b.pack(m_client->data.owner.getPluginInfoTime());
    b << (char)0;
    m_client->socket()->writeBuffer().tlv(0x0011, b);
    m_client->socket()->writeBuffer().tlv(0x0012, (unsigned short)0);
    m_client->sendPacket(false);
}

void SnacIcqService::sendPluginStatusUpdate(unsigned plugin_id, unsigned long status)
{
	log(L_DEBUG, "SnacIcqService::sendPluginInfoUpdate");
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->fullStatus(m_client->m_logonStatus));
    ICQBuffer directInfo(25);
    fillDirectInfo(directInfo);
    m_client->socket()->writeBuffer().tlv(0x000C, directInfo);
    ICQBuffer b;
    b << (char)3;
    b.pack(m_client->data.owner.getPluginStatusTime());
    b.pack((unsigned short)0);
    b.pack((unsigned short)1);
    b.pack((unsigned short)1);
    b.pack((char*)m_client->plugins[plugin_id], sizeof(plugin));
    b << (char)1;
    b.pack(status);
    b.pack(m_client->data.owner.getPluginStatusTime());
    b.pack((unsigned short)0);
    b.pack((unsigned short)0);
    b.pack((unsigned short)1);
    m_client->socket()->writeBuffer().tlv(0x0011, b);
    m_client->socket()->writeBuffer().tlv(0x0012, (unsigned short)0);
    m_client->sendPacket(false);
}

void SnacIcqService::sendUpdate()
{
    if (m_nUpdates == 0)
        return;
    if (--m_nUpdates)
        return;
    m_client->data.owner.setInfoUpdateTime(QDateTime::currentDateTime().toTime_t());
    snac(ICQ_SNACxSRV_SETxSTATUS);
    m_client->socket()->writeBuffer().tlv(0x0006, m_client->getFullStatus());
    ICQBuffer directInfo(25);
    fillDirectInfo(directInfo);
    m_client->socket()->writeBuffer().tlv(0x000C, directInfo);
    m_client->sendPacket(false);
}

void SnacIcqService::fillDirectInfo(ICQBuffer &directInfo)
{
    m_client->data.owner.setRealIP(m_client->socket()->localHost());
	/*
    if (m_client->getHideIP()){
        directInfo
        << (unsigned long)0
        << (unsigned long)0;
    }else{
        directInfo
        << (unsigned long)htonl(get_ip(m_client->data.owner.RealIP))
        << (unsigned short)0
        << (unsigned short)m_client->data.owner.Port.toULong();
    }

    char mode = DIRECT_MODE_DIRECT;
    unsigned long ip1 = get_ip(m_client->data.owner.IP);
    unsigned long ip2 = get_ip(m_client->data.owner.RealIP);
    if (ip1 && ip2 && (ip1 != ip2))
        mode = DIRECT_MODE_INDIRECT;
    switch (m_client->socket()->socket()->mode()){
    case Socket::Indirect:
        mode = DIRECT_MODE_INDIRECT;
        break;
    case Socket::Web:
        mode = DIRECT_MODE_DENIED;
        break;
    default:
        break;
    }
	*/
    directInfo
	<< (unsigned long)0
	<< (unsigned long)0
	<< (char)0x00//mode
    << (char)0x00
    << (char)ICQ_TCP_VERSION
    << m_client->data.owner.getDCcookie()
	<< 0x00000000L
	<< 0x00000000L
	<< 0x00000000L
	<< 0x00000000L
	<< 0x00000000L
	<< (unsigned short)0x0000;

	/*
    directInfo
    << 0x00000050L
    << 0x00000003L
    << m_client->data.owner.getInfoUpdateTime()
    << m_client->data.owner.getPluginInfoTime()
    << m_client->data.owner.getPluginStatusTime()
    << (unsigned short) 0x0000;
	*/
}

void SnacIcqService::sendIdleTime()
{
    // avoid traffic
    if(!m_idleTime && m_client->getIdleTime() == 0)
        return;
    snac(ICQ_SNACxSRV_SETxIDLE);
    if(m_client->getIdleTime()) {
        unsigned long idle = QDateTime::currentDateTime().toTime_t() - m_client->getIdleTime();
        if (idle <= 0)
            idle = 1;
        m_client->socket()->writeBuffer() << idle;
        m_idleTime = true;
    } else {
        m_client->socket()->writeBuffer() << (unsigned long)0;
        m_idleTime = false;
    }
    m_client->sendPacket(false);
}

void SnacIcqService::requestService(ServiceSocket *s)
{
    snac(ICQ_SNACxSRV_SERVICExREQ, true);
    m_client->socket()->writeBuffer() << s->id();
    m_client->sendPacket(true);
}

