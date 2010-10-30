#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "contacts/standardcontactlist.h"
#include "contacts/contactlist.h"

namespace
{
    using namespace SIM;
    class TestStandardContactList : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            SIM::createContactList();
        }

        virtual void TearDown()
        {
            SIM::destroyContactList();
        }
    };

    TEST_F(TestStandardContactList, createContact_CreatesContactWithGivenId)
    {
        ContactPtr contact = getContactList()->createContact(42);

        ASSERT_TRUE(contact);
        ASSERT_EQ(contact->id(), 42);
    }

    TEST_F(TestStandardContactList, createGroup_CreatesGroupWithGivenId)
    {
        GroupPtr group = getContactList()->createGroup(56);

        ASSERT_TRUE(group);
        ASSERT_EQ(group->id(), 56);
    }

    TEST_F(TestStandardContactList, addContact_NoContactWithGivenIdAdded_AddsContact)
    {
        ContactPtr contact = getContactList()->createContact(42);
        ASSERT_TRUE(contact);

        bool success = getContactList()->addContact(contact);

        EXPECT_TRUE(success);
        EXPECT_TRUE(contact == getContactList()->contact(42));
    }

    TEST_F(TestStandardContactList, addContact_ContactWithGivenIdAlreadyAdded_KeepsOldContact)
    {
        ContactPtr contact = getContactList()->createContact(42);
        ContactPtr contact2 = getContactList()->createContact(42);
        ASSERT_TRUE(contact);
        ASSERT_TRUE(contact2);
        bool success = getContactList()->addContact(contact);

        success = getContactList()->addContact(contact2);

        EXPECT_FALSE(success);
        EXPECT_TRUE(contact == getContactList()->contact(42));
    }

    TEST_F(TestStandardContactList, removeContact)
    {
        ContactPtr contact = getContactList()->createContact(42);
        ASSERT_TRUE(contact);
        getContactList()->addContact(contact);
        EXPECT_TRUE(contact == getContactList()->contact(42));

        getContactList()->removeContact(42);

        EXPECT_FALSE(getContactList()->contactExists(42));
    }

    TEST_F(TestStandardContactList, contactCount)
    {
        ContactPtr contact = getContactList()->createContact(42);
        getContactList()->addContact(contact);

        EXPECT_EQ(1, getContactList()->contactIds().size());
    }

    TEST_F(TestStandardContactList, addGroup_NoGroupWithGivenIdAdded_AddsGroup)
    {
        GroupPtr group = getContactList()->createGroup(56);
        ASSERT_TRUE(group);
        bool success = getContactList()->addGroup(group);
        EXPECT_TRUE(success);
        EXPECT_TRUE(group == getContactList()->group(56));
    }

    TEST_F(TestStandardContactList, addGroup_GroupWithGivenIdAlreadyAdded_KeepsOldGroup)
    {
        GroupPtr group = getContactList()->createGroup(56);
        GroupPtr group2 = getContactList()->createGroup(56);
        ASSERT_TRUE(group);
        ASSERT_TRUE(group2);
        bool success = getContactList()->addGroup(group);

        success = getContactList()->addGroup(group2);

        EXPECT_FALSE(success);
        EXPECT_TRUE(group == getContactList()->group(56));
    }

    TEST_F(TestStandardContactList, removeGroup)
    {
        GroupPtr group = getContactList()->createGroup(56);
        ASSERT_TRUE(group);
        getContactList()->addGroup(group);
        EXPECT_TRUE(group == getContactList()->group(56));

        getContactList()->removeGroup(56);

        EXPECT_FALSE(getContactList()->group(56));
    }

    TEST_F(TestStandardContactList, allGroups)
    {
        GroupPtr group1 = getContactList()->createGroup(56);
        GroupPtr group2 = getContactList()->createGroup(67);
        ASSERT_TRUE(group1);
        ASSERT_TRUE(group2);

        getContactList()->addGroup(group1);
        getContactList()->addGroup(group2);

        QList<GroupPtr> groups = getContactList()->allGroups();

        EXPECT_TRUE(groups.contains(group1));
        EXPECT_TRUE(groups.contains(group2));
    }

    TEST_F(TestStandardContactList, contactsForGroup)
    {
        ContactPtr contact1 = getContactList()->createContact(42);
        ContactPtr contact2 = getContactList()->createContact(43);
        contact1->setGroupId(12);

        getContactList()->addContact(contact1);
        getContactList()->addContact(contact2);
        QList<ContactPtr> contacts = getContactList()->contactsForGroup(12);

        EXPECT_TRUE(contacts.contains(contact1));
        EXPECT_FALSE(contacts.contains(contact2));
    }

    TEST_F(TestStandardContactList, contactIds_filtersIgnoredContacts)
    {
        int ignoredContactId = 43;
        ContactPtr contact1 = getContactList()->createContact(42);
        ContactPtr contact2 = getContactList()->createContact(ignoredContactId);
        contact2->setFlag(Contact::flIgnore, true);
        ContactPtr contact3 = getContactList()->createContact(44);
        getContactList()->addContact(contact1);
        getContactList()->addContact(contact2);
        getContactList()->addContact(contact3);

        QList<int> ids = getContactList()->contactIds();

        ASSERT_FALSE(ids.contains(ignoredContactId));


    }
}
