
#ifndef SIM_CLIENTUSERDATA_H
#define SIM_CLIENTUSERDATA_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>

#include "simapi.h"
#include "buffer.h"

namespace SIM
{
    class Client;
    class ClientUserDataPrivate;
    class IMContact;
    class EXPORT ClientUserData
    {
    public:
        ClientUserData();
        ~ClientUserData();
        QByteArray save() const;
        void load(Client *client, Buffer *cfg);

        void *createData(Client *client);
        void *getData(Client *client);
        bool have(void*);
        void sort();
        void join(ClientUserData &data);
        void join(IMContact *cData, ClientUserData &data);
        unsigned size();
        Client *activeClient(void *&data, Client *client);
        QString property(const char *name);
        void freeData(void*);
        void freeClientData(Client *client);

    protected:
        class ClientUserDataPrivate *p;
        friend class ClientDataIterator;

        COPY_RESTRICTED(ClientUserData)
    };

    class ClientDataIteratorPrivate;
    typedef QSharedPointer<ClientDataIteratorPrivate> ClientDataIteratorPrivatePtr;

    class EXPORT ClientDataIterator
    {
    public:
        ClientDataIterator();
        ClientDataIterator(ClientUserData &data, Client *client=NULL);
        ~ClientDataIterator();
        IMContact *operator ++();
        Client *client();
        void reset();
    protected:
        ClientDataIteratorPrivatePtr p;

        //COPY_RESTRICTED(ClientDataIterator)
    };
}

#endif

// vim: set expandtab:

