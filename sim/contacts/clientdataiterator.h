
#ifndef SIM_CLIENTDATAITERATOR_H
#define SIM_CLIENTDATAITERATOR_H

#include <QSharedPointer>
#include "simapi.h"
#include "clientuserdata.h"
#include "imcontact.h"

namespace SIM
{
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

