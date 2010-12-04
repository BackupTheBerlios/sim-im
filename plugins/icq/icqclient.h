/***************************************************************************
                          icqclient.h  -  description
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

#ifndef _ICQCLIENT_H
#define _ICQCLIENT_H

#include <map>
#include <vector>
#include <limits.h>

#include <QDateTime>
#include <QList>
#include <QByteArray>
#include <QTcpSocket>
#include <climits>

#include "misc.h"
#include "snac.h"
#include "icqstatus.h"
#include "icq_defines.h"
#include "oscarsocket.h"
//#include "icqbuddy.h"
//#include "icqservice.h"
//#include "icqicmb.h"
#include "authorizationsnachandler.h"
#include "servicesnachandler.h"

#include "icq.h"
#include "icqbuffer.h"
#include "icqcontact.h"
#include "oscarsocket.h"
#include "contacts/imcontact.h"


const unsigned ICQ_SIGN			= 0x0001;

const unsigned MESSAGE_DIRECT    = 0x0100;


const unsigned char ICQ_TCP_VERSION = 0x09;

const unsigned char MODE_DENIED   = 0x01;
const unsigned char MODE_INDIRECT = 0x02;
const unsigned char MODE_DIRECT   = 0x04;

// Server channels
const unsigned char ICQ_CHNxNEW                    = 0x01;
const unsigned char ICQ_CHNxDATA                   = 0x02;
const unsigned char ICQ_CHNxERROR                  = 0x03;
const unsigned char ICQ_CHNxCLOSE                  = 0x04;
const unsigned char ICQ_CHNxPING                   = 0x05;

// Server SNAC foodgroups
const unsigned short ICQ_SNACxFOOD_SERVICE          = 0x0001;
const unsigned short ICQ_SNACxFOOD_LOCATION         = 0x0002;
const unsigned short ICQ_SNACxFOOD_BUDDY            = 0x0003;
const unsigned short ICQ_SNACxFOOD_MESSAGE          = 0x0004;
const unsigned short ICQ_SNACxFOOD_AIMxINVITATION   = 0x0006;
const unsigned short ICQ_SNACxFOOD_ADMINISTRATIVE   = 0x0007;
const unsigned short ICQ_SNACxFOOD_BOS              = 0x0009;
const unsigned short ICQ_SNACxFOOD_PING             = 0x000B;
const unsigned short ICQ_SNACxFOOD_CHATxNAVIGATION  = 0x000D;
const unsigned short ICQ_SNACxFOOD_CHAT			   = 0x000E;
const unsigned short ICQ_SNACxFOOD_SSBI             = 0x0010;
const unsigned short ICQ_SNACxFOOD_LISTS            = 0x0013;
const unsigned short ICQ_SNACxFOOD_VARIOUS          = 0x0015;
const unsigned short ICQ_SNACxFOOD_LOGIN            = 0x0017;

#define SNAC(A, B)	((A << 16) + B)

// Status
const unsigned short ICQ_STATUS_OFFLINE            = 0xFFFF;
const unsigned short ICQ_STATUS_ONLINE             = 0x0000;
const unsigned short ICQ_STATUS_AWAY               = 0x0001;
const unsigned short ICQ_STATUS_DND                = 0x0002;
const unsigned short ICQ_STATUS_NA                 = 0x0004;
const unsigned short ICQ_STATUS_OCCUPIED           = 0x0010;
const unsigned short ICQ_STATUS_FFC                = 0x0020;

const unsigned long ICQ_STATUS_FxFLAGS             = 0xFFFF0000;
const unsigned long ICQ_STATUS_FxUNKNOWNxFLAGS     = 0xCFC0FCC8;
const unsigned long ICQ_STATUS_FxPRIVATE           = 0x00000100;
const unsigned long ICQ_STATUS_FxPFMxAVAILABLE     = 0x00000200;  // not implemented
const unsigned long ICQ_STATUS_FxWEBxPRESENCE      = 0x00010000;
const unsigned long ICQ_STATUS_FxHIDExIP           = 0x00020000;
const unsigned long ICQ_STATUS_FxPFM               = 0x00040000;  // not implemented
const unsigned long ICQ_STATUS_FxBIRTHDAY          = 0x00080000;
const unsigned long ICQ_STATUS_FxDIRECTxDISABLED   = 0x00100000;
const unsigned long ICQ_STATUS_FxICQxHOMEPAGE      = 0x00200000;  // not implemented
const unsigned long ICQ_STATUS_FxDIRECTxAUTH       = 0x10000000;  // will accept connections only when authorized
const unsigned long ICQ_STATUS_FxDIRECTxLISTED     = 0x20000000;  // will accept connections only when listed

const unsigned CLASS_UNCONFIRMED	= 0x0001;	// AOL unconfirmed user flsg
const unsigned CLASS_ADMINISTRATOR	= 0x0002;   // AOL administrator flag
const unsigned CLASS_AOL			= 0x0004;	// AOL staff user flag
const unsigned CLASS_COMMERCIAL     = 0x0008;	// AOL commercial account flag
const unsigned CLASS_FREE			= 0x0010;	// ICQ non-commercial account flag
const unsigned CLASS_AWAY			= 0x0020;	// Away status flag
const unsigned CLASS_ICQ			= 0x0040;	// ICQ user sign
const unsigned CLASS_WIRELESS		= 0x0100;	// AOL wireless user

const unsigned short ICQ_MSGxMSG               = 0x0001;
const unsigned short ICQ_MSGxCHAT              = 0x0002;
const unsigned short ICQ_MSGxFILE              = 0x0003;
const unsigned short ICQ_MSGxURL               = 0x0004;
const unsigned short ICQ_MSGxAUTHxREQUEST      = 0x0006;
const unsigned short ICQ_MSGxAUTHxREFUSED      = 0x0007;
const unsigned short ICQ_MSGxAUTHxGRANTED      = 0x0008;
const unsigned short ICQ_MSGxSERVERxMSG		   = 0x0009;
const unsigned short ICQ_MSGxADDEDxTOxLIST     = 0x000C;
const unsigned short ICQ_MSGxWEBxPANEL         = 0x000D;
const unsigned short ICQ_MSGxEMAILxPAGER       = 0x000E;
const unsigned short ICQ_MSGxCONTACTxLIST      = 0x0013;
const unsigned short ICQ_MSGxEXT               = 0x001A;

const unsigned short ICQ_MSGxSECURExCLOSE      = 0x00EE;
const unsigned short ICQ_MSGxSECURExOPEN       = 0x00EF;

const unsigned short ICQ_MSGxAR_AWAY           = 0x03E8;
const unsigned short ICQ_MSGxAR_OCCUPIED       = 0x03E9;
const unsigned short ICQ_MSGxAR_NA             = 0x03EA;
const unsigned short ICQ_MSGxAR_DND            = 0x03EB;
const unsigned short ICQ_MSGxAR_FFC            = 0x03EC;

const unsigned short ICQ_TCPxACK_ONLINE         = 0x0000;
const unsigned short ICQ_TCPxACK_AWAY           = 0x0004;
const unsigned short ICQ_TCPxACK_OCCUPIED       = 0x0009;
const unsigned short ICQ_TCPxACK_DND            = 0x000A;
const unsigned short ICQ_TCPxACK_OCCUPIEDxCAR   = 0x000B;
const unsigned short ICQ_TCPxACK_OCCUPIEDx2     = 0x000C;
const unsigned short ICQ_TCPxACK_NA             = 0x000E;
const unsigned short ICQ_TCPxACK_DNDxCAR        = 0x000F;
const unsigned short ICQ_TCPxACK_ACCEPT         = 0x0000;
const unsigned short ICQ_TCPxACK_REFUSE         = 0x0001;

const unsigned short ICQ_TCPxMSG_AUTOxREPLY     = 0x0000;
const unsigned short ICQ_TCPxMSG_NORMAL         = 0x0001;
const unsigned short ICQ_TCPxMSG_URGENT         = 0x0002;
const unsigned short ICQ_TCPxMSG_LIST           = 0x0004;

const unsigned short ICQ_MTN_FINISH   = 0x0000;
const unsigned short ICQ_MTN_TYPED    = 0x0001;
const unsigned short ICQ_MTN_START    = 0x0002;

const char DIRECT_MODE_DENIED    = 0x01;
const char DIRECT_MODE_INDIRECT  = 0x02;
const char DIRECT_MODE_DIRECT    = 0x04;

const unsigned MAX_PLAIN_MESSAGE_SIZE = 450;
const unsigned MAX_TYPE2_MESSAGE_SIZE = 4096;

const unsigned PING_TIMEOUT = 60;

const unsigned RATE_PAUSE = 3;
const unsigned RATE_LIMIT = 5;

const unsigned short SEARCH_DONE = USHRT_MAX;

const unsigned short TLV_CONTENT          = 0x2711;
const unsigned short TLV_EXTENDED_CONTENT = 0x2712;

class AIMFileTransfer;
class DirectClient;

class ICQClient;

class ICQClientData
{
public:

    ICQClientData(ICQClient* client);
    virtual QByteArray serialize();
    virtual void deserialize(Buffer* cfg);

    virtual void serialize(QDomElement& /*element*/) {}
    virtual void deserialize(QDomElement& /*element*/) {}

    virtual SIM::ClientWeakPtr client() { Q_ASSERT_X(false, "ICQClientData::client", "Shouldn't be called"); return SIM::ClientWeakPtr(); }

    virtual unsigned long getSign();

    QString getServer() const { return m_server; }
    void setServer(const QString& server) { m_server = server; }

    unsigned long getPort() const { return m_port; }
    void setPort(unsigned long port) { m_port = port; }

    unsigned long getContactsTime() const { return m_contactsTime; }
    void setContactsTime(unsigned long ct) { m_contactsTime = ct; }

    unsigned long getContactsLength() const { return m_contactsLength; }
    void setContactsLength(unsigned long cl) { m_contactsLength = cl; }

    unsigned long getContactsInvisible() const { return m_contactsInvisible; }
    void setContactsInvisible(unsigned long ci) { m_contactsInvisible = ci; }

    bool getHideIP() const { return m_hideIp; }
    void setHideIP(bool hideip) { m_hideIp = hideip; }

    bool getIgnoreAuth() const { return m_ignoreAuth; }
    void setIgnoreAuth(bool ignoreAuth) { m_ignoreAuth = ignoreAuth; }

    bool getUseMD5() const { return m_useMd5; }
    void setUseMD5(bool useMd5) { m_useMd5 = useMd5; }

    unsigned long getDirectMode() const { return m_directMode; }
    void setDirectMode(unsigned long mode) { m_directMode = mode; }

    unsigned long getIdleTime() const { return m_idleTime; }
    void setIdleTime(unsigned long time) { m_idleTime = time; }

    QString getListRequests() const { return m_listRequests; }
    void setListRequests(const QString& listrequests) { m_listRequests = listrequests; }

    QString getPicture() const { return m_picture; }
    void setPicture(const QString& pic) { m_picture = pic; }

    unsigned long getRandomChatGroup() const { return m_randomChatGroup; }
    void setRandomChatGroup(unsigned long group) { m_randomChatGroup = group; }

    unsigned long getRandomChatGroupCurrent() const { return m_randomChatGroupCurrent; }
    void setRandomChatGroupCurrent(unsigned long group) { m_randomChatGroupCurrent = group; }

    unsigned long getSendFormat() const { return m_sendFormat; }
    void setSendFormat(unsigned long format) { m_sendFormat = format; }

    bool getDisablePlugins() const { return m_disablePlugins; }
    void setDisablePlugins(bool p) { m_disablePlugins = p; }

    bool getDisableAutoUpdate() const { return m_disableAutoUpdate; }
    void setDisableAutoUpdate(bool b) { m_disableAutoUpdate = b; }

    bool getDisableAutoReplyUpdate() const { return m_disableAutoReplyUpdate; }
    void setDisableAutoReplyUpdate(bool b) { m_disableAutoReplyUpdate = b; }

    bool getDisableTypingNotification() const { return m_disableTypingNotification; }
    void setDisableTypingNotification(bool b) { m_disableTypingNotification = b; }

    bool getAcceptInDND() const { return m_acceptInDnd; }
    void setAcceptInDND(bool b) {m_acceptInDnd = b; }

    bool getAcceptInOccupied() const { return m_acceptInOccupied; }
    void setAcceptInOccupied(bool b) {m_acceptInOccupied = b; }

    unsigned long getMinPort() const { return m_minPort; }
    void setMinPort(unsigned long port) { m_minPort = port; }

    unsigned long getMaxPort() const { return m_maxPort; }
    void setMaxPort(unsigned long port) { m_maxPort = port; }

    bool getWarnAnonymously() const { return m_warnAnonymously; }
    void setWarnAnonymously(bool b) { m_warnAnonymously = b; }

    unsigned long getAckMode() const { return m_ackMode; }
    void setAckMode(unsigned long ackmode) { m_ackMode = ackmode; }

    bool getUseHttp() const { return m_useHttp; }
    void setUseHttp(bool b) { m_useHttp = b; }

    bool getAutoHttp() const { return m_autoHttp; }
    void setAutoHttp(bool b) { m_autoHttp = b; }

    bool getKeepAlive() const { return m_keepAlive; }
    void setKeepAlive(bool b) { m_keepAlive = b; }

    bool getMediaSense() const { return m_mediaSense; }
    void setMediaSense(bool b) { m_mediaSense = b; }

    ICQContact owner;

    virtual void deserializeLine(const QString& key, const QString& value);
public:

    QString m_server;
    unsigned long m_port;
    unsigned long m_contactsTime;
    unsigned long m_contactsLength;
    unsigned long m_contactsInvisible;
    bool m_hideIp;
    bool m_ignoreAuth;
    bool m_useMd5;
    unsigned long m_directMode;
    unsigned long m_idleTime;
    QString m_listRequests;
    QString m_picture;
    unsigned long m_randomChatGroup;
    unsigned long m_randomChatGroupCurrent;
    unsigned long m_sendFormat;
    bool m_disablePlugins;
    bool m_disableAutoUpdate;
    bool m_disableAutoReplyUpdate;
    bool m_disableTypingNotification;
    bool m_acceptInDnd;
    bool m_acceptInOccupied;
    unsigned long m_minPort;
    unsigned long m_maxPort;
    bool m_warnAnonymously;
    unsigned long m_ackMode;
    bool m_useHttp;
    bool m_autoHttp;
    bool m_keepAlive;
    bool m_mediaSense;
};

class ICQClient;

//struct SearchResult
//{
//    SearchResult() : data(SIM::ClientPtr(0)) {};
//    ICQUserData       data;
//    unsigned short    id;
//    ICQClient         *client;
//};

class QTimer;

typedef unsigned char capability[0x10];
typedef unsigned char plugin[0x12];

enum cap_id_t {
    CAP_AIM_SHORTCAPS = 0,      // 0x00000001
    CAP_AIM_VOICE,              // 0x00000002
    CAP_AIM_SENDFILE,           // 0x00000004
    CAP_DIRECT,                 // 0x00000008
    CAP_AIM_IMIMAGE,            // 0x00000010
    CAP_AIM_BUDDYCON,           // 0x00000020
    CAP_AIM_STOCKS,             // 0x00000040
    CAP_AIM_GETFILE,            // 0x00000080
    CAP_SRV_RELAY,              // 0x00000100
    CAP_AIM_GAMES,              // 0x00000200
    CAP_AIM_BUDDYLIST,          // 0x00000400
    CAP_AVATAR,                 // 0x00000800
    CAP_AIM_SUPPORT,            // 0x00001000
    CAP_UTF,                    // 0x00002000
    CAP_RTF,                    // 0x00004000
    CAP_TYPING,                 // 0x00008000
    CAP_SIM,                    // 0x00010000
    CAP_STR_2001,               // 0x00020000
    CAP_STR_2002,               // 0x00040000
    CAP_IS_2001,                // 0x00080000
    CAP_TRILLIAN,               // 0x00100000
    CAP_TRIL_CRYPT,             // 0x00200000
    CAP_MACICQ,                 // 0x00400000
    CAP_AIM_CHAT,               // 0x00800000
    CAP_MICQ,                   // 0x01000000
    CAP_LICQ,                   // 0x02000000
    CAP_SIMOLD,                 // 0x04000000
    CAP_KOPETE,                 // 0x08000000
    CAP_XTRAZ,                  // 0x10000000
    CAP_IS_2002,                // 0x20000000
    CAP_MIRANDA,                // 0x40000000
    CAP_ANDRQ,                  // 0x80000000
    CAP_QIP,                    // 0x00000001 - Caps2
    CAP_IMSECURE,               // 0x00000002
    CAP_KXICQ,                  // 0x00000004
    CAP_ICQ5_1,                 // 0x00000008
    CAP_UNKNOWN,                // 0x00000010
    CAP_ICQ5_3,                 // 0x00000020
    CAP_ICQ5_4,                 // 0x00000040
    CAP_ICQ51,                  // 0x00000080
    CAP_JIMM,                   // 0x00000100
    CAP_ICQJP,                  // 0x00000200
    CAP_HTMLMSGS,               // 0x00000400
    CAP_INFIUM,                 // 0x00000800
    CAP_ABV,                    // 0x00001000
    CAP_NETVIGATOR,             // 0x00002000
    CAP_RAMBLER,                // 0x00004000
    CAP_LITE_NEW,               // 0x00008000
    CAP_HOST_STATUS_TEXT_AWARE, // 0x00010000
    CAP_AIM_LIVE_VIDEO,         // 0x00020000
    CAP_AIM_LIVE_AUDIO,         // 0x00040000
    CAP_QIP2010,                // 0x00080000
    CAP_NULL                    // 0x00000400
};

const unsigned PLUGIN_PHONEBOOK          = 0;
const unsigned PLUGIN_PICTURE            = 1;
const unsigned PLUGIN_FILESERVER         = 2;
const unsigned PLUGIN_FOLLOWME           = 3;
const unsigned PLUGIN_ICQPHONE           = 4;
const unsigned PLUGIN_QUERYxINFO         = 5;
const unsigned PLUGIN_QUERYxSTATUS       = 6;
const unsigned PLUGIN_INFOxMANAGER       = 7;
const unsigned PLUGIN_STATUSxMANAGER     = 8;
const unsigned PLUGIN_RANDOMxCHAT        = 9;
const unsigned PLUGIN_VIDEO_CHAT         = 10;
const unsigned PLUGIN_NULL               = 11;
const unsigned PLUGIN_FILE               = 12;
const unsigned PLUGIN_CHAT               = 13;
const unsigned PLUGIN_AR                 = 14;
const unsigned PLUGIN_INVISIBLE          = 15;
const unsigned PLUGIN_REVERSE            = 16;
const unsigned PLUGIN_AIM_FT			 = 17;
const unsigned PLUGIN_AIM_FT_ACK		 = 18;

class ICQClient;

const unsigned LIST_USER_CHANGED    = 0;
const unsigned LIST_USER_DELETED    = 1;
const unsigned LIST_GROUP_CHANGED   = 2;
const unsigned LIST_GROUP_DELETED   = 3;
const unsigned LIST_BUDDY_CHECKSUM  = 4;

//class ListRequest
//{
//public:
//    ListRequest()
//        : type(0), icq_id(0), grp_id(0), visible_id(0), invisible_id(0), ignore_id(0),
//          icqUserData(NULL) {}

//public:
//    unsigned          type;
//    QString           screen;
//    unsigned short    icq_id;
//    unsigned short    grp_id;
//    unsigned short    visible_id;
//    unsigned short    invisible_id;
//    unsigned short    ignore_id;
//    //const ICQUserData *icqUserData;
//};

//class ICQListener : public SIM::ServerSocketNotify
//{
//public:
//    ICQListener(ICQClient *client);
//    ~ICQListener();
//protected:
//    virtual bool accept(SIM::Socket *s, unsigned long ip);
//    virtual void bind_ready(unsigned short port);
//    virtual bool error(const QString &err);
//    ICQClient  *m_client;
//};

class DirectSocket;
class ServerRequest;
class ListServerRequest;
class QTextCodec;

const unsigned SEND_PLAIN		= 0x0001;
const unsigned SEND_UTF			= 0x0002;
const unsigned SEND_RTF			= 0x0003;
const unsigned SEND_TYPE2		= 0x0004;
const unsigned SEND_RAW			= 0x0005;
const unsigned SEND_HTML		= 0x0006;
const unsigned SEND_HTML_PLAIN	= 0x0007;
const unsigned SEND_MASK		= 0x000F;
const unsigned SEND_1STPART		= 0x0010;

//struct ar_request
//{
//    unsigned short    type;
//    unsigned short    flags;
//    unsigned short    ack;
//    MessageId         id;
//    unsigned short    id1;
//    unsigned short    id2;
//    QString           screen;
//    bool              bDirect;
//};

typedef std::map<unsigned short, QString> INFO_REQ_MAP;

class DirectSocket;
class ServiceSocket;
class ICQClientSocket;


struct InfoRequest
{
    unsigned	uin;
    unsigned	request_id;
    unsigned	start_time;
};

//typedef std::map<SIM::my_string, alias_group>	CONTACTS_MAP;
typedef std::map<unsigned, unsigned>			RATE_MAP;
typedef QMap<unsigned short, SnacHandler*> mapSnacHandlers;

class ICQ_EXPORT ICQClient : public QObject, public SIM::Client
{
    Q_OBJECT
public:
    ICQClient(SIM::Protocol*, const QString& name, bool bAIM);
    virtual ~ICQClient();
	virtual QString retrievePasswordLink();
    virtual QString name();

    SIM::IMContactPtr createIMContact();
    SIM::IMGroupPtr createIMGroup();

    virtual QWidget* createSetupWidget(const QString& id, QWidget* parent);
    virtual void destroySetupWidget();
    virtual QStringList availableSetupWidgets() const;

    virtual QWidget* createStatusWidget();

    virtual SIM::IMStatusPtr currentStatus();
    virtual void changeStatus(const SIM::IMStatusPtr& status);
    virtual SIM::IMStatusPtr savedStatus();

    SIM::IMContactPtr ownerContact();
    void setOwnerContact(SIM::IMContactPtr contact);

    QWidget* createSearchWidow(QWidget *parent);
    QList<SIM::IMGroupPtr> groups();
    QList<SIM::IMContactPtr> contacts();

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(Buffer* cfg);

    void setUin(unsigned long);
    void setScreen(const QString &);
    unsigned long getUin();

    QString getServer() const;
    void setServer(const QString &);

    unsigned short getPort() const;
    void setPort(unsigned short port);

    unsigned long getContactsTime() const;
    void setContactsTime(unsigned long contactsTime);

    unsigned short getContactsLength() const;
    void setContactsLength(unsigned short contactsLength);

    unsigned short getContactsInvisible() const;
    void setContactsInvisible(unsigned short contactsInvisible);

    bool getHideIP() const;
    void setHideIP(bool hideip);

    bool getIgnoreAuth() const;
    void setIgnoreAuth(bool ignoreAuth);

    bool getUseMD5() const;
    void setUseMD5(bool usemd5);

    unsigned long getDirectMode();
    void setDirectMode(unsigned long mode);

    unsigned long getIdleTime() const;
    void setIdleTime(unsigned long time);

    QString getListRequests() const;
    void setListRequests(const QString& listrequests);

    QString getPicture() const;
    void setPicture(const QString& pic);

    unsigned long getRandomChatGroup() const;
    void setRandomChatGroup(unsigned long group);

    unsigned long getRandomChatGroupCurrent() const;
    void setRandomChatGroupCurrent(unsigned long group);

    unsigned long getSendFormat() const;
    void setSendFormat(unsigned long format);

    bool getDisablePlugins() const;
    void setDisablePlugins(bool b);

    bool getDisableAutoUpdate() const;
    void setDisableAutoUpdate(bool b);

    bool getDisableAutoReplyUpdate() const;
    void setDisableAutoReplyUpdate(bool b);

    bool getDisableTypingNotification() const;
    void setDisableTypingNotification(bool b);

    bool getAcceptInDND() const;
    void setAcceptInDND(bool b);

    bool getAcceptInOccupied() const;
    void setAcceptInOccupied(bool b);

    unsigned long getMinPort() const;
    void setMinPort(unsigned long port);

    unsigned long getMaxPort() const;
    void setMaxPort(unsigned long port);

    bool getWarnAnonymously() const;
    void setWarnAnonymously(bool b);

    unsigned long getAckMode() const;
    void setAckMode(unsigned long mode);

    bool getUseHTTP() const;
    void setUseHTTP(bool b);

    bool getAutoHTTP() const;
    void setAutoHTTP(bool b);

    bool getKeepAlive() const;
    void setKeepAlive(bool b);

    bool getMediaSense() const;
    void setMediaSense(bool b);

    ICQClientData* clientPersistentData;

    void initDefaultStates();
    ICQStatusPtr getDefaultStatus(const QString& id);

    void setOscarSocket(OscarSocket* socket);
    OscarSocket* oscarSocket() const;

    SnacHandler* snacHandler(int type);

    bool isAim() const;

    // reimplement socket() to get correct Buffer
//    virtual ICQClientSocket *socket() { return static_cast<ICQClientSocket*>(TCPClient::socket()); }
//    virtual ICQClientSocket *createClientSocket() { return new ICQClientSocket(this, createSocket()); }
//    // icq functions
//    unsigned short findByUin(unsigned long uin);
//    unsigned short findByMail(const QString &mail);
//    unsigned short findWP(const QString &first, const QString &last, const QString &nick,
//                          const QString &email, char age, char nGender,
//                          unsigned short nLanguage, const QString &city, const QString &szState,
//                          unsigned short nCountryCode,
//                          const QString &cCoName, const QString &szCoDept, const QString &szCoPos,
//                          unsigned short nOccupation,
//                          unsigned short nPast, const QString &szPast,
//                          unsigned short nInterests, const QString &szInterests,
//                          unsigned short nAffiliation, const QString &szAffiliation,
//                          unsigned short nHomePoge, const QString &szHomePage,
//                          const QString &sKeyWord, bool bOnlineOnly);
//    SIM::Contact *getContact(ICQUserData*);
//    ICQUserData *findContact(unsigned long uin,     const QString *alias, bool bCreate, SIM::Contact *&contact, SIM::Group *grp=NULL, bool bJoin=true);
//    ICQUserData *findContact(const QString &screen, const QString *alias, bool bCreate, SIM::Contact *&contact, SIM::Group *grp=NULL, bool bJoin=true);
//    ICQUserData *findGroup(unsigned id, const QString *name, SIM::Group *&group);
//    void addFullInfoRequest(unsigned long uin);
//    ListRequest *findContactListRequest(const QString &screen);
//    ListRequest *findGroupListRequest(unsigned short id);
//    void removeListRequest(ListRequest *lr);
//    virtual void setupContact(SIM::Contact*, void *data);
//    QString clientName(ICQUserData*);
//    void changePassword(const QString &new_pswd);
//    void searchChat(unsigned short);
//    void randomChatInfo(unsigned long uin);
//    unsigned short aimEMailSearch(const QString &name);
//    unsigned short aimInfoSearch(const QString &first, const QString &last, const QString &middle,
//                                 const QString &maiden, const QString &country, const QString &street,
//                                 const QString &city, const QString &nick, const QString &zip,
//                                 const QString &state);
//    SIM::Message *parseMessage(unsigned short type, const QString &screen,
//                          const QByteArray &p, ICQBuffer &packet, MessageId &id, unsigned cookie);
//    void sendPacket(bool bSend);
//    bool messageReceived(SIM::Message*, const QString &screen);
//    static bool parseRTF(const QByteArray &str, SIM::Contact *contact, QString &result);
//    static QString pictureFile(const ICQUserData *data);
//    virtual QImage userPicture(unsigned id);
//    QImage userPicture(ICQUserData *d);
//    static const capability *capabilities;
//    static const plugin *plugins;
//    static QString convert(Tlv *tlvInfo, TlvList &tlvs, unsigned n);
//    static QString convert(const char *text, unsigned size, TlvList &tlvs, unsigned n);
//    static QString screen(const ICQUserData*);
//    static unsigned long warnLevel(unsigned long);
//    static unsigned clearTags(QString &text);
	bool m_bAIM;
//    static QString addCRLF(const QString &str);
//    void uploadBuddy(const ICQUserData *data);
//    ICQUserData * toICQUserData(SIM::IMContact*);  // More safely type conversion from generic SIM::clientData into ICQUserData

//    unsigned long getFullStatus();

//    bool addSnacHandler(SnacHandler* handler);
//    void clearSnacHandlers();

//    // ICBM:
//    void deleteFileMessage(MessageId const& cookie);

//    // SSI:
//    void ssiStartTransaction();
//    void ssiEndTransaction();
//    bool isSSITransaction(){return false;}
//    unsigned short ssiAddBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs);
//    unsigned short ssiModifyBuddy(const QString& name, unsigned short grp_id, unsigned short usr_id, unsigned short subCmd, TlvList* tlv);
//    unsigned short ssiDeleteBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs);
//    void getGroupIDs(unsigned short group_id, ICQBuffer* buf); // hack
//    unsigned short ssiAddToGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id);
//    unsigned short ssiRemoveFromGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id);
//    TlvList *createListTlv(ICQUserData *data, SIM::Contact *contact);

//    // Snac handlers accessors
//    SnacIcqService* snacService() { return m_snacService; }
//    SnacIcqBuddy* snacBuddy() { return m_snacBuddy; }
//    SnacIcqICBM* snacICBM() { return m_snacICBM; }

protected slots:
    void oscarSocketConnected();
    void oscarSocketPacket(int channel, const QByteArray& data);
//    void ping();
//    void retry(int n, void*);
//    void interfaceDown(QString);
//    void interfaceUp(QString);
signals:
    void setStatusWidgetsBlinking(bool b);

protected:
//    void generateCookie(MessageId& id);

//    virtual void setInvisible(bool bState);
//    virtual void setStatus(unsigned status, bool bCommon);
//    virtual void setStatus(unsigned status);
//    virtual void disconnected();
//    virtual bool processEvent(SIM::Event *e);
//    virtual bool compareData(void*, void*);
//    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
//    virtual bool send(SIM::Message*, void*);
//    virtual bool canSend(unsigned type, void*);
//    virtual bool isMyData(SIM::IMContact*&, SIM::Contact*&);
//    virtual bool createData(SIM::IMContact*&, SIM::Contact*);
//    virtual QString contactTip(void *_data);
//    virtual SIM::CommandDef *infoWindows(SIM::Contact *contact, void *_data);
//    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact *contact, void *_data, unsigned id);
//    virtual SIM::CommandDef *configWindows();
//    virtual QWidget *configWindow(QWidget *parent, unsigned id);
//    virtual QWidget *searchWindow(QWidget *parent);
//    virtual void updateInfo(SIM::Contact *contact, void *_data);
//    virtual void setClientInfo(SIM::IMContact* data);
//    virtual SIM::Socket  *createSocket();
//    virtual QString contactName(void *clientData);
//    QString dataName(const QString &screen);
//    virtual void packet(unsigned long size);
//    void snac_location(unsigned short, unsigned short);
//    void snac_bos(unsigned short, unsigned short);
//    void snac_ping(unsigned short, unsigned short);
//    void snac_lists(unsigned short, unsigned short);
//    void snac_various(unsigned short, unsigned short);
//    void snac_login(unsigned short, unsigned short);
//    void parseRosterItem(unsigned short type, const QString &str,unsigned short grp_id,
//                         unsigned short id, TlvList *inf, bool &bIgnoreTime);
//    void listsRequest();
//    void locationRequest();
//    void buddyRequest();
//    void bosRequest();
//    void addCapability(ICQBuffer &cap, cap_id_t id);   // helper for sendCapability()
//    void sendCapability(const QString &msg=QString::null);
//    void sendMessageRequest();
//    void serverRequest(unsigned short cmd, unsigned short seq=0);
//    void sendServerRequest();
//    void sendInvisible(bool bState);
//    void sendContactList();
//    void setOffline(ICQUserData*);
//    void removeFullInfoRequest(unsigned long uin);
//    class SSBISocket *getSSBISocket();
//    unsigned long fullStatus(unsigned status);
//    unsigned long fullStatus(const SIM::IMStatusPtr& status);
//    virtual void connect_ready();
//    virtual void packet_ready();
//    const char* error_message(unsigned short error);
//    bool m_bVerifying;
//    ICQListener			*m_listener;
//    QTimer *m_processTimer;
//    unsigned short m_sendSmsId;
//    unsigned short m_offlineMessagesRequestId;
//    ListServerRequest *m_listRequest;
//    bool m_bRosters;
//    bool m_bBirthday;
//    bool m_bNoSend;
//    std::list<ServerRequest*> varRequests;
//    std::list<InfoRequest>	infoRequests;
//    QStringList         	buddies;
//    std::list<ListRequest>	listRequests;
//    std::list<ar_request>	arRequests;
//    void addGroupRequest(SIM::Group *group);
//    void addContactRequest(SIM::Contact *contact);
//    void checkListRequest();
//    void checkInfoRequest();
//    ServerRequest *findServerRequest(unsigned short id);
//    void clearServerRequests();
//    void clearListServerRequest();
//    void clearSMSQueue();
//    unsigned processListRequest();
//    unsigned processSMSQueue();
//    unsigned processInfoRequest();
//    static bool hasCap(const ICQUserData *data, cap_id_t fcap);
//    static void setCap(ICQUserData *data, cap_id_t fcap);
//    bool isSupportPlugins(ICQUserData *data);
//    QString trimPhone(const QString &phone);
//    unsigned short getListId();
//    unsigned short sendRoster(unsigned short cmd, const QString &name,
//                              unsigned short grp_id,  unsigned short usr_id,
//                              unsigned short subCmd=0, TlvList *tlv = NULL);
//    void sendRosterGrp(const QString &name, unsigned short grpId, unsigned short usrId);
//    bool isContactRenamed(ICQUserData *data, SIM::Contact *contact);
//    QString getUserCellular(SIM::Contact *contact);
//    void setMainInfo(ICQUserData *d);
//    void setAIMInfo(ICQUserData *data);
//    void setProfile(ICQUserData *data);
//    bool isOwnData(const QString &screen);
//    void packInfoList(const QString &str);
//    QString packContacts(SIM::ContactsMessage *msg, ICQUserData *data, CONTACTS_MAP &c);
//    QByteArray createRTF(QString &text, QString &part, unsigned long foreColor, SIM::Contact *contact, unsigned max_size);
//    QString removeImages(const QString &text, bool icqSmiles);
//    bool sendAuthRequest(SIM::Message *msg, void *data);
//    bool sendAuthGranted(SIM::Message *msg, void *data);
//    bool sendAuthRefused(SIM::Message *msg, void *data);
//    void parseAdvancedMessage(const QString &screen, ICQBuffer &msg, bool needAck, MessageId id);
//    void addPluginInfoRequest(unsigned long uin, unsigned plugin_index);
//    void setChatGroup();
//    SIM::Message *parseExtendedMessage(const QString &screen, ICQBuffer &packet, MessageId &id, unsigned cookie);
//    void parsePluginPacket(ICQBuffer &b, unsigned plugin_index, ICQUserData *data, unsigned uin, bool bDirect);
//    void pluginAnswer(unsigned plugin_type, unsigned long uin, ICQBuffer &b);
//    void packMessage(ICQBuffer &b, SIM::Message *msg, ICQUserData *data, unsigned short &type, bool bDirect, unsigned short flags=ICQ_TCPxMSG_NORMAL);
//    void packExtendedMessage(SIM::Message *msg, ICQBuffer &buf, ICQBuffer &msgBuf, ICQUserData *data);
//    void fetchProfile(ICQUserData *data);
//    void fetchAwayMessage(ICQUserData *data);
//    void fetchProfiles();
//    void setAwayMessage(const QString &msg = QString::null);
//    void encodeString(const QString &text, const QString &type, unsigned short charsetTlv, unsigned short infoTlv);
//    void encodeString(const QString &_str, unsigned short nTlv, bool bWide);
//    bool processMsg();
//    void packTlv(unsigned short tlv, unsigned short code, const QString &keywords);
//    void packTlv(unsigned short tlv, const QString &_data);
//    void packTlv(unsigned short tlv, unsigned short data);
//    void uploadBuddyIcon(unsigned short refNumber, const QImage &img);
//    void requestBuddy(const ICQUserData *data);
//    ICQUserData *findInfoRequest(unsigned short seq, SIM::Contact *&contact);
//    INFO_REQ_MAP m_info_req;
//    unsigned short msgStatus();
//    unsigned short m_advCounter;
//    bool     m_bJoin;
//    bool	 m_bFirstTry;
//    bool	 m_bHTTP;
//    bool	 m_bReady;
//    std::vector<RateInfo> m_rates;
//    RATE_MAP			m_rate_grp;
//    void                        setNewLevel(RateInfo &r);
//    unsigned			delayTime(unsigned snac);
//    unsigned			delayTime(RateInfo &r);
//    RateInfo			*rateInfo(unsigned snac);
//    std::list<SIM::Message*>	m_processMsg;
//    std::list<DirectSocket*>	m_sockets;
//    std::list<SIM::Message*>	m_acceptMsg;
//    std::list<AIMFileTransfer*> m_filetransfers;
//    SnacIcqBuddy* m_snacBuddy;
//    SnacIcqService* m_snacService;
//    SnacIcqICBM* m_snacICBM;
//    bool m_bconnectionLost;

//    friend class ListServerRequest;
//    friend class FullInfoRequest;
//    friend class SMSRequest;
//    friend class ServiceSocket;
//    friend class DirectSocket;
//    friend class DirectClient;
//    friend class ICQListener;
//    friend class AIMFileTransfer;
//    friend class ICQFileTransfer;
//    friend class SetBuddyRequest;
//    friend class SSBISocket;

//    // This should be removed when refactoring is over
//    friend class SnacIcqBuddy;
//    friend class SnacIcqService;
//    friend class SnacIcqICBM;

private:
    void initialize(bool bAIM);
    void initSnacHandlers();

    SIM::PropertyHubPtr m_propertyHub;
    QList<ICQStatusPtr> m_defaultStates;
    QString m_name;
    ICQStatusPtr m_currentStatus;
    OscarSocket* m_oscarSocket;

    enum State
    {
        sOffline,
        sConnecting,
        sConnected
    };

    State m_state;

    AuthorizationSnacHandler* m_authSnac;
    ServiceSnacHandler* m_serviceSnac;
    mapSnacHandlers m_snacHandlers;

    //friend class AuthorizationSnacHandler;

    //bool m_bBirthdayInfoDisplayed;
};

//class ServiceSocket : public SIM::ClientSocketNotify, public OscarSocket
//{
//public:
//    ServiceSocket(ICQClient*, unsigned short id);
//    ~ServiceSocket();
//    unsigned short id() const { return m_id; }
//    void connect(const char *addr, unsigned short port, const QByteArray &cookie);
//    virtual bool error_state(const QString &err, unsigned code = 0);
//    bool connected() const { return m_bConnected; }
//    void close();
//protected:
//    virtual const char *serviceSocketName() = 0;
//    virtual void connect_ready();
//    virtual void packet_ready();
//    virtual ICQClientSocket *socket() { return m_socket; }
//    virtual void packet(unsigned long size);
//    virtual void data(unsigned short food, unsigned short type, unsigned short seq) = 0;
//    unsigned short m_id;
//    QByteArray  m_cookie;
//    bool    m_bConnected;
//    ICQClientSocket *m_socket;
//    ICQClient *m_client;
//};

//class DirectSocket : public QObject, public SIM::ClientSocketNotify
//{
//    Q_OBJECT
//public:
//    enum SocketState{
//        NotConnected,
//        ConnectIP1,
//        ConnectIP2,
//        ConnectFail,
//        WaitInit,
//        WaitAck,
//        WaitReverse,
//        ReverseConnect,
//        Logged
//    };
//    DirectSocket(SIM::Socket *s, ICQClient*, unsigned long ip);
//    DirectSocket(ICQUserData *data, ICQClient *client);
//    ~DirectSocket();
//    virtual void packet_ready();
//    SocketState m_state;
//    void connect();
//    void reverseConnect(unsigned long ip, unsigned short port);
//    void acceptReverse(SIM::Socket *s);
//    virtual bool   error_state(const QString &err, unsigned code = 0);
//    virtual void   connect_ready();
//    unsigned short localPort();
//    unsigned short remotePort();
//    unsigned long  Uin();
//    ICQUserData    *m_data;
//    void setPort(unsigned short port) {m_port = port;}
//protected slots:
//    void timeout();
//protected:
//    virtual void login_timeout();
//    virtual void processPacket() = 0;
//    void init();
//    void sendInit();
//    void sendInitAck();
//    void removeFromClient();
//    bool             m_bIncoming;
//    unsigned short   m_nSequence;
//    unsigned short   m_port;
//    unsigned short	 m_localPort;
//    char             m_version;
//    bool			 m_bHeader;
//    unsigned long    m_nSessionId;
//    ICQClientSocket  *m_socket;
//    ICQClient        *m_client;
//    unsigned long m_ip;
//    friend class AIMFileTransfer;
//};

//struct SendDirectMsg
//{
//    SIM::Message        *msg;
//    unsigned            type;
//    unsigned short      seq;
//    unsigned short      icq_type;
//};

//class DirectClient : public DirectSocket
//{
//public:
//    DirectClient(SIM::Socket *s, ICQClient *client, unsigned long ip);
//    DirectClient(ICQUserData *data, ICQClient *client, unsigned channel = PLUGIN_NULL);
//    ~DirectClient();
//    bool sendMessage(SIM::Message*);
//    void acceptMessage(SIM::Message*);
//    void declineMessage(SIM::Message*, const QString &reason);
//    bool cancelMessage(SIM::Message*);
//    void sendAck(unsigned short, unsigned short msgType, unsigned short msgFlags,
//                 const char *message=NULL, unsigned short status=ICQ_TCPxACK_ACCEPT, SIM::Message *m=NULL);
//    bool isLogged() { return (m_state != None) && (m_state != WaitInit2); }
//    bool isSecure();
//    void addPluginInfoRequest(unsigned plugin_index);
//protected:
//    enum State{
//        None,
//        WaitLogin,
//        WaitInit2,
//        Logged,
//        SSLconnect
//    };
//    State       m_state;
//    unsigned    m_channel;
//    void processPacket();
//    void connect_ready();
//    virtual bool error_state(const QString &err, unsigned code);
//    void sendInit2();
//    void startPacket(unsigned short cms, unsigned short seq);
//    void sendPacket();
//    void processMsgQueue();
//    bool copyQueue(DirectClient *to);
//    QList<SendDirectMsg> m_queue;
//    QString name();
//    QString m_name;
//    void secureConnect();
//    void secureListen();
//    void secureStop(bool bShutdown);
//    SIM::SSLClient *m_ssl;
//};

//class ICQFileTransfer : public SIM::FileTransfer, public DirectSocket, public SIM::ServerSocketNotify
//{
//public:
//    ICQFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client);
//    ~ICQFileTransfer();
//    void connect(unsigned short port);
//    void listen();
//    void setSocket(ICQClientSocket *socket);
//    virtual void processPacket();
//protected:
//    enum State
//    {
//        None,
//        WaitLogin,
//        WaitInit,
//        InitSend,
//        InitReceive,
//        Send,
//        Receive,
//        Wait,
//        WaitReverse,
//        WaitReverseLogin,
//        Listen
//    };
//    State m_state;

//    virtual void connect_ready();
//    virtual bool error_state(const QString &err, unsigned code);
//    virtual void write_ready();
//    virtual void setSpeed(unsigned speed);
//    virtual void startReceive(unsigned pos);
//    virtual void bind_ready(unsigned short port);
//    virtual bool accept(SIM::Socket *s, unsigned long ip);
//    virtual bool error(const QString &err);
//    virtual void login_timeout();

//    void sendInit();
//    void startPacket(char cmd);
//    void sendPacket(bool dump=true);
//    void sendFileInfo();
//    void initReceive(char cmd);

//    friend class ICQClient;
//};

#endif


