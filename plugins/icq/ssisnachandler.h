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

    static const int SnacSsiRightsRequest = 0x02;
    static const int SnacSsiContactListRequest = 0x05;

private:
    int m_maxContacts;
};

#endif // SSISNACHANDLER_H
