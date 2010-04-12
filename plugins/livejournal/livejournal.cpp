/***************************************************************************
                          livejournal.cpp  -  description
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

#include "simapi.h"

#include <QTimer>
#include <QRegExp>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDateTime>


#include <stdio.h>

#include "clientmanager.h"
#include "fetch.h"
#include "html.h"
#include "log.h"
#include "unquot.h"
#include "core.h"


//#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"

#include "livejournal.h"
#include "livejournalcfg.h"
#include "msgjournal.h"
#include "journalsearch.h"

using namespace std;
using namespace SIM;

Plugin *createLiveJournalPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new LiveJournalPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createLiveJournalPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef journalMessageData[] =
    {
        { "Subject", DATA_UTF, 1, 0 },
        { "Private", DATA_ULONG, 1, 0 },
        { "Time", DATA_ULONG, 1, 0 },
        { "ItemID", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },		// oldID
        { "Mood", DATA_ULONG, 1, 0 },
        { "Comments", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

JournalMessage::JournalMessage(Buffer *cfg)
        : Message(MessageJournal, cfg)
{
    load_data(journalMessageData, &data, cfg);
}

JournalMessage::~JournalMessage()
{
    free_data(journalMessageData, &data);
}

QByteArray JournalMessage::save()
{
    QByteArray cfg = Message::save();
    QByteArray my_cfg = save_data(journalMessageData, &data);
    if (!my_cfg.isEmpty()){
        if (!cfg.isEmpty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

QString JournalMessage::presentation()
{
    QString subj = getSubject();
    QString res;
    if (!subj.isEmpty())
        res = i18n("<p>Subject: %1</p>") .arg(subj);
    res += Message::presentation();
    return res;
}

#if 0
i18n("LiveJournal post", "%n LiveJournal posts", 1);
i18n("Friends updated", "Friends updated %n", 1);
#endif

static Message *createJournalMessage(Buffer *cfg)
{
    return new JournalMessage(cfg);
}

static QObject* generateJournalMessage(MsgEdit *w, Message *msg)
{
    return new MsgJournal(w, msg);
}

static CommandDef journalMessageCommands[] =
    {
        CommandDef (
            CmdDeleteJournalMessage,
            I18N_NOOP("&Remove from journal"),
            "remove",
            QString::null,
            QString::null,
            ToolBarMsgEdit,
            0x1080,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

static MessageDef defJournalMessage =
    {
        NULL,
        journalMessageCommands,
        MESSAGE_SENDONLY,
        "LiveJournal post",
        "%n LiveJournal posts",
        createJournalMessage,
        generateJournalMessage,
        NULL
    };

static MessageDef defWWWJournalMessage =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY | MESSAGE_SILENT,
        NULL,
        NULL,
        createJournalMessage,
        NULL,
        NULL
    };

static Message *createUpdatedMessage(Buffer *cfg)
{
    return new Message(MessageUpdated, cfg);
}

static MessageDef defUpdatedMessage =
    {
        NULL,
        NULL,
        MESSAGE_SYSTEM,
        "Friends updated",
        "Friends updated %n",
        createUpdatedMessage,
        NULL,
        NULL
    };

unsigned LiveJournalPlugin::MenuCount = 0;

LiveJournalPlugin::LiveJournalPlugin(unsigned base)
        : Plugin(base)
{
    m_protocol = new LiveJournalProtocol(this);

    EventMenu(MenuWeb, EventMenu::eAdd).process();

    Command cmd;
    cmd->id			 = CmdMenuWeb;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuWeb;
    cmd->menu_grp	 = 0x1000;
    cmd->flags		 = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    cmd->id			 = MessageJournal;
    cmd->text		 = I18N_NOOP("LiveJournal &post");
    cmd->icon		 = "LiveJournal";
    cmd->accel		 = "Ctrl+P";
    cmd->menu_grp	 = 0x3080;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defJournalMessage;
    EventCreateMessageType(cmd).process();

    cmd->id			 = CmdMenuWeb;
    cmd->text		 = I18N_NOOP("LiveJournal &WWW");
    cmd->icon		 = QString::null;
    cmd->accel		 = QString::null;
    cmd->menu_grp	 = 0x3090;
    cmd->popup_id	 = MenuWeb;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defWWWJournalMessage;
    EventCreateMessageType(cmd).process();

    cmd->id			 = MessageUpdated;
    cmd->text		 = I18N_NOOP("Friends updated");
    cmd->icon		 = "LiveJournal_upd";
    cmd->accel		 = QString::null;
    cmd->menu_grp	 = 0;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defUpdatedMessage;
    EventCreateMessageType(cmd).process();
}

LiveJournalPlugin::~LiveJournalPlugin()
{
    EventMenu(MenuWeb, EventMenu::eRemove).process();

    EventRemoveMessageType(MessageJournal).process();
    EventRemoveMessageType(CmdMenuWeb).process();
    EventRemoveMessageType(MessageUpdated).process();

    delete m_protocol;
}

LiveJournalProtocol::LiveJournalProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

LiveJournalProtocol::~LiveJournalProtocol()
{
}

ClientPtr LiveJournalProtocol::createClient(Buffer *cfg)
{
	ClientPtr lj = ClientPtr(new LiveJournalClient(this, cfg));
	getClientManager()->addClient(lj);
    return lj;
}

static CommandDef livejournal_descr =
    CommandDef (
        0,
        I18N_NOOP("LiveJournal"),
        "LiveJournal",
        QString::null,
        "http://www.livejournal.com/lostinfo.bml",
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_NOSMS | PROTOCOL_NOPROXY,
        NULL,
        QString::null
    );

const CommandDef *LiveJournalProtocol::description()
{
    return &livejournal_descr;
}

static CommandDef livejournal_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "LiveJournal_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_OFFLINE,
            I18N_NOOP("Offline"),
            "LiveJournal_offline",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

const CommandDef *LiveJournalProtocol::statusList()
{
    return livejournal_status_list;
}

static DataDef liveJournalUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(5) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "User", DATA_UTF, 1, 0 },
        { "Shared", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },						// bChecked
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

/*
	char		*Server;
	char		*URL;
	unsigned	Port;
	unsigned	Interval;
*/

static DataDef liveJournalClientData[] =
    {
        { "Server", DATA_STRING, 1, "www.livejournal.com" },
        { "URL", DATA_STRING, 1, "/interface/flat" },
        { "Port", DATA_ULONG, 1, DATA(80) },
        { "Interval", DATA_ULONG, 1, DATA(5) },
        { "Mood", DATA_STRLIST, 1, 0 },
        { "Moods", DATA_ULONG, 1, 0 },
        { "Menu", DATA_STRLIST, 1, 0 },
        { "MenuURL", DATA_STRLIST, 1, 0 },
        { "FastServer", DATA_BOOL, 1, 0 },
        { "UseFormatting", DATA_BOOL, 1, 0 },
        { "UseSignature", DATA_BOOL, 1, DATA(1) },
        { "Signature", DATA_UTF, 1, 0 },
        { "", DATA_STRING, 1, 0 },			// LastUpdate
        { "", DATA_STRUCT, sizeof(LiveJournalUserData) / sizeof(Data), DATA(liveJournalUserData) },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

const DataDef *LiveJournalProtocol::userDataDef()
{
    return liveJournalUserData;
}

LiveJournalClient::LiveJournalClient(Protocol *proto, Buffer *cfg)
        : TCPClient(proto, cfg)
{
    load_data(liveJournalClientData, &data, cfg);
    m_request = NULL;
    m_timer   = new QTimer(this);
}

LiveJournalClient::~LiveJournalClient()
{
    if (m_request)
        delete m_request;
    free_data(liveJournalClientData, &data);
}

QByteArray LiveJournalClient::getConfig()
{
    QByteArray cfg = TCPClient::getConfig();
    QByteArray my_cfg = save_data(liveJournalClientData, &data);
    if (!my_cfg.isEmpty()){
        if (!cfg.isEmpty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

class MessageRequest : public LiveJournalRequest
{
public:
    MessageRequest(LiveJournalClient *client, JournalMessage *msg, const QString &journal);
    ~MessageRequest();
protected:
    void result(const QString &key, const QString &value);
    JournalMessage *m_msg;
    QString         m_err;
    unsigned        m_id;
    bool            m_bResult;
    bool            m_bEdit;
};

class BRParser : public HTMLParser
{
public:
    BRParser(unsigned color);
    QString m_str;
    void parse(const QString&);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    virtual void add_color();
    bool m_bSkip;
    unsigned m_color;
};

BRParser::BRParser(unsigned color)
{
    m_color = color;
    m_bSkip = false;
    add_color();
}

void BRParser::parse(const QString &str)
{
    HTMLParser::parse(str);
    m_str += "</span>";
}

void BRParser::text(const QString &text)
{
    if (m_bSkip)
        return;
    QString s = text;
    s = s.remove('\r');
    s = s.remove('\n');
    m_str += s;
}

void BRParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (m_bSkip)
        return;
    if (tag == "body"){
        m_str = "";
        add_color();
        return;
    }
    if (tag == "p"){
        return;
    }
    if (tag == "br"){
        m_str += "</span>\n";
        add_color();
        return;
    }
    m_str += "<";
    m_str += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        m_str += " ";
        m_str += name;
        if (!value.isEmpty()){
            m_str += "=\'";
            m_str += quoteString(value);
            m_str += "\'";
        }
    }
    m_str += ">";
}

void BRParser::tag_end(const QString &tag)
{
    if (m_bSkip)
        return;
    if (tag == "body"){
        m_bSkip = true;
        return;
    }
    if (tag == "p"){
        m_str += "</span>\n";
        add_color();
        return;
    }
    m_str += "</";
    m_str += tag;
    m_str += ">";
}

void BRParser::add_color()
{
    QString s;
    s.sprintf("<span style=\"background-color:#%06X\">", m_color & 0xFFFFFF);
    m_str += s;
}

MessageRequest::MessageRequest(LiveJournalClient *client, JournalMessage *msg, const QString &journal)
        : LiveJournalRequest(client, msg->getID() ? "editevent" : "postevent")
{
    m_msg = msg;
    m_bEdit   = (msg->getID() != 0);
    m_bResult = false;
    QString text;
    if (msg->getRichText().isEmpty()){
        text = QString::null;
    }else{
        // if (msg->getFlags() & MESSAGE_RICHTEXT){
        if (client->getUseFormatting()){
            BRParser parser(msg->getBackground());
            parser.parse(msg->getRichText());
            text = parser.m_str;
        }else{
            text = msg->getPlainText();
        }
        addParam("subject", msg->getSubject());
    }
    if (!m_bEdit && client->getUseSignature())
        text += "\n" + client->getSignatureText();
    addParam("event", text);
    addParam("lineendings", "unix");
    if (msg->getID())
        addParam("itemid", QString::number(msg->getID()));
    if (msg->getTime() == 0)
        msg->setTime(QDateTime::currentDateTime().toTime_t());
    QDateTime now = QDateTime::fromTime_t( msg->getTime() );
    now = now.toLocalTime();
    addParam("year", QString::number(now.date().year() + 1900));
    addParam("mon",  QString::number(now.date().month() + 1));
    addParam("day",  QString::number(now.date().day()));
    addParam("hour", QString::number(now.time().hour()));
    addParam("min",  QString::number(now.time().minute()));
    if (msg->getPrivate()){
        switch (msg->getPrivate()){
        case 0:
            addParam("security", "public");
            break;
        case 1:
            addParam("security", "usemask");
            addParam("allowmask", "0");
            break;
        case 2:
            addParam("security", "private");
            break;
        }
    }
    if (msg->getMood())
        addParam("prop_current_moodid", QString::number(msg->getMood()));
    if (!journal.isEmpty())
        addParam("usejournal", journal);
    if (msg->getComments() == COMMENT_NO_MAIL){
        addParam("prop%5Fopt%5Fnoemail", "1");
    }else if (msg->getComments() == COMMENT_DISABLE){
        addParam("prop%5Fopt%5Fnocomments", "1");
    }
}

MessageRequest::~MessageRequest()
{
    if (m_bResult){
        if ((m_msg->getFlags() & MESSAGE_NOHISTORY) == 0){
            if (m_bEdit){
                m_msg->setId(m_msg->getOldID());
                if (m_msg->getRichText().isEmpty()){
                    EventDeleteMessage(m_msg).process();
                }else{
                    EventRewriteMessage(m_msg).process();
                }
            }else{
                m_msg->setID(m_id);
                EventSent(m_msg).process();
            }
        }
    }else{
        if (m_err.isEmpty())
            m_err = I18N_NOOP("Posting failed");
        m_msg->setError(m_err);
    }
    EventMessageSent(m_msg).process();
    delete m_msg;
}

void MessageRequest::result(const QString &key, const QString &value)
{
    if (key == "errmsg")
        m_err = value;
    if (key == "success" && value == "OK")
        m_bResult = true;
    if (key == "itemid")
        m_id = value.toUInt();
}

bool LiveJournalClient::send(Message *msg, void *_data)
{
    if (!canSend(msg->type(), _data))
        return false;
    LiveJournalUserData *data = toLiveJournalUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    QString journal;
    if (data->User.str() != this->data.owner.User.str())
        journal = data->User.str();
    m_requests.push_back(new MessageRequest(this, static_cast<JournalMessage*>(msg), journal));
    msg->setClient(dataName(_data));
    send();
    return true;
}

bool LiveJournalClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.toULong() != LIVEJOURNAL_SIGN))
        return false;
    if (type == MessageJournal){
        if (getState() != Connected)
            return false;
        return true;
    }
    if (type == CmdMenuWeb){
        LiveJournalUserData *data = toLiveJournalUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
        if (data->User.str() != this->data.owner.User.str())
            return false;
        return true;
    }
    return false;
}

void LiveJournalClient::setupContact(Contact*, void*)
{
}

bool LiveJournalClient::createData(clientData*&, Contact*)
{
    return false;
}

bool LiveJournalClient::isMyData(clientData *&data, Contact*&)
{
    if (data->Sign.toULong() != LIVEJOURNAL_SIGN)
        return false;
    return false;
}

QString LiveJournalClient::dataName(void *data)
{
    QString res = name();
    res += ".";
    res += toLiveJournalUserData((SIM::clientData*)data)->User.str(); // FIXME unsafe type conversion
    return res;
}

QString LiveJournalClient::name()
{
    return "LiveJournal." + data.owner.User.str();
}

QWidget	*LiveJournalClient::setupWnd()
{
    return new LiveJournalCfg(NULL, this, true);
}

void LiveJournalClient::socketConnect()
{
    connect_ready();
    setStatus(STATUS_ONLINE);
}

const unsigned MAIN_INFO = 1;

static CommandDef cfgLiveJournalWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "LiveJournal",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

CommandDef *LiveJournalClient::configWindows()
{
    QString title =name();
    int n = title.indexOf('.');
    if (n > 0)
        title = title.left(n) + ' ' + title.mid(n + 1);
    cfgLiveJournalWnd[0].text_wrk = title;
    return cfgLiveJournalWnd;
}

QWidget *LiveJournalClient::configWindow(QWidget *parent, unsigned id)
{
    if (id == MAIN_INFO)
        return new LiveJournalCfg(parent, this, false);
    return NULL;
}

bool LiveJournalClient::add(const QString &name)
{
    Contact *contact;
    LiveJournalUserData *data = findContact(name, contact, false);
    if (data)
        return false;
    findContact(name, contact);
    return true;
}

LiveJournalUserData *LiveJournalClient::findContact(const QString &user, Contact *&contact, bool bCreate, bool bJoin)
{
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = toLiveJournalUserData(++itc)) != NULL){
            if (data->User.str() == user)
                return data;
        }
    }
    if (!bCreate)
        return NULL;
    if (bJoin){
        it.reset();
        while ((contact = ++it) != NULL){
            if (contact->getName().toLower() == user.toLower())
                break;;
        }
    }
    if (contact == NULL){
        contact = getContacts()->contact(0, true);
        contact->setName(user);
    }
    LiveJournalUserData *data = toLiveJournalUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    data->User.str() = user;
    EventContact e(contact, EventContact::eChanged);
    e.process();
    return data;
}

void LiveJournalClient::auth_ok()
{
    m_status = STATUS_ONLINE;
    setState(Connected);
    setPreviousPassword(QString::null);
    statusChanged();
    list<Contact*> forDelete;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = toLiveJournalUserData(++itc)) != NULL){
            if (!data->Shared.toBool())
                continue;
            if (data->bChecked.toBool())
                continue;
            contact->clientData.freeData(data);
            if (contact->clientData.size() == 0)
                forDelete.push_back(contact);
            break;
        }
    }
    for (list<Contact*>::iterator itc = forDelete.begin(); itc != forDelete.end(); ++itc)
        delete (*itc);
    QTimer::singleShot(0, this, SLOT(timeout()));
}

void LiveJournalClient::statusChanged()
{
    Contact *contact = NULL;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ClientDataIterator itc(contact->clientData, this);
        if ((++itc) != NULL){
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
    }
    findContact(data.owner.User.str(), contact);
}

QString LiveJournalClient::getSignatureText()
{
    QString res = getSignature();
    if (res.isEmpty())
        res = i18n("<div style=\"text-align:right;font-size:0.7em;font-style:italic;width:100%\">Powered by <a style=\"font-size:1em;font-style:italic;\" href=\"http://sim-im.org\">SIM Instant Messenger</a></div>");
    return res;
}

static void addIcon(QSet<QString> *s, const QString &icon, const QString &statusIcon)
{
    if (!s || statusIcon == icon)
        return;
    s->insert(icon);
}

void LiveJournalClient::contactInfo(void*, unsigned long &curStatus, unsigned&, QString &statusIcon, QSet<QString> *icons)
{
    unsigned long status = STATUS_OFFLINE;
    const char *dicon = "LiveJournal_offline";
    if ((getState() == Connected) && (m_status != STATUS_OFFLINE)){
        status = STATUS_ONLINE;
        dicon = "LiveJournal_online";
    }
    if (status > curStatus){
        curStatus = status;
        if (!statusIcon.isEmpty() && icons){
            icons->insert(statusIcon);
        }
        statusIcon = dicon;
    }else{
        if (!statusIcon.isEmpty()){
            addIcon(icons, dicon, statusIcon);
        }else{
            statusIcon = dicon;
        }
    }
}

struct Mood
{
    unsigned	id;
    QString     name;
};

class LoginRequest : public LiveJournalRequest
{
public:
    LoginRequest(LiveJournalClient *client);
    ~LoginRequest();
protected:
    void result(const QString &key, const QString &value);
    bool m_bOK;
    bool m_bResult;
    vector<Mood> m_moods;
    QString m_err;
};

LoginRequest::LoginRequest(LiveJournalClient *client)
        : LiveJournalRequest(client, "login")
{
    m_bOK     = false;
    m_bResult = false;
}

LoginRequest::~LoginRequest()
{
    if (m_bOK){
        for (unsigned i = 0; i < m_moods.size(); i++){
            if (m_moods[i].name.isEmpty())
                continue;
            if (m_client->getMoods() < m_moods[i].id)
                m_client->setMoods(m_moods[i].id);
            m_client->setMood(i, m_moods[i].name);
        }
        m_client->auth_ok();
    }else{
        if (!m_bResult)
            return;
        if (m_err.isEmpty())
            m_err = I18N_NOOP("Login failed");
        m_client->auth_fail(m_err);
    }
    EventClientChanged(m_client).process();
}

void LoginRequest::result(const QString &key, const QString &value)
{
    m_bResult = true;
    if (key == "success" && value == "OK"){
        m_bOK = true;
        return;
    }
    if (key == "errmsg"){
        m_err = value;
        return;
    }
    QString k = key;
    QString prefix = getToken(k, '_');
    if (prefix == "mood"){
        prefix = getToken(k, '_');
        unsigned id = prefix.toUInt();
        if (id == 0)
            return;
        while (m_moods.size() <= id){
            Mood m;
            m_moods.push_back(m);
        }
        if (k == "id")
            m_moods[id].id = value.toUInt();
        if (k == "name")
            m_moods[id].name = value;
    }
    if (prefix == "menu"){
        prefix = getToken(k, '_');
        unsigned menu_id = prefix.toUInt();
        prefix = getToken(k, '_');
        unsigned item_id = prefix.toUInt();
        if (item_id == 0)
            return;
        unsigned id = menu_id * 0x100 + item_id;
        if (k == "text")
            m_client->setMenu(id, value);
        if (k == "url")
            m_client->setMenuUrl(id, value);
        if (k == "sub"){
            QString v = "@";
            v += value;
            m_client->setMenuUrl(id, v);
        }
    }
    if (prefix == "access"){
        if (k.toULong() == 0)
            return;
        Contact *contact;
        LiveJournalUserData *data = m_client->findContact(value, contact);
        if (data){
            data->bChecked.asBool() = true;
            data->Shared.asBool()   = true;
        }
    }
}

void LiveJournalClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE)
        return;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = toLiveJournalUserData(++itc)) != NULL){
            data->bChecked.asBool() = false;
            if (data->User.str() == this->data.owner.User.str())
                data->bChecked.asBool() = true;
        }
    }
    LiveJournalRequest *req = new LoginRequest(this);
    QString version;
#ifdef WIN32
    version = "Win32";
#else
#ifdef Q_OS_MAC
    version = "MacOS";
#else
    version = "Qt";
#endif
#endif
    version += "-" PACKAGE "/" VERSION;
    req->addParam("clientversion", version);
    req->addParam("getmoods", QString::number(getMoods()));
    req->addParam("getmenus", "1");
    m_requests.push_back(req);
    send();
}

void LiveJournalClient::disconnected()
{
    m_timer->stop();
    statusChanged();
}

void LiveJournalClient::packet_ready()
{
}

void LiveJournalClient::auth_fail(const QString &err)
{
    m_reconnect = NO_RECONNECT;
    error_state(err, AuthError);
}

QWidget *LiveJournalClient::searchWindow(QWidget *parent)
{
    return new JournalSearch(this, parent);
}

bool LiveJournalClient::done(unsigned code, Buffer &data, const QString &)
{
    if (code == 200){
        m_request->result(&data);
    }else{
        QString err = "Fetch error ";
        err += QString::number(code);
        error_state(err, 0);
        statusChanged();
    }
    delete m_request;
    m_request = NULL;
    send();
    return false;
}

bool LiveJournalClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    if (e->type() == eEventOpenMessage){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->type() != MessageUpdated)
            return false;
        if (dataName(&data.owner) != msg->client())
            return false;
        EventMessageDeleted(msg).process();
        QString url = "http://";
        url += getServer();
        if (getPort() != 80){
            url += ":";
            url += QString::number(getPort());
        }
        url += '/';
        EventGoURL(url).process();
        if (getState() == Connected) {
            m_timer->setSingleShot(true);
            m_timer->start(getInterval() * 60 * 1000);
        }
        return true;
    }
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdDeleteJournalMessage){
            Message *msg = (Message*)(cmd->param);
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact == NULL)
                return false;
            LiveJournalUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = toLiveJournalUserData(++it)) != NULL){
                if (dataName(data) == msg->client()){
                    Buffer cfg;
                    cfg = "[Title]\n" + msg->save();
                    cfg.setWritePos(0);
                    cfg.getSection();
                    JournalMessage *m = new JournalMessage(&cfg);
                    m->setContact(msg->contact());
                    m->setOldID(msg->id());
                    m->setText("");
                    if (!send(m, data))
                        delete m;
                    return true;
                }
            }
            return false;
        }
        unsigned menu_id = cmd->menu_id - MenuWeb;
        if (menu_id > LiveJournalPlugin::MenuCount)
            return false;
        unsigned item_id = cmd->id - CmdMenuWeb;
        if ((item_id == 0) || (item_id >= 0x100))
            return false;
        QString url = getMenuUrl(menu_id * 0x100 + item_id);
        if (url.isEmpty())
            return false;
        EventGoURL eUrl(url);
        eUrl.process();
        return true;
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdMenuWeb){
            unsigned menu_id = cmd->menu_id - MenuWeb;
            if (menu_id > LiveJournalPlugin::MenuCount)
                return false;
            unsigned nItems = 0;
            unsigned list_id = menu_id * 0x100 + 1;
            for (;;){
                if (getMenu(list_id).isEmpty())
                    break;
                nItems++;
                list_id++;
            }
            if (nItems == 0)
                return false;
            CommandDef *cmds = new CommandDef[nItems + 1];
            list_id = menu_id * 0x100 + 1;
            for (unsigned i = 0;; i++){
                QString text = getMenu(list_id);
                if (text.isEmpty())
                    break;
                cmds[i].text = "_";
                if (text !=  "-"){
                    cmds[i].id = CmdMenuWeb + i + 1;
                    cmds[i].text = "_";
                    cmds[i].text_wrk = i18n(text);
                    QString url = getMenuUrl(list_id);
                    if (url.startsWith("@")){
                        url = url.mid(1);
                        unsigned nSub = url.toUInt();
                        while (nSub > LiveJournalPlugin::MenuCount){
                            unsigned long menu_id = MenuWeb + (++LiveJournalPlugin::MenuCount);
                            EventMenu(menu_id, EventMenu::eAdd).process();
                            CommandDef c;
                            c.id       = CmdMenuWeb;
                            c.text     = "_";
                            c.menu_id  = menu_id;
                            c.menu_grp = 0x1000;
                            c.flags    = COMMAND_CHECK_STATE;
                            EventCommandCreate(&c).process();
                        }
                        cmds[i].popup_id = MenuWeb + nSub;
                    }
                }else{
                    cmds[i].id = 0;
                }
                list_id++;
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return true;
        }
    }
    return false;
}

void LiveJournalClient::send()
{
    if ((m_requests.size() == 0) || m_request)
        return;
    m_request = m_requests.front();
    m_requests.erase(m_requests.begin());
    QString url;
    url = "http://";
    url += getServer();
    if (getPort() != 80){
        url += ":";
        url += QString::number(getPort());
    }
    url += getURL();
    QString headers = "Content-Type: application/x-www-form-urlencoded";
    if (getFastServer())
        headers += "\nCookie: ljfastserver=1";
    fetch(url, headers, m_request->m_buffer);
    m_request->m_buffer = NULL;
}

bool LiveJournalClient::error_state(const QString &err, unsigned code)
{
    return TCPClient::error_state(err, code);
}

class CheckFriendsRequest : public LiveJournalRequest
{
public:
    CheckFriendsRequest(LiveJournalClient *client);
    ~CheckFriendsRequest();
protected:
    void result(const QString &key, const QString &value);
    bool        m_bOK;
    bool        m_bChanged;
    unsigned	m_interval;
    QString     m_err;
};

CheckFriendsRequest::CheckFriendsRequest(LiveJournalClient *client)
        : LiveJournalRequest(client, "checkfriends")
{
    m_bOK		= false;
    m_bChanged	= false;
    m_interval	= 0;
    addParam("lastupdate", client->getLastUpdate());
}

void LiveJournalClient::messageUpdated()
{
    Contact *contact;
    LiveJournalUserData *data = findContact(this->data.owner.User.str(), contact);
    if (data == NULL)
        return;
    Message *msg = new Message(MessageUpdated);
    msg->setContact(contact->id());
    msg->setClient(dataName(data));
    msg->setFlags(MESSAGE_TEMP | MESSAGE_NOVIEW);
    EventMessageReceived e(msg);
    if (!e.process())
        delete msg;
}

CheckFriendsRequest::~CheckFriendsRequest()
{
    if (m_bChanged){
        m_client->messageUpdated();
        return;
    }
    if (m_bOK){
        m_client->m_timer->setSingleShot( true );
        m_client->m_timer->start( m_interval );
        return;
    }
    m_client->error_state(m_err, 0);
}

void CheckFriendsRequest::result(const QString &key, const QString &value)
{
    if (key == "success" && value == "OK"){
        m_bOK = true;
        return;
    }
    if (key == "lastupdate"){
        m_client->setLastUpdate(value);
        return;
    }
    if (key == "new"){
        if (value.toULong())
            m_bChanged = true;
        return;
    }
    if (key == "interval"){
        m_interval = value.toUInt();
        return;
    }
    if (key == "errmsg"){
        m_err = value;
        return;
    }
}

void LiveJournalClient::timeout()
{
    if (getState() != Connected)
        return;
    m_timer->stop();
    m_requests.push_back(new CheckFriendsRequest(this));
    send();
}


LiveJournalUserData* LiveJournalClient::toLiveJournalUserData(SIM::clientData * data)
{
   // This function is used to more safely preform type conversion from SIM::clientData* into LiveJournalUserData*
   // It will at least warn if the content of the structure is not LiveJournalUserData*
   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
   if (! data) return NULL;
   if (data->Sign.asULong() != LIVEJOURNAL_SIGN)
   {
      QString Signs[] = {
        "Unknown(0)" ,     // 0x0000
        "ICQ_SIGN",        // 0x0001
        "JABBER_SIGN",     // 0x0002
        "MSN_SIGN",        // 0x0003
        "Unknown(4)"       // 0x0004
        "LIVEJOURNAL_SIGN",// 0x0005
        "SMS_SIGN",        // 0x0006
        "Unknown(7)",      // 0x0007
        "Unknown(8)",      // 0x0008
        "YAHOO_SIGN"       // 0x0009
      };
      QString Sign;
      if (data->Sign.toULong()<=9) // is always >=0 as it is unsigned int
        Sign = Signs[data->Sign.toULong()];
      else
        Sign = QString("Unknown(%1)").arg(Sign.toULong());

      log(L_ERROR,
        "ATTENTION!! Unsafly converting %s user data into LIVEJOURNAL_SIGN",
         qPrintable(Sign));
//      abort();
   }
   return (LiveJournalUserData*) data;
}

LiveJournalRequest::LiveJournalRequest(LiveJournalClient *client, const char *mode)
{
    m_client = client;
    m_buffer = new Buffer;
    addParam("mode", mode);
    addParam("ver", "1");
    if (!client->data.owner.User.str().isEmpty())
        addParam("user", client->data.owner.User.str());
    QByteArray pass = QCryptographicHash::hash(client->getPassword().toUtf8(), QCryptographicHash::Md5);
    QString hpass;
    for (int i = 0; i < pass.size(); i++){
      char b[5];
      sprintf(b, "%02x", pass[(int)i] & 0xFF);
      hpass += b;
    }
    addParam("hpassword", hpass);
}

LiveJournalRequest::~LiveJournalRequest()
{
    if (m_buffer)
        delete m_buffer;
}

void LiveJournalRequest::addParam(const QString &key, const QString &value)
{
    if (m_buffer->size())
        m_buffer->pack("&", 1);
    m_buffer->pack(key.toUtf8(), key.toUtf8().length());
    m_buffer->pack("=", 1);
    QByteArray cstr = value.toUtf8();
    for (int i = 0; i < cstr.length(); i++){
        char c = cstr[(int)i];
        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) ||
                (c == '.') || (c == '-') || (c == '/') || (c == '_')){
            m_buffer->pack(&c, 1);
        }else{
            char buf[4];
            sprintf(buf, "%%%02X", c & 0xFF);
            m_buffer->pack(buf, 3);
        }
    }
}

void LiveJournalRequest::result(Buffer *b)
{
    for (;;){
        QByteArray key;
        QByteArray value;
        if (!getLine(b, key) || !getLine(b, value))
            break;
        log(L_DEBUG, "Result: %s=%s", key.data(), value.data());
        result(QString::fromUtf8(key), QString::fromUtf8(value));
    }
}

bool LiveJournalRequest::getLine(Buffer *b, QByteArray &line)
{
    if (b == NULL)
        return false;
    if (!b->scan("\n", line))
        return false;
    if (line.length() && (line[(int)line.length() - 1] == '\r'))
        line = line.left(line.length() - 1);
    return true;
}

#if 0
I18N_NOOP("Invalid username")

I18N_NOOP("aggravated")
I18N_NOOP("angry")
I18N_NOOP("annoyed")
I18N_NOOP("anxious")
I18N_NOOP("bored")
I18N_NOOP("confused")
I18N_NOOP("crappy")
I18N_NOOP("cranky")
I18N_NOOP("depressed")
I18N_NOOP("discontent")
I18N_NOOP("energetic")
I18N_NOOP("enraged")
I18N_NOOP("enthralled")
I18N_NOOP("exhausted")
I18N_NOOP("happy")
I18N_NOOP("high")
I18N_NOOP("horny")
I18N_NOOP("hungry")
I18N_NOOP("infuriated")
I18N_NOOP("irate")
I18N_NOOP("jubilant")
I18N_NOOP("lonely")
I18N_NOOP("moody")
I18N_NOOP("pissed off")
I18N_NOOP("sad")
I18N_NOOP("satisfied")
I18N_NOOP("sore")
I18N_NOOP("stressed")
I18N_NOOP("thirsty")
I18N_NOOP("thoughtful")
I18N_NOOP("tired")
I18N_NOOP("touched")
I18N_NOOP("lazy")
I18N_NOOP("drunk")
I18N_NOOP("ditzy")
I18N_NOOP("mischievous")
I18N_NOOP("morose")
I18N_NOOP("gloomy")
I18N_NOOP("melancholy")
I18N_NOOP("drained")
I18N_NOOP("excited")
I18N_NOOP("relieved")
I18N_NOOP("hopeful")
I18N_NOOP("amused")
I18N_NOOP("determined")
I18N_NOOP("scared")
I18N_NOOP("frustrated")
I18N_NOOP("indescribable")
I18N_NOOP("sleepy")
I18N_NOOP("groggy")
I18N_NOOP("hyper")
I18N_NOOP("relaxed")
I18N_NOOP("restless")
I18N_NOOP("disappointed")
I18N_NOOP("curious")
I18N_NOOP("mellow")
I18N_NOOP("peaceful")
I18N_NOOP("bouncy")
I18N_NOOP("nostalgic")
I18N_NOOP("okay")
I18N_NOOP("rejuvenated")
I18N_NOOP("complacent")
I18N_NOOP("content")
I18N_NOOP("indifferent")
I18N_NOOP("silly")
I18N_NOOP("flirty")
I18N_NOOP("calm")
I18N_NOOP("refreshed")
I18N_NOOP("optimistic")
I18N_NOOP("pessimistic")
I18N_NOOP("giggly")
I18N_NOOP("pensive")
I18N_NOOP("uncomfortable")
I18N_NOOP("lethargic")
I18N_NOOP("listless")
I18N_NOOP("recumbent")
I18N_NOOP("exanimate")
I18N_NOOP("embarrassed")
I18N_NOOP("envious")
I18N_NOOP("sympathetic")
I18N_NOOP("sick")
I18N_NOOP("hot")
I18N_NOOP("cold")
I18N_NOOP("worried")
I18N_NOOP("loved")
I18N_NOOP("awake")
I18N_NOOP("working")
I18N_NOOP("productive")
I18N_NOOP("accomplished")
I18N_NOOP("busy")
I18N_NOOP("full")
I18N_NOOP("grumpy")
I18N_NOOP("weird")
I18N_NOOP("nauseated")
I18N_NOOP("ecstatic")
I18N_NOOP("chipper")
I18N_NOOP("rushed")
I18N_NOOP("contemplative")
I18N_NOOP("nerdy")
I18N_NOOP("geeky")
I18N_NOOP("cynical")
I18N_NOOP("quixotic")
I18N_NOOP("crazy")
I18N_NOOP("creative")
I18N_NOOP("artistic")
I18N_NOOP("pleased")
I18N_NOOP("bitchy")
I18N_NOOP("guilty")
I18N_NOOP("irritated")
I18N_NOOP("blank")
I18N_NOOP("apathetic")
I18N_NOOP("dorky")
I18N_NOOP("impressed")
I18N_NOOP("naughty")
I18N_NOOP("predatory")
I18N_NOOP("dirty")
I18N_NOOP("giddy")
I18N_NOOP("surprised")
I18N_NOOP("shocked")
I18N_NOOP("rejected")
I18N_NOOP("numb")
I18N_NOOP("cheerful")
I18N_NOOP("good")
I18N_NOOP("distressed")
I18N_NOOP("intimidated")
I18N_NOOP("crushed")
I18N_NOOP("devious")
I18N_NOOP("thankful")
I18N_NOOP("grateful")
I18N_NOOP("jealous")
I18N_NOOP("nervous")

I18N_NOOP("Recent Entries")
I18N_NOOP("Calendar View")
I18N_NOOP("Friends View")
I18N_NOOP("Your Profile")
I18N_NOOP("Your To-Do List")
I18N_NOOP("Change Settings")
I18N_NOOP("Support")
I18N_NOOP("Personal Info")
I18N_NOOP("Customize Journal")
I18N_NOOP("Journal Settings")
I18N_NOOP("Upgrade your account")

#endif



