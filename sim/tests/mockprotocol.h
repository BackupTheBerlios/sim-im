#ifndef MOCKPROTOCOL_H
#define MOCKPROTOCOL_H

#include "contacts/imcontact.h"
#include "contacts/protocol.h"

namespace test
{
    using namespace SIM;

    class MockUserData : public SIM::IMContact
    {
    public:
        MockUserData();
        SIM::Data Alpha;
    };

    class MockProtocol : public Protocol
    {
    public:
        MockProtocol();
        virtual QSharedPointer<Client> createClient(Buffer *cfg);
        virtual QSharedPointer<Client> createClient(const QString& n, Buffer *cfg);
        virtual const CommandDef *description();
        virtual const CommandDef *statusList();
        virtual QStringList statuses();
        virtual IMStatusPtr status(const QString& id);
        virtual const DataDef *userDataDef();
    };
}

#endif // MOCKPROTOCOL_H