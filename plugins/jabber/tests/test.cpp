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

void registerEvents()
{
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("init"));
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("init_abort"));
    SIM::getEventHub()->registerEvent(SIM::StandardEvent::create("quit"));
    SIM::getEventHub()->registerEvent(SIM::LogEvent::create());
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    SIM::createEventHub();
    StubObjects::StubImageStorage imagestorage;
    SIM::setImageStorage(&imagestorage);
    SIM::createCommandHub();
    SIM::createContactList();
    registerEvents();
    return RUN_ALL_TESTS();
}

