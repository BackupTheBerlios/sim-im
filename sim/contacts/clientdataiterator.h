
#ifndef SIM_CLIENTDATAITERATOR_H
#define SIM_CLIENTDATAITERATOR_H

#include "simapi.h"
#include "clientuserdata.h"

namespace SIM
{
    class EXPORT ClientDataIterator
    {
    public:
        ClientDataIterator(ClientUserData &data, Client *client=NULL);
        ~ClientDataIterator();
        clientData *operator ++();
        Client *client();
        void reset();
    protected:
        class ClientDataIteratorPrivate *p;

        COPY_RESTRICTED(ClientDataIterator)
    };

}

#endif

// vim: set expandtab:

