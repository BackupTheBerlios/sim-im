#include "mockprotocol.h"
#include "mockclient.h"
#include <QStringList>
#include <stdio.h>

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
        //Sign.asLong() = 1;
    }

    unsigned long MockUserData::getSign()
    {
        return 1;
    }

    QByteArray MockUserData::serialize()
    {
        QString result;
        result += QString("Sign=%1\n").arg(getSign());
        result += QString("LastSend=%1\n").arg(getLastSend());
        result += QString("Alpha=%1\n").arg(Alpha.toULong());
        return result.toLocal8Bit();
    }

    void MockUserData::deserializeLine(const QString& key, const QString& value)
    {
        QString val = value;
        if(val.startsWith('\"') && val.endsWith('\"'))
            val = val.mid(1, val.length() - 2);
        if(key == "Sign") {
            //Sign.asULong() = val.toULong();
        }
        else if(key == "LastSend") {
            setLastSend(val.toULong());
        }
        else if(key == "Alpha") {
            Alpha.asULong() = val.toULong();
        }
    }

    void MockUserData::deserialize(Buffer* cfg)
    {
        while(1) {
            const QString line = QString::fromLocal8Bit(cfg->getLine());
            if (line.isEmpty())
                break;
            QStringList keyval = line.split('=');
            if(keyval.size() < 2)
                continue;
            deserializeLine(keyval.at(0), keyval.at(1));
        }
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

    QSharedPointer<Client> MockProtocol::createClient(const QString& name)
    {
        return createClient(name, 0);
    }

    const CommandDef *MockProtocol::description()
    {
        return &mock_descr;
    }

    const CommandDef *MockProtocol::statusList()
    {
        return 0;
    }

    QStringList MockProtocol::states()
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

    IMContact* MockProtocol::createIMContact(const QSharedPointer<SIM::Client>& client)
    {
        return new MockUserData();
    }
}
