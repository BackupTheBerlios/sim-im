
#ifndef SIM_TESTCLIENTMANAGER_H
#define SIM_TESTCLIENTMANAGER_H

#include <QObject>
#include <QtTest/QtTest>

#include "contacts/client.h"

using namespace SIM;
namespace testClientManager
{
    class TestClient : public Client
    {
    public:
        TestClient(const QString& n, Protocol* protocol);
        virtual QString name();
        virtual QString dataName(void*);
        virtual QWidget* setupWnd();
        virtual bool isMyData(clientData*&, Contact*&);
        virtual bool createData(clientData*&, Contact*);
        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        virtual void setupContact(Contact*, void *data);
        virtual bool send(Message*, void *data);
        virtual bool canSend(unsigned type, void *data);
        virtual QWidget* searchWindow(QWidget *parent);
    private:
        QString m_name;
    };

    class Test : public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testClientManipulation();
    };
}

#endif

// vim: set expandtab:

