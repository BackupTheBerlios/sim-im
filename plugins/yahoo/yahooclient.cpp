/***************************************************************************
                          yahooclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 * Based on libyahoo2
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *             1998-1999, Adam Fritzler <afritz@marko.net>
 *             1998-2002, Rob Flynn <rob@marko.net>
 *             2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *             2001-2002, Brian Macke <macke@strangelove.net>
 *                  2001, Anand Biligiri S <abiligiri@users.sf.net>
 *                  2001, Valdis Kletnieks
 *                  2002, Sean Egan <bj91704@binghamton.edu>
 *                  2002, Toby Gray <toby.gray@ntlworld.com>
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <time.h>

#include <stack>

#include <QTimer>
#include <QTextCodec>
#include <QRegExp>
#include <QFile>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>

#include "html.h"
#include "icons.h"
#include "log.h"
#include "unquot.h"
#include "core.h"
#include "socket/clientsocket.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include "yahoo.h"
#include "yahooclient.h"
#include "yahoocfg.h"
#include "yahooinfo.h"
#include "yahoosearch.h"

using namespace std;
using namespace SIM;

const unsigned long MessageYahooFile	= 0x700;

static char YAHOO_PACKET_SIGN[] = "YMSG";

const unsigned PING_TIMEOUT = 60;

const unsigned	YAHOO_LOGIN_OK		= 0;
const unsigned	YAHOO_LOGIN_PASSWD	= 13;
const unsigned	YAHOO_LOGIN_LOCK	= 14;
const unsigned	YAHOO_LOGIN_DUPL	= 99;

static DataDef yahooUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(9) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "Login", DATA_UTF, 1, 0 },
        { "Nick", DATA_UTF, 1, 0 },
        { "First", DATA_UTF, 1, 0 },
        { "Last", DATA_UTF, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(-1) },	// Status
        { "", DATA_BOOL, 1, 0 },			// bAway
        { "", DATA_UTF, 1, 0 },				// AwayMessage
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "Group", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },			// bChecked
        { "", DATA_BOOL, 1, 0 },			// bTyping
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static DataDef yahooClientData[] =
    {
        { "Server", DATA_STRING, 1, "scs.msg.yahoo.com" },
        { "Port", DATA_ULONG, 1, DATA(5050) },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFE) },
        { "UseHTTP", DATA_BOOL, 1, 0 },
        { "AutoHTTP", DATA_BOOL, 1, DATA(1) },
        { "ListRequests", DATA_STRING, 1, 0 },
        { "", DATA_STRUCT, sizeof(YahooUserData) / sizeof(Data), DATA(yahooUserData) },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

const DataDef *YahooProtocol::userDataDef()
{
    return yahooUserData;
}

YahooClient::YahooClient(Protocol *protocol, Buffer *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(yahooClientData, &data, cfg);
    m_status = STATUS_OFFLINE;
    m_bFirstTry = false;
    m_ft_id = 0;
    QString requests = getListRequests();
    while (!requests.isEmpty()){
        QString request = getToken(requests, ';');
        ListRequest lr;
        lr.type = getToken(request, ',').toUInt();
        lr.name = request;
        m_requests.push_back(lr);
    }
    setListRequests(QString::null);
}

YahooClient::~YahooClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(yahooClientData, &data);
}

QByteArray YahooClient::getConfig()
{
    QByteArray res = TCPClient::getConfig();
    if (res.length())
        res += "\n";
    QString requests;
    for (list<ListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if (!requests.isEmpty())
            requests += ";";
        requests += QString::number(it->type);
        requests += it->name;
    }
    setListRequests(requests);
    res += save_data(yahooClientData, &data);
    return res;
}

bool YahooClient::send(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    YahooUserData *data = toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    Message_ID msg_id;
    switch (msg->type()){
    case MessageTypingStart:
        sendTyping(data, true);
        return true;
    case MessageTypingStop:
        sendTyping(data, false);
        return true;
    case MessageGeneric:
        sendMessage(msg->getRichText(), msg, data);
        return true;
    case MessageUrl:{
            QString msgText = static_cast<UrlMessage*>(msg)->getUrl();
            if (!msg->getPlainText().isEmpty()){
                msgText += "<br>";
                msgText += msg->getRichText();
            }
            sendMessage(msgText, msg, data);
            return true;
        }
    case MessageFile:{
            msg_id.id  = 0;
            msg_id.msg = msg;
            m_waitMsg.push_back(msg_id);
            YahooFileTransfer *ft = static_cast<YahooFileTransfer*>(static_cast<FileMessage*>(msg)->m_transfer);
            if (ft == NULL)
                ft = new YahooFileTransfer(static_cast<FileMessage*>(msg), data, this);
            ft->listen();
            return true;
        }
    }
    return false;
}

bool YahooClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.toULong() != YAHOO_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    switch (type){
    case MessageGeneric:
    case MessageUrl:
    case MessageFile:
        return true;
    }
    return false;
}

void YahooClient::packet_ready()
{
    if (m_bHeader){
        char header[4];
        socket()->readBuffer().unpack(header, 4);
        if (memcmp(header, YAHOO_PACKET_SIGN, 4)){
            socket()->error_state("Bad packet sign");
            return;
        }
        socket()->readBuffer().incReadPos(4);
        socket()->readBuffer() >> m_data_size >> m_service;
        unsigned long session_id;
        socket()->readBuffer() >> m_pkt_status >> session_id;
        if (m_data_size){
            socket()->readBuffer().add(m_data_size);
            m_bHeader = false;
            return;
        }
    }
    EventLog::log_packet(socket()->readBuffer(), false, YahooPlugin::YahooPacket);
    scan_packet();
    socket()->readBuffer().init(20);
    socket()->readBuffer().packetStart();
    m_bHeader = true;
}

void YahooClient::sendPacket(unsigned short service, unsigned long status)
{
    if (m_bHTTP && !m_session_id.isEmpty()){
        addParam(0, getLogin());
        addParam(24, m_session_id);
    }
    unsigned short size = 0;
    if (!m_values.empty()){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            size += 4;
            size += it->second.length();
            size += QByteArray::number(it->first).length();
        }
    }
    socket()->writeBuffer().packetStart();
    socket()->writeBuffer().pack(YAHOO_PACKET_SIGN, 4);
    socket()->writeBuffer() << 0x000C0000L << size << service << status << m_session;
    if (size){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            socket()->writeBuffer()
            << QByteArray::number(it->first).constData()
            << (unsigned short)0xC080
            << it->second.constData()
            << (unsigned short)0xC080;
        }
    }
    m_values.clear();
    EventLog::log_packet(socket()->writeBuffer(), true, YahooPlugin::YahooPacket);
    socket()->write();
}

void YahooClient::addParam(unsigned key, const char *value)
{
    m_values.push_back(PARAM(key, QByteArray(value)));
}

void YahooClient::addParam(unsigned key, const QByteArray &value)
{
    m_values.push_back(PARAM(key, value));
}

void YahooClient::addParam(unsigned key, const QString &value)
{
    m_values.push_back(PARAM(key, value.toUtf8()));
}

void YahooClient::connect_ready()
{
    m_bFirstTry = false;
    socket()->readBuffer().init(20);
    socket()->readBuffer().packetStart();
    m_session = rand();
    m_bHeader = true;
    log(L_DEBUG, "Connect ready");
    TCPClient::connect_ready();
    if (m_bHTTP){
        addParam(1, getLogin());
        sendPacket(YAHOO_SERVICE_AUTH);
    }else{
        sendPacket(YAHOO_SERVICE_VERIFY);
    }
}

const char *Params::operator [](unsigned id)
{
    for (iterator it = begin(); it != end(); ++it){
        if (it->first == id)
            return it->second.data();
    }
    return "";
}

void YahooClient::scan_packet()
{
    Params params;
    int param7_cnt = 0;

    for (;;){
        QByteArray key;
        QByteArray value;
        if (!(socket()->readBuffer().scan("\xC0\x80", key) &&
                socket()->readBuffer().scan("\xC0\x80", value)))
            break;
        unsigned key_id = key.toUInt();
        log(L_DEBUG, "Param: %u %s", key_id, value.data());
        /* There can be multiple buddies in an YAHOO_SERVICE_IDDEACT and
           YAHOO_SERVICE_LOGON paket ... */
        if ((key_id == 7) && ((m_service == YAHOO_SERVICE_IDDEACT) ||
                              (m_service == YAHOO_SERVICE_LOGON))) {
            if (param7_cnt) {
                /* process the last buddie */
                process_packet(params);
                params.clear();
                param7_cnt = 0;
            } else {
                param7_cnt = 1;
            }
        }
        params.push_back(PARAM(key_id, value));
    }
    process_packet(params);
}

void YahooClient::process_packet(Params &params)
{
    log(L_DEBUG,"Service type: %02X",m_service);
    switch (m_service){
    case YAHOO_SERVICE_VERIFY:
        if (m_pkt_status != 1){
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Yahoo! login lock"));
            return;
        }
        addParam(1, getLogin());
        sendPacket(YAHOO_SERVICE_AUTH);
        break;
    case YAHOO_SERVICE_AUTH:
        process_auth(params[13], params[94], params[1]);
        break;
    case YAHOO_SERVICE_AUTHRESP:
        m_pkt_status = 0;
        if (params[66])
            m_pkt_status = atol(params[66]);
        switch (m_pkt_status){
        case YAHOO_LOGIN_OK:
            authOk();
            return;
        case YAHOO_LOGIN_PASSWD:
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Wrong password"), AuthError);
            return;
        case YAHOO_LOGIN_LOCK:
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Your account has been locked"), AuthError);
            return;
        case YAHOO_LOGIN_DUPL:
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        default:
            socket()->error_state(I18N_NOOP("Login failed"));
        }
        break;
    case YAHOO_SERVICE_LIST:
        authOk();
        loadList(params[87]);
        break;
    case YAHOO_SERVICE_LOGOFF:
        if (m_pkt_status == (unsigned long)(-1)){
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        }
    case YAHOO_SERVICE_LOGON:
        if (params[1]){
            if (params[24])
                m_session_id = QString::fromUtf8(params[24]);
            authOk();
        }
    case YAHOO_SERVICE_USERSTAT:
    case YAHOO_SERVICE_ISAWAY:
    case YAHOO_SERVICE_ISBACK:
    case YAHOO_SERVICE_GAMELOGON:
    case YAHOO_SERVICE_GAMELOGOFF:
    case YAHOO_SERVICE_IDACT:
    case YAHOO_SERVICE_IDDEACT:
        if (params[7] && params[13])
            processStatus(m_service, params[7], params[10], params[19], params[47], params[137]);
        break;
    case YAHOO_SERVICE_IDLE:
    case YAHOO_SERVICE_MAILSTAT:
    case YAHOO_SERVICE_CHATINVITE:
    case YAHOO_SERVICE_CALENDAR:
    case YAHOO_SERVICE_NEWPERSONALMAIL:
    case YAHOO_SERVICE_ADDIDENT:
    case YAHOO_SERVICE_ADDIGNORE:
    case YAHOO_SERVICE_PING:
    case YAHOO_SERVICE_GOTGROUPRENAME:
    case YAHOO_SERVICE_GROUPRENAME:
    case YAHOO_SERVICE_PASSTHROUGH2:
    case YAHOO_SERVICE_CHATLOGON:
    case YAHOO_SERVICE_CHATLOGOFF:
    case YAHOO_SERVICE_CHATMSG:
    case YAHOO_SERVICE_REJECTCONTACT:
    case YAHOO_SERVICE_PEERTOPEER:
        break;
    case YAHOO_SERVICE_MESSAGE:
        if (params[4] && params[14])
            process_message(params[4], params[14], params[97]);
        break;
    case YAHOO_SERVICE_NOTIFY:
        if (params[4] && params[49])
            notify(params[4], params[49], params[13]);
        break;
    case YAHOO_SERVICE_NEWCONTACT:
        if (params[1]){
            contact_added(params[3], params[14]);
            return;
        }
        if (params[7]){
            processStatus(m_service, params[7], params[10], params[14], params[47], params[137]);
            return;
        }
        if (m_pkt_status == 7)
            contact_rejected(params[3], params[14]);
        break;
    case YAHOO_SERVICE_P2PFILEXFER:
        if ((params[49] == NULL) || strcmp(params[49], "FILEXFER")){
            log(L_WARN, "Unhandled p2p type %s", params[49]);
            break;
        }
        if ((params[28] == NULL) && params[11]){
            unsigned id =atol(params[11]);
            for (list<Message_ID>::iterator it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
                if (it->id == id){
                    FileMessage *msg = static_cast<FileMessage*>(it->msg);
                    m_waitMsg.erase(it);
                    if (msg->m_transfer){
                        static_cast<YahooFileTransfer*>(msg->m_transfer)->error_state(I18N_NOOP("Message declined"), 0);
                        break;
                    }
                    msg->setError(I18N_NOOP("Message declined"));
                    EventMessageSent(msg).process();
                    delete msg;
                    break;
                }
            }
            break;
        }
    case YAHOO_SERVICE_FILETRANSFER:
        /*

        	params[14] - can be empty when no message is send with the file...
            params[20] - url - just for filetransfer through website
        */
        if (params[4] && params[27] && params[28])
            process_file(params[4], params[27], params[28], params[14], params[20], params[11]);
        else
            /* file-url-message */
            process_fileurl(params[4],params[14],params[20]);
        break;
    case YAHOO_SERVICE_ADDBUDDY:
        if (params[1] && params[7] && params[65])
            log(L_DEBUG,"%s added %s to group %s",params[1],params[7],params[65]);
        else
            log(L_DEBUG,"Please send paket to developer!");
        break;
    case YAHOO_SERVICE_REMBUDDY:
        if (params[1] && params[7] && params[65])
            log(L_DEBUG,"%s removed %s from group %s",params[1],params[7],params[65]);
        else
            log(L_DEBUG,"Please send paket to developer!");
        break;
    case YAHOO_SERVICE_CONFINVITE:
        log(L_WARN, "Conferencing currently not implemented!");
    default:
        log(L_WARN, "Unknown service %02X", m_service);
    }
}

class TextParser
{
public:
    TextParser(YahooClient *client, Contact *contact);
    QString parse(const char *msg);

    class Tag
    {
    public:
        Tag(const QString &str);
        bool operator == (const Tag &t) const;
        QString open_tag() const;
        QString close_tag() const;
    protected:
        QString	m_tag;
    };

class FaceSizeParser : public HTMLParser
    {
    public:
        FaceSizeParser(const QString&);
        QString face;
        QString size;
    protected:
        virtual void text(const QString &text);
        virtual void tag_start(const QString &tag, const list<QString> &options);
        virtual void tag_end(const QString &tag);
    };

protected:
    void addText(const char *str, unsigned size);
    unsigned m_state;
    Contact *m_contact;
    QString color;
    QString face;
    QString size;
    bool m_bChanged;
    stack<Tag> m_tags;
    void setState(unsigned code, bool bSet);
    void clearState(unsigned code);
    void put_color(unsigned color);
    void put_style();
    void push_tag(const QString &tag);
    void pop_tag(const QString &tag);
    YahooClient   *m_client;
    QString m_text;
};

TextParser::FaceSizeParser::FaceSizeParser(const QString &str)
{
    parse(str);
}

void TextParser::FaceSizeParser::text(const QString&)
{
}

void TextParser::FaceSizeParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag != "font")
        return;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        if (key == "face")
            face = QString("font-family:") + *it;
        if (key == "size")
            size = QString("font-size:") + *it + "pt";
    }
}

void TextParser::FaceSizeParser::tag_end(const QString&)
{
}

TextParser::Tag::Tag(const QString &tag)
{
    m_tag	= tag;
}

bool TextParser::Tag::operator == (const Tag &t) const
{
    return close_tag() == t.close_tag();
}

QString TextParser::Tag::open_tag() const
{
    QString res;
    res += "<";
    res += m_tag;
    res += ">";
    return res;
}

QString TextParser::Tag::close_tag() const
{
    int n = m_tag.indexOf(' ');
    QString res;
    res += "</";
    if (n >= 0){
        res += m_tag.left(n);
    }else{
        res += m_tag;
    }
    res += ">";
    return res;
}

TextParser::TextParser(YahooClient *client, Contact *contact)
{
    m_contact  = contact;
    m_client   = client;
    m_bChanged = false;
    m_state    = 0;
}

static unsigned esc_colors[] =
    {
        0x000000,
        0x0000FF,
        0x008080,
        0x808080,
        0x008000,
        0xFF0080,
        0x800080,
        0xFF8000,
        0xFF0000,
        0x808000
    };

QString TextParser::parse(const char *msg)
{
    Buffer b;
    b.pack(msg, strlen(msg));
    for (;;){
        QByteArray part;
        if (!b.scan("\x1B\x5B", part))
            break;
        addText(part, part.length());

        if (!b.scan("m", part))
            break;
        if (part.isEmpty())
            continue;
        if (part[0] == 'x'){
            unsigned code = part.mid(1).toUInt();
            switch (code){
            case 1:
            case 2:
            case 4:
                setState(code, false);
                break;
            }
            continue;
        }
        if (part[0] == '#'){
            put_color(part.mid(1).toUInt());
            continue;
        }
        unsigned code = part.toUInt();
        switch (code){
        case 1:
        case 2:
        case 4:
            setState(code, true);
            break;
        default:
            if ((code >= 30) && (code < 40))
                put_color(esc_colors[code - 30]);
        }
    }
    addText(b.data(b.readPos()), b.writePos() - b.readPos());
    while (!m_tags.empty()){
        m_text += m_tags.top().close_tag();
        m_tags.pop();
    }
    return m_text;
}

void TextParser::setState(unsigned code, bool bSet)
{
    if (bSet){
        if ((m_state & code) == code)
            return;
        m_state |= code;
    }else{
        if ((m_state & code) == 0)
            return;
        m_state &= ~code;
    }
    QString tag;
    switch (code){
    case 1:
        tag = "b";
        break;
    case 2:
        tag = "i";
        break;
    case 4:
        tag = "u";
        break;
    default:
        return;
    }
    if (bSet){
        push_tag(tag);
    }else{
        pop_tag(tag);
    }
}

void TextParser::put_color(unsigned _color)
{
    color.sprintf("color:#%06X", _color & 0xFFFFFF);
    m_bChanged = true;
}

void TextParser::put_style()
{
    if (!m_bChanged)
        return;
    m_bChanged = false;
    QString style;
    if (!color.isEmpty())
        style = color;
    if (!face.isEmpty()){
        if (!style.isEmpty())
            style += ";";
        style += face;
    }
    if (!size.isEmpty()){
        if (!style.isEmpty())
            style += ";";
        style += size;
    }
    QString tag("span style=\"");
    tag += style;
    tag += "\"";
    pop_tag(tag);
    push_tag(tag);
}

void TextParser::push_tag(const QString &tag)
{
    Tag t(tag);
    m_tags.push(t);
    m_text += t.open_tag();
}

void TextParser::pop_tag(const QString &tag)
{
    Tag t(tag);
    stack<Tag> tags;
    bool bFound = false;
    QString text;
    while (!m_tags.empty()){
        Tag top = m_tags.top();
        m_tags.pop();
        text += top.close_tag();
        if (top == t){
            bFound = true;
            break;
        }
        tags.push(top);
    }
    if (bFound)
        m_text += text;
    while (!tags.empty()){
        Tag top = tags.top();
        tags.pop();
        if (bFound)
            m_text += top.open_tag();
        m_tags.push(top);
    }
}

void TextParser::addText(const char *str, unsigned s)
{
    if (s == 0)
        return;
    QString text;
    if (m_contact){
        text = getContacts()->toUnicode(m_contact, QByteArray::fromRawData(str, s));
    }else{
        text = QString::fromUtf8(str, s);
    }
    while (!text.isEmpty()){
        bool bFace = false;
        int n1 = text.indexOf("<font size=\"");
        int n2 = text.indexOf("<font face=\"");
        int n = -1;
        if (n1 >= 0)
            n = n1;
        if ((n2 >= 0) && ((n == -1) || (n2 < n1))){
            n = n2;
            bFace = true;
        }
        if (n < 0){
            if (!text.isEmpty())
                put_style();
            m_text += quoteString(text);
            break;
        }
        if (n)
            put_style();
        m_text += quoteString(text.left(n));
        text = text.mid(n);
        n = text.indexOf('>');
        if (n < 0)
            break;
        FaceSizeParser p(text.left(n + 1));
        text = text.mid(n + 1);
        if (!p.face.isEmpty()){
            face = p.face;
            m_bChanged = true;
        }
        if (!p.size.isEmpty()){
            size = p.size;
            m_bChanged = true;
        }
    }
}

void YahooClient::process_message(const char *id, const char *msg, const char *utf)
{
    bool bUtf = false;
    if (utf && atol(utf))
        bUtf = true;
    Contact *contact = NULL;
    if (utf == NULL){
        if (findContact(id, NULL, contact) == NULL)
            contact = getContacts()->owner();
    }
    Message *m = new Message(MessageGeneric);
    m->setFlags(MESSAGE_RICHTEXT);
    TextParser parser(this, contact);
    m->setText(parser.parse(msg));
    messageReceived(m, id);
}

void YahooClient::notify(const char *id, const char *msg, const char *state)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    bool bState = false;
    if (state && atol(state))
        bState = true;
    if (!qstricmp(msg, "TYPING")){
        if (data->bTyping.toBool() != bState){
            data->bTyping.asBool() = bState;
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
    }
}

void YahooClient::contact_added(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageAdded);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

void YahooClient::contact_rejected(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageRemoved);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

void YahooClient::processStatus(unsigned short service, const char *id,
                                const char *_state, const char *_msg,
                                const char *_away, const char *_idle)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    unsigned long state = 0;
    unsigned away  = 0;
    int idle  = 0;
    if (_state)
        state = atol(_state);
    if (_away)
        away  = atol(_away);
    if (_idle)
        idle  = atol(_idle);
    if (service == YAHOO_SERVICE_LOGOFF)
        state = YAHOO_STATUS_OFFLINE;
    if ((state != data->Status.toULong()) ||
            ((state == YAHOO_STATUS_CUSTOM) &&
            (((away != 0) != data->bAway.toBool()) || QString::fromUtf8(_msg) != data->AwayMessage.str()))){

        unsigned long old_status = STATUS_UNKNOWN;
        unsigned style  = 0;
        QString statusIcon;
        contactInfo(data, old_status, style, statusIcon);

        QDateTime now(QDateTime::currentDateTime());
        now = now.addSecs(-idle);
        if (data->Status.toULong() == YAHOO_STATUS_OFFLINE)
            data->OnlineTime.asULong() = now.toTime_t();
        data->Status.asULong() = state;
        data->bAway.asBool() = (away != 0);
        data->StatusTime.asULong() = now.toTime_t();

        unsigned long new_status = STATUS_UNKNOWN;
        contactInfo(data, new_status, style, statusIcon);

        if (old_status != new_status){
            StatusMessage *m = new StatusMessage();
            m->setContact(contact->id());
            m->setClient(dataName(data));
            m->setFlags(MESSAGE_RECEIVED);
            m->setStatus(STATUS_OFFLINE);
            EventMessageReceived e(m);
            if(!e.process())
                delete m;
            if ((new_status == STATUS_ONLINE) && !contact->getIgnore() && (getState() == Connected) &&
                    (data->StatusTime.toULong() > this->data.owner.OnlineTime.toULong() + 30)){
                        EventContact e(contact, EventContact::eOnline);
                e.process();
            }
        }else{
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
    }
}

QString YahooClient::name()
{
    return "Yahoo." + data.owner.Login.str();
}

QString YahooClient::dataName(void *_data)
{
    YahooUserData *data = toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    return name() + "+" + data->Login.str();
}

void YahooClient::setStatus(unsigned status)
{
    if (status  == m_status)
        return;
    QDateTime now(QDateTime::currentDateTime());
    if (m_status == STATUS_OFFLINE)
        data.owner.OnlineTime.asULong() = now.toTime_t();
    data.owner.StatusTime.asULong() = now.toTime_t();
    m_status = status;
    data.owner.Status.asULong() = m_status;
    EventClientChanged(this).process();
    if (status == STATUS_OFFLINE){
        if (m_status != STATUS_OFFLINE){
            m_status = status;
            data.owner.Status.asULong() = status;
            data.owner.StatusTime.asULong() = time(NULL);
        }
        return;
    }
    unsigned long yahoo_status = YAHOO_STATUS_OFFLINE;
    switch (status){
    case STATUS_ONLINE:
        yahoo_status = YAHOO_STATUS_AVAILABLE;
        break;
    case STATUS_DND:
        yahoo_status = YAHOO_STATUS_BUSY;
        break;
    }
    if (yahoo_status != YAHOO_STATUS_OFFLINE){
        m_status = status;
        sendStatus(yahoo_status);
        return;
    }
    ARRequest ar;
    ar.contact  = NULL;
    ar.status   = status;
    ar.receiver = this;
    ar.param	= (void*)(unsigned long)status;
    EventARRequest(&ar).process();
}

void YahooClient::process_file(const char *id, const char *fileName, const char *fileSize, const char *msg, const char *url, const char *msgid)
{
    YahooFileMessage *m = new YahooFileMessage;
    m->setDescription(getContacts()->toUnicode(NULL, fileName));
    m->setSize(atol(fileSize));
    if (url)
        m->setUrl(url);
    if (msg)
        m->setServerText(msg);
    if (msgid)
        m->setMsgID(atol(msgid));
    messageReceived(m, id);
}

void YahooClient::process_fileurl(const char *id, const char *msg, const char *url)
{
    UrlMessage *m = new UrlMessage(MessageUrl);

    if (msg)
        m->setServerText(msg);
    m->setUrl(url);
    messageReceived(m, id);
}

void YahooClient::disconnected()
{
    m_values.clear();
    m_session_id.clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = toYahooUserData(++it)) != NULL){
            if (data->Status.toULong() != YAHOO_STATUS_OFFLINE){
                data->Status.asULong() = YAHOO_STATUS_OFFLINE;
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(dataName(data));
                m->setStatus(STATUS_OFFLINE);
                m->setFlags(MESSAGE_RECEIVED);
                EventMessageReceived e(m);
                if(!e.process())
                    delete m;
            }
        }
    }
    list<Message*>::iterator itm;
    for (itm = m_ackMsg.begin(); itm != m_ackMsg.end(); ++itm){
        Message *msg = *itm;
        EventMessageDeleted(msg).process();
        delete msg;
    }
    list<Message_ID>::iterator itw;
    for (itw = m_waitMsg.begin(); itw != m_waitMsg.end(); itw = m_waitMsg.begin()){
        Message *msg = itw->msg;
        msg->setError(I18N_NOOP("Client go offline"));
        EventMessageSent(msg).process();
        delete msg;
    }
}

bool YahooClient::isMyData(clientData *&_data, Contact*&contact)
{
    if (_data->Sign.toULong() != YAHOO_SIGN)
        return false;
    YahooUserData *data = toYahooUserData(_data);
    YahooUserData *my_data = findContact(data->Login.str().toUtf8(), NULL, contact);
    if (!my_data){
        contact = NULL;
    }
    return true;
}

bool YahooClient::createData(clientData *&_data, Contact *contact)
{
    YahooUserData *data = toYahooUserData(_data);
    YahooUserData *new_data = toYahooUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    new_data->Nick.str() = data->Nick.str();
    _data = (clientData*)new_data;
    return true;
}

void YahooClient::setupContact(Contact*, void*)
{
}

QWidget	*YahooClient::setupWnd()
{
    return new YahooConfig(NULL, this, false);
}

QString YahooClient::getLogin()
{
    return data.owner.Login.str();
}

void YahooClient::setLogin(const QString &login)
{
    data.owner.Login.str() = login;
}

void YahooClient::authOk()
{
    if (getState() == Connected)
        return;
    if (m_bHTTP && m_session_id.isEmpty())
        return;
    setState(Connected);
    setPreviousPassword(QString::null);
    setStatus(m_logonStatus);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

void YahooClient::loadList(const char *str)
{
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        while ((data = toYahooUserData(++itd)) != NULL){
            data->bChecked.asBool() = (contact->getGroup() == 0);
        }
    }
    if (str){
        QByteArray s = str;
        while (!s.isEmpty()){
            QByteArray line = getToken(s, '\n');
            QByteArray grp = getToken(line, ':');
            if (line.isEmpty()){
                line = grp;
                grp = "";
            }
            while (!line.isEmpty()){
                QByteArray id = getToken(line, ',');
                ListRequest *lr = findRequest(QString::fromUtf8(id));
                if (lr)
                    continue;
                Contact *contact;
                YahooUserData *data = findContact(id, grp, contact, false);
                QString grpName;
                if (contact->getGroup()){
                    Group *grp = getContacts()->group(contact->getGroup());
                    if (grp)
                        grpName = grp->getName();
                }
                if (grpName != getContacts()->toUnicode(NULL, grp))
                    moveBuddy(data, getContacts()->toUnicode(NULL, grp));
                data->bChecked.asBool() = true;
            }
        }
    }
    it.reset();
    for (list<ListRequest>::iterator itl = m_requests.begin(); itl != m_requests.end(); ++itl){
        if ((*itl).type == LR_CHANGE){
            YahooUserData *data = findContact((*itl).name.toUtf8(), NULL, contact, false);
            if (data){
                data->bChecked.asBool() = true;
                QString grpName;
                if (contact->getGroup()){
                    Group *grp = getContacts()->group(contact->getGroup());
                    if (grp)
                        grpName = grp->getName();
                }
                if (grpName != data->Group.str())
                    moveBuddy(data, grpName);
            }
        }
        if ((*itl).type == LR_DELETE){
            YahooUserData data;
            load_data(yahooUserData, &data, NULL);
            data.Login.str() = (*itl).name;
            removeBuddy(&data);
            free_data(yahooUserData, &data);
        }
    }
    m_requests.clear();
    list<Contact*> forRemove;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        list<YahooUserData*> dataForRemove;
        bool bChanged = false;
        while ((data = toYahooUserData(++itd)) != NULL){
            if (!data->bChecked.toBool()){
                dataForRemove.push_back(data);
                bChanged = true;
            }
        }
        if (!bChanged)
            continue;
        for (list<YahooUserData*>::iterator it = dataForRemove.begin(); it != dataForRemove.end(); ++it)
            contact->clientData.freeData(*it);
        if (contact->clientData.size()){
             EventContact e(contact, EventContact::eChanged);
            e.process();
        }else{
            forRemove.push_back(contact);
        }
    }
    for (list<Contact*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
        delete *itr;
}

YahooUserData *YahooClient::findContact(const char *_id, const char *grpname, Contact *&contact, bool bSend, bool bJoin)
{
    ContactList::ContactIterator it;
    QString id = QString::fromUtf8(_id);
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        while ((data = toYahooUserData(++itd)) != NULL){
            if (id == data->Login.str())
                return data;
        }
    }
    it.reset();
    if (bJoin){
        while ((contact = ++it) != NULL){
            if (contact->getName() == id){
                YahooUserData *data = toYahooUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
                data->Login.str() = id;
                data->Group.str() = QString::fromUtf8(grpname);
                EventContact e(contact, EventContact::eChanged);
                e.process();
                return data;
            }
        }
    }
    if (grpname == NULL)
        return NULL;
    Group *grp = NULL;
    if (*grpname){
        ContactList::GroupIterator it;
        while ((grp = ++it) != NULL)
            if (grp->getName() == getContacts()->toUnicode(NULL, grpname))
                break;
        if (grp == NULL){
            grp = getContacts()->group(0, true);
            grp->setName(getContacts()->toUnicode(NULL, grpname));
            EventGroup e(grp, EventGroup::eChanged);
            e.process();
        }
    }
    if (grp == NULL)
        grp = getContacts()->group(0);
    contact = getContacts()->contact(0, true);
    YahooUserData *data = toYahooUserData((SIM::clientData*)contact->clientData.createData(this));  // FIXME unsafe type conversion
    data->Login.str() = id;
    contact->setName(id);
    contact->setGroup(grp->id());
    EventContact e(contact, EventContact::eChanged);
    e.process();
    if (bSend)
        addBuddy(data);
    return data;
}

void YahooClient::messageReceived(Message *msg, const char *id)
{
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    if (msg->contact() == 0){
        Contact *contact;
        YahooUserData *data = findContact(id, NULL, contact);
        if (data == NULL){
            data = findContact(id, "", contact);
            if (data == NULL){
                delete msg;
                return;
            }
            contact->setFlags(CONTACT_TEMP);
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        msg->setClient(dataName(data));
        msg->setContact(contact->id());
    }
    bool bAck = (msg->type() == MessageYahooFile);
    if (bAck){
        msg->setFlags(msg->getFlags() | MESSAGE_TEMP);
        m_ackMsg.push_back(msg);
    }
    EventMessageReceived e(msg);
    if (e.process() && bAck){
        for (list<Message*>::iterator it = m_ackMsg.begin(); it != m_ackMsg.end(); ++it){
            if ((*it) == msg){
                m_ackMsg.erase(it);
                break;
            }
        }
    }
}

static void addIcon(QSet<QString> *s, const QString &icon, const QString &statusIcon)
{
    if (!s || statusIcon == icon)
        return;
    s->insert(icon);
}

void YahooClient::contactInfo(void *_data, unsigned long &status, unsigned&, QString &statusIcon, QSet<QString> *icons)
{
	YahooUserData *data = toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
	unsigned cmp_status = STATUS_OFFLINE;
	switch (data->Status.toULong())
	{
		case YAHOO_STATUS_AVAILABLE:
			cmp_status = STATUS_ONLINE;
			break;
		case YAHOO_STATUS_BUSY:
			cmp_status = STATUS_DND;
			break;
		case YAHOO_STATUS_NOTATHOME:
		case YAHOO_STATUS_NOTATDESK:
		case YAHOO_STATUS_NOTINOFFICE:
		case YAHOO_STATUS_ONVACATION:
			cmp_status = STATUS_NA;
			break;
		case YAHOO_STATUS_OFFLINE:
			break;
		case YAHOO_STATUS_CUSTOM:
			cmp_status = data->bAway.toBool() ? STATUS_AWAY : STATUS_ONLINE;
			break;
		default:
			cmp_status = STATUS_AWAY;
	}

	const CommandDef *def;
    for (def = protocol()->statusList(); !def->text.isEmpty(); def++)
	{
		if (def->id == cmp_status)
			break;
	}
	if (cmp_status > status)
	{
		status = cmp_status;
		if (!statusIcon.isEmpty() && icons){
			icons->insert(statusIcon);
		}
		statusIcon = def->icon;
	}
	else
	{
        if (!statusIcon.isEmpty())
		{
			addIcon(icons, def->icon, statusIcon);
		}
		else
		{
			statusIcon = def->icon;
		}
	}
	if (icons && data->bTyping.toBool())
		addIcon(icons, "typing", statusIcon);
}

QString YahooClient::contactTip(void *_data)
{
    YahooUserData *data = toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    unsigned long status = STATUS_UNKNOWN;
    unsigned style  = 0;
    QString statusIcon;
    contactInfo(data, status, style, statusIcon);
    QString res;
    res += "<img src=\"sim:icons/";
    res += statusIcon;
    res += "\">";
    QString statusText;
    for (const CommandDef *cmd = protocol()->statusList(); !cmd->text.isEmpty(); cmd++){
        if (cmd->icon == statusIcon){
            res += " ";
            statusText = i18n(cmd->text);
            res += statusText;
            break;
        }
    }
    res += "<br>";
    res += data->Login.str();
    res += "</b>";
    if (data->Status.toULong() == YAHOO_STATUS_OFFLINE){
        if (data->StatusTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
    }else{
        if (data->OnlineTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatDateTime(data->OnlineTime.toULong());
        }
        if (data->Status.toULong() != YAHOO_STATUS_AVAILABLE){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
            QString msg;
            switch (data->Status.toULong()){
            case YAHOO_STATUS_BRB:
                msg = i18n("Be right back");
                break;
            case YAHOO_STATUS_NOTATHOME:
                msg = i18n("Not at home");
                break;
            case YAHOO_STATUS_NOTATDESK:
                msg = i18n("Not at my desk");
                break;
            case YAHOO_STATUS_NOTINOFFICE:
                msg = i18n("Not in the office");
                break;
            case YAHOO_STATUS_ONPHONE:
                msg = i18n("On the phone");
                break;
            case YAHOO_STATUS_ONVACATION:
                msg = i18n("On vacation");
                break;
            case YAHOO_STATUS_OUTTOLUNCH:
                msg = i18n("Out to lunch");
                break;
            case YAHOO_STATUS_STEPPEDOUT:
                msg = i18n("Stepped out");
                break;
            case YAHOO_STATUS_CUSTOM:
                msg = data->AwayMessage.str();
            }
            if (!msg.isEmpty()){
                res += "<br>";
                res += quoteString(msg);
            }
        }
    }
    return res;
}

const unsigned MAIN_INFO = 1;
const unsigned NETWORK	 = 2;

static CommandDef yahooWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "Yahoo!_online",
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
        CommandDef (),
    };

static CommandDef cfgYahooWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "Yahoo!_online",
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
            NETWORK,
            I18N_NOOP("Network"),
            "network",
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
        CommandDef (),
    };

CommandDef *YahooClient::infoWindows(Contact*, void *_data)
{
    YahooUserData *data = toYahooUserData((SIM::clientData*)_data);  // FIXME unsafe type conversion
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += data->Login.str();
    yahooWnd[0].text_wrk = name;
    return yahooWnd;
}

CommandDef *YahooClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += data.owner.Login.str();
    cfgYahooWnd[0].text_wrk = name;
    return cfgYahooWnd;
}

QWidget *YahooClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    YahooUserData *data = toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, data, this);
    }
    return NULL;
}

QWidget *YahooClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, NULL, this);
    case NETWORK:
        return new YahooConfig(parent, this, true);
    }
    return NULL;
}

void YahooClient::ping()
{
    if (getState() != Connected)
        return;
    sendPacket(YAHOO_SERVICE_PING);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

class YahooParser : public HTMLParser
{
public:
    YahooParser(const QString&);
    QString res;
    bool bUtf;
protected:
    struct style
    {
        QString		tag;
        QString		face;
        unsigned	size;
        unsigned	color;
        unsigned	state;
    };
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    void set_style(const style &s);
    void set_state(unsigned oldState, unsigned newState, unsigned st);
    void escape(const QString &str);
    bool	m_bFirst;
    QString   esc;
    stack<style>	tags;
    style	curStyle;
};

YahooParser::YahooParser(const QString &str)
{
    bUtf  = false;
    m_bFirst = true;
    curStyle.face  = "Arial";
    curStyle.size  = 10;
    curStyle.color = 0;
    curStyle.state = 0;
    parse(str);
}

void YahooParser::text(const QString &str)
{
    if (str.isEmpty())
        return;
    if (!bUtf){
        for (int i = 0; i < (int)str.length(); i++){
            if (str[i].unicode() > 0x7F){
                bUtf = true;
                break;
            }
        }
    }
    res += esc;
    esc = QString::null;
    res += str;
}

void YahooParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "img"){
        QString src;
        QString alt;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString name = (*it);
            ++it;
            QString value = (*it);
            if (name == "src"){
                src = value;
                break;
            }
            if (name == "alt"){
                alt = value;
                break;
            }
        }
        QStringList smiles = getIcons()->getSmile(src.toLatin1());
        if (smiles.empty()){
            text(alt);
            return;
        }
        text(smiles.front());
        return;
    }
    if (tag == "br"){
        res += "\n";
        return;
    }
    style s = curStyle;
    s.tag = tag;
    tags.push(s);
    if (tag == "p"){
        if (!m_bFirst)
            res += "\n";
        m_bFirst = false;
    }
    if (tag == "font"){
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString name = *it;
            ++it;
            if (name == "color"){
                QColor c;
                c.setNamedColor(*it);
                s.color = c.rgb() & 0xFFFFFF;
            }
        }
    }
    if (tag == "b"){
        s.state |= 1;
        return;
    }
    if (tag == "i"){
        s.state |= 2;
        return;
    }
    if (tag == "u"){
        s.state |= 4;
        return;
    }
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString name = *it;
        ++it;
        if (name != "style")
            continue;
        list<QString> styles = parseStyle(*it);
        for (list<QString>::iterator its = styles.begin(); its != styles.end(); ++its){
            QString name = *its;
            ++its;
            if (name == "color"){
                QColor c;
                c.setNamedColor(*its);
                s.color = c.rgb() & 0xFFFFFF;
            }
            if (name == "font-size"){
                unsigned size = (*its).toUInt();
                if (size)
                    s.size = size;
            }
            if (name == "font-family")
                s.face = (*its);
            if (name == "font-weight")
                s.state &= ~1;
            if ((*its).toUInt() >= 600)
                s.state |= 1;
            if ((name == "font-style") && ((*its) == "italic"))
                s.state |= 2;
            if ((name == "text-decoration") && ((*its) == "underline"))
                s.state |= 4;
        }
    }
    set_style(s);
}

void YahooParser::tag_end(const QString &tag)
{
    style saveStyle =curStyle;
    while (!tags.empty()){
        saveStyle = tags.top();
        tags.pop();
        if (saveStyle.tag == tag)
            break;
    }
    set_style(saveStyle);
}

void YahooParser::set_state(unsigned oldState, unsigned newState, unsigned st)
{
    QString part;
    if ((oldState & st) == (newState & st))
        return;
    if ((newState & st) == 0)
        part = "x";
    part += QString::number(st);
    escape(part);
}

void YahooParser::set_style(const style &s)
{
    set_state(curStyle.state, s.state, 1);
    set_state(curStyle.state, s.state, 2);
    set_state(curStyle.state, s.state, 4);
    curStyle.state = s.state;
    if (curStyle.color != s.color){
        curStyle.color = s.color;
        unsigned i;
        for (i = 0; i < 10; i++){
            if (esc_colors[i] == s.color){
                escape(QString::number(30 + i));
                break;
            }
        }
        if (i >= 10){
            QString str;
            str.sprintf("#%06X", s.color & 0xFFFFFF);
            escape(str);
        }
    }
    QString fontAttr;
    if (curStyle.size != s.size){
        curStyle.size = s.size;
        fontAttr = QString(" size=\"%1\"") .arg(s.size);
    }
    if (curStyle.face != s.face){
        curStyle.face = s.face;
        fontAttr += QString(" face=\"%1\"") .arg(s.face);
    }
    if (!fontAttr.isEmpty()){
        esc += "<font";
        esc += fontAttr;
        esc += ">";
    }
}

void YahooParser::escape(const QString &str)
{
    esc += "\x1B\x5B";
    esc += str;
    esc += "m";
}

void YahooClient::sendMessage(const QString &msgText, Message *msg, YahooUserData *data)
{
    YahooParser p(msgText);
    addParam(0, getLogin());
    addParam(1, getLogin());
    addParam(5, data->Login.str());
    addParam(14, p.res);
    if(p.bUtf)
        addParam(97, "1");
    addParam(63, ";0");
    addParam(64, "0");
    sendPacket(YAHOO_SERVICE_MESSAGE, 0x5A55AA56);
    if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        msg->setClient(dataName(data));
        EventSent(msg).process();
    }
    EventMessageSent(msg).process();
    delete msg;
}

void YahooClient::sendTyping(YahooUserData *data, bool bState)
{
    addParam(5, data->Login.str());
    addParam(4, getLogin());
    addParam(14, " ");
    addParam(13, bState ? "1" : "0");
    addParam(49, "TYPING");
    sendPacket(YAHOO_SERVICE_NOTIFY, 0x16);
}

void YahooClient::addBuddy(YahooUserData *data)
{
    if ((getState() != Connected) || data->Group.str().isEmpty())
        return;
    addParam(1, getLogin());
    addParam(7, data->Login.str());
    addParam(65, data->Group.str());
    sendPacket(YAHOO_SERVICE_ADDBUDDY);
}

void YahooClient::removeBuddy(YahooUserData *data)
{
    if (data->Group.str().isEmpty())
        return;
    addParam(1, getLogin());
    addParam(7, data->Login.str());
    addParam(65, data->Group.str());
    sendPacket(YAHOO_SERVICE_REMBUDDY);
    data->Group.clear();
}

void YahooClient::moveBuddy(YahooUserData *data, const QString &grp)
{
    if (data->Group.str().isEmpty()){
        if (grp.isEmpty())
            return;
        data->Group.str() = grp;
        addBuddy(data);
        return;
    }
    if (grp.isEmpty()){
        removeBuddy(data);
        return;
    }
    if (data->Group.str() == grp)
        return;
    addParam(1, getLogin());
    addParam(7, data->Login.str());
    addParam(65, grp);
    sendPacket(YAHOO_SERVICE_ADDBUDDY);
    addParam(1, getLogin());
    addParam(7, data->Login.str());
    addParam(65, data->Group.str());
    sendPacket(YAHOO_SERVICE_REMBUDDY);
    data->Group.str() = grp;
}

bool YahooClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    switch(e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                ClientDataIterator it(contact->clientData, this);
                YahooUserData *data;
                while ((data = toYahooUserData(++it)) != NULL){
                    if (getState() == Connected){
                        removeBuddy(data);
                    }else{
                        ListRequest *lr = findRequest(data->Login.str());
                        if (lr == NULL){
                            ListRequest r;
                            r.type = LR_DELETE;
                            r.name = data->Login.str();
                            m_requests.push_back(r);
                        }
                    }
                }
                break;
            }
            case EventContact::eChanged: {
                QString grpName;
                Group *grp = NULL;
                if (contact->getGroup())
                    grp = getContacts()->group(contact->getGroup());
                if (grp)
                    grpName = grp->getName();
                ClientDataIterator it(contact->clientData, this);
                YahooUserData *data;
                while ((data = toYahooUserData(++it)) != NULL){
                    if (getState() == Connected){
                        moveBuddy(data, grpName);
                    }else{
                        ListRequest *lr = findRequest(data->Login.str());
                        if (lr == NULL){
                            ListRequest r;
                            r.type = LR_CHANGE;
                            r.name = data->Login.str();
                            m_requests.push_back(r);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventTemplateExpanded: {
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        sendStatus(YAHOO_STATUS_CUSTOM, t->tmpl);
        break;
    }
    case eEventMessageCancel: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        for (list<Message_ID>::iterator it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
            if (it->msg == msg){
                m_waitMsg.erase(it);
                delete msg;
                return true;
            }
        }
        break;
    }
    case eEventMessageAccept: {
        EventMessageAccept *ema = static_cast<EventMessageAccept*>(e);
        for (list<Message*>::iterator it = m_ackMsg.begin(); it != m_ackMsg.end(); ++it){
            if ((*it)->id() == ema->msg()->id()){
                YahooFileMessage *msg = static_cast<YahooFileMessage*>(*it);
                m_ackMsg.erase(it);
                Contact *contact = getContacts()->contact(msg->contact());
                YahooUserData *data;
                ClientDataIterator it(contact->clientData, this);
                while ((data = toYahooUserData(++it)) != NULL){
                    if (dataName(data) == msg->client())
                        break;
                }
                if (data){
                    YahooFileTransfer *ft = new YahooFileTransfer(static_cast<FileMessage*>(msg), data, this);
                    ft->setDir(ema->dir());
                    ft->setOverwrite(ema->mode());
                    EventMessageAcked(msg).process();
                    ft->connect();
                }
                EventMessageDeleted(msg).process();
                if (data == NULL)
                    delete msg;
                return true;
            }
        }
        break;
    }
    case eEventMessageDecline: {
        EventMessageDecline *emd = static_cast<EventMessageDecline*>(e);
        for (list<Message*>::iterator it = m_ackMsg.begin(); it != m_ackMsg.end(); ++it){
            if ((*it)->id() == emd->msg()->id()){
                YahooFileMessage *msg = static_cast<YahooFileMessage*>(*it);
                m_ackMsg.erase(it);
                YahooUserData *data = NULL;
                Contact *contact = getContacts()->contact(msg->contact());
                if (contact){
                    ClientDataIterator itc(contact->clientData, this);
                    while ((data = toYahooUserData(++itc)) != NULL){
                        if (dataName(data) == msg->client())
                            break;
                    }
                }
                if (msg->getMsgID() && data){
                    addParam(5, data->Login.str());
                    addParam(49, "FILEXFER");
                    addParam(1, getLogin());
                    addParam(13, "2");
                    addParam(27, getContacts()->fromUnicode(contact, msg->getDescription()));
                    addParam(53, getContacts()->fromUnicode(contact, msg->getDescription()));
                    addParam(11, QString::number(msg->getMsgID()));
                    sendPacket(YAHOO_SERVICE_P2PFILEXFER);
                }
                QString reason = emd->reason();
                EventMessageDeleted(msg).process();
                delete msg;
                if (!reason.isEmpty() && data){
                    Message *m = new Message(MessageGeneric);
                    m->setText(reason);
                    m->setFlags(MESSAGE_NOHISTORY);
                    if (!send(m, data))
                        delete m;
                }
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

QWidget *YahooClient::searchWindow(QWidget *parent)
{
    if (getState() != Connected)
        return NULL;
    return new YahooSearch(this, parent);
}

void YahooClient::setInvisible(bool bState)
{
    if (bState == getInvisible())
        return;
    TCPClient::setInvisible(bState);
    if (getState() != Connected)
        return;
    sendStatus(data.owner.Status.toULong(), data.owner.AwayMessage.str());
}

void YahooClient::sendStatus(unsigned long _status, const QString &msg)
{
    unsigned long status = _status;
    if (getInvisible())
        status = YAHOO_STATUS_INVISIBLE;
    unsigned long service = YAHOO_SERVICE_ISAWAY;
    if (!msg.isEmpty())
        status = YAHOO_STATUS_CUSTOM;
    /* data.owner.Status contains sim-status, not protocol-status! */
    if (data.owner.Status.toULong() == STATUS_ONLINE)
        service = YAHOO_SERVICE_ISBACK;
    addParam(10, QString::number(status));
    if ((status == YAHOO_STATUS_CUSTOM) && !msg.isEmpty()) {
        addParam(19, msg);
        addParam(47, "1");
    }
    sendPacket(service);
    if (data.owner.Status.toULong() != status)
        data.owner.StatusTime.asULong() = time(NULL);

    data.owner.Status.asULong() = _status;
    data.owner.AwayMessage.str() = msg;
}

void YahooClient::sendFile(FileMessage *msg, QFile *file, YahooUserData *data, unsigned short port)
{
    QString fn = file->fileName();
#if defined( WIN32 ) || defined( __OS2__ )
    fn = fn.replace('\\', '/');
#endif
    int n = fn.lastIndexOf('/');
    if (n > 0)
        fn = fn.mid(n + 1);
    QString url = "http://";
    url += QHostAddress(socket()->localHost()).toString();
    url += ":";
    url += QString::number(port);
    url += '/';
    QString nn;
    Contact *contact;
    findContact(data->Login.str().toUtf8(), NULL, contact);
    QByteArray ff = getContacts()->fromUnicode(contact, fn);
    for (const char *p = ff; *p; p++){
        if (((*p >= 'a') && (*p <='z')) || ((*p >= 'A') && (*p < 'Z')) || ((*p >= '0') && (*p <= '9')) || (*p == '.')){
            nn += *p;
        }else{
            nn += "_";
        }
    }
    url += nn;
    QString m = msg->getPlainText();
    addParam(5, data->Login.str());
    addParam(49, "FILEXFER");
    addParam(1, getLogin());
    addParam(13, "1");
    addParam(27, getContacts()->fromUnicode(contact, fn));
    addParam(28, QString::number(file->size()));
    addParam(20, url);
    addParam(14, getContacts()->fromUnicode(contact, m));
    addParam(53, nn);
    addParam(11, QString::number(++m_ft_id));
    addParam(54, "MSG1.0");
    sendPacket(YAHOO_SERVICE_P2PFILEXFER, YAHOO_STATUS_AVAILABLE);
    for (list<Message_ID>::iterator it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
        if (it->msg == msg){
            it->id = m_ft_id;
            break;
        }
    }
}

YahooUserData* YahooClient::toYahooUserData(SIM::clientData * data)
{
   // This function is used to more safely preform type conversion from SIM::clientData* into YahooUserData*
   // It will at least warn if the content of the structure is not YahooUserData
   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
   if (! data) return NULL;
   if (data->Sign.asULong() != YAHOO_SIGN)
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
        "ATTENTION!! Unsafly converting %s user data into YAHOO_SIGN",
         qPrintable(Sign));
//      abort();
   }
   return (YahooUserData*) data;
}

static Message *createYahooFile(Buffer *cfg)
{
    return new YahooFileMessage(cfg);
}

static MessageDef defYahooFile =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        "File",
        "%n files",
        createYahooFile,
        NULL,
        NULL
    };

static DataDef yahoMessageFile[] =
    {
        { "", DATA_STRING, 1, 0 },				// URL
        { "", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

YahooFileMessage::YahooFileMessage(Buffer *cfg)
        : FileMessage(MessageYahooFile, cfg)
{
    load_data(yahoMessageFile, &data, cfg);
}

YahooFileMessage::~YahooFileMessage()
{
    free_data(yahoMessageFile, &data);
}

QByteArray YahooFileMessage::save()
{
    return save_data(yahoMessageFile, &data);
}

void YahooPlugin::registerMessages()
{
    Command cmd;
    cmd->id			= MessageYahooFile;
    cmd->text		= "YahooFile";
    cmd->icon		= "file";
    cmd->param		= &defYahooFile;
    EventCreateMessageType(cmd).process();
}

void YahooPlugin::unregisterMessages()
{
    EventRemoveMessageType(MessageYahooFile).process();
}

ListRequest *YahooClient::findRequest(const QString &name)
{
    for (list<ListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if (it->name == name)
            return &(*it);
    }
    return NULL;
}

