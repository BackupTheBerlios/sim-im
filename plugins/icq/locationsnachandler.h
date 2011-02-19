#ifndef LOCATIONSNACHANDLER_H
#define LOCATIONSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class ICQClient;

class ICQ_EXPORT LocationSnacHandler : public SnacHandler
{
public:
    LocationSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();

    int maxCapabilities() const;

    static const int SnacLocationRightsRequest = 0x0002;
    static const int SnacLocationRightsInfo = 0x0003;
    static const int SnacSetUserInfo = 0x0004;

    static const int TlvMaxProfileLength = 0x0001;
    static const int TlvMaxCapabilities = 0x0002;
    static const int TlvClientCapabilities = 0x0005;

private:
    bool parseRightsInfo(const QByteArray& arr);
    bool sendUserInfo();
    int m_maxCapabilities;
};

#endif // LOCATIONSNACHANDLER_H
