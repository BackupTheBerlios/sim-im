#include "simclientsocket.h"

#include <QSslSocket>
#include <QNetworkInterface>

#include "socketfactory.h"
#include "log.h"
#include "misc.h"

namespace SIM
{
    const unsigned CONNECT_TIMEOUT = 60;  // FIXME hardcoded
    const unsigned CARRIER_CHECK_TIMEOUT = 10000;  // FIXME hardcoded

    SIMClientSocket::SIMClientSocket( QSslSocket *s )
        : m_carrierCheckTimer( this )
        , m_connectionTimer( this )
        , sock( NULL )
        , bInWrite( false )
        , m_bEncrypted( false )
    {
        pickUpSocket( s );
        m_carrierCheckTimer.setInterval( CARRIER_CHECK_TIMEOUT );
        QObject::connect( &m_carrierCheckTimer, SIGNAL(timeout()), SLOT(checkInterface()) );
        m_connectionTimer.setInterval( CONNECT_TIMEOUT );
        QObject::connect( &m_connectionTimer, SIGNAL(timeout()), SLOT(connectionTimeout()) );
    }

    SIMClientSocket::~SIMClientSocket()
    {
        dropSocket();
    }

    void SIMClientSocket::pickUpSocket( QSslSocket *pSocket ) {
        if( NULL != sock )
            dropSocket();

        sock = pSocket;
        if( NULL == sock )
            return;

        QObject::connect(sock, SIGNAL(connected()), this, SLOT(slotConnected()));
        QObject::connect(sock, SIGNAL(disconnected()), this, SLOT(slotConnectionClosed()));
        QObject::connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
        QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadReady()));
        QObject::connect(sock, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
        QObject::connect(sock, SIGNAL(encrypted()), this, SLOT(sslEncrypted()));
        QObject::connect(sock, SIGNAL(encryptedBytesWritten(qint64)), this, SLOT(sslEncryptedBytesWritten(qint64)));
        QObject::connect(sock, SIGNAL(modeChanged(QSslSocket::SslMode)), this, SLOT(sslModeChanged(QSslSocket::SslMode)));
        QObject::connect(sock, SIGNAL(peerVerifyError(const QSslError&)), this, SLOT(sslPeerVerifyError(const QSslError&)));
        QObject::connect(sock, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(sslErrors(const QList<QSslError>&)));

    }

    void SIMClientSocket::dropSocket() {
        if( NULL == sock )
            return;
        timerStop();
        disconnect( sock, 0, 0, 0 );
        sock->close();
        if( sock->state() == QAbstractSocket::ClosingState )
            sock->connect( sock, SIGNAL(disconnected()), SLOT(deleteLater()) );
        else
            delete sock;
        sock = NULL;
    }

    void SIMClientSocket::close()
    {
        timerStop();
        sock->close();
    }

    void SIMClientSocket::timerStop()
    {
        m_connectionTimer.stop();
        m_carrierCheckTimer.stop();
    }

    void SIMClientSocket::slotLookupFinished(int state)
    {
        log(L_DEBUG, "Lookup finished %u", state);
        if (state == 0){
            log(L_WARN, "Can't lookup");
            notify->error_state(I18N_NOOP("Connect error"));
            getSocketFactory()->setActive(false);
        }
    }

    int SIMClientSocket::read(char *buf, unsigned int size)
    {
        unsigned available = sock->bytesAvailable();
        if (size > available)
            size = available;
        if (size == 0)
            return size;
        int res = sock->read(buf, size);
        if (res < 0){
            log(L_DEBUG, "QClientSocket::read error %s", sock->errorString().toLatin1().data());
            if (notify)
                notify->error_state(I18N_NOOP("Read socket error"));
            return -1;
        }
        return res;
    }

    void SIMClientSocket::write(const char *buf, unsigned int size)
    {
        if( NULL == sock )
            return;
        bInWrite = true;
        int res = sock->write(buf, size);
        bInWrite = false;
        if (res != (int)size){
            if (notify)
                notify->error_state(I18N_NOOP("Write socket error"));
            return;
        }
        if (sock->bytesToWrite() == 0)
            QTimer::singleShot(0, this, SLOT(slotBytesWritten()));
    }

    void SIMClientSocket::connect(const QString &_host, unsigned short _port)
    {
        if( ( NULL != sock ) && sock->state() != QAbstractSocket::UnconnectedState ) {
            dropSocket();
        }
        if( NULL == sock ) {
            pickUpSocket( new QSslSocket( this ) );
        }
        port = _port;
        host = _host;
        log( L_DEBUG, QString("Connect to %1:%2").arg(host).arg(port) );
        sock->connectToHost( host, port );
    }

    void SIMClientSocket::resolveReady(const QHostAddress &/*addr*/, const QString &/*_host*/)
    {
    }

    void SIMClientSocket::slotConnected()
    {
        timerStop();
        log(L_DEBUG, "Connected");
        if (notify) notify->connect_ready();
        getSocketFactory()->setActive(true);
        m_state = true;
        m_carrierCheckTimer.start();
    }

    void SIMClientSocket::slotConnectionClosed()
    {
        log(L_WARN, "Connection closed");
        timerStop();
        if (notify)
            notify->error_state(I18N_NOOP("Connection closed"));
        checkInterface();
    }

    void SIMClientSocket::connectionTimeout() {
        QTimer::singleShot(0, this, SLOT(slotConnectionClosed()));
    }

    void SIMClientSocket::checkInterface()
    {
        bool bHaveInterfaces = false;
        QList<QNetworkInterface> listInterfaces = QNetworkInterface::allInterfaces();
        QNetworkInterface intfce;
        foreach( intfce, listInterfaces ) {
            if( !intfce.flags().testFlag( QNetworkInterface::IsLoopBack ) ) {
                bHaveInterfaces = true;
            }
        }
/*
#if !defined(WIN32) && !defined(Q_OS_MAC)
        int fd = sock->socket();
        if(fd == -1)
        {
            return;
        }
        struct ifreq ifr;
        struct ifreq* ifrp;
        struct ifreq ibuf[16];
        struct ifconf	ifc;

        ifc.ifc_len = sizeof(ifr)*16;
        ifc.ifc_buf = (caddr_t)&ibuf;
        memset(ibuf, 0, sizeof(struct ifreq)*16);

        int hret = ioctl(fd, SIOCGIFCONF, &ifc);
        if(hret == -1)
        {
            return;
        }
        bool iffound = false;
        for(int i = 0; i < ifc.ifc_len/sizeof(struct ifreq); i++)
        {
            ifrp = ibuf + i;
            strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));

            if  (
                    strcmp(ifr.ifr_name, "lo") == 0 ||
                    (htonl(((sockaddr_in*)&ifrp->ifr_addr)->sin_addr.s_addr) != sock->address().toIPv4Address())
                )	continue;

            m_interface = ifr.ifr_name;
            iffound = true;

            hret = ioctl(fd, SIOCGIFFLAGS, &ifr);
            if(hret != -1)
            {
                int state = ifr.ifr_flags & IFF_RUNNING;
                if(state < 0)
                {
                    log(L_DEBUG, "Incorrect state: %d (%s)", state, ifr.ifr_name);
                    return;
                }
                if((state == 0) && (m_state))
                {
                    m_state = false;
                    emit interfaceDown(fd);
                    EventInterfaceDown e(fd);
                    e.process();
                    return;
                }
                if((state != 0) && (!m_state))
                {
                    m_state = true;
                    emit interfaceUp(fd);
                    EventInterfaceUp e(fd);
                    e.process();
                    return;
                }
                return;
            }
        }
        if(!iffound)
        {
            m_state = false;
            emit interfaceDown(fd);
            EventInterfaceDown e(fd);
            e.process();
        }
#else
        return;
#endif
*/
    }

    void SIMClientSocket::error(int errcode)
    {
        log(L_DEBUG, "SIMClientSocket error(%d), Socket error: %s", errcode, sock->errorString().toLatin1().data());
    }

    void SIMClientSocket::slotReadReady()
    {
        if (notify)
            notify->read_ready();
    }

    void SIMClientSocket::slotBytesWritten(qint64)
    {
        slotBytesWritten();
    }

    void SIMClientSocket::slotBytesWritten()
    {
        if (bInWrite || (sock == NULL)) return;
        if ((sock->bytesToWrite() == 0) && notify)
            notify->write_ready();
    }

    unsigned long SIMClientSocket::localHost()
    {
        if( NULL != sock ) {
            return sock->localAddress().toIPv4Address();
        }
        return 0;
    }

    void SIMClientSocket::slotError(QAbstractSocket::SocketError err)
    {
        if (err)
            log(L_DEBUG, "Socket error %s", sock->errorString().toLatin1().data());
        timerStop();
        if (notify)
            notify->error_state(I18N_NOOP("Socket error"));
    }

    void SIMClientSocket::pause(unsigned t)
    {
        QTimer::singleShot(t * 1000, this, SLOT(slotBytesWritten()));
    }

    bool SIMClientSocket::isEncrypted() {
        if( NULL == sock )
            return false;

        return sock->isEncrypted();
    }

    bool SIMClientSocket::startEncryption() {
        if( NULL == sock )
            return false;
        if( sock->isEncrypted() )
            return true;
        sock->setPeerVerifyMode( QSslSocket::VerifyNone ); // No UI for SSL
        sock->startClientEncryption();
        return true;
    }

    void SIMClientSocket::sslEncrypted() {
        notify->connect_ready();
    }

    void SIMClientSocket::sslEncryptedBytesWritten( qint64 /*written*/ ) {
    }

    void SIMClientSocket::sslModeChanged( QSslSocket::SslMode /*mode*/ ) {
    }

    void SIMClientSocket::sslPeerVerifyError( const QSslError & /*error*/ ) {
    }

    void SIMClientSocket::sslErrors( const QList<QSslError> & /*errors*/ ) {
    }
}

// vim: set expandtab:

