#include "icbmsnachandler.h"
#include "icqclient.h"

IcbmSnacHandler::IcbmSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_MESSAGE)
{
}

bool IcbmSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    return true;
}

void IcbmSnacHandler::requestParametersInfo()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacIcbmParametersInfoRequest, 0, QByteArray());
}
