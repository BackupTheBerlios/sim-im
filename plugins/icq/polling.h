/***************************************************************************
                          polling.h  -  description
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

#ifndef _POLLING_H
#define _POLLING_H

#include <list>
#include "socket/socket.h"

class HttpRequest;
class HttpPacket;

class HttpPool : public QObject, public SIM::Socket
{
    Q_OBJECT
public:
    HttpPool(bool bAIM);
    ~HttpPool();
    virtual void connect(const QString &host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() const { return Web; }
    virtual bool isEncrypted(){ return false; }
    virtual bool startEncryption(){ return false; }
protected slots:
    void timeout();
protected:
    enum State
    {
        None,
        Connected
    };
    State state;

    QString sid;
    QString m_host;
    QString m_url;

    std::list<HttpPacket*> queue;
    unsigned seq;
    unsigned readn;
    Buffer readData;

    HttpRequest *hello;
    HttpRequest *monitor;
    HttpRequest *post;

    unsigned short nSock;
    void   request();
    virtual unsigned long localHost();
    virtual void pause(unsigned);

    bool m_bAIM;

    friend class HttpRequest;
    friend class HelloRequest;
    friend class MonitorRequest;
    friend class PostRequest;
};


#endif

