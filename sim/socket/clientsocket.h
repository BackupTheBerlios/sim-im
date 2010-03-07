
#ifndef SIM_CLIENTSOCKET_H
#define SIM_CLIENTSOCKET_H

#include "simapi.h"

#include "socketnotify.h"
#include "buffer.h"

namespace SIM
{
    class TCPClient;
    class Socket;
    class ClientSocketNotify;

    class EXPORT ClientSocket : public SocketNotify
    {
    public:
        ClientSocket(ClientSocketNotify*, Socket *sock=NULL);
        ~ClientSocket();

        virtual void error_state(const QString &err, unsigned code = 0);
        void connect(const QString &host, unsigned short port, TCPClient *client);
        void connect(unsigned long ip, unsigned short port, TCPClient* client);
        void write();
        void pause(unsigned);
        unsigned long localHost();
        bool created();
        virtual void read_ready();
        void close();
        void setRaw(bool mode);
        Socket *socket() const { return m_sock; }
        void setSocket(Socket *s, bool bClearError = true);
        void setNotify(ClientSocketNotify *n) { m_notify = n; }
        const QString &errorString() const;
        virtual Buffer &readBuffer() { return m_readBuffer; }
        virtual Buffer &writeBuffer() { return m_writeBuffer; }

    protected:
        virtual void connect_ready();
        virtual void write_ready();
        virtual void resolve_ready(QHostAddress ip);

        Socket *m_sock;
        ClientSocketNotify *m_notify;
        bool bRawMode;
        bool bClosed;

        Buffer m_readBuffer;
        Buffer m_writeBuffer;

        unsigned	errCode;
        QString     errString;
        friend class SocketFactory;
    };
}

#endif

// vim: set expandtab:

