/***************************************************************************
                          icqclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QTimer>
#include <QTextCodec>
#include <QRegExp>
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QByteArray>
#include <QDateTime>

#include "clientmanager.h"
#include "buffer.h"
#include "log.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include "icq.h"
#include "icqclient.h"
//#include "icqconfig.h"
#include "icqgroup.h"
#include "icqstatuswidget.h"
#include "imagestorage/imagestorage.h"
#include "standardoscarsocket.h"
#include "authorizationsnachandler.h"
#include "bytearraybuilder.h"
#include "bytearrayparser.h"

//#include "aimconfig.h"
//#include "icqinfo.h"
//#include "homeinfo.h"
//#include "workinfo.h"
//#include "moreinfo.h"
//#include "aboutinfo.h"
//#include "interestsinfo.h"
//#include "pastinfo.h"
//#include "icqpicture.h"
//#include "aiminfo.h"
//#include "icqsearch.h"
//#include "icqsecure.h"
//#include "icqmessage.h"
//#include "securedlg.h"
//#include "msgedit.h"
//#include "simgui/ballonmsg.h"
//#include "encodingdlg.h"
//#include "warndlg.h"

//#include "icqbuddy.h"
//#include "icqservice.h"

//#include "icqdirect.h"

const unsigned short FLAP_START = 0x2A;

using namespace std;
using namespace SIM;


static const char aim_server[] = "login.oscar.aol.com";
//static const char icq_server[] = "login.icq.com";
static const char icq_server[] = "login.messaging.aol.com";

ICQClientData::ICQClientData(ICQClient* client)
    : m_port(5190)
    , owner(client)
{

}

QByteArray ICQClientData::serialize()
{
    QString result;
    result += QString("Server=%1\n").arg(getServer());
    result += QString("ServerPort=%1\n").arg(getPort());
    result += QString("HideIP=%1\n").arg(getHideIP() ? "true" : "false");
    result += QString("IgnoreAuth=%1\n").arg(getIgnoreAuth() ? "true" : "false");
    result += QString("UseMD5=%1\n").arg(getUseMD5() ? "true" : "false");
    result += QString("DirectMode=%1\n").arg(getDirectMode());
    result += QString("IdleTime=%1\n").arg(getIdleTime());
    result += QString("ListRequests=%1\n").arg(getListRequests());
    result += QString("Picture=\"%1\"\n").arg(getPicture());
    result += QString("RandomChatGroup=%1\n").arg(getRandomChatGroup());
    result += QString("DisablePlugins=%1\n").arg(getDisablePlugins() ? "true" : "false");
    result += QString("DisableAutoUpdate=%1\n").arg(getDisableAutoUpdate() ? "true" : "false");
    result += QString("DisableAutoReplyUpdate=%1\n").arg(getDisableAutoReplyUpdate() ? "true" : "false");
    result += QString("DisableTypingNotification=%1\n").arg(getDisableTypingNotification() ? "true" : "false");
    result += QString("AcceptInDND=%1\n").arg(getAcceptInDND() ? "true" : "false");
    result += QString("AcceptInOccupied=%1\n").arg(getAcceptInOccupied() ? "true" : "false");
    result += QString("MinPort=%1\n").arg(getMinPort());
    result += QString("MaxPort=%1\n").arg(getMaxPort());
    result += QString("WarnAnonimously=%1\n").arg(getWarnAnonymously() ? "true" : "false");
    result += QString("ACKMode=%1\n").arg(getAckMode());
    result += QString("UseHTTP=%1\n").arg(getUseHttp() ? "true" : "false");
    result += QString("AutoHTTP=%1\n").arg(getAutoHttp() ? "true" : "false");
    result += QString("KeepAlive=%1\n").arg(getKeepAlive() ? "true" : "false");
    result += QString("MediaSense=%1\n").arg(getMediaSense() ? "true" : "false");

    return result.toLocal8Bit();
}

void ICQClientData::deserialize(Buffer* cfg)
{
    
    for(;;)
    {
        const QString line = QString::fromUtf8(cfg->getLine());
        if (line.isEmpty())
            break;
        QStringList keyval = line.split('=');
        if(keyval.size() < 2)
            continue;
		//if (keyval.at(0) == QString("Password"))
		//	__asm int 3;
        deserializeLine(keyval.at(0), keyval.at(1));
    }
}

void ICQClientData::deserializeLine(const QString& key, const QString& value)
{
    log(L_DEBUG, "[ICQClientData]key: %s, value: %s", qPrintable(key), qPrintable(value));
    QString val = value;
    if(val.startsWith('\"') && val.endsWith('\"'))
        val = val.mid(1, val.length() - 2);
    if(key == "Server") {
        setServer(val);
        return;
    }
	if(key == "Password") {
		owner.client()->setCryptedPassword(val);
    }
    if(key == "ServerPort") {
        setPort(val.toULong());
        return;
    }
    if(key == "HideIP") {
        setHideIP(val == "true");
        return;
    }
    if(key == "IgnoreAuth") {
        setIgnoreAuth(val == "true");
        return;
    }
    if(key == "UseMD5") {
        setUseMD5(val == "true");
        return;
    }
    if(key == "DirectMode") {
        setDirectMode(val.toULong());
        return;
    }
    if(key == "IdleTime") {
        setIdleTime(val.toULong());
        return;
    }
    if(key == "ListRequests") {
        setListRequests  (val);
        return;
    }
    if(key == "Picture") {
        setPicture(val);
        return;
    }
    if(key == "RandomChatGroup") {
        setRandomChatGroup(val.toULong());
        return;
    }
    if(key == "SendFormat") {
        setSendFormat(val.toULong());
        return;
    }
    if(key == "DisablePlugins") {
        setDisablePlugins(val == "true");
        return;
    }
    if(key == "DisableAutoUpdate") {
        setDisableAutoUpdate(val == "true");
        return;
    }
    if(key == "DisableAutoReplyUpdate") {
        setDisableAutoReplyUpdate(val == "true");
        return;
    }
    if(key == "DisableTypingNotification") {
        setDisableTypingNotification(val == "true");
        return;
    }
    if(key == "AcceptInDND") {
        setAcceptInDND(val == "true");
        return;
    }
    if(key == "AcceptInOccupied") {
        setAcceptInOccupied(val == "true");
        return;
    }
    if(key == "MinPort") {
        setMinPort(val.toULong());
        return;
    }
    if(key == "MaxPort") {
        setMaxPort(val.toULong());
        return;
    }
    if(key == "WarnAnonimously") {
        setWarnAnonymously(val == "true");
        return;
    }
    if(key == "ACKMode") {
        setAckMode(val.toULong());
        return;
    }
    if(key == "UseHTTP") {
        setUseHttp(val == "true");
        return;
    }
    if(key == "AutoHTTP") {
        setAutoHttp(val == "true");
        return;
    }
    if(key == "KeepAlive") {
        setKeepAlive(val == "true");
        return;
    }
    if(key == "MediaSense") {
        setMediaSense(val == "true");
        return;
    }
    owner.deserializeLine(key, value);
}

unsigned long ICQClientData::getSign()
{
    return 0;
}

ICQClient::ICQClient(SIM::Protocol* protocol, const QString& name, bool bAIM) : SIM::Client(protocol),
    m_name(name), m_state(ICQClient::sOffline)
{
    initialize(bAIM);
    clientPersistentData = new ICQClientData(this);
    m_oscarSocket = new StandardOscarSocket(this);
    connect(m_oscarSocket, SIGNAL(connected()), this, SLOT(oscarSocketConnected()));
    connect(m_oscarSocket, SIGNAL(packet(int, QByteArray)), this, SLOT(oscarSocketPacket(int, QByteArray)));
}

ICQClient::~ICQClient()
{
    if(m_oscarSocket)
        delete m_oscarSocket;
    qDeleteAll(m_snacHandlers);
    delete clientPersistentData;
}

void ICQClient::initialize(bool bAIM)
{
    initDefaultStates();
    m_currentStatus = getDefaultStatus("offline");
	m_bAIM = bAIM;
    initSnacHandlers();
    

//    clientPersistentData->owner.setDCcookie(rand());

//    QString requests = getListRequests();
//    while (requests.length())
//    {
//        QString req = getToken(requests, ';');
//        QString n = getToken(req, ',');
//        ListRequest lr;
//        lr.type   = n.toUInt();
//        lr.screen = req;
//        listRequests.push_back(lr);
//    }

//    m_snacBuddy = new SnacIcqBuddy(this);
//    m_snacICBM = new SnacIcqICBM(this);
//    m_snacService = new SnacIcqService(this);
//    addSnacHandler(m_snacBuddy);
//    addSnacHandler(m_snacICBM);
//    addSnacHandler(m_snacService);

//    m_processTimer = new QTimer(this);
//    connect(m_processTimer, SIGNAL(timeout()), m_snacICBM, SLOT(processSendQueue()));

//    disconnected();

//    ContactList::ContactIterator it;
//    Contact *contact;
//    while ((contact = ++it) != NULL)
//    {
//        ClientDataIterator itd = contact->clientDataIterator(this);
//        ICQUserData *data;
//        while ((data = toICQUserData(++itd)) != NULL)
//            data->setAlias(contact->getName());
//    }
//    if ( !getMediaSense() )
//        return;
}

//ICQClient::~ICQClient()
//{
//    setStatus(STATUS_OFFLINE, false);
//    freeData(); // before deleting of other members!

//    delete m_listener;
//    delete m_snacService;
//    delete m_snacBuddy;
//    delete m_snacICBM;
//    delete m_ifChecker; //independed if MediaSense is activated, it can be risk-less deleted, because it is initilized with NULL
//	//free_data(icqClientData, &data);
//    delete socket();
//    for(list<Message*>::iterator it = m_processMsg.begin(); it != m_processMsg.end(); ++it)
//	{
//        Message *msg = *it;
//        msg->setError(I18N_NOOP("Process message failed")); //crashed on shutdown
// // FIXME: this does not work and could crash!!!!
// //       Event e(EventRealSendMessage, msg);
// //       e.process();
//        delete msg;
//    }
//    while (!m_sockets.empty())
//        delete m_sockets.front();
//    m_processMsg.clear();
//}

//SIM::IMContact* ICQClient::getOwnerContact()
//{
//    return &clientPersistentData->owner;
//}

QString ICQClient::retrievePasswordLink()
{
    return QString("http://www.icq.com");
}

bool ICQClient::serialize(QDomElement& element)
{
    SIM::PropertyHubPtr hub = SIM::PropertyHub::create();
    hub->setValue("Server", getServer());
    hub->setValue("ServerPort", (unsigned int)getPort());
    hub->setValue("HideIP", getHideIP());
    hub->setValue("IgnoreAuth", getIgnoreAuth());
    hub->setValue("UseMD5", getUseMD5());
    hub->setValue("DirectMode", (unsigned int)getDirectMode());
    hub->setValue("IdleTime", (unsigned int)getIdleTime());
    hub->setValue("ListRequest", getListRequests());
    hub->setValue("Picture", getPicture());
    hub->setValue("RandomChatGroup", (unsigned int)getRandomChatGroup());
    hub->setValue("SendFormat", (unsigned int)getSendFormat());
    hub->setValue("DisablePlugins", getDisablePlugins());
    hub->setValue("DisableAutoUpdate", getDisableAutoUpdate());
    hub->setValue("DisableAutoReplyUpdate", getDisableAutoReplyUpdate());
    hub->setValue("DisableTypingNotification", getDisableTypingNotification());
    hub->setValue("AcceptInDND", getAcceptInDND());
    hub->setValue("AcceptInOccupied", getAcceptInOccupied());
    hub->setValue("MinPort", (unsigned int)getMinPort());
    hub->setValue("MaxPort", (unsigned int)getMaxPort());
    hub->setValue("WarnAnonymously", getWarnAnonymously());
    hub->setValue("ACKMode", (unsigned int)getAckMode());
    hub->setValue("UseHTTP", getUseHTTP());
    hub->setValue("AutoHTTP", getAutoHTTP());
    hub->setValue("KeepAlive", getKeepAlive());
    hub->setValue("MediaSense", getMediaSense());
    hub->serialize(element);
    return Client::serialize(element);
}

bool ICQClient::deserialize(QDomElement& element)
{
    SIM::PropertyHubPtr hub = SIM::PropertyHub::create();
    if(!hub->deserialize(element))
        return false;
    setServer(hub->value("Server").toString());
    setPort(hub->value("ServerPort").toUInt());
    setHideIP(hub->value("HideIP").toBool());
    setIgnoreAuth(hub->value("IgnoreAuth").toBool());
    setUseMD5(hub->value("UseMD5").toBool());
    setDirectMode(hub->value("DirectMode").toUInt());
    setIdleTime(hub->value("IdleTime").toUInt());
    setListRequests(hub->value("ListRequests").toString());
    setPicture(hub->value("Picture").toString());
    setRandomChatGroup(hub->value("RandomChatGroup").toUInt());
    setSendFormat(hub->value("SendFormat").toUInt());
    setDisablePlugins(hub->value("DisablePlugins").toBool());
    setDisableAutoUpdate(hub->value("DisableAutoUpdate").toBool());
    setDisableAutoReplyUpdate(hub->value("DisableAutoReplyUpdate").toBool());
    setDisableTypingNotification(hub->value("DisableTypingNotification").toBool());
    setAcceptInDND(hub->value("AcceptInDND").toBool());
    setAcceptInOccupied(hub->value("AcceptInOccupied").toBool());
    setMinPort(hub->value("MinPort").toUInt());
    setMaxPort(hub->value("MaxPort").toUInt());
    setWarnAnonymously(hub->value("WarnAnonymously").toBool());
    setAckMode(hub->value("ACKMode").toUInt());
    setUseHTTP(hub->value("UseHTTP").toBool());
    setAutoHTTP(hub->value("AutoHTTP").toBool());
    setKeepAlive(hub->value("KeepAlive").toBool());
    setMediaSense(hub->value("MediaSense").toBool());
    return Client::deserialize(element);
}

bool ICQClient::deserialize(Buffer* cfg)
{
    log(L_DEBUG, "ICQClient::deserialize");
    clientPersistentData->deserialize(cfg);
    //Client::deserialize(cfg);
    m_name = "";
    return true;
}

void ICQClient::initSnacHandlers()
{
    m_authSnac = new AuthorizationSnacHandler(this);
    m_serviceSnac = new ServiceSnacHandler(this);
    m_snacHandlers.insert(m_authSnac->getType(), m_authSnac);
    m_snacHandlers.insert(m_serviceSnac->getType(), m_serviceSnac);
}

SIM::IMStatusPtr ICQClient::currentStatus()
{
    return m_currentStatus;
}

void ICQClient::changeStatus(const SIM::IMStatusPtr& status)
{
    if((m_state == sOffline) && !status->flag(SIM::IMStatus::flOffline))
    {
        emit setStatusWidgetsBlinking(true);
        oscarSocket()->connectToHost(clientPersistentData->getServer(), clientPersistentData->getPort());
    }
    //    if (status->id() == "offline")
    //    {
    //        flap(ICQ_CHNxCLOSE);
    //        return;
    //    }
    //    if (m_bAIM)
    //    {
    //        if (status->id() == "online")
    //        {
    //            IMStatusPtr newstatus = protocol()->status("away");
    //            // TODO obtain AR

    //            /*
    //            ar_request req;
    //            req.bDirect = true;
    //            arRequests.push_back(req);

    //            ARRequest ar;
    //            ar.contact  = NULL;
    //            ar.param    = &arRequests.back();
    //            ar.receiver = this;
    //            ar.status   = status;
    //            EventARRequest(&ar).process();
    //            EventClientChanged(this).process();
    //            */
    //        }
    //        else if (m_status != STATUS_ONLINE)
    //        {
    //            setAwayMessage();
    //        }
    //    }
    //    else
    //    {
    //        if (status->id() == currentStatus()->id())
    //            return;

    //        snacService()->sendStatus(fullStatus(status));
    //    }
    //    TCPClient::changeStatus(status);
}

SIM::IMStatusPtr ICQClient::savedStatus()
{
    return getDefaultStatus("offline");
}

//void ICQClient::setOwnerContact(SIM::IMContact* contact)
//{
////    ICQUserData* d = 0;
////    if(contact->getSign() == ICQ_SIGN)
////        d = static_cast<ICQUserData*>(contact);

////    if(d)
////        clientPersistentData->owner = *d;
//}

unsigned long ICQClient::getContactsTime() const
{
    return clientPersistentData->getContactsTime();
}

void ICQClient::setContactsTime(unsigned long contactsTime)
{
    clientPersistentData->setContactsTime(contactsTime);
}

unsigned short ICQClient::getContactsLength() const
{
    return clientPersistentData->getContactsLength();
}

void ICQClient::setContactsLength(unsigned short contactsLength)
{
    clientPersistentData->setContactsLength(contactsLength);
}

unsigned short ICQClient::getContactsInvisible() const
{
    return clientPersistentData->getContactsInvisible();
}

void ICQClient::setContactsInvisible(unsigned short contactsInvisible)
{
    clientPersistentData->setContactsInvisible(contactsInvisible);
}

bool ICQClient::getHideIP() const
{
    return clientPersistentData->getHideIP();
}

void ICQClient::setHideIP(bool hideip)
{
    clientPersistentData->setHideIP(hideip);
}

bool ICQClient::getIgnoreAuth() const
{
    return clientPersistentData->getIgnoreAuth();
}

void ICQClient::setIgnoreAuth(bool ignoreAuth)
{
    clientPersistentData->setIgnoreAuth(ignoreAuth);
}

bool ICQClient::getUseMD5() const
{
    return clientPersistentData->getUseMD5();
}

void ICQClient::setUseMD5(bool usemd5)
{
    clientPersistentData->setUseMD5(usemd5);
}

unsigned long ICQClient::getDirectMode()
{
    return clientPersistentData->getDirectMode();
}

void ICQClient::setDirectMode(unsigned long mode)
{
    clientPersistentData->setDirectMode(mode);
}

unsigned long ICQClient::getIdleTime() const
{
    return clientPersistentData->getIdleTime();
}

void ICQClient::setIdleTime(unsigned long time)
{
    clientPersistentData->setIdleTime(time);
}

QString ICQClient::getListRequests() const
{
    return clientPersistentData->getListRequests();
}

void ICQClient::setListRequests(const QString& listrequests)
{
    clientPersistentData->setListRequests(listrequests);
}

QString ICQClient::getPicture() const
{
    return clientPersistentData->getPicture();
}

void ICQClient::setPicture(const QString& pic)
{
    clientPersistentData->setPicture(pic);
    clientPersistentData->owner.setPicture(pic);
}

unsigned long ICQClient::getRandomChatGroup() const
{
    return clientPersistentData->getRandomChatGroup();
}

void ICQClient::setRandomChatGroup(unsigned long group)
{
    clientPersistentData->setRandomChatGroup(group);
}

unsigned long ICQClient::getRandomChatGroupCurrent() const
{
    return clientPersistentData->getRandomChatGroupCurrent();
}

void ICQClient::setRandomChatGroupCurrent(unsigned long group)
{
    clientPersistentData->setRandomChatGroupCurrent(group);
}

unsigned long ICQClient::getSendFormat() const
{
    return clientPersistentData->getSendFormat();
}

void ICQClient::setSendFormat(unsigned long format)
{
    clientPersistentData->setSendFormat(format);
}

bool ICQClient::getDisablePlugins() const
{
    return clientPersistentData->getDisablePlugins();
}

void ICQClient::setDisablePlugins(bool b)
{
    clientPersistentData->setDisablePlugins(b);
}

bool ICQClient::getDisableAutoUpdate() const
{
    return clientPersistentData->getDisableAutoUpdate();
}

void ICQClient::setDisableAutoUpdate(bool b)
{
    clientPersistentData->setDisableAutoUpdate(b);
}

bool ICQClient::getDisableAutoReplyUpdate() const
{
    return clientPersistentData->getDisableAutoReplyUpdate();
}

void ICQClient::setDisableAutoReplyUpdate(bool b)
{
    clientPersistentData->setDisableAutoReplyUpdate(b);
}

bool ICQClient::getDisableTypingNotification() const
{
    return clientPersistentData->getDisableTypingNotification();
}

void ICQClient::setDisableTypingNotification(bool b)
{
    clientPersistentData->setDisableTypingNotification(b);
}

bool ICQClient::getAcceptInDND() const
{
    return clientPersistentData->getAcceptInDND();
}

void ICQClient::setAcceptInDND(bool b)
{
    clientPersistentData->setAcceptInDND(b);
}

bool ICQClient::getAcceptInOccupied() const
{
    return clientPersistentData->getAcceptInOccupied();
}

void ICQClient::setAcceptInOccupied(bool b)
{
    clientPersistentData->setAcceptInOccupied(b);
}

unsigned long ICQClient::getMinPort() const
{
    return clientPersistentData->getMinPort();
}

void ICQClient::setMinPort(unsigned long port)
{
    clientPersistentData->setMinPort(port);
}

unsigned long ICQClient::getMaxPort() const
{
    return clientPersistentData->getMinPort();
}

void ICQClient::setMaxPort(unsigned long port)
{
    clientPersistentData->setMinPort(port);
}

bool ICQClient::getWarnAnonymously() const
{
    return clientPersistentData->getWarnAnonymously();
}

void ICQClient::setWarnAnonymously(bool b)
{
    clientPersistentData->setWarnAnonymously(b);
}

unsigned long ICQClient::getAckMode() const
{
    return clientPersistentData->getAckMode();
}

void ICQClient::setAckMode(unsigned long mode)
{
    clientPersistentData->setAckMode(mode);
}

bool ICQClient::getUseHTTP() const
{
    return clientPersistentData->getUseHttp();
}

void ICQClient::setUseHTTP(bool b)
{
    clientPersistentData->setUseHttp(b);
}

bool ICQClient::getAutoHTTP() const
{
    return clientPersistentData->getAutoHttp();
}

void ICQClient::setAutoHTTP(bool b)
{
    clientPersistentData->setAutoHttp(b);
}

bool ICQClient::getKeepAlive() const
{
    return clientPersistentData->getKeepAlive();
}

void ICQClient::setKeepAlive(bool b)
{
    clientPersistentData->setKeepAlive(b);
}

bool ICQClient::getMediaSense() const
{
    return clientPersistentData->getMediaSense();
}

void ICQClient::setMediaSense(bool b)
{
    clientPersistentData->setMediaSense(b);
}

//bool ICQClient::addSnacHandler(SnacHandler* handler)
//{
//	if(!handler)
//		return false;
//	mapSnacHandlers::iterator it = m_snacHandlers.find(handler->getType());
//    if(it != m_snacHandlers.end())
//        delete it->second;
//	m_snacHandlers[handler->getType()] = handler;
//	return true;
//}

//void ICQClient::clearSnacHandlers()
//{
//	// TODO
//}

//void ICQClient::deleteFileMessage(MessageId const& cookie)
//{
//    for(list<Message*>::iterator it = m_processMsg.begin(); it != m_processMsg.end(); ++it)
//	{
//		if((*it)->baseType() == MessageFile)
//		{
//			AIMFileMessage* afm = static_cast<AIMFileMessage*>(*it);
//            if (afm && afm->getID_L() == cookie.id_l && afm->getID_H() == cookie.id_h)
//            {
//                m_processMsg.erase(it);
//                return;
//            }
//		}
//	}
//}

//void ICQClient::contactsLoaded()
//{
//}


//bool ICQClient::compareData(void *d1, void *d2)
//{
//    ICQUserData *data1 = toICQUserData((SIM::IMContact*) d1); // FIXME unsafe type conversion
//    ICQUserData *data2 = toICQUserData((SIM::IMContact*) d2); // FIXME unsafe type conversion
//    if (data1->getUin())
//        return data1->getUin() == data2->getUin();
//    if (data2->getUin())
//        return false;
//    return (data1->getScreen() == data2->getScreen());
//}

//QByteArray ICQClient::getConfig()
//{
//    QString listRequest;
//    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it)
//    {
//        if (listRequest.length())
//            listRequest += ';';
//        listRequest += QString::number(it->type);
//        listRequest += ',';
//        listRequest += it->screen;
//    }
//    setListRequests(listRequest);
//    QByteArray res = Client::getConfig();
//    if (res.length())
//        res += '\n';
//    return res += clientPersistentData->serialize(); //save_data(icqClientData, &data);
//}

QString ICQClient::name()
{
	if(!m_name.isEmpty())
        return m_name;
	if (m_bAIM) 
		m_name = "AIM." + clientPersistentData->owner.getScreen();
	else
		m_name = "ICQ." + QString::number(clientPersistentData->owner.getUin());
	
    return m_name;
}

SIM::IMContactPtr ICQClient::createIMContact()
{
    return SIM::IMContactPtr(new ICQContact(this));
}

SIM::IMGroupPtr ICQClient::createIMGroup()
{
    return SIM::IMGroupPtr(new ICQGroup(this));
}

QWidget* ICQClient::createSetupWidget(const QString& id, QWidget* parent)
{
	return new QWidget();
}

void ICQClient::destroySetupWidget()
{
}

QStringList ICQClient::availableSetupWidgets() const
{
	return QStringList();
}

QWidget* ICQClient::createStatusWidget()
{
    ICQStatusWidget* widget = new ICQStatusWidget(this);
    connect(this, SIGNAL(setStatusWidgetsBlinking(bool)), widget, SLOT(setBlinking(bool)));
    return widget;
}

SIM::IMContactPtr ICQClient::ownerContact()
{
	return SIM::IMContactPtr();
}

void ICQClient::setOwnerContact(SIM::IMContactPtr contact)
{
}


QWidget* ICQClient::createSearchWidow(QWidget *parent)
{
	return new QWidget();
}

QList<SIM::IMGroupPtr> ICQClient::groups()
{
	return QList<SIM::IMGroupPtr>();
}

QList<SIM::IMContactPtr> ICQClient::contacts()
{
	return QList<SIM::IMContactPtr>();
}

void ICQClient::initDefaultStates()
{
    ICQStatus* status = new ICQStatus("offline", "Offline", false, QString(), getImageStorage()->pixmap("ICQ_offline"));
    status->setFlag(IMStatus::flOffline, true);
    m_defaultStates.append(ICQStatusPtr(status));

    status = new ICQStatus("online", "Online", false, QString(), getImageStorage()->pixmap("ICQ_online"));
    status->setFlag(IMStatus::flOffline, false);
    m_defaultStates.append(ICQStatusPtr(status));
}

ICQStatusPtr ICQClient::getDefaultStatus(const QString& id)
{
    foreach(const ICQStatusPtr& status, m_defaultStates)
    {
        if(status->id() == id)
            return status->clone().dynamicCast<ICQStatus>();
    }
    return ICQStatusPtr();
}

//QString ICQClient::getScreen()
//{
//    if (m_bAIM)
//        return clientPersistentData->owner.getScreen();
//    return QString::number(clientPersistentData->owner.getUin());
//}

//QWidget	*ICQClient::setupWnd()
//{
//    if (m_bAIM)
//        return new AIMConfig(NULL, this, true);
//    return new ICQConfig(NULL, this, true);
//}

QString ICQClient::getServer() const
{
    return clientPersistentData->getServer();
}

void ICQClient::setServer(const QString &server)
{
    clientPersistentData->setServer(server);
}

unsigned short ICQClient::getPort() const
{
    return clientPersistentData->getPort();
}

void ICQClient::setPort(unsigned short port)
{
    clientPersistentData->setPort(port);
}

void ICQClient::setUin(unsigned long uin)
{
    clientPersistentData->owner.setUin(uin);
}

void ICQClient::setScreen(const QString &screen)
{
    clientPersistentData->owner.setScreen(screen);
}

unsigned long ICQClient::getUin()
{
    return clientPersistentData->owner.getUin();
}

void ICQClient::setOscarSocket(OscarSocket* socket)
{
    if(m_oscarSocket)
        delete m_oscarSocket;
    m_oscarSocket = socket;
    connect(m_oscarSocket, SIGNAL(packet(int,QByteArray)), this, SLOT(oscarSocketPacket(int,QByteArray)));
}

OscarSocket* ICQClient::oscarSocket() const
{
    return m_oscarSocket;
}

void ICQClient::oscarSocketConnected()
{
    log(L_DEBUG, "Connected, waiting for server to initiate login sequence");
}

//void ICQClient::generateCookie(MessageId& id)
//{
//	// Just for fun:
//	id.id_h = rand() + (rand() << 16);
//	id.id_l = rand() + (rand() << 16);
//}

//bool ICQClient::isMyData(IMContact *&_data, Contact *&contact)
//{
//    if (_data->getSign() != ICQ_SIGN)
//        return false;
//    ICQUserData *data = toICQUserData(_data);
//    if (m_bAIM)
//    {
//        if (!data->getScreen().isEmpty() &&
//            !this->clientPersistentData->owner.getScreen().isEmpty() &&
//            data->getScreen().toLower() == this->clientPersistentData->owner.getScreen().toLower())
//            return false;
//    }
//    else if (data->getUin() == this->clientPersistentData->owner.getUin())
//        return false;
//    ICQUserData *my_data = findContact(screen(data), NULL, false, contact);
//    if (my_data)
//        data = my_data;
//    else
//        contact = NULL;
//    return true;
//}

//bool ICQClient::createData(IMContact *&_data, Contact *contact)
//{
//    ICQUserData *data = toICQUserData(_data);
//    ICQUserData *new_data = toICQUserData((SIM::IMContact*)contact->createData(this)); // FIXME unsafe type conversion
//    new_data->setUin(data->getUin());
//    new_data->setScreen(data->getScreen());
//    _data = (IMContact*)new_data;
//    return true;
//}

//OscarSocket::OscarSocket()
//{
//    //m_nFlapSequence = (unsigned short)(rand() & 0x7FFF);
//	m_nFlapSequence = 8984;
//    m_nMsgSequence  = 0;
//}

//void OscarSocket::connect_ready()
//{
//    socket()->readBuffer().init(6);
//    socket()->readBuffer().packetStart();
//    m_bHeader = true;
//}

//void ICQClient::connect_ready()
//{
//	log(L_DEBUG, "ICQClient::connect_ready()");
//    m_bFirstTry = false;
//    if (m_listener == NULL)
//    {
//        m_listener = new ICQListener(this);
//        m_listener->bind(getMinPort(), getMaxPort(), NULL);
//    }
//    m_bNoSend	= false;
//    m_bReady	= true;
//    OscarSocket::connect_ready();
//    TCPClient::connect_ready();
//}

//void ICQClient::setNewLevel(RateInfo &r)
//{
//	QDateTime now = QDateTime::currentDateTime();
//	unsigned delta = 0;
//	if (now.date() == r.m_lastSend.date())
//		delta = r.m_lastSend.time().msecsTo(now.time());
//	unsigned res = (((r.m_winSize - 1) * r.m_curLevel) + delta) / 4 * r.m_winSize;
//	if (res > r.m_maxLevel)
//		res = r.m_maxLevel;
//	r.m_curLevel = res;
//	r.m_lastSend = now;
//	log(L_DEBUG, "Level: %04X [%04X %04X]", res, r.m_minLevel, r.m_winSize);
//}

//RateInfo *ICQClient::rateInfo(unsigned snac)
//{
//    RATE_MAP::iterator it = m_rate_grp.find(snac);
//    if (it == m_rate_grp.end())
//        return NULL;
//    return &m_rates[it->second];
//}

//unsigned ICQClient::delayTime(unsigned snac)
//{
//    RateInfo *r = rateInfo(snac);
//    if (r == NULL)
//        return 0;
//    return delayTime(*r);
//}

//unsigned ICQClient::delayTime(RateInfo &r)
//{
//    if (r.m_winSize == 0)
//        return 0;
//    int res = r.m_minLevel * r.m_winSize - r.m_curLevel * (r.m_winSize - 1);
//    if (res < 0)
//        return 0;
//    QDateTime now = QDateTime::currentDateTime();
//    unsigned delta = 0;
//    if (now.date() == r.m_lastSend.date())
//        delta = r.m_lastSend.time().msecsTo(now.time());
//    res -= delta;
//    return (res > 0) ? res : 0;
//}

//void ICQClient::setStatus(unsigned status, bool bCommon)
//{
//    if (status != STATUS_OFFLINE)
//    {
//        if (status != STATUS_NA && status != STATUS_AWAY)
//            setIdleTime(0);
//        else if (getIdleTime() == 0)
//            setIdleTime(QDateTime::currentDateTime().toTime_t());
//    }
//    TCPClient::setStatus(status, bCommon);
//}

//void ICQClient::setStatus(unsigned status)
//{
//    if (status == STATUS_OFFLINE)
//    {
//        flap(ICQ_CHNxCLOSE);
//        return;
//    }
//    if (m_bAIM)
//    {
//        if (status != STATUS_ONLINE)
//        {
//            m_status = STATUS_AWAY;

//            ar_request req;
//            req.type = 0;
//            req.flags = 0;
//            req.ack = 0;
//            req.id1 = req.id2 = 0;
//            req.bDirect = true;
//            arRequests.push_back(req);

//            ARRequest ar;
//            ar.contact  = NULL;
//            ar.param    = &arRequests.back();
//            ar.receiver = this;
//            ar.status   = status;
//            EventARRequest(&ar).process();
//            EventClientChanged(this).process();
//        }
//        else if (m_status != STATUS_ONLINE)
//        {
//            m_status = STATUS_ONLINE;
//            setAwayMessage();
//            EventClientChanged(this).process();
//        }
//        return;
//    }
//    if (status == m_status)
//        return;

//    m_status = status;
//    snacService()->sendStatus();
//    EventClientChanged(this).process();
//}

//void ICQClient::setInvisible(bool bState)
//{
//    log(L_DEBUG, "ICQClient::setInvisible(): %d", bState);
//    if (bState != getInvisible())
//    {
//        TCPClient::setInvisible(bState);
//        if (getState() == Connected)
//            snacService()->setInvisible();
//        EventClientChanged(this).process();
//    }
//}

//void ICQClient::disconnected()
//{
//    TCPClient::changeStatus(protocol()->status("offline"));
//    m_rates.clear();
//    m_rate_grp.clear();
//    snacICBM()->getSendTimer()->stop();
//    m_processTimer->stop();
//    clearServerRequests();
//    clearListServerRequest();
//    clearSMSQueue();
//    snacICBM()->clearMsgQueue();
//    buddies.clear();
//    Contact *contact;
//    ContactList::ContactIterator it;
//    arRequests.clear();
//    while ((contact = ++it) != NULL)
//    {
//        ICQUserData *data;
//        ClientDataIterator it = contact->clientDataIterator(this);
//        while ((data = toICQUserData(++it)) != NULL)
//        {
//            if ((data->getStatus() != ICQ_STATUS_OFFLINE) || data->getInvisible())
//            {
//                setOffline(data);
//                StatusMessage *m = new StatusMessage();
//                m->setContact(contact->id());
//                m->setClient(dataName(data));
//                m->setStatus(STATUS_OFFLINE);
//                m->setFlags(MESSAGE_RECEIVED);
//                EventMessageReceived e(m);
//                if(e.process())
//                    continue;
//                delete m;
//            }
//        }
//    }
//    for (list<Message*>::iterator itm = m_acceptMsg.begin(); itm != m_acceptMsg.end(); ++itm)
//    {
//        EventMessageDeleted(*itm).process();
//        delete *itm;
//    }
//    m_acceptMsg.clear();
//    m_bRosters = false;
//    m_nMsgSequence = 0;
//    m_bNoSend	= true;
//    m_bReady	= false;
//    m_cookie.resize(0);
//    m_advCounter = 0;
//    m_info_req.clear();
//	if(m_snacService)
//		m_snacService->clearServices();
//    if (m_listener)
//    {
//        delete m_listener;
//        m_listener = NULL;
//    }
//	m_nFlapSequence = 8984;
//}

const char *icq_error_codes[] = {I18N_NOOP("Unknown error"),
                                 I18N_NOOP("Invalid SNAC header"),
                                 I18N_NOOP("Server rate limit exceeded"),
                                 I18N_NOOP("Client rate limit exceeded"),
                                 I18N_NOOP("Recipient is not logged in"),
                                 I18N_NOOP("Requested service unavailable"),
                                 I18N_NOOP("Requested service not defined"),
                                 I18N_NOOP("We sent an obsolete SNAC"),
                                 I18N_NOOP("Not supported by server"),
                                 I18N_NOOP("Not supported by client"),
                                 I18N_NOOP("Refused by client"),
                                 I18N_NOOP("Reply too big"),
                                 I18N_NOOP("Responses lost"),
                                 I18N_NOOP("Request denied"),
                                 I18N_NOOP("Incorrect SNAC format"),
                                 I18N_NOOP("Insufficient rights"),
                                 I18N_NOOP("Recipient blocked"),
                                 I18N_NOOP("Sender too evil"),
                                 I18N_NOOP("Receiver too evil"),
                                 I18N_NOOP("User temporarily unavailable"),
                                 I18N_NOOP("No match"),
                                 I18N_NOOP("List overflow"),
                                 I18N_NOOP("Request ambiguous"),
                                 I18N_NOOP("Server queue full"),
                                 I18N_NOOP("Not while on AOL")};

//const char* ICQClient::error_message(unsigned short error)
//{
//    if (error >= 1 && error <= 0x18)
//        return icq_error_codes[error];
//    return icq_error_codes[0];
//}

//void OscarSocket::packet_ready()
//{
//	unsigned short l_size = 0;
//    if(m_bHeader)
//	{
//        char c;
//        socket()->readBuffer() >> c;
//        if (c != FLAP_START)
//        {
//            log(L_ERROR, "Server send bad packet start code: %02X", c);
//            socket()->error_state(I18N_NOOP("Protocol error"));
//            return;
//        }
//        socket()->readBuffer() >> m_nChannel;
//        unsigned short sequence;
//        socket()->readBuffer() >> sequence >> l_size;
//        m_bHeader = false;
//        if (l_size)
//        {
//            socket()->readBuffer().add(l_size);
//            return;
//        }
//    }
//	l_size = socket()->readBuffer().size() - socket()->readBuffer().readPos();
//    packet(l_size);
//}

//void ICQClient::packet_ready()
//{
//    OscarSocket::packet_ready();
//}

bool ICQClient::isAim() const
{
    return m_bAIM;
}

SnacHandler* ICQClient::snacHandler(int type)
{
    return m_snacHandlers.value(type);
}

void ICQClient::oscarSocketPacket(int channel, const QByteArray& data)
{
    //ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
    //EventLog::log_packet(socket()->readBuffer(), false, plugin->OscarPacket);

    if((channel == ICQ_CHNxNEW) || (channel == ICQ_CHNxCLOSE))
        m_authSnac->handleLoginAndCloseChannels(channel, data);
    else if (channel == ICQ_CHNxDATA)
    {
        ByteArrayParser parser(data);
        int food = parser.readWord();
        int type = parser.readWord();
        int flags = parser.readWord();
        unsigned int requestId = parser.readDword();
//        if (food == ICQ_SNACxFOOD_LOCATION)
//            snac_location(type, seq);
//        else if (food == ICQ_SNACxFOOD_BOS)
//            snac_bos(type, seq);
//        else if (food == ICQ_SNACxFOOD_PING)
//            snac_ping(type, seq);
//        else if (food == ICQ_SNACxFOOD_LISTS)
//            snac_lists(type, seq);
//        else if (food == ICQ_SNACxFOOD_VARIOUS)
//            snac_various(type, seq);
//        else if (food == ICQ_SNACxFOOD_LOGIN)
//            snac_login(type, seq);
//        else
        {
            mapSnacHandlers::iterator it = m_snacHandlers.find(food);
            if (it == m_snacHandlers.end())
                log(L_WARN, "Unknown foodgroup %04X", food);
            else
            {
//                ICQBuffer b;
//                b.resize(size - unknown_length);
//                b.setReadPos(0);
//                b.setWritePos(size - unknown_length);
                QByteArray snacData = parser.readAll();
                it.value()->process(type, snacData, flags, requestId);
            }
        }
    }
    else
        log(L_ERROR, "Unknown channel %u", channel & 0xFF);
//    socket()->readBuffer().init(6);
//    socket()->readBuffer().packetStart();
//    m_bHeader = true;
}

//void OscarSocket::flap(char channel)
//{
//    socket()->writeBuffer().packetStart();
//    socket()->writeBuffer()
//    << (char)0x2A
//    << channel
//    << 0x00000000L;
//}

//void OscarSocket::snac(unsigned short food, unsigned short type, bool msgId, bool bType)
//{
//    flap(ICQ_CHNxDATA);
//    socket()->writeBuffer()
//    << food
//    << type
//    << 0x0000
//    << (bType ? type : (unsigned short)0)
//    << (msgId ? ++m_nMsgSequence : 0x0000);
//}

//void OscarSocket::sendPacket(bool bSend)
//{
//    Buffer &writeBuffer = socket()->writeBuffer();
//    char *packet = writeBuffer.data(writeBuffer.packetStartPos());
//    unsigned size = writeBuffer.size() - writeBuffer.packetStartPos() - 6;
//    packet[4] = (char)((size >> 8) & 0xFF);
//    packet[5] = (char)(size & 0xFF);
//    if (bSend)
//    {
//        packet[2] = (m_nFlapSequence >> 8);
//        packet[3] = m_nFlapSequence;
//        EventLog::log_packet(socket()->writeBuffer(), true, ICQPlugin::icq_plugin->OscarPacket);
//        socket()->write();
//        ++m_nFlapSequence;
//    }
//}

//void ICQClient::sendPacket(bool bSend)
//{
//    Buffer &writeBuffer = socket()->writeBuffer();
//    unsigned char *packet = (unsigned char*)(writeBuffer.data(writeBuffer.readPos()));
//    unsigned long snac = 0;
//    if (writeBuffer.writePos() >= writeBuffer.readPos() + 10)
//        snac = (packet[6] << 24) + (packet[7] << 16) + (packet[8] << 8) + packet[9];
//    unsigned delay = delayTime(snac);
//    if (m_bNoSend)
//        bSend = false;
//    else if (!bSend && (delay == 0))
//        bSend = true;
//    RateInfo *r = rateInfo(snac);
//    if (!r)
//        bSend = true;
//    else if (m_bNoSend || r->delayed.size())
//        bSend = false;
//    if (bSend)
//    {
//        if (r)
//            setNewLevel(*r);
//        OscarSocket::sendPacket(true);
//        return;
//    }
//    OscarSocket::sendPacket(false);
//    r->delayed.pack(writeBuffer.data(writeBuffer.packetStartPos()), writeBuffer.size() - writeBuffer.packetStartPos());
//    writeBuffer.resize(writeBuffer.packetStartPos());
//    m_processTimer->stop();
//    m_processTimer->start(delay);
//}

//unsigned long ICQClient::getFullStatus()
//{
//	return fullStatus(m_status);
//}

//unsigned long ICQClient::fullStatus(const SIM::IMStatusPtr& status)
//{
//    unsigned long code = 0;
//    if(status->id() == "online")
//        code = ICQ_STATUS_ONLINE;
//    else if(status->id() == "away")
//        code = ICQ_STATUS_AWAY;
//    else if(status->id() == "n/a")
//        code = ICQ_STATUS_AWAY | ICQ_STATUS_NA;
//    else if(status->id() == "occupied")
//        code = ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED;
//    else if(status->id() == "dnd")
//        code = ICQ_STATUS_AWAY | ICQ_STATUS_DND | ICQ_STATUS_OCCUPIED;
//    else if(status->id() == "free_for_chat")
//        code = ICQ_STATUS_FFC;

//    if(clientPersistentData->owner.getWebAware())
//        code |= ICQ_STATUS_FxWEBxPRESENCE;
//    if (getHideIP())
//        code |= ICQ_STATUS_FxHIDExIP | ICQ_STATUS_FxDIRECTxAUTH;
//    else if (getDirectMode() == 1)
//        code |= ICQ_STATUS_FxDIRECTxLISTED;
//    else if (getDirectMode() == 2)
//        code |= ICQ_STATUS_FxDIRECTxAUTH;

//    if (m_bBirthday)
//        code |= ICQ_STATUS_FxBIRTHDAY;
//    if (getInvisible())
//    {
//        code |= ICQ_STATUS_FxPRIVATE | ICQ_STATUS_FxHIDExIP;
//        code &= ~(ICQ_STATUS_FxDIRECTxLISTED | ICQ_STATUS_FxDIRECTxAUTH);
//    }
//    return code;
//}

//unsigned long ICQClient::fullStatus(unsigned s)
//{
//    unsigned long status = 0;
//    switch (s)
//	{
//    case STATUS_ONLINE:
//        status = ICQ_STATUS_ONLINE;
//        break;
//    case STATUS_AWAY:
//        status = ICQ_STATUS_AWAY;
//        break;
//    case STATUS_NA:
//        status = ICQ_STATUS_NA | ICQ_STATUS_AWAY;
//        break;
//    case STATUS_OCCUPIED:
//        status = ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
//        break;
//    case STATUS_DND:
//        status = ICQ_STATUS_DND | ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
//        break;
//    case STATUS_FFC:
//        status = ICQ_STATUS_FFC;
//        break;
//    }
//    if(clientPersistentData->owner.getWebAware())
//        status |= ICQ_STATUS_FxWEBxPRESENCE;
//    if (getHideIP())
//        status |= ICQ_STATUS_FxHIDExIP | ICQ_STATUS_FxDIRECTxAUTH;
//    else if (getDirectMode() == 1)
//        status |= ICQ_STATUS_FxDIRECTxLISTED;
//    else if (getDirectMode() == 2)
//        status |= ICQ_STATUS_FxDIRECTxAUTH;

//    if (m_bBirthday)
//        status |= ICQ_STATUS_FxBIRTHDAY;
//    if (getInvisible())
//    {
//        status |= ICQ_STATUS_FxPRIVATE | ICQ_STATUS_FxHIDExIP;
//        status &= ~(ICQ_STATUS_FxDIRECTxLISTED | ICQ_STATUS_FxDIRECTxAUTH);
//    }
//    return status;
//}

//void ICQClient::interfaceDown(QString ifname)
//{
//        log(L_DEBUG, "icq: interface down: %s", qPrintable(ifname));
//}

//void ICQClient::interfaceUp(QString ifname)
//{
//	if(getMediaSense())
//	{
//        log(L_DEBUG, "icq: interface up: %s", qPrintable(ifname));
//        if(!m_bconnectionLost)
//            return;

//        // Try to connect
//        setStatus(STATUS_ONLINE, false);
//	}
//}

//ICQUserData *ICQClient::findContact(unsigned long l, const QString *alias, bool bCreate, Contact *&contact, Group *grp, bool bJoin)
//{
//    return findContact(QString::number(l), alias, bCreate, contact, grp, bJoin);
//}

//ICQUserData *ICQClient::findContact(const QString &screen, const QString *alias, bool bCreate, Contact *&contact, Group *grp, bool bJoin)
//{
//    if (screen.isEmpty())
//        return NULL;

//    QString s = screen.toLower();

//    ContactList::ContactIterator it;
//    ICQUserData *data;
//    unsigned long uin = screen.toULong();

//    while ((contact = ++it) != NULL)
//    {
//        ClientDataIterator it = contact->clientDataIterator(this);
//        while ((data = toICQUserData(++it)) != NULL)
//        {
//            if (uin && data->getUin() != uin || (uin == 0 && s != data->getScreen()))
//                continue;
//            bool bChanged = false;
//            if (alias)
//            {
//                if (!alias->isEmpty())
//				{
//					bChanged = contact->getName() != *alias;
//					if(bChanged)
//						contact->setName(*alias);
//				}
//                data->setAlias(*alias);
//            }
//            if (grp && contact->getGroup() != (int)grp->id())
//            {
//                contact->setGroup(grp->id());
//                bChanged = true;
//            }
//            if (bChanged)
//            {
//                EventContact e(contact, EventContact::eChanged);
//                e.process();
//            }
//            return data;
//        }
//    }
//    if (!bCreate)
//        return NULL;
//    if (bJoin)
//    {
//        for (unsigned i = 0; i < getContacts()->nClients(); i++)
//        {
//            Client *client = getContacts()->getClient(i);
//            if (client == this || client->protocol() != protocol())
//                continue;
//            ICQClient *c = static_cast<ICQClient*>(client);
//            it.reset();
//            while ((contact = ++it) != NULL)
//            {
//                ClientDataIterator it = contact->clientDataIterator(c);
//                while ((data = toICQUserData(++it)) != NULL)
//                {
//                    if (uin && data->getUin() != uin || (uin == 0 && s != data->getScreen()))
//                        continue;
//                    data = toICQUserData((SIM::IMContact*)contact->createData(this)); // FIXME unsafe type conversion
//                    data->setUin(uin);
//                    if (uin == 0)
//                        data->setScreen(s);
//                    bool bChanged = false;
//                    if(alias)
//					{
//                        if(!alias->isEmpty())
//						{
//							bChanged = contact->getName() != *alias;
//							if(bChanged)
//								contact->setName(*alias);
//						}
//                        data->setAlias(*alias);
//                    }
//                    if (grp && (int)grp->id() != contact->getGroup())
//                    {
//                        contact->setGroup(grp->id());
//                        bChanged = true;
//                    }
//                    if (bChanged)
//                    {
//                        EventContact e(contact, EventContact::eChanged);
//                        e.process();
//                        updateInfo(contact, data);
//                    }
//                    updateInfo(contact, data);
//                    return data;
//                }
//            }
//        }
//        if (alias && !alias->isEmpty())
//        {
//            QString name = alias->toLower();
//            it.reset();
//            while ((contact = ++it) != NULL)
//                if (contact->getName().toLower() == name){
//                    ICQUserData *data = toICQUserData((SIM::IMContact*) contact->createData(this)); // FIXME unsafe type conversion
//                    data->setUin(uin);
//                    if (uin == 0)
//                        data->getScreen() = screen;
//                    data->setAlias(alias ? *alias : QString::null);
//                    EventContact e(contact, EventContact::eChanged);
//                    e.process();
//                    m_bJoin = true;
//                    updateInfo(contact, data);
//                    return data;
//                }
//        }
//    }
//    contact = getContacts()->contact(0, true);
//    data = toICQUserData((SIM::IMContact*) contact->createData(this)); // FIXME unsafe type conversion
//    data->setUin(uin);
//    if (uin == 0)
//        data->getScreen() = s;
//    QString name;
//    if (alias)
//        name = *alias;
//    else if (uin)
//        name = QString::number(uin);
//    else
//        name = screen;
//    if(alias)
//        data->setAlias(*alias);
//    contact->setName(name);
//    if (grp)
//        contact->setGroup(grp->id());
//    EventContact e(contact, EventContact::eChanged);
//    e.process();
//    updateInfo(contact, data);
//    return data;
//}

//ICQUserData *ICQClient::findGroup(unsigned id, const QString *alias, Group *&grp)
//{
//    ContactList::GroupIterator it;
//    ICQUserData *data;
//    while ((grp = ++it) != NULL)
//    {
//        data = toICQUserData((SIM::IMContact*)grp->getData(this)); // FIXME unsafe type conversion
//        if (!data || data->getIcqID() != id)
//            continue;

//        if (alias)
//            data->setAlias(*alias);
//        return data;
//    }
//    if (alias == NULL)
//        return NULL;
//    it.reset();
//    QString name = *alias;
//    while ((grp = ++it) != NULL)
//    {
//        if (grp->getName() == name)
//        {
//            data = toICQUserData((SIM::IMContact*)grp->createData(this)); // FIXME unsafe type conversion
//            data->setIcqID(id);
//            data->setAlias(*alias);
//            return data;
//        }
//    }
//    grp = getContacts()->group(0, true);
//    grp->setName(name);
//    data = toICQUserData((SIM::IMContact*)grp->createData(this)); // FIXME unsafe type conversion
//    data->setIcqID(id);
//    data->setAlias(*alias);
//    EventGroup e(grp, EventGroup::eChanged);
//    e.process();
//    return data;
//}

//void ICQClient::setOffline(ICQUserData *data)
//{
//    QString name = dataName(data);
//    for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); )
//    {
//        Message *msg = *it; //will sometimes not work, content: it is broken then:	0xcdcdcdcd, reason seems to be Filetransfer.. however..

//        if(!msg->client().isEmpty() && name == msg->client())
//        {
//            EventMessageDeleted(msg).process();
//            delete msg;
//            m_acceptMsg.erase(it);
//            it = m_acceptMsg.begin();
//        }
//        ++it;  //FIXME: Exception: Client-Operator not incrementable, because variable "it" is broken at this position, anyhow
//    }
//    if (data->getDirect())
//    {
//        delete data->getDirect();
//        data->setDirect(0);
//    }
//    if (data->getDirectPluginInfo())
//    {
//        delete data->getDirectPluginInfo();
//        data->setDirectPluginInfo(0);
//    }
//    if (data->getDirectPluginStatus())
//    {
//        delete data->getDirectPluginStatus();
//        data->setDirectPluginStatus(0);
//    }
//    data->setNoDirect(false);
//    data->setStatus(ICQ_STATUS_OFFLINE);
//    data->setClass(0);
//    data->setTyping(false);
//    data->setBadClient(false);
//    data->setInvisible(false);
//    data->setStatusTime(QDateTime::currentDateTime().toTime_t());
//    data->setAutoReply(QString::null);
//}

//static void addIcon(QSet<QString> *s, const QString &icon, const QString &statusIcon)
//{
//    if (!s || statusIcon == icon)
//        return;
//    s->insert(icon);
//}

//void ICQClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QSet<QString> *icons)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    unsigned status = STATUS_ONLINE;
//    unsigned client_status = data->getStatus();
//    if (client_status == ICQ_STATUS_OFFLINE)
//        status = STATUS_OFFLINE;
//    else if (client_status & ICQ_STATUS_DND)
//        status = STATUS_DND;
//    else if (client_status & ICQ_STATUS_OCCUPIED)
//        status = STATUS_OCCUPIED;
//    else if (client_status & ICQ_STATUS_NA)
//        status = STATUS_NA;
//    else if (client_status & ICQ_STATUS_AWAY)
//        status = STATUS_AWAY;
//    else if (client_status & ICQ_STATUS_FFC)
//        status = STATUS_FFC;
//    unsigned iconStatus = status;
//    QString dicon;
//    if (data->getUin())
//        if (!(iconStatus == STATUS_ONLINE && client_status & ICQ_STATUS_FxPRIVATE))
//        {
//            const CommandDef *def = ICQProtocol::_statusList();
//            for (; !def->text.isEmpty(); def++)
//            {
//                if (def->id != iconStatus)
//                    continue;
//                dicon = def->icon;
//                break;
//            }
//        }
//        else dicon = "ICQ_invisible";
//    else
//    {
//        if (status != STATUS_OFFLINE)
//        {
//            status = STATUS_ONLINE;
//            dicon = "AIM_online";
//            if (data->getClass() & CLASS_AWAY)
//            {
//                status = STATUS_AWAY;
//                dicon = "AIM_away";
//            }
//        }
//        else
//            dicon = "AIM_offline";
//    }
//    if(dicon.isEmpty())
//        return;
//    if (status == STATUS_OCCUPIED)
//        status = STATUS_DND;
//    if (status == STATUS_FFC)
//        status = STATUS_ONLINE;
//    if (status > curStatus)
//    {
//        curStatus = status;
//        if (!statusIcon.isEmpty() && icons)
//            icons->insert(statusIcon);
//        statusIcon = dicon;
//    }
//    else if (!statusIcon.isEmpty())
//        addIcon(icons, dicon, statusIcon);
//    else
//        statusIcon = dicon;
//    if (status == STATUS_OFFLINE && data->getInvisible())
//    {
//        status = STATUS_INVISIBLE;
//        if (status > curStatus)
//            curStatus = status;
//    }
//    if (icons)
//    {
//        if ((iconStatus != STATUS_ONLINE && iconStatus != STATUS_OFFLINE && client_status & ICQ_STATUS_FxPRIVATE) || data->getInvisible())
//            addIcon(icons, "ICQ_invisible", statusIcon);
//        if (data->getStatus() & ICQ_STATUS_FxBIRTHDAY) {
//			QDate today=QDate::currentDate();
//            if (today.day()==(int)data->getBirthDay() && today.month()==(int)data->getBirthMonth())
//                addIcon(icons, "partytime", statusIcon);
//            else
//                addIcon(icons, "birthday", statusIcon);
//		}
//        if (data->getFollowMe() == 1)
//            addIcon(icons, "phone", statusIcon);
//        if (data->getFollowMe() == 2)
//            addIcon(icons, "nophone", statusIcon);
//        if (status != STATUS_OFFLINE)
//        {
//            if (data->getSharedFiles())
//                addIcon(icons, "sharedfiles", statusIcon);
//            if (data->getICQPhone() == 1)
//                addIcon(icons, "icqphone", statusIcon);
//            if (data->getICQPhone() == 2)
//                addIcon(icons, "icqphonebusy", statusIcon);
//        }
//        if (data->getTyping())
//            addIcon(icons, "typing", statusIcon);
//        DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//        if (dc && dc->isSecure())
//            addIcon(icons, "encrypted", statusIcon);
//    }
//    if (data->getInvisibleId())
//        style |= CONTACT_STRIKEOUT;
//    if (data->getVisibleId())
//        style |= CONTACT_ITALIC;
//    if (data->getWaitAuth())
//        style |= CONTACT_UNDERLINE;
//}

//void ICQClient::ping()
//{
//    if (getState() == Connected)
//    {
//        bool bBirthday = false;
//        if (!m_bAIM)
//        {
//            int year  = clientPersistentData->owner.getBirthYear();
//            int month = clientPersistentData->owner.getBirthMonth();
//            int day   = clientPersistentData->owner.getBirthDay();
//            if (day && month && year)
//            {
//				QDate tNow = QDate::currentDate();
//				QDate tBirthday(tNow.year(), month, day);
//                // we send it two days before we've birthday
//				int diff = tNow.daysTo(tBirthday);
//                if (diff < 0 || diff > 2)
//                {
//                    tBirthday = tBirthday.addYears(1);
//                    diff = tNow.daysTo(tBirthday);
//                    if(diff >= 0 && diff <=2)
//                        bBirthday = true;
//                }
//                else bBirthday = true;
//            }
//        }
//        if (bBirthday != m_bBirthday)
//        {
//            m_bBirthday = bBirthday;
//            setStatus(m_status);
//        }
//        else if (getKeepAlive() || m_bHTTP)
//        {
//            bool bSend = true;
//            for (unsigned i = 0; i < m_rates.size(); i++)
//            {
//                if (!m_rates[i].delayed.size())
//                    continue;
//                bSend = false;
//                break;
//            }
//            if (bSend)
//            {
//                flap(ICQ_CHNxPING);
//                sendPacket(false);
//            }
//        }
//        snacICBM()->processSendQueue();
//        checkListRequest();
//        checkInfoRequest();
//        QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
//    }
//}

//void ICQClient::setupContact(Contact *contact, void *_data)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    QString phones;
//    if (!data->getHomePhone().isEmpty())
//    {
//        phones += trimPhone(data->getHomePhone());
//        phones += ",Home Phone,";
//        phones += QString::number(PHONE);
//    }
//    if (!data->getHomeFax().isEmpty())
//    {
//        if (phones.length())
//            phones += ';';
//        phones += trimPhone(data->getHomeFax());
//        phones += ",Home Fax,";
//        phones += QString::number(FAX);
//    }
//    if (!data->getWorkPhone().isEmpty())
//    {
//        if (phones.length())
//            phones += ';';
//        phones += trimPhone(data->getWorkPhone());
//        phones += ",Work Phone,";
//        phones += QString::number(PHONE);
//    }
//    if (!data->getWorkFax().isEmpty())
//    {
//        if (phones.length())
//            phones += ';';
//        phones += trimPhone(data->getWorkFax());
//        phones += ",Work Fax,";
//        phones += QString::number(FAX);
//    }
//    if (!data->getPrivateCellular().isEmpty())
//    {
//        if (phones.length())
//            phones += ';';
//        phones += trimPhone(data->getPrivateCellular());
//        phones += ",Private Cellular,";
//        phones += QString::number(CELLULAR);
//    }
//    if(data->getPhoneBook().isEmpty())
//    {
//        if (phones.length())
//            phones += ';';
//        phones += data->getPhoneBook();
//    }
//    contact->setPhones(phones, name());
//    QString mails;
//    if (!data->getEmail().isEmpty())
//    {
//        mails += data->getEmail().trimmed();
//        QString emails = data->getEMails();
//        while (emails.length())
//        {
//            QString mailItem = getToken(emails, ';', false);
//            QString mail = getToken(mailItem, '/').trimmed();
//            if (mail.length())
//            {
//                if (mails.length())
//                    mails += ';';
//                mails += mail;
//            }
//        }
//    }
//    QString n = name();
//    contact->setEMails(mails, n);
//    QString firstName = data->getFirstName();
//    if (firstName.length())
//        contact->setFirstName(firstName, n);
//    QString lastName = data->getLastName();
//    if (lastName.length())
//        contact->setLastName(lastName, n);
//    if (contact->getName().isEmpty())
//        contact->setName(QString::number(data->getUin()));
//    QString nick = data->getNick();
//    if (nick.isEmpty())
//        nick = data->getAlias();
//    if (!nick.isEmpty())
//    {
//        QString name = QString::number(data->getUin());
//        if (name == contact->getName())
//            contact->setName(nick);
//    }
//}

//QString ICQClient::trimPhone(const QString &from)
//{
//    QString res;
//    if (from.isEmpty())
//        return res;
//    res = from;
//    int idx = res.indexOf("SMS");
//    if(idx != -1)
//        res = res.left(idx);
//    return res.trimmed();
//}

//QString ICQClient::contactTip(void *_data)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    QString res;
//    QString statusText;
//    unsigned long status = STATUS_OFFLINE;
//    unsigned style  = 0;
//    QString statusIcon;
//    contactInfo(data, status, style, statusIcon);
//    if (status == STATUS_INVISIBLE)
//    {
//        res += "<img src=\"sim:icons/ICQ_invisible\">";
//        res += i18n("Possibly invisible");
//    }
//    else
//    {
//        res += "<img src=\"sim:icons/";
//        res += statusIcon;
//        res += "\">";
//        if (statusIcon == "ICQ_invisible")
//        {
//            res += ' ';
//            res += i18n("Invisible");
//        }
//        else if (data->getUin())
//            for (const CommandDef *cmd = ICQProtocol::_statusList(); !cmd->text.isEmpty(); cmd++)
//            {
//                if (cmd->icon == statusIcon)
//                {
//                    res += ' ';
//                    statusText += i18n(cmd->text);
//                    res += statusText;
//                    break;
//                }
//            }
//        else
//        {
//            if (status == STATUS_OFFLINE)
//                res += i18n("Offline");
//            else if (status == STATUS_ONLINE)
//                res += i18n("Online");
//            else
//                res += i18n("Away");
//        }
//    }
//    res += "<br/>";
//    if (data->getUin())
//    {
//        res += "UIN: <b>";
//        res += QString::number(data->getUin());
//        res += "</b>";
//    }else{
//        res += "<b>";
//        res += data->getScreen();
//        res += "</b>";
//    }
//	if (data->getWarningLevel())
//    {
//        res += "<br/>";
//        res += i18n("Warning level");
//        res += ": <b>";
//		res += QString::number(warnLevel(data->getWarningLevel()));
//        res += "% </b></br>";
//    }
//    if (data->getStatus() == ICQ_STATUS_OFFLINE)
//    {
//        if (data->getStatusTime()){
//            res += "<br/><font size=-1>";
//            res += i18n("Last online");
//            res += ": </font>";
//            res += formatDateTime(data->getStatusTime());
//        }
//    }
//    else
//    {
//		if (data->getOnlineTime())
//        {
//            res += "<br/><font size=-1>";
//            res += i18n("Online");
//            res += ": </font>";
//			res += formatDateTime(data->getOnlineTime());
//        }
//        if (data->getStatus() & (ICQ_STATUS_AWAY | ICQ_STATUS_NA)){
//            res += "<br/><font size=-1>";
//            res += statusText;
//            res += ": </font>";
//            res += formatDateTime(data->getStatusTime());
//        }
//    }
//    if (data->getIP())
//    {
//        res += "<br/>";
//        res += formatAddr(data->getIP(), data->getPort());
//    }
//    if ((data->getRealIP()) && ((data->getIP() == 0) || ((data->getIP()) != (data->getRealIP()))))
//    {
//        res += "<br/>";
//        res += formatAddr(data->getRealIP(), data->getPort());
//    }
//    QString client_name = clientName(data);
//    if (client_name.length())
//    {
//        res += "<br/>";
//        res += quoteString(client_name);
//    }
//    QString pictureFileName = pictureFile(data);
//    QImage img(pictureFileName);
//    if (!img.isNull())
//    {
//        int w = img.width();
//        int h = img.height();
//        if (h > w)
//        {
//            if (h > 60)
//            {
//                w = w * 60 / h;
//                h = 60;
//            }
//        }
//        else if (w > 60)
//        {
//            h = h * 60 / w;
//            w = 60;
//        }
//        res += "<br/><img src=\"" + pictureFileName + "\" width=\"";
//        res += QString::number(w);
//        res += "\" height=\"";
//        res += QString::number(h);
//        res += "\">";
//    }
//    if (!data->getAutoReply().isEmpty())
//    {
//        res += "<br/><br/>";
//        res += quoteString(data->getAutoReply());
//    }
//    if (!(data->getStatus() & ICQ_STATUS_FxBIRTHDAY))
//        return res;

//    QDate today=QDate::currentDate();
//    if (today.day()==(int)data->getBirthDay() && today.month()==(int)data->getBirthMonth())
//    {
//        //Today is birthday!
//        //addIcon(icons, "partytime", statusIcon);
//        res += "<br/><br/><b>"+i18n("has birthday <font color='red'>today</font>!")+"</b><br/>";
//        if (!m_bBirthdayInfoDisplayed)
//        {
//            int ret=QMessageBox::question(0,
//                i18n("Birthday Notification"),
//                QString("%1 (%2 %3) %4\n\n%5").arg(data->getAlias(), data->getFirstName(), data->getLastName(), i18n("has birthday today!"), i18n("Send GreetingCard?")),
//                QMessageBox::Yes | QMessageBox::No);
//            m_bBirthdayInfoDisplayed=true;
//            //Todo: navigate to birthday greetingcard-webpage ;)
//            EventGoURL e(QString("http://www.google.com/search?q=ecards"));
//            if (ret==QMessageBox::Yes) e.process();
//        }

//    }
//    else
//    {
//        //Birthday one or two more days.
//        //addIcon(icons, "birthday", statusIcon);
//        int nextbirthdayyear=today.year();
//        if ((int)data->getBirthMonth()==1 && (int)data->getBirthDay()<2) //special case
//            nextbirthdayyear=today.year()+1;

//        QDate birthday(nextbirthdayyear,(int)data->getBirthMonth(),(int)data->getBirthDay());
//        int remainingdays=today.daysTo(birthday);
//        res += QString("<br/><br/><b>"+i18n("has birthday in <font color='red'>%1</font> days.").arg(QString::number(remainingdays))+"</b><br/>");
//    }
//    return res;
//}

//unsigned long ICQClient::warnLevel(unsigned long level)
//{
//    level = ((level + 5) / 10);
//    if (level > 100)
//        level = 100;
//    return level;
//}

//bool ICQClient::hasCap(const ICQUserData *data, cap_id_t n)
//{
//    unsigned long val = n > 31 ? data->getCaps2() : data->getCaps();
//    int pos = (int)n % 32;
//    return (val & (1 << pos)) != 0;
//}

//void ICQClient::setCap(ICQUserData *data, cap_id_t n)
//{
//    unsigned long val = n > 31 ? data->getCaps2() : data->getCaps();
//    int pos = (int)n % 32;
//    val |= (1 << pos);
//    if(n > 31)
//    {
//        data->setCaps2(val);
//    }
//    else
//        data->setCaps(val);
//}

//static QString verString(unsigned ver)
//{
//    QString res;
//    if (ver == 0)
//        return res;
//    unsigned char v[4];
//    v[0] = (unsigned char)((ver >> 24) & 0xFF);
//    v[1] = (unsigned char)((ver >> 16) & 0xFF);
//    v[2] = (unsigned char)((ver >>  8) & 0xFF);
//    v[3] = (unsigned char)((ver >>  0) & 0xFF);
//    if ((v[0] & 0x80) || (v[1] & 0x80) || (v[2] & 0x80) || (v[3] & 0x80))
//        return res;

//    res.sprintf(" %u.%u", v[0], v[1]);
//    if (v[2] || v[3])
//    {
//        QString s;
//        s.sprintf(".%u", v[2]);
//        res += s;
//    }
//    if (v[3])
//    {
//        QString s;
//        s.sprintf(".%u", v[3]);
//        res += s;
//    }
//    return res;
//}

//QString ICQClient::clientName(ICQUserData *data)
//{
//    QString res;
//    if (data->getVersion())
//        res.sprintf("v%lu ", data->getVersion());

//    if (data->getInfoUpdateTime() == 0xFFFFFFFFL)
//    {
//        if (data->getPluginStatusTime() == 0xFFFFFFFFL && data->getPluginInfoTime() == 0xFFFFFFFFL)
//        {
//            res += "GAIM";
//            return res;
//        }
//        res += "MIRANDA";
//        res += hasCap(data, CAP_ICQJP) ? verString(data->getBuild()) : verString(data->getPluginInfoTime() & 0xFFFFFF);
//        if (!(data->getPluginInfoTime() & 0x80000000))
//            return res;

//        res += " alpha";
//        return res;
//    }
//    if (data->getInfoUpdateTime() == 0xFFFFFF8FL)
//    {
//        res += "StrICQ";
//        res += verString(data->getPluginInfoTime() & 0xFFFFFF);
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0xFFFFFF42L)
//    {
//        res += "mICQ";
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0xFFFFFFBEL)
//    {
//        res += "alicq";
//        res += verString(data->getPluginInfoTime() & 0xFFFFFF);
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0xFFFFFF7FL)
//    {
//        res += "&RQ";
//        res += verString(data->getPluginInfoTime() & 0xFFFFFF);
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0xFFFFFFABL)
//    {
//        res += "YSM";
//        res += verString(data->getPluginInfoTime() & 0xFFFF);
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0x04031980L)
//    {
//        QString r;
//        r.sprintf("vICQ 0.43.%lu.%lu", data->getPluginInfoTime() & 0xffff, data->getPluginInfoTime() & (0x7fff0000) >> 16);
//        res += r;
//        return res;
//    }
//    else if (data->getInfoUpdateTime() == 0x3AA773EEL && data->getPluginStatusTime() == 0x3AA66380L && data->getPluginInfoTime() == 0x3A877A42L)
//    {
//        res += "libicq2000";
//        return res;
//    }

//    if (hasCap(data, CAP_MIRANDA))
//    {
//        QString r;
//        unsigned ver1 = (data->getBuild() >> 24) & 0x7F;
//        unsigned ver2 = (data->getBuild() >> 16) & 0xFF;
//        unsigned ver3 = (data->getBuild() >>  8) & 0xFF;
//        unsigned ver4 = (data->getBuild() >>  0) & 0xFF;
//        r.sprintf("Miranda %u.%u.%u.%u", ver1, ver2, ver3, ver4);
//        // highest bit set -> alpha version
//        if(((data->getBuild() >> 24) & 0x80) != 0x80)
//            return res + r;

//        r += " (alpha)";
//        return res + r;
//    }
//    if (hasCap(data, CAP_QIP))
//    {
//        QString r;
//        r.sprintf("QIP 2005a %lu%lu%lu%lu",
//            (data->getInfoUpdateTime() >> 24) & 0xFF,
//            (data->getInfoUpdateTime() >> 16) & 0xFF,
//            (data->getInfoUpdateTime() >>  8) & 0xFF,
//            (data->getInfoUpdateTime() >>  0) & 0xFF);
//        return res + r;
//    }
//    if (hasCap(data, CAP_INFIUM))
//    {
//        QString r;
//        r.sprintf("QIP Infium %lu", data->getInfoUpdateTime());
//        return res + r;
//    }
//    if (hasCap(data, CAP_QIP2010))
//    {
//        QString r;
//        r.sprintf("QIP 2010 %lu", data->getInfoUpdateTime());
//        return res + r;
//    }
//    if (hasCap(data, CAP_JIMM))
//    {
//        QString r;
//        unsigned maj = (data->getBuild() >> 24) & 0xFF;
//        unsigned min = (data->getBuild() >> 16) & 0xFF;
//        unsigned rev = (data->getBuild() >>  0) & 0xFFFF;
//        if(rev)
//            r.sprintf("Jimm %d.%d.%d", maj, min, rev);
//        else
//            r.sprintf("Jimm %d.%d", maj, min);
//        return res + r;
//    }

//    if (hasCap(data, CAP_SIMOLD))
//    {
//        QString r;
//        unsigned hiVersion = (data->getBuild() >> 6) - 1;
//        unsigned loVersion = data->getBuild() & 0x1F;
//        r.sprintf("SIM %u.%u", hiVersion, loVersion);
//        return res + r;
//    }

//    if (hasCap(data, CAP_SIM))
//    {
//        QString r;
//        unsigned ver1 = (data->getBuild() >> 24) & 0xFF;
//        unsigned ver2 = (data->getBuild() >> 16) & 0xFF;
//        unsigned ver3 = (data->getBuild() >> 8) & 0xFF;
//        unsigned ver4 = data->getBuild() & 0x0F;
//        if (ver4)
//            r.sprintf("SIM %u.%u.%u.%u", ver1, ver2, ver3, ver4);
//        else if (ver3)
//            r.sprintf("SIM %u.%u.%u", ver1, ver2, ver3);
//        else
//            r.sprintf("SIM %u.%u", ver1, ver2);
//        res += r;
//        if (data->getBuild() & 0x80)
//            res += "/win32";

//        if (data->getBuild() & 0x40)
//            res += "/MacOS X";
//        return res;
//    }

//    if (9 == data->getVersion())
//    {
//        if (hasCap(data, CAP_ICQ51)) // CAP_TZERS
//        {
//            if (   hasCap(data, CAP_LITE_NEW)
//                && hasCap(data, CAP_HOST_STATUS_TEXT_AWARE)
//                && hasCap(data, CAP_AIM_LIVE_VIDEO)
//                && hasCap(data, CAP_AIM_LIVE_AUDIO) )
//            {
//                res += "ICQ 7";
//                return res;
//            }
//            else
//            {
//                res += hasCap(data, CAP_HTMLMSGS) && hasCap(data, CAP_AIM_LIVE_VIDEO) && hasCap(data, CAP_AIM_LIVE_AUDIO) ? "ICQ 6" : "ICQ 5.1";
//                return res;
//            }
//        }
//        else if (hasCap(data, CAP_ICQ5_1)
//              && hasCap(data, CAP_ICQ5_3)
//              && hasCap(data, CAP_ICQ5_4))
//        {
//            res += "ICQ 5.0";
//            return res;
//        }
//        if (hasCap(data, CAP_ABV))
//        {
//            res += " Abv";
//            return res;
//        }
//        else if (hasCap(data, CAP_NETVIGATOR))
//        {
//            res += " Netvigator";
//            return res;
//        }
//        else if (hasCap(data, CAP_RAMBLER))
//        {
//            res += " Rambler";
//            return res;
//        }
//    }
//    if (hasCap(data, CAP_ICQ5_1))
//        log( L_DEBUG, "CAP_ICQ5_1 without all others" );
//    if (hasCap(data, CAP_ICQ5_3))
//        log( L_DEBUG, "CAP_ICQ5_3 without all others" );
//    if (hasCap(data, CAP_ICQ5_4))
//        log( L_DEBUG, "CAP_ICQ5_4 without all others" );
//    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
//    {
//        res += "Trillian";
//        return res;
//    }



//    if (hasCap(data, CAP_LICQ))
//    {
//        QString r;
//        unsigned ver1 = (data->getBuild() >> 24) & 0xFF;
//        unsigned ver2 = (data->getBuild() >> 16) & 0xFF;
//        unsigned ver3 = (data->getBuild() >> 8) & 0xFF;
//        ver2 %=100;	// see licq source
//        r.sprintf("Licq %u.%u.%u", ver1, ver2, ver3);
//        res += r;
//        if ((data->getBuild() & 0xFF) == 1)
//            res += "/SSL";
//        return res;
//    }
//    if (hasCap(data, CAP_KOPETE))
//    {
//        // last 4 bytes determine version
//        // NOTE change with each Kopete Release!
//        // first number, major version
//        // second number,  minor version
//        // third number, point version 100+
//        // fourth number,  point version 0-99
//        QString r;
//        unsigned ver1 =  (data->getBuild() >> 24) & 0xFF;	// major
//        unsigned ver2 =  (data->getBuild() >> 16) & 0xFF;	// minor
//        unsigned ver3 = ((data->getBuild() >>  8) & 0xFF) * 100;
//        ver3         +=  (data->getBuild() >>  0) & 0xFF;
//        r.sprintf("Kopete %u.%u.%u", ver1, ver2, ver3);
//        res += r;
//        return res;
//    }
//    if (hasCap(data, CAP_XTRAZ))
//    {
//        res += "ICQ 4.0 Lite";
//        return res;
//    }
//    if (hasCap(data, CAP_MACICQ))
//    {
//        res += "ICQ for Mac";
//        return res;
//    }
//    //  gaim 2.0
//    if (hasCap(data, CAP_AIM_SENDFILE) &&
//        hasCap(data, CAP_AIM_IMIMAGE) &&
//        hasCap(data, CAP_AIM_BUDDYCON) &&
//        hasCap(data, CAP_UTF) &&
//        hasCap(data, CAP_AIM_CHAT))
//    {
//            res += "gaim 2.0";
//            return res;
//    }
//    if (hasCap(data, CAP_AIM_CHAT))
//    {
//        res += "AIM";
//        return res;
//    }
//    if ((data->getInfoUpdateTime() & 0xFF7F0000L) == 0x7D000000L)
//    {
//        QString r;
//        unsigned ver = data->getInfoUpdateTime() & 0xFFFF;
//        if (ver % 10)
//            r.sprintf("Licq %u.%u.%u", ver / 1000, (ver / 10) % 100, ver % 10);
//        else
//            r.sprintf("Licq %u.%u", ver / 1000, (ver / 10) % 100);
//        res += r;
//        if (data->getInfoUpdateTime() & 0x00800000L)
//            res += "/SSL";
//        return res;
//    }


//    if (hasCap(data, CAP_TYPING))
//    {
//        if (data->getVersion() == 10)
//            res += "ICQ 2003b";
//        else if (data->getVersion() == 9)
//            res += "ICQ Lite";
//        else
//            res += "ICQ2go";
//        return res;
//    }
//    if (data->getInfoUpdateTime() &&
//        (data->getInfoUpdateTime() == data->getPluginStatusTime()) &&
//        (data->getPluginStatusTime() == data->getPluginInfoTime()) &&
//        (data->getCaps() == 0) && (data->getCaps2() == 0)){
//            res += "vICQ";
//            return res;
//    }
//    if (hasCap(data, CAP_AIM_BUDDYCON))
//    {
//        res += "gaim";
//        return res;
//    }
//    if ((hasCap(data, CAP_STR_2001) || hasCap(data, CAP_SRV_RELAY)) && hasCap(data, CAP_IS_2001))
//    {
//        res += "ICQ 2001";
//        return res;
//    }
//    if ((hasCap(data, CAP_STR_2001) || hasCap(data, CAP_SRV_RELAY)) && hasCap(data, CAP_IS_2002))
//    {
//        res += "ICQ 2002";
//        return res;
//    }
//    if (hasCap(data, CAP_RTF) && hasCap(data, CAP_UTF) &&
//        hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_DIRECT))
//    {
//            res += "ICQ 2003a";
//            return res;
//    }
//    if (hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_DIRECT))
//    {
//        res += "ICQ 2001b";
//        return res;
//    }
//    if ((data->getVersion() == 7) && hasCap(data, CAP_RTF))
//    {
//        res += "GnomeICU";
//        return res;
//    }
//    // ICQ2go doesn't use CAP_TYPING anymore
//    if ((data->getVersion() == 7) && hasCap(data, CAP_UTF))
//    {
//        res += "ICQ2go";
//        return res;
//    }
//    return res;
//}

//const unsigned MAIN_INFO = 1;
//const unsigned HOME_INFO = 2;
//const unsigned WORK_INFO = 3;
//const unsigned MORE_INFO = 4;
//const unsigned ABOUT_INFO = 5;
//const unsigned INTERESTS_INFO = 6;
//const unsigned PAST_INFO = 7;
//const unsigned PICTURE_INFO = 8;
//const unsigned NETWORK   = 9;
//const unsigned SECURITY  = 10;

//static CommandDef icqWnd[] =
//    {
//        CommandDef (
//            MAIN_INFO,
//            " ",
//            "ICQ_online",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            HOME_INFO,
//            I18N_NOOP("Home info"),
//            "home",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            WORK_INFO,
//            I18N_NOOP("Work info"),
//            "work",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            MORE_INFO,
//            I18N_NOOP("More info"),
//            "more",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            ABOUT_INFO,
//            I18N_NOOP("About info"),
//            "info",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            INTERESTS_INFO,
//            I18N_NOOP("Interests"),
//            "interest",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            PAST_INFO,
//            I18N_NOOP("Group/Past"),
//            "past",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            PICTURE_INFO,
//            I18N_NOOP("Picture"),
//            "pict",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (),
//    };

//static CommandDef aimWnd[] =
//    {
//        CommandDef (
//            MAIN_INFO,
//            " ",
//            "AIM_online",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            ABOUT_INFO,
//            I18N_NOOP("About info"),
//            "info",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (),
//    };

//static CommandDef icqConfigWnd[] =
//    {
//        CommandDef (
//            MAIN_INFO,
//            " ",
//            "ICQ_online",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            HOME_INFO,
//            I18N_NOOP("Home info"),
//            "home",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            WORK_INFO,
//            I18N_NOOP("Work info"),
//            "work",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            MORE_INFO,
//            I18N_NOOP("More info"),
//            "more",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            ABOUT_INFO,
//            I18N_NOOP("About info"),
//            "info",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            INTERESTS_INFO,
//            I18N_NOOP("Interests"),
//            "interest",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            PAST_INFO,
//            I18N_NOOP("Group/Past"),
//            "past",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            PICTURE_INFO,
//            I18N_NOOP("Picture"),
//            "pict",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            NETWORK,
//            I18N_NOOP("Network"),
//            "network",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            SECURITY,
//            I18N_NOOP("Security"),
//            "security",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (),
//    };

//static CommandDef aimConfigWnd[] =
//    {
//        CommandDef (
//            MAIN_INFO,
//            " ",
//            "AIM_online",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            ABOUT_INFO,
//            I18N_NOOP("About info"),
//            "info",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            NETWORK,
//            I18N_NOOP("Network"),
//            "network",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (),
//    };

//CommandDef *ICQClient::infoWindows(Contact*, void *_data)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*) _data); // FIXME unsafe type conversion
//    CommandDef *def = data->getUin() ? icqWnd : aimWnd;
//    QString name = i18n(protocol()->description()->text);
//    name += ' ';
//    if (data->getUin())
//        name += QString::number(data->getUin());
//    else
//        name += data->getScreen();
//    def->text_wrk = name;
//    return def;
//}

//CommandDef *ICQClient::configWindows()
//{
//    CommandDef *def = icqConfigWnd;
//    QString name = i18n(protocol()->description()->text);
//    name += ' ';
//    if (m_bAIM)
//    {
//        name += clientPersistentData->owner.getScreen();
//        def = aimConfigWnd;
//    }
//    else name += QString::number(clientPersistentData->owner.getUin());
//    def->text_wrk = name;
//    return def;
//}

//QWidget *ICQClient::infoWindow(QWidget *parent, Contact *contact, void *_data, unsigned id)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    switch (id){
//    case MAIN_INFO:
//        if (data->getUin())
//            return new ICQInfo(parent, data, contact->id(), this);
//        return new AIMInfo(parent, data, contact->id(), this);
//    case HOME_INFO:
//        return new HomeInfo(parent, data, contact->id(), this);
//    case WORK_INFO:
//        return new WorkInfo(parent, data, contact->id(), this);
//    case MORE_INFO:
//        return new MoreInfo(parent, data, contact->id(), this);
//    case ABOUT_INFO:
//        return new AboutInfo(parent, data, contact->id(), this);
//    case INTERESTS_INFO:
//        return new InterestsInfo(parent, data, contact->id(), this);
//    case PAST_INFO:
//        return new PastInfo(parent, data, contact->id(), this);
//    case PICTURE_INFO:
//        return new ICQPicture(parent, data, this);
//    }
//    return NULL;
//}

//QWidget *ICQClient::configWindow(QWidget *parent, unsigned id)
//{
//    switch (id){
//    case MAIN_INFO:
//        if (m_bAIM)
//            return new AIMInfo(parent, NULL, 0, this);
//        return new ICQInfo(parent, NULL, 0, this);
//    case HOME_INFO:
//        return new HomeInfo(parent, NULL, 0, this);
//    case WORK_INFO:
//        return new WorkInfo(parent, NULL, 0, this);
//    case MORE_INFO:
//        return new MoreInfo(parent, NULL, 0, this);
//    case ABOUT_INFO:
//        return new AboutInfo(parent, NULL, 0, this);
//    case INTERESTS_INFO:
//        return new InterestsInfo(parent, NULL, 0, this);
//    case PAST_INFO:
//        return new PastInfo(parent, NULL, 0, this);
//    case PICTURE_INFO:
//        return new ICQPicture(parent, NULL, this);
//    case NETWORK:
//        if (m_bAIM)
//            return new AIMConfig(parent, this, false);
//        return new ICQConfig(parent, this, false);
//    case SECURITY:
//        return new ICQSecure(parent, this);
//    }
//    return NULL;
//}

//QWidget *ICQClient::searchWindow(QWidget *parent)
//{
//    if (getState() != Connected)
//        return NULL;
//    return new ICQSearch(this, parent);
//}

//void ICQClient::updateInfo(Contact *contact, void *_data)
//{
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    if (getState() != Connected)
//    {
//        Client::updateInfo(contact, _data);
//        return;
//    }
//    if (data == NULL)
//        data = &this->clientPersistentData->owner;
//    if (data->getUin())
//    {
//        addFullInfoRequest(data->getUin());
//        addPluginInfoRequest(data->getUin(), PLUGIN_QUERYxINFO);
//        addPluginInfoRequest(data->getUin(), PLUGIN_QUERYxSTATUS);
//        addPluginInfoRequest(data->getUin(), PLUGIN_AR);
//    }
//    else
//        fetchProfile(data);
//    requestBuddy(data);
//}

//void ICQClient::fetchAwayMessage(ICQUserData *data)
//{
//    addPluginInfoRequest(data->getUin(), PLUGIN_AR);
//}

//bool ICQClient::processEvent(Event *e)
//{
//    TCPClient::processEvent(e);
//    switch (e->type())
//    {
//    case eEventAddContact: {
//        EventAddContact *ec = static_cast<EventAddContact*>(e);
//        EventAddContact::AddContact *ac = ec->addContact();
//        if (protocol()->description()->text == ac->proto)
//        {
//            Group *grp = getContacts()->group(ac->group);
//            Contact *contact;
//            QString tmp = ac->nick;
//            findContact(ac->addr, &tmp, true, contact, grp);
//            ec->setContact(contact);
//            return true;
//        }
//        break;
//                           }
//    case eEventDeleteContact: {
//        EventDeleteContact *ec = static_cast<EventDeleteContact*>(e);
//        QString addr = ec->alias();
//        ContactList::ContactIterator it;
//        Contact *contact;
//        while ((contact = ++it) != NULL)
//        {
//            ICQUserData *data;
//            ClientDataIterator itc = contact->clientDataIterator(this);
//            while ((data = toICQUserData(++itc)) != NULL)
//            {
//                if (data->getScreen() != addr)
//                    continue;
//                contact->freeData(data);
//                ClientDataIterator itc = contact->clientDataIterator();
//                if (++itc == NULL)
//                    delete contact;
//                return true;
//            }
//        }
//        break;
//                              }
//    case eEventGetContactIP: {
//        EventGetContactIP *ei = static_cast<EventGetContactIP*>(e);
//        Contact *contact = ei->contact();
//        ICQUserData *data;
//        ClientDataIterator it = contact->clientDataIterator(this);
//        while ((data = toICQUserData(++it)) != NULL)
//        {
//            if (data->getRealIP())
//            {
//                ei->setIP(data->getRealIP());
//                return true;
//            }
//            if (data->getIP())
//            {
//                ei->setIP(data->getIP());
//                return true;
//            }
//        }
//        break;
//                             }
//    case eEventMessageAccept: {
//        EventMessageAccept *ema = static_cast<EventMessageAccept*>(e);
//        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
//            if ((*it)->id() == ema->msg()->id()){
//                Message *msg = *it;
//                m_acceptMsg.erase(it);
//                snacICBM()->accept(msg, ema->dir(), ema->mode());
//                return msg;
//            }
//        }
//        break;
//                              }
//    case eEventMessageDecline: {
//        EventMessageDecline *emd = static_cast<EventMessageDecline*>(e);
//        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
//            if ((*it)->id() == emd->msg()->id())
//            {
//                Message *msg = *it;
//                m_acceptMsg.erase(it);
//                snacICBM()->decline(msg, emd->reason());
//                return msg;
//            }
//        }
//        break;
//                               }
//    case eEventMessageRetry: {
//        EventMessageRetry *emr = static_cast<EventMessageRetry*>(e);
//        EventMessageRetry::MsgSend *m = emr->msgRetry();
//        QStringList btns;
//        if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied)
//            btns.append(i18n("Send &urgent"));
//        else if (m->msg->getRetryCode() != static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND)
//            return false;
//        btns.append(i18n("Send to &list"));
//        btns.append(i18n("&Cancel"));
//        QString err = i18n(m->msg->getError());
//        Command cmd;
//        cmd->id		= CmdSend;
//        cmd->param	= m->edit;
//        EventCommandWidget eWidget(cmd);
//        eWidget.process();
//        QWidget *msgWidget = eWidget.widget();
//        if (msgWidget == NULL)
//            msgWidget = m->edit;
//        BalloonMsg *msg = new BalloonMsg(m, quoteString(err), btns, msgWidget, NULL, false);
//        connect(msg, SIGNAL(action(int, void*)), this, SLOT(retry(int, void*)));
//        msg->show();
//        return true;
//                             }
//    case eEventTemplateExpanded: {
//        EventTemplate *et = static_cast<EventTemplate*>(e);
//        EventTemplate::TemplateExpand *t = et->templateExpand();
//        list<ar_request>::iterator it;
//        for (it = arRequests.begin(); it != arRequests.end(); ++it)
//            if (&(*it) == t->param)
//                break;
//        if (it == arRequests.end())
//            return false;
//        if (m_bAIM)
//        {
//            if (getState() != Connected || m_status != STATUS_AWAY)
//                return true;

//            if (it->bDirect)
//                setAwayMessage(t->tmpl);
//            else
//            {
//                sendCapability(t->tmpl);
//                m_snacICBM->sendICMB(1, 11);
//                m_snacICBM->sendICMB(2,  3);
//                m_snacICBM->sendICMB(4,  3);
//                snacICBM()->processSendQueue();
//                fetchProfiles();
//            }
//            return true;
//        }
//        ar_request ar = (*it);
//        if (ar.bDirect)
//        {
//            Contact *contact;
//            ICQUserData *data = findContact(ar.screen, NULL, false, contact);
//            DirectClient *dc = dynamic_cast<DirectClient*>(data ? data->getDirect() : 0);
//            if (dc)
//            {
//                QByteArray answer;
//                if (data->getVersion() >= 10)
//                    answer = t->tmpl.toUtf8();
//                else
//                    answer = getContacts()->fromUnicode(contact, t->tmpl);
//                dc->sendAck((unsigned short)(ar.id.id_l), ar.type, ar.flags, answer);
//            }
//        }
//        else
//        {
//            ICQBuffer copy;
//            snacICBM()->sendAutoReply(ar.screen, ar.id, plugins[PLUGIN_NULL],
//                ar.id1, ar.id2, ar.type, (char)(ar.ack), 0, t->tmpl, 0, copy);
//        }
//        arRequests.erase(it);
//        return true;
//                                 }
//    case eEventContact:
//    {
//        EventContact *ec = static_cast<EventContact*>(e);
//        Contact *contact = ec->contact();
//        if (ec->action() == EventContact::eDeleted)
//        {
//            ICQUserData *data;
//            ClientDataIterator it = contact->clientDataIterator(this);
//            while ((data = toICQUserData(++it)) != NULL)
//            {
//                if (data->getIcqID() == 0)
//                    continue;
//                list<ListRequest>::iterator it;
//                for (it = listRequests.begin(); it != listRequests.end(); it++)
//                {
//                    if (it->type != LIST_USER_CHANGED)
//                        continue;
//                    if (it->screen == screen(data))
//                        break;
//                }
//                if (it != listRequests.end())
//                    listRequests.erase(it);
//                ListRequest lr;
//                lr.type = LIST_USER_DELETED;
//                lr.screen = screen(data);
//                lr.icq_id = (unsigned short)(data->getIcqID());
//                lr.grp_id = (unsigned short)(data->getGrpID());
//                lr.visible_id = (unsigned short)(data->getContactVisibleId());
//                lr.invisible_id = (unsigned short)(data->getContactInvisibleId());
//                lr.ignore_id = (unsigned short)(data->getIgnoreId());
//                listRequests.push_back(lr);
//                snacICBM()->processSendQueue();
//            }
//            //m_snacBuddy->removeBuddy(contact);
//        }
//        else if (ec->action() == EventContact::eChanged)
//        {
//            if (getState() != Connected && !m_bAIM)
//                m_snacBuddy->addBuddy(contact);
//            if (contact == getContacts()->owner())
//            {
//                QDateTime now(QDateTime::currentDateTime());
//                if (getContacts()->owner()->getPhones() != clientPersistentData->owner.getPhoneBook())
//                {
//                    clientPersistentData->owner.setPhoneBook(getContacts()->owner()->getPhones());
//                    clientPersistentData->owner.setPluginInfoTime(now.toTime_t());
//                    snacService()->sendPluginInfoUpdate(PLUGIN_PHONEBOOK);
//                }
//                /*
//                if (getPicture() != clientPersistentData->owner.Picture.str()){
//                clientPersistentData->owner.Picture.str() = getPicture();
//                clientPersistentData->owner.PluginInfoTime.asULong() = now;
//                snacService()->sendPluginInfoUpdate(PLUGIN_PICTURE);
//                }
//                */
//                if (getContacts()->owner()->getPhoneStatus() == (int)clientPersistentData->owner.getFollowMe())
//                    return false;

//                clientPersistentData->owner.setFollowMe(getContacts()->owner()->getPhoneStatus());
//                clientPersistentData->owner.setPluginStatusTime(now.toTime_t());
//                snacService()->sendPluginStatusUpdate(PLUGIN_FOLLOWME, clientPersistentData->owner.getFollowMe());
//                return false;
//            }
//            ICQUserData *data;
//            ClientDataIterator it = contact->clientDataIterator(this);
//            while ((data = toICQUserData(++it)) != NULL)
//            {
//                if (data->getUin() || data->getProfileFetch())
//                    continue;
//                fetchProfile(data);
//            }
//            addContactRequest(contact);
//        }
//        break;
//    }
//    case eEventGroup:
//    {
//        EventGroup *ev = static_cast<EventGroup*>(e);
//        Group *group = ev->group();
//        if(!group->id())
//            return false;
//        if (ev->action() == EventGroup::eChanged)
//            addGroupRequest(group);
//        else if (ev->action() == EventGroup::eDeleted)
//        {
//            ICQUserData *data = toICQUserData((SIM::IMContact*)group->getData(this));
//            if (data)
//            {
//                ListRequest lr;
//                lr.type = LIST_GROUP_DELETED;
//                lr.icq_id = (unsigned short)(data->getIcqID());
//                listRequests.push_back(lr);
//                snacICBM()->processSendQueue();
//            }
//        }
//        else if (ev->action() == EventGroup::eAdded)
//            return false;
//        break;
//    }
//    case eEventMessageCancel:
//        {
//        EventMessage *em = static_cast<EventMessage*>(e);
//        Message *msg = em->msg();
//        return snacICBM()->cancelMessage(msg);
//        break;
//    }
//    case eEventCheckCommandState:
//    {
//        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
//        CommandDef *cmd = ecs->cmd();
//        if (cmd->id == CmdPhones)
//            return !m_bAIM;
//        if(cmd->id == CmdFetchAway) {
//            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//            if (!contact)
//                return false;
//            ClientDataIterator it = contact->clientDataIterator(this);
//            ICQUserData *data;
//            while ((data = toICQUserData(++it)) != NULL){
//                unsigned long status = STATUS_OFFLINE;
//                unsigned style  = 0;
//                QString statusIcon;
//                contactInfo(data, status, style, statusIcon);
//                if(status != STATUS_ONLINE && status != STATUS_OFFLINE)
//                {
//                    cmd->flags &= ~BTN_HIDE;
//                    return true;
//                }
//            }
//            return false;
//        }
//        if (cmd->bar_id == ToolBarContainer || cmd->bar_id == ToolBarHistory){
//            if (cmd->id == CmdChangeEncoding)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact == NULL)
//                {
//                    cmd->flags |= BTN_HIDE;
//                    return true;
//                }
//                for (unsigned i = 0; i < getContacts()->nClients(); i++)
//                {
//                    Client *client = getContacts()->getClient(i);
//                    if (client == this)
//                    {
//                        cmd->flags |= BTN_HIDE;
//                        break;
//                    }
//                    if (client->protocol() == protocol())
//                        break;
//                }
//                ClientDataIterator it = contact->clientDataIterator(this);
//                if ((++it) != NULL)
//                {
//                    cmd->flags &= ~BTN_HIDE;
//                    return true;
//                }
//                return false;
//            }
//        }
//        if (cmd->menu_id == MenuContactGroup){
//            if (cmd->id == CmdVisibleList)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact == NULL)
//                    return false;
//                for (unsigned i = 0; i < getContacts()->nClients(); i++)
//                {
//                    Client *client = getContacts()->getClient(i);
//                    if (client == this)
//                    {
//                        cmd->flags &= ~COMMAND_CHECKED;
//                        break;
//                    }
//                    if (client->protocol() != protocol())
//                        continue;

//                    break;
//                }
//                ICQUserData *data;
//                bool bOK = false;
//                ClientDataIterator it = contact->clientDataIterator(this);
//                while ((data = toICQUserData(++it)) != NULL)
//                {
//                    bOK = true;
//                    if (data->getVisibleId())
//                        cmd->flags |= COMMAND_CHECKED;
//                }
//                return bOK;
//            }
//            if (cmd->id == CmdInvisibleList){
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact == NULL)
//                    return false;
//                for (unsigned i = 0; i < getContacts()->nClients(); i++)
//                {
//                    Client *client = getContacts()->getClient(i);
//                    if (client == this)
//                    {
//                        cmd->flags &= ~COMMAND_CHECKED;
//                        break;
//                    }
//                    if (client->protocol() == protocol())
//                        break;
//                }
//                ICQUserData *data;
//                bool bOK = false;
//                ClientDataIterator it = contact->clientDataIterator(this);
//                while ((data = toICQUserData(++it)) != NULL)
//                {
//                    bOK = true;
//                    if (data->getInvisibleId())
//                        cmd->flags |= COMMAND_CHECKED;
//                }
//                return (void*)bOK;
//            }
//        }
//        break;
//                                  }
//    case eEventCommandExec: {
//        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
//        CommandDef *cmd = ece->cmd();
//        if(cmd->id == CmdFetchAway)
//        {
//            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//            ClientDataIterator it = contact->clientDataIterator(this);
//            ICQUserData *data;
//            while ((data = toICQUserData(++it)) != NULL)
//            {
//                unsigned long status = STATUS_OFFLINE;
//                unsigned style  = 0;
//                QString statusIcon;
//                contactInfo(data, status, style, statusIcon);
//                if(status != STATUS_ONLINE && status != STATUS_OFFLINE)
//                    fetchAwayMessage(data);
//            }
//            cmd->flags &= ~COMMAND_CHECKED;
//            return false;
//        }
//        if (cmd->menu_id == MenuContactGroup){
//            if (cmd->id == CmdVisibleList){
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact == NULL)
//                    return false;
//                SIM::IMContact *data;
//                ICQUserData * icq_user_data;
//                ClientDataIterator it = contact->clientDataIterator();
//                while ((data = ++it) != NULL)
//                {
//                    if (data->getSign() == ICQ_SIGN)
//                    {  // Only ICQ contacts can be added to Visible list
//                        icq_user_data=toICQUserData(data);
//                        icq_user_data->setVisibleId((cmd->flags & COMMAND_CHECKED) ? getListId() : 0);
//                        EventContact eContact(contact, EventContact::eChanged);
//                        eContact.process();
//                    }
//                }
//                return true;
//            }
//            if (cmd->id == CmdInvisibleList)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact == NULL)
//                    return false;
//                SIM::IMContact *data;
//                ICQUserData * icq_user_data;
//                ClientDataIterator it = contact->clientDataIterator();
//                while ((data = ++it) != NULL)
//                {
//                    if (data->getSign() == ICQ_SIGN)
//                    { // Only ICQ contacts can be added to Invisible list
//                        icq_user_data=toICQUserData(data);
//                        icq_user_data->setInvisibleId((cmd->flags & COMMAND_CHECKED) ? getListId() : 0);
//                        EventContact eContact(contact, EventContact::eChanged);
//                        eContact.process();
//                    }
//                }
//                return true;
//            }
//        }
//        break;
//                            }
//    case eEventGoURL: {
//        EventGoURL *u = static_cast<EventGoURL*>(e);
//        QString url = u->url();
//        QString proto;
//        int n = url.indexOf(':');
//        if (n < 0)
//            return false;
//        proto = url.left(n);
//        if ((proto != "icq") && (proto != "aim"))
//            return false;
//        url = url.mid(proto.length() + 1);
//        while (url.startsWith("/"))
//            url = url.mid(1);
//        QString s = unquoteString(url);
//        QString screen = getToken(s, ',');
//        if (!screen.isEmpty())
//        {
//            Contact *contact;
//            findContact(screen, &s, true, contact);
//            Command cmd;
//            cmd->id		 = MessageGeneric;
//            cmd->menu_id = MenuMessage;
//            cmd->param	 = (void*)(contact->id());
//            EventCommandExec(cmd).process();
//            return true;
//        }
//        break;
//                      }
//    case eEventInterfaceDown:
//        {
//            if(getMediaSense())
//            {
//                EventInterfaceDown* ev = static_cast<EventInterfaceDown*>(e);
//                if (socket() != NULL && ev->getFd() == socket()->socket()->getFd())
//                {
//                    setState(Error, "Interface down");
//                    setStatus(STATUS_OFFLINE, false);
//                    m_bconnectionLost = true;
//                }
//            }
//            break;
//        }
//    case eEventOpenMessage:
//    {
//        if (getState() != Connected)
//            return false;
//        EventMessage *em = static_cast<EventMessage*>(e);
//        Message *msg = em->msg();
//        if (msg->type() != MessageOpenSecure &&
//            msg->type() != MessageCloseSecure &&
//            msg->type() != MessageWarning)
//            return false;
//        QString client = msg->client();
//        Contact *contact = getContacts()->contact(msg->contact());
//        if (contact == NULL)
//            return false;
//        ICQUserData *data = NULL;
//        ClientDataIterator it = contact->clientDataIterator(this);
//        if (client.isEmpty())
//            while ((data = toICQUserData(++it)) != NULL)
//                break;
//        while ((data = toICQUserData(++it)) != NULL)
//            if (dataName(data) == client)
//                break;
//        if (data == NULL)
//            return false;
//        if (msg->type() == MessageOpenSecure)
//        {
//            SecureDlg *dlg = NULL;
//            QWidgetList list = QApplication::topLevelWidgets();
//            QWidget * w;
//            foreach(w,list)
//            {
//                if (!w->inherits("SecureDlg"))
//                    continue;
//                dlg = static_cast<SecureDlg*>(w);
//                if (dlg->m_client == this && dlg->m_contact == contact->id() && dlg->m_data == data)
//                    break;
//                dlg = NULL;
//            }
//            if (dlg == NULL)
//                dlg = new SecureDlg(this, contact->id(), data);
//            raiseWindow(dlg);
//            return true;
//        }
//        else if (msg->type() == MessageWarning)
//        {
//            if (!(data && (m_bAIM || data->getUin() == 0)))
//                return false;

//            WarnDlg *dlg = new WarnDlg(NULL, data, this);
//            raiseWindow(dlg);
//            return true;
//        }
//        DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//        if (dc && dc->isSecure())
//        {
//            Message *m = new Message(MessageCloseSecure);
//            m->setContact(msg->contact());
//            m->setClient(msg->client());
//            m->setFlags(MESSAGE_NOHISTORY);
//            if (!dc->sendMessage(m))
//                delete m;
//            return true;
//        }
//        break;
//    }
//    default:
//        break;
//    }
//    return false;
//}

//bool ICQClient::send(Message *msg, void *_data)
//{
//    if (getState() != Connected)
//        return false;
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    SendMsg s;
//    switch (msg->type()){
//    case MessageSMS:
//        if (m_bAIM)
//            return false;
//        s.msg    = static_cast<SMSMessage*>(msg);
//        s.text   = s.msg->getPlainText();
//        s.flags  = SEND_1STPART;
//		snacICBM()->sendSMS(s);
//        return true;
//    case MessageAuthRequest:
//        if (data && data->getWaitAuth())
//            return sendAuthRequest(msg, data);
//        return false;
//    case MessageAuthGranted:
//        if (data && data->getWantAuth())
//            return sendAuthGranted(msg, data);
//        return false;
//    case MessageAuthRefused:
//        if (data && data->getWantAuth())
//            return sendAuthRefused(msg, data);
//        return false;
//    case MessageFile:
//        if (data && ((data->getStatus() & 0xFFFF) != ICQ_STATUS_OFFLINE))
//		{
//			log(L_DEBUG, "send: MessageFile");
//            if (!hasCap(data, CAP_AIM_SENDFILE))
//				return false;
//			snacICBM()->sendThruServer(msg, data);
//            return true;
//        }
//        return false;
//    case MessageTypingStop:
//    case MessageTypingStart:
//        if ((data == NULL) || getDisableTypingNotification())
//            return false;
//        if((data->getStatus() & 0xFFFF) == ICQ_STATUS_OFFLINE)
//            return false;
//        if (getInvisible()){
//            if (data->getVisibleId() == 0)
//                return false;
//        }else{
//            if (data->getInvisibleId())
//                return false;
//        }
//        if (!hasCap(data, CAP_TYPING) && !hasCap(data, CAP_AIM_BUDDYCON))
//            return false;
//        snacICBM()->sendMTN(screen(data), msg->type() == MessageTypingStart ? ICQ_MTN_START : ICQ_MTN_FINISH);
//        delete msg;
//        return true;
//    case MessageOpenSecure: {
//        if (data == NULL)
//            return false;
//        DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//        if (dc && dc->isSecure())
//            return false;
//        if (!dc){
//            dc = new DirectClient(data, this, PLUGIN_NULL);
//            data->setDirect(dc);
//            dc->connect();
//        }
//        return dc->sendMessage(msg);
//    }
//    case MessageCloseSecure: {
//        if (data == NULL)
//            return false;
//        DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//        if (dc && dc->isSecure())
//            return dc->sendMessage(msg);
//        return false;
//    }
//    case MessageWarning:
//        return snacICBM()->sendThruServer(msg, data);
//    case MessageContacts:
//        if ((data == NULL) || ((data->getUin() == 0) && !hasCap(data, CAP_AIM_BUDDYLIST)))
//            return false;
//    }
//    if (data == NULL)
//        return false;
//    if (data->getUin()){
//        bool bCreateDirect = false;
//        DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//        if ((dc == NULL) &&
//                !data->getNoDirect() &&
//                (data->getStatus() != ICQ_STATUS_OFFLINE) &&
//                ((data->getIP()) == (this->clientPersistentData->owner.getIP())))
//            bCreateDirect = true;
//        if (!bCreateDirect &&
//                (msg->type() == MessageGeneric) &&
//                (data->getStatus() != ICQ_STATUS_OFFLINE) &&
//                ((data->getIP())) &&
//                ((unsigned)msg->getPlainText().length() >= MAX_TYPE2_MESSAGE_SIZE))
//            bCreateDirect = true;
//        if ((getInvisible() && (data->getVisibleId() == 0)) ||
//                (!getInvisible() && data->getInvisibleId()))
//            bCreateDirect = false;
//        if (bCreateDirect){
//            dc = new DirectClient(data, this, PLUGIN_NULL);
//            data->setDirect(dc);
//            dc->connect();
//        }
//        if (dc)
//            return dc->sendMessage(msg);
//    }
//    return snacICBM()->sendThruServer(msg, data);
//}

//bool ICQClient::canSend(unsigned type, void *_data)
//{
//    if (_data && (((IMContact*)_data)->getSign() != ICQ_SIGN))
//        return false;
//    if (getState() != Connected)
//        return false;
//    ICQUserData *data = toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
//    switch (type){
//    case MessageSMS:
//        return !m_bAIM;
//    case MessageGeneric:
//    case MessageUrl:
//        return (data != NULL);
//    case MessageContacts:
//        return (data != NULL) && (data->getUin() || hasCap(data, CAP_AIM_BUDDYLIST));
//    case MessageAuthRequest:
//        return data && (data->getWaitAuth());
//    case MessageAuthGranted:
//        return data && (data->getWantAuth());
//    case MessageFile:
//        return data &&
//               ((data->getStatus() & 0xFFFF) != ICQ_STATUS_OFFLINE) &&
//               (data->getUin() || hasCap(data, CAP_AIM_SENDFILE));
//    case MessageWarning:
//        return data && (data->getUin() == 0);
//    case MessageOpenSecure:
//        if ((data == NULL) || ((data->getStatus() & 0xFFFF) == ICQ_STATUS_OFFLINE))
//            return false;
//        if (hasCap(data, CAP_LICQ) ||
//                hasCap(data, CAP_SIM) ||
//                hasCap(data, CAP_SIMOLD) ||
//                ((data->getInfoUpdateTime() & 0xFF7F0000L) == 0x7D000000L)){
//            DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//            if (dc)
//                return !(dc->isSecure());
//            return (data->getIP()) || (data->getRealIP());
//        }
//        return false;
//    case MessageCloseSecure: {
//            if(!data)
//                return false;
//            DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//            return dc && dc->isSecure();
//        }
//    }
//    return false;
//}

//QString ICQClient::dataName(void *data)
//{
//    return dataName(screen(toICQUserData((SIM::IMContact*)data))); // FIXME unsafe type conversion
//}

//QString ICQClient::dataName(const QString &screen)
//{
//    return name() + '.' + screen;
//}

//QString ICQClient::screen(const ICQUserData *data)
//{
//    if (data->getUin() == 0)
//        return data->getScreen();
//    return QString::number(data->getUin());
//}

//bool ICQClient::messageReceived(Message *msg, const QString &screen)
//{
//    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
//    if (msg->contact() == 0){
//        Contact *contact;
//        ICQUserData *data = findContact(screen, NULL, false, contact);
//        if (data == NULL){
//            data = findContact(screen, NULL, true, contact);
//            if (data == NULL){
//                delete msg;
//                return true;
//            }
//            contact->setFlags(CONTACT_TEMP);
//            EventContact e(contact, EventContact::eChanged);
//            e.process();
//        }
//        msg->setClient(dataName(data));
//        msg->setContact(contact->id());
//        if (data->getTyping()){
//            data->setTyping(false);
//            EventContact e(contact, EventContact::eStatus);;
//            e.process();
//        }
//    }
//    bool bAccept = false;
//    switch (msg->type()){
//    case MessageICQFile:
//    case MessageFile:
//        bAccept = true;
//        break;
//    }
//    if (bAccept)
//        m_acceptMsg.push_back(msg);
//    EventMessageReceived e(msg);
//    if (e.process()){
//        if (bAccept){
//            for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
//                if ((*it) == msg){
//                    m_acceptMsg.erase(it);
//                    break;
//                }
//            }
//        }
//    }else{
//        if (!bAccept)
//            delete msg;
//    }
//    return !bAccept;
//}

//QString ICQClient::contactName(void *clientData)
//{
//    QString res;
//    ICQUserData *data = toICQUserData((SIM::IMContact*)clientData); // FIXME unsafe type conversion
//    res = data->getUin() ? "ICQ: " : "AIM: ";
//    if (!data->getNick().isEmpty()){
//        res += data->getNick();
//        res += " (";
//    }
//    res += data->getUin() ? QString::number(data->getUin()) : data->getScreen();
//    if (!data->getNick().isEmpty())
//        res += ')';
//    return res;
//}

//bool ICQClient::isSupportPlugins(ICQUserData *data)
//{
//    if (data->getVersion() < 7)
//        return false;
//    switch (data->getInfoUpdateTime()){
//    case 0xFFFFFF42:
//    case 0xFFFFFFFF:
//    case 0xFFFFFF7F:
//    case 0xFFFFFFBE:
//    case 0x3B75AC09:
//    case 0x3AA773EE:
//    case 0x3BC1252C:
//    case 0x3B176B57:
//    case 0x3BA76E2E:
//    case 0x3C7D8CBC:
//    case 0x3CFE0688:
//    case 0x3BFF8C98:
//        return false;
//    }
//    if ((data->getInfoUpdateTime() & 0xFF7F0000L) == 0x7D000000L)
//        return false;
//    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
//        return false;
//    return true;
//}

//void ICQClient::addPluginInfoRequest(unsigned long uin, unsigned plugin_index)
//{
//	log(L_DEBUG, "ICQClient::addPluginInfoRequest");
//    Contact *contact;
//    ICQUserData *data = findContact(uin, NULL, false, contact);
//    if (data && !data->getNoDirect() &&
//            (data->getIP()) && ((data->getIP()) == (this->clientPersistentData->owner.getIP())) &&
//            ((getInvisible() && data->getVisibleId()) ||
//             (!getInvisible() && (data->getInvisibleId() == 0)))){
//        switch (plugin_index){
//        case PLUGIN_AR: {
//                DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirect());
//                if ((dc == NULL) && !getHideIP()){
//                    dc = new DirectClient(data, this, PLUGIN_NULL);
//                    data->setDirect(dc);
//                    dc->connect();
//                }
//                if (dc){
//                    dc->addPluginInfoRequest(plugin_index);
//                    return;
//                }
//                break;
//            }
//        case PLUGIN_QUERYxINFO:
//        case PLUGIN_PHONEBOOK:
//        case PLUGIN_PICTURE: {
//                if (!isSupportPlugins(data))
//                    return;
//                DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirectPluginInfo());
//                if ((dc == NULL) && !getHideIP()){
//                    dc = new DirectClient(data, this, PLUGIN_INFOxMANAGER);
//                    data->setDirectPluginInfo(dc);
//                    dc->connect();
//                }
//                if (dc){
//                    dc->addPluginInfoRequest(plugin_index);
//                    return;
//                }
//                break;
//            }
//        case PLUGIN_QUERYxSTATUS:
//        case PLUGIN_FILESERVER:
//        case PLUGIN_FOLLOWME:
//        case PLUGIN_ICQPHONE: {
//                if (!isSupportPlugins(data))
//                    return;
//                DirectClient *dc = dynamic_cast<DirectClient*>(data->getDirectPluginStatus());
//                if ((dc == NULL) && !getHideIP()){
//                    dc = new DirectClient(data, this, PLUGIN_STATUSxMANAGER);
//                    data->setDirectPluginStatus(dc);
//                    dc->connect();
//                }
//                if (dc){
//                    dc->addPluginInfoRequest(plugin_index);
//                    return;
//                }
//                break;
//            }
//        }
//    }
//    snacICBM()->pluginInfoRequest(uin, plugin_index);
//}

//void ICQClient::randomChatInfo(unsigned long uin)
//{
//    addPluginInfoRequest(uin, PLUGIN_RANDOMxCHAT);
//}

//unsigned short ICQClient::msgStatus()
//{
//    return (unsigned short)(fullStatus(getStatus()) & 0xFF);
//}

//static char PICT_PATH[] = "pictures/";

//QString ICQClient::pictureFile(const ICQUserData *data)
//{
//    QString f = user_file(PICT_PATH);
//    QFileInfo fi(f);
//    if(!fi.exists())
//      QDir().mkdir(f);
//    if(!fi.isDir())
//      log(L_ERROR, QString("%1 is not a directory!").arg(f));
//    f += "icq.avatar.";
//    f += data->getUin() ? QString::number(data->getUin()) : data->getScreen();
//    f += '.';
//    f += QString::number(data->getBuddyID());
//    return f;
//}

//QImage ICQClient::userPicture(unsigned id)
//{
//    if (id==0)
//        return QImage();
//    Contact *contact = getContacts()->contact(id);
//    if(!contact)
//        return QImage();
//    ClientDataIterator it = contact->clientDataIterator(this);

//    ICQUserData *d;
//    while ((d = toICQUserData(++it)) != NULL){
//        QImage img = userPicture(d);
//        if(!img.isNull())
//            return img;
//    }
//    return QImage();
//}

//QImage ICQClient::userPicture(ICQUserData *d)
//{
//    QImage img = QImage(d ? pictureFile(d) : clientPersistentData->owner.getPicture());

//    if(img.isNull())
//        return img;

//    int w = img.width();
//    int h = img.height();
//    if (h > w){
//        if (h > 60){
//            w = w * 60 / h;
//            h = 60;
//        }
//    }else{
//        if (w > 60){
//            h = h * 60 / w;
//            w = 60;
//        }
//   }

//   return img.scaled(w, h);
//}


//void ICQClient::retry(int n, void *p)
//{
//    EventMessageRetry::MsgSend *m = reinterpret_cast<EventMessageRetry::MsgSend*>(p);
//    if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND){
//        if (n == 0){
//            m->edit->m_flags = MESSAGE_LIST;
//        }else{
//            return;
//        }
//    }else if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied){
//        switch (n){
//        case 0:
//            m->edit->m_flags = MESSAGE_URGENT;
//            break;
//        case 1:
//            m->edit->m_flags = MESSAGE_LIST;
//            break;
//        default:
//            return;
//        }
//    }else{
//        return;
//    }
//    Command cmd;
//    cmd->id    = CmdSend;
//    cmd->param = m->edit;
//    EventCommandExec(cmd).process();
//}

//bool ICQClient::isOwnData(const QString &screen)
//{
//    if (screen.isEmpty())
//        return false;
//    if(clientPersistentData->owner.getUin())
//        return (clientPersistentData->owner.getUin() == screen.toULong());
//    return (screen.toLower() == clientPersistentData->owner.getScreen().toLower());
//}

//QString ICQClient::addCRLF(const QString &str)
//{
//    QString res = str;
//    return res.replace(QRegExp("\r?\n"), "\r\n");
//}

//Contact *ICQClient::getContact(ICQUserData *data)
//{
//    Contact *contact = NULL;
//    findContact(screen(data), NULL, false, contact);
//    return contact;
//}


//ICQUserData* ICQClient::toICQUserData(SIM::IMContact * data)
//{
//   // This function is used to more safely preform type conversion from SIM::clientData* into ICQUserData*
//   // It will at least warn if the content of the structure is not ICQUserData
//   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
//   if (! data) return NULL;
//   if (data->getSign() != ICQ_SIGN)
//   {
//      QString Signs[] = {
//        "Unknown(0)" ,     // 0x0000
//        "ICQ_SIGN",        // 0x0001
//        "JABBER_SIGN",     // 0x0002
//        "MSN_SIGN",        // 0x0003
//        "Unknown(4)"       // 0x0004
//        "LIVEJOURNAL_SIGN",// 0x0005
//        "SMS_SIGN",        // 0x0006
//        "Unknown(7)",      // 0x0007
//        "Unknown(8)",      // 0x0008
//        "YAHOO_SIGN"       // 0x0009
//      };
//      QString Sign;
//      if (data->getSign()<=9) // is always >=0 as it is unsigned int
//        Sign = Signs[data->getSign()];
//      else
//        Sign = QString("Unknown(%1)").arg(Sign.toULong());

//      log(L_ERROR,
//        "ATTENTION!! Unsafly converting %s user data into ICQ_SIGN",
//         qPrintable(Sign));
////      abort();
//   }
//   return (ICQUserData*) data;
//}

