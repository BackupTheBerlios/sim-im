
#include "testcontact.h"
#include "contacts/contact.h"
#include "contacts/client.h"

namespace testContact
{
    using namespace SIM;

    struct testClientData : public clientData
    {

    };

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
        //Contact* c = getContacts()->contact(1, true);

    }
}

// vim: set expandtab:

