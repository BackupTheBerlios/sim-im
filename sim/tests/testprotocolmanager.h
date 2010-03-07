
#ifndef SIM_TESTPROTOCOLMANAGER_H
#define SIM_TESTPROTOCOLMANAGER_H

#include <QtTest/QtTest>
#include <QObject>
#include "contacts/protocolmanager.h"

namespace testProtocolManager
{
    using namespace SIM;
    class Test : public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

		void protocolManipulation();
		void testProtocolIterator();
        
    private:
    };

}

#endif

// vim: set expandtab:

