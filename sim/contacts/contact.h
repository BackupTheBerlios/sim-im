
#ifndef SIM_CONTACT_H
#define SIM_CONTACT_H

#include <QSharedPointer>
#include "userdata.h"
#include "propertyhub.h"
#include "clientuserdata.h"
#include "misc.h"
#include "imcontact.h"

namespace SIM
{
    class Contact;
    typedef QSharedPointer<Contact> ContactPtr;

    class EXPORT Contact
    {
    public:
        enum Flag
        {
            flIgnore = 0,
            flTemporary,
            flMaxFlag
        };

        Contact(int id);
        virtual ~Contact();

        int id() const { return m_id; }

        int groupId() const;
        void setGroupId(int g);

        bool flag(Flag fl) const;
        void setFlag(Flag fl, bool value);

        QString name() const;
        void setName(const QString& s);

        time_t lastActive() const;
        void setLastActive(time_t la);

        QString notes() const;
        void setNotes(const QString& n);

        void addClientContact(const IMContactPtr& contact);
        IMContactPtr clientContact(const QString& clientId) const;
        QStringList clientContactNames() const;

        bool hasUnreadMessages();

        void join(const ContactPtr& contact);

        QString toolTipText();

        UserDataPtr getUserData() { return m_userData; }

        bool serialize(QDomElement& element);
        bool deserialize(const QDomElement& element);

    protected:
        bool serializeMainInfo(QDomElement& element);
        bool deserializeMainInfo(const QDomElement& element);

    private:
        UserDataPtr m_userData;
        int m_id;
        int m_groupId;
        QBitArray m_flags;
        QString m_name;
        time_t m_lastActive;
        QString m_notes;
        QList<IMContactPtr> m_imContacts;
    };
}

#endif

// vim: set expandtab:

