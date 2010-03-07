
#ifndef SIM_TESTSOCKETFACTORY_H
#define SIM_TESTSOCKETFACTORY_H

#include <cstdio>
#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QtTest/QtTest>
#include "socket/iresolver.h"
#include "socket/socketfactory.h"

namespace testSocketFactory
{

class TestResolver : public QObject, public SIM::IResolver
{
    Q_OBJECT
public:
    TestResolver(QObject* parent, const QString& host) : QObject(parent),
            m_host(host)
    {
        if(host == "test.com")
        {
            m_addr = 0xc0a80101;
            m_done = true;
            m_timeout = false;
        }
        if(host == "timeout.com")
        {
            m_addr = 0;
            m_done = true;
            m_timeout = true;
        }
        QTimer::singleShot(1, this, SLOT(ready()));
    }

    virtual ~TestResolver() {}
    virtual QHostAddress addr()
    {
        return QHostAddress( m_addr );
    }

    virtual QList<QHostAddress> addresses()
    {
        QList<QHostAddress> listAddresses;
        listAddresses << QHostAddress( m_addr );
        return listAddresses;
    }

    virtual QString host() const
    {
        return m_host;
    }

    virtual bool isDone()
    {
        return m_done;
    }

    virtual bool isTimeout()
    {
        return m_timeout;
    }

    virtual SIM::IResolver* clone(const QString& host)
    {
        return new TestResolver(parent(), host);
    }

public slots:
    void ready()
    {
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

private:
    QString m_host;
    unsigned long m_addr;
    bool m_done;
    bool m_timeout;
};

class TestThread : public QThread
{
    Q_OBJECT
public:
    TestThread(QObject* parent, const QString& host);
    virtual ~TestThread();

protected:
    virtual void run();

private:
    SIM::SocketFactory* m_factory;
    QString m_host;
};

class TestSocketFactory : public QObject
{
    Q_OBJECT
};

}
#endif

// vim: set expandtab:

