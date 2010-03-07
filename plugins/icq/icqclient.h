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
#include <climits>

#include "misc.h"
#include "snac.h"
#include "icqbuddy.h"
#include "icqservice.h"
#include "icqicmb.h"

#include "socket/socket.h"
#include "socket/serversocketnotify.h"
#include "socket/clientsocket.h"
#include "socket/tcpclient.h"
#include "socket/interfacechecker.h"
#include "socket/sslclient.h"
#include "contacts/clientdataiterator.h"
#include "icq.h"
#include "icqbuffer.h"

const unsigned ICQ_SIGN			= 0x0001;

const unsigned MESSAGE_DIRECT    = 0x0100;

//const unsigned STATUS_INVISIBLE	  = 2;     //took over to contacts.h
//const unsigned STATUS_OCCUPIED    = 100;  //took over to contacts.h

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

class AIMFileTransfer;
class DirectClient;

struct ICQUserData : public SIM::clientData
{
	SIM::Data        Alias;
	SIM::Data        Cellular;
	SIM::Data		Status;
	SIM::Data		Class;
	SIM::Data		StatusTime;
	SIM::Data		OnlineTime;
	SIM::Data		WarningLevel;
	SIM::Data        IP;
	SIM::Data        RealIP;
	SIM::Data		Port;
	SIM::Data		DCcookie;
	SIM::Data		Caps;
	SIM::Data		Caps2;
	SIM::Data        AutoReply;
	SIM::Data		Uin;
	SIM::Data        Screen;
	SIM::Data		IcqID;
	SIM::Data		bChecked;
	SIM::Data		GrpId;
	SIM::Data		IgnoreId;
	SIM::Data		VisibleId;
	SIM::Data		ContactVisibleId;
	SIM::Data		InvisibleId;
	SIM::Data		ContactInvisibleId;
	SIM::Data		WaitAuth;
	SIM::Data		WantAuth;
	SIM::Data		WebAware;
	SIM::Data		InfoUpdateTime;
	SIM::Data		PluginInfoTime;
	SIM::Data		PluginStatusTime;
	SIM::Data		InfoFetchTime;
	SIM::Data		PluginInfoFetchTime;
	SIM::Data		PluginStatusFetchTime;
	SIM::Data		Mode;
	SIM::Data		Version;
	SIM::Data		Build;
	SIM::Data        Nick;
	SIM::Data        FirstName;
	SIM::Data        LastName;
	SIM::Data		MiddleName;
	SIM::Data		Maiden;
	SIM::Data        EMail;
	SIM::Data		HiddenEMail;
	SIM::Data        City;
	SIM::Data        State;
	SIM::Data        HomePhone;
	SIM::Data        HomeFax;
	SIM::Data        Address;
	SIM::Data        PrivateCellular;
	SIM::Data        Zip;
	SIM::Data		Country;
	SIM::Data		TimeZone;
	SIM::Data		Age;
	SIM::Data		Gender;
	SIM::Data        Homepage;
	SIM::Data		BirthYear;
	SIM::Data		BirthMonth;
	SIM::Data		BirthDay;
	SIM::Data		Language;
	SIM::Data        EMails;
	SIM::Data        WorkCity;
	SIM::Data        WorkState;
	SIM::Data        WorkPhone;
	SIM::Data        WorkFax;
	SIM::Data        WorkAddress;
	SIM::Data        WorkZip;
	SIM::Data		WorkCountry;
	SIM::Data        WorkName;
	SIM::Data        WorkDepartment;
	SIM::Data        WorkPosition;
	SIM::Data		Occupation;
	SIM::Data        WorkHomepage;
	SIM::Data        About;
	SIM::Data        Interests;
	SIM::Data        Backgrounds;
	SIM::Data        Affilations;
	SIM::Data		FollowMe;
	SIM::Data		SharedFiles;
	SIM::Data		ICQPhone;
	SIM::Data        Picture;
	SIM::Data		PictureWidth;
	SIM::Data		PictureHeight;
	SIM::Data        PhoneBook;
	SIM::Data		ProfileFetch;
	SIM::Data		bTyping;
	SIM::Data		bBadClient;
	SIM::Data		Direct;
	SIM::Data		DirectPluginInfo;
	SIM::Data		DirectPluginStatus;
	SIM::Data		bNoDirect;
	SIM::Data		bInvisible;
	SIM::Data       buddyRosterID;
	SIM::Data       buddyID;
	SIM::Data       buddyHash;
	SIM::Data		unknown2;
	SIM::Data		unknown4;
	SIM::Data		unknown5;
};

struct ICQClientData
{
    SIM::Data        Server;
    SIM::Data		Port;
    SIM::Data		ContactsTime;
    SIM::Data		ContactsLength;
    SIM::Data		ContactsInvisible;
    SIM::Data		HideIP;
    SIM::Data		IgnoreAuth;
    SIM::Data		UseMD5;
    SIM::Data		DirectMode;
    SIM::Data		IdleTime;
    SIM::Data        ListRequests;
    SIM::Data        Picture;
    SIM::Data		RandomChatGroup;
    SIM::Data		RandomChatGroupCurrent;
    SIM::Data		SendFormat;
    SIM::Data		DisablePlugins;
    SIM::Data		DisableAutoUpdate;
    SIM::Data		DisableAutoReplyUpdate;
    SIM::Data		DisableTypingNotification;
//    Data		AutoCheckInvisible;
//    Data		CheckInvisibleInterval;
    SIM::Data		AcceptInDND;
    SIM::Data		AcceptInOccupied;
    SIM::Data		MinPort;
    SIM::Data		MaxPort;
    SIM::Data		WarnAnonimously;
    SIM::Data		AckMode;
    SIM::Data		UseHTTP;
    SIM::Data		AutoHTTP;
    SIM::Data		KeepAlive;
	SIM::Data		MediaSense;
    ICQUserData	owner;
};

class ICQClient;

struct SearchResult
{
    ICQUserData       data;
    unsigned short    id;
    ICQClient         *client;
};

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
    CAP_NULL,                   // 0x00000400
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

class ListRequest
{
public:
    ListRequest()
        : type(0), icq_id(0), grp_id(0), visible_id(0), invisible_id(0), ignore_id(0),
          icqUserData(NULL) {}

public:
    unsigned          type;
    QString           screen;
    unsigned short    icq_id;
    unsigned short    grp_id;
    unsigned short    visible_id;
    unsigned short    invisible_id;
    unsigned short    ignore_id;
    const ICQUserData *icqUserData;
};

class ICQListener : public SIM::ServerSocketNotify
{
public:
    ICQListener(ICQClient *client);
    ~ICQListener();
protected:
    virtual bool accept(SIM::Socket *s, unsigned long ip);
    virtual void bind_ready(unsigned short port);
    virtual bool error(const QString &err);
    ICQClient  *m_client;
};

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

struct ar_request
{
    unsigned short    type;
    unsigned short    flags;
    unsigned short    ack;
    MessageId         id;
    unsigned short    id1;
    unsigned short    id2;
    QString           screen;
    bool              bDirect;
};

typedef std::map<unsigned short, QString> INFO_REQ_MAP;

class DirectSocket;
class ServiceSocket;
class ICQClientSocket;

class OscarSocket
{
public:
    OscarSocket();
    virtual ~OscarSocket(){};

    void snac(unsigned short food, unsigned short type, bool msgId=false, bool bType=true);
    void sendPacket(bool bSend = true);
protected:
    virtual ICQClientSocket *socket() = 0;
    virtual void packet(unsigned long size) = 0;
    void flap(char channel);
    void connect_ready();
    void packet_ready();
    bool m_bHeader;
    char m_nChannel;
    unsigned short m_nFlapSequence;
    unsigned short m_nMsgSequence;
};

struct alias_group
{
    QString     alias;
    unsigned    grp;
};

struct RateInfo
{
    Buffer				delayed;
    QDateTime			m_lastSend;
    unsigned			m_curLevel;
    unsigned			m_maxLevel;
    unsigned			m_minLevel;
    unsigned			m_winSize;
};

struct InfoRequest
{
    unsigned	uin;
    unsigned	request_id;
    unsigned	start_time;
};

typedef std::map<SIM::my_string, alias_group>	CONTACTS_MAP;
typedef std::map<unsigned, unsigned>			RATE_MAP;
typedef std::map<unsigned short, SnacHandler*> mapSnacHandlers;

class ICQClient : public SIM::TCPClient, public OscarSocket
{
    Q_OBJECT
public:
    ICQClient(SIM::Protocol*, Buffer *cfg, bool bAIM);
    ~ICQClient();
    virtual QString     name();
    virtual QString getScreen();
    virtual QString     dataName(void*);
    virtual QWidget    *setupWnd();
    virtual QByteArray  getConfig();
    virtual void contactsLoaded();
    void setUin(unsigned long);
    void setScreen(const QString &);
    unsigned long getUin();
    QString getServer() const;
    void setServer(const QString &);
    PROP_USHORT(Port);
    PROP_ULONG(ContactsTime);
    PROP_USHORT(ContactsLength);
    PROP_USHORT(ContactsInvisible);
    PROP_BOOL(HideIP);
    PROP_BOOL(IgnoreAuth);
    PROP_BOOL(UseMD5);
    PROP_ULONG(DirectMode);
    PROP_ULONG(IdleTime);
    PROP_STR(ListRequests);
    PROP_UTF8(Picture);
    PROP_USHORT(RandomChatGroup);
    PROP_ULONG(RandomChatGroupCurrent);
    PROP_ULONG(SendFormat);
    PROP_BOOL(DisablePlugins);
    PROP_BOOL(DisableAutoUpdate);
    PROP_BOOL(DisableAutoReplyUpdate);
    PROP_BOOL(DisableTypingNotification);
    PROP_BOOL(AcceptInDND);
    PROP_BOOL(AcceptInOccupied);
    PROP_USHORT(MinPort);
    PROP_USHORT(MaxPort);
    PROP_BOOL(WarnAnonimously);
    PROP_USHORT(AckMode);
    PROP_BOOL(UseHTTP);
    PROP_BOOL(AutoHTTP);
    PROP_BOOL(KeepAlive);
	PROP_BOOL(MediaSense);
    ICQClientData   data;
    // reimplement socket() to get correct Buffer
    virtual ICQClientSocket *socket() { return static_cast<ICQClientSocket*>(TCPClient::socket()); }
    virtual ICQClientSocket *createClientSocket() { return new ICQClientSocket(this, createSocket()); }
    // icq functions
    unsigned short findByUin(unsigned long uin);
    unsigned short findByMail(const QString &mail);
    unsigned short findWP(const QString &first, const QString &last, const QString &nick,
                          const QString &email, char age, char nGender,
                          unsigned short nLanguage, const QString &city, const QString &szState,
                          unsigned short nCountryCode,
                          const QString &cCoName, const QString &szCoDept, const QString &szCoPos,
                          unsigned short nOccupation,
                          unsigned short nPast, const QString &szPast,
                          unsigned short nInterests, const QString &szInterests,
                          unsigned short nAffiliation, const QString &szAffiliation,
                          unsigned short nHomePoge, const QString &szHomePage,
                          const QString &sKeyWord, bool bOnlineOnly);
    SIM::Contact *getContact(ICQUserData*);
    ICQUserData *findContact(unsigned long uin,     const QString *alias, bool bCreate, SIM::Contact *&contact, SIM::Group *grp=NULL, bool bJoin=true);
    ICQUserData *findContact(const QString &screen, const QString *alias, bool bCreate, SIM::Contact *&contact, SIM::Group *grp=NULL, bool bJoin=true);
    ICQUserData *findGroup(unsigned id, const QString *name, SIM::Group *&group);
    void addFullInfoRequest(unsigned long uin);
    ListRequest *findContactListRequest(const QString &screen);
    ListRequest *findGroupListRequest(unsigned short id);
    void removeListRequest(ListRequest *lr);
    virtual void setupContact(SIM::Contact*, void *data);
    QString clientName(ICQUserData*);
    void changePassword(const QString &new_pswd);
    void searchChat(unsigned short);
    void randomChatInfo(unsigned long uin);
    unsigned short aimEMailSearch(const QString &name);
    unsigned short aimInfoSearch(const QString &first, const QString &last, const QString &middle,
                                 const QString &maiden, const QString &country, const QString &street,
                                 const QString &city, const QString &nick, const QString &zip,
                                 const QString &state);
    SIM::Message *parseMessage(unsigned short type, const QString &screen,
                          const QByteArray &p, ICQBuffer &packet, MessageId &id, unsigned cookie);
    void sendPacket(bool bSend);
    bool messageReceived(SIM::Message*, const QString &screen);
    static bool parseRTF(const QByteArray &str, SIM::Contact *contact, QString &result);
    static QString pictureFile(const ICQUserData *data);
    virtual QImage userPicture(unsigned id);
    QImage userPicture(ICQUserData *d);
    static const capability *capabilities;
    static const plugin *plugins;
    static QString convert(Tlv *tlvInfo, TlvList &tlvs, unsigned n);
    static QString convert(const char *text, unsigned size, TlvList &tlvs, unsigned n);
    static QString screen(const ICQUserData*);
    static unsigned long warnLevel(unsigned long);
    static unsigned clearTags(QString &text);
    bool m_bAIM;
    static QString addCRLF(const QString &str);
    void uploadBuddy(const ICQUserData *data);
    ICQUserData * toICQUserData(SIM::clientData*);  // More safely type conversion from generic SIM::clientData into ICQUserData

    virtual void changeStatus(const SIM::IMStatusPtr& status);

    unsigned long getFullStatus();

    bool addSnacHandler(SnacHandler* handler);
    void clearSnacHandlers();

    // ICBM:
    void deleteFileMessage(MessageId const& cookie);

    // SSI:
    void ssiStartTransaction();
    void ssiEndTransaction();
    bool isSSITransaction(){return false;}
    unsigned short ssiAddBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs);
    unsigned short ssiModifyBuddy(const QString& name, unsigned short grp_id, unsigned short usr_id, unsigned short subCmd, TlvList* tlv);
    unsigned short ssiDeleteBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs);
    void getGroupIDs(unsigned short group_id, ICQBuffer* buf); // hack
    unsigned short ssiAddToGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id);
    unsigned short ssiRemoveFromGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id);
    TlvList *createListTlv(ICQUserData *data, SIM::Contact *contact);

    // Snac handlers accessors
    SnacIcqService* snacService() { return m_snacService; }
    SnacIcqBuddy* snacBuddy() { return m_snacBuddy; }
    SnacIcqICBM* snacICBM() { return m_snacICBM; }

protected slots:
    void ping();
    void retry(int n, void*);
    void interfaceDown(QString);
    void interfaceUp(QString);
protected:
    void generateCookie(MessageId& id);

    virtual void setInvisible(bool bState);
    virtual void setStatus(unsigned status, bool bCommon);
    virtual void setStatus(unsigned status);
    virtual void disconnected();
    virtual bool processEvent(SIM::Event *e);
    virtual bool compareData(void*, void*);
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
    virtual bool send(SIM::Message*, void*);
    virtual bool canSend(unsigned type, void*);
    virtual bool isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool createData(SIM::clientData*&, SIM::Contact*);
    virtual QString contactTip(void *_data);
    virtual SIM::CommandDef *infoWindows(SIM::Contact *contact, void *_data);
    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact *contact, void *_data, unsigned id);
    virtual SIM::CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget *searchWindow(QWidget *parent);
    virtual void updateInfo(SIM::Contact *contact, void *_data);
    virtual void setClientInfo(void *data);
    virtual SIM::Socket  *createSocket();
    virtual QString contactName(void *clientData);
    QString dataName(const QString &screen);
    QByteArray  m_cookie;
    virtual void packet(unsigned long size);
    void snac_location(unsigned short, unsigned short);
    void snac_bos(unsigned short, unsigned short);
    void snac_ping(unsigned short, unsigned short);
    void snac_lists(unsigned short, unsigned short);
    void snac_various(unsigned short, unsigned short);
    void snac_login(unsigned short, unsigned short);
    void parseRosterItem(unsigned short type, const QString &str,unsigned short grp_id,
                         unsigned short id, TlvList *inf, bool &bIgnoreTime);
    void chn_login();
    void chn_close();
    void listsRequest();
    void locationRequest();
    void buddyRequest();
    void bosRequest();
    void addCapability(ICQBuffer &cap, cap_id_t id);   // helper for sendCapability()
    void sendCapability(const QString &msg=QString::null);
    void sendMessageRequest();
    void serverRequest(unsigned short cmd, unsigned short seq=0);
    void sendServerRequest();
    void sendInvisible(bool bState);
    void sendContactList();
    void setOffline(ICQUserData*);
    void removeFullInfoRequest(unsigned long uin);
    class SSBISocket *getSSBISocket();
    unsigned long fullStatus(unsigned status);
    unsigned long fullStatus(const SIM::IMStatusPtr& status);
    QByteArray cryptPassword();
    virtual void connect_ready();
    virtual void packet_ready();
    const char* error_message(unsigned short error);
    bool m_bVerifying;
    ICQListener			*m_listener;
    QTimer *m_processTimer;
    unsigned short m_sendSmsId;
    unsigned short m_offlineMessagesRequestId;
    ListServerRequest *m_listRequest;
    bool m_bRosters;
    bool m_bBirthday;
    bool m_bNoSend;
    std::list<ServerRequest*> varRequests;
    std::list<InfoRequest>	infoRequests;
    QStringList         	buddies;
    std::list<ListRequest>	listRequests;
    std::list<ar_request>	arRequests;
    void addGroupRequest(SIM::Group *group);
    void addContactRequest(SIM::Contact *contact);
    void checkListRequest();
    void checkInfoRequest();
    ServerRequest *findServerRequest(unsigned short id);
    void clearServerRequests();
    void clearListServerRequest();
    void clearSMSQueue();
    unsigned processListRequest();
    unsigned processSMSQueue();
    unsigned processInfoRequest();
    static bool hasCap(const ICQUserData *data, cap_id_t fcap);
    static void setCap(ICQUserData *data, cap_id_t fcap);
    bool isSupportPlugins(ICQUserData *data);
    QString trimPhone(const QString &phone);
    unsigned short getListId();
    unsigned short sendRoster(unsigned short cmd, const QString &name,
                              unsigned short grp_id,  unsigned short usr_id,
                              unsigned short subCmd=0, TlvList *tlv = NULL);
    void sendRosterGrp(const QString &name, unsigned short grpId, unsigned short usrId);
    bool isContactRenamed(ICQUserData *data, SIM::Contact *contact);
    QString getUserCellular(SIM::Contact *contact);
    void setMainInfo(ICQUserData *d);
    void setAIMInfo(ICQUserData *data);
    void setProfile(ICQUserData *data);
    bool isOwnData(const QString &screen);
    void packInfoList(const QString &str);
    QString packContacts(SIM::ContactsMessage *msg, ICQUserData *data, CONTACTS_MAP &c);
    QByteArray createRTF(QString &text, QString &part, unsigned long foreColor, SIM::Contact *contact, unsigned max_size);
    QString removeImages(const QString &text, bool icqSmiles);
    bool sendAuthRequest(SIM::Message *msg, void *data);
    bool sendAuthGranted(SIM::Message *msg, void *data);
    bool sendAuthRefused(SIM::Message *msg, void *data);
    void parseAdvancedMessage(const QString &screen, ICQBuffer &msg, bool needAck, MessageId id);
    void addPluginInfoRequest(unsigned long uin, unsigned plugin_index);
    void setChatGroup();
    SIM::Message *parseExtendedMessage(const QString &screen, ICQBuffer &packet, MessageId &id, unsigned cookie);
    void parsePluginPacket(ICQBuffer &b, unsigned plugin_index, ICQUserData *data, unsigned uin, bool bDirect);
    void pluginAnswer(unsigned plugin_type, unsigned long uin, ICQBuffer &b);
    void packMessage(ICQBuffer &b, SIM::Message *msg, ICQUserData *data, unsigned short &type, bool bDirect, unsigned short flags=ICQ_TCPxMSG_NORMAL);
    void packExtendedMessage(SIM::Message *msg, ICQBuffer &buf, ICQBuffer &msgBuf, ICQUserData *data);
    void fetchProfile(ICQUserData *data);
    void fetchAwayMessage(ICQUserData *data);
    void fetchProfiles();
    void setAwayMessage(const QString &msg = QString::null);
    void encodeString(const QString &text, const QString &type, unsigned short charsetTlv, unsigned short infoTlv);
    void encodeString(const QString &_str, unsigned short nTlv, bool bWide);
    bool processMsg();
    void packTlv(unsigned short tlv, unsigned short code, const QString &keywords);
    void packTlv(unsigned short tlv, const QString &_data);
    void packTlv(unsigned short tlv, unsigned short data);
    void uploadBuddyIcon(unsigned short refNumber, const QImage &img);
    void requestBuddy(const ICQUserData *data);
    ICQUserData *findInfoRequest(unsigned short seq, SIM::Contact *&contact);
    INFO_REQ_MAP m_info_req;
    unsigned short msgStatus();
    unsigned short m_advCounter;
    bool     m_bJoin;
    bool	 m_bFirstTry;
    bool	 m_bHTTP;
    bool	 m_bReady;
    std::vector<RateInfo> m_rates;
    RATE_MAP			m_rate_grp;
    void                        setNewLevel(RateInfo &r);
    unsigned			delayTime(unsigned snac);
    unsigned			delayTime(RateInfo &r);
    RateInfo			*rateInfo(unsigned snac);
    std::list<SIM::Message*>	m_processMsg;
    std::list<DirectSocket*>	m_sockets;
    std::list<SIM::Message*>	m_acceptMsg;
    std::list<AIMFileTransfer*> m_filetransfers;
    SnacIcqBuddy* m_snacBuddy;
    SnacIcqService* m_snacService;
    SnacIcqICBM* m_snacICBM;
    mapSnacHandlers m_snacHandlers;
    bool m_bconnectionLost;

    friend class ListServerRequest;
    friend class FullInfoRequest;
    friend class SMSRequest;
    friend class ServiceSocket;
    friend class DirectSocket;
    friend class DirectClient;
    friend class ICQListener;
    friend class AIMFileTransfer;
    friend class ICQFileTransfer;
    friend class SetBuddyRequest;
    friend class SSBISocket;

    // This should be removed when refactoring is over
    friend class SnacIcqBuddy;
    friend class SnacIcqService;
    friend class SnacIcqICBM;

private:
	SIM::PropertyHubPtr m_propertyHub;
    SIM::InterfaceChecker* m_ifChecker;
    bool m_bBirthdayInfoDisplayed;
};

class ServiceSocket : public SIM::ClientSocketNotify, public OscarSocket
{
public:
    ServiceSocket(ICQClient*, unsigned short id);
    ~ServiceSocket();
    unsigned short id() const { return m_id; }
    void connect(const char *addr, unsigned short port, const QByteArray &cookie);
    virtual bool error_state(const QString &err, unsigned code = 0);
    bool connected() const { return m_bConnected; }
    void close();
protected:
    virtual const char *serviceSocketName() = 0;
    virtual void connect_ready();
    virtual void packet_ready();
    virtual ICQClientSocket *socket() { return m_socket; }
    virtual void packet(unsigned long size);
    virtual void data(unsigned short food, unsigned short type, unsigned short seq) = 0;
    unsigned short m_id;
    QByteArray  m_cookie;
    bool    m_bConnected;
    ICQClientSocket *m_socket;
    ICQClient *m_client;
};

class DirectSocket : public QObject, public SIM::ClientSocketNotify
{
    Q_OBJECT
public:
    enum SocketState{
        NotConnected,
        ConnectIP1,
        ConnectIP2,
        ConnectFail,
        WaitInit,
        WaitAck,
        WaitReverse,
        ReverseConnect,
        Logged
    };
    DirectSocket(SIM::Socket *s, ICQClient*, unsigned long ip);
    DirectSocket(ICQUserData *data, ICQClient *client);
    ~DirectSocket();
    virtual void packet_ready();
    SocketState m_state;
    void connect();
    void reverseConnect(unsigned long ip, unsigned short port);
    void acceptReverse(SIM::Socket *s);
    virtual bool   error_state(const QString &err, unsigned code = 0);
    virtual void   connect_ready();
    unsigned short localPort();
    unsigned short remotePort();
    unsigned long  Uin();
    ICQUserData    *m_data;
    void setPort(unsigned short port) {m_port = port;}
protected slots:
    void timeout();
protected:
    virtual void login_timeout();
    virtual void processPacket() = 0;
    void init();
    void sendInit();
    void sendInitAck();
    void removeFromClient();
    bool             m_bIncoming;
    unsigned short   m_nSequence;
    unsigned short   m_port;
    unsigned short	 m_localPort;
    char             m_version;
    bool			 m_bHeader;
    unsigned long    m_nSessionId;
    ICQClientSocket  *m_socket;
    ICQClient        *m_client;
    unsigned long m_ip;
    friend class AIMFileTransfer;
};

struct SendDirectMsg
{
    SIM::Message        *msg;
    unsigned            type;
    unsigned short      seq;
    unsigned short      icq_type;
};

class DirectClient : public DirectSocket
{
public:
    DirectClient(SIM::Socket *s, ICQClient *client, unsigned long ip);
    DirectClient(ICQUserData *data, ICQClient *client, unsigned channel = PLUGIN_NULL);
    ~DirectClient();
    bool sendMessage(SIM::Message*);
    void acceptMessage(SIM::Message*);
    void declineMessage(SIM::Message*, const QString &reason);
    bool cancelMessage(SIM::Message*);
    void sendAck(unsigned short, unsigned short msgType, unsigned short msgFlags,
                 const char *message=NULL, unsigned short status=ICQ_TCPxACK_ACCEPT, SIM::Message *m=NULL);
    bool isLogged() { return (m_state != None) && (m_state != WaitInit2); }
    bool isSecure();
    void addPluginInfoRequest(unsigned plugin_index);
protected:
    enum State{
        None,
        WaitLogin,
        WaitInit2,
        Logged,
        SSLconnect
    };
    State       m_state;
    unsigned    m_channel;
    void processPacket();
    void connect_ready();
    virtual bool error_state(const QString &err, unsigned code);
    void sendInit2();
    void startPacket(unsigned short cms, unsigned short seq);
    void sendPacket();
    void processMsgQueue();
    bool copyQueue(DirectClient *to);
    QList<SendDirectMsg> m_queue;
    QString name();
    QString m_name;
    void secureConnect();
    void secureListen();
    void secureStop(bool bShutdown);
    SIM::SSLClient *m_ssl;
};

class ICQFileTransfer : public SIM::FileTransfer, public DirectSocket, public SIM::ServerSocketNotify
{
public:
    ICQFileTransfer(SIM::FileMessage *msg, ICQUserData *data, ICQClient *client);
    ~ICQFileTransfer();
    void connect(unsigned short port);
    void listen();
    void setSocket(ICQClientSocket *socket);
    virtual void processPacket();
protected:
    enum State
    {
        None,
        WaitLogin,
        WaitInit,
        InitSend,
        InitReceive,
        Send,
        Receive,
        Wait,
        WaitReverse,
        WaitReverseLogin,
        Listen
    };
    State m_state;

    virtual void connect_ready();
    virtual bool error_state(const QString &err, unsigned code);
    virtual void write_ready();
    virtual void setSpeed(unsigned speed);
    virtual void startReceive(unsigned pos);
    virtual void bind_ready(unsigned short port);
    virtual bool accept(SIM::Socket *s, unsigned long ip);
    virtual bool error(const QString &err);
    virtual void login_timeout();

    void sendInit();
    void startPacket(char cmd);
    void sendPacket(bool dump=true);
    void sendFileInfo();
    void initReceive(char cmd);

    friend class ICQClient;
};

#endif


