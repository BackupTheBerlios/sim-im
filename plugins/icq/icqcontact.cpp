#include "icqcontact.h"
#include "buffer.h"

ICQContact::ICQContact(const SIM::ClientPtr& cl) : SIM::IMContact(), m_uin(0),
    m_status(0),
    m_class(0),
    m_statusTime(0),
    m_onlineTime(0),
    m_warningLevel(0),
    m_ip(0),
    m_realip(0),
    m_port(0),
    m_dcCookie(0),
    m_caps(0),
    m_caps2(0),
    m_icqId(0),
    m_checked(true),
    m_grpId(0),
    m_ignoreId(0),
    m_visibleId(0),
    m_invisibleId(0),
    m_contactVisibleId(0),
    m_contactInvisibleId(0),
    m_waitAuth(false),
    m_wantAuth(false),
    m_webAware(true),
    m_infoUpdateTime(0),
    m_pluginInfoTime(0),
    m_pluginStatusTime(0),
    m_infoFetchTime(0),
    m_pluginInfoFetchTime(0),
    m_pluginStatusFetchTime(0),
    m_mode(0),
    m_version(0),
    m_build(0),
    m_hiddenEmail(false),
    m_workcountry(0),
    m_occupation(0),
    m_followme(0),
    m_sharedFiles(false),
    m_profileFetch(false),
    m_typing(false),
    m_badClient(false),
    m_noDirect(false),
    m_invisible(false),
    m_buddyRosterId(0),
    m_buddyId(0),
    m_direct(0),
    m_directPluginInfo(0),
    m_directPluginStatus(0),
    m_client(cl)
{

}

void ICQContact::deserializeLine(const QString& key, const QString& value)
{
    QString val = value;
    if(val.startsWith('\"') && val.endsWith('\"'))
        val = val.mid(1, val.length() - 2);
    if(key == "Alias") {
        setAlias(val);
    }
    else if(key == "Cellular") {
        setCellular(val);
    }
    else if(key == "StatusTime") {
        setStatusTime(val.toULong());
    }
    else if(key == "WarningLevel") {
        setWarningLevel(val.toULong());
    }
    else if(key == "IP") {
        setIP(val.toULong());
    }
    else if(key == "RealIP") {
        setRealIP(val.toULong());
    }
    else if(key == "Port") {
        setPort(val.toULong());
    }
    else if(key == "Caps") {
        setCaps(val.toULong());
    }
    else if(key == "Caps2") {
        setCaps2(val.toULong());
    }
    else if(key == "Uin") {
        setUin(val.toULong());
    }
    else if(key == "Screen") {
        setScreen(val);
    }
    else if(key == "ID") {
        setIcqID(val.toULong());
    }
    else if(key == "GroupID") {
        setGrpID(val.toULong());
    }
    else if(key == "Ignore") {
        setIgnoreId(val.toULong());
    }
    else if(key == "Visible") {
        setContactVisibleId(val.toULong());
    }
    else if(key == "Invsible") {
        setContactInvisibleId(val.toULong());
    }
    else if(key == "WaitAuth") {
        setWaitAuth(val == "true");
    }
    else if(key == "WantAuth") {
        setWantAuth(val == "true");
    }
    else if(key == "WebAware") {
        setWebAware(val == "true");
    }
    else if(key == "InfoUpdateTime") {
        setInfoUpdateTime(val.toULong());
    }
    else if(key == "PluginInfoTime") {
        setPluginInfoTime(val.toULong());
    }
    else if(key == "PluginStatusTime") {
        setPluginStatusTime(val.toULong());
    }
    else if(key == "InfoFetchTime") {
        setInfoFetchTime(val.toULong());
    }
    else if(key == "PluginInfoFetchTime") {
        setPluginInfoFetchTime(val.toULong());
    }
    else if(key == "PluginStatusFetchTime") {
        setPluginStatusFetchTime(val.toULong());
    }
    else if(key == "Mode") {
        setMode(val.toULong());
    }
    else if(key == "Version") {
        setVersion(val.toULong());
    }
    else if(key == "Build") {
        setBuild(val.toULong());
    }
    else if(key == "Nick") {
        setNick(val);
    }
    else if(key == "FirstName") {
        setFirstName(val);
    }
    else if(key == "LastName") {
        setLastName(val);
    }
    else if(key == "MiddleName") {
        setMiddleName(val);
    }
    else if(key == "Maiden") {
        setMaiden(val);
    }
    else if(key == "EMail") {
        setEmail(val);
    }
    else if(key == "HiddenEMail") {
        setHiddenEmail(val == "true");
    }
    else if(key == "City") {
        setCity(val);
    }
    else if(key == "State") {
        setState(val);
    }
    else if(key == "HomePhone") {
        setHomePhone(val);
    }
    else if(key == "HomeFax") {
        setHomeFax(val);
    }
    else if(key == "Address") {
        setAddress(val);
    }
    else if(key == "PrivateCellular") {
        setPrivateCellular(val);
    }
    else if(key == "Zip") {
        setZip(val);
    }
    else if(key == "Country") {
        setCountry(val.toULong());
    }
    else if(key == "TimeZone") {
        setTimeZone(val.toULong());
    }
    else if(key == "Age") {
        setAge(val.toULong());
    }
    else if(key == "Gender") {
        setGender(val.toULong());
    }
    else if(key == "HomePage") {
        setHomepage(val);
    }
    else if(key == "BirthYear") {
        setBirthYear(val.toULong());
    }
    else if(key == "BirthMonth") {
        setBirthMonth(val.toULong());
    }
    else if(key == "BirthDay") {
        setBirthDay(val.toULong());
    }
    else if(key == "Language") {
        setLanguage(val.toULong());
    }
    else if(key == "WorkCity") {
        setWorkCity(val);
    }
    else if(key == "WorkState") {
        setWorkState(val);
    }
    else if(key == "WorkAddress") {
        setWorkAddress(val);
    }
    else if(key == "WorkZip") {
        setWorkZip(val);
    }
    else if(key == "WorkCountry") {
        setWorkCountry(val.toULong());
    }
    else if(key == "WorkName") {
        setWorkName(val);
    }
    else if(key == "WorkDepartment") {
        setWorkDepartment(val);
    }
    else if(key == "WorkPosition") {
        setWorkPosition(val);
    }
    else if(key == "Occupation") {
        setOccupation(val.toULong());
    }
    else if(key == "WorkHomepage") {
        setWorkHomepage(val);
    }
    else if(key == "About") {
        setAbout(val);
    }
    else if(key == "Interests") {
        setInterests(val);
    }
    else if(key == "Backgrounds") {
        setBackgrounds(val);
    }
    else if(key == "Affilations") {
        setAffilations(val);
    }
    else if(key == "FollowMe") {
        setFollowMe(val.toULong());
    }
    else if(key == "SharedFiles") {
        setSharedFiles(val == "true");
    }
    else if(key == "ICQPhone") {
        setICQPhone(val.toULong());
    }
    else if(key == "Picture") {
        setPicture(val);
    }
    else if(key == "PictureWidth") {
        setPictureWidth(val.toULong());
    }
    else if(key == "PictureHeight") {
        setPictureHeight(val.toULong());
    }
    else if(key == "PhoneBook") {
        setPhoneBook(val);
    }
    else if(key == "ProfileFetch") {
        setProfileFetch(val == "true");
    }
    else if(key == "buddyID") {
        setBuddyID(val.toULong());
    }
    else if(key == "buddyHash") {
        setBuddyHash(QByteArray::fromHex(val.toLocal8Bit()));
    }
}

QByteArray ICQContact::serialize()
{
    QString result;
    result += QString("Alias=%1\n").arg(getAlias());
    result += QString("Cellular=\"%1\"\n").arg(getCellular());
    result += QString("StatusTime=%1\n").arg(getStatusTime());
    result += QString("WarningLevel=%1\n").arg(getWarningLevel());
    result += QString("IP=%1\n").arg(getIP());
    result += QString("RealIP=%1\n").arg(getRealIP());
    result += QString("Port=%1\n").arg(getPort());
    result += QString("Caps=%1\n").arg(getCaps());
    result += QString("Caps2=%1\n").arg(getCaps2());
    result += QString("Uin=%1\n").arg(getUin());
    result += QString("Screen=%1\n").arg(getScreen());
    result += QString("ID=%1\n").arg(getIcqID());
    result += QString("GroupID=%1\n").arg(getGrpID());
    result += QString("Ignore=%1\n").arg(getIgnoreId());
    result += QString("Visible=%1\n").arg(getVisibleId());
    result += QString("Invisible=%1\n").arg(getInvisibleId());
    result += QString("WaitAuth=%1\n").arg(getWaitAuth() ? "true" : "false");
    result += QString("WantAuth=%1\n").arg(getWantAuth() ? "true" : "false");
    result += QString("WebAware=%1\n").arg(getWebAware() ? "true" : "false");
    result += QString("InfoUpdateTime=%1\n").arg(getInfoUpdateTime());
    result += QString("PluginInfoTime=%1\n").arg(getPluginInfoTime());
    result += QString("PluginStatusTime=%1\n").arg(getPluginStatusTime());
    result += QString("InfoFetchTime=%1\n").arg(getInfoFetchTime());
    result += QString("PluginInfoFetchTime=%1\n").arg(getPluginInfoFetchTime());
    result += QString("PluginStatusFetchTime=%1\n").arg(getPluginStatusFetchTime());
    result += QString("Mode=%1\n").arg(getMode());
    result += QString("Version=%1\n").arg(getVersion());
    result += QString("Build=%1\n").arg(getBuild());
    result += QString("Nick=%1\n").arg(getNick());
    result += QString("FirstName=%1\n").arg(getFirstName());
    result += QString("LastName=%1\n").arg(getLastName());
    result += QString("MiddleName=%1\n").arg(getMiddleName());
    result += QString("Maiden=%1\n").arg(getMaiden());
    result += QString("EMail=%1\n").arg(getEmail());
    result += QString("HiddenEMail=%1\n").arg(getHiddenEmail() ? "true" : "false");
    result += QString("City=%1\n").arg(getCity());
    result += QString("State=%1\n").arg(getState());
    result += QString("HomePhone=%1\n").arg(getHomePhone());
    result += QString("HomeFax=%1\n").arg(getHomeFax());
    result += QString("Address=\"%1\"\n").arg(getAddress());
    result += QString("PrivateCellular=%1\n").arg(getPrivateCellular());
    result += QString("Zip=%1\n").arg(getZip());
    result += QString("Country=%1\n").arg(getCountry());
    result += QString("TimeZone=%1\n").arg(getTimeZone());
    result += QString("Age=%1\n").arg(getAge());
    result += QString("Gender=%1\n").arg(getGender());
    result += QString("Homepage=%1\n").arg(getHomepage());
    result += QString("BirthYear=%1\n").arg(getBirthYear());
    result += QString("BirthMonth=%1\n").arg(getBirthMonth());
    result += QString("BirthDay=%1\n").arg(getBirthDay());
    result += QString("Language=%1\n").arg(getLanguage());
    result += QString("WorkCity=%1\n").arg(getWorkCity());
    result += QString("WorkState=%1\n").arg(getWorkState());
    result += QString("WorkAddress=%1\n").arg(getWorkAddress());
    result += QString("WorkZip=%1\n").arg(getWorkZip());
    result += QString("WorkCountry=%1\n").arg(getWorkCountry());
    result += QString("WorkName=%1\n").arg(getWorkName());
    result += QString("WorkDepartment=%1\n").arg(getWorkDepartment());
    result += QString("WorkPosition=%1\n").arg(getWorkPosition());
    result += QString("Occupation=%1\n").arg(getOccupation());
    result += QString("WorkHomepage=%1\n").arg(getWorkHomepage());
    result += QString("About=%1\n").arg(getAbout());
    result += QString("Interests=%1\n").arg(getInterests());
    result += QString("Backgrounds=%1\n").arg(getBackgrounds());
    result += QString("Affilations=%1\n").arg(getAffilations());
    result += QString("FollowMe=%1\n").arg(getFollowMe());
    result += QString("SharedFiles=%1\n").arg(getSharedFiles() ? "true" : "false");
    result += QString("Picture=\"%1\"\n").arg(getPicture());
    result += QString("PictureWidth=%1\n").arg(getPictureWidth());
    result += QString("PictureHeight=%1\n").arg(getPictureHeight());
    result += QString("PhoneBook=\"%1\"\n").arg(getPhoneBook());
    result += QString("ProfileFetch=%1\n").arg(getProfileFetch() ? "true" : "false");
    result += QString("buddyID=%1\n").arg(getBuddyID());
    result += QString("buddyHash=%1\n").arg(QString(getBuddyHash().toHex()));

    return result.toLocal8Bit();
}

void ICQContact::deserialize(Buffer* cfg)
{
    for(;;)
    {
        const QString line = QString::fromUtf8(cfg->getLine());
        if (line.isEmpty())
            break;
        QStringList keyval = line.split('=');
        if(keyval.size() < 2)
            continue;
        deserializeLine(keyval.at(0), keyval.at(1));
    }
}

void ICQContact::serialize(QDomElement& element)
{
    SIM::PropertyHubPtr hub = SIM::PropertyHub::create();
    hub->setValue("Alias", getAlias());
    hub->setValue("Cellular", getCellular());
    hub->setValue("StatusTime", getStatusTime());
    hub->setValue("WarningLevel", getWarningLevel());
    hub->setValue("IP", (unsigned int)getIP());
    hub->setValue("RealIP", (unsigned int)getRealIP());
    hub->setValue("Port", getPort());
    hub->setValue("Caps", (unsigned int)getCaps());
    hub->setValue("Caps2", (unsigned int)getCaps2());
    hub->setValue("Uin", (unsigned int)getUin());
    hub->setValue("Screen", getScreen());
    hub->setValue("ID", (unsigned int)getIcqID());
    hub->setValue("GroupID", (unsigned int)getGrpID());
    hub->setValue("Ignore", (unsigned int)getIgnoreId());
    hub->setValue("Visible", (unsigned int)getVisibleId());
    hub->setValue("Invisible", (unsigned int)getInvisibleId());
    hub->setValue("WaitAuth", getWaitAuth());
    hub->setValue("WantAuth", getWantAuth());
    hub->setValue("WebAware", getWebAware());
    hub->setValue("InfoUpdateTime", (unsigned int)getInfoUpdateTime());
    hub->setValue("PluginInfoTime", (unsigned int)getPluginInfoTime());
    hub->setValue("PluginStatusTime", (unsigned int)getPluginStatusTime());
    hub->setValue("InfoFetchTime", (unsigned int)getInfoFetchTime());
    hub->setValue("PluginInfoFetchTime", (unsigned int)getPluginInfoFetchTime());
    hub->setValue("PluginStatusFetchTime", (unsigned int)getPluginStatusFetchTime());
    hub->setValue("Mode", getMode());
    hub->setValue("Version", getVersion());
    hub->setValue("Build", getBuild());
    hub->setValue("Nick", getNick());
    hub->setValue("FirstName", getFirstName());
    hub->setValue("LastName", getLastName());
    hub->setValue("MiddleName", getMiddleName());
    hub->setValue("Maiden", getMaiden());
    hub->setValue("EMail", getEmail());
    hub->setValue("HiddenEMail", getHiddenEmail());
    hub->setValue("City", getCity());
    hub->setValue("State", getState());
    hub->setValue("HomePhone", getHomePhone());
    hub->setValue("HomeFax", getHomeFax());
    hub->setValue("Address", getAddress());
    hub->setValue("PrivateCellular", getPrivateCellular());
    hub->setValue("Zip", getZip());
    hub->setValue("Country", (unsigned int)getCountry());
    hub->setValue("TimeZone", (unsigned int)getTimeZone());
    hub->setValue("Age", (unsigned int)getAge());
    hub->setValue("Gender", (unsigned int)getGender());
    hub->setValue("HomePage", getHomepage());
    hub->setValue("BirthYear", (unsigned int)getBirthYear());
    hub->setValue("BirthMonth", (unsigned int)getBirthMonth());
    hub->setValue("BirthDay", (unsigned int)getBirthDay());
    hub->setValue("Language", (unsigned int)getLanguage());
    hub->setValue("WorkCity", getWorkCity());
    hub->setValue("WorkState", getWorkState());
    hub->setValue("WorkAddress", getWorkAddress());
    hub->setValue("WorkZip", getWorkZip());
    hub->setValue("WorkCountry", (unsigned int)getWorkCountry());
    hub->setValue("WorkName", getWorkName());
    hub->setValue("WorkDepartment", getWorkDepartment());
    hub->setValue("WorkPosition", getWorkPosition());
    hub->setValue("Occupation", (unsigned int)getOccupation());
    hub->setValue("WorkHomepage", getWorkHomepage());
    hub->setValue("About", getAbout());
    hub->setValue("Interests", getInterests());
    hub->setValue("Backgrounds", getBackgrounds());
    hub->setValue("Affilations", getAffilations());
    hub->setValue("FollowMe", (unsigned int)getFollowMe());
    hub->setValue("SharedFiles", getSharedFiles());
    hub->setValue("ICQPhone", (unsigned int)getICQPhone());
    hub->setValue("Picture", getPicture());
    hub->setValue("PictureWidth", (unsigned int)getPictureWidth());
    hub->setValue("PictureHeight", (unsigned int)getPictureHeight());
    hub->setValue("PhoneBook", getPhoneBook());
    hub->setValue("ProfileFetch", getProfileFetch());
    hub->setValue("buddyID", (unsigned int)getBuddyID());
    hub->setValue("buddyHash", getBuddyHash());
    hub->setValue("unknown2", getUnknown(2));
    hub->setValue("unknown4", getUnknown(4));
    hub->setValue("unknown5", getUnknown(5));
    hub->serialize(element);
}

void ICQContact::deserialize(QDomElement& element)
{
    SIM::PropertyHubPtr hub = SIM::PropertyHub::create();
    hub->deserialize(element);
    setAlias(hub->value("Alias").toString());
    setCellular(hub->value("Cellular").toString());
    setStatusTime(hub->value("StatusTime").toUInt());
    setWarningLevel(hub->value("WarningLevel").toUInt());
    setIP(hub->value("IP").toUInt());
    setRealIP(hub->value("RealIP").toUInt());
    setPort(hub->value("Port").toUInt());
    setCaps(hub->value("Caps").toUInt());
    setCaps2(hub->value("Caps2").toUInt());
    setUin(hub->value("Uin").toUInt());
    setScreen(hub->value("Screen").toString());
    setIcqID(hub->value("ID").toUInt());
    setGrpID(hub->value("GroupID").toUInt());
    setIgnoreId(hub->value("Ignore").toUInt());
    setVisibleId(hub->value("Visible").toUInt());
    setInvisibleId(hub->value("Invisible").toUInt());
    setWaitAuth(hub->value("WaitAuth").toBool());
    setWantAuth(hub->value("WantAuth").toBool());
    setWebAware(hub->value("WebAware").toBool());
    setInfoUpdateTime(hub->value("InfoUpdateTime").toUInt());
    setPluginInfoTime(hub->value("PluginInfoTime").toUInt());
    setPluginStatusTime(hub->value("PluginStatusTime").toUInt());
    setInfoFetchTime(hub->value("InfoFetchTime").toUInt());
    setPluginInfoFetchTime(hub->value("PluginInfoFetchTime").toUInt());
    setPluginStatusFetchTime(hub->value("PluginStatusFetchTime").toUInt());
    setMode(hub->value("Mode").toUInt());
    setVersion(hub->value("Version").toUInt());
    setBuild(hub->value("Build").toUInt());
    setNick(hub->value("Nick").toString());
    setFirstName(hub->value("FirstName").toString());
    setLastName(hub->value("LastName").toString());
    setMiddleName(hub->value("MiddleName").toString());
    setMaiden(hub->value("Maiden").toString());
    setEmail(hub->value("EMail").toString());
    setHiddenEmail(hub->value("HiddenEMail").toBool());
    setCity(hub->value("City").toString());
    setState(hub->value("State").toString());
    setHomePhone(hub->value("HomePhone").toString());
    setHomeFax(hub->value("HomeFax").toString());
    setAddress(hub->value("Address").toString());
    setPrivateCellular(hub->value("PrivateCellular").toString());
    setZip(hub->value("Zip").toString());
    setCountry(hub->value("Country").toUInt());
    setTimeZone(hub->value("TimeZone").toUInt());
    setAge(hub->value("Age").toUInt());
    setGender(hub->value("Gender").toUInt());
    setHomepage(hub->value("Homepage").toString());
    setBirthYear(hub->value("BirthYear").toUInt());
    setBirthMonth(hub->value("BirthMonth").toUInt());
    setBirthDay(hub->value("BirthDay").toUInt());
    setLanguage(hub->value("Language").toUInt());
    setWorkCity(hub->value("WorkCity").toString());
    setWorkState(hub->value("WorkState").toString());
    setWorkAddress(hub->value("WorkAddress").toString());
    setWorkZip(hub->value("WorkZip").toString());
    setWorkCountry(hub->value("WorkCountry").toUInt());
    setWorkName(hub->value("WorkName").toString());
    setWorkDepartment(hub->value("WorkDepartment").toString());
    setWorkPosition(hub->value("WorkPosition").toString());
    setOccupation(hub->value("WorkOccupation").toUInt());
    setWorkHomepage(hub->value("WorkHomepage").toString());
    setAbout(hub->value("About").toString());
    setInterests(hub->value("Interests").toString());
    setBackgrounds(hub->value("Backgrounds").toString());
    setAffilations(hub->value("Affilations").toString());
    setFollowMe(hub->value("FollowMe").toUInt());
    setSharedFiles(hub->value("SharedFiles").toBool());
    setICQPhone(hub->value("ICQPhone").toUInt());
    setPicture(hub->value("Picture").toString());
    setPictureWidth(hub->value("PictureWidth").toUInt());
    setPictureHeight(hub->value("PictureHeight").toUInt());
    setPhoneBook(hub->value("PhoneBook").toString());
    setProfileFetch(hub->value("ProfileFetch").toBool());
    setBuddyID(hub->value("buddyId").toUInt());
    setBuddyHash(hub->value("buddyHash").toByteArray());
    setUnknown(2, hub->value("unknown2").toByteArray());
    setUnknown(4, hub->value("unknown4").toByteArray());
    setUnknown(5, hub->value("unknown5").toByteArray());
}

SIM::IMStatusPtr ICQContact::status()
{
    return SIM::IMStatusPtr();
}

bool ICQContact::sendMessage(const SIM::MessagePtr& message)
{
    return true;
}

bool ICQContact::hasUnreadMessages()
{
    return true;
}

SIM::MessagePtr ICQContact::dequeueUnreadMessage()
{
    return SIM::MessagePtr();
}

void ICQContact::enqueueUnreadMessage(const SIM::MessagePtr& message)
{

}

SIM::IMGroupWeakPtr ICQContact::group()
{
    return SIM::IMGroupWeakPtr();
}

QString ICQContact::makeToolTipText()
{
    return QString("IMPLEMENT ME!!! ICQContact::makeToolTipText");
}

