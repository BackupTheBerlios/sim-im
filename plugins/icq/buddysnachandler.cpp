#include "buddysnachandler.h"
#include "icqclient.h"

BuddySnacHandler::BuddySnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_BUDDY)
{
}

bool BuddySnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    return true;
}

void BuddySnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacBuddyRightsRequest, 0, QByteArray());
}
