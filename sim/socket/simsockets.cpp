
#include <list>
#include <QApplication>

#include "simsockets.h"

#include "stdresolver.h"
#include "simresolver.h"
#include "simclientsocket.h"
#include "simserversocket.h"
#include "fetch.h"

namespace SIM
{
    SIMSockets::SIMSockets(QObject *parent)
        : SocketFactory(parent)
    {
    }

    SIMSockets::~SIMSockets()
    {
    }

    void SIMSockets::checkState()
    {
#ifdef WIN32
        bool state;
        if (get_connection_state(state))
            setActive(state);
#endif
    }

    void SIMSockets::idle()
    {
        SocketFactory::idle();
    }

    Socket *SIMSockets::createSocket()
    {
        return new SIMClientSocket();
    }

    ServerSocket *SIMSockets::createServerSocket()
    {
        return new SIMServerSocket();
    }

    static SocketFactory* gs_socketfactory = 0;

    EXPORT void createSocketFactory()
    {
        if(!gs_socketfactory)
            gs_socketfactory = new SIMSockets(qApp);
    }

    EXPORT void destroySocketFactory()
    {
        if(gs_socketfactory)
            delete gs_socketfactory;
        gs_socketfactory = 0;
    }

    SocketFactory *getSocketFactory()
    {
        return gs_socketfactory;
    }

}

// vim: set expandtab:

