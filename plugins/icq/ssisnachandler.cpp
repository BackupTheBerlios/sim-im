#include "ssisnachandler.h"
#include "icqclient.h"
#include "bytearrayparser.h"
#include "tlvlist.h"

SsiSnacHandler::SsiSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SSI),
    m_maxContacts(0),
    m_maxGroups(0),
    m_maxVisibleContacts(0),
    m_maxInvisibleContacts(0),
    m_maxBitmasks(0),
    m_maxPresenceInfoFields(0),
    m_maxIgnore(0)
{
}

bool SsiSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacSsiRightsInfo:
        return parseRightsInfo(data);
    }

    return false;
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

bool SsiSnacHandler::parseRightsInfo(const QByteArray& data)
{
    TlvList tlvs = TlvList::fromByteArray(data);
    Tlv maxItemsTlv = tlvs.firstTlv(TlvIdMaximumItems);
    if(!maxItemsTlv.isValid())
        return false;

    ByteArrayParser parser(maxItemsTlv.data());
    m_maxContacts = parser.readWord();
    m_maxGroups = parser.readWord();
    m_maxVisibleContacts = parser.readWord();
    m_maxInvisibleContacts = parser.readWord();
    m_maxBitmasks = parser.readWord();
    m_maxPresenceInfoFields = parser.readWord();
    parser.readBytes(8 * 2); // Skip 8 words
    m_maxIgnore = parser.readWord();

    // The rest of data is unknown fields
    return true;
}

int SsiSnacHandler::maxContacts() const
{
    return m_maxContacts;
}

int SsiSnacHandler::maxGroups() const
{
    return m_maxGroups;
}

int SsiSnacHandler::maxVisibleContacts() const
{
    return m_maxVisibleContacts;
}

int SsiSnacHandler::maxInvisibleContacts() const
{
    return m_maxInvisibleContacts;
}

int SsiSnacHandler::maxBitmasks() const
{
    return m_maxBitmasks;
}

int SsiSnacHandler::maxPresenceInfoFields() const
{
    return m_maxPresenceInfoFields;
}

int SsiSnacHandler::maxIgnore() const
{
    return m_maxIgnore;
}
