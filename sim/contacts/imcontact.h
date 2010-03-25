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

        //Data    Sign;       // Protocol ID, must be ICQ_SIGN, JABBER_SIGN etc
        virtual unsigned long getSign() = 0;
        Data    LastSend;

        virtual QByteArray serialize() = 0;
        virtual void deserialize(Buffer* cfg) = 0;

        // We will uncomment it when everything will be ready
//        virtual void serialize(QDomElement& element) = 0;
//        virtual void deserialize(QDomElement& element) = 0;
    };
}

#endif // IMCONTACT_H
