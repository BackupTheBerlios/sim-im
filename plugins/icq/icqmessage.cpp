/***************************************************************************
                          icqmessage.cpp  -  description
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
#include <QLayout>
#include <QTextCodec>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegExp>
#include <QList>
#include <QByteArray>

#include <memory>

#include "simgui/toolbtn.h"
#include "log.h"

#include "icqmessage.h"
#include "icqclient.h"
#include "icq.h"
#include "core.h"
#include "xml.h"
#include "contacts/contact.h"

using namespace std;
using namespace SIM;

static DataDef aimFileMessageData[] =
    {
        { "", DATA_ULONG, 1, 0 },				// Port
        { "", DATA_ULONG, 1, 0 },				// ID_L
        { "", DATA_ULONG, 1, 0 },				// ID_H
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

AIMFileMessage::AIMFileMessage(Buffer *cfg)
        : FileMessage(MessageFile, cfg)
{
    load_data(aimFileMessageData, &data, cfg);
	isProxy = false;
}

AIMFileMessage::~AIMFileMessage()
{
    free_data(aimFileMessageData, &data);
}

static DataDef icqFileMessageData[] =
    {
        { "ServerDescr", DATA_STRING, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// IP
        { "", DATA_ULONG, 1, 0 },				// Port
        { "", DATA_ULONG, 1, 0 },				// ID_L
        { "", DATA_ULONG, 1, 0 },				// ID_H
        { "", DATA_ULONG, 1, 0 },				// Cookie
        { "", DATA_ULONG, 1, 0 },				// Extended
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

ICQFileMessage::ICQFileMessage(Buffer *cfg)
        : FileMessage(MessageICQFile, cfg)
{
    load_data(icqFileMessageData, &data, cfg);
}

ICQFileMessage::~ICQFileMessage()
{
    free_data(icqFileMessageData, &data);
}

QString ICQFileMessage::getDescription()
{
    QString serverText = getServerDescr();
    if (serverText.isEmpty())
        return FileMessage::getDescription();
    return serverText;
}

QByteArray ICQFileMessage::save()
{
    QByteArray s = FileMessage::save();
    QByteArray s1 = save_data(icqFileMessageData, &data);
    if (!s1.isEmpty()){
        if (!s.isEmpty())
            s += '\n';
        s += s1;
    }
    return s;
}

IcqContactsMessage::IcqContactsMessage(Buffer *cfg)
        : ContactsMessage(MessageICQContacts, cfg)
{
}

IcqContactsMessage::~IcqContactsMessage()
{
}

QString IcqContactsMessage::getContacts() const
{
    QByteArray serverText = getServerText();
    if (serverText.isEmpty())
        return ContactsMessage::getContacts();
    return serverText;  // this is wrong ... but I'm currently unsure what'ssaved in there
}

static DataDef icqAuthMessageData[] =
    {
        { "Charset", DATA_STRING, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

ICQAuthMessage::ICQAuthMessage(unsigned type, unsigned baseType, Buffer *cfg)
        : AuthMessage(type, cfg)
{
    load_data(icqAuthMessageData, &data, cfg);
    m_baseType = baseType;
}

ICQAuthMessage::~ICQAuthMessage()
{
    free_data(icqAuthMessageData, &data);
}

QString ICQAuthMessage::getText() const
{
    QByteArray serverText = getServerText();
    if (serverText.isEmpty())
        return Message::getText();
    QString charset = getCharset();
    if (!charset.isEmpty()){
        QTextCodec *codec = QTextCodec::codecForName(charset.toUtf8().constData());
        if (codec)
            return codec->toUnicode(serverText);
    }
    return Message::getText();
}

QByteArray ICQAuthMessage::save()
{
    QByteArray s = Message::save();
    QByteArray s1 = save_data(icqAuthMessageData, &data);
    if (!s1.isEmpty()){
        if (!s.isEmpty())
            s += '\n';
        s += s1;
    }
    return s;
}

static bool h2b(const char *&p, char &r)
{
    char c = *(p++);
    if ((c >= '0') && (c <= '9')){
        r = (char)(c - '0');
        return true;
    }
    if ((c >= 'A') && (c <= 'F')){
        r = (char)(c - 'A' + 10);
        return true;
    }
    return false;
}

static bool h2b(const char *&p, QByteArray &cap)
{
    char r1, r2;
    if (h2b(p, r1) && h2b(p, r2)){
        cap += (char)((r1 << 4) + r2);
        return true;
    }
    return false;
}

static bool parseFE(QByteArray str, QList<QByteArray> &l, unsigned n)
{
    int idx = str.indexOf('\xFE');
    while(idx != -1) {
        l += str.left(idx);
        str = str.mid(idx+1);
        idx = str.indexOf('\xFE');
    }
    l += str;
    for( unsigned i = l.count(); i < n; i++ )
        l += QByteArray();
    return true;
}

static Message *parseTextMessage(const QByteArray &str, const QByteArray &_pp, Contact *contact)
{
    if (str.isEmpty())
        return NULL;
    log(L_DEBUG, "Text message: %s %s", str.data(), _pp.data());
    if (_pp.length() == 38){
        QByteArray cap;
        const char *pp = _pp.data();
        if ((*(pp++) == '{') &&
                h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '}')){
            const char *unpack_cap = cap.data();
            if (!memcmp(unpack_cap, ICQClient::capabilities[CAP_RTF], sizeof(capability))){
                Message *msg = new Message(MessageGeneric);
                QString text;
                if (ICQClient::parseRTF(str, contact, text))
                    msg->setFlags(MESSAGE_RICHTEXT);
                log(L_DEBUG, "Msg: %s", str.data());
                msg->setText(text);
                return msg;
            }
            if (!memcmp(unpack_cap, ICQClient::capabilities[CAP_UTF], sizeof(capability))){
                Message *msg = new Message(MessageGeneric);
                msg->setText(QString::fromUtf8(str));
                return msg;
            }
        }
    }
    Message *m = new Message;
    m->setServerText(str);
    return m;
}

static Message *parseURLMessage(const QByteArray &str)
{
    QList<QByteArray> l;
    if (!parseFE(str, l, 2)){
        log(L_WARN, "Parse error URL message");
        return NULL;
    }
    UrlMessage *m = new UrlMessage;
    m->setServerText(l[0]);
    m->setUrl(l[1]);
    return m;
}

//ToDo: Send Contacts does not work
static Message *parseContactMessage(const QByteArray &str)
{
    QList<QByteArray> l;
    if (!parseFE(str, l, 2)){
        log(L_WARN, "Parse error contacts message");
        return NULL;
    }
    unsigned nContacts = l[0].toUInt();
    if (nContacts == 0){
        log(L_WARN, "No contacts found");
        return NULL;
    }
    QList<QByteArray> c;
    if (!parseFE(l[1], c, nContacts*2+1)){
        log(L_WARN, "Parse error contacts message");
        return NULL;
    }
    QByteArray serverText;
    for (unsigned i = 0; i < nContacts; i++){
        QByteArray screen = c[i*2];
        QByteArray alias  = c[i*2+1];
        if (!serverText.isEmpty())
            serverText += ';';
        if (screen.toULong()){
            serverText += "icq:";
            serverText += screen;
            serverText += '/';
            serverText += alias;
            serverText += ',';
            if (screen == alias){
                serverText += "ICQ ";
                serverText += screen;
            }else{
                serverText += alias;
                serverText += " (ICQ ";
                serverText += screen;
                serverText += ')';
            }
        }else{
            serverText += "aim:";
            serverText += screen;
            serverText += '/';
            serverText += alias;
            serverText += ',';
            if (screen == alias){
                serverText += "AIM ";
                serverText += screen;
            }else{
                serverText += alias;
                serverText += " (AIM ";
                serverText += screen;
                serverText += ')';
            }
        }
    }
    IcqContactsMessage *m = new IcqContactsMessage;
    m->setServerText(serverText);
    return m;
}

static Message *parseAuthRequest(const QByteArray &str)
{
    QList<QByteArray> l;
    if (!parseFE(str, l, 6)){
        log(L_WARN, "Parse error auth request message");
        return NULL;
    }
    ICQAuthMessage *m = new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest);
    m->setServerText(l[4]);
    return m;
}

Message *ICQClient::parseExtendedMessage(const QString &screen, ICQBuffer &packet, MessageId &id, unsigned cookie)
{
    string header;
    packet >> header;
	ICQBuffer h(header.size());
	//QString strheader(header);
	h.pack(header.c_str(), header.size());
	h.incReadPos(16);
    unsigned short msg_type;
    h >> msg_type;
    //QString msgType;
	string msgType;
    h.unpackStr32(msgType);
	//QString info;
    string info;
    packet.unpackStr32(info);
	ICQBuffer b(info.size());
	b.pack(info.c_str(), info.size());

    log(L_DEBUG, "Extended message %s [%04X] %u", msgType.data(), msg_type, (unsigned int)info.length());

    int n = msgType.find("URL");
    if (n >= 0){
        QByteArray info;
        b.unpackStr32(info);
        return parseURLMessage(info);
    }
    if (msgType == "Request For Contacts"){
        QByteArray info;
        b.unpackStr32(info);
        ICQAuthMessage *m = new ICQAuthMessage(MessageContactRequest, MessageContactRequest);
		m->setServerText(info);
        return m;
    }
    if (msgType == "Contacts"){
        QByteArray p;
        b.unpackStr32(p);
        return parseContactMessage(p);
    }
    if (msgType == "Message"){
        QByteArray p;
        b.unpackStr32(p);
        unsigned long forecolor, backcolor;
        b >> forecolor >> backcolor;
        QByteArray cap_str;
        b.unpackStr32(cap_str);
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, false, contact);
        if (data == NULL) {
            data = findContact(screen, NULL, true, contact);
            if (data == NULL) {
               return NULL;
	    }
            contact->setFlags(contact->getFlags() | CONTACT_TEMP);
        }
        Message *msg = parseTextMessage(p, cap_str, contact);
        if (msg){
            if (forecolor != backcolor){
                msg->setForeground(forecolor >> 8);
                msg->setBackground(backcolor >> 8);
            }
        }
        return msg;
    }
    n = msgType.find("File");
    if (n >= 0){
        string fileDescr, fileName;
        b.unpackStr32(fileDescr);
        unsigned short port;
        b >> port;
        b.incReadPos(2);
        b >> fileName;
        unsigned long fileSize;
        b.unpack(fileSize);
        ICQFileMessage *m = new ICQFileMessage;
//#ifdef __OS2__  // to make it compileable under OS/2 (gcc 3.3.5)
        m->setServerDescr(fileName.c_str());
//#else
//        m->setServerDescr(fileName);
//#endif
        m->setServerText(QByteArray(fileDescr.data()));
        m->setSize(fileSize);
        m->setPort(port);
        m->setFlags(MESSAGE_TEMP);
        m->setID_L(id.id_l);
        m->setID_H(id.id_h);
        m->setCookie(cookie);
        m->setExtended(true);
        return m;
    }
    if (msgType == "ICQSMS"){
        string p;
        b.unpackStr32(p);
		//p = QCString(info).data();   // FIXME!!
        string::iterator s = p.begin();
        auto_ptr<XmlNode> top(XmlNode::parse(s, p.end()));
        if (top.get() == NULL){
            log(L_WARN, "Parse SMS XML error");
            return NULL;
        }
        if (msg_type == 0){
            if (top->getTag() != "sms_message"){
                log(L_WARN, "No sms_message tag in SMS message");
                return NULL;
            }
            XmlNode *n = top.get();
            if ((n == NULL) || !n->isBranch()){
                log(L_WARN, "Parse no branch");
                return NULL;
            }
            XmlBranch *sms_message = static_cast<XmlBranch*>(n);
            XmlLeaf *text = sms_message->getLeaf("text");
            if (text == NULL){
                log(L_WARN, "No <text> in SMS message");
                return NULL;
            }
            SMSMessage *m = new SMSMessage;
            XmlLeaf *sender = sms_message->getLeaf("sender");
            if (sender != NULL){
                m->setPhone(QString::fromUtf8(sender->getValue().c_str()));
                // string -> QString is ok here since phone doesn't contain non ascii chars
                Contact *contact = getContacts()->contactByPhone(sender->getValue().c_str());
                m->setContact(contact->id());
            }
            XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
            if (senders_network != NULL)
                m->setNetwork(QString::fromUtf8(senders_network->getValue().c_str()));
            m->setText(QString::fromUtf8(text->getValue().c_str()));
            return m;
        }
    }
    if (msgType == "StatusMsgExt"){
        StatusMessage *m = new StatusMessage;
        return m;
    }
    log(L_WARN, "Unknown extended message type %s", msgType.data());
    return NULL;
}

Message *ICQClient::parseMessage(unsigned short type, const QString &screen, const QByteArray &p, ICQBuffer &packet, MessageId &id, unsigned cookie)
{
    if (screen.toULong() == 0x0A){
        QList<QByteArray> l;
        if (!parseFE(p, l, 6)){
            log(L_WARN, "Parse error web panel message");
            return NULL;
        }
        char SENDER_IP[] = "Sender IP:";
        QByteArray head = l[5].left(strlen(SENDER_IP));
        Message *msg = new Message((head == SENDER_IP) ? MessageWebPanel : MessageEmailPager);
        QString name = getContacts()->toUnicode(NULL, l[0]);
        QString mail = getContacts()->toUnicode(NULL, l[3]);
        msg->setServerText(l[5]);
        Contact *contact = getContacts()->contactByMail(mail, name);
        if (contact == NULL){
            delete msg;
            return NULL;
        }
        msg->setContact(contact->id());
        return msg;
    }
    log(L_DEBUG, "Parse message [type=%u]", type);
    Message *msg = NULL;
    switch (type){
    case ICQ_MSGxMSG:{
            unsigned long forecolor, backcolor;
            packet >> forecolor >> backcolor;
            QByteArray cap_str;
            packet.unpackStr32(cap_str);
            Contact *contact;
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data == NULL) {
                data = findContact(screen, NULL, true, contact);
                if (data == NULL) {
                   return NULL;
                }
                contact->setFlags(contact->getFlags() | CONTACT_TEMP);
            }
            msg = parseTextMessage(p, cap_str, contact);
            if (msg == NULL)
                break;
            if (forecolor != backcolor){
                msg->setForeground(forecolor >> 8);
                msg->setBackground(backcolor >> 8);
            }
            break;
        }
    case ICQ_MSGxURL:
        msg = parseURLMessage(p);
        break;
    case ICQ_MSGxAUTHxREQUEST:
        msg = parseAuthRequest(p);
        break;
    case ICQ_MSGxAUTHxGRANTED:
        msg = new AuthMessage(MessageAuthGranted);
        break;
    case ICQ_MSGxAUTHxREFUSED:
        msg = new AuthMessage(MessageAuthRefused);
        break;
    case ICQ_MSGxADDEDxTOxLIST:
        msg = new AuthMessage(MessageAdded);
        break;
    case ICQ_MSGxCONTACTxLIST:
        msg = parseContactMessage(p);
        break;
    case ICQ_MSGxFILE:{
            ICQFileMessage *m = new ICQFileMessage;
            m->setServerText(p);
            unsigned short port;
            unsigned long  fileSize;
            QByteArray fileName;
            packet >> port;
            packet.incReadPos(2);
            packet >> fileName;
            packet.unpack(fileSize);
            m->setPort(port);
            m->setSize(fileSize);
            m->setServerDescr(fileName);
            msg = m;
            break;
        }
    case ICQ_MSGxEXT:
        msg = parseExtendedMessage(screen, packet, id, cookie);
        break;
    default:
        log(L_WARN, "Unknown message type %04X", type);
    }
    return msg;
}

static Message *createIcqFile(Buffer *cfg)
{
    return new ICQFileMessage(cfg);
}

#if 0
i18n("File", "%n files", 1);
#endif

static MessageDef defIcqFile =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        "File",
        "%n files",
        createIcqFile,
        NULL,
        NULL
    };

#if 0
i18n("WWW-panel message", "%n WWW-panel messages", 1);
#endif

static Message *createWebPanel(Buffer *cfg)
{
    return new Message(MessageWebPanel, cfg);
}

static MessageDef defWebPanel =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "WWW-panel message",
        "%n WWW-panel messages",
        createWebPanel,
        NULL,
        NULL
    };

#if 0
i18n("Email pager message", "%n Email pager messages", 1);
#endif

static Message *createEmailPager(Buffer *cfg)
{
    return new Message(MessageEmailPager, cfg);
}

static MessageDef defEmailPager =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Email pager message",
        "%n Email pager messages",
        createEmailPager,
        NULL,
        NULL
    };

#if 0
i18n("Request secure channel", "%n requests secure channel", 1);
#endif

static Message *createOpenSecure(Buffer *cfg)
{
    return new Message(MessageOpenSecure, cfg);
}

static MessageDef defOpenSecure =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY,
        "Request secure channel",
        "%n requests secure channel",
        createOpenSecure,
        NULL,
        NULL
    };

#if 0
i18n("Close secure channel", "%n times close secure channel", 1);
#endif

static Message *createCloseSecure(Buffer *cfg)
{
    return new Message(MessageCloseSecure, cfg);
}

static MessageDef defCloseSecure =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_SENDONLY,
        "Close secure channel",
        "%n times close secure channel",
        createCloseSecure,
        NULL,
        NULL
    };

#if 0
i18n("Warning", "%n warnings", 1);
#endif

static DataDef warningMessageData[] =
    {
        { "Anonymous", DATA_BOOL, 1, 0 },
        { "OldLevel", DATA_ULONG, 1, 0 },
        { "NewLevel", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

WarningMessage::WarningMessage(Buffer *cfg)
        : AuthMessage(MessageWarning, cfg)
{
    load_data(warningMessageData, &data, cfg);
}

QByteArray WarningMessage::save()
{
    QByteArray res = AuthMessage::save();
    if (!res.isEmpty())
        res += '\n';
    return res + save_data(warningMessageData, &data);
}

QString WarningMessage::presentation()
{
    return QString("Increase warning level from %1% to %2%")
           .arg(ICQClient::warnLevel(getOldLevel()))
           .arg(ICQClient::warnLevel(getNewLevel()));
}

static Message *createWarning(Buffer *cfg)
{
    return new WarningMessage(cfg);
}

static MessageDef defWarning =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY,
        "Warning",
        "%n warnings",
        createWarning,
        NULL,
        NULL
    };

static Message *createIcqAuthRequest(Buffer *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest, cfg);
}

static MessageDef defIcqAuthRequest =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        NULL,
        NULL,
        createIcqAuthRequest,
        NULL,
        NULL
    };

static Message *createIcqAuthGranted(Buffer *cfg)
{
    return new ICQAuthMessage(MessageICQAuthGranted, MessageAuthGranted, cfg);
}

static MessageDef defIcqAuthGranted =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        NULL,
        NULL,
        createIcqAuthGranted,
        NULL,
        NULL
    };

static Message *createIcqAuthRefused(Buffer *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRefused, MessageAuthRefused, cfg);
}

static MessageDef defIcqAuthRefused =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        NULL,
        NULL,
        createIcqAuthRefused,
        NULL,
        NULL
    };

static Message *createContactRequest(Buffer *cfg)
{
    return new ICQAuthMessage(MessageContactRequest, MessageContactRequest, cfg);
}

#if 0
i18n("Contact request", "%n contact requests", 1);
#endif

static MessageDef defContactRequest =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT | MESSAGE_SYSTEM,
        "Contact request",
        "%n contact requests",
        createContactRequest,
        NULL,
        NULL
    };

static Message *createIcqContacts(Buffer *cfg)
{
    return new IcqContactsMessage(cfg);
}

static MessageDef defIcqContacts =
    {
        NULL,
        NULL,
        MESSAGE_CHILD,
        NULL,
        NULL,
        createIcqContacts,
        NULL,
        NULL
    };

void ICQPlugin::registerMessages()
{
    Command cmd;

    cmd->id			 = MessageICQContacts;
    cmd->text		 = "ICQContacts";
    cmd->icon		 = "contacts";
    cmd->param		 = &defIcqContacts;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageICQFile;
    cmd->text		= "ICQFile";
    cmd->icon		= "file";
    cmd->param		= &defIcqFile;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageContactRequest;
    cmd->text		= I18N_NOOP("Contact Request");
    cmd->icon		= "contacts";
    cmd->param		= &defContactRequest;
    EventCreateMessageType(cmd).process();

    cmd->id         = MessageICQAuthRequest;
    cmd->text       = "ICQAuthRequest";
    cmd->icon       = "auth";
    cmd->param      = &defIcqAuthRequest;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageICQAuthGranted;
    cmd->text		= "ICQAuthGranted";
    cmd->icon		= "auth";
    cmd->param		= &defIcqAuthGranted;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageICQAuthRefused;
    cmd->text		= "ICQAuthRefused";
    cmd->icon		= "auth";
    cmd->param		= &defIcqAuthRefused;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageWebPanel;
    cmd->text		= I18N_NOOP("Web panel");
    cmd->icon		= "web";
    cmd->param		= &defWebPanel;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageEmailPager;
    cmd->text		= I18N_NOOP("Email pager");
    cmd->icon		= "mailpager";
    cmd->param		= &defEmailPager;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageOpenSecure;
    cmd->text		= I18N_NOOP("Request secure channel");
    cmd->icon		= "encrypted";
    cmd->menu_grp	= 0x30F0;
    cmd->param		= &defOpenSecure;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageCloseSecure;
    cmd->text		= I18N_NOOP("Close secure channel");
    cmd->icon		= "encrypted";
    cmd->menu_grp	= 0x30F0;
    cmd->param		= &defCloseSecure;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageWarning;
    cmd->text		= I18N_NOOP("Warning");
    cmd->icon		= "error";
    cmd->menu_grp	= 0x30F2;
    cmd->param		= &defWarning;
    EventCreateMessageType(cmd).process();

    cmd->id			= CmdUrlInput;
    cmd->text		= I18N_NOOP("&URL");
    cmd->icon		= "empty";
    cmd->icon_on	= QString::null;
    cmd->bar_id		= ToolBarMsgEdit;
    cmd->bar_grp	= 0x1030;
    cmd->menu_id	= 0;
    cmd->menu_grp	= 0;
    cmd->flags		= BTN_EDIT | BTN_NO_BUTTON | COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();
}

void ICQPlugin::unregisterMessages()
{
    EventRemoveMessageType(MessageICQUrl).process();
    EventRemoveMessageType(MessageICQContacts).process();
    EventRemoveMessageType(MessageICQ).process();
    EventRemoveMessageType(MessageICQFile).process();
    EventRemoveMessageType(MessageICQAuthRequest).process();
    EventRemoveMessageType(MessageICQAuthGranted).process();
    EventRemoveMessageType(MessageICQAuthRefused).process();
    EventRemoveMessageType(MessageContactRequest).process();
    EventRemoveMessageType(MessageWebPanel).process();
    EventRemoveMessageType(MessageEmailPager).process();
    EventRemoveMessageType(MessageOpenSecure).process();
    EventRemoveMessageType(MessageCloseSecure).process();
    EventRemoveMessageType(MessageWarning).process();

    EventCommandRemove(CmdUrlInput).process();
}

void ICQClient::packExtendedMessage(Message *msg, ICQBuffer &buf, ICQBuffer &msgBuf, ICQUserData *data)
{
    unsigned short port = 0;
    switch (msg->type()){
    case MessageICQFile:
        port = static_cast<ICQFileMessage*>(msg)->getPort();
    case MessageFile:
        buf.pack((char*)plugins[PLUGIN_FILE], sizeof(plugin));
        buf.packStr32("File");
        buf << 0x00000100L << 0x00010000L << 0x00000000L << (unsigned short)0 << (char)0;
        //msgBuf.packStr32(getContacts()->fromUnicode(getContact(data), msg->getPlainText()));
		string msgdata1 = getContacts()->fromUnicode(getContact(data), msg->getPlainText()).data();
		msgBuf.packStr32(msgdata1.c_str());
        msgBuf << port << (unsigned short)0;
		//msgBuf << getContacts()->fromUnicode(getContact(data), static_cast<FileMessage*>(msg)->getDescription());
		string msgdata2 = getContacts()->fromUnicode(getContact(data), static_cast<FileMessage*>(msg)->getDescription()).data();
        msgBuf << msgdata2.c_str();
        msgBuf.pack((unsigned long)(static_cast<FileMessage*>(msg)->getSize()));
        msgBuf << 0x00000000L;
        break;
    }
}

QString ICQClient::packContacts(ContactsMessage *msg, ICQUserData *, CONTACTS_MAP &c)
{
    QString contacts = msg->getContacts();
    QString newContacts;
    while (!contacts.isEmpty()){
        QString contact = getToken(contacts, ';');
        QString url = getToken(contact, ',');
        QString proto = getToken(url, ':');
        if (proto == "sim"){
            Contact *contact = getContacts()->contact(url.toULong());
            if (contact){
                ClientDataIterator it(contact->clientData);
                clientData *cdata;
                while ((cdata = ++it) != NULL){
                    Contact *cc = contact;
                    if (!isMyData(cdata, cc))
                        continue;
                    ICQUserData *d = toICQUserData(cdata);
                    QString screen = this->screen(d);
                    CONTACTS_MAP::iterator it = c.find(screen);
                    if (it == c.end()){
                        alias_group ci;
                        ci.alias = contact->getName();
                        ci.grp   = cc ? cc->getGroup() : 0;
                        c.insert(CONTACTS_MAP::value_type(screen, ci));
                        if (!newContacts.isEmpty())
                            newContacts += ';';
                        if (screen.toULong()){
                            newContacts += "icq:";
                            newContacts += screen;
                            newContacts += '/';
                            newContacts += contact->getName();
                            newContacts += ',';
                            if (contact->getName() == screen){
                                newContacts += "ICQ ";
                                newContacts += screen;
                            }else{
                                newContacts += contact->getName();
                                newContacts += " (ICQ ";
                                newContacts += screen;
                                newContacts += ')';
                            }
                        }else{
                            newContacts += "aim:";
                            newContacts += screen;
                            newContacts += '/';
                            newContacts += contact->getName();
                            newContacts += ',';
                            if (contact->getName() == screen){
                                newContacts += "AIM ";
                                newContacts += screen;
                            }else{
                                newContacts += contact->getName();
                                newContacts += " (AIM ";
                                newContacts += screen;
                                newContacts += ')';
                            }
                        }
                    }
                }
            }
        }
        if ((proto == "icq") || (proto == "aim")){
            QString screen = getToken(url, '/');
            if (url.isEmpty())
                url = screen;
            CONTACTS_MAP::iterator it = c.find(screen);
            if (it == c.end()){
                alias_group ci;
                ci.alias = url;
                ci.grp   = 0;
                c.insert(CONTACTS_MAP::value_type(screen, ci));
            }
        }
    }
    return newContacts;
}

void ICQClient::packMessage(ICQBuffer &b, Message *msg, ICQUserData *data, unsigned short &type, bool bDirect, unsigned short flags)
{
    ICQBuffer msgBuf, buf;
    QString res;
    switch (msg->type()){
    case MessageUrl:
        res = getContacts()->fromUnicode(getContact(data), msg->getPlainText());
        res += '\xFE';
        res += getContacts()->fromUnicode(getContact(data), static_cast<UrlMessage*>(msg)->getUrl());
        type = ICQ_MSGxURL;
        break;
    case MessageContacts:{
            CONTACTS_MAP c;
            QString nc = packContacts(static_cast<ContactsMessage*>(msg), data, c);
            if (c.empty()){
                msg->setError(I18N_NOOP("No contacts for send"));
                return;
            }
            static_cast<ContactsMessage*>(msg)->setContacts(nc);
            res = QString::number(c.size());
            for (CONTACTS_MAP::iterator it = c.begin(); it != c.end(); ++it){
                res += '\xFE';
                res += getContacts()->fromUnicode(getContact(data), it->first.str());
                res += '\xFE';
                res += getContacts()->fromUnicode(getContact(data), it->second.alias);
            }
            res += '\xFE';
            type = ICQ_MSGxCONTACTxLIST;
            break;
        }
    case MessageICQFile:
        if (!static_cast<ICQFileMessage*>(msg)->getExtended()){
            res = getContacts()->fromUnicode(getContact(data), msg->getPlainText());
            type = ICQ_MSGxFILE;
            break;
        }
    case MessageFile:	// FALLTHROW
        type = ICQ_MSGxEXT;
        packExtendedMessage(msg, buf, msgBuf, data);
        break;
    case MessageOpenSecure:
        type = ICQ_MSGxSECURExOPEN;
        break;
    case MessageCloseSecure:
        type = ICQ_MSGxSECURExCLOSE;
        break;
    }
    if (flags == ICQ_TCPxMSG_NORMAL){
        if (msg->getFlags() & MESSAGE_URGENT)
            flags = ICQ_TCPxMSG_URGENT;
        if (msg->getFlags() & MESSAGE_LIST)
            flags = ICQ_TCPxMSG_LIST;
    }
    if (bDirect || (type == ICQ_MSGxEXT)){
        b.pack(type);
        b.pack(msgStatus());
        b.pack(flags);
    }else{
        b.pack(this->data.owner.Uin.toULong());
        b.pack((char)type);
        b.pack((char)0x01);
    }
    b << res;
    if (buf.size()){
        b.pack((unsigned short)buf.size());
        b.pack(buf.data(0), buf.size());
        b.pack32(msgBuf);
    }
}

void ICQClient::parsePluginPacket(ICQBuffer &b, unsigned plugin_type, ICQUserData *data, unsigned uin, bool bDirect)
{
    b.incReadPos(1);
    unsigned short type;
    b >> type;
    b.incReadPos(bDirect ? 1 : 4);
    QList<QByteArray> phonebook;
    QList<QByteArray> numbers;
    QList<QByteArray> phonedescr;
    Contact *contact = NULL;
    unsigned long state, time, size, nEntries;
    unsigned i;
    unsigned nActive;
    switch (type){
    case 0:
    case 1:
        b.unpack(time);
        b.unpack(size);
        b.incReadPos(4);
        b.unpack(nEntries);
        if (data)
            log(L_DEBUG, "Plugin info reply %lu %lu (%lu %lu) %lu %lu (%u)",
                data->Uin.toULong(), time, data->PluginInfoTime.toULong(),
                data->PluginStatusTime.toULong(), size, nEntries, plugin_type);
        switch (plugin_type){
        case PLUGIN_RANDOMxCHAT:{
                QByteArray name, topic, homepage;

                b.incReadPos(-12);
                b.unpackStr(name);
                b.unpackStr(topic);
                unsigned short age;
                char gender;
                unsigned short country;
                unsigned short language;
                b.unpack(age);
                b.unpack(gender);
                b.unpack(country);
                b.unpack(language);
                b.unpackStr(homepage);
                ICQUserData data;
                load_data(static_cast<ICQProtocol*>(protocol())->icqUserData, &data, NULL);
                data.Uin.asULong() = uin;
                data.Alias.str() = QString::fromUtf8(name);
                data.About.str() = QString::fromUtf8(topic);
                data.Age.asULong() = age;
                data.Gender.asULong() = gender;
                data.Country.asULong() = country;
                data.Language.asULong() = language;
                data.Homepage.str() = QString::fromUtf8(homepage);
// currently unhandled
//                Event e(EventRandomChatInfo, &data);
//                e.process();
                free_data(static_cast<ICQProtocol*>(protocol())->icqUserData, &data);
                break;
            }
        case PLUGIN_QUERYxSTATUS:
            if (data == NULL)
                break;
            if (!bDirect){
                b.incReadPos(5);
                b.unpack(nEntries);
            }
            log(L_DEBUG, "Status info answer %lu", nEntries);
        case PLUGIN_QUERYxINFO:
            if (data == NULL)
                break;
            if (nEntries > 0x80){
                log(L_DEBUG, "Bad entries value %lX", nEntries);
                break;
            }
            for (i = 0; i < nEntries; i++){
                plugin p;
                b.unpack((char*)p, sizeof(p));
                b.incReadPos(4);
                QByteArray name, descr;
                b.unpackStr32(name);
                b.unpackStr32(descr);
                b.incReadPos(4);
                unsigned plugin_index;
                for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
                    if (memcmp(p, plugins[plugin_index], sizeof(p)) == 0)
                        break;
                if (plugin_index >= PLUGIN_NULL){
                    log(L_DEBUG, "Unknown plugin sign %s %s", name.data(), descr.data());
                    continue;
                }
                log(L_DEBUG, "Plugin %u %s %s", plugin_index, name.data(), descr.data());
                switch (plugin_index){
                case PLUGIN_PHONEBOOK:
                case PLUGIN_FOLLOWME:
                    if (plugin_type == PLUGIN_QUERYxINFO){
                        addPluginInfoRequest(uin, PLUGIN_PHONEBOOK);
                    }else{
                        addPluginInfoRequest(uin, PLUGIN_FOLLOWME);
                    }
                    break;
                case PLUGIN_PICTURE:
                    if (plugin_type == PLUGIN_QUERYxINFO) {
                        // when buddyID -> new avatar support, no need to ask for old picture plugin
                        if(data->buddyID.toULong() == 0 || data->buddyHash.toBinary().size() != 16) {
                            data->buddyID.asULong() = 0;
                            addPluginInfoRequest(uin, plugin_index);
                        }
                    }
                    break;
                case PLUGIN_FILESERVER:
                case PLUGIN_ICQPHONE:
                    if (plugin_type == PLUGIN_QUERYxSTATUS)
                        addPluginInfoRequest(uin, plugin_index);
                    break;
                }
            }
            if (plugin_type == PLUGIN_QUERYxINFO){
                data->PluginInfoFetchTime = data->PluginInfoTime;
            }else{
                data->PluginStatusFetchTime = data->PluginStatusTime;
            }
            break;
        case PLUGIN_PICTURE:
            if (data){
                b.incReadPos(-4);
                QByteArray pict;
                QByteArray ba;
                b.unpackStr32(pict);
                b.unpackStr32(ba);
                QImage img;
                QString fName = pictureFile(data);
                QFile f(fName);
                if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                    f.write(ba.data());
                    f.close();
                    img.load(fName);
                }else{
                    log(L_ERROR, "Can't create %s", qPrintable(fName));
                }
                data->PictureWidth.asULong()  = img.width();
                data->PictureHeight.asULong() = img.height();
            }
            break;
        case PLUGIN_PHONEBOOK:
            if (data){
                QString phones;
                nActive = (unsigned)(-1);
                if (nEntries > 0x80){
                    log(L_DEBUG, "Bad entries value %lX", nEntries);
                    break;
                }
                for (i = 0; i < nEntries; i++){
                    QByteArray descr, area, phone, ext, country;
                    unsigned long active;
                    b.unpackStr32(descr);
                    b.unpackStr32(area);
                    b.unpackStr32(phone);
                    b.unpackStr32(ext);
                    b.unpackStr32(country);
                    numbers.push_back(phone);
                    QByteArray value;
                    for (const ext_info *e = getCountries(); e->szName; e++){
                        if (country == e->szName){
                            value = "+";
                            value += QByteArray::number(e->nCode);
                            break;
                        }
                    }
                    if (!area.isEmpty()){
                        if (!value.isEmpty())
                            value += ' ';
                        value += '(';
                        value += area;
                        value += ')';
                    }
                    if (!value.isEmpty())
                        value += ' ';
                    value += phone;
                    if (!ext.isEmpty()){
                        value += " - ";
                        value += ext;
                    }
                    b.unpack(active);
                    if (active)
                        nActive = i;
                    phonebook.push_back(value);
                    phonedescr.push_back(descr);
                }
                for (i = 0; i < nEntries; i++){
                    unsigned long type;
                    QByteArray phone = phonebook[i];
                    QByteArray gateway;
                    b.incReadPos(4);
                    b.unpack(type);
                    b.unpackStr32(gateway);
                    b.incReadPos(16);
                    switch (type){
                    case 1:
                    case 2:
                        type = CELLULAR;
                        break;
                    case 3:
                        type = FAX;
                        break;
                    case 4:{
                            type = PAGER;
                            phone = numbers[i];
                            const pager_provider *p;
                            for (p = getProviders(); *p->szName; p++){
                                if (gateway == p->szName){
                                    phone += '@';
                                    phone += p->szGate;
                                    phone += '[';
                                    phone += p->szName;
                                    phone += ']';
                                    break;
                                }
                            }
                            if (*p->szName == 0){
                                phone += '@';
                                phone += gateway;
                            }
                            break;
                        }
                    default:
                        type = PHONE;
                    }
                    phone += ',';
                    phone += phonedescr[i];
                    phone += ',';
                    phone += QByteArray::number((quint32)type);
                    if (i == nActive)
                        phone += ",1";
                    if (!phones.isEmpty())
                        phones += ';';
                    phones += QString::fromUtf8(phone);
                }
                data->PhoneBook.str() = phones;
                Contact *contact = NULL;
                findContact(data->Uin.toULong(), NULL, false, contact);
                if (contact){
                    setupContact(contact, data);
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                }
            }
        }
        break;
    case 2:
        if (data){
            if (bDirect)
                b.incReadPos(3);
            b.unpack(state);
            b.unpack(time);
            log(L_DEBUG, "Plugin status reply %u %lu %lu (%u)", uin, state, time, plugin_type);
            findContact(uin, NULL, false, contact);
            if (contact == NULL)
                break;
            switch (plugin_type){
            case PLUGIN_FILESERVER:
                if ((state != 0) != (data->SharedFiles.toBool() != 0)){
                    data->SharedFiles.asBool() = (state != 0);
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                }
                break;
            case PLUGIN_FOLLOWME:
                if (state != data->FollowMe.toULong()){
                    data->FollowMe.asULong() = state;
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                }
                break;
            case PLUGIN_ICQPHONE:
                if ((state != 0) != (data->ICQPhone.toULong() != 0)){
                    data->ICQPhone.asULong() = (state != 0);
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                }
                break;
            }
        }
        break;
    default:
        log(L_DEBUG, "Unknown plugin type answer %u %u (%u)", uin, type, plugin_type);
    }
}

static const char* plugin_name[] =
    {
        "Phone Book",				// PLUGIN_PHONEBOOK
        "Picture",					// PLUGIN_PICTURE
        "Shared Files Directory",	// PLUGIN_FILESERVER
        "Phone \"Follow Me\"",		// PLUGIN_FOLLOWME
        "ICQphone Status"			// PLUGIN_ICQPHONE
    };

static const char* plugin_descr[] =
    {
        "Phone Book / Phone \"Follow Me\"",		// PLUGIN_PHONEBOOK
        "Picture",								// PLUGIN_PICTURE
        "Shared Files Directory",				// PLUGIN_FILESERVER
        "Phone Book / Phone \"Follow Me\"",		// PLUGIN_FOLLOWME
        "ICQphone Status"						// PLUGIN_ICQPHONE
    };

void ICQClient::pluginAnswer(unsigned plugin_type, unsigned long uin, ICQBuffer &info)
{
    Contact *contact;
    ICQUserData *data = findContact(uin, NULL, false, contact);
    log(L_DEBUG, "Request about %u", plugin_type);
    ICQBuffer answer;
    unsigned long typeAnswer = 0;
    unsigned long nEntries = 0;
    unsigned long time = 0;
    switch (plugin_type){
    case PLUGIN_PHONEBOOK:{
            if (data && data->GrpId.toULong() && !contact->getIgnore()){
                ICQBuffer answer1;
                time = this->data.owner.PluginInfoTime.toULong();
                QString phones = getContacts()->owner()->getPhones();
                while (!phones.isEmpty()){
                    QString item = getToken(phones, ';', false);
                    unsigned long publish = 0;
                    QString phoneItem = getToken(item, '/', false);
                    if (item != "-")
                        publish = 1;
                    QString number = getToken(phoneItem, ',');
                    QString descr = getToken(phoneItem, ',');
                    unsigned long type = getToken(phoneItem, ',').toUInt();
                    unsigned long active = 0;
                    if (!phoneItem.isEmpty())
                        active = 1;
                    QString area;
                    QString phone;
                    QString ext;
                    QString country;
                    QString gateway;
                    if (type == PAGER){
                        phone = getToken(number, '@');
                        int n = number.indexOf('[');
                        if (n >= 0){
                            getToken(number, '[');
                            gateway = getToken(number, ']');
                        }else{
                            gateway = number;
                        }
                    }else{
                        int n = number.indexOf('(');
                        if (n >= 0){
                            country = getToken(number, '(');
                            area    = getToken(number, ')');
                            if (country[0] == '+')
                                country = country.mid(1);
                            unsigned code = country.toULong();
                            country = QString::null;
                            for (const ext_info *e = getCountries(); e->nCode; e++){
                                if (e->nCode == code){
                                    country = e->szName;
                                    break;
                                }
                            }
                        }
                        n = number.indexOf(" - ");
                        if (n >= 0){
                            ext = number.mid(n + 3);
                            number = number.left(n);
                        }
                        phone = number;
                    }
                    answer.packStr32(descr.toUtf8().data());
                    answer.packStr32(area.toUtf8().data());
                    answer.packStr32(phone.toUtf8().data());
                    answer.packStr32(ext.toUtf8().data());
                    answer.packStr32(country.toUtf8().data());
                    answer.pack(active);

                    unsigned long len = gateway.length() + 24;
                    unsigned long sms_available = 0;
                    switch (type){
                    case PHONE:
                        type = 0;
                        break;
                    case FAX:
                        type = 3;
                        break;
                    case CELLULAR:
                        type = 2;
                        sms_available = 1;
                        break;
                    case PAGER:
                        type = 4;
                        break;
                    }
                    answer1.pack(len);
                    answer1.pack(type);
                    answer1.packStr32(gateway.toUtf8().data());
                    answer1.pack((unsigned long)0);
                    answer1.pack(sms_available);
                    answer1.pack((unsigned long)0);
                    answer1.pack(publish);
                    nEntries++;
                }
                answer.pack(answer1.data(0), answer1.size());
                typeAnswer = 0x00000003;
                break;
            }
        }
    case PLUGIN_PICTURE:{
            time = this->data.owner.PluginInfoTime.toULong();
            typeAnswer = 0x00000001;
            QString pictFile = getPicture();
            if (!pictFile.isEmpty()){
                QFile f(pictFile);
                if (f.open(QIODevice::ReadOnly)){
                    QFileInfo fi(f);
                    pictFile = fi.fileName();
                    nEntries = pictFile.length();
                    answer.pack(pictFile.toLocal8Bit(), pictFile.length());
                    unsigned long size = f.size();
                    answer.pack(size);
                    while (size > 0){
                        char buf[2048];
                        unsigned tail = sizeof(buf);
                        if (tail > size)
                            tail = size;
                        f.read(buf, tail);
                        answer.pack(buf, tail);
                        size -= tail;
                    }
                }
            }
            break;
        }
    case PLUGIN_FOLLOWME:
        time = this->data.owner.PluginStatusTime.toULong();
        break;
    case PLUGIN_QUERYxINFO:
        time = this->data.owner.PluginInfoTime.toULong();
        typeAnswer = 0x00010002;
        if (!getPicture().isEmpty()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_PICTURE], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_PICTURE]);
            answer.packStr32(plugin_descr[PLUGIN_PICTURE]);
            answer.pack((unsigned long)0);
        }
        if (!getContacts()->owner()->getPhones().isEmpty()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_PHONEBOOK], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_PHONEBOOK]);
            answer.packStr32(plugin_descr[PLUGIN_PHONEBOOK]);
            answer.pack((unsigned long)0);
        }
        break;
    case PLUGIN_QUERYxSTATUS:
        time = this->data.owner.PluginStatusTime.toULong();
        typeAnswer = 0x00010000;
        nEntries++;
        answer.pack((char*)plugins[PLUGIN_FOLLOWME], sizeof(plugin));
        answer.pack((unsigned short)0);
        answer.pack((unsigned short)1);
        answer.packStr32(plugin_name[PLUGIN_FOLLOWME]);
        answer.packStr32(plugin_descr[PLUGIN_FOLLOWME]);
        answer.pack((unsigned long)0);
        if (this->data.owner.SharedFiles.toBool()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_FILESERVER], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_FILESERVER]);
            answer.packStr32(plugin_descr[PLUGIN_FILESERVER]);
            answer.pack((unsigned long)0);
        }
        if (this->data.owner.ICQPhone.toULong()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_ICQPHONE], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_ICQPHONE]);
            answer.packStr32(plugin_descr[PLUGIN_ICQPHONE]);
            answer.pack((unsigned long)0);
        }
        break;
    default:
        log(L_DEBUG, "Bad plugin type request %u", plugin_type);
    }
    unsigned long size = answer.size() + 8;
    info.pack((unsigned short)0);
    info.pack((unsigned short)1);
    switch (plugin_type){
    case PLUGIN_FOLLOWME:
        info.pack(this->data.owner.FollowMe.toULong());
        info.pack(time);
        info.pack((char)1);
        break;
    case PLUGIN_QUERYxSTATUS:
        info.pack((unsigned long)0);
        info.pack((unsigned long)0);
        info.pack((char)1);
    default:
        info.pack(time);
        info.pack(size);
        info.pack(typeAnswer);
        info.pack(nEntries);
        info.pack(answer.data(0), answer.size());
    }
}



