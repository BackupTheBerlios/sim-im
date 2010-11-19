#include "authorizationsnachandler.h"
#include "icqclient.h"

AuthorizationSnacHandler::AuthorizationSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LOGIN)
{
}

bool AuthorizationSnacHandler::process(unsigned short subtype, const QByteArray& data, unsigned short seq)
{
    return false;
}
