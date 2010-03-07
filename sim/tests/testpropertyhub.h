
#ifndef SIM_TESTPROPERTYHUB_H
#define SIM_TESTPROPERTYHUB_H

#include <QtTest/QtTest>
#include <QObject>
#include "propertyhub.h"

namespace testPropertyHub
{
    using namespace SIM;
    class Test : public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testDataManipulation();
        void testSerializationInnerByteArray();
        void testSerializationInnerStringList();
        void testSerialization();
    private:
    };
}

#endif

// vim: set expandtab:

