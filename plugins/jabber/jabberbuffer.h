/***************************************************************************
                          jabberbuffer.h  -  description
                             -------------------
    begin                : Mon Jan 08 2006
    copyright            : (C) 2007 by Christian Ehrlicher
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
#ifndef _JABBERBUFFER_H
#define _JABBERBUFFER_H

#include "buffer.h"
#include "socket/socket.h"
#include "socket/clientsocket.h"
#include <QByteArray>

class JabberBuffer : public Buffer
{
public:
    JabberBuffer(unsigned size = 0);
    JabberBuffer(const QByteArray &ba);
    virtual ~JabberBuffer();

    JabberBuffer &operator << (const QString &s);
    JabberBuffer &operator << (const QByteArray &s);
    JabberBuffer &operator << (const char *s);      // interpreted as utf-8
};

class JabberClientSocket : public SIM::ClientSocket
{
public:
    JabberClientSocket(SIM::ClientSocketNotify *notify, SIM::Socket *sock = NULL)
        : ClientSocket(notify, sock) {};
    ~JabberClientSocket() {};

    virtual JabberBuffer &readBuffer() { return m_readJabberBuffer; }
    virtual JabberBuffer &writeBuffer() { return m_writeJabberBuffer; }
protected:
    JabberBuffer m_readJabberBuffer;
    JabberBuffer m_writeJabberBuffer;
};

#endif  // _JABBERBUFFER_H
