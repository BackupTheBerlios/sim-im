#include "oscarsocket.h"
#include "network/tcpasyncsocket.h"

OscarSocket::OscarSocket(QObject *parent) :
    QObject(parent)
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
