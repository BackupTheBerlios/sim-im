#include "standardoscarsocket.h"
#include "network/tcpasyncsocket.h"
#include "log.h"
#include "misc.h"

using namespace SIM;

StandardOscarSocket::StandardOscarSocket(QObject *parent) : OscarSocket(parent), m_nFlapSequence(8984)
{
    m_socket = new SIM::TcpAsyncSocket(this);
    connect(m_socket, SIGNAL(connected()), this, SLOT(slot_connected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    m_bHeader = true;
}

StandardOscarSocket::~StandardOscarSocket()
{
    if(m_socket)
        delete m_socket;
}

void StandardOscarSocket::setSocket(SIM::AsyncSocket* socket)
{
    if(m_socket)
        delete m_socket;
    m_socket = socket;
    connect(m_socket, SIGNAL(connected()), this, SLOT(slot_connected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void StandardOscarSocket::connectToHost(const QString& host, int port)
{
    log(L_DEBUG, "StandardOscarSocket::connectToHost(%s, %d)", qPrintable(host), port);
    m_socket->connectToHost(host, port);
}

void StandardOscarSocket::disconnectFromHost()
{
    m_socket->disconnectFromHost();
}

void StandardOscarSocket::flap(int channel, const QByteArray& data)
{
    QByteArray arr = makeFlapPacket(channel, data.length());
    arr += data;
    m_socket->write(arr);
}

QByteArray StandardOscarSocket::makeFlapPacket(int channel, int length)
{
    QByteArray buf(6, 0);
    buf[0] = FlapId;
    buf[1] = channel;
    buf[2] = (m_nFlapSequence >> 8) & 0xff;
    buf[3] = m_nFlapSequence & 0xff;
    buf[4] = (length >> 8) & 0xff;
    buf[5] = length & 0xff;

    m_nFlapSequence++;

    return buf;
}

void StandardOscarSocket::snac(int type, int subtype, int requestId, const QByteArray& data)
{
    QByteArray arr = makeFlapPacket(FlapChannelSnac, data.size() + SizeOfSnacHeader);
    QByteArray snacHeader = makeSnacHeader(type, subtype, requestId);
    m_socket->write(arr + snacHeader + data);
}

QByteArray StandardOscarSocket::makeSnacHeader(int type, int subtype, int requestId)
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

void StandardOscarSocket::readyRead()
{
    while(m_socket->bytesAvailable() > 0)
    {
        if(m_bHeader)
        {
            QByteArray header = m_socket->read(SizeOfFlapHeader);
            if(header.size() < SizeOfFlapHeader)
            {
                log(L_ERROR, "Truncated header");
                emit error(I18N_NOOP("Protocol error"));
                return;
            }

            //log(L_DEBUG, "Header: %s", header.toHex().data());

            char c = header.at(0);
            if(c != FlapId)
            {
                log(L_ERROR, "Server send bad packet start code: %02X", c);
                emit error(I18N_NOOP("Protocol error"));
                return;
            }
            quint16 sequence;
            m_nChannel = header.at(1);
            sequence = ((unsigned char)header.at(2)) * 0x100 + (unsigned char)header.at(3);
            m_bHeader = false;
            m_packet.clear();
            m_packetLength = ((unsigned char)header.at(4)) * 0x100 + (unsigned char)header.at(5);
            //log(L_DEBUG, "packet length :%d (%d:%d)", m_packetLength, (unsigned char)header.at(4), (unsigned char)header.at(5));
        }
        int toread = m_packetLength - m_packet.size();
        QByteArray chunk = m_socket->read(toread);
        m_packet.append(chunk);
        //log(L_DEBUG, "Chunk: (toread: %d, chunksize: %d, packetLength: %d)", toread, chunk.size(), m_packetLength);
        if(m_packet.size() == m_packetLength)
        {
            m_bHeader = true;
            emit packet(m_nChannel, m_packet);
        }
    }
}

void StandardOscarSocket::slot_connected()
{
    emit connected();
}
