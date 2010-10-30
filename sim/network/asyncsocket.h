#ifndef ASYNCSOCKET_H
#define ASYNCSOCKET_H

#include <QObject>
#include "simapi.h"

namespace SIM
{
    class EXPORT AsyncSocket : public QObject
    {
        Q_OBJECT
    public:
        explicit AsyncSocket(QObject *parent = 0);

        virtual void connectToHost(const QString& host, int port) = 0;
        virtual void disconnectFromHost() = 0;

        virtual qint64 read(char* data, qint64 maxSize) = 0;
        virtual QByteArray read(qint64 maxSize) = 0;

        virtual qint64 write(char* data, qint64 maxSize) = 0;
        virtual qint64 write(const QByteArray& arr) = 0;

    signals:
        void connected();
        void disconnected();

        void readyRead();
        void bytesWritten(qint64 bytes);
    };
}

#endif // ASYNCSOCKET_H
