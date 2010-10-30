
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/gtest-qt.h"

#include <QPixmap>

#include "roster/userviewmodel.h"
#include "contacts/contactlist.h"
#include "tests/mocks/mockcontactlist.h"
#include "tests/mocks/mockimcontact.h"
#include "tests/mocks/mockimstatus.h"

namespace
{
    using namespace SIM;
    using namespace MockObjects;
    using ::testing::Return;
    class TestUserViewModel : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            contactList = new MockObjects::MockContactList();
        }

        virtual void TearDown()
        {
            delete contactList;
        }

        ContactPtr makeContact(int id, bool offline, const QPixmap& icon = QPixmap())
        {
            MockIMStatusPtr imstatus = MockIMStatusPtr(new MockIMStatus());
            ON_CALL(*imstatus.data(), flag(IMStatus::flOffline)).WillByDefault(Return(offline));
            ON_CALL(*imstatus.data(), icon()).WillByDefault(Return(icon));

            MockIMContactPtr imcontact = MockIMContactPtr(new MockIMContact());
            ON_CALL(*imcontact.data(), status()).WillByDefault(Return(imstatus));

            ContactPtr contact = ContactPtr(new Contact(id));
            contact->addClientContact(imcontact);
            return contact;
        }

        ContactPtr insertContactToContactList(bool offline)
        {
            int contactId = 12;
            QList<int> contactIds;
            contactIds.append(contactId);
            ContactPtr contact = makeContact(contactId, offline);
            ON_CALL(*contactList, contactIds()).WillByDefault(Return(contactIds));
            ON_CALL(*contactList, contact(contactId)).WillByDefault(Return(contact));
            return contact;
        }

        ContactPtr insertContactToContactListWithStatusIcon(bool offline, const QPixmap& icon)
        {
            int contactId = 12;
            QList<int> contactIds;
            contactIds.append(contactId);
            ContactPtr contact = makeContact(contactId, offline, icon);
            ON_CALL(*contactList, contactIds()).WillByDefault(Return(contactIds));
            ON_CALL(*contactList, contact(contactId)).WillByDefault(Return(contact));
            return contact;
        }

        QPixmap makeTestIcon()
        {
            QPixmap img(5, 5);
            img.fill(0xff123456);
            return img;
        }

        QModelIndex createContactIndex(UserViewModel& model, int row, bool offline)
        {
            return model.index(row, 0, model.index(offline ? UserViewModel::OfflineRow : UserViewModel::OnlineRow, 0));
        }

        MockObjects::MockContactList* contactList;
    };

    TEST_F(TestUserViewModel, columnCount_returns1)
    {
        UserViewModel model(contactList);

        ASSERT_EQ(1, model.columnCount());
    }

    TEST_F(TestUserViewModel, rowCount_withoutParent_returns2)
    {
        UserViewModel model(contactList);

        int rows = model.rowCount();

        ASSERT_EQ(2, rows);
    }

    TEST_F(TestUserViewModel, index_withoutParent_correct)
    {
        UserViewModel model(contactList);
        QModelIndex index;

        index = model.index(0, 0);
        ASSERT_TRUE(index.isValid()) << "Online group index";

        index = model.index(1, 0);
        ASSERT_TRUE(index.isValid()) << "Offline group index";
    }

    TEST_F(TestUserViewModel, rowCount_online)
    {
        insertContactToContactList(false);
        UserViewModel model(contactList);

        int rows = model.rowCount(model.index(UserViewModel::OnlineRow, 0));

        ASSERT_EQ(1, rows);
    }

    TEST_F(TestUserViewModel, rowCount_offline)
    {
        insertContactToContactList(true);
        UserViewModel model(contactList);

        int rows = model.rowCount(model.index(UserViewModel::OfflineRow, 0));

        ASSERT_EQ(1, rows);
    }

    TEST_F(TestUserViewModel, data_online_name)
    {
        ContactPtr contact = insertContactToContactList(false);
        contact->setName("Foo");
        UserViewModel model(contactList);

        QModelIndex index = createContactIndex(model, 0, false);
        ASSERT_EQ(0, index.column());
        QString contactName = model.data(index, Qt::DisplayRole).toString();

        ASSERT_EQ(contact->name(), contactName);
    }

    TEST_F(TestUserViewModel, data_offline_name)
    {
        ContactPtr contact = insertContactToContactList(true);
        contact->setName("Bar");
        UserViewModel model(contactList);

        QModelIndex index = createContactIndex(model, 0, true);
        ASSERT_EQ(0, index.column());
        QString contactName = model.data(index, Qt::DisplayRole).toString();

        ASSERT_EQ(contact->name(), contactName);
    }

    TEST_F(TestUserViewModel, data_offline_icon)
    {
        QPixmap icon = makeTestIcon();
        ContactPtr contact = insertContactToContactListWithStatusIcon(true, icon);
        contact->setName("Bar");
        UserViewModel model(contactList);

        QModelIndex index = createContactIndex(model, 0, true);
        ASSERT_EQ(0, index.column());
        QVariant currentIconVariant = model.data(index, Qt::DecorationRole);

        ASSERT_TRUE(currentIconVariant.isValid());
        QPixmap currentIcon = currentIconVariant.value<QPixmap>();
        ASSERT_TRUE(currentIcon.toImage() == icon.toImage());
    }
}
