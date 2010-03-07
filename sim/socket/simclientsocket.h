
#ifndef SIM_SIMCLIENTSOCKET_H
#define SIM_SIMCLIENTSOCKET_H

#include <QObject>
#include <QTimer>
#include <QSslSocket>

#include "socket.h"

namespace SIM
{
    class SIMClientSocket : public QObject, public Socket
    {
        Q_OBJECT
    public:
        SIMClientSocket(QSslSocket *s=NULL);
        virtual ~SIMClientSocket();
        virtual int read(char *buf, unsigned int size);
        virtual void write(const char *buf, unsigned int size);
        virtual void connect(const QString &host, unsigned short port);
        virtual unsigned long localHost();
        virtual void pause(unsigned);
        virtual void close();
        virtual bool isEncrypted();
        virtual bool startEncryption();

    signals:
        void interfaceDown(int sockfd); // Probably, sockfd is not needed
        void interfaceUp(int sockfd);

    protected slots:
        void slotConnected();
        void slotConnectionClosed();
        void slotReadReady();
        void slotBytesWritten(qint64);
        void slotBytesWritten();
        void slotError(QAbstractSocket::SocketError);
        void slotLookupFinished(int);
        void resolveReady(const QHostAddress &addr, const QString &host);
        void error(int errcode);
        void checkInterface();
        void connectionTimeout();
        void sslEncrypted();
        void sslEncryptedBytesWritten( qint64 written );
        void sslModeChanged( QSslSocket::SslMode mode );
        void sslPeerVerifyError( const QSslError & error );
        void sslErrors( const QList<QSslError> & errors );

    protected:
        void pickUpSocket( QSslSocket *pSocket );
        void dropSocket();
        QTimer m_carrierCheckTimer;
        QTimer m_connectionTimer;
        bool m_state;
        void timerStop();
        unsigned short port;
        QString host;
        QSslSocket *sock;
        bool bInWrite;
        QString m_interface;
        bool m_bEncrypted;
    };
}

#endif

// vim: set expandtab:
