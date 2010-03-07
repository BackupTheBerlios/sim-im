/***************************************************************************
                          jabberclient.h  -  description
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

#ifndef _JABBERCLIENT_H
#define _JABBERCLIENT_H

#include <vector>
#include <list>
#include <QStack>
#include <QByteArray>

#include "simapi.h"
#include "sax.h"
#include "socket/socket.h"
#include "socket/serversocketnotify.h"
#include "socket/tcpclient.h"
#include "jabberstatus.h"

#include "jabberbuffer.h"

using namespace std;

class JabberProtocol;
class JabberClient;

const unsigned JABBER_SIGN		= 0x0002;

const unsigned SUBSCRIBE_NONE	= 0;
const unsigned SUBSCRIBE_FROM	= 1;
const unsigned SUBSCRIBE_TO		= 2;
const unsigned SUBSCRIBE_BOTH	= (SUBSCRIBE_FROM | SUBSCRIBE_TO);

struct JabberUserData : public SIM::clientData
{
    SIM::Data		ID;
    SIM::Data		Node;
    SIM::Data		Resource;
    SIM::Data		Name;
    SIM::Data		Status;
    SIM::Data		FirstName;
    SIM::Data		Nick;
    SIM::Data		Desc;
    SIM::Data		Bday;
    SIM::Data		Url;
    SIM::Data		OrgName;
    SIM::Data		OrgUnit;
    SIM::Data		Title;
    SIM::Data		Role;
    SIM::Data		Street;
    SIM::Data		ExtAddr;
    SIM::Data		City;
    SIM::Data		Region;
    SIM::Data		PCode;
    SIM::Data		Country;
    SIM::Data		EMail;
    SIM::Data		Phone;
    SIM::Data		StatusTime;
    SIM::Data		OnlineTime;
    SIM::Data		Subscribe;
    SIM::Data		Group;
    SIM::Data		bChecked;
    SIM::Data		TypingId;
    SIM::Data		SendTypingEvents;
    SIM::Data		IsTyping;
    SIM::Data		composeId;
    SIM::Data		richText;
    SIM::Data		invisible;
    SIM::Data		PhotoWidth;
    SIM::Data		PhotoHeight;
    SIM::Data		LogoWidth;
    SIM::Data		LogoHeight;
    SIM::Data		nResources;
    SIM::Data		Resources;
    SIM::Data		ResourceStatus;
    SIM::Data		ResourceReply;
    SIM::Data		ResourceStatusTime;
    SIM::Data		ResourceOnlineTime;
    SIM::Data		AutoReply;
    SIM::Data		ResourceClientName;
    SIM::Data		ResourceClientVersion;
    SIM::Data		ResourceClientOS;
};

struct JabberClientData
{
    SIM::Data		Server;
    SIM::Data		Port;
    SIM::Data		UseSSL;
    SIM::Data		UsePlain;
    SIM::Data		UseVHost;
    SIM::Data		Register;
    SIM::Data		Priority;
    SIM::Data		ListRequest;
    SIM::Data		VHost;
    SIM::Data		Typing;
    SIM::Data		RichText;
    SIM::Data		UseVersion;
    SIM::Data		ProtocolIcons;
    SIM::Data		MinPort;
    SIM::Data		MaxPort;
    SIM::Data		Photo;
    SIM::Data		Logo;
    SIM::Data		AutoSubscribe;
    SIM::Data		AutoAccept;
    SIM::Data		UseHTTP;
    SIM::Data		URL;
    SIM::Data		InfoUpdated;
    JabberUserData	owner;
};

struct JabberAgentsInfo
{
    SIM::Data		VHost;
    SIM::Data		ID;
    SIM::Data		Name;
    SIM::Data		Search;
    SIM::Data		Register;
    JabberClient	*Client;
};

struct JabberAgentInfo
{
    SIM::Data		ReqID;
    SIM::Data		VHost;
    SIM::Data		ID;
    SIM::Data		Field;
    SIM::Data		Type;
    SIM::Data		Label;
    SIM::Data		Value;
    SIM::Data		Desc;
    SIM::Data		Options;
    SIM::Data		OptionLabels;
    SIM::Data		nOptions;
    SIM::Data		bRequired;
};

struct JabberSearchData
{
    SIM::Data		ID;
    SIM::Data		JID;
    SIM::Data		First;
    SIM::Data		Last;
    SIM::Data		Nick;
    SIM::Data		EMail;
    SIM::Data		Status;
    SIM::Data		Fields;
    SIM::Data		nFields;
};

struct JabberListRequest
{
    QString             jid;
    QString             grp;
    QString             name;
    bool                bDelete;
};

struct DiscoItem
{
    QString             id;
    QString             jid;
    QString             node;
    QString             name;
    QString             type;
    QString             category;
    QString             features;
};

// XEP-0092 Software Version
struct ClientVersionInfo
{
    QString             jid;
    QString             node;
    QString             name;
    QString             version;
    QString             os;
};

// XEP-0012 Last Activity
struct ClientLastInfo
{
    QString             jid;
    unsigned int        seconds;
};

// XEP-0090 Entity Time
struct ClientTimeInfo
{
    QString             jid;
    QString             utc;
    QString             tz;
    QString             display;
};

class JabberClient : public SIM::TCPClient, public SAXParser
{
    Q_OBJECT
public:
    class ServerRequest
    {
    public:
        ServerRequest(JabberClient *client, const char *type, const QString &from, const QString &to, const QString &id=QString());
        virtual ~ServerRequest();
        void	send();
        void	start_element(const QString &name);
        void	end_element(bool bNewLevel = false);
        void	add_attribute(const QString &name, const char *value);
        void	add_attribute(const QString &name, const QString &value);
        void	add_condition(const QString &cond, bool bXData);
        void	add_text(const QString &text);
        void	text_tag(const QString &name, const QString &value);
        static const char *_GET;
        static const char *_SET;
        static const char *_RESULT;
    protected:
        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
        virtual void element_end(const QString& el);
        virtual void char_data(const QString& str);
        QString  		m_element;
        QStack<QString>	m_els;
        QString		    m_id;
        JabberClient	*m_client;
        friend class JabberClient;
    };

    class IqRequest : public ServerRequest
    {
    public:
        IqRequest(JabberClient *client);
        ~IqRequest();
    public:
        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
        virtual void element_end(const QString& el);
        virtual void char_data(const QString& str);
        QString		*m_data;
        QString		m_url;
        QString		m_descr;
        QString		m_query;
        QString		m_from;
        QString		m_id;
        QString		m_type;
        QString		m_file_name;
        unsigned	m_file_size;
    };

    class PresenceRequest : public ServerRequest
    {
    public:
        PresenceRequest(JabberClient *client);
        ~PresenceRequest();
    protected:
        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
        virtual void element_end(const QString& el);
        virtual void char_data(const QString& str);
        QString m_from;
        QString m_data;
        QString m_type;
        QString m_status;
        QString m_show;
        QString m_stamp1;
        QString m_stamp2;
    };

    class MessageRequest : public ServerRequest
    {
    public:
        MessageRequest(JabberClient *client);
        ~MessageRequest();
    protected:
        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
        virtual void element_end(const QString& el);
        virtual void char_data(const QString& str);
        QString m_from;
        QString *m_data;
        QString m_body;
        QString m_richText;
        QString m_subj;
        QString m_error;
        QString m_contacts;
        QString m_target;
        QString m_desc;
		QString m_enc;
        vector<QString> m_targets;
        vector<QString> m_descs;

        bool   m_bBody;
        bool   m_bRosters;
        bool   m_bError;
        QString m_id;
        bool   m_bCompose;
        bool   m_bEvent;
        bool   m_bRichText;
		bool   m_bEnc;
        unsigned m_errorCode;
    };

    class StreamErrorRequest : public ServerRequest
    {
    public:
        StreamErrorRequest(JabberClient *client);
        ~StreamErrorRequest();
    protected:
        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
        virtual void element_end(const QString& el);
        virtual void char_data(const QString& str);
        QString *m_data;
        QString m_descr;
    };

    JabberClient(JabberProtocol*, Buffer *cfg);
    ~JabberClient();
    virtual QString name();
    virtual QString dataName(void*);
    virtual QWidget	*setupWnd();
    virtual QByteArray getConfig();
    virtual QImage userPicture(unsigned id);
    QImage userPicture(JabberUserData *d);

    void setID(const QString &id);
    QString getID()
    {
        return data.owner.ID.str();
    }
    PROP_STR(Server);
    PROP_STR(VHost);
    PROP_USHORT(Port);
    PROP_BOOL(UseSSL);
    PROP_BOOL(UsePlain);
    PROP_BOOL(UseVHost);
    PROP_BOOL(Register);
    PROP_ULONG(Priority);
    PROP_UTF8(ListRequest);
    PROP_BOOL(Typing);
    PROP_BOOL(RichText);
    PROP_BOOL(UseVersion);
    PROP_BOOL(ProtocolIcons);
    PROP_USHORT(MinPort);
    PROP_USHORT(MaxPort);
    PROP_UTF8(Photo);
    PROP_UTF8(Logo);
    PROP_BOOL(AutoSubscribe);
    PROP_BOOL(AutoAccept);
    PROP_BOOL(UseHTTP);
    PROP_STR(URL);
    PROP_BOOL(InfoUpdated);

    QString         buildId(JabberUserData *data);
    JabberUserData *findContact(const QString &jid, const QString &name, bool bCreate, SIM::Contact *&contact, QString &resource, bool bJoin=true);
    bool            add_contact(const char *id, unsigned grp);
    QString         get_agents(const QString &jid);
    QString         get_agent_info(const QString &jid, const QString &node, const QString &type);
    void            auth_request(const QString &jid, unsigned type, const QString &text, bool bCreate);
    QString         search(const QString &jid, const QString &node, const QString &condition);
    QString         process(const QString &jid, const QString &node, const QString &condition, const QString &type);

    static QString	get_attr(const char *name, const char **attrs);
    virtual void setupContact(SIM::Contact*, void *data);
    virtual void updateInfo(SIM::Contact *contact, void *data);

    JabberClientData	data;

    JabberListRequest *findRequest(const QString &jid, bool bRemove);

    QString VHost();
    bool isAgent(const QString &jid);
    virtual bool send(SIM::Message*, void*);
    void    listRequest(JabberUserData *data, const QString &name, const QString &grp, bool bDelete);
    void	sendFileRequest(SIM::FileMessage *msg, unsigned short port, JabberUserData *data, const QString &url, unsigned size);
    void	sendFileAccept(SIM::FileMessage *msg, JabberUserData *data);

    list<SIM::Message*> m_ackMsg;
    list<SIM::Message*> m_waitMsg;

    QString photoFile(JabberUserData*);
    QString logoFile(JabberUserData*);
    list<ServerRequest*>	m_requests;

    QString discoItems(const QString &jid, const QString &node);
    QString discoInfo(const QString &jid, const QString &node);
    QString browse(const QString &jid);
    QString versionInfo(const QString &jid, const QString &node = QString::null);
    QString timeInfo(const QString &jid, const QString &node);
    QString lastInfo(const QString &jid, const QString &node);
    QString statInfo(const QString &jid, const QString &node);
    void addLang(ServerRequest *req);
    void info_request(JabberUserData *user_data, bool bVCard);
    virtual void setClientInfo(void *data);
    void changePassword(const QString &pass);

    // reimplement socket() to get correct Buffer
    virtual JabberClientSocket *socket() { return static_cast<JabberClientSocket*>(TCPClient::socket()); }
    virtual JabberClientSocket *createClientSocket() { return new JabberClientSocket(this, createSocket()); }

    virtual void changeStatus(const SIM::IMStatusPtr& status);
    
    JabberUserData* toJabberUserData(SIM::clientData *); // More safely type conversion from generic SIM::clientData into JabberUserData
protected slots:
    void	ping();
    void	auth_failed();
    void	auth_ok();
protected:
    virtual bool processEvent(SIM::Event *e);
    SIM::Socket *createSocket();

    virtual QString contactName(void *clientData);
    virtual void setStatus(unsigned status);
    void setStatus(unsigned status, const QString &ar);
    virtual void disconnected();
    virtual void connect_ready();
    virtual void packet_ready();
    virtual void setInvisible(bool bState);
    virtual bool isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool createData(SIM::clientData*&, SIM::Contact*);
    virtual bool compareData(void*, void*);
    virtual bool canSend(unsigned, void*);
    virtual void contactInfo(void *data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
    virtual QString resources(void *data);
    virtual QString contactTip(void *data);
    virtual QWidget *searchWindow(QWidget *parent);
    virtual SIM::CommandDef *infoWindows(SIM::Contact *contact, void *data);
    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact *contact, void *data, unsigned id);
    virtual SIM::CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);

    void init();
    void sendPacket();
    void startHandshake();
    void connected();
    void handshake(const QString &id);
    void rosters_request();
    void setOffline(JabberUserData *data);

    static	QString encodeXML(const QString &str);
    static	QString encodeXMLattr(const QString &str);
    QString		m_id;
    unsigned	m_depth;

    QString		get_unique_id();
    unsigned	m_id_seed;
    unsigned	m_msg_id;

    bool		m_bHTTP;

    void element_start(const QString& el, const QXmlAttributes& attrs);
    void element_end(const QString& el);
    void char_data(const QString& str);

    list<JabberListRequest>	m_listRequests;
    ServerRequest			*m_curRequest;

    class JabberAuthMessage;
    vector<JabberAuthMessage*>	tempAuthMessages;

    QString get_icon(JabberUserData *data, unsigned status, bool invisible);

    void		processList();

    void		auth_plain();
    void		auth_digest();
    void		auth_register();
    bool		m_bSSL;
    bool		m_bJoin;

    friend class ServerRequest;
    friend class RostersRequest;
    friend class PresenceRequest;
    friend class JabberBrowser;

private:
};

class JabberFileTransfer : public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
{
public:
    JabberFileTransfer(SIM::FileMessage *msg, JabberUserData *data, JabberClient *client);
    ~JabberFileTransfer();
    void listen();
    void connect();
protected:
    JabberClient	*m_client;
    JabberUserData	*m_data;
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
        Receive
    };
    State m_state;
    virtual bool    error_state(const QString &err, unsigned code = 0);
    virtual void    packet_ready();
    virtual void    connect_ready();
    virtual void    write_ready();
    virtual void    startReceive(unsigned pos);
    virtual void    bind_ready(unsigned short port);
    virtual bool    error(const QString &err);
    virtual bool    accept(SIM::Socket *s, unsigned long ip);
    bool get_line(const QByteArray &str);
    void send_line(const QString &str);
    void send_line(const QByteArray &str);
    void send_line(const char *str);
    unsigned m_startPos;
    unsigned m_endPos;
    unsigned m_answer;
    QString             m_url;
    JabberClientSocket *m_socket;
};

class JabberSearch;

struct agentRegisterInfo
{
    QString		id;
    unsigned		err_code;
    QString		error;
};

#endif

// vim: set expandtab:


