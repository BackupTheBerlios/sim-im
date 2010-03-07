/***************************************************************************
                          polling.cpp  -  description
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

#include <stdio.h>

#include "fetch.h"
#include "log.h"

#include "icqclient.h"
#include "polling.h"

using namespace std;
using namespace SIM;

const unsigned short HTTP_PROXY_VERSION = 0x0443;

const unsigned short HTTP_PROXY_HELLO	   = 2;
const unsigned short HTTP_PROXY_LOGIN      = 3;
const unsigned short HTTP_PROXY_UNK1	   = 4;
const unsigned short HTTP_PROXY_FLAP       = 5;
const unsigned short HTTP_PROXY_CONNECT	   = 6;
const unsigned short HTTP_PROXY_UNK2       = 7;

class HttpPacket
{
    COPY_RESTRICTED(HttpPacket)
public:
    HttpPacket(const char *data, unsigned short size, unsigned short type, unsigned long nSock);
    ~HttpPacket();
    char *data;
    unsigned short size;
    unsigned short type;
    unsigned long  nSock;
};

HttpPacket::HttpPacket(const char *_data, unsigned short _size, unsigned short _type, unsigned long _nSock)
{
    size = _size;
    type = _type;
    nSock = _nSock;
    data = NULL;
    if (size){
        data = new char[size];
        memcpy(data, _data, size);
    }
}

HttpPacket::~HttpPacket()
{
    if (data) delete[] data;
}

// ___________________________________________________________________________________

static char ANSWER_ERROR[] = "Bad answer";

class HttpRequest : public FetchClient
{
public:
    HttpRequest(HttpPool *pool);
    void send();
protected:
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    virtual HttpPacket *packet()     = 0;
    virtual QString url()            = 0;
    virtual void data_ready(ICQBuffer*) = 0;
    HttpPool *m_pool;
};

HttpRequest::HttpRequest(HttpPool *pool)
{
    m_pool     = pool;
}

void HttpRequest::send()
{
    HttpPacket *p = packet();
    ICQBuffer *postData = NULL;
    if (p){
        postData = new ICQBuffer;
        unsigned short len = (unsigned short)(p->size + 12);
        *postData
        << len
        << HTTP_PROXY_VERSION
        << p->type
        << 0x00000000L
        << p->nSock;
        if (p->size)
            postData->pack(p->data, p->size);
        m_pool->queue.remove(p);
        delete p;
    }
    char headers[] = "Cache-control: no-store, no-cache\n"
                     "Pragma: no-cache";
    fetch(url(), headers, postData);
}

bool HttpRequest::done(unsigned code, Buffer &data, const QString &)
{
    if (code != 200){
        log(L_DEBUG, "Res: %u %s", code, qPrintable(url()));
        m_pool->error(ANSWER_ERROR);
        return false;
    }
    ICQBuffer d(data);
    data_ready(&d);
    return true;
}

unsigned long HttpPool::localHost()
{
    return 0;
}

void HttpPool::pause(unsigned time)
{
    QTimer::singleShot(time * 1000, this, SLOT(timeout()));
}

void HttpPool::timeout()
{
    if (notify)
        notify->write_ready();
}

// ______________________________________________________________________________________

class HelloRequest : public HttpRequest
{
public:
    HelloRequest(HttpPool *poll, bool bAIM);
protected:
    virtual HttpPacket *packet();
    virtual QString url();
    virtual void data_ready(ICQBuffer*);
    bool m_bAIM;
};

HelloRequest::HelloRequest(HttpPool *poll, bool bAIM)
        : HttpRequest(poll)
{
    m_bAIM = bAIM;
    send();
}

HttpPacket *HelloRequest::packet()
{
    return NULL;
}

QString HelloRequest::url()
{
    return m_bAIM ? "http://aimhttp.oscar.aol.com/hello" : "http://http.proxy.icq.com/hello";
}

void HelloRequest::data_ready(ICQBuffer *bIn)
{
    m_pool->hello = NULL;
    bIn->incReadPos(12);
    unsigned long SID[4];
    (*bIn) >> SID[0] >> SID[1] >> SID[2] >> SID[3];
    char b[34];
    snprintf(b, sizeof(b), "%08lx%08lx%08lx%08lx", SID[0], SID[1], SID[2], SID[3]);
    m_pool->sid = b;
    bIn->unpackStr(m_pool->m_host);
    m_pool->request();
}

// ______________________________________________________________________________________

class MonitorRequest : public HttpRequest
{
public:
    MonitorRequest(HttpPool *pool);
protected:
    virtual HttpPacket *packet();
    virtual QString url();
    virtual void data_ready(ICQBuffer*);
};

MonitorRequest::MonitorRequest(HttpPool *pool)
        : HttpRequest(pool)
{
    send();
}

HttpPacket *MonitorRequest::packet()
{
    return NULL;
}

QString MonitorRequest::url()
{
    QString sURL;
    sURL  = "http://";
    sURL += m_pool->m_host;
    sURL += "/monitor?sid=";
    sURL += m_pool->sid;
    return sURL;
}

void MonitorRequest::data_ready(ICQBuffer *bIn)
{
    m_pool->monitor = NULL;
    m_pool->readn = 0;
    while (bIn->readPos() < (unsigned)bIn->size()){
        unsigned short len, ver, type;
        (*bIn) >> len >> ver >> type;
        bIn->incReadPos(8);
        len -= 12;
        if (len > (bIn->size() - bIn->readPos())){
            m_pool->error(ANSWER_ERROR);
            return;
        }
        if (ver != HTTP_PROXY_VERSION){
            m_pool->error(ANSWER_ERROR);
            return;
        }
        switch (type){
        case HTTP_PROXY_FLAP:
            if (len){
                unsigned short nSock;
                bIn->incReadPos(-2);
                *bIn >> nSock;
                if (nSock == m_pool->nSock){
                    char *data = bIn->data(bIn->readPos());
                    m_pool->readData.pack(data, len);
                    m_pool->readn += len;
                }
                bIn->incReadPos(len);
            }
            break;
        case HTTP_PROXY_UNK1:
        case HTTP_PROXY_UNK2:
            if (len)
                bIn->incReadPos(len);
            break;
        default:
            m_pool->error(ANSWER_ERROR);
            return;
        }
    }
    m_pool->request();
}

// ______________________________________________________________________________________

class PostRequest : public HttpRequest
{
public:
    PostRequest(HttpPool *proxy);
protected:
    virtual HttpPacket *packet();
    virtual QString url();
    virtual void data_ready(ICQBuffer *b);
};

PostRequest::PostRequest(HttpPool *proxy)
        : HttpRequest(proxy)
{
    send();
}

HttpPacket *PostRequest::packet()
{
    if (m_pool->queue.size())
        return m_pool->queue.front();
    return NULL;
}

QString PostRequest::url()
{
    QString sURL;
    sURL  = "http://";
    sURL += m_pool->m_host;
    sURL += "/data?sid=";
    sURL += m_pool->sid;
    sURL += "&seq=";
    char b[15];
    snprintf(b, sizeof(b), "%u", ++m_pool->seq);
    sURL += b;
    return sURL;
}

void PostRequest::data_ready(ICQBuffer*)
{
    m_pool->post = NULL;
    m_pool->request();
}

// ______________________________________________________________________________________

HttpPool::HttpPool(bool bAIM)
{
    m_bAIM = bAIM;
    hello = NULL;
    monitor = NULL;
    post = NULL;
    state = None;
    seq = 0;
    readn = 0;
    nSock = 0;
}

HttpPool::~HttpPool()
{
    if (hello) delete hello;
    if (monitor) delete monitor;
    if (post) delete post;
    for (list<HttpPacket*>::iterator it = queue.begin(); it != queue.end(); ++it)
        delete *it;
}

int HttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == (unsigned)readData.size())
        readData.init(0);
    return size;
}

void HttpPool::write(const char *buf, unsigned size)
{
    queue.push_back(new HttpPacket(buf, (unsigned short)size, HTTP_PROXY_FLAP, nSock));
    request();
}

void HttpPool::close()
{
    readData.init(0);
}

void HttpPool::connect(const QString &host, unsigned short port)
{
    state = None;
    ICQBuffer b;
    unsigned short len = host.length();
    b << len << host.toLocal8Bit().data() << port;
    nSock++;
    queue.push_back(new HttpPacket(b.data(0), (unsigned short)(b.size()), HTTP_PROXY_LOGIN, nSock));
    if (sid.length()){
        unsigned char close_packet[] = { 0x2A, 0x04, 0x14, 0xAB, 0x00, 0x00 };
        queue.push_back(new HttpPacket((char*)close_packet, sizeof(close_packet), HTTP_PROXY_FLAP, 1));
        queue.push_back(new HttpPacket(NULL, 0, HTTP_PROXY_CONNECT, 1));
    }
    request();
}

void HttpPool::request()
{
    if (sid.length() == 0){
        if (hello == NULL)
            hello = new HelloRequest(this, m_bAIM);
        return;
    }
    if (monitor == NULL)
        monitor = new MonitorRequest(this);
    if (queue.size() && (post == NULL))
        post = new PostRequest(this);
    if (readn && notify){
        if (state == None){
            state = Connected;
            notify->connect_ready();
        }
        readn = 0;
        notify->read_ready();
    }
}

Socket *ICQClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
        if (!m_bFirstTry)
            m_bFirstTry = true;
    }
    if (m_bHTTP)
        return new HttpPool(m_bAIM);
    return NULL;
}

