#include "locationsnachandler.h"
#include "icqclient.h"
#include "log.h"

using SIM::log;
using SIM::L_WARN;

LocationSnacHandler::LocationSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LOCATION),
    m_maxCapabilities(0)
{
}

bool LocationSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacLocationRightsInfo:
        {
            if(parseRightsInfo(data))
                return sendUserInfo();
            return false;
        }
    default:
        log(L_WARN, "Unhandled location snac, subtype: %04x", subtype);
        return false;
    }

    return true;
}

void LocationSnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacLocationRightsRequest, 0, QByteArray());
}

int LocationSnacHandler::maxCapabilities() const
{
    return m_maxCapabilities;
}

bool LocationSnacHandler::parseRightsInfo(const QByteArray& arr)
{
    TlvList list = TlvList::fromByteArray(arr);
    Tlv tlvMaxCaps = list.firstTlv(TlvMaxCapabilities);
    if(tlvMaxCaps.isValid())
        m_maxCapabilities = tlvMaxCaps.toUint16();
    return true;
}

bool LocationSnacHandler::sendUserInfo()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    ClientCapabilitiesRegistry* registry = m_client->clientCapabilitiesRegistry();
    Q_ASSERT(registry);

    QByteArray caps;
    caps.append(registry->capabilityByName("short_caps")->guid());
    caps.append(registry->capabilityByName("aim_support")->guid());
    caps.append(registry->capabilityByName("avatar")->guid());
    caps.append(registry->capabilityByName("send_file")->guid());
    caps.append(registry->capabilityByName("direct")->guid());
    caps.append(registry->capabilityByName("relay")->guid());
    caps.append(registry->capabilityByName("utf")->guid());

    TlvList list;
    list.append(Tlv(TlvClientCapabilities, caps));

    socket->snac(getType(), SnacSetUserInfo, 0, list.toByteArray());

    return true;
}
