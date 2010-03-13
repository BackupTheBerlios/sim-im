
#include "testcontact.h"
#include "contacts/contact.h"
#include "contacts/client.h"
#include "contacts/imcontact.h"
#include "mockprotocol.h"
#include "mockclient.h"

namespace testContact
{
    using namespace SIM;

    void Test::init()
    {
        SIM::createContactList();
    }

    void Test::cleanup()
    {
        SIM::destroyContactList();
    }

    void Test::testAccessors()
    {
        Contact* c = getContacts()->contact(1, true);
        QCOMPARE(c->id(), (unsigned long)1);
        c->setGroup(42);
        QCOMPARE(c->getGroup(), 42);
        c->setIgnore(true);
        QCOMPARE(c->getIgnore(), true);
        c->setName("qux");
        QCOMPARE(c->getName(), QString("qux"));
        c->setLastActive(666);
        QCOMPARE(c->getLastActive(), 666);
        c->setEMails("banana");
        QCOMPARE(c->getEMails(), QString("banana"));
        c->setPhoneStatus(23);
        QCOMPARE(c->getPhoneStatus(), 23);
        c->setNotes("gamma");
        QCOMPARE(c->getNotes(), QString("gamma"));
        c->setFlags(56);
        QCOMPARE(c->getFlags(), 56);
        c->setEncoding("UTF-8");
        QCOMPARE(c->getEncoding(), QString("UTF-8"));
    }

    void Test::testCompositeProperties()
    {
        Contact* c = getContacts()->contact(1, true);
        c->setFirstName("foo", "bar");
        QCOMPARE(c->getFirstName(), QString("foo/bar"));
        c->setLastName("alpha", "beta");
        QCOMPARE(c->getLastName(), QString("alpha/beta"));
    }

    void Test::testClientData()
    {
        Contact* c = getContacts()->contact(1, true);
        test::MockProtocol p;
        ClientPtr client = p.createClient("mock", 0);
        test::MockUserData* d = (test::MockUserData*)c->createData(client.data());
        test::MockUserData* d2 = (test::MockUserData*)c->getData(client.data());
        d->Alpha.asULong() = 12;
        QCOMPARE(d->Alpha.toULong(), 12ul);
        QCOMPARE(d, d2);
        QVERIFY(c->have(d));
        QCOMPARE(c->size(), (unsigned int)1);

        Contact* c2 = getContacts()->contact(2, true);
        test::MockUserData* d3 = (test::MockUserData*)c2->createData(client.data());
        QVERIFY(!c->have(d3));
        c->join(c2);

        QVERIFY(c->have(d3));
    }

    void Test::testClientDataPartialJoin()
    {
        Contact* c1 = getContacts()->contact(22, true);
        Contact* c2 = getContacts()->contact(33, true);
        test::MockProtocol p;
        ClientPtr client1 = p.createClient("mock1", 0);
        ClientPtr client2 = p.createClient("mock2", 0);

        test::MockUserData* d = (test::MockUserData*)c1->createData(client1.data());

        test::MockUserData* d2 = (test::MockUserData*)c2->createData(client1.data());
        test::MockUserData* d3 = (test::MockUserData*)c2->createData(client2.data());

        QVERIFY(!c1->have(d2));
        QVERIFY(c2->have(d2));
        QVERIFY(c2->have(d3));

        c1->join(d2, c2);

        QVERIFY(c1->have(d2));
        QVERIFY(!c2->have(d2));

    }

    void Test::testClientDataPersistance()
    {
        Contact* c1 = getContacts()->contact(1, true);
        test::MockProtocol p;
        ClientPtr client1 = p.createClient("mock1", 0);
        test::MockUserData* d = (test::MockUserData*)c1->createData(client1.data());
        d->Alpha.asULong() = 12ul;
        QByteArray arr = c1->saveUserData();

        Contact* c2 = getContacts()->contact(2, true);
        Buffer buf(arr);
        c2->loadUserData(client1.data(), &buf);
        QCOMPARE(((test::MockUserData*)c2->getData(client1.data()))->Alpha.toULong(), 12ul);
    }
}

// vim: set expandtab:

