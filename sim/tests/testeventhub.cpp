#include "testeventhub.h"
#include "events/eventhub.h"
#include "events/standardevent.h"

namespace testEventHub
{

void Test::testRegistration()
{
    bool rc;
    rc = SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("test_event1"));
    QCOMPARE(rc, true);
    rc = SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("test_event2"));
    QCOMPARE(rc, true);
    rc = SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("test_event2"));
    QCOMPARE(rc, false);
}

void Test::testEventConnection()
{
    m_testEvent1 = 0;
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("test_event1"));
    SIM::IEventPtr ev = SIM::getEventHub()->getEvent("test_event1");
    QVERIFY(!ev.isNull());
    ev->connectTo(this, SLOT(testEvent1()));
    SIM::getEventHub()->triggerEvent("test_event1");
    QCOMPARE(m_testEvent1, 1);
}

void Test::init()
{
    SIM::createEventHub();
}

void Test::cleanup()
{
    SIM::destroyEventHub();
}

void Test::testEvent1()
{
    m_testEvent1++;
}

}
