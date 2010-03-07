#include "mockprotocol.h"
#include "mockclient.h"

namespace test
{
    DataDef mockUserData[] =
        {
         { "Alpha", DATA_UTF, 1, 0 },
         { NULL, DATA_UNKNOWN, 0, 0 }
        };

    MockUserData::MockUserData()
    {
        Sign.asLong() = 1;
    }

    MockProtocol::MockProtocol() : Protocol(0)
    {
    }

    QSharedPointer<Client> MockProtocol::createClient(Buffer *cfg)
    {
        return createClient("mock", cfg);
    }

    QSharedPointer<Client> MockProtocol::createClient(const QString& n, Buffer *cfg)
    {
        return QSharedPointer<Client>(new MockClient(n, this));
    }

    const CommandDef *MockProtocol::description()
    {
        return 0;
    }

    const CommandDef *MockProtocol::statusList()
    {
        return 0;
    }

    QStringList MockProtocol::statuses()
    {
        return QStringList();
    }

    IMStatusPtr MockProtocol::status(const QString& id)
    {
        return IMStatusPtr();
    }

    const DataDef *MockProtocol::userDataDef()
    {
        return mockUserData;
    }
}
