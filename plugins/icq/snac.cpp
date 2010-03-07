
#include "snac.h"
#include "icqclient.h"

SnacHandler::SnacHandler(ICQClient* client, unsigned short snac) : m_snac(snac)
{
	m_client = client;
}

SnacHandler::~SnacHandler()
{
}

void SnacHandler::snac(const unsigned short subtype, bool bMsgID, bool bType)
{
	m_client->snac(m_snac, subtype, bMsgID, bType);
}

ICQClient* SnacHandler::client()
{
	return m_client;
}
