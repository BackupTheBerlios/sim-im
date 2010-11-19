#ifndef TCPASYNCSOCKET_H
#define TCPASYNCSOCKET_H

#include "asyncsocket.h"
#include "simapi.h"

#include <QTcpSocket>

namespace SIM {

class EXPORT TcpAsyncSocket : public AsyncSocket
{
    Q_OBJECT
public:
    explicit TcpAsyncSocket(QObject* parent = 0);

    virtual void connectToHost(const QString& host, int port);
    virtual void disconnectFromHost();

    virtual qint64 read(char* data, qint64 maxSize);
    virtual QByteArray read(qint64 maxSize);

    virtual qint64 write(char* data, qint64 maxSize);
    virtual qint64 write(const QByteArray& arr);

private slots:
    void slot_connected();
    void slot_disconnected();
    void slot_readyRead();
    void slot_bytesWritten(qint64 bytes);

private:
    QTcpSocket m_socket;
};

} // namespace SIM

#endif // TCPASYNCSOCKET_H
