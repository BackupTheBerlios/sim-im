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

    void snac(unsigned short food, unsigned short type, bool msgId = false, bool bType = true);
    void sendPacket(bool bSend = true);

protected:
    //virtual ICQClientSocket *socket() = 0;
    //virtual void packet(unsigned long size) = 0;
    void flap(char channel);
    void connect_ready();
    void packet_ready();

signals:

public slots:

private:
    bool m_bHeader;
    char m_nChannel;
    unsigned short m_nFlapSequence;
    unsigned short m_nMsgSequence;
    SIM::AsyncSocket* m_socket;
};

#endif // OSCARSOCKET_H
