#ifndef MOCKOSCARSOCKET_H
#define MOCKOSCARSOCKET_H

#include <gmock/gmock.h>
#include "oscarsocket.h"
#include "bytearraybuilder.h"

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

        QByteArray makeSnacPacket(int type, int subtype, int requestId, const QByteArray& payload)
        {
            ByteArrayBuilder builder;
            builder.appendWord(type);
            builder.appendWord(subtype);
            builder.appendWord(0); // Flags
            builder.appendDword(requestId);
            builder.appendBytes(payload);
            return builder.getArray();
        }

        void provokePacketSignal(int channel, const QByteArray& arr)
        {
            emit packet(channel, arr);
        }
    };
}

#endif // MOCKOSCARSOCKET_H
