#include "clientcapabilitiesregistry.h"

ClientCapabilitiesRegistry::ClientCapabilitiesRegistry()
{
    addCapability(QByteArray("\x09\x46\x00\x00\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x0000, "short_caps", "");
    addCapability(QByteArray("\x09\x46\x13\x41\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1341, "voice", "");
    addCapability(QByteArray("\x09\x46\x13\x43\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1343, "send_file", "");
    addCapability(QByteArray("\x09\x46\x13\x44\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1344, "direct", "");
    addCapability(QByteArray("\x09\x46\x13\x45\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1345, "im_image", "");
    addCapability(QByteArray("\x09\x46\x13\x46\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1346, "buddy_con", "");
    addCapability(QByteArray("\x09\x46\x13\x47\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1347, "stocks", "");
    addCapability(QByteArray("\x09\x46\x13\x48\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1348, "get_file", "");
    addCapability(QByteArray("\x09\x46\x13\x49\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x1349, "relay", "");
    addCapability(QByteArray("\x09\x46\x13\x4a\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x134a, "games", "");
    addCapability(QByteArray("\x09\x46\x13\x4b\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x134b, "buddylist", "");
    addCapability(QByteArray("\x09\x46\x13\x4c\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x134c, "avatar", "");
    addCapability(QByteArray("\x09\x46\x13\x4d\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x134d, "aim_support", "");
    addCapability(QByteArray("\x09\x46\x13\x4e\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00", 16), 0x134e, "utf", "");
    addCapability(QByteArray("\x97\xb1\x27\x51\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x92", 16), 0x0000, "rtf", "");
    addCapability(QByteArray("\x56\x3f\xc8\x09\x0b\x6f\x41\xbd\x9f\x79\x42\x26\x09\xdf\xa2\xf3", 16), 0x0000, "typing", "");
    addCapability(QByteArray("SIM client  \x00\x00\x00\x00", 16), 0x0000, "sim", "");
    // TODO moar

    m_nullCapability = new ClientCapability(QByteArray("NULL"), 0, "null", "");
}

ClientCapabilitiesRegistry::~ClientCapabilitiesRegistry()
{
    qDeleteAll(m_capabilites);
    delete m_nullCapability;
}

ClientCapability* ClientCapabilitiesRegistry::capabilityByGuid(const QByteArray& guid)
{
    foreach(ClientCapability* cap, m_capabilites)
    {
        if(cap->guid() == guid)
            return cap;
    }
    return 0;
}

ClientCapability* ClientCapabilitiesRegistry::capabilityByName(const QString& name)
{
    foreach(ClientCapability* cap, m_capabilites)
    {
        if(cap->name() == name)
            return cap;
    }
    return 0;
}

ClientCapability* ClientCapabilitiesRegistry::capabilityByShortId(int shortId)
{
    foreach(ClientCapability* cap, m_capabilites)
    {
        if(cap->shortId() == shortId)
            return cap;
    }
    return 0;
}

void ClientCapabilitiesRegistry::addCapability(const QByteArray& guid, int shortId, const QString& name, const QString& description)
{
    m_capabilites.append(new ClientCapability(guid, shortId, name, description));
}

