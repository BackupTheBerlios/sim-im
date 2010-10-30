#ifndef MOCKCLIENT_H
#define MOCKCLIENT_H

#include <gmock/gmock.h>
#include <QSharedPointer>
#include "contacts/client.h"

namespace MockObjects
{
    class MockClient : public SIM::Client
    {
    public:
        MockClient(SIM::Protocol* protocol) : Client(protocol) {}
        MOCK_METHOD0(name, QString());
        MOCK_METHOD0(createIMContact, SIM::IMContactPtr());
        MOCK_METHOD0(createIMGroup, SIM::IMGroupPtr());
        MOCK_METHOD2(createSetupWidget, QWidget*(const QString& id, QWidget* parent));
        MOCK_METHOD0(destroySetupWidget, void());
        MOCK_CONST_METHOD0(availableSetupWidgets, QStringList());
        MOCK_METHOD0(currentStatus, SIM::IMStatusPtr());
        MOCK_METHOD1(changeStatus, void(const SIM::IMStatusPtr& status));
        MOCK_METHOD0(savedStatus, SIM::IMStatusPtr());
        MOCK_METHOD0(ownerContact, SIM::IMContactPtr());
        MOCK_METHOD1(setOwnerContact, void(SIM::IMContactPtr contact));
        MOCK_METHOD1(serialize, bool(QDomElement& element));
        MOCK_METHOD1(deserialize,bool(QDomElement& element));
        MOCK_METHOD1(deserialize, bool(Buffer* buf));
        MOCK_METHOD1(createSearchWidow, QWidget*(QWidget *parent));
        MOCK_METHOD0(groups, QList<SIM::IMGroupPtr>());
        MOCK_METHOD0(contacts, QList<SIM::IMContactPtr>());
    };

    typedef QSharedPointer<MockClient> MockClientPtr;
}

#endif // MOCKCLIENT_H
