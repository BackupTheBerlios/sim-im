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
    static const int SnacBuddyUserOnline = 0x0b;
    static const int SnacBuddyUserOffline = 0x0c;

    static const int TlvUserClass = 0x0001;
    static const int TlvOnlineSince = 0x0003;
    static const int TlvOnlineStatus = 0x0006;
    static const int TlvUserIp = 0x000a;

private:
    bool processUserOnline(const QByteArray& data);
};

#endif // BUDDYSNACHANDLER_H
