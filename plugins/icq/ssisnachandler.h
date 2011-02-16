#ifndef SSISNACHANDLER_H
#define SSISNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"
#include "bytearrayparser.h"
#include "tlvlist.h"

class ICQ_EXPORT SsiSnacHandler : public SnacHandler
{
public:
    SsiSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();
    void requestContactList();
    void activate();

    int maxContacts() const;
    int maxGroups() const;
    int maxVisibleContacts() const;
    int maxInvisibleContacts() const;
    int maxBitmasks() const;
    int maxPresenceInfoFields() const;
    int maxIgnore() const;

    static const int TlvIdMaximumItems = 0x0004;

    static const int SnacSsiRightsRequest = 0x02;
    static const int SnacSsiRightsInfo = 0x03;
    static const int SnacSsiContactListRequest = 0x05;
    static const int SnacSsiContactList = 0x06;
    static const int SnacSsiActivate = 0x07;

    static const int EntryContact = 0x0000;
    static const int EntryGroup = 0x0001;
    static const int EntryVisibleList = 0x0002;
    static const int EntryInvisibleList = 0x0003;
    static const int EntryInvisibleState = 0x0004;
    static const int EntryPresenceInfo = 0x0005;
    static const int EntryShortcutBar = 0x0009;
    static const int EntryIgnoreList = 0x000E;
    static const int EntryLastUpdate = 0x000F;
    static const int EntryNonIm = 0x0010;
    static const int EntryUnknown = 0x0011;
    static const int EntryImportTime = 0x0013;
    static const int EntryBuddyChecksum = 0x0014;
    static const int EntryUnknown2 = 0x0019;
    static const int EntryUnknown3 = 0x001A;
    static const int EntryAwaitingAuth = 0x001B;

    static const int TlvWaitAuth = 0x0066;
    static const int TlvUnknown2 = 0x006d;
    static const int TlvUnknown3 = 0x006e;
    static const int TlvSubitems = 0x00C8;
    static const int TlvShortcutBar = 0x00cd;
    static const int TlvTime = 0x00D4;
    static const int TlvBuddyHash = 0x00D5;
    static const int TlvAlias = 0x0131;
    static const int TlvCellular = 0x013A;
    static const int TlvUnknown4 = 0x015c;
    static const int TlvUnknown5 = 0x015d;

private:
    bool parseRightsInfo(const QByteArray& data);
    bool parseContactList(const QByteArray& data);
    bool parseContactListEntry(ByteArrayParser& parser);
    bool parseEntryContact(int contactId, const QString& contactName, int groupId, const TlvList& list);
    bool parseEntryGroup(int groupId, const QString& groupName, const TlvList& list);

    int m_maxContacts;
    int m_maxGroups;
    int m_maxVisibleContacts;
    int m_maxInvisibleContacts;
    int m_maxBitmasks;
    int m_maxPresenceInfoFields;
    int m_maxIgnore;
};

#endif // SSISNACHANDLER_H
