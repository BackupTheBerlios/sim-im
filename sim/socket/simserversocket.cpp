#include "simserversocket.h"

#include <QSslSocket>

#include "log.h"
#include "misc.h"
#include "serversocketnotify.h"
#include "socketfactory.h"
#include "simclientsocket.h"

namespace SIM
{
    SIMServerSocket::SIMServerSocket() {
    }

    SIMServerSocket::~SIMServerSocket() {
        QTcpServer::close();
    }

    void SIMServerSocket::close() {
        QTcpServer::close();
    }

    void SIMServerSocket::bind(unsigned short minPort, unsigned short maxPort, TCPClient *client) {
        if( client && notify ) {
            EventSocketListen e( notify, client );
            if( e.process() )
                return;
        }
        unsigned short startPort = (unsigned short)(minPort + get_random() % (maxPort - minPort + 1));
        bool bOK = false;
        for( m_nPort = startPort;; ){
            if( listen( QHostAddress::Any, m_nPort ) ){
                bOK = true;
                break;
            }
            if( ++m_nPort > maxPort )
                m_nPort = minPort;
            if( m_nPort == startPort )
                break;
        }
        if( !bOK && !listen( QHostAddress::Any, 50 ) ){
            error( I18N_NOOP( "Can't allocate port" ) );
            return;
        }
        if( notify )
            notify->bind_ready(m_nPort);
    }

    void SIMServerSocket::error( const char *err ) {
        QTcpServer::close();
        if (notify && notify->error(err)){
            notify->m_listener = NULL;
            getSocketFactory()->remove(this);
        }
    }

    void SIMServerSocket::incomingConnection( int socketDescriptor ) {
        log(L_DEBUG, "accept ready");
        QSslSocket *s = new QSslSocket();
        s->setSocketDescriptor( socketDescriptor );
        if( notify ){
            if( notify->accept( new SIMClientSocket(s), s->peerAddress().toIPv4Address() ) ) {
                notify->m_listener = NULL;
                getSocketFactory()->remove(this);
            }
        }else{
            s->close();
        }
    }
}

// vim: set expandtab:

