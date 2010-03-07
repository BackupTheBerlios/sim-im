
#ifndef SIM_TCPCLIENT_H
#define SIM_TCPCLIENT_H

#include <QObject>
#include "clientsocketnotify.h"
#include "event.h"
#include "contacts.h"
#include "contacts/client.h"

namespace SIM
{
    class Socket;
    class ClientSocket;
    class EXPORT TCPClient : virtual public QObject, public Client, public ClientSocketNotify, public EventReceiver
    {
        Q_OBJECT
    public:
        TCPClient(Protocol *protocol, Buffer *cfg, unsigned priority = DefaultPriority);

        virtual void changeStatus(const IMStatusPtr& status);

        virtual QString getServer() const = 0;
        virtual unsigned short getPort() const = 0;
        unsigned long ip() const { return m_ip; }
        virtual ClientSocket *socket() { return m_clientSocket; }

        unsigned m_reconnect;

    protected slots:
        void reconnect();
        void loginTimeout();

    protected:
        SIM_DEPRECATED virtual void setStatus(unsigned status, bool bCommon);
        virtual void setStatus(unsigned status) = 0;
        virtual void disconnected() = 0;

        virtual void resolve_ready(QHostAddress);
        virtual void connect_ready();
        virtual bool error_state(const QString &err, unsigned code = 0);
        virtual void socketConnect();
        virtual bool processEvent(Event *e);
        virtual Socket *createSocket();
        virtual ClientSocket *createClientSocket();
        void setClientStatus(unsigned status);
        void setClientStatus(const IMStatusPtr& status);
        unsigned m_logonStatus;
        unsigned m_ip;
        QTimer *m_timer;
        QTimer *m_loginTimer;
        bool m_bWaitReconnect;

    private:
        //IMStatusPtr m_logonStatus;
        ClientSocket* m_clientSocket;
    };

	const unsigned NO_RECONNECT = (unsigned)(-1);
}

#endif

// vim: set expandtab:

