#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QDomElement>
#include "userdata.h"
#include "simapi.h"
#include "contact.h"
#include "group.h"

namespace SIM {

class EXPORT ContactList
{
public:
    virtual ~ContactList() {}

    virtual void clear() = 0;
    virtual bool load() = 0;
    virtual bool save() = 0;

    virtual bool addContact(const ContactPtr& newContact) = 0;
    virtual ContactPtr contact(int id) const = 0;
    virtual bool contactExists(int id) const = 0;
    virtual void removeContact(int id) = 0;
    virtual ContactPtr createContact(int id) = 0;
    virtual QList<int> contactIds() const = 0;

    virtual bool addGroup(const GroupPtr& group) = 0;
    virtual GroupPtr group(int id) const = 0;
    virtual void removeGroup(int id) = 0;
    virtual GroupPtr createGroup(int id) = 0;

    virtual ContactPtr ownerContact() = 0;

    virtual QList<GroupPtr> allGroups() const = 0;
    virtual QList<ContactPtr> contactsForGroup(int groupId) = 0;
    virtual void incomingMessage(const MessagePtr& message) = 0;
    virtual UserDataPtr userdata() const = 0;
};

EXPORT void createContactList();
EXPORT void destroyContactList();
EXPORT void setContactList(ContactList* cl);
EXPORT ContactList *getContactList();


} // namespace SIM

#endif // CONTACTLIST_H
