
#ifndef SIM_CLIENTUSERDATAPRIVATE_H
#define SIM_CLIENTUSERDATAPRIVATE_H

#include <vector>
#include "contacts.h"

namespace SIM
{
    class ClientUserDataPrivate : public std::vector<_ClientUserData>
    {
    public:
        ClientUserDataPrivate();
        ~ClientUserDataPrivate();
    };
}

#endif

// vim: set expandtab:

