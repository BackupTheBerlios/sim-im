#ifndef IMCONTACT_H
#define IMCONTACT_H

#include "simapi.h"
#include "cfg.h"

namespace SIM
{
    class EXPORT IMContact
    {
    public:
        IMContact();

        Data    Sign;       // Protocol ID, must be ICQ_SIGN, JABBER_SIGN etc
        Data    LastSend;
    };
}

#endif // IMCONTACT_H
