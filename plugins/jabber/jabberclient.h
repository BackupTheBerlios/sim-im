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
#include "contacts/imcontact.h"
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

class JabberUserData : public SIM::IMContact
{
public:
    JabberUserData(const SIM::ClientPtr& cl);

    virtual QByteArray serialize();
    virtual void deserialize(Buffer* cfg);

    virtual void serialize(QDomElement& element);
    virtual void deserialize(QDomElement& element);

    virtual SIM::ClientPtr client() { return m_client.toStrongRef(); }

    virtual unsigned long getSign();

    QString getId() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString getNode() const { return m_node; }
    void setNode(const QString& node) { m_node = node; }

    QString getResource() const { return m_resource; }
    void setResource(const QString& resource) { m_resource = resource; }

    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    unsigned long getStatus() const { return m_status; }
    void setStatus(unsigned long status) { m_status = status; }

    QString getFirstName() const { return m_firstName; }
    void setFirstName(const QString& name) { m_firstName; }

    QString getNick() const { return m_nick; }
    void setNick(const QString& nick) { m_nick = nick; }

    QString getDesc() const { return m_desc; }
    void setDesc(const QString& desc) { m_desc = desc; }

    QString getBirthday() const { return m_birthday; }
    void setBirthday(const QString& birthday) { m_birthday = birthday; }

    QString getUrl() const { return m_url; }
    void setUrl(const QString& url) { m_url = url; }

    QString getOrgName() const { return m_orgName; }
    void setOrgName(const QString& orgname) { m_orgName = orgname; }

    QString getOrgUnit() const { return m_orgUnit; }
    void setOrgUnit(const QString& orgunit) { m_orgUnit = orgunit; }

    QString getTitle() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString getRole() const { return m_role; }
    void setRole(const QString& role) { m_role; }

    QString getStreet() const { return m_street; }
    void setStreet(const QString& street) { m_street = street; }

    QString getExtAddr() const { return m_extAddr; }
    void setExtAddr(const QString& addr) { m_extAddr = addr; }

    QString getCity() const { return m_city; }
    void setCity(const QString& city) { m_city = city; }

    QString getRegion() const { return m_region; }
    void setRegion(const QString& region) { m_region = region; }

    QString getPCode() const { return m_pcode; }
    void setPCode(const QString& pcode) { m_pcode = pcode; }

    QString getCountry() const { return m_country; }
    void setCountry(const QString& country) { m_country = country; }

    QString getEmail() const { return m_email; }
    void setEmail(const QString& email) { m_email = email; }

    QString getPhone() const { return m_phone; }
    void setPhone(const QString& phone) { m_phone = phone; }

    unsigned long getStatusTime() const { return m_statusTime; }
    void setStatusTime(unsigned long statustime) { m_statusTime = statustime; }

    unsigned long getOnlineTime() const { return m_onlineTime; }
    void setOnlineTime(unsigned long onlinetime) { m_onlineTime = onlinetime; }

    unsigned long getSubscribe() const { return m_subscribe; }
    void setSubscribe(unsigned long subscribe) { m_subscribe = subscribe; }

    QString getGroup() const { return m_group; }
    void setGroup(const QString& group) { m_group = group; }

    bool isChecked() const { return m_checked; }
    void setChecked(bool c) { m_checked = c; }

    QString getTypingId() const { return m_typingId; }
    void setTypingId(const QString& id) { m_typingId = id; }

    bool getSendTypingEvents() const { return m_sendTypingEvents; }
    void setSendTypingEvents(bool b) { m_sendTypingEvents = b; }

    bool isTyping() const { return m_typing; }
    void setTyping(bool t) { m_typing = t; }

    unsigned long getComposeId() const { return m_composeId; }
    void setComposeId(unsigned long id) { m_composeId = id; }

    bool isRichText() const { return m_richText; }
    void setRichText(bool rt) { m_richText = rt; }

    bool isInvisible() const { return m_invisible; }
    void setInvisible(bool i) { m_invisible = i; }

    unsigned long getPhotoWidth() const { return m_photoSize.width(); }
    void setPhotoWidth(unsigned long width) { m_photoSize.setWidth(width); }

    unsigned long getPhotoHeight() const { return m_photoSize.height(); }
    void setPhotoHeight(unsigned long height) { m_photoSize.setHeight(height); }

    unsigned long getLogoWidth() const { return m_logoSize.width(); }
    void setLogoWidth(unsigned long width) { m_logoSize.setWidth(width); }

    unsigned long getLogoHeight() const { return m_logoSize.height(); }
    void setLogoHeight(unsigned long height) { m_logoSize.setHeight(height); }

    unsigned long getNResources() const { return m_nResources; }
    void setNResources(unsigned long nResources) { m_nResources = nResources; }

    QString getResource(int n) const { return m_resources.at(n); }
    void setResource(int n, const QString& resource) { m_resources.replace(n, resource); }
    void appendResource(const QString& resource) { m_resources.append(resource); }
    void clearResources() { m_resources.clear(); }

    QString getResourceStatus(int n) const { return m_resourceStatuses.at(n); }
    void setResourceStatus(int n, const QString& resourcestatus) { m_resourceStatuses.replace(n, resourcestatus); }
    void appendResourceStatus(const QString& resourcestatus) { m_resourceStatuses.append(resourcestatus); }
    void clearResourceStatuses() { m_resourceStatuses.clear(); }

    QString getResourceReply(int n) const { return m_resourceReplies.at(n); }
    void setResourceReply(int n, const QString& resourcereply) { m_resourceReplies.replace(n, resourcereply); }
    void appendResourceReply(const QString& resourcereply) { m_resourceReplies.append(resourcereply); }
    void clearResourceReplies() { m_resourceReplies.clear(); }

    QString getResourceStatusTime(int n) const { return m_resourceStatusTimes.at(n); }
    void setResourceStatusTime(int n, const QString& resourcestatustimes) { m_resourceStatusTimes.replace(n, resourcestatustimes); }
    void appendResourceStatusTime(const QString& resourcestatustimes) { m_resourceStatusTimes.append(resourcestatustimes); }
    void clearResourceStatusTimes() { m_resourceStatusTimes.clear(); }

    QString getResourceOnlineTime(int n) const { return m_resourceOnlineTimes.at(n); }
    void setResourceOnlineTime(int n, const QString& resourceonlinetimes) { m_resourceOnlineTimes.replace(n, resourceonlinetimes); }
    void appendResourceOnlineTime(const QString& resourceonlinetimes) { m_resourceOnlineTimes.append(resourceonlinetimes); }
    void clearResourceOnlineTimes() { m_resourceOnlineTimes.clear(); }

    QString getAutoReply() const { return m_autoReply; }
    void setAutoReply(const QString& autoreply) { m_autoReply = autoreply; }

    QString getResourceClientName(int n) const { return m_resourceClientNames.at(n); }
    void setResourceClientName(int n, const QString& resourceclientname) { m_resourceClientNames.replace(n, resourceclientname); }
    void appendResourceClientName(const QString& resourceclientname) { m_resourceClientNames.append(resourceclientname); }
    void clearResourceClientNames() { m_resourceClientNames.clear(); }
    
    QString getResourceClientVersion(int n) const { return m_resourceClientVersions.at(n); }
    void setResourceClientVersion(int n, const QString& resourceclientversion) { m_resourceClientVersions.replace(n, resourceclientversion); }
    void appendResourceClientVersion(const QString& resourceclientversion) { m_resourceClientVersions.append(resourceclientversion); }
    void clearResourceClientVersions() { m_resourceClientVersions.clear(); }

    QString getResourceClientOS(int n) const { return m_resourceClientOSes.at(n); }
    void setResourceClientOS(int n, const QString& resourceclientos) { m_resourceClientOSes.replace(n, resourceclientos); }
    void appendResourceClientOS(const QString& resourceclientos) { m_resourceClientOSes.append(resourceclientos); }
    void clearResourceClientOSes() { m_resourceClientOSes.clear(); }

    virtual void deserializeLine(const QString& key, const QString& value);
public:

    QString m_id;
    QString m_node;
    QString m_resource;
    QString m_name;
    unsigned long m_status;
    QString m_firstName;
    QString m_nick;
    QString m_desc;
    QString m_birthday;
    QString m_url;
    QString m_orgName;
    QString m_orgUnit;
    QString m_title;
    QString m_role;
    QString m_street;
    QString m_extAddr;
    QString m_city;
    QString m_region;
    QString m_pcode;
    QString m_country;
    QString m_email;
    QString m_phone;
    unsigned long m_statusTime;
    unsigned long m_onlineTime;
    unsigned long m_subscribe;
    QString m_group;
    bool m_checked;
    QString m_typingId;
    bool m_sendTypingEvents;
    bool m_typing;
    unsigned long m_composeId;
    bool m_richText;
    bool m_invisible;
    QSize m_photoSize;
    QSize m_logoSize;
    unsigned long m_nResources;
    QStringList m_resources;
    QStringList m_resourceStatuses;
    QStringList m_resourceReplies;
    QStringList m_resourceStatusTimes;
    QStringList m_resourceOnlineTimes;
    QString m_autoReply;
    QStringList m_resourceClientNames;
    QStringList m_resourceClientVersions;
    QStringList m_resourceClientOSes;

    QWeakPointer<SIM::Client> m_client;
};

struct JabberClientData : public SIM::IMContact
{
public:
    JabberClientData(const SIM::ClientPtr& client);
    virtual QByteArray serialize();
    virtual void deserialize(Buffer* cfg);
    virtual void serialize(QDomElement& element) {};
    virtual void deserialize(QDomElement& element) {};

    virtual SIM::ClientPtr client() { Q_ASSERT_X(false, "ICQClientData::client", "Shouldn't be called"); return SIM::ClientPtr(); }

    virtual unsigned long getSign();

    QString getServer() const { return m_server;}
    void setServer(const QString& server) { m_server = server; }

    unsigned long getPort() const { return m_port; }
    void setPort(unsigned long port) { m_port = port; }

    bool getUseSSL() const { return m_useSSL; }
    void setUseSSL(bool b) { m_useSSL = b; }

    bool getUsePlain() const { return m_usePlain; }
    void setUsePlain(bool b) { m_usePlain = b; }

    bool getUseVHost() const { return m_useVHost; }
    void setUseVHost(bool b) { m_useVHost = b; }

    bool getRegister() const { return m_register; }
    void setRegister(bool b) { m_register = b; }

    unsigned long getPriority() const { return m_priority; }
    void setPriority(unsigned long priority) { m_priority = priority; }

    QString getListRequest() const { return m_listRequest; }
    void setListRequest(const QString& list) { m_listRequest = list; }

    QString getVHost() const { return m_vHost; }
    void setVHost(const QString& vhost) { m_vHost = vhost; }

    bool isTyping() const { return m_typing; }
    void setTyping(bool t) { m_typing = t; }

    bool isRichText() const { return m_richText; }
    void setRichText(bool rt) { m_richText = rt; }

    bool getUseVersion() const { return m_useVersion; }
    void setUseVersion(bool b) { m_useVersion = b; }

    bool getProtocolIcons() const { return m_protocolIcons; }
    void setProtocolIcons(bool b) { m_protocolIcons = b; } 

    unsigned long getMinPort() const { return m_minPort; }
    void setMinPort(unsigned long port) { m_minPort = port; }

    unsigned long getMaxPort() const { return m_maxPort; }
    void setMaxPort(unsigned long port) { m_maxPort = port; }

    QString getPhoto() const { return m_photo; }
    void setPhoto(const QString& photo) { m_photo = photo; }

    QString getLogo() const { return m_logo; }
    void setLogo(const QString& logo) { m_logo = logo; }

    bool getAutoSubscribe() const { return m_autoSubscribe; }
    void setAutoSubscribe(bool b) { m_autoSubscribe = b; }

    bool getAutoAccept() const { return m_autoAccept; }
    void setAutoAccept(bool b) { m_autoAccept = b; }

    bool getUseHttp() const { return m_useHttp; }
    void setUseHttp(bool b) { m_useHttp = b; }

    QString getUrl() const { return m_url; }
    void setUrl(const QString& url) { m_url = url; }

    bool getInfoUpdated() const { return m_infoUpdated; }
    void setInfoUpdated(bool b) { m_infoUpdated = b; }

    JabberUserData	owner;

    virtual void deserializeLine(const QString& key, const QString& value);
public:

    QString m_server;
    unsigned long m_port;
    bool m_useSSL;
    bool m_usePlain;
    bool m_useVHost;
    bool m_register;
    unsigned long m_priority;
    QString m_listRequest;
    QString m_vHost;
    bool m_typing;
    bool m_richText;
    bool m_useVersion;
    bool m_protocolIcons;
    unsigned long m_minPort;
    unsigned long m_maxPort;
    QString m_photo;
    QString m_logo;
    bool m_autoSubscribe;
    bool m_autoAccept;
    bool m_useHttp;
    QString m_url;
    bool m_infoUpdated;
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
    JabberClient(JabberProtocol*, const QString& name);
    ~JabberClient();
    virtual QString name();
    virtual QString dataName(void*);
    virtual QWidget	*setupWnd();
    virtual QByteArray getConfig();
    virtual QImage userPicture(unsigned id);
    QImage userPicture(JabberUserData *d);

    virtual SIM::IMContact*  getOwnerContact();
    virtual void setOwnerContact(SIM::IMContact* contact);

    void setID(const QString &id);
    QString getID()
    {
        return data.owner.getId();
    }

    //PROP_STR(Server);
    QString getServer() const;
    void setServer(const QString& server);

    //PROP_STR(VHost);
    QString getVHost() const;
    void setVHost(const QString& vhost);

    //PROP_USHORT(Port);
    unsigned short getPort() const;
    void setPort(unsigned long port);
    
    //PROP_BOOL(UseSSL);
    bool getUseSSL() const;
    void setUseSSL(bool b);

    //PROP_BOOL(UsePlain);
    bool getUsePlain() const; 
    void setUsePlain(bool b);

    //PROP_BOOL(UseVHost);
    bool getUseVHost() const;
    void setUseVHost(bool b);

    //PROP_BOOL(Register);
    bool getRegister() const;
    void setRegister(bool b);

    //PROP_ULONG(Priority);
    unsigned long getPriority() const;
    void setPriority(unsigned long p);

    //PROP_UTF8(ListRequest);
    QString getListRequest() const;
    void setListRequest(const QString& request);

    //PROP_BOOL(Typing);
    bool getTyping() const;
    void setTyping(bool t);

    //PROP_BOOL(RichText);
    bool getRichText() const;
    void setRichText(bool rt);

    //PROP_BOOL(UseVersion);
    bool getUseVersion();
    void setUseVersion(bool b);

    //PROP_BOOL(ProtocolIcons);
    bool getProtocolIcons() const;
    void setProtocolIcons(bool b);

    //PROP_USHORT(MinPort);
    unsigned long getMinPort() const;
    void setMinPort(unsigned long port);

    //PROP_USHORT(MaxPort);
    unsigned long getMaxPort() const;
    void setMaxPort(unsigned long port);

    //PROP_UTF8(Photo);
    QString getPhoto() const;
    void setPhoto(const QString& photo);

    //PROP_UTF8(Logo);
    QString getLogo() const;
    void setLogo(const QString& logo);

    //PROP_BOOL(AutoSubscribe);
    bool getAutoSubscribe() const;
    void setAutoSubscribe(bool b);

    //PROP_BOOL(AutoAccept);
    bool getAutoAccept() const;
    void setAutoAccept(bool b);
    
    //PROP_BOOL(UseHTTP);
    bool getUseHTTP() const;
    void setUseHTTP(bool b);

    //PROP_STR(URL);
    QString getURL() const;
    void setURL(const QString& url);

    //PROP_BOOL(InfoUpdated);
    bool getInfoUpdated() const;
    void setInfoUpdated(bool b);

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
    
    JabberUserData* toJabberUserData(SIM::IMContact *); // More safely type conversion from generic SIM::clientData into JabberUserData
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
    virtual bool isMyData(SIM::IMContact*&, SIM::Contact*&);
    virtual bool createData(SIM::IMContact*&, SIM::Contact*);
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


