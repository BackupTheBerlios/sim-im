#include "oscarsocket.h"
#include "network/tcpasyncsocket.h"

OscarSocket::OscarSocket(QObject *parent) :
    QObject(parent), m_nFlapSequence(0)
{
    m_socket = new SIM::TcpAsyncSocket(this);
}

OscarSocket::~OscarSocket()
{
    if(m_socket)
        delete m_socket;
}

void OscarSocket::setSocket(SIM::AsyncSocket* socket)
{
    if(m_socket)
        delete m_socket;
    m_socket = socket;
}

void OscarSocket::connectToHost(const QString& host, int port)
{
    m_socket->connectToHost(host, port);
}

void OscarSocket::disconnectFromHost()
{
    m_socket->disconnectFromHost();
}

void OscarSocket::flap(int channel, int length)
{
    QByteArray arr = makeFlapPacket(channel, length);
    m_socket->write(arr);
}

QByteArray OscarSocket::makeFlapPacket(int channel, int length)
{
    QByteArray buf(6, 0);
    buf[0] = FlapId;
    buf[1] = channel;
    buf[2] = (m_nFlapSequence >> 8) & 0xff;
    buf[3] = m_nFlapSequence & 0xff;
    buf[4] = (length >> 8) & 0xff;
    buf[5] = length & 0xff;
    return buf;
}

void OscarSocket::snac(int type, int subtype, int requestId, const QByteArray& data)
{
    QByteArray arr = makeFlapPacket(FlapChannelSnac, data.size() + SizeOfSnacHeader);
    QByteArray snacHeader = makeSnacHeader(type, subtype, requestId);
    m_socket->write(arr + snacHeader + data);
}

QByteArray OscarSocket::makeSnacHeader(int type, int subtype, int requestId)
{
    QByteArray buf(SizeOfSnacHeader, 0);
    buf[0] = (type >> 8) & 0xff;
    buf[1] = type & 0xff;
    buf[2] = (subtype >> 8) & 0xff;
    buf[3] = subtype & 0xff;
    buf[6] = (requestId >> 24) & 0xff;
    buf[7] = (requestId >> 16) & 0xff;
    buf[8] = (requestId >> 8) & 0xff;
    buf[9] = (requestId >> 0) & 0xff;
    return buf;
}
