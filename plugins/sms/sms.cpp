/***************************************************************************
                          sms.cpp  -  description
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

#include "sms.h"
#include "smssetup.h"
#include "serial.h"
#include "gsm_ta.h"
#include "core.h"

#include "maininfo.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "clientmanager.h"

#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QObject>

using namespace SIM;

const unsigned COL_TYPE	= 3;

static DataDef _smsUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(6) },	// Sign
        { "", DATA_ULONG, 1, 0 },			// LastSend
        { "", DATA_UTF, 1, 0 },				// Name
        { "", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// Index
        { "", DATA_ULONG, 1, 0 },			// Type
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

Plugin *createSMSPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new SMSPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        0,
        0,
        VERSION,
        createSMSPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

unsigned SMSPlugin::SerialPacket = 0;

static Message *createPhoneCall(Buffer *cfg)
{
    return new Message(MessagePhoneCall, cfg);
}

static MessageDef defPhoneCall =
    {
        NULL,
        NULL,
        MESSAGE_INFO,
        "Phone call",
        "%n phone calls",
        createPhoneCall,
        NULL,
        NULL
    };

#if 0
i18n("Phone call", "%n phone calls", 1);
#endif

SMSPlugin::SMSPlugin(unsigned base)
        : Plugin(base)
{
    SerialPacket = registerType();
    getContacts()->addPacketType(SerialPacket, "Serial port", true);

    Command cmd;
    cmd->id			 = MessagePhoneCall;
    cmd->text		 = I18N_NOOP("Phone call");
    cmd->icon		 = "phone";
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defPhoneCall;
    EventCreateMessageType(cmd).process();

    m_protocol = new SMSProtocol(this);

    qApp->installEventFilter(this);
    setPhoneCol();
}

SMSPlugin::~SMSPlugin()
{
    removePhoneCol();
    delete m_protocol;
    getContacts()->removePacketType(SerialPacket);
    EventRemoveMessageType(MessagePhoneCall).process();
}

void SMSPlugin::setPhoneCol()
{
    QWidgetList list = QApplication::topLevelWidgets();
    Q_FOREACH(QWidget *w, list) {
        QList<MainInfo*> l = qFindChildren<MainInfo *>(w);
        Q_FOREACH(MainInfo *obj,l) {
            setPhoneCol(obj);
        }
    }
}

void SMSPlugin::removePhoneCol()
{
    QWidgetList list = QApplication::topLevelWidgets();
    Q_FOREACH(QWidget *w, list) {
        QList<MainInfo*> l = qFindChildren<MainInfo *>(w);
        Q_FOREACH(MainInfo *obj,l) {
            removePhoneCol(obj);
        }
    }
}

void SMSPlugin::setPhoneCol(MainInfo *w)
{
    w->lstPhones->addColumn(" "/*, 16*/);
}

void SMSPlugin::removePhoneCol(MainInfo *w)
{
    //w->lstPhones->removeColumn(COL_TYPE);
}

bool SMSPlugin::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::ChildAdded){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        if (ce->child()->inherits("MainInfo"))
            setPhoneCol(static_cast<MainInfo*>(ce->child()));
    }
    return QObject::eventFilter(obj, e);
}

SMSProtocol::SMSProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

SMSProtocol::~SMSProtocol()
{
}

ClientPtr SMSProtocol::createClient(Buffer *cfg)
{
    ClientPtr sms = ClientPtr(new SMSClient(this, cfg));
	getClientManager()->addClient(sms);
    return sms;
}

static CommandDef sms_descr =
    CommandDef (
        0,
        I18N_NOOP("SMS"),
        "SMS",
        QString::null,
        QString::null,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_NOPROXY | PROTOCOL_TEMP_DATA | PROTOCOL_NODATA | PROTOCOL_NO_AUTH,
        NULL,
        QString::null
    );

const CommandDef *SMSProtocol::description()
{
    return &sms_descr;
}

static CommandDef sms_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "SMS_online",
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
            "SMS_offline",
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

const CommandDef *SMSProtocol::statusList()
{
    return sms_status_list;
}

const DataDef *SMSProtocol::userDataDef()
{
    return _smsUserData;
}

static DataDef smsClientData[] =
    {
#ifdef WIN32
        { "Port", DATA_STRING, 1, "COM1" },
#else
        { "Port", DATA_STRING, 1, "cuaa0" },
#endif
        { "BaudRate", DATA_ULONG, 1, DATA(19200) },
        { "XonXoff", DATA_BOOL, 1, 0 },
        { "", DATA_ULONG, 1, 0 },		// Charge
        { "", DATA_BOOL, 1, 0 },		// Charging
        { "", DATA_ULONG, 1, 0 },		// Quality
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

SMSClient::SMSClient(Protocol *protocol, Buffer *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(smsClientData, &data, cfg);
    m_ta = NULL;
    m_call = NULL;
    m_callTimer = new QTimer(this);
    connect(m_callTimer, SIGNAL(timeout()), this, SLOT(callTimeout()));
}

SMSClient::~SMSClient()
{
    free_data(smsClientData, &data);
}

QByteArray SMSClient::getConfig()
{
    QByteArray cfg = TCPClient::getConfig();
    QByteArray my_cfg = save_data(smsClientData, &data);
    if (!my_cfg.isEmpty()){
        if (!cfg.isEmpty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

QByteArray SMSClient::model() const
{
    if (getState() == Connected)
        return m_ta->model();
    return QByteArray();
}

QByteArray SMSClient::oper() const
{
    if (getState() == Connected)
        return m_ta->oper();
    return QByteArray();
}

QString SMSClient::name()
{
    QString res = "SMS.";
    if (getState() == Connected){
        res += model();
        res += " ";
        res += oper();
    }else{
        res += getDevice();
    }
    return res;
}

QString SMSClient::dataName(void*)
{
    return QString();
}

bool SMSClient::isMyData(clientData*&, Contact*&)
{
    return false;
}

bool SMSClient::createData(clientData*&, Contact*)
{
    return false;
}

void SMSClient::setupContact(Contact*, void*)
{
}

bool SMSClient::send(Message*, void*)
{
    return false;
}

bool SMSClient::canSend(unsigned type, void *data)
{
    if ((data == NULL) && (type == MessageSMS))
        return true;
    return false;
}

QWidget	*SMSClient::setupWnd()
{
    return new SMSSetup(NULL, this);
}

const unsigned MAIN_INFO = 1;

static CommandDef cfgSmsWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "SMS",
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

CommandDef *SMSClient::configWindows()
{
    QString title = name();
    int n = title.indexOf('.');
    if (n > 0)
        title = title.left(n) + ' ' + title.mid(n + 1);
    cfgSmsWnd[0].text_wrk = title;
    return cfgSmsWnd;
}

QWidget *SMSClient::configWindow(QWidget *parent, unsigned id)
{
    if (id == MAIN_INFO)
        return new SMSSetup(parent, this);
    return NULL;
}

void SMSClient::packet_ready()
{
}

void SMSClient::disconnected()
{
    setStatus(STATUS_OFFLINE);
}

void SMSClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE){
        Contact *contact;
        ContactList::ContactIterator it;
        while ((contact = ++it) != NULL)
            contact->clientData.freeClientData(this);
        if (m_ta){
            delete m_ta;
            m_ta = NULL;
        }
        return;
    }
    if (m_ta)
        return;
    m_ta = new GsmTA(this);
    connect(m_ta, SIGNAL(init_done()), this, SLOT(init()));
    connect(m_ta, SIGNAL(error()), this, SLOT(error()));
    connect(m_ta, SIGNAL(phonebookEntry(int, int, const QString&, const QString&)), this, SLOT(phonebookEntry(int, int, const QString&, const QString&)));
    connect(m_ta, SIGNAL(charge(bool, unsigned)), this, SLOT(charge(bool, unsigned)));
    connect(m_ta, SIGNAL(quality(unsigned)), this, SLOT(quality(unsigned)));
    connect(m_ta, SIGNAL(phoneCall(const QString&)), this, SLOT(phoneCall(const QString&)));
    if (!m_ta->open(qPrintable(getDevice()), getBaudRate(), getXonXoff())){
        error_state("Can't open port", 0);
        return;
    }
}

void SMSClient::phonebookEntry(int index, int type, const QString &phone, const QString &name)
{
    bool bNew = false;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        smsUserData *data;
        ClientDataIterator itd(contact->clientData);
        while ((data = tosmsUserData(++itd)) != NULL){
            if (name == data->Name.str())
                break;
        }
        if (data)
            break;
    }
    if (contact == NULL){
        contact = getContacts()->contactByPhone(phone);
        if (contact->getFlags() & CONTACT_TEMPORARY){
            bNew = true;
            contact->setFlags(contact->getFlags() & ~CONTACT_TEMPORARY);
            contact->setName(name);
        }
    }
    QString phones = contact->getPhones();
    bool bFound = false;
    while (!phones.isEmpty()){
        QString item = getToken(phones, ';', false);
        QString number = getToken(item, ',');
        if (number == phone){
            bFound = true;
            break;
        }
    }
    if (!bFound){
        phones = contact->getPhones();
        if (!phones.isEmpty())
            phones += ";";
        contact->setPhones(phones + phone + ",,2/-");
    }
    smsUserData *data = tosmsUserData((SIM::clientData*)contact->clientData.createData(this)); // FIXME unsafe type conversion
    data->Phone.str() = phone;
    data->Name.str()  = name;
    data->Index.asULong() = index;
    data->Type.asULong()  = type;
    if (bNew){
        EventContact e(contact, EventContact::eChanged);
        e.process();
    }
}

QString SMSClient::getServer() const
{
    return QString::null;
}

unsigned short SMSClient::getPort() const
{
    return 0;
}

void SMSClient::socketConnect()
{
    connect_ready();
    setStatus(STATUS_ONLINE);
}

void SMSClient::error()
{
    QTimer::singleShot(0, this, SLOT(ta_error()));
}

void SMSClient::ta_error()
{
    error_state(I18N_NOOP("Port error"), 0);
}

void SMSClient::init()
{
    m_status = STATUS_ONLINE;
    setState(Connected);
    m_ta->getPhoneBook();
}

void SMSClient::charge(bool bCharge, unsigned capacity)
{
    bool bChange = false;
    if (bCharge != getCharging()){
        bChange = true;
        setCharging(bCharge);
    }
    if (capacity != getCharge()){
        bChange = true;
        setCharge(capacity);
    }
    if (bChange){
        EventClientChanged(this).process();
    }
}

void SMSClient::quality(unsigned quality)
{
    if (quality != getQuality()){
        setQuality(quality);
        EventClientChanged(this).process();
    }
}

void SMSClient::phoneCall(const QString &number)
{
    if (m_call && (number == m_callNumber))
        return;
    if (m_call){
        m_callTimer->stop();
        EventMessageDeleted(m_call).process();
        delete m_call;
        m_call = NULL;
    }
    m_callNumber = number;
    m_call = new Message(MessagePhoneCall);
    if (!number.isEmpty()){
        bool bNew = false;
        Contact *contact = getContacts()->contactByPhone(number);
        if (contact->getFlags() & CONTACT_TEMPORARY){
            bNew = true;
            contact->setFlags(contact->getFlags() & ~CONTACT_TEMPORARY);
            contact->setName(number);
        }
        QString phones = contact->getPhones();
        bool bFound = false;
        while (!phones.isEmpty()){
            QString item = getToken(phones, ';', false);
            QString phone = getToken(item, ',');
            if (number == phone){
                bFound = true;
                break;
            }
        }
        if (!bFound){
            phones = contact->getPhones();
            if (!phones.isEmpty())
                phones += ";";
            contact->setPhones(phones + number + ",,2/-");
        }
        if (bNew){
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        m_call->setContact(contact->id());
    }
    m_call->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
    EventMessageReceived e(m_call);
    if (e.process()){
        m_call = NULL;
        return;
    }
    m_bCall = false;
    m_callTimer->start(12000);
}

void SMSClient::callTimeout()
{
    if (m_bCall){
        m_bCall = false;
        return;
    }
    if (m_call == NULL)
        return;
    EventMessageDeleted(m_call).process();
    delete m_call;
    m_call = NULL;
    m_callTimer->stop();
    m_callNumber = "";
}

smsUserData* SMSClient::tosmsUserData(SIM::clientData * data)
{
   // This function is used to more safely preform type conversion from SIM::clientData* into smsUserData*
   // It will at least warn if the content of the structure is not smsUserData
   // Brave wariors may uncomment abort() function call to know for sure about wrong conversion ;-)
   if (! data) return NULL;
   if (data->Sign.asULong() != SMS_SIGN)
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
        "ATTENTION!! Unsafly converting %s user data into SMS_SIGN",
         qPrintable(Sign));
//      abort();
   }
   return (smsUserData*) data;
}

QWidget *SMSClient::searchWindow(QWidget*)
{
    return NULL;
}

