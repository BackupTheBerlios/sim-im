#include "ssisnachandler.h"
#include "icqclient.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"
#include "tlvlist.h"
#include "log.h"
#include "icqcontact.h"
#include "contacts/contactlist.h"
#include "contacts/contact.h"
#include "events/eventhub.h"

using SIM::log;
using SIM::L_DEBUG;

SsiSnacHandler::SsiSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LISTS),
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
    case SnacSsiContactList:
        return parseContactList(data);
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

    ByteArrayBuilder builder;
    builder.appendDword(0); // Last change time
    builder.appendWord(0); // Object count

    socket->snac(getType(), SnacSsiContactListRequest, 0, builder.getArray());
}

void SsiSnacHandler::activate()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacSsiActivate, 0, QByteArray());
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

bool SsiSnacHandler::parseContactList(const QByteArray& data)
{
    ByteArrayParser parser(data);
    int version = parser.readByte();
    int objects = parser.readWord();
    for(int objnum = 0; objnum < objects; objnum++)
    {
        if(!parseContactListEntry(parser))
            return false;
    }
    SIM::getEventHub()->triggerEvent("contact_list_updated");
    activate();
    return true;
}

bool SsiSnacHandler::parseContactListEntry(ByteArrayParser& parser)
{
    int nameLength = parser.readWord();
    QByteArray entryName = parser.readBytes(nameLength);
    if(entryName.length() < nameLength)
        return false;
    int groupId = parser.readWord();
    int entryId = parser.readWord();
    int entryType = parser.readWord();
    int tlvLen = parser.readWord();
    //log(L_DEBUG, "tlvLen: %d", tlvLen);
    TlvList list = TlvList::fromByteArray(parser.readBytes(tlvLen));

    switch(entryType)
    {
    case EntryContact:
        return parseEntryContact(entryId, QString::fromUtf8(entryName.data(), entryName.length()), groupId, list);
    case EntryGroup:
        return parseEntryGroup(groupId, QString::fromUtf8(entryName.data(), entryName.length()), list);
    default:
        log(L_DEBUG, "Unknown item type in SSI list: %04x", entryType);
    }
    return true;
}

bool SsiSnacHandler::parseEntryContact(int contactId, const QString& contactName, int groupId, const TlvList& list)
{
    ICQContactList* contactList = client()->contactList();
    Q_ASSERT(contactList);
    bool bChanged = false;
    QString alias;
    if(list.contains(TlvAlias))
        alias = QString::fromUtf8(list.firstTlv(TlvAlias).data().data());

    log(L_DEBUG, "User %s [%s] id %u - group %u", qPrintable(contactName), qPrintable(alias), contactId, groupId);

    ICQContactPtr contact = contactList->contact(contactId);
    if(!contact)
    {
        contact = client()->createIMContact().dynamicCast<ICQContact>(); // FIXME
        bool ok;
        int uin = contactName.toUInt(&ok);
        if(!ok)
            return false;
        contact->setUin(uin);
        contact->setAlias(alias);
        contact->setIcqID(contactId);
        contact->setGrpID(groupId);
        bChanged = true;
        SIM::ContactPtr metacontact = SIM::getContactList()->createContact();
        metacontact->addClientContact(contact);
        metacontact->setName(alias);
        contact->setMetaContact(metacontact.data());
        SIM::getContactList()->addContact(metacontact);
    }
    if(list.contains(TlvUnknown2))
        contact->setUnknown(2, list.firstTlv(TlvUnknown2).data());
    if(list.contains(TlvUnknown4))
        contact->setUnknown(4, list.firstTlv(TlvUnknown4).data());
    if(list.contains(TlvUnknown5))
        contact->setUnknown(5, list.firstTlv(TlvUnknown5).data());

    if(list.contains(TlvWaitAuth))
    {
        contact->setWaitAuth(true);
        bChanged = true;
    }
    else
    {
        if(contact->getWaitAuth())
        {
            contact->setWaitAuth(false);
            bChanged = true;
        }
    }

    if(list.contains(TlvCellular)) {
        contact->setCellular(QString::fromUtf8(list.firstTlv(TlvCellular).data()));
    } else {
        contact->setCellular("");
    }
    if(bChanged) {
        // provoke update signal
    }
    contactList->addContact(contact);
//    if ((data->getInfoFetchTime() == 0) && data->getUin())
//        addFullInfoRequest(data->getUin());
    return true;
}

bool SsiSnacHandler::parseEntryGroup(int groupId, const QString& groupName, const TlvList& list)
{
    ICQContactList* contactList = client()->contactList();
    Q_ASSERT(contactList);

    ICQGroupPtr group = contactList->group(groupId);
    if(!group)
    {
        group = client()->createIMGroup().dynamicCast<ICQGroup>(); // FIXME
        if(!group)
            return false;
        group->setIcqId(groupId);
        group->setName(groupName);
        contactList->addGroup(group);

        Tlv membersOfTheGroup = list.firstTlv(TlvSubitems);
        ByteArrayParser parser(membersOfTheGroup.data());
        while(!parser.atEnd())
        {
            int contactId = parser.readWord();
            group->addContactId(contactId);
        }
    }
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
