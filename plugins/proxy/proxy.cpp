/***************************************************************************
                          proxy.cpp  -  description
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

#include <QTabWidget>
#include <QObject>
#include <QByteArray>

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "fetch.h"
#include "log.h"
#include "misc.h"
#include "socket/socket.h"
#include "socket/serversocketnotify.h"
#include "socket/serversocket.h"
#include "socket/clientsocket.h"
#include "socket/tcpclient.h"
#include "socket/socketfactory.h"

#include "proxy.h"
#include "proxycfg.h"
#include "newprotocol.h"
#include "proxyerror.h"

using namespace std;
using namespace SIM;

#ifndef INADDR_NONE
#define INADDR_NONE	0xFFFFFFFF
#endif

static const char *CONNECT_ERROR = I18N_NOOP("Can't connect to proxy");
static const char *ANSWER_ERROR  = I18N_NOOP("Bad proxy answer");
static const char *AUTH_ERROR    = I18N_NOOP("Proxy authorization failed");
static const char *STATE_ERROR	 = "Connect in bad state";

static DataDef _proxyData[] =
    {
        { "Client", DATA_STRING, 1, 0 },
        { "Clients", DATA_STRLIST, 1, 0 },
        { "Type", DATA_ULONG, 1, DATA(0) },
        { "Host", DATA_STRING, 1, "proxy" },
        { "Port", DATA_ULONG, 1, DATA(1080) },
        { "Auth", DATA_BOOL, 1, 0 },
        { "User", DATA_STRING, 1, 0 },
        { "Password", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "NoShow", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

ProxyData::ProxyData()
{
    bInit = false;
    load_data(_proxyData, this, NULL);
}

ProxyData::ProxyData(const ProxyData &d)
{
    bInit = false;
    *this = d;
}

ProxyData::ProxyData(const char *cfg)
{
    bInit = false;
    if (cfg) {
        Buffer config;
        config << "[Title]\n" << cfg;
        config.setWritePos(0);
        config.getSection();
        load_data(_proxyData, this, &config);
        bInit = true;
    }else{
        load_data(_proxyData, this, NULL);
    }
}

ProxyData::~ProxyData()
{
    if (bInit)
        free_data(_proxyData, this);
}

bool ProxyData::operator == (const ProxyData &d) const
{
    if (Type.toULong() != d.Type.toULong())
        return false;
    if (Type.toULong() == PROXY_NONE)
        return true;
    if ((Port.toULong() != d.Port.toULong()) && (Host.str() != d.Host.str()))
        return false;
    if (Type.toULong() == PROXY_SOCKS4)
        return true;
    if (Auth.toBool() != d.Auth.toBool())
        return false;
    if (!d.Auth.toBool())
        return true;
    return ((User.str() == d.User.str()) && (Password.str() == d.Password.str()));
}

ProxyData& ProxyData::operator = (const ProxyData &d)
{
    if (bInit){
        free_data(_proxyData, this);
        bInit = false;
    }
    if (d.bInit){
        Buffer cfg;
        cfg = "[Title]\n" + save_data(_proxyData, (void*)(&d));
        cfg.setWritePos(0);
        cfg.getSection();
        load_data(_proxyData, this, &cfg);
        bInit = true;
        Default = d.Default;
    }else{
        load_data(_proxyData, this, NULL);
    }

    return *this;
}

ProxyData& ProxyData::operator = (Buffer *cfg)
{
    if (bInit){
        free_data(_proxyData, this);
        bInit = false;
    }
    load_data(_proxyData, this, cfg);
    bInit = true;
    return *this;
}

class Proxy : public Socket, public SocketNotify
{
public:
    Proxy(ProxyPlugin *plugin, ProxyData *data, TCPClient *client);
    ~Proxy();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    void setSocket(ClientSocket*);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    virtual Mode mode() const { return Indirect; }
    virtual bool isEncrypted(){ return false; }
    virtual bool startEncryption(){ return false; }
    PROP_ULONG(Type);
    PROP_STR(Host);
    PROP_USHORT(Port);
    PROP_BOOL(Auth);
    PROP_STR(User);
    PROP_STR(Password);
    PROP_BOOL(NoShow);
    ProxyPlugin *m_plugin;
protected:
    virtual void write();
    virtual void write_ready();
    virtual void error_state(const QString &text, unsigned code = 0);
    virtual void proxy_connect_ready();
    void read(unsigned size, unsigned minsize=0);
    bool		m_bClosed;
    TCPClient	*m_client;
    Socket		*m_sock;
    Buffer		bOut;
    Buffer		bIn;
    ProxyData	data;
    QString         m_host;
    unsigned short  m_port;
};

class Listener : public SocketNotify, public ServerSocket
{
public:
    Listener(ProxyPlugin *plugin, ProxyData *data, ServerSocketNotify *notify, unsigned long ip);
    ~Listener();
    PROP_ULONG(Type);
    PROP_STR(Host);
    PROP_USHORT(Port);
    PROP_BOOL(Auth);
    PROP_STR(User);
    PROP_STR(Password);
protected:
    virtual void write();
    virtual void write_ready();
    virtual void bind(unsigned short mixPort, unsigned short maxPort, TCPClient *client);
#ifndef WIN32
    virtual void bind(const char *path);
#endif
    virtual void close();
    void read(unsigned size, unsigned minsize=0);
    unsigned long m_ip;
    Socket		*m_sock;
    ProxyData	data;
    ProxyPlugin	*m_plugin;
    Buffer		bOut;
    Buffer		bIn;
};

Proxy::Proxy(ProxyPlugin *plugin, ProxyData *d, TCPClient *client)
{
    data = *d;
    m_plugin   = plugin;
    m_sock     = NULL;
    m_client   = client;
    m_bClosed  = false;
    m_plugin->proxies.push_back(this);
    bIn.packetStart();
    bOut.packetStart();
}

Proxy::~Proxy()
{
    if (notify)
        static_cast<ClientSocket*>(notify)->setSocket(m_sock);
    if (m_sock)
        delete m_sock;
    for (list<Proxy*>::iterator it = m_plugin->proxies.begin(); it != m_plugin->proxies.end(); ++it){
        if (*it == this){
            m_plugin->proxies.erase(it);
            break;
        }
    }
}

void Proxy::setSocket(ClientSocket *s)
{
    m_sock = s->socket();
    s->setSocket(this);
    m_sock->setNotify(this);
}

int Proxy::read(char*, unsigned int)
{
    return 0;
}

void Proxy::write(const char*, unsigned int)
{
    log(L_WARN, "Proxy can't write");
    if (notify)
        notify->error_state("Error proxy write");
}

void Proxy::close()
{
    if (m_bClosed)
        return;
    m_bClosed = true;
    if (m_sock){
        m_sock->setNotify(notify);
        m_sock->close();
    }
}

unsigned long Proxy::localHost()
{
    return m_sock->localHost();
}

void Proxy::pause(unsigned n)
{
    m_sock->pause(n);
}

void Proxy::write_ready()
{
}

void Proxy::error_state(const QString &err, unsigned code)
{
    if (notify){
        if (code == m_plugin->ProxyErr){
            if (getNoShow()){
                code = 0;
            }else{
                if (m_client != (TCPClient*)(-1))
                    m_client->m_reconnect = NO_RECONNECT;
            }
        }
        notify->error_state(err, code);
    }
}

void Proxy::read(unsigned size, unsigned minsize)
{
    bIn.init(size);
    bIn.packetStart();
    int readn = m_sock->read(bIn.data(0), size);
    if ((readn != (int)size) || (minsize && (readn < (int)minsize))){
        if (notify)
            notify->error_state("Error proxy read");
        return;
    }
    EventLog::log_packet(bIn, false, m_plugin->ProxyPacket);
}

void Proxy::write()
{
    EventLog::log_packet(bOut, true, m_plugin->ProxyPacket);
    m_sock->write(bOut.data(0), bOut.size());
    bOut.init(0);
    bOut.packetStart();
}

void Proxy::proxy_connect_ready()
{
    if (notify){
        SocketNotify *n = notify;
        static_cast<ClientSocket*>(n)->setSocket(m_sock);
        m_sock = NULL;
        n->connect_ready();
    }
    getSocketFactory()->remove(this);
}

Listener::Listener(ProxyPlugin *plugin, ProxyData *_data, ServerSocketNotify *notify, unsigned long ip)
{
    m_ip     = ip;
    m_plugin = plugin;
    m_sock   = getSocketFactory()->createSocket();
    m_sock->setNotify(this);
    data     = *_data;
    notify->setListener(this);
}

Listener::~Listener()
{
    if (m_sock)
        delete m_sock;
}

void Listener::bind(unsigned short, unsigned short, TCPClient*)
{
}

#ifndef WIN32

void Listener::bind(const char*)
{
}

#endif

void Listener::close()
{
}

void Listener::write()
{
    EventLog::log_packet(bOut, true, m_plugin->ProxyPacket);
    m_sock->write(bOut.data(0), bOut.size());
    bOut.init(0);
    bOut.packetStart();
}

void Listener::read(unsigned size, unsigned minsize)
{
    bIn.init(size);
    bIn.packetStart();
    int readn = m_sock->read(bIn.data(0), size);
    if ((readn != (int)size) || (minsize && (readn < (int)minsize))){
        if (notify && notify->error("Error proxy read"))
            delete notify;
        return;
    }
    EventLog::log_packet(bIn, false, m_plugin->ProxyPacket);
}

void Listener::write_ready()
{
}

// ______________________________________________________________________________________

class SOCKS4_Proxy : public Proxy
{
public:
    SOCKS4_Proxy(ProxyPlugin *plugin, ProxyData *data, TCPClient *client);
    virtual void connect(const QString &host, unsigned short port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    virtual void error_state(const QString &text, unsigned code);
    enum State
    {
        None,
        Connect,
        WaitConnect
    };
    State m_state;
};

class SOCKS4_Listener : public Listener
{
public:
    SOCKS4_Listener(ProxyPlugin *plugin, ProxyData *data, ServerSocketNotify *notify, unsigned long ip);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    virtual void error_state(const QString &text, unsigned code = 0);
    enum State
    {
        Connect,
        WaitListen,
        Accept
    };
    State m_state;
};

SOCKS4_Proxy::SOCKS4_Proxy(ProxyPlugin *plugin, ProxyData *data, TCPClient *client)
        : Proxy(plugin, data, client)
{
    m_state = None;
}

void SOCKS4_Proxy::connect(const QString &host, unsigned short port)
{
    if (m_state != None){
        if (notify) notify->error_state(STATE_ERROR);
        return;
    }
    m_host = host;
    m_port = port;
    log(L_DEBUG, "Connect to proxy SOCKS4 %s:%u", qPrintable(getHost()), getPort());
    m_sock->connect(getHost(), getPort());
    m_state = Connect;
}

void SOCKS4_Proxy::error_state(const QString &text, unsigned code)
{
    if (m_state == Connect){
        Proxy::error_state(CONNECT_ERROR, m_plugin->ProxyErr);
        return;
    }
    Proxy::error_state(text, code);
}

void SOCKS4_Proxy::connect_ready()
{
    if (m_state != Connect){
        error_state(STATE_ERROR, 0);
        return;
    }
    unsigned long addr = inet_addr(qPrintable(m_host));
    if (addr == INADDR_NONE){
        struct hostent *hp = gethostbyname(qPrintable(m_host));
        if (hp) addr = *((unsigned long*)(hp->h_addr_list[0]));
    }
    if (notify)
        notify->resolve_ready(QHostAddress(addr));
    bOut
    << (char)4
    << (char)1
    << m_port
    << (unsigned long)htonl(addr)
    << (char)0;
    m_state = WaitConnect;
}

void SOCKS4_Proxy::read_ready()
{
    if (m_state != WaitConnect) return;
    read(9, 4);
    char b1, b2;
    bIn >> b1 >> b2;
    if (b2 != 90){
        error_state(ANSWER_ERROR, m_plugin->ProxyErr);
        return;
    }
    proxy_connect_ready();
}

SOCKS4_Listener::SOCKS4_Listener(ProxyPlugin *plugin, ProxyData *data, ServerSocketNotify *notify, unsigned long ip)
        : Listener(plugin, data, notify, ip)
{
    log(L_DEBUG, "Connect to proxy SOCKS4 %s:%u", qPrintable(getHost()), getPort());
    m_sock->connect(getHost(), getPort());
    m_state = Connect;
}

void SOCKS4_Listener::connect_ready()
{
    bOut
    << (char)4
    << (char)2
    << (unsigned short)0
    << (unsigned long)m_ip
    << (char)0;
    m_state = WaitListen;
}

void SOCKS4_Listener::read_ready()
{
    char b1, b2;
    unsigned short port;
    unsigned long  ip;
    switch (m_state){
    case WaitListen:
        read(8);
        bIn >> b1 >> b2;
        if (b2 != 90){
            error_state("bad proxy answer", 0);
            return;
        }
        bIn >> port;
        m_state = Accept;
        if (notify)
            notify->bind_ready(port);
        break;
    case Accept:
        read(8);
        bIn >> b1 >> b2;
        if (b2 != 90){
            error_state("bad proxy answer", 0);
            return;
        }
        bIn >> port >> ip;
        if (notify){
            notify->accept(m_sock, ip);
            m_sock = NULL;
        }else{
            error_state("Bad state", 0);
        }
        break;
    default:
        break;
    }
}

void SOCKS4_Listener::error_state(const QString &err, unsigned)
{
    if (notify)
        notify->error(err);
}

// ______________________________________________________________________________________

class SOCKS5_Proxy : public Proxy
{
public:
    SOCKS5_Proxy(ProxyPlugin*, ProxyData*, TCPClient*);
    virtual void connect(const QString &host, unsigned short port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    virtual void error_state(const QString &text, unsigned code);
    enum State
    {
        None,
        Connect,
        WaitAnswer,
        WaitAuth,
        WaitConnect
    };
    State m_state;
    void send_connect();
};

class SOCKS5_Listener : public Listener
{
public:
    SOCKS5_Listener(ProxyPlugin *plugin, ProxyData *data, ServerSocketNotify *notify, unsigned long ip);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    virtual void error_state(const QString &text, unsigned code = 0);
    void send_listen();
    enum State
    {
        Connect,
        WaitAnswer,
        WaitAuth,
        WaitListen,
        Accept
    };
    State m_state;
};

SOCKS5_Proxy::SOCKS5_Proxy(ProxyPlugin *plugin, ProxyData *d, TCPClient *client)
        : Proxy(plugin, d, client)
{
    m_state = None;
}

void SOCKS5_Proxy::connect(const QString &host, unsigned short port)
{
    if (m_state != None){
        error_state(STATE_ERROR, 0);
        return;
    }
    m_host = host;
    m_port = port;
    log(L_DEBUG, "Connect to proxy SOCKS5 %s:%u", qPrintable(getHost()), getPort());
    m_sock->connect(getHost(), getPort());
    m_state = Connect;
}

void SOCKS5_Proxy::connect_ready()
{
    if (m_state != Connect){
        error_state(STATE_ERROR, 0);
        return;
    }
    bOut << 0x05020002L;
    m_state = WaitAnswer;
    write();
}

void SOCKS5_Proxy::read_ready()
{
    char b1, b2;
    unsigned long ip;
    switch (m_state){
    case WaitAnswer:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 == '\xFF')) {
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        if (b2 == 0x02) {
            const QByteArray user = getUser().toUtf8();
            const QByteArray pswd = getPassword().toUtf8();
            bOut
            << (char)0x01
            << (char)user.length()
            << user.data()
            << (char)pswd.length()
            << pswd.data();
            m_state = WaitAuth;
            write();
            return;
        }
        send_connect();
        return;
    case WaitAuth:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x01) || (b2 != 0x00)) {
            error_state(AUTH_ERROR, m_plugin->ProxyErr);
            return;
        }
        send_connect();
        return;
    case WaitConnect:
        read(10);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 != 0x00)) {
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        bIn >> b1 >> b2;
        bIn >> ip;
        if (notify)
            notify->resolve_ready(QHostAddress(ip));
        proxy_connect_ready();
        return;
    default:
        break;
    }
}

void SOCKS5_Proxy::error_state(const QString &text, unsigned code)
{
    if (m_state == Connect){
        Proxy::error_state(CONNECT_ERROR, m_plugin->ProxyErr);
        return;
    }
    Proxy::error_state(text, code);
}

void SOCKS5_Proxy::send_connect()
{
    unsigned long addr = inet_addr(qPrintable(m_host));
    bOut << (char)0x05
    << (char)0x01			/* CONNECT */
    << (char)0x00;			/* reserved */
    if (addr != INADDR_NONE){
        bOut << (char)0x01	    /* address type -- ip addr */
        << (unsigned long)htonl(addr);
    }else{
        bOut << (char)0x03		/* address type -- host name */
        << (char)m_host.length();
        const QByteArray ba = m_host.toLocal8Bit();
        bOut.pack(ba.data(), ba.length());
    }
    bOut << m_port;
    m_state = WaitConnect;
    write();
}

SOCKS5_Listener::SOCKS5_Listener(ProxyPlugin *plugin, ProxyData *data, ServerSocketNotify *notify, unsigned long ip)
        : Listener(plugin, data, notify, ip)
{
    log(L_DEBUG, "Connect to proxy SOCKS5 %s:%u", qPrintable(getHost()), getPort());
    m_sock->connect(getHost(), getPort());
    m_state = Connect;
}

void SOCKS5_Listener::connect_ready()
{
    if (m_state != Connect){
        error_state(STATE_ERROR, 0);
        return;
    }
    bOut << 0x05020002L;
    m_state = WaitAnswer;
    write();
}

void SOCKS5_Listener::read_ready()
{
    char b1, b2;
    unsigned short port;
    unsigned long ip;
    switch (m_state){
    case WaitAnswer:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 == '\xFF')) {
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        if (b2 == 0x02) {
            const QByteArray user = getUser().toUtf8();
            const QByteArray pswd = getPassword().toUtf8();
            bOut
            << (char)0x01
            << (char)user.length()
            << user.data()
            << (char)pswd.length()
            << pswd.data();
            m_state = WaitAuth;
            write();
            return;
        }
        send_listen();
        return;
    case WaitAuth:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x01) || (b2 != 0x00)) {
            error_state(AUTH_ERROR, m_plugin->ProxyErr);
            return;
        }
        send_listen();
        return;
    case WaitListen:
        read(10);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 != 0x00)) {
            error_state(AUTH_ERROR, m_plugin->ProxyErr);
            return;
        }
        bIn >> b1 >> b2;
        bIn >> ip;
        bIn >> port;
        m_state = Accept;
        if (notify)
            notify->bind_ready(port);
        return;
    case Accept:
        read(10);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 != 0x02)) {
            error_state("Bad accept code", 0);
            return;
        }
        bIn >> b1 >> b2;
        bIn >> ip;
        if (notify){
            notify->accept(m_sock, ip);
            m_sock = NULL;
        }else{
            error_state("Bad accept code", 0);
            return;
        }
        return;
    default:
        break;
    }
}

void SOCKS5_Listener::send_listen()
{
    bOut << 0x05020001L << m_ip << (unsigned short)0;
    write();
    m_state = WaitListen;
}

void SOCKS5_Listener::error_state(const QString &err, unsigned)
{
    if (notify)
        notify->error(err);
}

// ______________________________________________________________________________________

class HTTPS_Proxy : public Proxy
{
public:
    HTTPS_Proxy(ProxyPlugin *plugin, ProxyData*, TCPClient *client);
    virtual void connect(const QString &host, unsigned short port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    void error_state(const QString &text, unsigned code);
    void send_auth();
    enum State
    {
        None,
        Connect,
        WaitConnect,
        WaitEmpty
    };
    State m_state;
    bool readLine(QByteArray &s);
};

HTTPS_Proxy::HTTPS_Proxy(ProxyPlugin *plugin, ProxyData *d, TCPClient *client)
        : Proxy(plugin, d, client)
{
    m_state = None;
}

void HTTPS_Proxy::connect(const QString &host, unsigned short port)
{
    if (m_state != None){
        error_state(STATE_ERROR, 0);
        return;
    }
    m_host = host;
    m_port = port;
    if ((m_client != (TCPClient*)(-1)) && (m_client->protocol()->description()->flags & PROTOCOL_ANY_PORT))
        m_port = 443;
    log(L_DEBUG, "Connect to proxy HTTPS %s:%u", qPrintable(getHost()), getPort());
    m_sock->connect(getHost(), getPort());
    m_state = Connect;
}

void HTTPS_Proxy::connect_ready()
{
    if (m_state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        error_state(CONNECT_ERROR, 0);
        return;
    }
    bIn.packetStart();
    bOut << "CONNECT "
    << qPrintable(m_host)
    << ":"
    << QByteArray::number(m_port).data()
    << " HTTP/1.0\r\n"
    << "User-Agent: "
    << qPrintable(get_user_agent())
    << "\r\n";
    send_auth();
    bOut << "\r\n";
    m_state = WaitConnect;
    write();
}

static char HTTP[] = "HTTP/";

void HTTPS_Proxy::send_auth()
{
    if (getAuth()){
        QByteArray s = basic_auth(getUser(), getPassword());
        bOut << "Proxy-Authorization: Basic ";
        bOut << s.constData();
        bOut << "\r\n";
    }
}

void HTTPS_Proxy::error_state(const QString &text, unsigned code)
{
    if (m_state == Connect){
        Proxy::error_state(CONNECT_ERROR, m_plugin->ProxyErr);
        return;
    }
    Proxy::error_state(text, code);
}

void HTTPS_Proxy::read_ready()
{
    if (m_state == WaitConnect){
        QByteArray s;
        if (!readLine(s))
            return;
        if ((unsigned)s.length() < strlen(HTTP)){
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        int idx = s.indexOf(' ');
        if (idx == -1){
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        s = s.mid(idx + 1);
        idx = s.indexOf(' ');
        if (idx!=-1)
            s=s.left(idx+1);
        int code = s.toInt();
        if (code == 407){
            error_state(AUTH_ERROR, m_plugin->ProxyErr);
            return;
        }
        if (code != 200){
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return;
        }
        m_state = WaitEmpty;
    }
    if (m_state == WaitEmpty){
        for (;;){
            QByteArray s;
            if (!readLine(s))
                return;
            if (s.length() == 0)
                break;
        }
        proxy_connect_ready();
    }
}

bool HTTPS_Proxy::readLine(QByteArray &s)
{
    for (;;){
        char c;
        int n = m_sock->read(&c, 1);
        if (n < 0){
            error_state(ANSWER_ERROR, m_plugin->ProxyErr);
            return false;
        }
        if (n == 0)
            return false;
        if (c == '\r')
            continue;
        if (c == '\n')
            break;
        bIn << c;
    }
    bIn << '\0';
    EventLog::log_packet(bIn, false, m_plugin->ProxyPacket);
    if(bIn.size())
        s = bIn;
    bIn.init(0);
    bIn.packetStart();
    return true;
}

// ______________________________________________________________________________________

class HTTP_Proxy : public HTTPS_Proxy
{
public:
    HTTP_Proxy(ProxyPlugin *plugin, ProxyData*, TCPClient *client);
    void connect(const QString &host, unsigned short port);
protected:
    virtual void write(const char *buf, unsigned int size);
    virtual int read(char *buf, unsigned int size);
    void read_ready();
    void write_ready();
    void connect_ready();
    enum State
    {
        WaitHeader,
        Headers,
        Data
    };
    State		m_state;
    Buffer		m_out;
    bool		m_bHTTP;
    unsigned	m_size;
    QByteArray	m_head;
};

HTTP_Proxy::HTTP_Proxy(ProxyPlugin *plugin, ProxyData *data, TCPClient *client)
        : HTTPS_Proxy(plugin, data, client)
{
    m_bHTTP = true;
    m_state = WaitHeader;
    m_size  = 0;
}

void HTTP_Proxy::read_ready()
{
    if (!m_bHTTP){
        HTTPS_Proxy::read_ready();
        return;
    }
    if (!m_head.isEmpty())
        return;
    if (!readLine(m_head))
        return;
    if ((unsigned)m_head.length() < strlen(HTTP)){
        error_state(ANSWER_ERROR, m_plugin->ProxyErr);
        return;
    }
    int idx = m_head.indexOf(' ');
    if (idx == -1){
        error_state(ANSWER_ERROR, m_plugin->ProxyErr);
        return;
    }
    QByteArray str = m_head.mid(idx + 1);
    int code = str.toInt();
    if (code == 407){
        error_state(AUTH_ERROR, m_plugin->ProxyErr);
        return;
    }
    m_head += "\r\n";
    if (notify)
        notify->read_ready();
}

void HTTP_Proxy::connect(const QString &host, unsigned short port)
{
    if (port == 443)
        m_bHTTP = false;
    HTTPS_Proxy::connect(host, port);
}

void HTTP_Proxy::connect_ready()
{
    if (!m_bHTTP){
        HTTPS_Proxy::connect_ready();
        return;
    }
    bIn.packetStart();
    if (notify)
        notify->connect_ready();
}

void HTTP_Proxy::write_ready()
{
    if ((m_state == Data) && notify)
        notify->write_ready();
}

int HTTP_Proxy::read(char *buf, unsigned int size)
{
    if (!m_bHTTP)
        return HTTPS_Proxy::read(buf, size);
    if (m_head.isEmpty())
        return 0;
    if (size > (unsigned)m_head.length())
        size = m_head.length();
    memcpy(buf, m_head.data(), size);
    m_head = m_head.mid(size);
    if (m_head.isEmpty()){
        static_cast<ClientSocket*>(notify)->setSocket(m_sock);
        m_sock = NULL;
        getSocketFactory()->remove(this);
    }
    return size;
}

void HTTP_Proxy::write(const char *buf, unsigned int size)
{
    if (!m_bHTTP){
        HTTPS_Proxy::write(buf, size);
        return;
    }
    if (m_state == Data){
        unsigned out_size = size;
        if (out_size > m_size)
            out_size = m_size;
        if (out_size == 0)
            return;
        bOut.pack(buf, out_size);
        m_size -= out_size;
        HTTPS_Proxy::write();
        return;
    }
    m_out.pack(buf, size);
    QByteArray line;
    if (m_state == WaitHeader){
        if (!m_out.scan("\r\n", line))
            return;
        bOut
        << getToken(line, ' ', false).data()
        << " http://"
        << qPrintable(m_host);
        if (m_port != 80)
            bOut << ":" << QByteArray::number(m_port).data();
        bOut << getToken(line, ' ', false).data();
        bOut << " HTTP/1.1\r\n";
        m_state = Headers;
    }
    if (m_state == Headers){
        for (;;){
            if (!m_out.scan("\r\n", line)){
                HTTPS_Proxy::write();
                return;
            }
            if (line.isEmpty())
                break;
            QByteArray param = getToken(line, ':');
            if (param == "Content-Length"){
                QByteArray p = line.trimmed();
                m_size = p.toUInt();
            }
            bOut << param.data() << ":" << line.data() << "\r\n";
        }
        send_auth();
        bOut << "\r\n";
        if (m_out.readPos() < m_out.writePos()){
            unsigned out_size = m_out.writePos() - m_out.readPos();
            if (out_size > m_size)
                out_size = m_size;
            bOut.pack(m_out.data(m_out.readPos()), out_size);
            m_size -= out_size;
        }
        m_out.init(0);
        m_state = Data;
        HTTPS_Proxy::write();
    }
}

// ______________________________________________________________________________________


Plugin *createProxyPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new ProxyPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Proxy"),
        I18N_NOOP("Plugin provides proxy support (SOCKS4, SOCKS5, HTTPS/HTTP)"),
        VERSION,
        createProxyPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

ProxyPlugin::ProxyPlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    data = config;
    ProxyPacket = registerType();
    ProxyErr    = registerType();
    getContacts()->addPacketType(ProxyPacket, info.title);
}

ProxyPlugin::~ProxyPlugin()
{
    while (proxies.size()){
        delete proxies.front();
    }
    getContacts()->removePacketType(ProxyPacket);
}

QString ProxyPlugin::clientName(TCPClient *client)
{
    if (client == (TCPClient*)(-1))
        return "HTTP";
    return static_cast<Client*>(client)->name();
}

void ProxyPlugin::clientData(TCPClient *client, ProxyData &cdata)
{
    for(unsigned i = 1;; i++)
    {
        const QString proxyCfg = getClients(i);
        if (proxyCfg.isEmpty())
            break;
        ProxyData wdata(qPrintable(proxyCfg));
        if (clientName(client) == wdata.Client.str())
        {
            cdata = wdata;
            cdata.Default.asBool() = false;
            cdata.Client.str() = clientName(client);
            return;
        }
    }
    cdata = data;
    cdata.Client.str() = clientName(client);
    cdata.Default.asBool() = true;
    cdata.Clients.clear();
}

bool ProxyPlugin::processEvent(Event *e)
{
    switch (e->type()) {
    case eEventSocketConnect: {
        EventSocketConnect *esc = static_cast<EventSocketConnect*>(e);
        list<Proxy*>::iterator it;
        for (it = proxies.begin(); it != proxies.end(); ++it){
            if ((*it)->getNotify() == esc->socket())
                return false;
        }
        ProxyData data;
        clientData(esc->client(), data);
        Proxy *proxy = NULL;
        switch (data.Type.toULong()){
        case PROXY_SOCKS4:
            proxy = new SOCKS4_Proxy(this, &data, esc->client());
            break;
        case PROXY_SOCKS5:
            proxy = new SOCKS5_Proxy(this, &data, esc->client());
            break;
        case PROXY_HTTPS:
            if (esc->client() == (TCPClient*)(-1)){
                proxy = new HTTP_Proxy(this, &data, esc->client());
            }else{
                proxy = new HTTPS_Proxy(this, &data, esc->client());
            }
            break;
        }
        if (proxy){
            proxy->setSocket(esc->socket());
            return true;
        }
        break;
    }
    case eEventSocketListen: {
        EventSocketListen *esl = static_cast<EventSocketListen*>(e);
        ProxyData data;
        clientData(esl->client(), data);
        Listener *listener = NULL;
        switch (data.Type.toULong())
		{
        case PROXY_SOCKS4:
            listener = new SOCKS4_Listener(this, &data, esl->notify(), esl->client()->ip());
            break;
        case PROXY_SOCKS5:
            listener = new SOCKS5_Listener(this, &data, esl->notify(), esl->client()->ip());
            break;
        }
        if (listener)
            return true;
        break;
    }
    case eEventRaiseWindow: {
        EventRaiseWindow *win = static_cast<EventRaiseWindow*>(e);
        QWidget *w = win->widget();
        if (!w || !w->inherits("NewProtocol"))
            return false;
        NewProtocol *p = static_cast<NewProtocol*>(w);
        if (p->m_client->protocol()->description()->flags & PROTOCOL_NOPROXY)
            return false;
        ProxyConfig *cfg = w->findChild<ProxyConfig*>();
        if (cfg)
            return false;
        QTabWidget *tab  = w->findChild<QTabWidget*>();
        if (tab){
            cfg = new ProxyConfig(tab, this, tab, p->m_client);
            QObject::connect(tab->topLevelWidget(), SIGNAL(apply()), cfg, SLOT(apply()));
        }
        break;
    }
    case eEventClientNotification: {
        EventClientNotification *ee = static_cast<EventClientNotification*>(e);
        const EventNotification::ClientNotificationData &data = ee->data();
        if (data.code == ProxyErr){
            QString msg;
            if (!data.text.isEmpty())
                msg = i18n(data.text).arg(data.args);
            ProxyError *err = new ProxyError(this, static_cast<TCPClient*>(data.client), msg);
            raiseWindow(err);
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

QByteArray ProxyPlugin::getConfig()
{
    return save_data(_proxyData, &data);
}

QWidget *ProxyPlugin::createConfigWindow(QWidget *parent)
{
    return new ProxyConfig(parent, this, NULL, static_cast <ClientPtr> (NULL));
}

const DataDef *ProxyPlugin::proxyData = _proxyData;
