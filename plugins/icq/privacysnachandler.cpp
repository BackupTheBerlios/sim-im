#include "privacysnachandler.h"
#include "icqclient.h"

PrivacySnacHandler::PrivacySnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_BOS)
{
}

bool PrivacySnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    return true;
}

void PrivacySnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacPrivacyRightsRequest, 0, QByteArray());
}
