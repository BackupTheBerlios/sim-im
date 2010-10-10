#ifndef STANDARDCONTACTLIST_H
#define STANDARDCONTACTLIST_H

#include "contactlist.h"

namespace SIM {

class StandardContactList : public ContactList
{
public:
    StandardContactList();

    virtual void clear();
    virtual bool load();
    virtual bool save();
    virtual bool addContact(const ContactPtr& newContact);
    virtual ContactPtr contact(int id) const;
    virtual bool contactExists(int id) const;
    virtual void removeContact(int id);
    virtual ContactPtr createContact(int id);
    virtual bool addGroup(const GroupPtr& group);
    virtual GroupPtr group(int id) const;
    virtual void removeGroup(int id);
    virtual GroupPtr createGroup(int id);
    virtual QList<GroupPtr> allGroups() const;
    virtual QList<ContactPtr> contactsForGroup(int groupId);
    virtual void incomingMessage(const MessagePtr& message);
    virtual UserDataPtr userdata() const;

protected:
    void save_new();
    bool save_owner(QDomElement element);
    bool save_groups(QDomElement element);
    bool save_contacts(QDomElement element);
    bool load_new();
    bool load_owner(const QDomElement& owner);
    bool load_groups(const QDomElement& groups);
    bool load_contacts(const QDomElement& contacts);
    bool load_old();

private:
    UserDataPtr getUserData() { return m_userData; }
    UserDataPtr m_userData;
    QMap<int, ContactPtr> m_contacts;
    QMap<int, GroupPtr> m_groups;
};

} // namespace SIM

#endif // STANDARDCONTACTLIST_H
