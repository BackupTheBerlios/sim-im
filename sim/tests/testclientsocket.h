
#ifndef SIM_TESTCLIENTSOCKET_H
#define SIM_TESTCLIENTSOCKET_H

#include <QtTest/QtTest>
#include <QObject>
#include "event.h"
#include "contacts.h"
#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "socket/iresolver.h"
#include "socket/tcpclient.h"

namespace testClientSocket
{
    class TestSocket : public SIM::Socket
    {
    public:
        TestSocket();
        virtual ~TestSocket();
        virtual int read(char *buf, unsigned int size);
        virtual void write(const char *buf, unsigned int size);
        virtual void close();
        virtual unsigned long localHost();
        virtual void pause(unsigned);
        virtual bool isEncrypted(){ return false; }
        virtual bool startEncryption(){ return false; }

        static int socketCounter;
        int length;
    };

    class TestFactory : public SIM::SocketFactory
    {
    public:
        TestFactory(QObject* parent);
        virtual ~TestFactory();
        virtual SIM::Socket* createSocket();
        virtual SIM::ServerSocket* createServerSocket();
        virtual bool erase(SIM::ClientSocket* sock);
        virtual void resolve(const QString &host);
        virtual void setResolver(SIM::IResolver* resolver);

        // Testing variables:
        int socketCounter;
    };

    class TestClientSocketNotify : public SIM::ClientSocketNotify
    {
    public:
        TestClientSocketNotify();
        virtual bool error_state(const QString &err, unsigned code = 0);
        virtual void connect_ready();
        virtual void packet_ready();
        virtual void write_ready() {}
        virtual void resolve_ready(unsigned long) {}
        
    };

    class TestTCPClient : public SIM::TCPClient, public TestClientSocketNotify
    {
    public:
        TestTCPClient();
        virtual QString getServer() const;
        virtual unsigned short getPort() const;

        QString name()
        {
            return QString("TestTCPClient");
        }
        QString dataName(void*)
        {
            return QString("TestDataName");
        }
        QWidget *setupWnd() {return 0;}
        bool isMyData(SIM::clientData*&, SIM::Contact*&) {return false;}
        bool createData(SIM::clientData*&, SIM::Contact*) { return false;}
		void contactInfo(void * /*clientData*/, unsigned long& /*status*/, unsigned& /*style*/, QString& /*statusIcon*/, QSet<QString>* /*icons*/ = NULL) {}
		void setupContact(SIM::Contact*, void* /*data*/){}
		bool send(SIM::Message*, void* /*data*/){return false;}
		bool canSend(unsigned /*type*/, void* /*data*/){return false;}
		QWidget *searchWindow(QWidget* /*parent*/){return 0;}
        virtual void packet_ready() {}

    protected:
        virtual void setStatus(unsigned status);
        virtual void disconnected();
    };
    
    class TestClientSocket : public QObject, public SIM::EventReceiver
    {
        Q_OBJECT
    public:
        TestClientSocket() : QObject(),
            SIM::EventReceiver(),
            m_eSocketConnect(0)
        {
        }
        virtual bool processEvent(SIM::Event*);
    private slots:
        void testCtorDtor();
        void testReading();
        void testConnectEvent();
        void initTestCase();
        void cleanupTestCase();

    private:
        SIM::ClientSocket* m_socket;
        TestClientSocketNotify* m_notify;
        TestFactory* m_factory;

        int m_eSocketConnect;
    };
}

#endif

// vim: set expandtab:

