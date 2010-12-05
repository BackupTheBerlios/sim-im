#ifndef PRIVACYSNACHANDLER_H
#define PRIVACYSNACHANDLER_H

#include "snac.h"

class PrivacySnacHandler : public SnacHandler
{
public:
    PrivacySnacHandler(ICQClient* client);

    bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();

    static const int SnacPrivacyRightsRequest = 0x02;
};

#endif // PRIVACYSNACHANDLER_H
