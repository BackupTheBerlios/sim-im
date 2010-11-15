#ifndef OSCARSOCKET_H
#define OSCARSOCKET_H

#include "network/asyncsocket.h"
#include "icq_defines.h"

#include "stdint.h"

class ICQ_EXPORT OscarSocket : public QObject
{
    Q_OBJECT
public:
    explicit OscarSocket(QObject *parent = 0);
    virtual ~OscarSocket();

    virtual void connectToHost(const QString& host, int port) = 0;
    virtual void disconnectFromHost() = 0;

    virtual void flap(int channel, int length) = 0;
    virtual void snac(int type, int subtype, int requestId, const QByteArray& data) = 0;

    static const char FlapChannelNewConnection = 0x01;
    static const char FlapChannelSnac = 0x02;
    static const char FlapChannelError = 0x03;
    static const char FlapChannelCloseConnection = 0x04;
    static const char FlapChannelKeepAlive = 0x05;

signals:
    void error(const QString& errmsg);
    void packet(const QByteArray& data);
};

#endif // OSCARSOCKET_H
