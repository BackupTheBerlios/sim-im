#include "icqstatusconverter.h"
#include "icqclient.h"

ICQStatusConverter::ICQStatusConverter(ICQClient* client) : m_client(client)
{
}

ICQStatusPtr ICQStatusConverter::makeStatus(unsigned int icqStatusId)
{
    ICQStatusPtr status;
    int statusId = icqStatusId & 0xffff;
    if(statusId == ICQ_STATUS_ONLINE)
    {
        status = m_client->getDefaultStatus("online");
    }
    else if(statusId == ICQ_STATUS_AWAY)
    {
        status = m_client->getDefaultStatus("away");
    }
    if(status)
        return status->clone().dynamicCast<ICQStatus>();
    return ICQStatusPtr();
}
