
#include "contacts/client.h"
#include "stubimcontact.h"
#include "stubimgroup.h"

namespace StubObjects
{
    class StubClient : public SIM::Client
    {
    public:
        StubClient(SIM::Protocol* proto, const QString& id) : SIM::Client(proto), m_name(id)
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

        virtual QWidget* createStatusWidget()
        {
            return 0;
        }

        virtual SIM::IMStatusPtr currentStatus()
        {
            return SIM::IMStatusPtr();
        }

        virtual void changeStatus(const SIM::IMStatusPtr& status)
        {
            Q_UNUSED(status);
        }

        virtual SIM::IMStatusPtr savedStatus()
        {
            return SIM::IMStatusPtr();
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

		virtual QString retrievePasswordLink()
        {
            return QString();
        }

    private:
        QString m_name;
    };
}

// vim: set expandtab:

