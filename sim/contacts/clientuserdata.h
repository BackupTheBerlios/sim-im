
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

        void addData(IMContact* data);
        IMContact* createData(Client *client);
        IMContact* getData(Client *client);
        IMContact* getData(const QString& clientName);
        QStringList clientNames();
        bool have(IMContact*);
        void sort();
        void join(ClientUserData &data);
        void join(IMContact *cData, ClientUserData &data);
        unsigned size();
        Client *activeClient(void *&data, Client *client);
        void freeData(SIM::IMContact*);
        void freeClientData(Client *client);

    protected:
        class ClientUserDataPrivate *p;
        friend class ClientDataIterator;

        COPY_RESTRICTED(ClientUserData)
    };

    class ClientDataIteratorPrivate;

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
        ClientDataIteratorPrivate* p;

    };
}

#endif

// vim: set expandtab:

