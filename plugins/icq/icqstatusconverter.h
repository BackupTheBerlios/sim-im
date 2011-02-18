#ifndef ICQSTATUSCONVERTER_H
#define ICQSTATUSCONVERTER_H

#include "icqstatus.h"
#include "icq_defines.h"

class ICQClient;
class ICQ_EXPORT ICQStatusConverter
{
public:
    ICQStatusConverter(ICQClient* client);

    ICQStatusPtr makeStatus(unsigned int icqStatusId);
private:
    ICQClient* m_client;
};

#endif // ICQSTATUSCONVERTER_H
