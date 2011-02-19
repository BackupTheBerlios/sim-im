#include "buddysnachandler.h"
#include "icqclient.h"
#include "log.h"
#include "events/eventhub.h"
#include "events/contactevent.h"

using SIM::log;
using SIM::L_DEBUG;
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

    ICQContactPtr contact = contactList->contactByScreen(screen);

    parser.readWord(); // Read Warning level and discard it
    int tlvCount = parser.readWord();
    Q_UNUSED(tlvCount);

    TlvList list = TlvList::fromByteArray(parser.readAll());

    Tlv statusTlv = list.firstTlv(TlvOnlineStatus);
    if(statusTlv.isValid())
    {
        log(L_DEBUG, "Buddy online[%s] status: %08x", qPrintable(screen), statusTlv.toUint32());
        ICQStatusPtr newStatus = converter->makeStatus(statusTlv.toUint32());
        if(contact)
        {
            contact->setIcqStatus(newStatus);
            log(L_DEBUG, "metaContactId: %08x", contact->metaContactId());
            SIM::getEventHub()->triggerEvent("contact_change_status", SIM::ContactEventData::create(contact->metaContactId()));
        }
    }

    return true;
}
