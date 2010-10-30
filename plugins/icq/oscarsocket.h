#ifndef OSCARSOCKET_H
#define OSCARSOCKET_H

#include "network/asyncsocket.h"
#include "icq_defines.h"

class ICQ_EXPORT OscarSocket : public QObject
{
    Q_OBJECT
public:
    explicit OscarSocket(QObject *parent = 0);
    virtual ~OscarSocket();

    void connectToHost(const QString& host, int port);
    void disconnectFromHost();

    // takes ownership of socket
    void setSocket(SIM::AsyncSocket* socket);

    void flap(int channel, int length);
    void snac(int type, int subtype, int requestId, const QByteArray& data);

protected:
    //virtual ICQClientSocket *socket() = 0;
    //virtual void packet(unsigned long size) = 0;
    void connect_ready();
    void packet_ready();

    QByteArray makeFlapPacket(int channel, int length);
    QByteArray makeSnacHeader(int type, int subtype, int requestId);

signals:

public slots:

private:
    static const char FlapId = 0x2a;

    static const char FlapChannelNewConnection = 0x01;
    static const char FlapChannelSnac = 0x02;
    static const char FlapChannelError = 0x03;
    static const char FlapChannelCloseConnection = 0x04;
    static const char FlapChannelKeepAlive = 0x05;

    static const int SizeOfSnacHeader = 10;

    bool m_bHeader;
    char m_nChannel;
    unsigned short m_nFlapSequence;
    unsigned short m_nMsgSequence;
    SIM::AsyncSocket* m_socket;
};

#endif // OSCARSOCKET_H
