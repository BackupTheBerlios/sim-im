#include "tcpasyncsocket.h"

namespace SIM {

TcpAsyncSocket::TcpAsyncSocket(QObject* parent) : AsyncSocket(parent), m_socket(this)
{
    connect(&m_socket, SIGNAL(connected()), this, SLOT(slot_connected()));
    connect(&m_socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(slot_readyRead()));
    connect(&m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_bytesWritten(qint64)));
}

void TcpAsyncSocket::connectToHost(const QString& host, int port)
{
    m_socket.connectToHost(host, port);
}

void TcpAsyncSocket::disconnectFromHost()
{
    m_socket.disconnectFromHost();
}

qint64 TcpAsyncSocket::read(char* data, qint64 maxSize)
{
    return m_socket.read(data, maxSize);
}

QByteArray TcpAsyncSocket::read(qint64 maxSize)
{
    return m_socket.read(maxSize);
}

qint64 TcpAsyncSocket::write(char* data, qint64 maxSize)
{
    return m_socket.write(data, maxSize);
}

qint64 TcpAsyncSocket::write(const QByteArray& arr)
{
    return m_socket.write(arr);
}

void TcpAsyncSocket::slot_connected()
{
    emit connected();
}

void TcpAsyncSocket::slot_disconnected()
{
    emit disconnected();
}

void TcpAsyncSocket::slot_readyRead()
{
    emit readyRead();
}

void TcpAsyncSocket::slot_bytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}


} // namespace SIM
