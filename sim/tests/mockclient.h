#ifndef MOCKCLIENT_H
#define MOCKCLIENT_H

#include "contacts/client.h"

namespace test
{
    using namespace SIM;
    class MockClient : public Client
    {
    public:
        MockClient(const QString& n, Protocol* protocol);
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
        virtual SIM::IMContact*  getOwnerContact() {};
        virtual void setOwnerContact(SIM::IMContact* contact) {};
    private:
        QString m_name;
    };
}

#endif // MOCKCLIENT_H
