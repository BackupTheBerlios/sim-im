
#ifndef SIM_SIMSERVERSOCKET_H
#define SIM_SIMSERVERSOCKET_H

#include <QTcpServer>

#include "serversocket.h"

namespace SIM
{
    class SIMServerSocket : public QTcpServer, public ServerSocket
    {
        Q_OBJECT
    public:
        SIMServerSocket();
        virtual ~SIMServerSocket();
        virtual unsigned short port() { return m_nPort; }
        virtual void bind(unsigned short minPort, unsigned short maxPort, TCPClient *client);
        virtual void close();
    protected:
        void error(const char *err);
        unsigned short m_nPort;
        virtual void incomingConnection( int socketDescriptor );
    };

}

#endif

// vim: set expandtab:

