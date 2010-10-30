#ifndef MOCKASYNCSOCKET_H
#define MOCKASYNCSOCKET_H

#include "gmock/gmock.h"

#include "network/asyncsocket.h"

namespace MockObjects
{
    class MockAsyncSocket : public SIM::AsyncSocket
    {
    public:
        MOCK_METHOD2(connectToHost, void(const QString& host, int port));
        MOCK_METHOD0(disconnectFromHost, void());

        MOCK_METHOD2(read, qint64(char* data, qint64 maxSize));
        MOCK_METHOD1(read, QByteArray(qint64 maxSize));

        MOCK_METHOD2(write, qint64(char* data, qint64 maxSize));
        MOCK_METHOD1(write, qint64(const QByteArray& arr));
    };
}

#endif // MOCKASYNCSOCKET_H
