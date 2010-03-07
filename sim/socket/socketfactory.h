
#ifndef SIM_SOCKETFACTORY_H
#define SIM_SOCKETFACTORY_H

#include <QObject>
#include "simapi.h"

namespace SIM
{
    class ClientSocket;
    class ServerSocket;
    class Socket;
    class SocketFactoryPrivate;
    class IResolver;

    class EXPORT SocketFactory : public QObject
    {
        Q_OBJECT
    public:
        SocketFactory(QObject *parent);
        virtual ~SocketFactory();
        virtual Socket *createSocket() = 0;
        virtual ServerSocket *createServerSocket() = 0;
        void remove(Socket*);
        void remove(ServerSocket*);
        bool add(ClientSocket *s);
        virtual bool erase(ClientSocket *s);
        void setActive(bool);
        bool isActive() const;
        virtual void checkState(){}

    protected slots:
        void idle();

    private:
        SocketFactoryPrivate *d;

        COPY_RESTRICTED(SocketFactory)
    };

    EXPORT void createSocketFactory();
    EXPORT void destroySocketFactory();
    SocketFactory EXPORT *getSocketFactory();
}

#endif

// vim: set expandtab:

