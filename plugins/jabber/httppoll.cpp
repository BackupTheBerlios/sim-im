/***************************************************************************
                          httppool.cpp  -  description
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

#include "fetch.h"
#include "log.h"

#include "jabberclient.h"
#include <QCryptographicHash>
#include <QByteArray>

using namespace SIM;

class JabberHttpPool : public Socket, public FetchClient
{
public:
    JabberHttpPool(const QString &url);
    ~JabberHttpPool();
    virtual void connect(const QString &host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() const { return Web; }
    virtual bool isEncrypted(){ return false; }
    virtual bool startEncryption(){ return false; }
protected:
    QString getKey();
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    JabberBuffer readData;
    JabberBuffer writeData;
    QString m_url;
    QByteArray m_key;
    QByteArray m_seed;
    QString m_cookie;
    virtual unsigned long localHost();
    virtual void pause(unsigned);
};

// ______________________________________________________________________________________

JabberHttpPool::JabberHttpPool(const QString &url)
    : m_url(url)
{
    m_cookie = "0";
/*
        Buffer k;
        for (unsigned i = 0; i < 48; i++){
            char c = get_random() & 0xFF;
            k.pack(&c, 1);
        }
        m_seed += k.toBase64();
*/
    m_seed = "foo";
}

JabberHttpPool::~JabberHttpPool()
{
}

QString JabberHttpPool::getKey()
{
    if (m_key.isEmpty()){
        m_key = m_seed;
        return m_key;
    }
    QByteArray digest = QCryptographicHash::hash(m_key, QCryptographicHash::Sha1);
    Buffer b;
    b.pack(digest, digest.size());
    m_key = b.toBase64();
    return m_key;
}

int JabberHttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == (unsigned)readData.size())
        readData.init(0);
    return size;
}

void JabberHttpPool::write(const char *buf, unsigned size)
{
    writeData.pack(buf, size);
    if (!isDone())
        return;
    Buffer *packet = new Buffer;
    *packet << (const char*)m_cookie.toLocal8Bit().data();
    *packet << ";" << (const char*)getKey().toLocal8Bit().data();
    *packet << ",";
    log(L_DEBUG, "%s;%s,", qPrintable(m_cookie), qPrintable(getKey()));
    packet->pack(writeData.data(), writeData.writePos());
    char headers[] = "Content-Type: application/x-www-form-urlencoded";
    fetch(m_url, headers, packet);
    writeData.init(0);
}

void JabberHttpPool::close()
{
    writeData.init(0);
    stop();
}

void JabberHttpPool::connect(const QString&, unsigned short)
{
    if (notify)
        notify->connect_ready();
}

bool JabberHttpPool::done(unsigned code, Buffer &data, const QString &headers)
{
    if (code != 200){
        log(L_DEBUG, "HTTP result %u", code);
        error("Bad result");
        return false;
    }
    QString cookie;
    int idx = headers.indexOf("Set-Cookie:");
    if(idx != -1) {
        int end = headers.indexOf("\n", idx);
        if(end == -1)
            m_cookie = headers.mid(idx);
        else
            m_cookie = headers.mid(end - idx + 1);
    }
    m_cookie = cookie;
    int err_code = getToken(cookie, ':').toInt();
    if (cookie == "0"){
        const char *err = "Unknown poll error";
        switch (err_code){
        case -1:
            err = "Server Error";
            break;
        case -2:
            err = "Bad Request";
            break;
        case -3:
            err = "Key Sequence Error";
            break;
        }
        error(err);
        return false;
    }
    readData = data;
    if (notify)
        notify->read_ready();
    return false;
}

unsigned long JabberHttpPool::localHost()
{
    return 0;
}

void JabberHttpPool::pause(unsigned)
{
}

Socket *JabberClient::createSocket()
{
    m_bHTTP = getUseHTTP() && !getURL().isEmpty();
    if (m_bHTTP)
        return new JabberHttpPool(getURL());
    return NULL;
}


