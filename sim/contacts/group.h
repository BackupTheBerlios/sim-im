
#ifndef SIM_GROUP_H
#define SIM_GROUP_H

#include "simapi.h"
#include "userdata.h"
#include "clientuserdata.h"
#include "propertyhub.h"
#include "misc.h"

namespace SIM
{
    class EXPORT Group
    {
    public:
        Group(unsigned long id = 0, Buffer *cfg = NULL);
        virtual ~Group();
        unsigned long id() { return m_id; }

        QString getName();
        void setName(const QString& name);

        PropertyHubPtr getUserData(const QString& id, bool bCreate = false);
        PropertyHubPtr userdata() const { return m_userData->root(); }
        UserDataPtr getUserData() { return m_userData; }

        ClientDataIterator clientDataIterator(Client* client = NULL);
        QByteArray saveUserData() const;
        void loadUserData(Client *client, Buffer *cfg);
        IMContact* createData(Client* client);
        IMContact* getData(Client *client);
        IMContact* getData(const QString& clientName);
        QStringList clientNames();
        bool have(IMContact*);
        void sort();
        void join(Group* c);
        void join(SIM::IMContact *cData, Group* c);
        unsigned size();
        Client *activeClient(void *&data, Client *client);
        void freeData(SIM::IMContact*);
        void freeClientData(Client *client);

    protected:
        unsigned long m_id;
        ClientUserData m_clientData;
        friend class ContactList;
        friend class ContactListPrivate;

    private:
        QString m_name;
        UserDataPtr m_userData; // FIXME this mess
    };
}

#endif

// vim: set expandtab:

