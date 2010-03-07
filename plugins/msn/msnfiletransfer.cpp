/***************************************************************************
                          msnfiltetransfer.cpp  -  description
                             -------------------
    begin                : Fri Jan 05 2007
    copyright            : (C) 2007 Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QTimer>
#include <QRegExp>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>

#include "simapi.h"

#include "log.h"
#include "message.h"
#include "misc.h"

#include "msn.h"
#include "msnclient.h"
#include "socket/clientsocket.h"

using namespace SIM;

const unsigned long FT_TIMEOUT		= 60;
const unsigned MAX_FT_PACKET		= 2045;

MSNFileTransfer::MSNFileTransfer(FileMessage *msg, MSNClient *client, MSNUserData *data)
        : FileTransfer(msg)
{
    m_socket = new ClientSocket(this);
    m_client = client;
    m_state  = None;
    m_data	 = data;
    m_timer  = NULL;
    m_size   = msg->getSize();
    m_bHeader  = false;
    m_nFiles   = 1;
}

MSNFileTransfer::~MSNFileTransfer()
{
    if (m_socket)
        delete m_socket;
}


void MSNFileTransfer::setSocket(Socket *s)
{
    m_state  = Incoming;
    m_socket->setSocket(s);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
    send("VER MSNFTP");
    FileTransfer::m_state = FileTransfer::Negotiation;
    if (m_notify)
        m_notify->process();
}

void MSNFileTransfer::listen()
{
    if (m_notify)
        m_notify->createFile(m_msg->getDescription(), m_size, false);
}

void MSNFileTransfer::connect()
{
    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();
    if ((m_state == None) || (m_state == Wait)){
        m_state = ConnectIP1;
        if (ip1 && port1){
            m_socket->connect(QHostAddress(ip1).toString(), port1, NULL);
            return;
        }
    }
    if (m_state == ConnectIP1){
        m_state = ConnectIP2;
        if (ip2 && port2){
            m_socket->connect(QHostAddress(ip2).toString(), port2, NULL);
            return;
        }
    }
    if (m_state == ConnectIP2){
        m_state = ConnectIP3;
        if (ip2 && port1){
            m_socket->connect(QHostAddress(ip2).toString(), port1, NULL);
            return;
        }
    }
    error_state(I18N_NOOP("Can't established direct connection"), 0);
}

bool MSNFileTransfer::error_state(const QString &err, unsigned)
{
    if (m_state == WaitDisconnect)
        FileTransfer::m_state = FileTransfer::Done;
    if (m_state == ConnectIP1){
        connect();
        return false;
    }
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

void MSNFileTransfer::packet_ready()
{
    if (m_state == Receive){
        if (m_bHeader){
            char cmd;
            char s1, s2;
            m_socket->readBuffer() >> cmd >> s1 >> s2;
            log(L_DEBUG, "MSN FT header: %02X %02X %02X", cmd & 0xFF, s1 & 0xFF, s2 & 0xFF);
            if (cmd != 0){
                m_socket->error_state(I18N_NOOP("Transfer canceled"), 0);
                return;
            }
            unsigned size = (unsigned char)s1 + ((unsigned char)s2 << 8);
            m_bHeader = false;
            log(L_DEBUG, "MSN FT header: %u", size);
            m_socket->readBuffer().init(size);
        }else{
            unsigned size = m_socket->readBuffer().size();
            if (size == 0)
                return;
            log(L_DEBUG, "MSN FT data: %u", size);
            m_file->write(m_socket->readBuffer().data(), size);
            m_socket->readBuffer().incReadPos(size);
            m_bytes      += size;
            m_totalBytes += size;
            m_transferBytes += size;
            if (m_notify)
                m_notify->process();
            m_size -= size;
            if (m_size <= 0){
                m_socket->readBuffer().init(0);
                m_socket->setRaw(true);
                send("BYE 16777989");
                m_state = WaitDisconnect;
                if (m_notify)
                    m_notify->transfer(false);
                return;
            }
            m_bHeader = true;
            m_socket->readBuffer().init(3);
        }
        return;
    }
    if (m_socket->readBuffer().writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->readBuffer(), false, plugin->MSNPacket);
    for (;;){
        QByteArray s;
        if (!m_socket->readBuffer().scan("\r\n", s))
            break;
        if (getLine(s))
            return;
    }
    if (m_socket->readBuffer().readPos() == m_socket->readBuffer().writePos())
        m_socket->readBuffer().init(0);
}

void MSNFileTransfer::connect_ready()
{
    log(L_DEBUG, "Connect ready");
    m_state = Connected;
    FileTransfer::m_state = Negotiation;
    if (m_notify)
        m_notify->process();
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
}

void MSNFileTransfer::startReceive(unsigned pos)
{
    if (pos > m_size){
        SBSocket *sock = dynamic_cast<SBSocket*>(m_data->sb.object());
        FileTransfer::m_state = FileTransfer::Done;
        m_state = None;
        if (sock)
            sock->declineMessage(cookie);
        m_socket->error_state("", 0);
        return;
    }
    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(FT_TIMEOUT * 1000);
    m_state = Listen;
    FileTransfer::m_state = FileTransfer::Listen;
    if (m_notify)
        m_notify->process();
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
}

void MSNFileTransfer::send(const QString &line)
{
    log(L_DEBUG, "Send: %s", qPrintable(line));
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer() << (const char*)line.toUtf8();
    m_socket->writeBuffer() << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    EventLog::log_packet(m_socket->writeBuffer(), true, plugin->MSNPacket);
    m_socket->write();
}

bool MSNFileTransfer::getLine(const QByteArray &line)
{
    QString l = QString::fromUtf8(line);
    l = l.remove('\r');
    log(L_DEBUG, "Get: %s", qPrintable(l));

    QString cmd = getToken(l, ' ');
    if ((cmd == "VER") && (l == "MSNFTP")){
        if (m_state == Incoming){
            QString usr = "USR ";
            usr += m_client->quote(m_client->data.owner.EMail.str());
            usr += " ";
            usr += QString::number(auth_cookie);
            send(usr);
        }else{
            send("VER MSNFTP");
        }
        return false;
    }
    if (cmd == "USR"){
        QString mail = m_client->unquote(getToken(l, ' '));
        unsigned auth = l.toUInt();
        if (mail.toLower() != m_data->EMail.str().toLower()){
            error_state("Bad address", 0);
            return false;
        }
        if (auth != auth_cookie){
            error_state("Bad auth cookie", 0);
            return false;
        }
        if (m_file == NULL){
            for (;;){
                if (!openFile()){
                    if (FileTransfer::m_state == FileTransfer::Done)
                        m_socket->error_state("");
                    if (m_notify)
                        m_notify->transfer(false);
                    return false;
                }
                if (!isDirectory())
                    break;
            }
        }
        QString cmd = "FIL ";
        cmd += QString::number(m_fileSize);
        send(cmd);
        return false;
    }
    if (cmd == "TFR"){
        FileTransfer::m_state = FileTransfer::Write;
        m_state = Send;
        if (m_notify)
            m_notify->transfer(true);
        write_ready();
        return false;
    }
    if (cmd == "FIL"){
        send("TFR");
        m_bHeader = true;
        m_socket->readBuffer().init(3);
        m_socket->readBuffer().packetStart();
        m_state = Receive;
        m_socket->setRaw(false);
        FileTransfer::m_state = FileTransfer::Read;
        m_size = l.toULong();
        m_bytes = 0;
        if (m_notify){
            m_notify->transfer(true);
            m_notify->process();
        }
        return true;
    }
    if (cmd == "BYE"){
        if (m_notify)
            m_notify->transfer(false);
        for (bool doloop=true;doloop;){
            if (!openFile()){
                if (FileTransfer::m_state == FileTransfer::Done)
                    m_socket->error_state("");
                return true;
            }
            if (isDirectory()){
                doloop=false;
                continue;
            }
            m_state = Wait;
            FileTransfer::m_state = FileTransfer::Wait;
            if (!((Client*)m_client)->send(m_msg, m_data))
                error_state(I18N_NOOP("File transfer failed"), 0);
        }
        if (m_notify)
            m_notify->process();
        m_socket->close();
        return true;
    }
    error_state("Bad line", 0);
    return false;
}

void MSNFileTransfer::timeout()
{
}

void MSNFileTransfer::write_ready()
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
    if (m_bytes >= m_fileSize){
        m_state = WaitBye;
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
    unsigned long tail = m_fileSize - m_bytes;
    if (tail > MAX_FT_PACKET) tail = MAX_FT_PACKET;
    m_socket->writeBuffer().packetStart();
    char buf[MAX_FT_PACKET + 3];
    buf[0] = 0;
    buf[1] = (char)(tail & 0xFF);
    buf[2] = (char)((tail >> 8) & 0xFF);
    int readn = m_file->read(&buf[3], tail);
    if (readn <= 0){
        m_socket->error_state("Read file error");
        return;
    }
    m_transfer    = readn;
    m_bytes      += readn;
    m_totalBytes += readn;
    m_sendSize   += readn;
    m_socket->writeBuffer().pack(buf, readn + 3);
    m_socket->write();
}

bool MSNFileTransfer::accept(Socket *s, unsigned long ip)
{
    log(L_DEBUG, "Accept direct connection %s", qPrintable(QHostAddress(ip).toString()));
    m_socket->setSocket(s);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    m_socket->setRaw(true);
    FileTransfer::m_state = Negotiation;
    m_state = Incoming;
    if (m_notify)
        m_notify->process();
    send("VER MSNFTP");
    return true;
}

void MSNFileTransfer::bind_ready(unsigned short port)
{
    SBSocket *sock = dynamic_cast<SBSocket*>(m_data->sb.object());
    if (sock == NULL){
        error_state("No switchboard socket", 0);
        return;
    }
    sock->acceptMessage(port, cookie, auth_cookie);
}

bool MSNFileTransfer::error(const QString &err)
{
    return error_state(err, 0);
}
