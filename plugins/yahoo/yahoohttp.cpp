/***************************************************************************
                          yahoohttp.cpp  -  description
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

#include "fetch.h"
#include "log.h"

#include "yahooclient.h"

using namespace SIM;

class YahooHttpPool : public SIM::Socket, public FetchClient
{
public:
    YahooHttpPool();
    ~YahooHttpPool();
    virtual void connect(const QString &host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() const { return Web; }
    virtual bool isEncrypted(){ return false; }
    virtual bool startEncryption(){ return false; }
protected:
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    Buffer readData;
    Buffer *writeData;
    virtual unsigned long localHost();
    virtual void pause(unsigned);
};

// ______________________________________________________________________________________

YahooHttpPool::YahooHttpPool()
{
    writeData = new Buffer;
}

YahooHttpPool::~YahooHttpPool()
{
    delete writeData;
}

int YahooHttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == (unsigned)readData.size())
        readData.init(0);
    return size;
}

static char YAHOO_HTTP[] = "http://shttp.msg.yahoo.com/notify/";

void YahooHttpPool::write(const char *buf, unsigned size)
{
    writeData->pack(buf, size);
    if (!isDone())
        return;
    char headers[] = "Accept: application/octet-stream";
    fetch(YAHOO_HTTP, headers, writeData);
    writeData = new Buffer;
}

void YahooHttpPool::close()
{
    delete writeData;
    writeData = new Buffer;
    stop();
}

void YahooHttpPool::connect(const QString&, unsigned short)
{
    if (notify)
        notify->connect_ready();
}

bool YahooHttpPool::done(unsigned code, Buffer &data, const QString&)
{
    if (code != 200){
        log(L_DEBUG, "HTTP result %u", code);
        error("Bad result");
        return false;
    }
    unsigned long packet_id;
    data >> packet_id;
    log(L_DEBUG, "Packet ID: %lu %lX", packet_id, packet_id);
    readData.pack(data.data(data.readPos()), data.writePos() - data.readPos());
    if (notify)
        notify->read_ready();
    return false;
}

unsigned long YahooHttpPool::localHost()
{
    return 0;
}

void YahooHttpPool::pause(unsigned)
{
}

Socket *YahooClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
        if (!m_bFirstTry)
            m_bFirstTry = true;
    }
    if (m_bHTTP)
        return new YahooHttpPool;
    return NULL;
}


