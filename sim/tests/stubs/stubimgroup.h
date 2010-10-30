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
        StubIMGroup(const SIM::ClientPtr& cl)
        {
            m_client = cl.toWeakRef();
        }

        virtual SIM::ClientWeakPtr client()
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
        SIM::ClientWeakPtr m_client;
    };
}

#endif // STUBIMGROUP_H
