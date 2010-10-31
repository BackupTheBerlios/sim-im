#ifndef STUBIMGROUP_H
#define STUBIMGROUP_H

#include <QSharedPointer>
#include "contacts/imgroup.h"
#include "contacts/client.h"

namespace StubObjects
{
    class StubIMGroup : public SIM::IMGroup
    {
    public:
        StubIMGroup(SIM::Client* cl)
        {
            m_client = cl;
        }

        virtual SIM::Client* client()
        {
            return m_client;
        }

        virtual QList<SIM::IMContactPtr> contacts()
        {
            return QList<SIM::IMContactPtr>();
        }

        virtual bool serialize(QDomElement& /*element*/)
        {
            return false;
        }

        virtual bool deserialize(QDomElement& /*element*/)
        {
            return false;
        }

        virtual bool deserialize(const QString& /*data*/)
        {
            return false;
        }

    private:
        SIM::Client* m_client;
    };
}

#endif // STUBIMGROUP_H
