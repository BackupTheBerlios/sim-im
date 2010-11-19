#ifndef AUTHORIZATIONSNACHANDLER_H
#define AUTHORIZATIONSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class AuthorizationSnacHandler : public SnacHandler
{
public:
    AuthorizationSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, unsigned short seq);

};

#endif // AUTHORIZATIONSNACHANDLER_H
