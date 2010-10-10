
#include "contacts/client.h"
#include "stubimcontact.h"
#include "stubimgroup.h"

namespace StubObjects
{
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

        virtual SIM::IMContactPtr createIMContact()
        {
            return SIM::IMContactPtr();
        }

        virtual SIM::IMGroupPtr createIMGroup()
        {
            return SIM::IMGroupPtr();
        }

        virtual QWidget* createSetupWidget(const QString& id, QWidget* parent)
        {
            Q_UNUSED(id);
            Q_UNUSED(parent);
            return NULL;
        }

        virtual void destroySetupWidget()
        {
        }

        virtual QStringList availableSetupWidgets() const
        {
            return QStringList();
        }

        virtual SIM::IMContactPtr ownerContact()
        {
            return SIM::IMContactPtr();
        }

        virtual void setOwnerContact(SIM::IMContactPtr contact)
        {
            Q_UNUSED(contact);
        }

        virtual bool serialize(QDomElement& element)
        {
            Q_UNUSED(element);
            return false;
        }

        virtual bool deserialize(QDomElement& element)
        {
            Q_UNUSED(element);
            return false;
        }

        virtual bool deserialize(Buffer* buf)
        {
            Q_UNUSED(buf);
            return false;
        }

        virtual QWidget* createSearchWidow(QWidget *parent)
        {
            Q_UNUSED(parent);
            return NULL;
        }

        virtual QList<SIM::IMGroupPtr> groups()
        {
            return QList<SIM::IMGroupPtr>();
        }

        virtual QList<SIM::IMContactPtr> contacts()
        {
            return QList<SIM::IMContactPtr>();
        }

    private:
        QString m_name;
    };
}

// vim: set expandtab:

