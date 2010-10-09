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

        virtual QByteArray serialize();
        virtual void deserialize(Buffer* cfg);

        virtual ClientPtr client() { return ClientPtr(); }
        virtual void serialize(QDomElement& element) {};
        virtual void deserialize(QDomElement& element) {};

        virtual unsigned long getSign();

        SIM::Data Alpha;
        void deserializeLine(const QString& key, const QString& value);
    };

    class MockProtocol : public Protocol
    {
    public:
        MockProtocol();
        virtual QSharedPointer<Client> createClient(Buffer *cfg);
        virtual QSharedPointer<Client> createClient(const QString& name);
        virtual QSharedPointer<Client> createClient(const QString& n, Buffer *cfg);
        virtual const CommandDef *description();
        virtual const CommandDef *statusList();
        virtual QStringList states();
        virtual IMStatusPtr status(const QString& id);
        virtual const DataDef *userDataDef();
        virtual IMContact* createIMContact(const QSharedPointer<SIM::Client>& client);
		virtual SIM::IMStatusInfo* statusInfo(const QString&) {}
    };
}

#endif // MOCKPROTOCOL_H
