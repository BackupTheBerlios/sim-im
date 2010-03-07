
#include "clientuserdataprivate.h"
#include "contacts.h"
#include "client.h"

namespace SIM
{
    ClientUserDataPrivate::ClientUserDataPrivate()
    {
    }

    ClientUserDataPrivate::~ClientUserDataPrivate()
    {
        // why do I have to delete something here which is created somehwere else??
//        for (ClientUserDataPrivate::iterator it = begin(); it != end(); ++it){
//            _ClientUserData &d = *it;
//            free_data(d.client->protocol()->userDataDef(), d.data);
//            delete[] d.data;
//        }
    }
}

// vim: set expandtab:

