#ifndef MOCKCONTACTLIST_H
#define MOCKCONTACTLIST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "contacts/contactlist.h"

namespace MockObjects
{
    class MockContactList : public SIM::ContactList
    {
    public:
        MOCK_METHOD0(clear, void());
        MOCK_METHOD0(load, bool());
        MOCK_METHOD0(save, bool());
        MOCK_METHOD1(addContact, bool(const SIM::ContactPtr& newContact));
        MOCK_CONST_METHOD1(contact, SIM::ContactPtr(int id));
        MOCK_CONST_METHOD1(contactExists, bool(int id));
        MOCK_METHOD1(removeContact, void(int id));
        MOCK_METHOD1(createContact, SIM::ContactPtr(int id));
        MOCK_CONST_METHOD0(contactIds, QList<int>());
        MOCK_METHOD1(addGroup, bool(const SIM::GroupPtr& group));
        MOCK_CONST_METHOD1(group, SIM::GroupPtr(int id));
        MOCK_METHOD1(removeGroup, void(int id));
        MOCK_METHOD1(createGroup, SIM::GroupPtr(int id));
        MOCK_METHOD0(ownerContact, SIM::ContactPtr());
        MOCK_CONST_METHOD0(allGroups, QList<SIM::GroupPtr>());
        MOCK_METHOD1(contactsForGroup, QList<SIM::ContactPtr>(int groupId));
        MOCK_METHOD1(incomingMessage, void(const SIM::MessagePtr& message));
        MOCK_CONST_METHOD0(userdata, SIM::UserDataPtr());
    };
}

#endif // MOCKCONTACTLIST_H
