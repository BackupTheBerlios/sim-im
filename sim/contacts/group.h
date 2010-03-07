
#ifndef SIM_GROUP_H
#define SIM_GROUP_H

#include "simapi.h"
#include "userdata.h"
#include "clientuserdata.h"
#include "propertyhub.h"
#include "misc.h"

namespace SIM
{

    struct GroupData
    {
        Data        Name;       // Display name (UTF-8)
    };
    class EXPORT Group
    {
    public:
        Group(unsigned long id = 0, Buffer *cfg = NULL);
        virtual ~Group();
        unsigned long id() { return m_id; }

        QString getName();
        void setName(const QString& name);

        PropertyHubPtr getUserData(const QString& id, bool bCreate = false);
        ClientUserData clientData;
        PropertyHubPtr userdata() const { return m_userData->root(); }
        UserDataPtr getUserData() { return m_userData; }

    protected:
        unsigned long m_id;
        GroupData data; friend class ContactList;
        friend class ContactListPrivate;

    private:
        QString m_name;
        UserDataPtr m_userData; // FIXME this mess
    };
}

#endif

// vim: set expandtab:

