/***************************************************************************
                          yahooclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 * Based on libyahoo2
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *             1998-1999, Adam Fritzler <afritz@marko.net>
 *             1998-2002, Rob Flynn <rob@marko.net>
 *             2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *             2001-2002, Brian Macke <macke@strangelove.net>
 *                  2001, Anand Biligiri S <abiligiri@users.sf.net>
 *                  2001, Valdis Kletnieks
 *                  2002, Sean Egan <bj91704@binghamton.edu>
 *                  2002, Toby Gray <toby.gray@ntlworld.com>
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <list>

#include <QFile>
#include <QByteArray>
#include <QDateTime>

#include "simapi.h"

#include "log.h"
#include "message.h"
#include "misc.h"
#include "socket/clientsocket.h"

#include "yahoo.h"
#include "yahooclient.h"

using namespace SIM;
using namespace std;

YahooFileTransfer::YahooFileTransfer(FileMessage *msg, YahooUserData *data, YahooClient *client)
        : FileTransfer(msg)
{
    m_data   = data;
    m_client = client;
    m_state  = None;
    m_socket = new ClientSocket(this);
    m_startPos = 0;
    m_endPos   = 0xFFFFFFFF;
}

YahooFileTransfer::~YahooFileTransfer()
{
    for (list<Message_ID>::iterator it = m_client->m_waitMsg.begin(); it != m_client->m_waitMsg.end(); ++it){
        if (it->msg == m_msg){
            m_client->m_waitMsg.erase(it);
            break;
        }
    }
    if (m_socket)
        delete m_socket;
}

void YahooFileTransfer::listen()
{
    if (m_file == NULL){
        for (;;){
            if (!openFile()){
                if (FileTransfer::m_state == FileTransfer::Done)
                    m_socket->error_state("");
                return;
            }
            if (!isDirectory())
                break;
        }
        return;
    }
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
}

void YahooFileTransfer::startReceive(unsigned pos)
{
    m_startPos = pos;
    YahooFileMessage *msg = static_cast<YahooFileMessage*>(m_msg);
    QString proto, user, pass, uri, extra;
    unsigned short port;
    FetchClient::crackUrl(msg->getUrl(), proto, m_host, port, user, pass, m_url, extra);
    if (!extra.isEmpty()){
        m_url += "?";
        m_url += extra;
    }
    m_socket->connect(m_host, port, m_client);
    m_state = Connect;
    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();
}

void YahooFileTransfer::bind_ready(unsigned short port)
{
    if (m_state == None){
        m_state = Listen;
    }else{
        m_state = ListenWait;
        FileTransfer::m_state = FileTransfer::Listen;
        if (m_notify)
            m_notify->process();
    }
    m_client->sendFile(m_msg, m_file, m_data, port);
}

bool YahooFileTransfer::error(const QString &err)
{
    error_state(err, 0);
    return true;
}

bool YahooFileTransfer::accept(Socket *s, unsigned long)
{
    if (m_state == Listen){
        EventMessageAcked(m_msg).process();
    }
    m_state = ListenWait;
    log(L_DEBUG, "Accept connection");
    m_startPos = 0;
    m_endPos   = 0xFFFFFFFF;
    Socket *old_s = m_socket->socket();
    m_socket->setSocket(s);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
    m_answer = 400;
    if (old_s)
        delete old_s;
    return false;
}

bool YahooFileTransfer::error_state(const QString &err, unsigned)
{
    if ((m_state == Wait) || (m_state == Skip))
        return false;
    if (FileTransfer::m_state != FileTransfer::Done){
        m_state = None;
        FileTransfer::m_state = FileTransfer::Error;
        m_msg->setError(err);
        if (m_notify)
            m_notify->process();
    }
    m_msg->m_transfer = NULL;
    m_msg->setFlags(m_msg->getFlags() & ~MESSAGE_TEMP);
    EventMessageSent(m_msg).process();
    return true;
}

void YahooFileTransfer::packet_ready()
{
    if (m_socket->readBuffer().writePos() == 0)
        return;
    if (m_state == Skip)
        return;
    if (m_state != Receive){
        EventLog::log_packet(m_socket->readBuffer(), false, YahooPlugin::YahooPacket);
        for (;;){
            QByteArray s;
            if (!m_socket->readBuffer().scan("\n", s))
                break;
            if (!s.isEmpty() && (s[(int)s.length() - 1] == '\r'))
                s = s.left(s.length() - 1);
            if (!get_line(s))
                break;
        }
    }
    if (m_state == Receive){
        if (m_file == NULL){
            m_socket->error_state("", 0);
            return;
        }
        unsigned size = m_socket->readBuffer().size() - m_socket->readBuffer().readPos();
        if (size > m_endPos - m_startPos)
            size = m_endPos - m_startPos;
        if (size){
            m_file->write(m_socket->readBuffer().data(m_socket->readBuffer().readPos()), size);
            m_bytes += size;
            m_totalBytes += size;
            m_startPos += size;
            m_transferBytes += size;
            if (m_startPos == m_endPos){
                FileTransfer::m_state = FileTransfer::Done;
                if (m_notify){
                    m_notify->transfer(false);
                    m_notify->process();
                }
                m_socket->error_state("");
            }
            if (m_notify)
                m_notify->process();
        }
    }
    if (m_socket->readBuffer().readPos() == m_socket->readBuffer().writePos())
        m_socket->readBuffer().init(0);
}

void YahooFileTransfer::connect_ready()
{
    QString line;
    line = "GET /";
    line += m_url;
    line += " HTTP/1.1\r\n"
            "Host :";
    line += m_host;
    line += "\r\n";
    if (m_startPos){
        line += "Range: ";
        line += QString::number(m_startPos);
        line += "-\r\n";
    }
    m_startPos = 0;
    m_endPos   = 0xFFFFFFFF;
    send_line(line);
    FileTransfer::m_state = FileTransfer::Negotiation;
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
}

void YahooFileTransfer::write_ready()
{
    if (m_state != Send){
        ClientSocketNotify::write_ready();
        return;
    }
    if (m_transfer){
        m_transferBytes += m_transfer;
        m_transfer = 0;
        if (m_notify)
            m_notify->process();
    }
    if (m_startPos >= m_endPos){
        if (m_notify)
            m_notify->transfer(false);
        m_bytes += m_file->size() - m_endPos;
        m_totalBytes += m_file->size() - m_endPos;
        for (;;){
            if (!openFile()){
                m_state = None;
                if (FileTransfer::m_state == FileTransfer::Done)
                    m_socket->error_state("");
            }else{
                if (isDirectory())
                    continue;
                m_state = Wait;
                FileTransfer::m_state = FileTransfer::Wait;
                if (!((Client*)m_client)->send(m_msg, m_data))
                    error_state(I18N_NOOP("File transfer failed"), 0);
                break;
            }
        }
        if (m_notify)
            m_notify->process();
        m_socket->close();
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
    char buf[2048];
    unsigned tail = sizeof(buf);
    if (tail > m_endPos - m_startPos)
        tail = m_endPos - m_startPos;
    int readn = m_file->read(buf, tail);
    if (readn <= 0){
        m_socket->error_state("Read file error");
        return;
    }
    m_startPos   += readn;
    m_transfer    = readn;
    m_bytes      += readn;
    m_totalBytes += readn;
    m_sendSize   += readn;
    m_socket->writeBuffer().pack(buf, readn);
    m_socket->write();
}

bool YahooFileTransfer::get_line(const QByteArray &_line)
{
    QByteArray line = _line;
    if (line.isEmpty()){
        if (m_state == Connect){
            m_socket->error_state(I18N_NOOP("File transfer failed"));
            return true;
        }
        if (m_state == ReadHeader){
            if (m_endPos < m_startPos)
                m_endPos = m_startPos;
            if (m_file)
                m_file->seek(m_startPos);
            m_state = Receive;
            FileTransfer::m_state = FileTransfer::Read;
            m_bytes += m_startPos;
            m_totalBytes += m_startPos;
            m_fileSize = m_endPos;
            m_totalSize = m_endPos;
            if (m_notify){
                m_notify->process();
                m_notify->transfer(true);
            }
            return true;
        }
        if (m_file->size() < m_endPos)
            m_endPos = m_file->size();
        if (m_startPos > m_endPos)
            m_startPos = m_endPos;
        if ((m_answer == 200) && (m_startPos == m_endPos))
            m_answer = 204;
        if ((m_answer == 200) && ((m_startPos != 0) || (m_endPos < m_file->size())))
            m_answer = 206;
        QString s;
        s = "HTTP/1.0 ";
        s += QString::number(m_answer);
        switch (m_answer){
        case 200:
            s += " OK";
            break;
        case 204:
            s += " No content";
            break;
        case 206:
            s += " Partial content";
            break;
        case 400:
            s += " Bad request";
            break;
        case 404:
            s += " Not found";
            break;
        default:
            s += " Error";
        }
        send_line(s);
        if ((m_answer == 200) || (m_answer == 206)){
            send_line("Content-Type: application/data");
            s = "Content-Length: ";
            s += QString::number(m_endPos - m_startPos);
            send_line(s);
        }
        if (m_answer == 206){
            s = "Range: ";
            s += QString::number(m_startPos);
            s += "-";
            s += QString::number(m_endPos);
            send_line(s);
        }
        send_line("");
        if (m_method == "HEAD"){
            m_state = Skip;
            return false;
        }
        if (m_answer < 300){
            m_file->seek(m_startPos);
            FileTransfer::m_state = FileTransfer::Write;
            m_state = Send;
            m_bytes = m_startPos;
            m_totalBytes += m_startPos;
            if (m_notify){
                m_notify->process();
                m_notify->transfer(true);
            }
            write_ready();
        }else{
            m_socket->error_state("Bad request");
        }
        return false;
    }
    if (m_state == ListenWait){
        QByteArray t = getToken(line, ' ');
        if ((t == "GET") || (t == "HEAD")){
            m_method = t;
            m_answer = 200;
        }
        m_state = Header;
        return true;
    }
    if (m_state == Connect){
        QByteArray t = getToken(line, ' ');
        t = getToken(t, '/');
        if (t != "HTTP"){
            m_socket->error_state(I18N_NOOP("File transfer fail"));
            return true;
        }
        unsigned code = getToken(line, ' ').toUInt();
        switch (code){
        case 200:
        case 206:
            m_startPos = 0;
            m_endPos   = 0xFFFFFFFF;
            break;
        case 204:
            m_startPos = 0;
            m_endPos   = 0;
            break;
        }
        m_state = ReadHeader;
        return true;
    }
    if (m_state == ReadHeader){
        QByteArray t = getToken(line, ':');
        // FIXME: this should be easier with QCString::find() !
        if ((t == "Content-Length") || (t == "Content-length")){
            const char *p;
            for (p = line.data(); *p; p++)
                if ((*p > '0') && (*p < '9'))
                    break;
            m_endPos = m_startPos + strtoul(p, NULL, 10);
        }
        if (t == "Range"){
            const char *p;
            for (p = line.data(); *p; p++)
                if ((*p > '0') && (*p < '9'))
                    break;
            m_startPos = strtoul(p, NULL, 10);
            for (; *p; p++)
                if (*p == '-'){
                    ++p;
                    break;
                }
            if ((*p > '0') && (*p < '9'))
                m_endPos = m_startPos + strtoul(p, NULL, 10);
        }
        return true;
    }
    QByteArray t = getToken(line, ':');
    if (t == "Range"){
        const char *p = line.data();
        for (; *p; p++)
            if (*p != ' ')
                break;
        m_startPos = strtoul(p, NULL, 10);
        for (; *p; p++)
            if (*p == '-'){
                p++;
                break;
            }
        if ((*p >= '0') && (*p <= '9'))
            m_endPos = strtoul(p, NULL, 10);
    }
    return true;
}

void YahooFileTransfer::send_line(const QString &line)
{
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer() << (const char*)line.toUtf8();
    m_socket->writeBuffer() << "\r\n";
    EventLog::log_packet(m_socket->writeBuffer(), true, YahooPlugin::YahooPacket);
    m_socket->write();
}

void YahooFileTransfer::connect()
{
    m_nFiles = 1;
    if (m_notify)
        m_notify->createFile(m_msg->getDescription(), 0xFFFFFFFF, false);
}
