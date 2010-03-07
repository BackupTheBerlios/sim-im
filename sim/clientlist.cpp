
#include "contacts/client.h"
#include "clientlist.h"
#include "contacts.h"

namespace SIM
{
    ClientList::ClientList() : QObject(),
        std::vector<ClientPtr>()
    {
    }

    ClientList::~ClientList()
    {
        emit ignoreEvents(true);
        for (ClientList::iterator it = begin(); it != end(); ++it)
            (*it).clear();
        emit ignoreEvents(false);
    }

    void ClientList::addToContacts()
    {
        for (ClientList::iterator it = begin(); it != end(); ++it)
            getContacts()->addClient(it->data());
        clear();
    }
}

// vim: set expandtab:

