#ifndef MOCKCLIENTMANAGER_H
#define MOCKCLIENTMANAGER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "clientmanager.h"

namespace MockObjects
{
    class MockClientManager : public SIM::ClientManager
    {
    public:
        MOCK_METHOD1(addClient, void(ClientPtr client));
        MOCK_METHOD1(client, ClientPtr(const QString& name));
        MOCK_METHOD0(clientList, QStringList());

        MOCK_METHOD0(load, bool());
        MOCK_METHOD0(save, bool());
    };
}

#endif // MOCKCLIENTMANAGER_H
