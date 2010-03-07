
#include "testclientsocket.h"
#include "plugins.h"
#include "socket/clientsocket.h"

namespace testClientSocket
{
    int TestSocket::socketCounter = 0;
    TestSocket::TestSocket() : SIM::Socket()
    {
        socketCounter++;
        length = 40;
    }

    TestSocket::~TestSocket()
    {
        socketCounter--;
    }

    int TestSocket::read(char *buf, unsigned int size)
    {
        if(length <= 0)
            return 0;
        // bogus data for tesing
        if(size > 10)
        {
            for(int i = 0; i < 10; i++)
            {
                buf[i] = 'A' + i;
            }
            length -= 10;
            return 10;
        }
        return 0;
    }

    void TestSocket::write(const char *buf, unsigned int size)
    {
    }

    void TestSocket::close()
    {
    }

    unsigned long TestSocket::localHost()
    {
        return 0;
    }

    void TestSocket::pause(unsigned)
    {
    }

    TestFactory::TestFactory(QObject* parent) : SIM::SocketFactory(parent),
        socketCounter(0)
    {
    }

    TestFactory::~TestFactory()
    {
    }

    SIM::Socket* TestFactory::createSocket()
    {
        socketCounter++;
        return new TestSocket();
    }

    bool TestFactory::erase(SIM::ClientSocket* sock)
    {
        socketCounter--;
        return true;
    }

    SIM::ServerSocket* TestFactory::createServerSocket()
    {
        return NULL;
    }

    void TestFactory::resolve(const QString &host) 
    {
    }

    void TestFactory::setResolver(SIM::IResolver* resolver)
    {
    }

    TestClientSocketNotify::TestClientSocketNotify()
    {
    }

    bool TestClientSocketNotify::error_state(const QString &err, unsigned code)
    {
        return false;
    }

    void TestClientSocketNotify::connect_ready()
    {
    }

    void TestClientSocketNotify::packet_ready()
    {
    }

    TestTCPClient::TestTCPClient() : TCPClient(0, 0)
    {
    }

    QString TestTCPClient::getServer() const
    {
        return QString();
    }

    unsigned short TestTCPClient::getPort() const
    {
        return 0;
    }

    void TestTCPClient::setStatus(unsigned status)
    {
    }

    void TestTCPClient::disconnected()
    {
    }

    void TestClientSocket::testCtorDtor()
    {
        SIM::ClientSocket* socket = new SIM::ClientSocket(m_notify);
        QVERIFY2(m_factory->socketCounter == 1, "SIM::ClientSocket doesn't create a Socket on construction");
        QVERIFY2(TestSocket::socketCounter == 1, "SIM::ClientSocket doesn't create a Socket on construction");
        delete socket;
        QVERIFY2(m_factory->socketCounter == 0, "SIM::ClientSocket doesn't destroy a Socket on destruction");
        QVERIFY2(TestSocket::socketCounter == 0, "SIM::ClientSocket doesn't destroy a Socket on destruction");
    }

    void TestClientSocket::testReading()
    {
        SIM::ClientSocket* socket = new SIM::ClientSocket(m_notify);
        int oldSocketConnect = m_eSocketConnect;
        socket->connect("doesntmatter.com", 80, NULL);
        QCOMPARE(m_eSocketConnect, oldSocketConnect); // There should be no event, if client is NULL

        TestSocket* s = dynamic_cast<TestSocket*>(socket->socket());
        QVERIFY(s);

        s->length = 40;
        socket->readBuffer().resize(256);
        socket->read_ready();
        Buffer b = socket->readBuffer();
        char buf[256];
        int r = b.unpack(buf, 10);
        buf[10] = 0;

        QCOMPARE(r, 10);
        QVERIFY2(strncmp(buf, "ABCDEFGHIJ", 10) == 0, "Socket reading failed");

        delete socket;
    }
    
    void TestClientSocket::testConnectEvent()
    {
        SIM::ClientSocket* socket = new SIM::ClientSocket(m_notify);
        int oldSocketConnect = m_eSocketConnect;
        socket->connect("doesntmatter.com", 80, new TestTCPClient());
        QCOMPARE(m_eSocketConnect, oldSocketConnect + 1); // Exactly one event should occur
        delete socket;
    }

    void TestClientSocket::initTestCase()
    {
        m_factory = new TestFactory(this);
        m_notify = new TestClientSocketNotify();
    }

    void TestClientSocket::cleanupTestCase()
    {
    }

    bool TestClientSocket::processEvent(SIM::Event* e)
    {
        switch(e->type())
        {
            case SIM::eEventSocketConnect:
                m_eSocketConnect++;
                break;
            default:
                break;
        }
        return true;
    }
}

// vim: set expandtab:

