#include "mockprotocol.h"
#include "mockclient.h"

namespace test
{
    DataDef mockUserData[] =
        {
         { "Sign", DATA_ULONG, 1, 0 },
         { "LastSend", DATA_ULONG, 1, 0 },
         { "Alpha", DATA_ULONG, 1, 0 },
         { NULL, DATA_UNKNOWN, 0, 0 }
        };

    static CommandDef mock_descr =
        CommandDef (
            0,
            I18N_NOOP("MOCK"),
            "Mock_online",
            QString::null,
            "",
            0,
            0,
            0,
            0,
            0,
            PROTOCOL_INFO | PROTOCOL_AR | PROTOCOL_ANY_PORT,
            NULL,
            QString::null
        );

    MockUserData::MockUserData()
    {
        Sign.asLong() = 1;
    }

//    void MockUserData::serialize(Buffer* cfg)
//    {
//        load_data(def, data.data, cfg);
//    }
//
//    void MockUserData::deserialize(Buffer* cfg)
//    {
//
//    }

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
        return &mock_descr;
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

    IMContact* MockProtocol::createIMContact()
    {
        return new MockUserData();
    }
}
