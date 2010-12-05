#ifndef ICBMSNACHANDLER_H
#define ICBMSNACHANDLER_H

#include "snac.h"

class IcbmSnacHandler : public SnacHandler
{
public:
    IcbmSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestParametersInfo();

    static const int SnacIcbmParametersInfoRequest = 0x04;
};

#endif // ICBMSNACHANDLER_H
