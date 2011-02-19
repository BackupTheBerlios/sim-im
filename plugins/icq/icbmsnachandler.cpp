#include "icbmsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"
#include "log.h"

using SIM::log;
using SIM::L_WARN;

IcbmSnacHandler::IcbmSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_MESSAGE)
{
}

bool IcbmSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacIcbmParametersInfo:
        {
            if(!processParametersInfo(data))
                return false;
            m_channel = 0;
            m_messageFlags = 0x0b;
            m_maxSnacSize = 0x1f40;
            m_maxSenderWarnLevel = 0x30e7;
            m_maxReceiverWarnLevel = 0x30e7;
            return sendNewParametersInfo();
        }
        break;
    default:
        log(L_WARN, "Unhandled icbm snac, subtype: %04x", subtype);
        return false;
    }

    return true;
}

void IcbmSnacHandler::requestParametersInfo()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacIcbmParametersInfoRequest, 0, QByteArray());
}

int IcbmSnacHandler::minMessageInterval() const
{
    return m_minMessageInterval;
}

bool IcbmSnacHandler::processParametersInfo(const QByteArray& arr)
{
    ByteArrayParser parser(arr);
    m_channel = parser.readWord();
    m_messageFlags = parser.readDword();
    m_maxSnacSize = parser.readWord();
    m_maxSenderWarnLevel = parser.readWord();
    m_maxReceiverWarnLevel = parser.readWord();
    m_minMessageInterval = parser.readDword();

    return true;
}

bool IcbmSnacHandler::sendNewParametersInfo()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    ByteArrayBuilder builder;

    builder.appendWord(m_channel);
    builder.appendDword(m_messageFlags);
    builder.appendWord(m_maxSnacSize);
    builder.appendWord(m_maxSenderWarnLevel);
    builder.appendWord(m_maxReceiverWarnLevel);
    builder.appendByte(m_minMessageInterval);

    socket->snac(getType(), SnacIcbmSetParameters, 0, builder.getArray());
    return true;
}
