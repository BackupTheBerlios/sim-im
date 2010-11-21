
#include "snac.h"
#include "icqclient.h"

SnacHandler::SnacHandler(ICQClient* client, unsigned short snac) : m_snac(snac),
    m_client(client)
{
}

SnacHandler::~SnacHandler()
{
}

ICQClient* SnacHandler::client() const
{
    return m_client;
}
