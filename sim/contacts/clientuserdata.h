
#ifndef SIM_CLIENTUSERDATA_H
#define SIM_CLIENTUSERDATA_H

#include <QByteArray>
#include <QString>

#include "simapi.h"
#include "buffer.h"

namespace SIM
{
    class Client;
    class ClientUserDataPrivate;
    struct clientData;
    class EXPORT ClientUserData
    {
    public:
        ClientUserData();
        ~ClientUserData();
        QByteArray save() const;
        void load(Client *client, Buffer *cfg);
        void *getData(Client *client);
        bool have(void*);
        void *createData(Client *client);
        void freeData(void*);
        void freeClientData(Client *client);
        void sort();
        void join(ClientUserData &data);
        void join(clientData *cData, ClientUserData &data);
        unsigned size();
        Client *activeClient(void *&data, Client *client);
        QString property(const char *name);

    protected:
        class ClientUserDataPrivate *p;
        friend class ClientDataIterator;

        COPY_RESTRICTED(ClientUserData)
    };
}

#endif

// vim: set expandtab:

