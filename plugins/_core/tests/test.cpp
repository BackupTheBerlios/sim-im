
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include "events/eventhub.h"
#include "imagestorage/imagestorage.h"
#include "commands/commandhub.h"
#include "events/standardevent.h"
#include "events/logevent.h"
#include "contacts/contactlist.h"
#include "contacts.h"
#include "tests/stubs/stubimagestorage.h"

#include <QModelIndex>

void registerEvents()
{
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("init"));
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("init_abort"));
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("quit"));
    SIM::getEventHub()->registerEvent(SIM::LogEvent::create());
}

StubObjects::StubImageStorage* imagestorage;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    qRegisterMetaType<QModelIndex>("QModelIndex");
    SIM::createEventHub();
	SIM::setImageStorage(imagestorage);
    //SIM::setImageStorage(&imagestorage);
    SIM::createCommandHub();
    registerEvents();
    int ret = RUN_ALL_TESTS();
#ifdef WIN32
    getchar();
#endif
	return ret;
}

