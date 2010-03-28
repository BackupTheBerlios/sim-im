
#ifndef SIM_TESTCONTACTLIST_H
#define SIM_TESTCONTACTLIST_H

#include <QtTest/QtTest>
#include <QObject>
#include "contacts.h"
#include "contacts/client.h"
#include "event.h"

namespace testContactList
{
    using namespace SIM;
    class TestClient : public Client
    {
    public:
        TestClient(Protocol* protocol, Buffer* buf);
        virtual QString name();
        virtual QString dataName(void*);
        virtual QWidget* setupWnd();
        virtual bool isMyData(IMContact*&, Contact*&);
        virtual bool createData(IMContact*&, Contact*);
        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        virtual void setupContact(Contact*, void *data);
        virtual bool send(Message*, void *data);
        virtual bool canSend(unsigned type, void *data);
        virtual QWidget* searchWindow(QWidget *parent);
        QStringList statuses() { return QStringList(); };
        virtual SIM::IMContact*  getOwnerContact() {};
        virtual void setOwnerContact(SIM::IMContact* contact) {};
    };

    class Test: public QObject, public SIM::EventReceiver
    {
        Q_OBJECT
    public:
        Test();
        virtual bool processEvent(SIM::Event*);
    private slots:
        void init();
        void cleanup();

        void testClientManipulation();
        void testGroupManipulation();
        void testGroupIterator();
        void testPacketTypeManipulation();
        void testContactManipulation();
        void testGetCodec();
        void testEncoding();
        
    private:
        ContactList* m_contactList;
        int m_groupAdded;
        int m_contactAdded;
    };

}

#endif

// vim: set expandtab:

