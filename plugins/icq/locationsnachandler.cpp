#include "locationsnachandler.h"
#include "icqclient.h"

LocationSnacHandler::LocationSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LOCATION)
{
}

bool LocationSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    return true;
}

void LocationSnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacLocationRightsRequest, 0, QByteArray());
}
