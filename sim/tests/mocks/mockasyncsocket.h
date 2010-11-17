#ifndef MOCKASYNCSOCKET_H
#define MOCKASYNCSOCKET_H

#include <QByteArray>
#include <QBuffer>

#include "gmock/gmock.h"

#include "network/asyncsocket.h"

namespace MockObjects
{
    class MockAsyncSocket : public SIM::AsyncSocket
    {
    public:
        MockAsyncSocket()
        {
            m_read.setBuffer(&m_readBuffer);
            m_read.open(QIODevice::ReadOnly);
            m_write.setBuffer(&m_writeBuffer);
            m_write.open(QIODevice::WriteOnly);
        }

        virtual qint64 read(char* data, qint64 maxSize)
        {
            return m_read.read(data, maxSize);
        }

        virtual QByteArray read(qint64 maxSize)
        {
            return m_read.read(maxSize);
        }

        virtual qint64 write(char* data, qint64 maxSize)
        {
            return m_write.write(data, maxSize);
        }

        virtual qint64 write(const QByteArray& arr)
        {
            return m_write.write(arr);
        }

        void setReadBuffer(const QByteArray& arr)
        {
            m_read.close();
            m_readBuffer = arr;
            m_read.setBuffer(&m_readBuffer);
            m_read.open(QIODevice::ReadOnly);
        }

        QByteArray getWriteBuffer()
        {
            return m_writeBuffer;
        }

        void resetWriteBuffer()
        {
            m_write.close();
            m_writeBuffer.clear();
            m_write.open(QIODevice::WriteOnly);
        }

        void provokeConnectedSignal()
        {
            emit connected();
        }

        MOCK_METHOD2(connectToHost, void(const QString& host, int port));
        MOCK_METHOD0(disconnectFromHost, void());

    private:
        QByteArray m_readBuffer;
        QByteArray m_writeBuffer;
        QBuffer m_read;
        QBuffer m_write;
    };
}

#endif // MOCKASYNCSOCKET_H
