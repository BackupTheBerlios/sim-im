/***************************************************************************
                          icqicmb.cpp  -  description
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

#include <stdio.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#endif

#include <algorithm>

#include <QTextCodec>
#include <QFile>
#include <QTimer>
#include <QImage>
#include <QRegExp>
#include <QByteArray>
#include <QHostAddress>

#include "contacts.h"
#include "html.h"
#include "unquot.h"
#include "log.h"
#include "core_events.h"
#include "misc.h"

#include "icqclient.h"
#include "icqmessage.h"
#include "icqdirect.h"
#include "icq.h"
#include "contacts/contact.h"
#include "contacts/group.h"

using namespace std;
using namespace SIM;

const unsigned MAX_TYPE2_SIZE	= 0x1800;
const unsigned SEND_TIMEOUT		= 50000;

bool operator < (const alias_group &s1, const alias_group &s2)
{
    return s1.grp < s2.grp;
}

const unsigned short ICQ_SNACxMSG_ERROR            = 0x0001;
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_RESETxICQxMODE   = 0x0003;    // not implemented
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_BLAMExUSER       = 0x0008;
const unsigned short ICQ_SNACxMSG_BLAMExSRVxACK    = 0x0009;
const unsigned short ICQ_SNACxMSG_SRV_MISSED_MSG   = 0x000A;
const unsigned short ICQ_SNACxMSG_AUTOREPLY        = 0x000B;
const unsigned short ICQ_SNACxMSG_ACK              = 0x000C;
const unsigned short ICQ_SNACxMSG_MTN			   = 0x0014;

static void copyTlv(ICQBuffer &b, TlvList *tlvs, unsigned nTlv)
{
    if (tlvs == NULL)
        return;
    Tlv *tlv = (*tlvs)(nTlv);
    if (tlv == NULL)
        return;
    b.tlv(nTlv, *tlv, tlv->Size());
}

static char c2h(char c)
{
    c = (char)(c & 0xF);
    if (c < 10)
        return (char)('0' + c);
    return (char)('A' + c - 10);
}

static void b2h(char *&p, char c)
{
    *(p++) = c2h((char)(c >> 4));
    *(p++) = c2h(c);
}

void packCap(ICQBuffer &b, const capability &c)
{
    char pack_cap[0x27];
    char *p = pack_cap;
    *(p++) = '{';
    b2h(p, c[0]); b2h(p, c[1]); b2h(p, c[2]); b2h(p, c[3]);
    *(p++) = '-';
    b2h(p, c[4]); b2h(p, c[5]);
    *(p++) = '-';
    b2h(p, c[6]); b2h(p, c[7]);
    *(p++) = '-';
    b2h(p, c[8]); b2h(p, c[9]);
    *(p++) = '-';
    b2h(p, c[10]); b2h(p, c[11]);
    b2h(p, c[12]); b2h(p, c[13]); b2h(p, c[14]); b2h(p, c[15]);
    *(p++) = '}';
    *p = 0;
    b << pack_cap;
}

SnacIcqICBM::SnacIcqICBM(ICQClient* client) : QObject(NULL), SnacHandler(client, 0x0004)
{
	m_sendTimer = new QTimer(this);
	connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(sendTimeout()));
}

SnacIcqICBM::~SnacIcqICBM()
{
}

void SnacIcqICBM::rightsRequest()
{
    client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
    client()->sendPacket(true);
}

void SnacIcqICBM::sendICMB(unsigned short channel, unsigned long flags)
{
    client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE);
    client()->socket()->writeBuffer()
    << channel << flags
	<< (unsigned short)0x1f40
	<< (unsigned short)0x30e7
	<< (unsigned short)0x30e7
	<< (unsigned short)0x0000
	<< (unsigned short)0x0000;
    client()->sendPacket(true);
}

void SnacIcqICBM::sendThroughServer(const QString &screen, unsigned short channel, ICQBuffer &b, const MessageId &id, bool bOffline, bool bReqAck)
{
    // we need informations about channel 2 tlvs !
    unsigned short tlv_type = 5;
    client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_SENDxSERVER, true, true);
    socket()->writeBuffer() << id.id_l << id.id_h;
    socket()->writeBuffer() << channel;
    socket()->writeBuffer().packScreen(screen);
    if (channel == 1)
        tlv_type = 2;
    if (b.size())
        socket()->writeBuffer().tlv(tlv_type, b);
    if (bReqAck)
        socket()->writeBuffer().tlv(3);		// req. ack from server
    if (bOffline)
        socket()->writeBuffer().tlv(6);	// store if user is offline
    client()->sendPacket(true);
}

bool SnacIcqICBM::ackMessage(Message *msg, unsigned short ackFlags, const QByteArray &msg_str)
{
    switch (ackFlags){
    case ICQ_TCPxACK_OCCUPIED:
    case ICQ_TCPxACK_DND:
    case ICQ_TCPxACK_REFUSE:
        if (msg_str.isEmpty())
            msg->setError(I18N_NOOP("Message declined"));
        else
            msg->setError(msg_str);
        switch (ackFlags){
        case ICQ_TCPxACK_OCCUPIED:
            msg->setRetryCode(static_cast<ICQPlugin*>(client()->protocol()->plugin())->RetrySendOccupied);
            break;
        case ICQ_TCPxACK_DND:
            msg->setRetryCode(static_cast<ICQPlugin*>(client()->protocol()->plugin())->RetrySendDND);
            break;
        }
        return false;
    }
    return true;
}

void SnacIcqICBM::sendType1(const QString &text, bool bWide, ICQUserData *data)
{
    ICQBuffer msgBuf;
    const ENCODING *encoding = getContacts()->getEncoding(client()->getContact(data));
    unsigned short usLang = 0;
    if( ( NULL == encoding ) || !strcmp( encoding->codec, "UTF-8" ) ) {
        bWide = true;
    }
    else {
        usLang = encoding->cp_code;
    }

    if (bWide)
    {
        QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");
        Q_ASSERT(codec);
        msgBuf << (unsigned short)0x0002L;
        msgBuf << (unsigned short)0x0000L;
        QByteArray ba = codec->fromUnicode( text );
        msgBuf.pack(ba, ba.size() );
    }
    else
    {
        log(L_DEBUG, "%s", qPrintable(client()->getContact(data)->getEncoding()));
        QByteArray msg_text = getContacts()->fromUnicode(client()->getContact(data), text);
        EventSend e(m_send.msg, msg_text);
        e.process();
        msg_text = e.localeText();
        msgBuf << (unsigned short)0x0000L;
        msgBuf << (unsigned short)usLang;
        msgBuf << msg_text.data();
    }
    ICQBuffer b;
    b.tlv(0x0501, "\x01", 1);
    b.tlv(0x0101, msgBuf);
    sendThroughServer(m_send.screen, 1, b, m_send.id, true, true);
    if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) || (client()->getAckMode() == 0))
        ackMessage(m_send);
}

void SnacIcqICBM::sendType2(const QString &screen, ICQBuffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, unsigned short port, TlvList *tlvs, unsigned short type)
{
    ICQBuffer b;
    b << (unsigned short)0;
    b << id.id_l << id.id_h;
    b.pack((char*)client()->capabilities[cap], sizeof(capability));
    b.tlv(0x0A, (unsigned short)type);
	if(type != 4)
		b.tlv(0x0F);
    copyTlv(b, tlvs, 0x14);
    if(port)
	{
		if(type != 4)
		{
			b.tlv(0x03, (unsigned long)htonl(get_ip(client()->data.owner.RealIP)));
			if(type != 3)
			{
				b.tlv(0x04, (unsigned long)htonl(get_ip(client()->data.owner.IP)));
			}
		}
        b.tlv(0x05, port);
                log(L_DEBUG, "RealIP = %08x, IP = %08x, port = %04x", (unsigned int)(get_ip(client()->data.owner.RealIP)), (unsigned int)(get_ip(client()->data.owner.IP)), port);
    }
    copyTlv(b, tlvs, 0x17);
    copyTlv(b, tlvs, 0x0E);
    copyTlv(b, tlvs, 0x0D);
    copyTlv(b, tlvs, 0x0C);
    copyTlv(b, tlvs, 0x10);
    copyTlv(b, tlvs, 0x02);
    copyTlv(b, tlvs, 0x16);
	if(type != 4)
		b.tlv(0x2711, msgBuf);
    copyTlv(b, tlvs, 0x2712);
    copyTlv(b, tlvs, 0x03);
    sendThroughServer(screen, 2, b, id, bOffline, true);
}

ICQClientSocket* SnacIcqICBM::socket()
{
	return client()->socket();
}

void SnacIcqICBM::sendAdvMessage(const QString &screen, ICQBuffer &msgText, unsigned plugin_index, const MessageId &id, bool bOffline, bool bDirect, unsigned short cookie1, unsigned short cookie2, unsigned short type)
{
	if (cookie1 == 0)
	{
		client()->m_advCounter--;
		cookie1 = client()->m_advCounter;
		cookie2 = (plugin_index == PLUGIN_NULL) ? 0x0E : 0x12;
	}
	ICQBuffer msgBuf;
	msgBuf.pack((unsigned short)0x1B);
	msgBuf.pack((unsigned short)0x08);
	msgBuf.pack((char*)client()->plugins[plugin_index], sizeof(plugin));
	msgBuf.pack(0x00000003L);
	msgBuf.pack((char)(type ? 4 : 0));
	msgBuf.pack(cookie1);
	msgBuf.pack(cookie2);
	msgBuf.pack(cookie1);
	msgBuf.pack(0x00000000L);
	msgBuf.pack(0x00000000L);
	msgBuf.pack(0x00000000L);
	msgBuf.pack(msgText.data(0), msgText.size());
	sendType2(screen, msgBuf, id, CAP_SRV_RELAY, bOffline, bDirect ? client()->data.owner.Port.toULong() : 0, NULL, type);
}

void SnacIcqICBM::ackMessage(SendMsg &s)
{
    if (s.flags == PLUGIN_AIM_FT){
        s.msg->setError(I18N_NOOP("File transfer declined"));
        EventMessageSent(s.msg).process();
        delete s.msg;
        s.msg = NULL;
        s.screen = QString::null;
        m_sendTimer->stop();
        processSendQueue();
        return;
    }
    if ((s.msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        if ((s.flags & SEND_MASK) == SEND_RAW){
            s.msg->setClient(client()->dataName(m_send.screen));
            EventSent(s.msg).process();
        }else if (!s.part.isEmpty()){
            Message m(MessageGeneric);
            m.setContact(s.msg->contact());
            m.setBackground(s.msg->getBackground());
            m.setForeground(s.msg->getForeground());
            unsigned flags = s.msg->getFlags() & (~MESSAGE_RICHTEXT);
            if ((s.flags & SEND_MASK) == SEND_RTF){
                flags |= MESSAGE_RICHTEXT;
                m.setText(client()->removeImages(s.part, true));
            }else if ((s.flags & SEND_MASK) == SEND_HTML){
                flags |= MESSAGE_RICHTEXT;
                m.setText(client()->removeImages(s.part, false));
            }else{
                m.setText(s.part);
            }
            m.setFlags(flags);
            m.setClient(client()->dataName(s.screen));
            EventSent(&m).process();
        }
    }
    if ((s.text.length() == 0) || (s.msg->type() == MessageWarning)){
        EventMessageSent(s.msg).process();
        delete s.msg;
        s.msg = NULL;
        s.screen = QString::null;
    }
	else
	{
        sendFgQueue.push_front(s);
    }
    m_sendTimer->stop();
    processSendQueue();
}

bool SnacIcqICBM::sendThruServer(Message *msg, void *_data)
{
    ICQUserData *data = client()->toICQUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    Contact *contact = getContacts()->contact(msg->contact());
    if ((contact == NULL) || (data == NULL))
        return false;
    SendMsg s;
    switch (msg->type()){
    case MessageGeneric:
        if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) && (client()->getSendFormat() == 0) &&
                client()->hasCap(data, CAP_RTF) && (msg->getFlags() & MESSAGE_RICHTEXT) &&
                !data->bBadClient.toBool()){
            s.flags  = SEND_RTF;
            s.msg    = msg;
            s.text   = msg->getRichText();
            s.screen = client()->screen(data);
            sendFgQueue.push_back(s);
            processSendQueue();
            return true;
        }
        if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) &&
                (client()->getSendFormat() <= 1) &&
                client()->hasCap(data, CAP_UTF) &&
                ((msg->getFlags() & MESSAGE_SECURE) == 0) &&
                (data->Version.toULong() >= 8) && !data->bBadClient.toBool()){
            s.flags  = SEND_UTF;
            s.msg    = msg;
            s.text   = client()->addCRLF(msg->getPlainText());
            s.screen = client()->screen(data);
            sendFgQueue.push_back(s);
            processSendQueue();
            return true;
        }
        if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) &&
                (data->Version.toULong() >= 8) &&
                !data->bBadClient.toBool() &&
                ((unsigned)msg->getPlainText().length() >= MAX_PLAIN_MESSAGE_SIZE)){
            s.flags  = SEND_TYPE2;
            s.msg    = msg;
            s.text   = msg->getPlainText();
            s.screen = client()->screen(data);
            sendFgQueue.push_back(s);
            processSendQueue();
            return true;
        }
        if ((data->Uin.toULong() == 0) || client()->m_bAIM){
            s.msg	 = msg;
            if (msg->getFlags() & MESSAGE_RICHTEXT){
                s.flags  = SEND_HTML;
                s.text	 = client()->removeImages(msg->getRichText(), false);
            }else{
                s.flags  = SEND_HTML_PLAIN;
                s.text	 = msg->getPlainText();
            }
            s.screen = client()->screen(data);
            sendFgQueue.push_back(s);
            processSendQueue();
            return true;
        }
        s.flags	 = SEND_PLAIN;
        s.msg	 = msg;
        s.text	 = client()->addCRLF(msg->getPlainText());
        s.screen = client()->screen(data);
        sendFgQueue.push_back(s);
        processSendQueue();
        return true;
    case MessageUrl:
        if ((data->Uin.toULong() == 0) || client()->m_bAIM){
            UrlMessage *m = static_cast<UrlMessage*>(msg);
            QString text = "<a href=\"";
            text += m->getUrl();
            text += "\">";
            text += m->getUrl();
            text += "</a><br>";
            text += client()->removeImages(msg->getRichText(), false);
            s.flags  = SEND_HTML;
            s.msg	 = msg;
            s.text	 = text;
            s.screen = client()->screen(data);
            sendFgQueue.push_back(s);
            processSendQueue();
            return true;
        }
    case MessageContacts:
    case MessageFile:
    case MessageWarning:
        s.flags  = SEND_RAW;
        s.msg    = msg;
        s.screen = client()->screen(data);
        sendFgQueue.push_back(s);
        processSendQueue();
        return true;
    }
    return false;
}

void SnacIcqICBM::clearMsgQueue()
{
	list<SendMsg>::iterator it;
	for (it = sendFgQueue.begin(); it != sendFgQueue.end(); ++it){
		if (it->socket){
			// dunno know if this is ok - vladimir please take a look
			it->socket->acceptReverse(NULL);
			continue;
		}
		if (it->msg)
		{
			it->msg->setError(I18N_NOOP("Client go offline"));
			EventMessageSent(it->msg).process();
			if (it->msg == m_send.msg)
				m_send.msg = NULL;
			delete it->msg;
		}
	}
	sendFgQueue.clear();
	for (it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it){
		if (it->socket){
			// dunno know if this is ok - vladimir please take a look
			it->socket->acceptReverse(NULL);
			continue;
		}
		if(it->msg)
		{
			it->msg->setError(I18N_NOOP("Client go offline"));
			EventMessageSent(it->msg).process();
			if (it->msg == m_send.msg)
				m_send.msg = NULL;
			delete it->msg;
		}
	}
	sendBgQueue.clear();
	if (m_send.msg)
	{
		m_send.msg->setError(I18N_NOOP("Client go offline"));
		EventMessageSent(m_send.msg).process();
		delete m_send.msg;
	}
	m_send.msg    = NULL;
	m_send.screen = QString::null;
}

void SnacIcqICBM::sendFile(TlvList& tlv, unsigned long primary_ip, unsigned long secondary_ip, unsigned short port,const QString &screen, MessageId const& id)
{
	log(L_DEBUG, "ICQClient::icbmSendFile()");
	Tlv *desc = tlv(0x0A);
	Tlv *info = tlv(0x2711);
	QString d;
	unsigned short type;
	unsigned short nFiles;
	unsigned long  size;
	bool is_proxy = tlv(0x10);
	log(L_DEBUG, "Desc = %d", (uint16_t)(*desc));

	// First, let's find our filetransfer
	AIMFileTransfer* ft = NULL;
	for(list<AIMFileTransfer*>::iterator it = client()->m_filetransfers.begin(); it != client()->m_filetransfers.end(); ++it)
	{
		if((*it)->getICBMCookie() == id)
		{
			ft = (*it);
			break;
		}
	}
	if(ft == NULL)
	{
		log(L_DEBUG, "ft == NULL");
		// Incoming file
		if(info == NULL) 
		{
			// This is baaad
			log(L_WARN, "No info tlv in send file");
			return;
		}
		ICQBuffer b(*info);
		b >> type >> nFiles >> size;
		QString name = client()->convert(b.data(8), b.size() - 8, tlv, 0x2712);
		AIMFileMessage *msg = new AIMFileMessage;
		msg->setPort(port);
		msg->setBackground(client()->clearTags(d));
		//msg->setText(d);
		msg->setSize(size);
		msg->setID_L(id.id_l);
		msg->setID_H(id.id_h);
		if(is_proxy)
		{
			msg->isProxy = true;
		}
		if(tlv(5))
		{
			msg->cookie2 = *tlv(5);
		}
		if(type == 2)
		{
			d = i18n("Directory");
			d += ' ';
			d += name;
			d += " (";
			d += i18n("%n file", "%n files", nFiles);
			d += ')';
		}
		else
		{
			if (nFiles == 1)
			{
				d = name;
			}
			else
			{
				d = i18n("%n file", "%n files", nFiles);
			}
		}
		msg->setDescription(d);
		msg->setFlags(MESSAGE_RECEIVED | MESSAGE_RICHTEXT | MESSAGE_TEMP);
		client()->m_processMsg.push_back(msg);
		client()->messageReceived(msg, screen);
		return;
	}
	AIMFileMessage* afm = NULL;
	for(list<Message*>::iterator it = client()->m_processMsg.begin(); it != client()->m_processMsg.end(); ++it)
	{
		if ((*it)->type() == MessageFile)
		{
			afm = static_cast<AIMFileMessage*>((*it));
			MessageId this_id;
			this_id.id_l = afm->getID_L();
			this_id.id_h = afm->getID_H();
			if(this_id == id)
			{
				afm->setPort(port);
			}
		}
	}

	unsigned short ft_type = *desc;
	ft->setStage(ft_type);
	log(L_DEBUG, "stage = %d", ft_type);
	if(is_proxy) // Connection through proxy
	{
		log(L_DEBUG, "Proxy request");
		for(list<AIMFileTransfer*>::iterator it = client()->m_filetransfers.begin(); it != client()->m_filetransfers.end(); ++it)
		{
			if((*it)->getICBMCookie() == id)
			{
				Contact *contact;
				ICQUserData *data = client()->findContact(screen, NULL, false, contact);
				if(data)
				{
					if(primary_ip)
						set_ip(&data->RealIP, primary_ip);
					AIMFileTransfer *ft = (*it); //Fixme:Local declaration of 'ft' hides declaration from line: 857

					ft->setProxyActive(false);
					unsigned short cookie2 = 0;
					if(tlv(5))
					{
						cookie2 = *tlv(5);
					};
					if(primary_ip)
                        ft->connectThroughProxy(QHostAddress(primary_ip).toString(), AOL_PROXY_PORT, cookie2);
					else
					{
						ft->setProxyActive(true);
						ft->connectThroughProxy(AOL_PROXY_HOST, AOL_PROXY_PORT, cookie2);
					}

					return;
				}
			}
		}
	}
	else
	{
		log(L_DEBUG, "No Proxy request: %d", ft_type);
		if(ft_type == 3)
		{
			ft->setProxyActive(true);
			ft->connectThroughProxy(AOL_PROXY_HOST, AOL_PROXY_PORT, 0);
		}
		if(ft_type == 2)
		{
			for(list<AIMFileTransfer*>::iterator it = client()->m_filetransfers.begin(); it != m_client->m_filetransfers.end(); ++it)
			{
				AIMFileTransfer *ft = (*it); //Fixme:Local declaration of 'ft' hides declaration from line: 857
				if(ft->getICBMCookie() == id)
				{
					if(primary_ip)
						ft->connect(primary_ip, port);
					else
						ft->connect(secondary_ip, port);
				}
			}
		}
		return;
	}
	if(info == NULL && afm) 
	{
		afm->setPort(port);
		return;
	}
}

void SnacIcqICBM::sendAutoReply(const QString &screen, MessageId id,
                              const plugin p, unsigned short cookie1, unsigned short cookie2,
                              unsigned short msgType, char msgFlags, unsigned short msgState,
                              const QString &response, unsigned short response_type, ICQBuffer &copy)
{
    client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
    socket()->writeBuffer() << id.id_l << id.id_h << 0x0002;
    socket()->writeBuffer().packScreen(screen);
    socket()->writeBuffer() << 0x0003 << 0x1B00 << 0x0800;
    socket()->writeBuffer().pack((char*)p, sizeof(plugin));
    socket()->writeBuffer() << 0x03000000L << (char)0;
    socket()->writeBuffer().pack(cookie1);
    socket()->writeBuffer().pack(cookie2);
    socket()->writeBuffer().pack(cookie1);
    socket()->writeBuffer() << 0x00000000L << 0x00000000L << 0x00000000L;
    socket()->writeBuffer().pack(msgType);
    socket()->writeBuffer() << msgFlags << msgState << (char)0;
    if (!response.isEmpty()){
        Contact *contact = NULL;
        client()->findContact(screen, NULL, false, contact);
        QByteArray r = getContacts()->fromUnicode(contact, response);
        unsigned short size = (unsigned short)(r.length() + 1);
        socket()->writeBuffer().pack(size);
        socket()->writeBuffer().pack(r.data(), size);
    }else{
        socket()->writeBuffer() << (char)0x01 << response_type;
    }
    if (response_type != 3){
        if (copy.size()){
            socket()->writeBuffer().pack(copy.data(0), copy.writePos());
        }else{
            socket()->writeBuffer() << 0x00000000L << 0xFFFFFF00L;
        }
    }
    client()->sendPacket(false);
}

void SnacIcqICBM::sendMTN(const QString &screen, unsigned short type)
{
    if (client()->getDisableTypingNotification())
        return;
    client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_MTN);
    socket()->writeBuffer() << 0x00000000L << 0x00000000L << (unsigned short)0x0001;
    socket()->writeBuffer().packScreen(screen);
    socket()->writeBuffer() << type;
    client()->sendPacket(true);
}

void SnacIcqICBM::sendTimeout()
{
    m_sendTimer->stop();
    if (m_send.screen.length()){
        log(L_WARN, "Send timeout");
        if (m_send.msg){
            m_send.msg->setError(I18N_NOOP("Send timeout"));
            EventMessageSent(m_send.msg).process();
            delete m_send.msg;
        }
        m_send.msg = NULL;
        m_send.screen = QString::null;
    }
    processSendQueue();
}

void SnacIcqICBM::accept(Message *msg, ICQUserData *data)
{
    MessageId id;
    if (msg->getFlags() & MESSAGE_DIRECT){
        Contact *contact = getContacts()->contact(msg->contact());
        ICQUserData *data = NULL; //Fixme: Local declaration of 'data' hides declaration of the same name in outer scope, see previous declaration at line '2163'
        if (contact){
            ClientDataIterator it(contact->clientData, client());
            while ((data = (client()->toICQUserData(++it))) != NULL){
				if (!msg->client().isEmpty() && (client()->dataName(data) == msg->client()))
                    break;
                data = NULL;
            }
        }
        if (data == NULL){
            log(L_WARN, "Data for request not found");
            return;
        }
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc == NULL){
            log(L_WARN, "No direct connection");
            return;
        }
        dc->acceptMessage(msg);
    }else{
        id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
        id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
        ICQBuffer b;
        unsigned short type = ICQ_MSGxEXT;
        client()->packMessage(b, msg, data, type, false, 0);
        unsigned cookie  = static_cast<ICQFileMessage*>(msg)->getCookie();
        sendAdvMessage(client()->screen(data), b, PLUGIN_NULL, id, false, true, (unsigned short)(cookie & 0xFFFF), (unsigned short)((cookie >> 16) & 0xFFFF), 2);
    }
}

void SnacIcqICBM::accept(Message *msg, const QString &dir, OverwriteMode overwrite)
{
    ICQUserData *data = NULL;
    bool bDelete = true;
	if (!msg->client().isEmpty()){
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact){
            ClientDataIterator it(contact->clientData, client());
            while ((data = (client()->toICQUserData(++it))) != NULL){
                if (client()->dataName(data) == msg->client())
                    break;
                data = NULL;
            }
        }
    }
    if (data){
        switch (msg->type()){
        case MessageICQFile:{
                ICQFileTransfer *ft = new ICQFileTransfer(static_cast<FileMessage*>(msg), data, client());
                ft->setDir(dir);
                ft->setOverwrite(overwrite);
                EventMessageAcked(msg).process();
                client()->m_processMsg.push_back(msg);
                bDelete = false;
                ft->listen();
                break;
            }
        case MessageFile:
			{	
                AIMFileTransfer *ft = new AIMIncomingFileTransfer(static_cast<FileMessage*>(msg), data, client());
				AIMFileMessage* fmsg = static_cast<AIMFileMessage*>(msg);
				fmsg->m_transfer = ft;
                ft->setDir(dir);
                ft->setOverwrite(overwrite);
                EventMessageAcked(msg).process();
                bDelete = false;
				ft->setPort(fmsg->getPort());
				MessageId this_id;
				this_id.id_l = fmsg->getID_L();
				this_id.id_h = fmsg->getID_H();
				ft->setICBMCookie(this_id);
				log(L_DEBUG, "port = %d", fmsg->getPort());
				ft->setStage(1);
				if(fmsg->isProxy)
				{
					ft->setICBMCookie2(fmsg->cookie2);
					ft->setProxyActive(false);
					ft->forceProxyConnection();
				}
                ft->accept();
				return;
            }
        default:
            log(L_DEBUG, "Bad message type %u for accept", msg->type());
        }
    }
    EventMessageDeleted(msg).process();
    if (bDelete)
	{
        delete msg;
	}
}

void SnacIcqICBM::decline(Message *msg, const QString &reason)
{
    if (msg->getFlags() & MESSAGE_DIRECT){
        Contact *contact = getContacts()->contact(msg->contact());
        ICQUserData *data = NULL;
        if (contact){
            ClientDataIterator it(contact->clientData, client());
            while ((data = (client()->toICQUserData(++it))) != NULL){
				if (!msg->client().isEmpty() && (client()->dataName(data) == msg->client()))
                    break;
                data = NULL;
            }
        }
        if (data == NULL){
            log(L_WARN, "Data for request not found");
            return;
        }
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc == NULL){
            log(L_WARN, "No direct connection");
            return;
        }
        dc->declineMessage(msg, reason);
    }else{
        MessageId id;
        unsigned cookie = 0;
        switch (msg->type()){
        case MessageICQFile:
            id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
            id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
            cookie  = static_cast<ICQFileMessage*>(msg)->getCookie();
            break;
        case MessageFile:
            id.id_l = static_cast<AIMFileMessage*>(msg)->getID_L();
            id.id_h = static_cast<AIMFileMessage*>(msg)->getID_H();
            break;
        default:
            log(L_WARN, "Bad type %u for decline", msg->type());
        }
        ICQUserData *data = NULL;
        Contact *contact = NULL;
		if (!msg->client().isEmpty()){
            contact = getContacts()->contact(msg->contact());
            if (contact){
                ClientDataIterator it(contact->clientData, client());
                while ((data = (client()->toICQUserData(++it))) != NULL){
                    if (client()->dataName(data) == msg->client())
                        break;
                    data = NULL;
                }
            }
        }
        if (data && (id.id_l || id.id_h)){
            if (msg->type() == MessageICQFile){
                ICQBuffer buf, msgBuf;
                ICQBuffer b;
                client()->packExtendedMessage(msg, buf, msgBuf, data);
                b.pack((unsigned short)buf.size());
                b.pack(buf.data(0), buf.size());
                b.pack32(msgBuf);
                unsigned short type = ICQ_MSGxEXT;
                sendAutoReply(client()->screen(data), id, client()->plugins[PLUGIN_NULL], (unsigned short)(cookie & 0xFFFF),
                              (unsigned short)((cookie >> 16) & 0xFFFF), type, 1, 0, reason, 2, b);
            }else{
                client()->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
                socket()->writeBuffer() << id.id_l << id.id_h << 0x0002;
                socket()->writeBuffer().packScreen(client()->screen(data));
                socket()->writeBuffer() << 0x0003 << 0x0002 << 0x0001;
                client()->sendPacket(false);
                if (!reason.isEmpty()){
                    Message *msg = new Message(MessageGeneric);  //Fixme: Local declaration of 'msg' hides declaration of the same name in outer scope, see previous declaration at line '2262'
                    msg->setText(reason);
                    msg->setFlags(MESSAGE_NOHISTORY);
                    msg->setContact(contact->id());
                    if (!client()->send(msg, data))
                        delete msg;
                }
            }
        }
    }
    EventMessageDeleted(msg).process();
    delete msg;
}


void SnacIcqICBM::requestReverseConnection(const QString &screen, DirectSocket *socket)
{
    SendMsg s;
    s.flags  = PLUGIN_REVERSE;
    s.socket = socket;
    s.screen = screen;
    sendFgQueue.push_back(s);
    processSendQueue();
}

bool SnacIcqICBM::cancelMessage(SIM::Message* msg)
{
	list<Message*>::iterator it;
	for(it = client()->m_processMsg.begin(); it != m_client->m_processMsg.end(); ++it)
		if (*it == msg)
			break;
	if(it != m_client->m_processMsg.end())
	{
		m_client->m_processMsg.erase(it);
		delete msg;
		return true;
	}
	if(msg->type() == MessageSMS)
	{
		for (list<SendMsg>::iterator it = smsQueue.begin(); it != smsQueue.end(); ++it)
		{
			if (it->msg == msg)
			{
				if (it == smsQueue.begin())
				{
					it->text = QString::null;
				}
				else
				{
					smsQueue.erase(it);
				}
				return msg;
			}
		}
	}
	else
	{
		Contact *contact = getContacts()->contact(msg->contact());
		if(contact)
		{
			ICQUserData *data;
			ClientDataIterator it(contact->clientData, m_client);
			while ((data = m_client->toICQUserData(++it)) != NULL)
			{
				DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
				if (dc && dc->cancelMessage(msg))
					return msg;
			}
		}
		if(m_send.msg == msg)
		{
			m_send.msg = NULL;
			m_send.screen = QString::null;
			m_sendTimer->stop();
			processSendQueue();
			return msg;
		}
		list<SendMsg>::iterator it;
		for(it = sendFgQueue.begin(); it != sendFgQueue.end(); ++it)
		{
			if(it->msg == msg)
			{
				sendFgQueue.erase(it);
				delete msg;
				return msg;
			}
		}
		for(it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it)
		{
			if (it->msg == msg)
			{
				sendBgQueue.erase(it);
				delete msg;
				return msg;
			}
		}
		for(it = replyQueue.begin(); it != replyQueue.end(); ++it)
		{
			if (it->msg == msg)
			{
				replyQueue.erase(it);
				delete msg;
				return msg;
			}
		}
	}
	return true;
}

void SnacIcqICBM::sendSMS(SendMsg msg)
{
	smsQueue.push_back(msg);
	m_client->processSMSQueue();
}

void SnacIcqICBM::pluginInfoRequest(unsigned long uin, unsigned plugin_index)
{
	list<SendMsg>::iterator it;
	for (it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it){
		SendMsg &s = *it;
		if((s.screen.toULong() == uin) && (s.flags == plugin_index) && (s.msg == NULL))
			break;
	}
	if (it != sendBgQueue.end())
		return;
	SendMsg s;
	s.screen = QString::number(uin);
	s.flags  = plugin_index;
	sendBgQueue.push_back(s);
	processSendQueue();
}

bool SnacIcqICBM::process(unsigned short subtype, ICQBuffer* buf, unsigned short seq)
{
    switch (subtype){
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
        log(L_DEBUG, "Message rights granted");
        break;
    case ICQ_SNACxMSG_MTN:{
            buf->incReadPos(10);
            QString screen = buf->unpackScreen();
            unsigned short type; //Fixme!!! Local declaration of 'type' hides declaration of the same name in outer scope: Function parameter "type" 
            *buf >> type;
            bool bType = (type > 1);
            Contact *contact;
            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
            if (data == NULL)
                break;
            if (data->bTyping.toBool() == bType)
                break;
            data->bTyping.asBool() = bType;
            EventContact e(contact, EventContact::eStatus);;
            e.process();
            break;
        }
    case ICQ_SNACxMSG_ERROR:{
            if(seq == 0)
                break;
            unsigned short error;
            *buf >> error;
            QString err_str = I18N_NOOP("Unknown error");
            if ((error == 0x0009) &&
					((m_send.msg == NULL) || (m_send.msg->type() != MessageContacts))){
                err_str = I18N_NOOP("Not supported by client");
                Contact *contact;
                ICQUserData *data = m_client->findContact(m_send.screen, NULL, false, contact);
                if (data){
                    list<SendMsg>::iterator it;
                    for (it = sendFgQueue.begin(); it != sendFgQueue.end();){
                        if (it->screen != m_send.screen){
                            ++it;
                            continue;
                        }
                        if (it->msg){
                            it->flags = 0;
                            ++it;
                            continue;
                        }
                        sendFgQueue.erase(it);
                        it = sendFgQueue.begin();
                    }
                    for (it = sendBgQueue.begin(); it != sendBgQueue.end();){
                        if (it->screen != m_send.screen){
                            ++it;
                            continue;
                        }
                        if (it->msg){
                            it->flags = 0;
                            ++it;
                            continue;
                        }
                        sendBgQueue.erase(it);
                        it = sendBgQueue.begin();
                    }
                    data->bBadClient.asBool() = true;
                    if (m_send.msg)
                        sendThruServer(m_send.msg, data);
                    m_send.msg    = NULL;
                    m_send.screen = QString::null;
                    m_sendTimer->stop();
                    processSendQueue();
                    break;
                }
            }else{
                err_str = m_client->error_message(error);
            }
            if(error == 2)
                m_client->snacService()->requestRateInfo();
            if (m_send.msg){
                m_send.msg->setError(err_str);
                EventMessageSent(m_send.msg).process();
                delete m_send.msg;
            }
            m_send.msg    = NULL;
            m_send.screen = QString::null;
            m_sendTimer->stop();
            processSendQueue();
            break;
        }
    case ICQ_SNACxMSG_SRV_MISSED_MSG: {
            unsigned short mFormat; // missed channel
            QString screen;         // screen
            unsigned short wrnLevel;// warning level
            unsigned short nTlv;    // number of tlvs
            TlvList  lTlv;          // all tlvs in message
            unsigned short missed;  // number of missed messages
            unsigned short error;   // error reason
            socket()->readBuffer() >> mFormat;
            screen = socket()->readBuffer().unpackScreen();
            socket()->readBuffer() >> wrnLevel;
            socket()->readBuffer() >> nTlv;
            for(unsigned i = 0; i < nTlv; i++) {
                unsigned short num;
                unsigned short size;
                const char*    data;
                *buf >> num >> size;
                data = buf->data(socket()->readBuffer().readPos());
                Tlv* tlv = new Tlv(num,size,data);
                lTlv += tlv;
            }
            *buf >> missed >> error;
            const char *err_str = NULL;
            switch (error) {
            case 0x00:
                err_str = I18N_NOOP("Invalid message");
                break;
            case 0x01:
                err_str = I18N_NOOP("Message was too large");
                break;
            case 0x02:
                err_str = I18N_NOOP("Message rate exceeded");
                break;
            case 0x03:
                err_str = I18N_NOOP("Sender too evil");
                break;
            case 0x04:
                err_str = I18N_NOOP("We are to evil :(");
                break;
            default:
                err_str = I18N_NOOP("Unknown error");
            }
            log(L_DEBUG, "ICMB error %u (%s) - screen(%s)", error, err_str, qPrintable(screen));
            break;
        }
    case ICQ_SNACxMSG_BLAMExSRVxACK:
        if((m_send.id.id_l == seq) && m_send.msg)
		{
            unsigned short oldLevel, newLevel;
            *buf >> oldLevel >> newLevel;
            WarningMessage *msg = static_cast<WarningMessage*>(m_send.msg);
            msg->setOldLevel((unsigned short)(newLevel - oldLevel));
            msg->setNewLevel(newLevel);
            ackMessage(m_send);
        }
        break;
    case ICQ_SNACxMSG_ACK:
        {
            MessageId id;
            *buf >> id.id_l >> id.id_h;
            buf->incReadPos(2);
            QString screen = buf->unpackScreen();
            bool bAck = false;
            if (m_send.id == id){
                if(screen.toLower() == m_send.screen.toLower())
                    bAck = true;
            }
            if (bAck){
                log(L_DEBUG, "Ack: %lu %lu (%s)", m_send.id.id_h, m_send.id.id_l, qPrintable(m_send.screen));
                if (m_send.msg){
                    Contact *contact;
                    ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
                    if (((data == NULL) ||
								(data->Status.toULong() == ICQ_STATUS_OFFLINE) ||
								(m_client->getAckMode() == 1)) &&
                            (m_send.msg->type() != MessageFile)){
                        m_sendTimer->stop();
                        ackMessage(m_send);
                        return true;
                    }else{
                        replyQueue.push_back(m_send);
                    }
                }else{
                    replyQueue.push_back(m_send);
                }
            }
            m_send.msg    = NULL;
            m_send.screen = QString::null;
            m_sendTimer->stop();
            processSendQueue();
            break;
        }
    case ICQ_SNACxMSG_AUTOREPLY:{
            MessageId id;
            unsigned short len, channel, reason, version;
            *buf >> id.id_l >> id.id_h;
            buf->unpack(channel);
            if (channel == 1) {
                log(L_DEBUG,"Please send paket to developer!");
                return true;
            }
            QString screen = buf->unpackScreen();
            buf->unpack(reason);
            buf->unpack(len);
            buf->unpack(version);
            plugin p;
            buf->unpack((char*)p, sizeof(p));
            buf->incReadPos(len - sizeof(plugin) + 2);
            buf->unpack(len);
            buf->incReadPos(len + 12);
            unsigned short ackFlags, msgFlags;
            buf->unpack(ackFlags);
            buf->unpack(msgFlags);

            list<SendMsg>::iterator it;
            for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                SendMsg &s = *it;
                if ((s.id == id) && (s.screen == screen))
                    break;
            }
            if (it == replyQueue.end())
                break;

            unsigned plugin_type = it->flags;
            if (plugin_type == PLUGIN_AIM_FT_ACK){
                m_client->m_processMsg.push_back(it->msg);
                replyQueue.erase(it);
                break;
            }
            if (it->msg){
                QByteArray answer;
                socket()->readBuffer() >> answer;
                if (ackMessage(it->msg, ackFlags, answer)){
                    ackMessage(*it);
                }else{
                    EventMessageSent(it->msg).process();
                    delete it->msg;
                }
                replyQueue.erase(it);
                break;
            }

            replyQueue.erase(it);
            Contact *contact;
            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);

            if (memcmp(p, m_client->plugins[PLUGIN_NULL], sizeof(plugin))){
                unsigned plugin_index;
                for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++){
                    if (memcmp(p, m_client->plugins[plugin_index], sizeof(plugin)) == 0)
                        break;
                }
                if (plugin_index == PLUGIN_NULL){
                    QString plugin_str;
                    unsigned i;
                    for (i = 0; i < sizeof(plugin); i++){
                        char b[4];
                        sprintf(b, "%02X ", p[i]);
                        plugin_str += b;
                    }
                    log(L_WARN, "Unknown plugin sign in reply %s", qPrintable(plugin_str));
                    break;
                }
                if ((data == NULL) && (plugin_index != PLUGIN_RANDOMxCHAT))
                    break;
                m_client->parsePluginPacket(socket()->readBuffer(), plugin_type, data, screen.toULong(), false);
                break;
            }

            if (plugin_type == PLUGIN_AR){
                QByteArray answer;
                *buf >> answer;
                log(L_DEBUG, "Autoreply from %s %s", qPrintable(screen), answer.data());
                Contact *contact; //Fixme: Local declaration of 'contact' hides declaration of the same name in outer scope, see previous declaration at line '300'
                ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
                if (data && data->AutoReply.setStr(getContacts()->toUnicode(contact, answer))){
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                }
            }
            break;
        }
    case ICQ_SNACxMSG_SERVERxMESSAGE:{
            MessageId id;
            *buf >> id.id_l >> id.id_h;
            unsigned short mFormat;
            *buf >> mFormat;
            QString screen = buf->unpackScreen();
            log(L_DEBUG, "Message from %s [%04X]", qPrintable(screen), mFormat);
            unsigned short level, nTLV;
            *buf >> level >> nTLV;
            TlvList tlvFixed(*buf, nTLV);
            TlvList tlvChannel(*buf);
            switch (mFormat){
            case 0x0001:{
                    if (!tlvChannel(2)){
                        log(L_WARN, "TLV 0x0002 not found");
                        break;
                    }
                    ICQBuffer m(*tlvChannel(2));
                    TlvList tlv_msg(m);
                    Tlv *m_tlv = tlv_msg(0x101);
                    if (m_tlv == NULL){
                        log(L_WARN, "TLV 0x0101 not found");
                        break;
                    }
                    if (m_tlv->Size() <= 4){
                        log(L_WARN, "Bad TLV 0x0101 size (%d)",m_tlv->Size());
                        break;
                    }
                    char *m_data = (*m_tlv);
                    unsigned short encoding = (unsigned short)((m_data[0] << 8) + m_data[1]);
                    unsigned short codepage = (unsigned short)((m_data[2] << 8) + m_data[3]);
                    m_data += 4;
                    QString text;
                    switch( encoding ) {
                        case 0 : { // ASCII
                            QTextCodec *pCodec = ContactList::getCodecByCodePage(codepage);
                            if( NULL != pCodec ) {
                                text = pCodec->toUnicode( m_data, m_tlv->Size() - 4 );
                            }
                            else {
                                text = QString::fromAscii( m_data, m_tlv->Size() - 4 );
                            }
                            break;
                        }
                        case 2 : { // Unicode
                            QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");
                            Q_ASSERT(codec);
                            text = codec->toUnicode( m_data, m_tlv->Size() - 4 );
                            break;
                        }
                        case 3 : { // Latin_1
                            text = QString::fromLatin1( m_data, m_tlv->Size() - 4 );
                            break;
                        }
                    }

                    Message *msg = new Message(MessageGeneric);
                    if (screen.toULong()){
                        msg->setText(text);
                    }else{
                        unsigned bgColor = m_client->clearTags(text);
                        msg->setText(text);
                        msg->setBackground(bgColor);
                        msg->setFlags(MESSAGE_RICHTEXT);
                    }
                    log(L_DEBUG, "Message %s", qPrintable(text));
                    m_client->messageReceived(msg, screen);
                    break;
                }
            case 0x0002:{
                    Tlv *tlv5 = tlvChannel(5);
                    if(!tlv5)
					{
                        log(L_WARN, "TLV 0x0005 not found");
                        break;
                    }
                    ICQBuffer msg(*tlv5);
					unsigned short type; //Fixme: Local declaration of 'type' hides declaration of the same name in outer scope, see previous declaration at line '73'
                    msg >> type;
                    switch (type){
                    case 0:
                        parseAdvancedMessage(screen, msg, tlvChannel(3) != NULL, id);
                        break;
                    case 1:
						{
							Contact *contact;
							ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
							if (data){
								QString name = m_client->dataName(data);
								for (list<Message*>::iterator it = m_client->m_acceptMsg.begin(); it != m_client->m_acceptMsg.end(); ++it){
									Message *msg = *it; //Fixme: Local declaration of 'msg' hides declaration of the same name in outer scope, see previous declaration at line '413'
									if (!msg->client().isEmpty() && (name == msg->client())){
										MessageId msg_id;
										switch (msg->type()){
											case MessageICQFile:
												msg_id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
												msg_id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
												break;
											case MessageFile:
												msg_id.id_l = static_cast<AIMFileMessage*>(msg)->getID_L();
												msg_id.id_h = static_cast<AIMFileMessage*>(msg)->getID_H();
												break;
										}
										if (msg_id == id)
										{
											m_client->m_acceptMsg.erase(it);
											EventMessageDeleted(msg).process();
											delete msg;
											break;
										}
									}
								}
							}
							break;
						}
                    case 2:
                        log(L_DEBUG, "File ack");
                        break;
                    default:
                        log(L_WARN, "Unknown type: 0x%04X", type);
                    }
                    break;
                }
            case 0x0004:{
                    Tlv *tlv5 = tlvChannel(5);
                    if (!tlv5){
                        log(L_WARN, "TLV 0x0005 not found");
                        break;
                    }
                    ICQBuffer msg(*tlv5);
                    unsigned long msg_uin;
                    msg >> msg_uin;
                    if (msg_uin == 0){
                        parseAdvancedMessage(screen, msg, tlvChannel(6) != NULL, id);
                        return true;
                    }
					unsigned char type, flags; //Fixme: Local declaration of 'type' hides declaration of the same name in outer scope, see previous declaration at line '73'
                    QByteArray msg_str;
                    msg >> type;
                    msg >> flags;
                    msg >> msg_str;
                    Message *m = m_client->parseMessage(type, screen, msg_str, msg, id, 0);
                    if (m)
                        m_client->messageReceived(m, screen);
                    break;
                }
            default:
                log(L_WARN, "Unknown message format %04X", mFormat);
            }
            break;
        }
    default:
		break;
    }
	return true;
}

void SnacIcqICBM::parseAdvancedMessage(const QString &screen, ICQBuffer &m, bool needAck, MessageId id)
{
    m.incReadPos(8);    /* msg-id cookie */
    capability cap;
    m.unpack((char*)cap, sizeof(cap));
    if (!memcmp(cap, m_client->capabilities[CAP_DIRECT], sizeof(cap)))
	{
        TlvList tlv(m);
        if(!tlv(0x2711))
		{
            log(L_DEBUG, "TLV 0x2711 not found");
            return;
        }
        unsigned long req_uin;
        unsigned long localIP;
        unsigned long localPort;
        unsigned long remotePort;
        unsigned long localPort1;
        char mode;
        ICQBuffer adv(*tlv(0x2711));
        adv.unpack(req_uin);
        adv.unpack(localIP);
        adv.unpack(localPort);
        adv.unpack(mode);
        adv.unpack(remotePort);
        adv.unpack(localPort1);
        if (req_uin != screen.toULong()){
            log(L_WARN, "Bad UIN in reverse direct request");
            return;
        }
        Contact *contact;
        ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
        if ((data == NULL) || contact->getIgnore()){
            log(L_DEBUG, "Reverse direct request from unknown user");
            return;
        }
        if (get_ip(data->RealIP) == 0)
            set_ip(&data->RealIP, localIP);
        for (list<Message*>::iterator it = m_client->m_processMsg.begin(); it != m_client->m_processMsg.end(); ++it){
            if ((*it)->type() != MessageICQFile)
                continue;
            ICQFileMessage *msg = static_cast<ICQFileMessage*>(*it);
            if (msg->m_transfer == NULL)
                continue;
        }
        log(L_DEBUG, "Setup reverse connect to %s %s:%lu",
            qPrintable(screen), qPrintable(QHostAddress(localIP).toString()), localPort);
        DirectClient *direct = new DirectClient(data, m_client);
        m_client->m_sockets.push_back(direct);
        direct->reverseConnect(localIP, localPort);
        return;
    }

    TlvList tlv(m);
    unsigned long real_ip = 0;
    unsigned long ip = 0;
    unsigned short port = 0;
	unsigned long test_ip = 0;

	if (tlv(2))
		test_ip = ((uint32_t)(*tlv(2)));
    if (tlv(3))
        real_ip = ((uint32_t)(*tlv(3)));
    if (tlv(4))
        ip = ((uint32_t)(*tlv(4)));
    if (tlv(5))
        port = (*tlv(5));

        if(tlv(13)) {
           // lang?
        }
        if(tlv(14)) {
           // codepage?
        }

        log(L_DEBUG, "Test IP: %08x, Real IP: %08x, IP: %08x, PORT: %d",(unsigned int)test_ip, (unsigned int)real_ip, (unsigned int)ip, port);

    if(real_ip || ip)
	{
        Contact *contact;
        ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
        if (data)
		{
            //if(real_ip && (get_ip(data->RealIP) == 0))
            if(real_ip)
                set_ip(&data->RealIP, real_ip);
            if(ip && (get_ip(data->IP) == 0))
                set_ip(&data->IP, ip);
            if(port && (data->Port.toULong() == 0))
                data->Port.asULong() = port;
        }
    }

    if (!memcmp(cap, m_client->capabilities[CAP_AIM_IMIMAGE], sizeof(cap))){
        log(L_DEBUG, "AIM set direct connection");
        return;
    }

    if (!memcmp(cap, m_client->capabilities[CAP_AIM_SENDFILE], sizeof(cap)))
	{
		sendFile(tlv, test_ip, ip, port, screen, id);
		return;
    }

    if (!memcmp(cap, m_client->capabilities[CAP_AIM_BUDDYLIST], sizeof(cap)))
	{
        log(L_DEBUG, "AIM buddies list");
        if (!tlv(0x2711)){
            log(L_WARN, "No body in ICMB message found");
            return;
        }
        ICQBuffer adv(*tlv(0x2711));
        QString contacts;
        while (adv.readPos() < (unsigned)adv.size()){
            QString grp;
            adv.unpackStr(grp);
            unsigned short nBuddies;
            adv >> nBuddies;
            for (unsigned short i = 0; i < nBuddies; i++){
                QString s;
                adv.unpackStr(s);
                if (!contacts.isEmpty())
                    contacts += ';';
                if (s.toULong()){
                    contacts += "icq:";
                    contacts += s;
                    contacts += ",ICQ ";
                    contacts += s;
                }else{
                    contacts += "aim:";
                    contacts += s;
                    contacts += ",AIM ";
                    contacts += s;
                }
            }
        }
        m_client->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
        socket()->writeBuffer() << id.id_l << id.id_h << 0x0002;
        socket()->writeBuffer().packScreen(screen);
        socket()->writeBuffer() << 0x0003 << 0x0002 << 0x0002;
        m_client->sendPacket(false);
        ContactsMessage *msg = new ContactsMessage;
        msg->setContacts(contacts);
        m_client->messageReceived(msg, screen);
        return;
    }

    if (memcmp(cap, m_client->capabilities[CAP_SRV_RELAY], sizeof(cap))){
        QString s;
        for (unsigned i = 0; i < sizeof(cap); i++){
            char b[5];
            sprintf(b, "0x%02X ", cap[i] & 0xFF);
            s += b;
        }
        log(L_DEBUG, "Unknown capability in advanced message\n%s", qPrintable(s));
        return;
    }

    if (!tlv(0x2711)){
        log(L_WARN, "No body in ICMB message found");
        return;
    }

    ICQBuffer adv(*tlv(0x2711));
    unsigned short len;
    unsigned short tcp_version;
    plugin p;

    adv.unpack(len);
    adv.unpack(tcp_version);
    adv.unpack((char*)p, sizeof(p));
    adv.incReadPos(len - sizeof(p) - 4);

    unsigned short cookie1;
    unsigned short cookie2;
    unsigned short cookie3;
    adv.unpack(cookie1);
    adv.unpack(cookie2);
    adv.unpack(cookie3);
    if ((cookie1 != cookie3) && (cookie1 + 1 != cookie3)){
        log(L_WARN, "Bad cookie in TLV 2711 (%X %X %X)", cookie1, cookie2, cookie3);
        return;
    }
    adv.unpack(len);
    adv.incReadPos(len + 10);

    if (memcmp(p, m_client->plugins[PLUGIN_NULL], sizeof(p))){
        unsigned plugin_index;
        for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
            if (memcmp(p, m_client->plugins[plugin_index], sizeof(p)) == 0)
                break;
        if (plugin_index >= PLUGIN_NULL){
            QString sign;
            unsigned int i;
            for (i = 0; i < sizeof(p); i++) {
                char temp[8];
                int  value = p[i];

                sprintf(temp,"%02X",value);
                sign += QString(temp);
            }
            if (sign.length())
                log(L_WARN, "Unknown plugin sign %s",qPrintable(sign));
            return;
        }
        switch (plugin_index){
        case PLUGIN_INFOxMANAGER:
        case PLUGIN_STATUSxMANAGER:
            break;
        default:
            log(L_WARN, "Unsupported plugin request %u", plugin_index);
            return;
        }
        char type;
        adv.unpack(type);
        if (type != 1){
            log(L_WARN, "Unknown type plugin request %u", type);
            return;
        }
        adv.incReadPos(8);
        plugin p; //Fixme: Local declaration of 'p' hides declaration of the same name in outer scope: previous declaration at line '1176'
        adv.unpack((char*)p, sizeof(p));
        unsigned plugin_type;
        for (plugin_type = 0; plugin_type < PLUGIN_NULL; plugin_type++){
            if (memcmp(p, m_client->plugins[plugin_type], sizeof(p)) == 0)
                break;
        }
        if (plugin_type >= PLUGIN_NULL){
            log(L_WARN, "Unknown plugin request");
            return;
        }
        ICQBuffer info;
        m_client->pluginAnswer(plugin_type, screen.toULong(), info);
        sendAutoReply(screen, id, m_client->plugins[plugin_index],
                      cookie1, cookie2, 0, 0, 0x0200, QString::null, 1, info);
        return;
    }

    unsigned short msgType;
    unsigned short msgFlags;
    unsigned short msgState;
    adv.unpack(msgType);
    adv.unpack(msgState);
    adv.unpack(msgFlags);
    QByteArray msg;
    adv >> msg;

    switch (msgType){
    case ICQ_MSGxAR_AWAY:
    case ICQ_MSGxAR_OCCUPIED:
    case ICQ_MSGxAR_NA:
    case ICQ_MSGxAR_DND:
    case ICQ_MSGxAR_FFC:{
            unsigned req_status = STATUS_AWAY;
            switch (msgType){
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
            Contact *contact;
            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
            if (data == NULL)
                return;
            if ((m_client->getInvisible() && (data->VisibleId.toULong() == 0)) ||
                    (!m_client->getInvisible() && data->InvisibleId.toULong()))
                return;
            ar_request req;
            req.screen  = screen;
            req.type    = msgType;
            req.ack     = 0;
            req.id      = id;
            req.id1     = cookie1;
            req.id2     = cookie2;
            req.bDirect = false;
            m_client->arRequests.push_back(req);

            ARRequest ar;
            ar.contact  = contact;
            ar.param    = &m_client->arRequests.back();
            ar.receiver = m_client;
            ar.status   = req_status;
            EventARRequest(&ar).process();

            if (!msg.isEmpty()){
                Contact *contact; //Fixme: Local declaration of 'contact' hides declaration of the same name in outer scope, see previous declaration at line '1278'
                ICQUserData *data = m_client->findContact(screen, NULL, false, contact); //Fixme: Local declaration of 'data' hides declaration of the same name in outer scope, see previous declaration at line '1279'
                QString m = getContacts()->toUnicode(contact, msg); //Fixme: Local declaration of 'm' hides declaration of the same name in outer scope. For additional information, see previous declaration at line '1006'
                data->AutoReply.str() = m;
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }
            return;
        }
    }
    ICQBuffer copy;
    if (!msg.isEmpty() || (msgType == ICQ_MSGxEXT)){
        if (adv.readPos() < adv.writePos())
            copy.pack(adv.data(adv.readPos()), adv.writePos() - adv.readPos());
        log(L_DEBUG, "Msg size=%lu type=%u", (unsigned long) msg.size(), msgType);
        if (msg.size() || (msgType == ICQ_MSGxEXT)){
            Message *m = m_client->parseMessage(msgType, screen, msg, adv, id, cookie1 | (cookie2 << 16)); //Fixme: Local declaration of 'm' hides declaration of the same name in outer scope. For additional information, see previous declaration at line '1006'
            if (m){
                if ((m_send.id == id) && (m_send.screen == screen)){
                    replyQueue.push_back(m_send);
                    m_send.msg    = NULL;
                    m_send.screen = "";
                    m_sendTimer->stop();
                    processSendQueue();
                    return;
                }
                list<SendMsg>::iterator it;
                for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                    SendMsg &s = *it;
                    log(L_DEBUG, "%lu %lu (%s) - %lu %lu (%s)",
                        s.id.id_h, s.id.id_l, qPrintable(s.screen),
                        id.id_h, id.id_l, qPrintable(screen));
                    if ((s.id == id) && (s.screen == screen))
                        break;
                }
                if (it == replyQueue.end()){
                    bool bAccept = true;
                    unsigned short ackFlags = 0;
                    if (m->type() != MessageICQFile){
                        if (m->type() == MessageStatus){
                            bAccept = false;
                        }else{
                            switch (m_client->getStatus()){
                            case STATUS_DND:
                                if (m_client->getAcceptInDND())
                                    break;
                                ackFlags = ICQ_TCPxACK_DND;
                                bAccept = false;
                                break;
                            case STATUS_OCCUPIED:
                                if (m_client->getAcceptInOccupied())
                                    break;
                                ackFlags = ICQ_TCPxACK_OCCUPIED;
                                bAccept = false;
                                break;
                            }
                            if (msgFlags & (ICQ_TCPxMSG_URGENT | ICQ_TCPxMSG_LIST))
                                bAccept = true;
                        }
                        if (!bAccept){
                            Contact *contact;
                            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
                            if (data == NULL)
                                return;

                            ar_request req;
                            req.screen  = screen;
                            req.type    = msgType;
                            req.ack		= ackFlags;
                            req.id      = id;
                            req.id1     = cookie1;
                            req.id2     = cookie2;
                            req.bDirect = false;
                            m_client->arRequests.push_back(req);

                            ARRequest ar;
                            ar.contact  = contact;
                            ar.param    = &m_client->arRequests.back();
                            ar.receiver = m_client;
                            ar.status   = m_client->getStatus();
                            EventARRequest(&ar).process();
                            return;
                        }
                    }
                    if (msgFlags & ICQ_TCPxMSG_URGENT)
                        m->setFlags(m->getFlags() | MESSAGE_URGENT);
                    if (msgFlags & ICQ_TCPxMSG_LIST)
                        m->setFlags(m->getFlags() | MESSAGE_LIST);
                    needAck = m_client->messageReceived(m, screen);
                }
				else
				{
                    Message *msg = it->msg; //Fixme: Local declaration of 'msg' hides declaration of the same name in outer scope, see previous declaration at line '1254'
                    replyQueue.erase(it);
                    if(msg->type() == MessageFile)
					{
                        Contact *contact;
                        ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
                        if ((m->type() != MessageICQFile) || (data == NULL)){
                            log(L_WARN, "Bad answer type");
                            msg->setError(I18N_NOOP("Send failed"));
                            EventMessageSent(msg).process();
                            delete msg;
                            return;
                        }
                        if(m_client->getState() == 1)
						{
                            msg->setError(I18N_NOOP("Message declined"));
                            EventMessageSent(msg).process();
                            delete msg;
                            return;
                        }
                    }
					else
					{
                        log(L_WARN, "Unknown message type for ACK");
                        delete msg;
                    }
                }
            }
        }
    }
    if (!needAck)
        return;
    sendAutoReply(screen, id, p, cookie1, cookie2,
                  msgType, 0, 0, QString::null, 0, copy); //variable p seems corrupted, maybe crash?
}

class AIMParser : public HTMLParser
{
public:
    AIMParser() {}
    QString parse(const QString &str);
protected:
    bool bPara;
    QString res;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

QString AIMParser::parse(const QString &str)
{
    bPara = false;
    res = QString::null;
    HTMLParser::parse(str);
    return res;
}

void AIMParser::text(const QString &text)
{
    if (text.isEmpty())
        return;
    bPara = true;
    res += text;
}

void AIMParser::tag_start(const QString &tag, const list<QString> &options)
{
	QString dummytag=tag; //remove this cramp later
	list<QString>::const_iterator dummyoptions = options.begin();//remove this cramp later
    /*QString otag;
    QString add;
    if (tag == "br")
        otag = "BR";
    if (tag == "p"){
        if (!bPara)
            return;
        otag = "BR";
    }
    if ((tag == "font") || (tag == "b") || (tag == "u") || (tag == "i"))
        otag = tag.upper();
    if (tag == "span")
        otag = "FONT";
    if (otag.isEmpty())
        return;
    res += '<';
    res += otag;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        QString value = *it;
        if (key.lower() == "style"){
            list<QString> styles = parseStyle(value);
            for (list<QString>::const_iterator its = styles.begin(); its != styles.end(); ++its){
                QString key = *its;
                ++its;
                QString value = *its;
                if (key == "font-family")
                {
                    res += " FACE=\"";
                    res += value;
                    res += '\"';
                }
                else if (key == "font-size")
                {
                    if (value == "smaller")
                    {
                        res += " SIZE=2";
                    }
                    else if (value == "larger")
                    {
                        res += " SIZE=4";
                    }
                }
                else if (key == "font-style")
                {
                    if (value.lower() == "italic")
                        add = "<i>";
                }
                else if (key == "font-weight")
                {
                    if (value.toInt() >= 600)
                        add = "<b>";
                }
                else if (key == "text-decoration")
                {
                    if (value.lower() == "underline")
                        add = "u";
                }
                else if (key == "color")
                {
                    res += " COLOR=\"";
                    res += value;
                    res += '\"';
                }
            }
            continue;
        }
        res += ' ';
        res += key.upper();
        res += "=\"";
        res += value;
        res += '\"';
    }
    res += ">";*/
}

void AIMParser::tag_end(const QString &tag)
{
    if(tag == "p") {
        res += '\n';
    }
    /*QString otag;
    if ((tag == "font") || (tag == "b") || (tag == "u") || (tag == "i"))
        otag = tag.upper();
    if (tag == "span")
        otag = "FONT";
    if (otag.isEmpty())
        return;
    res += "</";
    res += otag;
    res += ">";*/
}

void SnacIcqICBM::processSendQueue()
{
    if (m_sendTimer->isActive()) //Crash here on change Profile, m_sendTimer is 0
        return;
    m_client->m_processTimer->stop();
    if (m_client->m_bNoSend)
        return;
    if (m_client->getState() != Client::Connected){
        m_sendTimer->stop();
        return;
    }
    unsigned delay = 0;
    unsigned send_delay = 0;
    if (m_client->m_bReady){
        while (!sendFgQueue.empty()){
            send_delay = m_client->delayTime(SNAC(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_SENDxSERVER));
            if (send_delay){
                delay = send_delay;
                break;
            }
            log(L_DEBUG, "Process fg queue");
            m_send = sendFgQueue.front();
            sendFgQueue.pop_front();
            m_sendTimer->start(SEND_TIMEOUT);
            if (processMsg())
                return;
            m_sendTimer->stop();
        }
    }
    send_delay = m_client->processSMSQueue();
    if (send_delay && (delay > send_delay))
        delay = send_delay;
    for (unsigned i = 0; i < m_client->m_rates.size(); i++){
        RateInfo &r = m_client->m_rates[i];
        for (;;){
            if (r.delayed.readPos() == r.delayed.writePos())
                break;
            send_delay = m_client->delayTime(r);
            if (send_delay){
                log(L_DEBUG, "Delay for group %d: %u", i, send_delay);
                m_client->m_processTimer->start(send_delay);
                return;
            }
            unsigned char *packet = (unsigned char*)(r.delayed.data(r.delayed.readPos()));
            unsigned size = (packet[4] << 8) + packet[5] + 6;
            ++m_client->m_nFlapSequence;
            packet[2] = (m_client->m_nFlapSequence >> 8);
            packet[3] = m_client->m_nFlapSequence;
            socket()->writeBuffer().packetStart();
            socket()->writeBuffer().pack(r.delayed.data(r.delayed.readPos()), size);
            EventLog::log_packet(socket()->writeBuffer(), true, ICQPlugin::icq_plugin->OscarPacket);
            r.delayed.incReadPos(size);
            m_client->setNewLevel(r);
            socket()->write();
        }
        if (r.delayed.readPos() == r.delayed.writePos())
            r.delayed.init(0);
    }
    send_delay = m_client->processInfoRequest();
    if (send_delay && (delay > send_delay))
        delay = send_delay;
    send_delay = m_client->processListRequest();
    if (send_delay && (delay > send_delay))
        delay = send_delay;
    if (m_client->m_bReady){
        while (!sendBgQueue.empty()){
            send_delay = m_client->delayTime(SNAC(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_SENDxSERVER));
            if (send_delay){
                if (send_delay < delay)
                    delay = send_delay;
                break;
            }
            m_send = sendBgQueue.front();
            sendBgQueue.pop_front();
            m_sendTimer->start(SEND_TIMEOUT);
            if (processMsg())
                return;
            m_sendTimer->stop();
        }
    }
    if (delay){
        log(L_DEBUG, "Delay: %u", delay);
        m_client->m_processTimer->start(delay);
    }
}

static QString getUtf8Part(QString &str, unsigned size)
{
    if ((unsigned) str.toUtf8().length() < size){
        QString res = str;
        str = QString::null;
        return res;
    }
    unsigned s = 0;
    int n;
    int wordStart = 0;
    bool bWord = false;
    for (n = 0; n < str.length(); n++)
	{
        QChar c = str[n];
        if (c.isSpace())
		{
            if (bWord)
			{
                unsigned word_size = str.mid(wordStart, n - wordStart).toUtf8().length();
                if (s + word_size > 0){
                    if (wordStart == 0)
					{
                        s = 0;
                        for (n = 0; n < str.length(); n++)
						{
                            unsigned char_size = str.mid(n, 1).toUtf8().length();
                            if (s + char_size > 0)
                                break;
                        }
                    }
                    break;
                }
                s += word_size;
                bWord = false;
            }
            unsigned char_size = str.mid(n, 1).toUtf8().length();
            if (s + char_size > 0)
                break;
            s += char_size;
        }
		else if (!bWord)
		{
            wordStart = n;
            bWord = true;
        }
    }
    QString res = str.left(n);
    str = str.mid(n);
    return res;
}

bool SnacIcqICBM::processMsg()
{
	log(L_DEBUG, "SnacIcqICBM::processMsg()");
    Contact *contact;
    ICQUserData *data = m_client->findContact(m_send.screen, NULL, false, contact);
    if ((data == NULL) && (m_send.flags != PLUGIN_RANDOMxCHAT)){
        if (m_send.msg != NULL)
        {
            m_send.msg->setError(I18N_NOOP("No contact"));
            EventMessageSent(m_send.msg).process();
            delete m_send.msg;
            m_send.msg = NULL;
        }
        m_send.screen = QString::null;
        return false;
    }
    unsigned short type = 0;
    if (m_send.msg)
	{
        type = m_send.msg->type();
        log(L_DEBUG, "Send: %s %u %X", qPrintable(m_send.screen), type, m_send.flags);
    }
    if(m_send.msg && (m_send.socket == NULL))
	{
        ICQBuffer b;
        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        switch (m_send.msg->type())
		{
        case MessageContacts:
            if (data->Uin.toULong() == 0){
                CONTACTS_MAP c;
                QString nc = m_client->packContacts(static_cast<ContactsMessage*>(m_send.msg), data, c);
                if (c.empty()){
                    m_send.msg->setError(I18N_NOOP("No contacts for send"));
                    EventMessageSent(m_send.msg).process();
                    delete m_send.msg;
                    m_send.msg = NULL;
                    m_send.screen = QString::null;
                    return false;
                }
                static_cast<ContactsMessage*>(m_send.msg)->setContacts(nc);
                ICQBuffer msgBuf;
                vector<alias_group> cc;
                for (CONTACTS_MAP::iterator it = c.begin(); it != c.end(); ++it){
                    alias_group c; //Fixme: Local declaration of 'c' hides declaration of the same name in outer scope, see previous declaration at line '1786'
                    c.alias = it->first.str();
                    c.grp   = it->second.grp;
                    cc.push_back(c);
                }
                sort(cc.begin(), cc.end());

                unsigned grp   = (unsigned)(-1);
                unsigned start = 0;
                unsigned short size = 0;
                unsigned i;
                for (i = 0; i < cc.size(); i++){
                    if (cc[i].grp != grp){
                        if (grp != (unsigned)(-1)){
                            QString s = "Not in list";
                            if (grp){
                                Group *group = getContacts()->group(grp);
                                if (group)
                                    s = group->getName();
                            }
                            msgBuf.pack(s);
                            msgBuf << size;
                            for (unsigned j = start; j < i; j++)
                                msgBuf.pack(cc[j].alias);
                        }
                        size  = 0;
                        start = i;
                        grp   = cc[i].grp;
                    }
                    size++;
                }
                QString s = "Not in list";
                if (grp){
                    Group *group = getContacts()->group(grp);
                    if (group)
                        s = group->getName();
                }
                msgBuf.pack(s);
                msgBuf << size;
                for (unsigned j = start; j < i; j++)
                    msgBuf.pack(cc[j].alias);
                m_send.id.id_l = rand();
                m_send.id.id_h = rand();
                sendType2(m_send.screen, msgBuf, m_send.id, CAP_AIM_BUDDYLIST, false, 0);
                return true;
            }
        case MessageUrl:{
                if (data->Uin.toULong() == 0)
                    break;
                m_client->packMessage(b, m_send.msg, data, type, false);
                QString err = m_send.msg->getError();
                if (!err.isEmpty()){
                    EventMessageSent(m_send.msg).process();
                    delete m_send.msg;
                    m_send.msg = NULL;
                    m_send.screen = QString::null;
                    return false;
                }
                sendThroughServer(m_client->screen(data), 4, b, m_send.id, true, false);
                if (data->Status.toULong() != ICQ_STATUS_OFFLINE)
                    m_sendTimer->stop();
                if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) || (m_client->getAckMode() == 0))
                    ackMessage(m_send);
                return true;
            }
        case MessageFile:
			{
				log(L_DEBUG, "processMsg: MessageFile");
				MessageId id;
				m_client->generateCookie(id);
				FileMessage* msg = static_cast<FileMessage*>(m_send.msg);
				AIMOutcomingFileTransfer *ft = new AIMOutcomingFileTransfer(msg, data, m_client);
				ft->setICBMCookie(id);
				ft->listen();
				QString filename = msg->getDescription();
				ft->setStage(1);
				//unsigned long filesize = msg->getSize(); //Fixme: filesize is initialized, but not used.
				ft->requestFT();
				return true;
			}
        case MessageWarning:{
                WarningMessage *msg = static_cast<WarningMessage*>(m_send.msg);
                m_client->snac(ICQ_SNACxFOOD_MESSAGE, ICQ_SNACxMSG_BLAMExUSER, true);
                m_send.id.id_l = m_client->m_nMsgSequence;
                unsigned short flag = 0;
                if (msg->getAnonymous())
                    flag = 1;
                socket()->writeBuffer() << flag;
                socket()->writeBuffer().packScreen(m_client->screen(data));
                m_client->sendPacket(false);
                return true;
            }
        }
        QByteArray text;
		log(L_DEBUG, "Alpha %x", m_send.flags);
        switch (m_send.flags & SEND_MASK){
        case SEND_RTF:
            text = m_client->createRTF(m_send.text, m_send.part, m_send.msg->getForeground(), contact, MAX_TYPE2_MESSAGE_SIZE);
            break;
        case SEND_UTF:
            m_send.part = getUtf8Part(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
            text = m_send.part.toUtf8();
            break;
        case SEND_TYPE2:{
                m_send.part = getPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                text = getContacts()->fromUnicode(contact, m_send.part);
                EventSend e(m_send.msg, text);
                e.process();
                text = e.localeText();
                break;
            }
        case SEND_HTML:
        case SEND_HTML_PLAIN:{
                QString t;
                unsigned max_size = MAX_TYPE2_MESSAGE_SIZE;
                bool bWide = false;
                for (int i = 0; i < (int)(m_send.text.length()); i++){
                    if (m_send.text[i].unicode() > 0x7F){
                        max_size = max_size / 2;
                        bWide = true;
                        break;
                    }
                }
                m_send.part = getPart(m_send.text, max_size);
                //char b[15];
                //sprintf(b, "%06X", (unsigned)(m_send.msg->getBackground() & 0xFFFFFF));
                //t += "<HTML><BODY BGCOLOR=\"#";
                //t += b;
                //t += "\">";
                if ((m_send.flags & SEND_MASK) == SEND_HTML){
                    AIMParser p;
                    t += p.parse(m_send.part);
                }else{
                    EventSend e(m_send.msg, m_send.part.toUtf8());
                    e.process();
                    m_send.part = QString::fromUtf8( e.localeText() );
                    t += quoteString(m_send.part);
                }
                //t += "</BODY></HTML>";
                sendType1(t, bWide, data);
                return true;
            }
        default:
            m_send.part = getPart(m_send.text, MAX_PLAIN_MESSAGE_SIZE);
            sendType1(m_send.part, false, data);
            return true;
        }

        ICQBuffer msgBuf;
        unsigned short size  = (unsigned short)(text.length() + 1);
        unsigned short flags = ICQ_TCPxMSG_NORMAL;
        if (m_send.msg->getFlags() & MESSAGE_URGENT)
            flags = ICQ_TCPxMSG_URGENT;
        if (m_send.msg->getFlags() & MESSAGE_LIST)
            flags = ICQ_TCPxMSG_LIST;
        msgBuf.pack((unsigned short)1);
        msgBuf.pack(m_client->msgStatus());
        msgBuf.pack(flags);
        msgBuf.pack(size);
        msgBuf.pack(text.data(), size);
        if (m_send.msg->getBackground() == m_send.msg->getForeground()){
            msgBuf << 0x00000000L << 0xFFFFFF00L;
        }else{
            msgBuf << (m_send.msg->getForeground() << 8) << (m_send.msg->getBackground() << 8);
        }
        if ((m_send.flags & SEND_MASK) != SEND_TYPE2){
            msgBuf << 0x26000000L;
            packCap(msgBuf, m_client->capabilities[((m_send.flags & SEND_MASK) == SEND_RTF) ? CAP_RTF : CAP_UTF]);
        }
        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        sendAdvMessage(m_send.screen, msgBuf, PLUGIN_NULL, m_send.id, true, false);
        return true;
    }
    if(m_send.socket)
	{
        ICQBuffer msgBuf;
        if(m_send.flags == PLUGIN_AIM_FT_ACK)
		{
			log(L_DEBUG, "This way");
            AIMFileMessage *msg = static_cast<AIMFileMessage*>(m_send.msg);
            m_send.id.id_l = msg->getID_L();
            m_send.id.id_h = msg->getID_H();
            ICQBuffer b;
            b << (unsigned short)0;
            b << m_send.id.id_l << m_send.id.id_h;
            b.pack((char*)m_client->capabilities[CAP_AIM_SENDFILE], sizeof(capability));
            b.tlv(0x0A, (unsigned short)2);
            b.tlv(0x03, (unsigned long)htonl(get_ip(m_client->data.owner.RealIP)));
            b.tlv(0x05, static_cast<AIMFileTransfer*>(msg->m_transfer)->remotePort());
            sendThroughServer(m_send.screen, 2, b, m_send.id, false, false);
            replyQueue.push_back(m_send);
            m_send.msg    = NULL;
            m_send.screen = QString::null;
            return false;
        }
        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        if(m_send.flags == PLUGIN_AIM_FT)
		{
            TlvList tlvs;
            tlvs += new Tlv(0x0E, 2, "en");
            char b[15];
            sprintf(b, "%06X", (unsigned)(m_send.msg->getBackground() & 0xFFFFFF));
            QString text = QString("<HTML><BODY BGCOLOR=\"#%1\">%2</BODY></HTML>")
                           .arg(b)
                           .arg(m_client->removeImages(m_send.msg->getRichText(), false));
            bool bWide = false;
            int i;
            for (i = 0; i < (int)(text.length()); i++){
                if (text[i].unicode() > 0x7F){
                    bWide = true;
                    break;
                }
            }
            QString charset = bWide ? "unicode-2-0" : "us-ascii";
            tlvs += new Tlv(0x0D, charset.length(), charset.toLatin1());
            QByteArray st;
            if (bWide){
                for (i = 0; i < (int)(text.length()); i++){
                    unsigned short s = text[i].unicode();
                    st += (char)((s >> 8) & 0xFF);
                    st += (char)(s & 0xFF);
                }
            }else{
                st = text.toUtf8();
            }
            tlvs += new Tlv(0x0C, st.length(), st.data());
            FileMessage *msg = static_cast<FileMessage*>(m_send.msg);
            FileMessage::Iterator it(*msg);
            msgBuf
            << (unsigned short)0x0001
            << (unsigned short)(it.count())
            << (unsigned long)(it.size());
            QString fname;
            if (it.count() == 1){
                fname = *(it[0]);
                fname = fname.replace('\\', '/');
                int n = fname.lastIndexOf('/');
                if (n >= 0)
                    fname = fname.mid(n + 1);
            }else{
                fname = QString::number(it.count());
                fname += " files";
            }
            bWide = false;
            for (i = 0; i < (int)(fname.length()); i++){
                if (fname[i].unicode() > 0x7F){
                    bWide = true;
                    break;
                }
            }
            charset = bWide ? "utf8" : "us-ascii";
            tlvs += new Tlv(0x2712, charset.length(), charset.toUtf8());
            msgBuf << (const char*)(fname.toUtf8()) << (char)0;
            sendType2(m_send.screen, msgBuf, m_send.id, CAP_AIM_SENDFILE, false, m_send.socket->localPort(), &tlvs);
            return true;
        }
        msgBuf.pack(m_client->data.owner.Uin.toULong());
        unsigned long ip = get_ip(m_client->data.owner.IP);
        if (ip == get_ip(m_send.socket->m_data->IP))
            ip = get_ip(m_client->data.owner.RealIP);
        msgBuf.pack(ip);
        msgBuf.pack((unsigned long)(m_send.socket->localPort()));
        msgBuf.pack((char)MODE_DIRECT);
        msgBuf.pack((unsigned long)(m_send.socket->remotePort()));
        msgBuf.pack(m_client->data.owner.Port.toULong());
        msgBuf.pack((unsigned short)8);
        msgBuf.pack((unsigned long)m_client->m_nMsgSequence);
        sendType2(m_send.screen, msgBuf, m_send.id, CAP_DIRECT, false, 0);
        return true;
    }
    if (m_send.flags == PLUGIN_AR){
        log(L_DEBUG, "Request auto response %s", qPrintable(m_send.screen));

        unsigned long status = data->Status.toULong();
        if ((status == ICQ_STATUS_ONLINE) || (status == ICQ_STATUS_OFFLINE))
            return false;

        unsigned short type = ICQ_MSGxAR_AWAY; //Fixme: Local declaration of 'type' hides declaration of the same name in outer scope. For additional information, see previous declaration at line '1771'
        if (status & ICQ_STATUS_DND){
            type = ICQ_MSGxAR_DND;
        }else if (status & ICQ_STATUS_OCCUPIED){
            type = ICQ_MSGxAR_OCCUPIED;
        }else if (status & ICQ_STATUS_NA){
            type = ICQ_MSGxAR_NA;
        }else if (status & ICQ_STATUS_FFC){
            type = ICQ_MSGxAR_FFC;
        }

        ICQBuffer msg;
        msg.pack(type);
        msg.pack((unsigned short)(m_client->fullStatus(m_client->m_status) & 0xFFFF));
        msg << 0x0100 << 0x0100 << (char)0;

        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        sendAdvMessage(m_client->screen(data), msg, PLUGIN_NULL, m_send.id, false, false);
        return true;
    }else if (m_send.flags == PLUGIN_RANDOMxCHAT){
        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        ICQBuffer b;
        b << (char)1 << 0x00000000L << 0x00010000L;
        sendAdvMessage(m_send.screen, b, PLUGIN_RANDOMxCHAT, m_send.id, false, false);
    }else{
        unsigned plugin_index = m_send.flags;
        log(L_DEBUG, "Plugin info request %s (%u)", qPrintable(m_send.screen), plugin_index);

        ICQBuffer b;
        unsigned short type = 0; //Fixme: Local declaration of 'type' hides declaration of the same name in outer scope. For additional information, see previous declaration at line '1771'
        switch (plugin_index){
        case PLUGIN_QUERYxINFO:
        case PLUGIN_PHONEBOOK:
        case PLUGIN_PICTURE:
            type = 2;
            break;
        }
        b.pack((unsigned short)1);
        b.pack((unsigned short)0);
        b.pack((unsigned short)2);
        b.pack((unsigned short)1);
        b.pack((char)0);
        b.pack((char*)m_client->plugins[plugin_index], sizeof(plugin));
        b.pack((unsigned long)0);

        m_send.id.id_l = rand();
        m_send.id.id_h = rand();
        sendAdvMessage(m_send.screen, b, type ? PLUGIN_INFOxMANAGER : PLUGIN_STATUSxMANAGER, m_send.id, false, false);
        return true;
    }
    return false;
}

static const plugin arrPlugins[] =
    {
        // PLUGIN_PHONExBOOK
        { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
          0xAC, 0x96, 0xAA, 0xB2, 0x00, 0x00 },
        // PLUGIN_PICTURE
        { 0x80, 0x66, 0x28, 0x83, 0x80, 0x28,
          0xD3, 0x11, 0x8D, 0xBB, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_FILExSERVER
        { 0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91,
          0xD3, 0x11, 0x8D, 0xD7, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x04, 0x00 },
        // PLUGIN_FOLLOWxME
        { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
          0xAC, 0x96, 0xAA, 0xB2, 0x02, 0x00 },
        // PLUGIN_ICQxPHONE
        { 0x3F, 0xB6, 0x5E, 0x38, 0xA0, 0x30,
          0xD4, 0x11, 0xBD, 0x0F, 0x00, 0x06,
          0x29, 0xEE, 0x4D, 0xA1, 0x00, 0x00 },
        // PLUGIN_QUERYxINFO
        { 0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71,
          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_QUERYxSTATUS
        { 0x10, 0x18, 0x06, 0x70, 0x54, 0x71,
          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_INFOxMANAGER
        { 0xA0, 0xE9, 0x3F, 0x37, 0x4F, 0xE9,
          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 },
        // PLUGIN_STATUSxMANAGER
        { 0x10, 0xCF, 0x40, 0xD1, 0x4F, 0xE9,
          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 },
        // PLUGIN_RANDOM_CHAT
        { 0x60, 0xF1, 0xA8, 0x3D, 0x91, 0x49,
          0xD3, 0x11, 0x8D, 0xBE, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_VIDEO_CHAT
        { 0x68, 0x33, 0x01, 0x6B, 0x0B, 0x7D,
          0x36, 0x4B, 0x98, 0x6C, 0x63, 0x72,
          0x01, 0x5E, 0x7C, 0x8E, 0x00, 0x00 },
        // PLUGIN_NULL
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        // PLUGIN_FILE
        { 0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91,
          0xD3, 0x11, 0x8D, 0xD7, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_CHAT
        { 0xBF, 0xF7, 0x20, 0xB2, 0x37, 0x8E,
          0xD4, 0x11, 0xBD, 0x28, 0x00, 0x04,
          0xAC, 0x96, 0xD9, 0x05, 0x00, 0x00 }
    };

plugin const *ICQClient::plugins = arrPlugins;

bool operator == (const MessageId &m1, const MessageId &m2)
{
    return ((m1.id_l == m2.id_l) && (m1.id_h == m2.id_h));
}
/*
#ifndef NO_MOC_INCLUDES
#include "icqicmb.moc"
#endif
*/
