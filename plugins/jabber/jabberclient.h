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
#include "contacts/imcontact.h"
#include "jabberstatus.h"
#include "log.h"
#include "jabbercontact.h"
#include "jabbergroup.h"
#include "contacts/client.h"
#include "jabber_api.h"

//#include "jabberbuffer.h"

using namespace std;

class JabberProtocol;
class JabberClient;

const unsigned JABBER_SIGN		= 0x0002;

const unsigned SUBSCRIBE_NONE	= 0;
const unsigned SUBSCRIBE_FROM	= 1;
const unsigned SUBSCRIBE_TO		= 2;
const unsigned SUBSCRIBE_BOTH	= (SUBSCRIBE_FROM | SUBSCRIBE_TO);

struct JabberClientData // : public SIM::IMContact
{
public:
    JabberClientData(JabberClient* client);
    virtual QByteArray serialize();
    virtual void deserialize(Buffer* cfg);
    virtual void serialize(QDomElement& /*element*/) {}
    virtual void deserialize(QDomElement& /*element*/) {}

    virtual SIM::Client* client() { Q_ASSERT_X(false, "JabberClientData::client", "Shouldn't be called"); return 0; }

	QString getID() const { return m_ID;}
    void setID(const QString& ID) { m_ID = ID; }

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

    JabberContactPtr owner;

    virtual void deserializeLine(const QString& key, const QString& value);
public:

	QString m_ID;
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

//struct JabberAgentsInfo
//{
//    SIM::Data		VHost;
//    SIM::Data		ID;
//    SIM::Data		Name;
//    SIM::Data		Search;
//    SIM::Data		Register;
//    JabberClient	*Client;
//};

//struct JabberAgentInfo
//{
//    SIM::Data		ReqID;
//    SIM::Data		VHost;
//    SIM::Data		ID;
//    SIM::Data		Field;
//    SIM::Data		Type;
//    SIM::Data		Label;
//    SIM::Data		Value;
//    SIM::Data		Desc;
//    SIM::Data		Options;
//    SIM::Data		OptionLabels;
//    SIM::Data		nOptions;
//    SIM::Data		bRequired;
//};

//struct JabberSearchData
//{
//    SIM::Data		ID;
//    SIM::Data		JID;
//    SIM::Data		First;
//    SIM::Data		Last;
//    SIM::Data		Nick;
//    SIM::Data		EMail;
//    SIM::Data		Status;
//    SIM::Data		Fields;
//    SIM::Data		nFields;
//};

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

class JABBER_EXPORT JabberClient : public QObject, public SIM::Client // : public SIM::TCPClient, public SAXParser
{
    Q_OBJECT
public:
//    class ServerRequest
//    {
//    public:
//        ServerRequest(JabberClient *client, const char *type, const QString &from, const QString &to, const QString &id=QString());
//        virtual ~ServerRequest();
//        void	send();
//        void	start_element(const QString &name);
//        void	end_element(bool bNewLevel = false);
//        void	add_attribute(const QString &name, const char *value);
//        void	add_attribute(const QString &name, const QString &value);
//        void	add_condition(const QString &cond, bool bXData);
//        void	add_text(const QString &text);
//        void	text_tag(const QString &name, const QString &value);
//        static const char *_GET;
//        static const char *_SET;
//        static const char *_RESULT;
//    protected:
//        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
//        virtual void element_end(const QString& el);
//        virtual void char_data(const QString& str);
//        QString  		m_element;
//        QStack<QString>	m_els;
//        QString		    m_id;
//        JabberClient	*m_client;
//        friend class JabberClient;
//    };

//    class IqRequest : public ServerRequest
//    {
//    public:
//        IqRequest(JabberClient *client);
//        ~IqRequest();
//    public:
//        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
//        virtual void element_end(const QString& el);
//        virtual void char_data(const QString& str);
//        QString		*m_data;
//        QString		m_url;
//        QString		m_descr;
//        QString		m_query;
//        QString		m_from;
//        QString		m_id;
//        QString		m_type;
//        QString		m_file_name;
//        unsigned	m_file_size;
//    };

//    class PresenceRequest : public ServerRequest
//    {
//    public:
//        PresenceRequest(JabberClient *client);
//        ~PresenceRequest();
//    protected:
//        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
//        virtual void element_end(const QString& el);
//        virtual void char_data(const QString& str);
//        QString m_from;
//        QString m_data;
//        QString m_type;
//        QString m_status;
//        QString m_show;
//        QString m_stamp1;
//        QString m_stamp2;
//    };

//    class MessageRequest : public ServerRequest
//    {
//    public:
//        MessageRequest(JabberClient *client);
//        ~MessageRequest();
//    protected:
//        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
//        virtual void element_end(const QString& el);
//        virtual void char_data(const QString& str);
//        QString m_from;
//        QString *m_data;
//        QString m_body;
//        QString m_richText;
//        QString m_subj;
//        QString m_error;
//        QString m_contacts;
//        QString m_target;
//        QString m_desc;
//		QString m_enc;
//        vector<QString> m_targets;
//        vector<QString> m_descs;

//        bool   m_bBody;
//        bool   m_bRosters;
//        bool   m_bError;
//        QString m_id;
//        bool   m_bCompose;
//        bool   m_bEvent;
//        bool   m_bRichText;
//		bool   m_bEnc;
//        unsigned m_errorCode;
//    };

//    class StreamErrorRequest : public ServerRequest
//    {
//    public:
//        StreamErrorRequest(JabberClient *client);
//        ~StreamErrorRequest();
//    protected:
//        virtual void element_start(const QString& el, const QXmlAttributes& attrs);
//        virtual void element_end(const QString& el);
//        virtual void char_data(const QString& str);
//        QString *m_data;
//        QString m_descr;
//    };

    JabberClient(JabberProtocol*, const QString& name);
    ~JabberClient();
    virtual QString name();
	virtual QString retrievePasswordLink();
    virtual SIM::IMContactPtr createIMContact();
    virtual SIM::IMGroupPtr createIMGroup();

    virtual QWidget* createSetupWidget(const QString& id, QWidget* parent);
    virtual void destroySetupWidget();
    virtual QStringList availableSetupWidgets() const;

    virtual QWidget* createStatusWidget();

    virtual SIM::IMStatusPtr currentStatus();
    virtual void changeStatus(const SIM::IMStatusPtr& status);
    virtual SIM::IMStatusPtr savedStatus();

    virtual SIM::IMContactPtr ownerContact();
    virtual void setOwnerContact(SIM::IMContactPtr contact);

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(Buffer* cfg);

    virtual QWidget* createSearchWidow(QWidget *parent);
    virtual QList<SIM::IMGroupPtr> groups();
    virtual QList<SIM::IMContactPtr> contacts();

    JabberStatusPtr getDefaultStatus(const QString& id) const;

    void setID(const QString &id);
    QString getID();

    QString getServer() const;
    void setServer(const QString& server);

    QString getVHost() const;
    void setVHost(const QString& vhost);

    unsigned short getPort() const;
    void setPort(unsigned long port);
    
    bool getUseSSL() const;
    void setUseSSL(bool b);

    bool getUsePlain() const;
    void setUsePlain(bool b);

    bool getUseVHost() const;
    void setUseVHost(bool b);

    bool getRegister() const;
    void setRegister(bool b);

    unsigned long getPriority() const;
    void setPriority(unsigned long p);

    QString getListRequest() const;
    void setListRequest(const QString& request);

    bool getTyping() const;
    void setTyping(bool t);

    bool getRichText() const;
    void setRichText(bool rt);

    bool getUseVersion();
    void setUseVersion(bool b);

    bool getProtocolIcons() const;
    void setProtocolIcons(bool b);

    unsigned long getMinPort() const;
    void setMinPort(unsigned long port);

    unsigned long getMaxPort() const;
    void setMaxPort(unsigned long port);

    QString getPhoto() const;
    void setPhoto(const QString& photo);

    QString getLogo() const;
    void setLogo(const QString& logo);

    bool getAutoSubscribe() const;
    void setAutoSubscribe(bool b);

    bool getAutoAccept() const;
    void setAutoAccept(bool b);
    
    bool getUseHTTP() const;
    void setUseHTTP(bool b);

    QString getURL() const;
    void setURL(const QString& url);

    bool getInfoUpdated() const;
    void setInfoUpdated(bool b);

//    QString         buildId(JabberUserData *data);
//    JabberUserData *findContact(const QString &jid, const QString &name, bool bCreate, SIM::Contact *&contact, QString &resource, bool bJoin=true);
//    bool            add_contact(const char *id, unsigned grp);
//    QString         get_agents(const QString &jid);
//    QString         get_agent_info(const QString &jid, const QString &node, const QString &type);
//    void            auth_request(const QString &jid, unsigned type, const QString &text, bool bCreate);
//    QString         search(const QString &jid, const QString &node, const QString &condition);
//    QString         process(const QString &jid, const QString &node, const QString &condition, const QString &type);

//    static QString	get_attr(const char *name, const char **attrs);
//    virtual void setupContact(SIM::Contact*, void *data);
//    virtual void updateInfo(SIM::Contact *contact, void *data);

    JabberClientData* clientPersistentData;

//    JabberListRequest *findRequest(const QString &jid, bool bRemove);

//    QString VHost();
//    bool isAgent(const QString &jid);
//    virtual bool send(SIM::Message*, void*);
//    void    listRequest(JabberUserData *data, const QString &name, const QString &grp, bool bDelete);
//    void	sendFileRequest(SIM::FileMessage *msg, unsigned short port, JabberUserData *data, const QString &url, unsigned size);
//    void	sendFileAccept(SIM::FileMessage *msg, JabberUserData *data);

//    list<SIM::Message*> m_ackMsg;
//    list<SIM::Message*> m_waitMsg;

//    QString photoFile(JabberUserData*);
//    QString logoFile(JabberUserData*);
//    list<ServerRequest*>	m_requests;

//    QString discoItems(const QString &jid, const QString &node);
//    QString discoInfo(const QString &jid, const QString &node);
//    QString browse(const QString &jid);
//    QString versionInfo(const QString &jid, const QString &node = QString::null);
//    QString timeInfo(const QString &jid, const QString &node);
//    QString lastInfo(const QString &jid, const QString &node);
//    QString statInfo(const QString &jid, const QString &node);
//    void addLang(ServerRequest *req);
//    void info_request(JabberUserData *user_data, bool bVCard);
//    virtual void setClientInfo(SIM::IMContact *data);
//    void changePassword(const QString &pass);

//    // reimplement socket() to get correct Buffer
//    virtual JabberClientSocket *socket() { return static_cast<JabberClientSocket*>(TCPClient::socket()); }
//    virtual JabberClientSocket *createClientSocket() { return new JabberClientSocket(this, createSocket()); }

//    virtual void changeStatus(const SIM::IMStatusPtr& status);
    
//    JabberUserData* toJabberUserData(SIM::IMContact *); // More safely type conversion from generic SIM::clientData into JabberUserData
//protected slots:
//    void	ping();
//    void	auth_failed();
//    void	auth_ok();
protected:
//    virtual bool processEvent(SIM::Event *e);
//    SIM::Socket *createSocket();

//    virtual QString contactName(void *clientData);
//    virtual void setStatus(unsigned status);
//    void setStatus(unsigned status, const QString &ar);
//    virtual void disconnected();
//    virtual void connect_ready();
//    virtual void packet_ready();
//    virtual void setInvisible(bool bState);
//    virtual bool isMyData(SIM::IMContact*&, SIM::Contact*&);
//    virtual bool createData(SIM::IMContact*&, SIM::Contact*);
//    virtual bool compareData(void*, void*);
//    virtual bool canSend(unsigned, void*);
//    virtual void contactInfo(void *data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
//    virtual QString resources(void *data);
//    virtual QString contactTip(void *data);
//    virtual QWidget *searchWindow(QWidget *parent);
//    virtual SIM::CommandDef *infoWindows(SIM::Contact *contact, void *data);
//    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact *contact, void *data, unsigned id);
//    virtual SIM::CommandDef *configWindows();
//    virtual QWidget *configWindow(QWidget *parent, unsigned id);

    void init();
    void addDefaultStates();
//    void sendPacket();
//    void startHandshake();
//    void connected();
//    void handshake(const QString &id);
//    void rosters_request();
//    void setOffline(JabberUserData *data);

//    static	QString encodeXML(const QString &str);
//    static	QString encodeXMLattr(const QString &str);
//    QString		m_id;
//    unsigned	m_depth;

//    QString		get_unique_id();
//    unsigned	m_id_seed;
//    unsigned	m_msg_id;

//    bool		m_bHTTP;

//    void element_start(const QString& el, const QXmlAttributes& attrs);
//    void element_end(const QString& el);
//    void char_data(const QString& str);

//    list<JabberListRequest>	m_listRequests;
//    ServerRequest			*m_curRequest;

//    class JabberAuthMessage;
//    vector<JabberAuthMessage*>	tempAuthMessages;

//    QString get_icon(JabberUserData *data, unsigned status, bool invisible);

//    void		processList();

//    void		auth_plain();
//    void		auth_digest();
//    void		auth_register();
//    bool		m_bSSL;
//    bool		m_bJoin;

//    friend class ServerRequest;
//    friend class RostersRequest;
//    friend class PresenceRequest;
//    friend class JabberBrowser;

private:
    QString m_name;
    QList<JabberStatusPtr> m_defaultStates;
    JabberStatusPtr m_currentStatus;
};

//class JabberFileTransfer : public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
//{
//public:
//    JabberFileTransfer(SIM::FileMessage *msg, JabberUserData *data, JabberClient *client);
//    ~JabberFileTransfer();
//    void listen();
//    void connect();
//protected:
//    JabberClient	*m_client;
//    JabberUserData	*m_data;
//    enum State
//    {
//        None,
//        Listen,
//        ListenWait,
//        Header,
//        Send,
//        Wait,
//        Connect,
//        ReadHeader,
//        Receive
//    };
//    State m_state;
//    virtual bool    error_state(const QString &err, unsigned code = 0);
//    virtual void    packet_ready();
//    virtual void    connect_ready();
//    virtual void    write_ready();
//    virtual void    startReceive(unsigned pos);
//    virtual void    bind_ready(unsigned short port);
//    virtual bool    error(const QString &err);
//    virtual bool    accept(SIM::Socket *s, unsigned long ip);
//    bool get_line(const QByteArray &str);
//    void send_line(const QString &str);
//    void send_line(const QByteArray &str);
//    void send_line(const char *str);
//    unsigned m_startPos;
//    unsigned m_endPos;
//    unsigned m_answer;
//    QString             m_url;
//    JabberClientSocket *m_socket;
//};

//class JabberSearch;

//struct agentRegisterInfo
//{
//    QString		id;
//    unsigned		err_code;
//    QString		error;
//};

#endif

// vim: set expandtab:


