/***************************************************************************
                          jabberclient.cpp  -  description
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
#include "buffer.h"
#include "log.h"
#include "message.h"
#include "misc.h"

#include "jabber.h"
#include "jabberclient.h"
#include "jabbermessage.h"

#include <list>

#include <QRegExp>
#include <QByteArray>
#include <QDateTime>

using namespace SIM;
using namespace std;

JabberFileTransfer::JabberFileTransfer(FileMessage *msg, JabberUserData *data, JabberClient *client)
        : FileTransfer(msg)
{
    m_data   = data;
    m_client = client;
    m_state  = None;
    m_socket = new JabberClientSocket(this);
    m_startPos = 0;
    m_endPos   = 0xFFFFFFFF;
}

JabberFileTransfer::~JabberFileTransfer()
{
    for (list<Message*>::iterator it = m_client->m_waitMsg.begin(); it != m_client->m_waitMsg.end(); ++it){
        if ((*it) == m_msg){
            m_client->m_waitMsg.erase(it);
            break;
        }
    }
    if (m_socket)
        delete m_socket;
}

void JabberFileTransfer::listen()
{
    if (m_file == NULL){
        for (;;){
            if (!openFile()){
                if (FileTransfer::m_state == FileTransfer::Done)
                    m_socket->error_state(QString::null);
                return;
            }
            if (!isDirectory())
                break;
        }
    }
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
}

void JabberFileTransfer::startReceive(unsigned pos)
{
    m_startPos = pos;
    JabberFileMessage *msg = static_cast<JabberFileMessage*>(m_msg);
    m_socket->connect(msg->getHost(), msg->getPort(), m_client);
    m_state = Connect;
    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();
}

void JabberFileTransfer::bind_ready(unsigned short port)
{
    if (m_state == None){
        m_state = Listen;
    }else{
        m_state = ListenWait;
        FileTransfer::m_state = FileTransfer::Listen;
        if (m_notify)
            m_notify->process();
    }
    QString fname = m_file->fileName();
    fname = fname.replace('\\', '/');
    int n = fname.lastIndexOf('/');
    if (n >= 0)
        fname = fname.mid(n + 1);
    m_url = fname;
    m_client->sendFileRequest(m_msg, port, m_data, m_url, m_fileSize);
}

bool JabberFileTransfer::error(const QString &err)
{
    error_state(err, 0);
    return true;
}

bool JabberFileTransfer::accept(Socket *s, unsigned long)
{
    if (m_state == Listen){
        EventMessageAcked(m_msg).process();
        m_state = ListenWait;
    }
    log(L_DEBUG, "Accept connection");
    m_startPos = 0;
    m_endPos   = 0xFFFFFFFF;
    m_socket->setSocket(s);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
    m_answer = 400;
    return true;
}

bool JabberFileTransfer::error_state(const QString &err, unsigned)
{
    if (m_state == Wait)
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

void JabberFileTransfer::packet_ready()
{
    if (m_socket->readBuffer().writePos() == 0)
        return;
    if (m_state != Receive){
        JabberPlugin *plugin = static_cast<JabberPlugin*>(m_client->protocol()->plugin());
        EventLog::log_packet(m_socket->readBuffer(), false, plugin->JabberPacket);
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
            m_socket->error_state(QString::null, 0);
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
                m_socket->error_state(QString::null);
            }
            if (m_notify)
                m_notify->process();
        }
    }
    if (m_socket->readBuffer().readPos() == m_socket->readBuffer().writePos())
        m_socket->readBuffer().init(0);
}

void JabberFileTransfer::connect_ready()
{
    JabberFileMessage *msg = static_cast<JabberFileMessage*>(m_msg);
    QString line;
    line = "GET /";
    line += msg->getDescription();
    line += " HTTP/1.1\r\n"
            "Host :";
    line += msg->getHost();
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

void JabberFileTransfer::write_ready()
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
                    m_socket->error_state(QString::null);
                break;
            }
            if (isDirectory())
                continue;
            m_state = Wait;
            FileTransfer::m_state = FileTransfer::Wait;
            if (!((Client*)m_client)->send(m_msg, m_data))
                error_state(I18N_NOOP("File transfer failed"), 0);
            break;
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

bool JabberFileTransfer::get_line(const QByteArray &str)
{
    QByteArray line = str;
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
            s += '-';
            s += QString::number(m_endPos);
            send_line(s);
        }
        send_line("");
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
        if (t == "GET"){
            m_answer = 404;
            t = getToken(line, ' ');
            if (t[0] == '/'){
                if (m_url == QString::fromUtf8((t.data() + 1)))
                    m_answer = 200;
            }
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
        if (t == "Content-Length"){
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

void JabberFileTransfer::send_line(const QString &line)
{
    send_line(line.toUtf8().data());
}

void JabberFileTransfer::send_line(const QByteArray &line)
{
    send_line(line.data());
}

void JabberFileTransfer::send_line(const char *line)
{
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer() << line << "\r\n";
    JabberPlugin *plugin = static_cast<JabberPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->JabberPacket);
    m_socket->write();
}

void JabberFileTransfer::connect()
{
    m_nFiles = 1;
    if (static_cast<JabberFileMessage*>(m_msg)->getPort() == 0)
        m_client->sendFileAccept(m_msg, m_data);
    if (m_notify)
        m_notify->createFile(m_msg->getDescription(), 0xFFFFFFFF, false);
}
