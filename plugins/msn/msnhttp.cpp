/***************************************************************************
                          msnhttp.cpp  -  description
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
#include <QByteArray>

#include "fetch.h"
#include "log.h"

#include "msnhttp.h"
#include "msnclient.h"

using namespace SIM;

const unsigned POLL_TIMEOUT	= 10;

// ______________________________________________________________________________________

MSNHttpPool::MSNHttpPool(MSNClient *client, bool bSB)
{
    m_client   = client;
    m_bSB = bSB;
    writeData = new Buffer;
}

MSNHttpPool::~MSNHttpPool()
{
    delete writeData;
}

int MSNHttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == (unsigned)readData.size())
        readData.init(0);
    return size;
}

static char MSN_HTTP[] = "/gateway/gateway.dll?";

void MSNHttpPool::write(const char *buf, unsigned size)
{
    writeData->pack(buf, size);
    if (!isDone())
        return;
    QString url = "http://";
    if (m_session_id.isEmpty()){
        url += "gateway.messenger.hotmail.com";
        url += MSN_HTTP;
        url += "Action=open&Server=";
        url += m_bSB ? "SB" : "NS";
        url += "&IP=";
        url += m_ip;
    }else{
        url += m_host;
        url += MSN_HTTP;
        if (writeData->writePos() == 0)
            url += "Action=poll&";
        url += "SessionID=" + m_session_id;
    }
    const char *headers =
        "Content-Type: application/x-msn-messenger\n"
        "Proxy-Connection: Keep-Alive";
    fetch(url, headers, writeData);
    writeData = new Buffer;
}

void MSNHttpPool::close()
{
    delete writeData;
    writeData = new Buffer;
    m_session_id = QString::null;
    m_host = QString::null;
    stop();
}

void MSNHttpPool::connect(const QString &host, unsigned short)
{
    m_ip = host;
    if (notify)
        notify->connect_ready();
}

void MSNHttpPool::idle()
{
    if (isDone() && (m_client->isDone())){
        log(L_DEBUG, "send idle");
        write("", 0);
    }
}

bool MSNHttpPool::done(unsigned code, Buffer &data, const QString &headers)
{
    if (code != 200){
        log(L_DEBUG, "HTTP result %u", code);
        error("Bad result");
        return false;
    }
    const QStringList sl = headers.split(QLatin1Char('\0'));
    Q_FOREACH(QString h, sl) {
        if (getToken(h, ':') == QLatin1String("X-MSN-Messenger")){
            QString h = h.trimmed ();
            while (!h.isEmpty()){
                QString part = getToken(h, ';');
                QString v = part.trimmed ();
                QString k = getToken(v, '=');
                if (k == "SessionID"){
                    m_session_id = v;
                }else if (k == "GW-IP"){
                    m_host = v;
                }
            }
            break;
        }
    }
    if (m_session_id.isEmpty() || m_host.isEmpty()){
        error("No session in answer");
        return false;
    }
    readData.pack(data.data(), data.writePos());
    if (notify)
        notify->read_ready();
    QTimer::singleShot(POLL_TIMEOUT * 1000, this, SLOT(idle()));
    return false;
}

unsigned long MSNHttpPool::localHost()
{
    return 0;
}

void MSNHttpPool::pause(unsigned)
{
}

Socket *MSNClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
        if (!m_bFirstTry)
            m_bFirstTry = true;
    }
    if (m_bHTTP)
        return new MSNHttpPool(this, false);
    return NULL;
}

Socket *MSNClient::createSBSocket()
{
    if (m_bHTTP)
        return new MSNHttpPool(this, true);
    return NULL;
}

