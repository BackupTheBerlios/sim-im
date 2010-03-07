#ifndef SIM_TESTCONTACT_H
#define SIM_TESTCONTACT_H

#include <QObject>
#include <QtTest>

namespace testContact
{
    class Test: public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testAccessors();
        void testCompositeProperties();
    };
}

#endif

// vim: set expandtab:

