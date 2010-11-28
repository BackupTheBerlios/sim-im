#include "servicesnachandler.h"
#include "bytearraybuilder.h"
#include "bytearrayparser.h"
#include "icqclient.h"
#include "log.h"

using SIM::log;
using SIM::L_ERROR;

ServiceSnacHandler::ServiceSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SERVICE)
{
}

bool ServiceSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacServiceServerReady:
        return sendServices(data);

    case SnacServiceCapabilitiesAck:
        return requestRateInfo();

    case SnacServiceRateInfo:
        parseRateInfo(data);
        return requestRights();
    }

    return true;
}

bool ServiceSnacHandler::sendServices(const QByteArray& data)
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);
    ByteArrayBuilder builder;
    if(client()->isAim()) {
        builder.appendDword(0x00010003L);
        builder.appendDword(0x00130003L);
        builder.appendDword(0x00020001L);
        builder.appendDword(0x00030001L);
        builder.appendDword(0x00040001L);
        builder.appendDword(0x00060001L);
        builder.appendDword(0x00080001L);
        builder.appendDword(0x00090001L);
        builder.appendDword(0x000A0001L);
        builder.appendDword(0x000B0001L);
    } else {
        builder.appendDword(0x00010004L);
        builder.appendDword(0x00130004L);
        builder.appendDword(0x00020001L);
        builder.appendDword(0x00030001L);
        builder.appendDword(0x00150001L);
        builder.appendDword(0x00040001L);
        builder.appendDword(0x00060001L);
        builder.appendDword(0x00090001L);
        builder.appendDword(0x000A0001L);
        builder.appendDword(0x000B0001L);
    }
    socket->snac(ICQ_SNACxFOOD_SERVICE, SnacServiceCapabilities, 0, builder.getArray());
    return true;
}

bool ServiceSnacHandler::requestRateInfo()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);
    socket->snac(getType(), SnacServiceRateInfoRequest, 0, QByteArray());
    return true;
}

bool ServiceSnacHandler::parseRateInfo(const QByteArray& data)
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    m_rateInfoList.clear();
    ByteArrayParser parser(data);
    int classes = parser.readWord();
    for(int i = 0; i < classes; i++)
    {
        int id = parser.readWord();
        int windowSize = parser.readDword();
        int clearLevel = parser.readDword();
        int alertLevel = parser.readDword();
        int limitLevel = parser.readDword();
        int disconnectLevel = parser.readDword();
        int currentLevel = parser.readDword();
        int maxLevel = parser.readDword();
        int lastTime = parser.readDword();
        int currentState = parser.readByte();

        RateInfoPtr info = RateInfoPtr(new RateInfo(id));
        info->setWindowSize(windowSize);
        info->setLevels(clearLevel, alertLevel, limitLevel, disconnectLevel, maxLevel);
        info->setCurrentLevel(currentLevel);
        m_rateInfoList.append(info);
    }

    for(int classnum = 0; classnum < classes; classnum++)
    {
        int id = parser.readWord();
        int entries = parser.readWord();
        RateInfoPtr info = rateInfo(id);
        if(!info)
        {
            log(L_ERROR, "Invalid rate info packet");
            return false;
        }
        for(int entrynum = 0; entrynum < entries; entrynum++)
        {
            int type = parser.readWord();
            int subtype = parser.readWord();
            info->addSnac(type, subtype);
        }
    }

    socket->snac(getType(), SnacServiceRateInfoAck, 0, QByteArray());
    return true;
}

bool ServiceSnacHandler::requestRights()
{

    return true;
}

RateInfoPtr ServiceSnacHandler::rateInfo(int group) const
{
    foreach(const RateInfoPtr& info, m_rateInfoList)
    {
        if(info->group() == group)
            return info;
    }
    return RateInfoPtr();
}
