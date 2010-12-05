#ifndef SERVICESNACHANDLER_H
#define SERVICESNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"
#include "rateinfo.h"
#include "bytearrayparser.h"

class ICQClient;
class ICQ_EXPORT ServiceSnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    ServiceSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    RateInfoPtr rateInfo(int group) const;

    static const int SnacServiceServerReady = 0x0003;
    static const int SnacServiceRateInfoRequest = 0x0006;
    static const int SnacServiceRateInfo = 0x0007;
    static const int SnacServiceRateInfoAck = 0x0008;
    static const int SnacServiceSelfInfoRequest = 0x000e;
    static const int SnacServiceCapabilities = 0x0017;
    static const int SnacServiceCapabilitiesAck = 0x0018;

signals:
    void initiateLoginStep2();

private:
    bool sendServices(const QByteArray& data);
    bool requestRateInfo();
    bool parseRateInfo(const QByteArray& data);
    RateInfoPtr readNextRateInfoClass(ByteArrayParser& parser);
    int readNextRateInfoGroup(ByteArrayParser& parser);
    bool requestSelfInfo();

    QList<RateInfoPtr> m_rateInfoList;
};

#endif // SERVICESNACHANDLER_H
