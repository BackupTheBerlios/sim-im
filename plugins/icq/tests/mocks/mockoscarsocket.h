#ifndef MOCKOSCARSOCKET_H
#define MOCKOSCARSOCKET_H

#include <gmock/gmock.h>
#include "oscarsocket.h"

namespace MockObjects
{
    class MockOscarSocket : public OscarSocket
    {
    public:
        MOCK_METHOD2(connectToHost, void(const QString& host, int port));
        MOCK_METHOD0(disconnectFromHost, void());
        MOCK_METHOD2(flap, void(int channel, const QByteArray& data));
        MOCK_METHOD4(snac, void(int type, int subtype, int requestId, const QByteArray& data));
        MOCK_CONST_METHOD0(channel, int());

        void provokeConnectedSignal()
        {
            emit connected();
        }

        void provokePacketSignal(int channel, const QByteArray& arr)
        {
            emit packet(channel, arr);
        }
    };
}

#endif // MOCKOSCARSOCKET_H
