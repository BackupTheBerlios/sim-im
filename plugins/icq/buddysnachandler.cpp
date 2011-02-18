#include "buddysnachandler.h"
#include "icqclient.h"
#include "log.h"

using SIM::log;
using SIM::L_WARN;

BuddySnacHandler::BuddySnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_BUDDY)
{
}

bool BuddySnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacBuddyUserOnline:
        return processUserOnline(data);
        break;
    default:
        log(L_WARN, "Unknown buddy snac subtype: %04x", subtype);
        break;
    }

    return true;
}

void BuddySnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacBuddyRightsRequest, 0, QByteArray());
}

bool BuddySnacHandler::processUserOnline(const QByteArray& data)
{
    ICQContactList* contactList = m_client->contactList();
    ICQStatusConverter* converter = m_client->statusConverter();
    Q_ASSERT(contactList);

    ByteArrayParser parser(data);
    int screenLength = parser.readByte();
    QByteArray screenRaw = parser.readBytes(screenLength);
    QString screen(screenRaw); // TODO test it

    parser.readWord(); // Read Warning level and discard it
    int tlvCount = parser.readWord();

    TlvList list = TlvList::fromByteArray(parser.readAll());
    Tlv tlv = list.firstTlv(TlvOnlineStatus);

    ICQStatusPtr newStatus = converter->makeStatus(tlv.toUint32());

    ICQContactPtr contact = contactList->contactByScreen(screen);
    if(contact)
    {
        contact->setIcqStatus(newStatus);
    }

    return true;
}
