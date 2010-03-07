
#include "testcontactlist.h"

namespace testContactList
{
    TestClient::TestClient(Protocol* protocol, Buffer* buf) : Client(protocol, buf)
    {
    }

    QString TestClient::name()
    {
        return QString();
    }

    QString TestClient::dataName(void*)
    {
        return QString();
    }

    QWidget* TestClient::setupWnd()
    {
        return NULL;
    }

    bool TestClient::isMyData(IMContact*& data, Contact*& contact)
    {
        return false;
    }

    bool TestClient::createData(IMContact*& data, Contact* contact)
    {
        return false;
    }

    void TestClient::contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons)
    {
    }

    void TestClient::setupContact(Contact*, void *data)
    {
    }

    bool TestClient::send(Message*, void *data)
    {
        return false;
    }

    bool TestClient::canSend(unsigned type, void *data)
    {
        return false;
    }

    QWidget* TestClient::searchWindow(QWidget *parent)
    {
        return NULL;
    }

    Test::Test() : QObject(), SIM::EventReceiver()
    {
    }

    bool Test::processEvent(SIM::Event* event)
    {
        switch(event->type())
        {
            case eEventGroup:
            {
                SIM::EventGroup* ev = static_cast<EventGroup*>(event);
                if(ev->action() == SIM::EventGroup::eAdded)
                {
                    m_groupAdded++;
                }
                break;
            }
            case eEventContact:
            {
                SIM::EventContact* ev = static_cast<EventContact*>(event);
                if(ev->action() == SIM::EventContact::eCreated)
                {
                    m_contactAdded++;
                }
                break;
            }
            default:
            break;
        }

        return false;
    }


    void Test::init()
    {
        m_groupAdded = 0;
        SIM::createContactList();
        m_contactList = SIM::getContacts();
    }

    void Test::cleanup()
    {
        SIM::destroyContactList();
    }

    void Test::testClientManipulation()
    {
        TestClient* client1 = new TestClient(0, 0);
        TestClient* client2 = new TestClient(0, 0);
        getContacts()->addClient(client1);
        getContacts()->addClient(client2);
        QCOMPARE(client1, getContacts()->getClient(0));
        QCOMPARE(client2, getContacts()->getClient(1));

        getContacts()->moveClient(client1, false);
        QCOMPARE(client2, getContacts()->getClient(0));
        QCOMPARE(client1, getContacts()->getClient(1));
    }

    void Test::testGroupManipulation()
    {
        QCOMPARE(getContacts()->groupCount(), 1);
        Group* zero = getContacts()->group(0);

        Group* group1 = getContacts()->group(12, true);
        QVERIFY(group1);
        QCOMPARE(m_groupAdded, 1);
        Group* group2 = getContacts()->group(23, true);
        QVERIFY(group2);
        QCOMPARE(m_groupAdded, 2);
        Group* group3 = getContacts()->group(34, true);
        QVERIFY(group3);
        QCOMPARE(m_groupAdded, 3);
        Group* nonexistant = getContacts()->group(66, false);
        QVERIFY(!nonexistant);

        QCOMPARE(getContacts()->groupCount(), 4);
        QCOMPARE(getContacts()->groupIndex(12), 1);
        QCOMPARE(getContacts()->groupIndex(23), 2);
        QCOMPARE(getContacts()->groupIndex(34), 3);
        QCOMPARE(getContacts()->groupIndex(66), -1);

        QCOMPARE(getContacts()->groupExists(12), true);
        QCOMPARE(getContacts()->groupExists(23), true);
        QCOMPARE(getContacts()->groupExists(34), true);
        QCOMPARE(getContacts()->groupExists(66), false);
        QCOMPARE(getContacts()->groupExists(42), false);

        getContacts()->moveGroup(12, false);
        QCOMPARE(getContacts()->groupIndex(12), 2);
        QCOMPARE(getContacts()->groupIndex(23), 1);

        getContacts()->moveGroup(12, false);
        QCOMPARE(getContacts()->groupIndex(12), 3);
        QCOMPARE(getContacts()->groupIndex(34), 2);

        getContacts()->moveGroup(34, true);
        QCOMPARE(getContacts()->groupIndex(12), 3);
        QCOMPARE(getContacts()->groupIndex(23), 2);
        QCOMPARE(getContacts()->groupIndex(34), 1);

        getContacts()->removeGroup(12);
        QCOMPARE(getContacts()->groupCount(), 3);

        getContacts()->clear();
        QCOMPARE(getContacts()->groupCount(), 1);
    }

    void Test::testGroupIterator()
    {
        Group* zero = getContacts()->group(0);
        Group* group1 = getContacts()->group(12, true);
        Group* group2 = getContacts()->group(23, true);
        Group* group3 = getContacts()->group(34, true);

        SIM::ContactList::GroupIterator it;
        Group* null = 0;
        Group* group = 0;
        group = ++it;
        QCOMPARE(group, zero);
        group = ++it;
        QCOMPARE(group, group1);
        group = ++it;
        QCOMPARE(group, group2);
        group = ++it;
        QCOMPARE(group, group3);
        group = ++it;
        QCOMPARE(group, null);
    }

    void Test::testPacketTypeManipulation()
    {
        getContacts()->addPacketType(1, "alpha", true);
        getContacts()->addPacketType(2, "beta", false);
        getContacts()->addPacketType(3, "gamma", true);

        QCOMPARE(getContacts()->getPacketType(42), (void*)NULL);

        PacketType* type = 0;
        SIM::ContactList::PacketIterator it;
        type = ++it;
        QCOMPARE(type->id(), (unsigned int)1);
        QCOMPARE(type->name(), QString("alpha"));
        type = ++it;
        QCOMPARE(type->id(), (unsigned int)2);
        QCOMPARE(type->name(), QString("beta"));
        type = ++it;
        QCOMPARE(type->id(), (unsigned int)3);
        QCOMPARE(type->name(), QString("gamma"));
        type = ++it;
        QCOMPARE(type, (void*)NULL);
    }

    void Test::testContactManipulation()
    {
        Contact* c = getContacts()->contact(2, true);
        c->addPhone("0-0-0");
        c->addPhone("1-2-3");

        Contact* c2 = getContacts()->contact(3, true);
        c->addPhone("42");
        c->addPhone("666");

        QCOMPARE(getContacts()->contactByPhone("1-2-3")->id(), c->id());
        QCOMPARE(getContacts()->contactByPhone("666")->id(), c2->id());
    }

    void Test::testGetCodec()
    {
        Contact* c = getContacts()->contact(10, true);
        c->setEncoding("CP 1251");
        QTextCodec* codec = getContacts()->getCodec(c);
        QString s = codec->toUnicode("\xc0\xc1\xc2");
        QCOMPARE(s.length(), 3);
        QCOMPARE(s.at(0).unicode(), (unsigned short)0x0410);
        QCOMPARE(s.at(1).unicode(), (unsigned short)0x0411);
        QCOMPARE(s.at(2).unicode(), (unsigned short)0x0412);
    }

    void Test::testEncoding()
    {
        Contact* c = getContacts()->contact(10, true);
        c->setEncoding("CP 1251");

        QByteArray arr;
        arr.append((char)0xc0);
        arr.append((char)0xc1);
        arr.append((char)0xc2);
        QString s = getContacts()->toUnicode(c, arr);

        QCOMPARE(s.length(), 3);
        QCOMPARE(s.at(0).unicode(), (unsigned short)0x0410);
        QCOMPARE(s.at(1).unicode(), (unsigned short)0x0411);
        QCOMPARE(s.at(2).unicode(), (unsigned short)0x0412);
    }
}

// vim: set expandtab:

