
#include "contacts/client.h"

namespace StubObjects
{
    using namespace SIM;
    class StubClient : public SIM::Client
    {
    public:
        StubClient(const QString& id) : SIM::Client(0), m_name(id)
        {
        }

        virtual QString name()
        {
            return m_name;
        }

        virtual QString dataName(void*)
        {
            return QString();
        }
        virtual QWidget *setupWnd()
        {
            return 0;
        }

        virtual IMContact* getOwnerContact()
        {
            return 0;
        }

        virtual void setOwnerContact(IMContact* contact)
        {
        }

        virtual QWidget *searchWindow(QWidget *parent)
        {
            return 0;
        }

        virtual bool isMyData(IMContact*&, Contact*&)
        {
            return false;
        }

        virtual bool createData(IMContact*&, Contact*)
        {
            return false;
        }

        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL)
        {
        }

        virtual void setupContact(Contact*, void *data)
        {
        }

        virtual bool send(Message*, void *data) 
        {
            return false;
        }

        virtual bool canSend(unsigned type, void *data)
        {
            return false;
        }
    private:
        QString m_name;
    };
}

// vim: set expandtab:

