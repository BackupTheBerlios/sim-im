#ifndef STANDARDOSCARSOCKET_H
#define STANDARDOSCARSOCKET_H

#include "oscarsocket.h"
#include "icq_defines.h"

class ICQ_EXPORT StandardOscarSocket : public OscarSocket
{
    Q_OBJECT
public:
    StandardOscarSocket(QObject *parent = 0);
    virtual ~StandardOscarSocket();
    void connectToHost(const QString& host, int port);
    void disconnectFromHost();

    // takes ownership of socket
    void setSocket(SIM::AsyncSocket* socket);

    void flap(int channel, const QByteArray& data);
    void snac(int type, int subtype, int requestId, const QByteArray& data);

    static const char FlapChannelNewConnection = 0x01;
    static const char FlapChannelSnac = 0x02;
    static const char FlapChannelError = 0x03;
    static const char FlapChannelCloseConnection = 0x04;
    static const char FlapChannelKeepAlive = 0x05;

protected:
    QByteArray makeFlapPacket(int channel, int length);
    QByteArray makeSnacHeader(int type, int subtype, int requestId);

protected slots:
    void readyRead();
    void slot_connected();

private:
    static const char FlapId = 0x2a;

    static const int SizeOfFlapHeader = 6;
    static const int SizeOfSnacHeader = 10;

    bool m_bHeader;
    QByteArray m_packet;
    int m_packetLength;
    char m_nChannel;
    unsigned short m_nFlapSequence;
    unsigned short m_nMsgSequence;
    SIM::AsyncSocket* m_socket;
};

#endif // STANDARDOSCARSOCKET_H
