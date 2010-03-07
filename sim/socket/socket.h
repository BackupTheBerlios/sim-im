/***************************************************************************
                          socket.h  -  description
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

#ifndef _SOCKET_H
#define _SOCKET_H

#include <QObject>
#include <map>
#include <string>
#include "buffer.h"
#include "contacts.h"
#include "event.h"
#include "simapi.h"
#include "socketnotify.h"
#include "clientsocketnotify.h"

namespace SIM
{
    class EXPORT Socket
    {
    protected:
        QString m_host;
        unsigned short m_port;
        SocketNotify *notify;
    public:
        Socket() : m_port(0), notify(NULL) {}
        virtual ~Socket() {}
        virtual int read(char *buf, unsigned int size) = 0;
        virtual void write(const char *buf, unsigned int size) = 0;
        virtual void connect(const QString &host, unsigned short port)
        {
            m_host = host;
            m_port = port;
        }
        virtual void close() = 0;
        virtual unsigned long localHost() = 0;
        virtual void pause(unsigned) = 0;
        void error(const QString &err_text, unsigned code=0);
        void setNotify(SocketNotify *n) { notify = n; }
        enum Mode
        {
            Direct,
            Indirect,
            Web
        };
        virtual Mode mode() const { return Direct; }
        const QString &getHost() const { return m_host; }
        unsigned short getPort() const { return m_port; }
        const SocketNotify *getNotify() const { return notify; }
        virtual bool isEncrypted() = 0;
        virtual bool startEncryption() = 0;
        virtual int getFd() { return 0;}; // kind of hack
    };
}

#endif

// vim: set expandtab:
