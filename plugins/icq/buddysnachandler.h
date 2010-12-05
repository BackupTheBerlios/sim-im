#ifndef BUDDYSNACHANDLER_H
#define BUDDYSNACHANDLER_H

#include "snac.h"

class BuddySnacHandler : public SnacHandler
{
public:
    BuddySnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();

    static const int SnacBuddyRightsRequest = 0x02;
};

#endif // BUDDYSNACHANDLER_H
