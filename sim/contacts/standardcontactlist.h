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
    virtual QList<int> contactIds() const;

    virtual bool addGroup(const GroupPtr& group);
    virtual GroupPtr group(int id) const;
    virtual void removeGroup(int id);
    virtual GroupPtr createGroup(int id);

    virtual ContactPtr ownerContact();

    virtual QList<GroupPtr> allGroups() const;
    virtual QList<ContactPtr> contactsForGroup(int groupId);
    virtual void incomingMessage(const MessagePtr& message);
    virtual UserDataPtr userdata() const;

protected:
    bool save_owner(QDomElement element);
    bool save_groups(QDomElement element);
    bool save_contacts(QDomElement element);
    bool load_new();
    bool load_owner(const QDomElement& owner);
    bool load_groups(const QDomElement& groups);
    bool load_contacts(const QDomElement& contacts);
    bool load_old();

    struct ParserState
    {
        enum SectionType
        {
            Contact,
            Group,
            Client
        };

        int contactId;
        int groupId;
        QString dataname;
        SectionType nextSection;
        QString data;
    };

    bool load_old_dispatch(ParserState& state);
    void resetState(ParserState& state);
    bool deserializeLines(const UserDataPtr& ud, const QString& dataname, const QString& data);

private:
    UserDataPtr m_userData;
    ContactPtr m_owner;
    QMap<int, ContactPtr> m_contacts;
    QMap<int, GroupPtr> m_groups;
};

} // namespace SIM

#endif // STANDARDCONTACTLIST_H
