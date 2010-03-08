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
        void init();
        void cleanup();

        void testAccessors();
        void testCompositeProperties();
        void testClientData();
        void testClientDataPartialJoin();
        void testClientDataPersistance();
    };
}

#endif

// vim: set expandtab:

