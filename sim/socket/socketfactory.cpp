
#include <QTimer>
#include <QSet>

#include "socketfactory.h"

#include "event.h"
#include "socket.h"
#include "serversocket.h"
#include "clientsocket.h"

namespace SIM
{
    class SocketFactoryPrivate
    {
    public:
        bool m_bActive;

        QSet<ClientSocket*> errSockets;
        QSet<ClientSocket*> errSocketsCopy;
        QSet<Socket*> removedSockets;
        QSet<ServerSocket*> removedServerSockets;

        SocketFactoryPrivate() : m_bActive(true) {}
    };

    SocketFactory::SocketFactory(QObject *parent)
        : QObject(parent)
    {
        d = new SocketFactoryPrivate;
    }

    SocketFactory::~SocketFactory()
    {
        idle();
        delete d;
    }

    bool SocketFactory::isActive() const
    {
        return d->m_bActive;
    }

    void SocketFactory::setActive(bool isActive)
    {
        if (isActive == d->m_bActive)
            return;
        d->m_bActive = isActive;
        EventSocketActive(d->m_bActive).process();
    }

    void SocketFactory::remove(Socket *s)
    {
        s->setNotify(NULL);
        s->close();

        if(d->removedSockets.contains(s))
            return;

        d->removedSockets.insert(s);

        QTimer::singleShot(0, this, SLOT(idle()));
    }

    void SocketFactory::remove(ServerSocket *s)
    {
        s->setNotify(NULL);
        s->close();

        if(d->removedServerSockets.contains(s))
            return;

        d->removedServerSockets.insert(s);
        QTimer::singleShot(0, this, SLOT(idle()));
    }

    bool SocketFactory::add(ClientSocket *s)
    {
        if(!d->errSockets.contains(s)) 
        {
            d->errSockets += s;
            return true;
        }
        return false;
    }

    bool SocketFactory::erase(ClientSocket *s)
    {
        return(d->errSockets.remove(s));
    }

    void SocketFactory::idle()
    {
        d->errSocketsCopy = d->errSockets;  // important! error_state() modifes d->errSockets
        d->errSockets.clear();

        QSetIterator<ClientSocket*> it(d->errSocketsCopy);
        while ( it.hasNext() ){
            ClientSocket *s = it.next();
            // can be removed in SocketFactory::erase();
            if(!s)
                continue;
            ClientSocketNotify *n = s->m_notify; //crash here after killing INET-Connection. s = 0x0643c820 {m_sock=0xfeeefeee m_notify=0xfeeefeee bRawMode=true errString = 0xfeeefeee ...}
            if (n){
                QString errString = s->errorString();
                s->errString.clear();
                if (n->error_state(errString, s->errCode))
                    delete n;
            }
        }

        qDeleteAll(d->removedSockets);
        d->removedSockets.clear();

        qDeleteAll(d->removedServerSockets);
        d->removedServerSockets.clear();
    }

}

// vim: set expandtab:

