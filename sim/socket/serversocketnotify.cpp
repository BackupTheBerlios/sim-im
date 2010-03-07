#include "serversocketnotify.h"

#include "socket.h"
#include "socketfactory.h"
#include "serversocket.h"

namespace SIM
{
    ServerSocketNotify::ServerSocketNotify()
    {
        m_listener = NULL;
    }

    ServerSocketNotify::~ServerSocketNotify()
    {
        if (m_listener)
            getSocketFactory()->remove(m_listener);
    }

    void ServerSocketNotify::setListener(ServerSocket *listener)
    {
        if (m_listener)
            getSocketFactory()->remove(m_listener);
        m_listener = listener;
        if (m_listener)
            m_listener->setNotify(this);
    }

    void ServerSocketNotify::bind(unsigned short minPort, unsigned short maxPort, TCPClient *client)
    {
        if (m_listener)
            getSocketFactory()->remove(m_listener);
        m_listener = getSocketFactory()->createServerSocket();
        m_listener->setNotify(this);
        m_listener->bind(minPort, maxPort, client);
    }
}

