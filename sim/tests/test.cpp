
#include <QApplication>
#include <QtTest/QtTest>
#include "testbuffer.h"
#include "testsocketfactory.h"
#include "testclientmanager.h"
#include "testclientsocket.h"
#include "testconfig.h"
#include "testprotocolmanager.h"
#include "testcontact.h"
#include "testcontactlist.h"
#include "testpropertyhub.h"
#include "testuserdata.h"
#include "testeventhub.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QTest::qExec(new TestBuffer());
    QTest::qExec(new testSocketFactory::TestSocketFactory());
    QTest::qExec(new testProtocolManager::Test());
    QTest::qExec(new testPropertyHub::Test());
    QTest::qExec(new testContactList::Test());
    QTest::qExec(new testUserData::Test());
    QTest::qExec(new testConfig::Test());
    //QTest::qExec(new testClientManager::Test());
    //QTest::qExec(new testContact::Test());
    QTest::qExec(new testEventHub::Test());
    return 0;
}

