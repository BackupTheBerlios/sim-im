/***************************************************************************
                          yahooclient.h  -  description
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

#ifndef _YAHOOCLIENT_H
#define _YAHOOCLIENT_H

#include "socket/socket.h"
#include "socket/serversocketnotify.h"
#include "socket/tcpclient.h"
#include "fetch.h"
#include <QByteArray>

#ifdef __OS2__
#define PARAM YAHOOPARAM
#endif

const unsigned YAHOO_SIGN = 9;

const unsigned long YAHOO_STATUS_AVAILABLE		= 0;
const unsigned long YAHOO_STATUS_BRB			= 1;
const unsigned long YAHOO_STATUS_BUSY			= 2;
const unsigned long YAHOO_STATUS_NOTATHOME		= 3;
const unsigned long YAHOO_STATUS_NOTATDESK		= 4;
const unsigned long YAHOO_STATUS_NOTINOFFICE	= 5;
const unsigned long YAHOO_STATUS_ONPHONE		= 6;
const unsigned long YAHOO_STATUS_ONVACATION		= 7;
const unsigned long YAHOO_STATUS_OUTTOLUNCH		= 8;
const unsigned long YAHOO_STATUS_STEPPEDOUT		= 9;
const unsigned long YAHOO_STATUS_INVISIBLE		= 12;
const unsigned long YAHOO_STATUS_CUSTOM			= 99;
const unsigned long YAHOO_STATUS_OFFLINE		= (unsigned long)(-1);

const unsigned short YAHOO_SERVICE_LOGON			= 0x01;
const unsigned short YAHOO_SERVICE_LOGOFF			= 0x02;
const unsigned short YAHOO_SERVICE_ISAWAY			= 0x03;
const unsigned short YAHOO_SERVICE_ISBACK			= 0x04;
const unsigned short YAHOO_SERVICE_IDLE				= 0x05;
const unsigned short YAHOO_SERVICE_MESSAGE			= 0x06;
const unsigned short YAHOO_SERVICE_IDACT			= 0x07;
const unsigned short YAHOO_SERVICE_IDDEACT			= 0x08;
const unsigned short YAHOO_SERVICE_MAILSTAT			= 0x09;
const unsigned short YAHOO_SERVICE_USERSTAT			= 0x0A;
const unsigned short YAHOO_SERVICE_NEWMAIL			= 0x0B;
const unsigned short YAHOO_SERVICE_CHATINVITE		= 0x0C;
const unsigned short YAHOO_SERVICE_CALENDAR			= 0x0D;
const unsigned short YAHOO_SERVICE_NEWPERSONALMAIL	= 0x0E;
const unsigned short YAHOO_SERVICE_NEWCONTACT		= 0x0F;
const unsigned short YAHOO_SERVICE_ADDIDENT			= 0x10;
const unsigned short YAHOO_SERVICE_ADDIGNORE		= 0x11;
const unsigned short YAHOO_SERVICE_PING				= 0x12;
const unsigned short YAHOO_SERVICE_GOTGROUPRENAME	= 0x13;
const unsigned short YAHOO_SERVICE_SYSMESSAGE		= 0x14;
const unsigned short YAHOO_SERVICE_PASSTHROUGH2		= 0x16;
const unsigned short YAHOO_SERVICE_CONFINVITE		= 0x18;
const unsigned short YAHOO_SERVICE_CONFLOGON		= 0x19;
const unsigned short YAHOO_SERVICE_CONFDECLINE		= 0x1A;
const unsigned short YAHOO_SERVICE_CONFLOGOFF		= 0x1B;
const unsigned short YAHOO_SERVICE_CONFADDINVITE	= 0x1C;
const unsigned short YAHOO_SERVICE_CONFMSG			= 0x1D;
const unsigned short YAHOO_SERVICE_CHATLOGON		= 0x1E;
const unsigned short YAHOO_SERVICE_CHATLOGOFF		= 0x1F;
const unsigned short YAHOO_SERVICE_CHATMSG			= 0x20;
const unsigned short YAHOO_SERVICE_GAMELOGON		= 0x28;
const unsigned short YAHOO_SERVICE_GAMELOGOFF		= 0x29;
const unsigned short YAHOO_SERVICE_GAMEMSG			= 0x2A;
const unsigned short YAHOO_SERVICE_FILETRANSFER		= 0x46;
const unsigned short YAHOO_SERVICE_VOICECHAT		= 0x4A;
const unsigned short YAHOO_SERVICE_NOTIFY			= 0x4B;
const unsigned short YAHOO_SERVICE_VERIFY			= 0x4C;
const unsigned short YAHOO_SERVICE_P2PFILEXFER		= 0x4D;
const unsigned short YAHOO_SERVICE_PEERTOPEER		= 0x4F;
const unsigned short YAHOO_SERVICE_WEBCAM			= 0x4E;
const unsigned short YAHOO_SERVICE_AUTHRESP			= 0x54;
const unsigned short YAHOO_SERVICE_LIST				= 0x55;
const unsigned short YAHOO_SERVICE_AUTH				= 0x57;
const unsigned short YAHOO_SERVICE_ADDBUDDY			= 0x83;
const unsigned short YAHOO_SERVICE_REMBUDDY			= 0x84;
const unsigned short YAHOO_SERVICE_IGNORECONTACT	= 0x85;
const unsigned short YAHOO_SERVICE_REJECTCONTACT	= 0x86;
const unsigned short YAHOO_SERVICE_GROUPRENAME		= 0x89;
const unsigned short YAHOO_SERVICE_CHATONLINE		= 0x96;
const unsigned short YAHOO_SERVICE_CHATGOTO			= 0x97;
const unsigned short YAHOO_SERVICE_CHATJOIN			= 0x98;
const unsigned short YAHOO_SERVICE_CHATLEAVE		= 0x99;
const unsigned short YAHOO_SERVICE_CHATEXIT			= 0x9B;
const unsigned short YAHOO_SERVICE_CHATLOGOUT		= 0xA0;
const unsigned short YAHOO_SERVICE_CHATPING			= 0xA1;
const unsigned short YAHOO_SERVICE_COMMENT			= 0xA8;

const unsigned LR_CHANGE		= 0;
const unsigned LR_DELETE		= 1;
const unsigned LR_CHANGE_GROUP	= 2;
const unsigned LR_DELETE_GROUP	= 3;
/*
struct YahooUserData : public SIM::clientData //Fixme: clientData not a member of SIM
{
    SIM::Data		Login;
    SIM::Data		Nick;
    SIM::Data		First;
    SIM::Data		Last;
    SIM::Data		EMail;
    SIM::Data		Status;
    SIM::Data		bAway;
    SIM::Data		AwayMessage;
    SIM::Data		OnlineTime;
    SIM::Data		StatusTime;
    SIM::Data		Group;
    SIM::Data		bChecked;
    SIM::Data		bTyping;
};
*/
struct YahooClientData
{
    SIM::Data	Server;
    SIM::Data	Port;
    SIM::Data	MinPort;
    SIM::Data	MaxPort;
    SIM::Data	UseHTTP;
    SIM::Data	AutoHTTP;
    SIM::Data	ListRequests;
    YahooUserData	owner;
};

typedef std::pair<unsigned, QByteArray> PARAM;

class Params : public std::list<PARAM>
{
public:
    Params() {}
    const char *operator[](unsigned id);
};

class QTextCodec;

struct Message_ID
{
    SIM::Message	*msg;
    unsigned		id;
};

struct ListRequest
{
    unsigned	type;
    QString     name;
};

class YahooClient : public SIM::TCPClient
{
    Q_OBJECT
public:
    YahooClient(SIM::Protocol*, Buffer *cfg);
    ~YahooClient();
    PROP_STR(Server);
    PROP_USHORT(Port);
    PROP_USHORT(MinPort);
    PROP_USHORT(MaxPort);
    PROP_BOOL(UseHTTP);
    PROP_BOOL(AutoHTTP);
    PROP_STR(ListRequests);
    virtual QByteArray getConfig();
    QString getLogin();
    void setLogin(const QString&);
    QString name();
    QString dataName(void*);
    YahooClientData	data;
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
    YahooUserData *findContact(const char *id, const char *grp, SIM::Contact *&contact, bool bSend=true, bool bJoin=true);
    void sendFile(SIM::FileMessage *msg, QFile *file, YahooUserData *data, unsigned short port);
    std::list<Message_ID>		m_waitMsg;
    std::list<SIM::Message*>	m_ackMsg;
    YahooUserData* toYahooUserData(SIM::clientData * data);
protected slots:
    void ping();
protected:
    virtual bool processEvent(SIM::Event *e);
    void	setStatus(unsigned status);
    virtual void setInvisible(bool bState);
    void	disconnected();
    SIM::Socket  *createSocket();
    QWidget	*setupWnd();
    bool isMyData(SIM::clientData*&, SIM::Contact*&);
    bool createData(SIM::clientData*&, SIM::Contact*);
    void setupContact(SIM::Contact*, void *data);
    bool send(SIM::Message*, void *data);
    bool canSend(unsigned type, void *data);
    SIM::CommandDef *infoWindows(SIM::Contact*, void *_data);
    SIM::CommandDef *configWindows();
    QWidget *infoWindow(QWidget *parent, SIM::Contact*, void *_data, unsigned id);
    QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget *searchWindow(QWidget *parent);
    virtual QString contactTip(void *_data);
    virtual void connect_ready();
    virtual void packet_ready();
    void scan_packet();
    void process_packet(Params &params);
    void process_auth(const char *method, const char *seed, const char *sn);
    void process_auth_0x0b(const char *seed, const char *sn);
    void sendPacket(unsigned short service, unsigned long status=YAHOO_STATUS_AVAILABLE);
    void addParam(unsigned key, const char *value);
    void addParam(unsigned key, const QByteArray &value);
    void addParam(unsigned key, const QString &value);
    void loadList(const char *data);
    void contact_added(const char *id, const char *message);
    void contact_rejected(const char *id, const char *message);
    void processStatus(unsigned short service, const char *id,
                       const char *_state, const char *_msg,
                       const char *_away, const char *_idle);
    void messageReceived(SIM::Message *msg, const char *id);
    void process_message(const char *id, const char *msg, const char *utf);
    void process_file(const char *id, const char *fileName, const char *fileSize, const char *msg, const char *url, const char *msgid);
    void process_fileurl(const char *id, const char *msg, const char *url);
    void notify(const char *id, const char *msg, const char *state);
    void sendMessage(const QString &msgText, SIM::Message *msg, YahooUserData*);
    void sendTyping(YahooUserData*, bool);
    void addBuddy(YahooUserData*);
    void removeBuddy(YahooUserData*);
    void moveBuddy(YahooUserData *data, const QString &grp);
    void sendStatus(unsigned long status, const QString &msg = QString::null);
    ListRequest *findRequest(const QString &login);
    std::list<PARAM>	   m_values;
    std::list<ListRequest> m_requests;
    unsigned long  m_session;
    unsigned long  m_pkt_status;
    unsigned short m_data_size;
    unsigned short m_service;
    unsigned	   m_ft_id;
    QString        m_session_id;
    bool m_bHeader;
    bool m_bHTTP;
    bool m_bFirstTry;
    void authOk();
};

struct YahooFileData
{
    SIM::Data	Url;
    SIM::Data	MsgID;
};

class YahooFileMessage : public SIM::FileMessage
{
public:
    YahooFileMessage(Buffer *cfg=NULL);
    ~YahooFileMessage();
    PROP_STR(Url);
    PROP_ULONG(MsgID);
    virtual	QByteArray save();
    virtual unsigned baseType() { return SIM::MessageFile; }
protected:
    YahooFileData data;
};

class YahooFileTransfer : public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
{
public:
    YahooFileTransfer(SIM::FileMessage *msg, YahooUserData *data, YahooClient *client);
    ~YahooFileTransfer();
    void listen();
    void connect();
    virtual bool    error_state(const QString &err, unsigned code = 0);
protected:
    YahooClient	*m_client;
    YahooUserData	*m_data;
    enum State
    {
        None,
        Listen,
        ListenWait,
        Header,
        Send,
        Wait,
        Connect,
        ReadHeader,
        Receive,
        Skip
    };
    State m_state;
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	write_ready();
    virtual void	startReceive(unsigned pos);
    virtual void	bind_ready(unsigned short port);
    virtual bool	error(const QString &err);
    virtual bool	accept(SIM::Socket *s, unsigned long ip);
    bool get_line(const QByteArray &str);
    void send_line(const QString &str);
    unsigned m_startPos;
    unsigned m_endPos;
    unsigned m_answer;
    QString	m_url;
    QString	m_host;
    QString	m_method;
    SIM::ClientSocket	*m_socket;
};

#endif

