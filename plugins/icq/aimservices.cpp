/***************************************************************************
                          services.cpp  -  description
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

#include "log.h"

#include "icqclient.h"

#include <map>

using namespace std;
using namespace SIM;

ServiceSocket::ServiceSocket(ICQClient *client, unsigned short id)
	: m_client( client )
    , m_id    ( id)
	, m_socket( NULL)
    , m_bConnected( false)

{
    m_client->m_snacService->addService(this);
}

ServiceSocket::~ServiceSocket()
{
	m_client->m_snacService->deleteService(this);
    delete m_socket;
}

void ServiceSocket::connect(const char *addr, unsigned short port, const QByteArray &cookie)
{
    log(L_DEBUG, "%s: connect to %s:%d ", serviceSocketName(), addr, port);
    m_cookie = cookie;
    if (m_socket != NULL)
	{
		m_socket->close();
		delete m_socket;
    }    
    m_socket = new ICQClientSocket(this);
    m_socket->connect(addr, port, m_client);
}

void ServiceSocket::close()
{
    log(L_DEBUG, "%s: close()", serviceSocketName());
    m_socket->close();
}

bool ServiceSocket::error_state(const QString &err, unsigned)
{
    log(L_DEBUG, "%s: Service error %s", serviceSocketName(), qPrintable(err));
    return true;
}

void ServiceSocket::connect_ready()
{
    log(L_DEBUG, "%s: connect_ready()", serviceSocketName());
    OscarSocket::connect_ready();
}

void ServiceSocket::packet_ready()
{
    log(L_DEBUG, "%s: packet_ready()", serviceSocketName());
    OscarSocket::packet_ready();
}

void ServiceSocket::packet(unsigned long size) //Fixme: Unsed Parameter size
{
    EventLog::log_packet(m_socket->readBuffer(), false,ICQPlugin::icq_plugin->OscarPacket);
    switch (m_nChannel)
	{
    case ICQ_CHNxNEW:
        flap(ICQ_CHNxNEW);
        m_socket->writeBuffer() << 0x00000001L;
        m_socket->writeBuffer().tlv(6, m_cookie.data(), (unsigned short)(m_cookie.size()));
        m_cookie.resize(0);
        sendPacket();
        break;
    case ICQ_CHNxDATA:
        unsigned short food, type;
        unsigned short flags, seq, cmd;
        m_socket->readBuffer() >> food >> type >> flags >> seq >> cmd;
		if (flags & 0x8000) 
		{	// some unknown data before real snac data
            // just read the length and forget it ;-)
            unsigned short unknown_length = 0;
            m_socket->readBuffer() >> unknown_length;
            m_socket->readBuffer().incReadPos(unknown_length);
        }
        // now just take a look at the type because 0x0001 == error
        // in all foodgroups
        if (type == 0x0001) 
		{
            unsigned short err_code;
            m_socket->readBuffer() >> err_code;
            log(L_DEBUG,"%s: Error! foodgroup: %04X reason", serviceSocketName(), food);
            // now decrease for icqicmb & icqvarious
            m_socket->readBuffer().decReadPos(sizeof(unsigned short));
        }
        data(food, type, seq);
        break;
    default:
        log(L_ERROR, "%s: Unknown channel %u", serviceSocketName(), m_nChannel & 0xFF);
    }
    m_socket->readBuffer().init(6);
    m_socket->readBuffer().packetStart();
    m_bHeader = true;
}

const unsigned short USER_DIRECTORY_SERVICE		= 0x000F;

const unsigned short USER_DIRECTORY_SEARCH	     = 0x0002;
const unsigned short USER_DIRECTORY_RESULT	     = 0x0003;

const unsigned short SNACxSRV_CLIENTxREADY		= 0x0002;
const unsigned short SNACxSRV_READYxSERVER		= 0x0003;
const unsigned short SNACxSRV_REQxRATExINFO		= 0x0006;
const unsigned short SNACxSRV_RATExINFO			= 0x0007;
const unsigned short SNACxSRV_RATExACK			= 0x0008;
const unsigned short SNACxSRV_I_AM_ICQ			= 0x0017;
const unsigned short SNACxSRV_ACK_ICQ			= 0x0018;

typedef QMap<unsigned short, QStringList> REQUEST_MAP;
typedef QMap<unsigned short, unsigned short> SEQ_MAP;

class SearchSocket : public ServiceSocket
{
public:
    SearchSocket(ICQClient*);
    unsigned short add(const QStringList &str);
protected:
    virtual const char *serviceSocketName() { return "SearchSocket"; }
    void data(unsigned short food, unsigned short type, unsigned short seq);
    void snac_service(unsigned short type);
    void snac_search(unsigned short type, unsigned short seq);
    void process();
    void addTlv(unsigned short n, const QString&, bool);
    REQUEST_MAP m_requests;
    SEQ_MAP		m_seq;
    unsigned short m_id;
};

SearchSocket::SearchSocket(ICQClient *client)
        : ServiceSocket(client, USER_DIRECTORY_SERVICE)
{
    m_id = 0;
}

static bool bLatin1(const QString &s)
{
    for (int i = 0; i < (int)(s.length()); i++)
	{
        if (s[i].unicode() > 0x7F)
            return false;
    }
    return true;
}

void SearchSocket::addTlv(unsigned short n, const QString &s, bool bLatin)
{
    QByteArray str;
	if (bLatin)
		str = s.toLatin1();
	else
		str = s.toUtf8();
    m_socket->writeBuffer().tlv(n, str.data());
}

void SearchSocket::process()
{
    if (!connected())
        return;
    for (REQUEST_MAP::iterator it = m_requests.begin(); it != m_requests.end(); ++it)
    {
        snac(USER_DIRECTORY_SERVICE, USER_DIRECTORY_SEARCH, true);
        bool bLatin;
        if (!it->count() == 0)
            continue;
        if (!it->count() == 1)
		{
            QStringList sl = (*it);
            QString mail = sl[0];
            bLatin = bLatin1(mail);
            m_socket->writeBuffer().tlv(0x1C, bLatin ? "us-ascii" : "utf8");
            m_socket->writeBuffer().tlv(0x0A, (unsigned short)1);
            addTlv(0x05, mail, bLatin);
        }
		else
		{
            QStringList sl = (*it);
            bLatin = bLatin1(sl[0]) &&
                     bLatin1(sl[1]) &&
                     bLatin1(sl[2]) &&
                     bLatin1(sl[3]) &&
                     bLatin1(sl[4]) &&
                     bLatin1(sl[5]) &&
                     bLatin1(sl[6]) &&
                     bLatin1(sl[7]) &&
                     bLatin1(sl[8]) &&
                     bLatin1(sl[9]);
            m_socket->writeBuffer().tlv(0x1C, bLatin ? "us-ascii" : "utf8");
            m_socket->writeBuffer().tlv(0x0A, (unsigned short)0);
            if (!sl[0].isEmpty())
                addTlv(0x01, sl[0], bLatin);
            if (!sl[1].isEmpty())
                addTlv(0x02, sl[1], bLatin);
            if (!sl[2].isEmpty())
                addTlv(0x03, sl[2], bLatin);
            if (!sl[3].isEmpty())
                addTlv(0x04, sl[3], bLatin);
            if (!sl[4].isEmpty())
                addTlv(0x06, sl[4], bLatin);
            if (!sl[5].isEmpty())
                addTlv(0x07, sl[5], bLatin);
            if (!sl[6].isEmpty())
                addTlv(0x08, sl[6], bLatin);
            if (!sl[7].isEmpty())
                addTlv(0x0C, sl[7], bLatin);
            if (!sl[8].isEmpty())
                addTlv(0x0D, sl[8], bLatin);
            if (!sl[9].isEmpty())
                addTlv(0x21, sl[9], bLatin);
        }
        sendPacket();
        m_seq.insert(m_nMsgSequence, it.key());
    }
    m_requests.clear();
}

unsigned short SearchSocket::add(const QStringList &name)
{
    m_requests.insert(++m_id, name);
    process();
    return m_id;
}

void SearchSocket::data(unsigned short food, unsigned short type, unsigned short seq)
{
    switch (food)
	{
    case ICQ_SNACxFOOD_SERVICE:
        snac_service(type);
        break;
    case USER_DIRECTORY_SERVICE:
        snac_search(type, seq);
        break;
    default:
        log(L_WARN, "Unknown foodgroup %04X", food);
    }
}

void SearchSocket::snac_service(unsigned short type)
{
    switch (type){
    case SNACxSRV_READYxSERVER:
        snac(ICQ_SNACxFOOD_SERVICE, SNACxSRV_I_AM_ICQ);
        m_socket->writeBuffer() << 0x00010004L << 0x000F0001L;
        sendPacket();
        break;
    case SNACxSRV_ACK_ICQ:
        snac(ICQ_SNACxFOOD_SERVICE, SNACxSRV_REQxRATExINFO);
        sendPacket();
        break;
    case SNACxSRV_RATExINFO:
        snac(ICQ_SNACxFOOD_SERVICE, SNACxSRV_RATExACK);
        m_socket->writeBuffer() << 0x00010002L << 0x00030004L << 0x0005;
        sendPacket();
        snac(ICQ_SNACxFOOD_SERVICE, SNACxSRV_CLIENTxREADY);
        m_socket->writeBuffer() << 0x00010003L << 0x00100739L << 0x000F0001L << 0x00100739L;
        sendPacket();
        m_bConnected = true;
        process();
        break;
    default:
        log(L_DEBUG, "Unknown service type %u", type);
    }
}

void SearchSocket::snac_search(unsigned short type, unsigned short seq)
{
    SEQ_MAP::iterator it;
    switch (type){
    case USER_DIRECTORY_RESULT:
        it = m_seq.find(seq);
		if (it == m_seq.end())
			log(L_WARN, "Bad sequence in search answer");
        else
		{
            unsigned short r;
            unsigned long nSearch;
            m_socket->readBuffer() >> r >> nSearch;

            SearchResult res;
            res.id = (*it);
            res.client = m_client;
            for (unsigned n = 0; n < nSearch; n++)
			{
                unsigned short nTlvs;
                m_socket->readBuffer() >> nTlvs;
                TlvList tlvs(m_socket->readBuffer(), nTlvs);
                Tlv *tlv = tlvs(0x09);
                if (tlv)
				{
                    load_data(ICQProtocol::icqUserData, &res.data, NULL);
                    res.data.setScreen(tlv->Data());    // utf8 ?
                    tlv = tlvs(0x01);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setFirstName(str);
                    }
                    tlv = tlvs(0x02);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setLastName(str);
                    }
                    tlv = tlvs(0x03);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
						res.data.setMiddleName(str);
                    }
                    tlv = tlvs(0x07);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setAddress(str);
                    }
                    tlv = tlvs(0x08);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setCity(str);
                    }
                    tlv = tlvs(0x0C);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setNick(str);
                    }
                    tlv = tlvs(0x07);
                    if (tlv)
					{
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.setState(str);
                    }
                    tlv = tlvs(0x06);
                    if (tlv)
					{
                        QString country_text;
                        country_text = QString::fromLatin1(tlv->Data());
                        country_text = country_text.toLower();
						for (const ext_info *info = getCountryCodes(); info->szName; ++info)
						{
							if (country_text != info->szName)
								continue;

                            res.data.setCountry(info->nCode);
							break;
						}
                    }
                    EventSearch(&res).process();
                    free_data(ICQProtocol::icqUserData, &res.data);
                }
            }
            if (r != 6)
			{
                load_data(ICQProtocol::icqUserData, &res.data, NULL);
                EventSearchDone(&res).process();
                free_data(ICQProtocol::icqUserData, &res.data);
                m_seq.erase(it);
            }
        }
        break;
    default:
        log(L_WARN, "Unknown search foodgroup type %04X", type);
    }
}

unsigned short ICQClient::aimEMailSearch(const QString &name)
{
    SearchSocket *s = static_cast<SearchSocket*>(m_snacService->getService(USER_DIRECTORY_SERVICE));
    if (s == NULL)
	{
        s = new SearchSocket(this);
        snacService()->requestService(s);
    }
    QStringList sl;
    sl.append(name);
    return s->add(sl);
}

unsigned short ICQClient::aimInfoSearch(const QString &first, const QString &last, const QString &middle,
                                        const QString &maiden, const QString &country, const QString &street,
                                        const QString &city, const QString &nick, const QString &zip,
                                        const QString &state)
{
    SearchSocket *s = static_cast<SearchSocket*>(m_snacService->getService(USER_DIRECTORY_SERVICE));
    if(s == NULL)
	{
        s = new SearchSocket(this);
        snacService()->requestService(s);
    }
    QStringList info;

    info.append(first);
    info.append(last);
    info.append(middle);
    info.append(maiden);
    info.append(country);
    info.append(street);
    info.append(city);
    info.append(nick);
    info.append(zip);
    info.append(state);
    return s->add(info);
}

