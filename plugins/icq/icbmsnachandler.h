#ifndef ICBMSNACHANDLER_H
#define ICBMSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class ICQ_EXPORT IcbmSnacHandler : public SnacHandler
{
public:
    IcbmSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestParametersInfo();
    bool sendNewParametersInfo();

    int minMessageInterval() const;

    static const int SnacIcbmSetParameters = 0x02;
    static const int SnacIcbmParametersInfoRequest = 0x04;
    static const int SnacIcbmParametersInfo = 0x05;

private:
    bool processParametersInfo(const QByteArray& arr);

    int m_channel;
    int m_messageFlags;
    int m_maxSnacSize;
    int m_maxSenderWarnLevel;
    int m_maxReceiverWarnLevel;
    int m_minMessageInterval;
};

#endif // ICBMSNACHANDLER_H
