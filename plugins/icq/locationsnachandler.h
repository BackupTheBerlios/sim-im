#ifndef LOCATIONSNACHANDLER_H
#define LOCATIONSNACHANDLER_H

#include "snac.h"

class ICQClient;

class LocationSnacHandler : public SnacHandler
{
public:
    LocationSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();

    static const int SnacLocationRightsRequest = 0x02;
};

#endif // LOCATIONSNACHANDLER_H
