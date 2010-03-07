
#include "testclientmanager.h"
#include "clientmanager.h"
#include "contacts.h"

namespace testClientManager
{
    TestClient::TestClient(const QString& n, Protocol* protocol) : Client(protocol, 0),
        m_name(n)
    {
    }

    QString TestClient::name()
    {
        return m_name;
    }

    QString TestClient::dataName(void*)
    {
        return QString();
    }

    QWidget* TestClient::setupWnd()
    {
        return NULL;
    }

	bool TestClient::isMyData(clientData*& /*data*/, Contact*& /*contact*/)
    {
        return false;
    }

	bool TestClient::createData(clientData*& /*data*/, Contact* /*contact*/)
    {
        return false;
    }

	void TestClient::contactInfo(void* /*clientData*/, unsigned long& /*status*/, unsigned& /*style*/, QString& /*statusIcon*/, QSet<QString>* /*icons*/)
    {
    }

	void TestClient::setupContact(Contact*, void* /*data*/)
    {
    }

	bool TestClient::send(Message*, void* /*data*/)
    {
        return false;
    }

	bool TestClient::canSend(unsigned /*type*/, void* /*data*/)
    {
        return false;
    }

	QWidget* TestClient::searchWindow(QWidget* /*parent*/)
    {
        return NULL;
    }
    void Test::initTestCase()
    {
        SIM::createClientManager();
        QVERIFY(SIM::getClientManager());
        SIM::createContactList();
    }

    void Test::cleanupTestCase()
    {
        SIM::destroyClientManager();
        QVERIFY(!SIM::getClientManager());
        SIM::destroyContactList();
    }

    void Test::testClientManipulation()
    {
        SIM::ClientPtr icqclient = SIM::ClientPtr(new TestClient("ICQ.666666666", 0));
        SIM::ClientPtr jabberclient = SIM::ClientPtr(new TestClient("Jabber.loh@jabber.org", 0));
        QCOMPARE(icqclient->name(), QString("ICQ.666666666"));
        SIM::getClientManager()->addClient(icqclient);
        SIM::getClientManager()->addClient(jabberclient);
        SIM::ClientPtr client2 = SIM::getClientManager()->client("ICQ.666666666");
        QCOMPARE(client2->name(), QString("ICQ.666666666"));

        SIM::ClientPtr nonexistant = SIM::getClientManager()->client("Nothing");
        QVERIFY(nonexistant.isNull());
    }
}

// vim: set expandtab:

