
#include <QCryptographicHash>
#include <gtest/gtest.h>
#include "icqclient.h"
#include "authorizationsnachandler.h"
#include "mocks/mockoscarsocket.h"
#include "testauthorizationsnachandler.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"
#include "tlvlist.h"

namespace
{
    using ::testing::InSequence;
    using ::testing::Truly;
    using ::testing::_;
    class TestAuthorizationSnacHandler : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new MockObjects::MockOscarSocket();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
            client->clientPersistentData->owner.setUin(123456);
            handler = static_cast<AuthorizationSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_LOGIN));
            ASSERT_TRUE(handler);

            m_bosHost = QString("12.34.56.78");
            m_bosPort = 5190;
            m_bosServer = QString("%1:%2").arg(m_bosHost).arg(m_bosPort);

            m_salt = "1493215011";
            m_password = "foobar";

            client->setPassword(m_password);
        }

        virtual void TearDown()
        {
            delete client; // socket is deleted as well
        }

        virtual QByteArray makeFlapPacket(int channel, const QByteArray& data)
        {
            QByteArray packet("\x2a\x00\x00\x00\x00\x00");
            packet[1] = channel;
            packet[4] = (data.size() >> 8) & 0xff;
            packet[5] = (data.size()) & 0xff;
            packet.append(data);
            return packet;
        }

        virtual QByteArray makeSnacPacket(int channel, int type, int subtype, int flags, int id, const QByteArray& data)
        {
            ByteArrayBuilder builder;
            builder.appendWord(type);
            builder.appendWord(subtype);
            builder.appendWord(flags);
            builder.appendDword(id);
            builder.appendBytes(data);
            return builder.getArray();
        }

        QByteArray makeAuthCookie()
        {
            QByteArray cookie;
            for(int i = 0; i < 256; i++)
            {
                cookie.append(i);
            }
            return cookie;
        }

        TlvList makeAuthCookieTlvList(const QByteArray& authCookie)
        {
            TlvList authCookieTlv;
            authCookieTlv.append(Tlv(0x06, authCookie));
            return authCookieTlv;
        }

        QByteArray makeSaltArray()
        {
            ByteArrayBuilder builder;
            builder.appendWord(m_salt.size());
            builder.appendBytes(m_salt);
            return builder.getArray();
        }

        TlvList makeUinTlvList(const QString& uin)
        {
            TlvList uinTlv;
            uinTlv.append(Tlv(0x01, uin.toAscii()));
            uinTlv.append(Tlv(0x4b, QByteArray()));
            uinTlv.append(Tlv(0x5a, QByteArray()));
            return uinTlv;
        }

        TlvList makeCloseConnectionTlvList(const QString& uin, const QByteArray& authCookie)
        {
            TlvList closeConnectionTlvs;
            closeConnectionTlvs.append(Tlv(0x01, uin.toAscii()));
            closeConnectionTlvs.append(Tlv(0x05, m_bosServer.toAscii()));
            closeConnectionTlvs.append(Tlv(0x06, authCookie));
            return closeConnectionTlvs;
        }

        static bool isValidSignonPacket(const QByteArray& packet)
        {
            ByteArrayParser parser(packet);

            quint32 protocolVersion = parser.readDword();
            if(protocolVersion != 1)
                return false;

            TlvList list = TlvList::fromByteArray(parser.readAll());

            Tlv uin = list.firstTlv(0x01);
            if(uin.data() != QByteArray("123456"))
                return false;

            if(!list.contains(0x02))
                return false;

            // etc, we won't check other tlvs

            return true;
        }

        ICQClient* client;
        MockObjects::MockOscarSocket* socket;
        AuthorizationSnacHandler* handler;
        QString m_bosServer;
        QString m_bosHost;
        int m_bosPort;
        QByteArray m_salt;
        QByteArray m_password;
    };

    class IsPacketMd5Correct
    {
    public:
        IsPacketMd5Correct(const QByteArray& salt, const QByteArray& password) : m_salt(salt), m_password(password) {}

        bool operator()(const QByteArray& packet) const
        {
            TlvList list = TlvList::fromByteArray(packet);

            Tlv md5 = list.firstTlv(0x25);
            if(!md5.isValid())
                return false;

            QByteArray hash = QCryptographicHash::hash(m_salt + m_password + AuthorizationSnacHandler::magicHashString(), QCryptographicHash::Md5);
            return md5.data() == hash;
        }

    private:
        QByteArray m_salt;
        QByteArray m_password;
    };

    TEST_F(TestAuthorizationSnacHandler, loginSequence_noMd5)
    {
        client->setUseMD5(false);
        QByteArray authCookie = makeAuthCookie();
        TlvList authCookieTlv = makeAuthCookieTlvList(authCookie);

        QByteArray firstPacketData = QByteArray("\x00\x00\x00\x01", 4);
        TlvList secondPacketTlvs = makeCloseConnectionTlvList("123456", authCookie);

        {
            InSequence seq;
            EXPECT_CALL(*socket, flap(0x01, Truly(isValidSignonPacket)));
            EXPECT_CALL(*socket, disconnectFromHost());
            EXPECT_CALL(*socket, connectToHost(m_bosHost, m_bosPort));
            EXPECT_CALL(*socket, flap(0x01, firstPacketData + authCookieTlv.toByteArray()));
        }

        socket->provokePacketSignal(0x01, firstPacketData);
        socket->provokePacketSignal(0x04, secondPacketTlvs.toByteArray());
        socket->provokePacketSignal(0x01, firstPacketData);

    }

    TEST_F(TestAuthorizationSnacHandler, loginSequence_md5)
    {
        client->setUseMD5(true);

        QByteArray authCookie = makeAuthCookie();
        TlvList authCookieTlv = makeAuthCookieTlvList(authCookie);
        QByteArray firstPacketData = QByteArray("\x00\x00\x00\x01", 4);
        TlvList uinTlv = makeUinTlvList("123456");
        TlvList closeConnectionTlvs = makeCloseConnectionTlvList("123456", authCookie);
        QByteArray saltArray = makeSaltArray();

        {
            InSequence seq;
            EXPECT_CALL(*socket, flap(0x01, firstPacketData));
            EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_LOGIN, AuthorizationSnacHandler::SnacAuthKeyRequest, _, uinTlv.toByteArray()));
            EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_LOGIN, AuthorizationSnacHandler::SnacAuthMd5Login, _, Truly(IsPacketMd5Correct(m_salt, m_password))));
            EXPECT_CALL(*socket, disconnectFromHost());
            EXPECT_CALL(*socket, connectToHost(m_bosHost, m_bosPort));
            EXPECT_CALL(*socket, flap(0x01, firstPacketData + authCookieTlv.toByteArray()));
        }

        socket->provokePacketSignal(0x01, firstPacketData);
        socket->provokePacketSignal(0x02, makeSnacPacket(0x02, ICQ_SNACxFOOD_LOGIN, AuthorizationSnacHandler::SnacAuthKeyResponse, 0, 0, saltArray));
        socket->provokePacketSignal(0x02, makeSnacPacket(0x02, ICQ_SNACxFOOD_LOGIN, AuthorizationSnacHandler::SnacAuthLoginReply, 0, 0, closeConnectionTlvs.toByteArray()));
        socket->provokePacketSignal(0x04, QByteArray());
        socket->provokePacketSignal(0x01, firstPacketData);
    }
}
