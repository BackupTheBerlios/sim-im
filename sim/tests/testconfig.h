
#ifndef SIM_TESTCONFIG_H
#define SIM_TESTCONFIG_H

#include <QtTest/QtTest>
#include <QObject>
#include <QByteArray>

namespace testConfig
{
    class Test : public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void init();
        void cleanup();

        void testSerialization();
        void testDeserialization();
    private:
        QByteArray m_config;

    };
}

#endif

// vim: set expandtab:

