#include "ssisnachandler.h"
#include "icqclient.h"

SsiSnacHandler::SsiSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SSI)
{
}

bool SsiSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    return true;
}

void SsiSnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacSsiRightsRequest, 0, QByteArray());
}

void SsiSnacHandler::requestContactList()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacSsiContactListRequest, 0, QByteArray());
}

int SsiSnacHandler::maxContacts() const
{
    return m_maxContacts;
}
