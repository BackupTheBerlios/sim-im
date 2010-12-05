#ifndef SSISNACHANDLER_H
#define SSISNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class ICQ_EXPORT SsiSnacHandler : public SnacHandler
{
public:
    SsiSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();
    void requestContactList();

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

private:
    bool parseRightsInfo(const QByteArray& data);

    int m_maxContacts;
    int m_maxGroups;
    int m_maxVisibleContacts;
    int m_maxInvisibleContacts;
    int m_maxBitmasks;
    int m_maxPresenceInfoFields;
    int m_maxIgnore;
};

#endif // SSISNACHANDLER_H
