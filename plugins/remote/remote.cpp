/***************************************************************************
                          remote.cpp  -  description
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

#include <time.h>

#include <algorithm>
#include <iostream>

#include <QApplication>
#include <QFile>
#include <QPixmap>
#include <QRegExp>
#include <QTimer>
#include <QThread>
#include <QWidget>
#include <QByteArray>

#include "icons.h"
#include "log.h"

#include "profile.h"
#include "profilemanager.h"

#include "socket/clientsocket.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"
#include "remote.h"
#include "remotecfg.h"
#include "core.h"

using namespace std;
using namespace SIM;

Plugin *createRemotePlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new RemotePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Remote control"),
        I18N_NOOP("Plugin provides remote control"),
        VERSION,
        createRemotePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

//static DataDef remoteData[] =
//    {
//#ifdef WIN32
//        { "Path", DATA_STRING, 1, "auto:" },
//        { "EnableMenu", DATA_BOOL, 1, DATA(1) },
//#else
//        { "Path", DATA_STRING, 1, "/tmp/sim.%user%" },
//#endif
//        { NULL, DATA_UNKNOWN, 0, 0 }
//    };

#ifdef WIN32

#include <windows.h>

static RemotePlugin *remote = NULL;

class IPC : public QThread
{
public:
    IPC();
    ~IPC();
    QString prefix();
    void    process();
protected:
    unsigned *s;
    Qt::HANDLE	hMem;
    Qt::HANDLE	hMutex;
    Qt::HANDLE	hEventIn;
    Qt::HANDLE	hEventOut;
    bool    bExit;
    virtual void run();
    friend class IPCLock;
};

class IPCLock
{
public:
    IPCLock(IPC *ipc);
    ~IPCLock();
protected:
    IPC *m_ipc;
};

IPC::IPC()
{
    s = NULL;
    QString name = prefix() + "mem";
    hMem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, N_SLOTS * sizeof(unsigned), name.toLatin1());
    if (hMem)
        s = (unsigned*)MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (s)
        memset(s, 0, N_SLOTS * sizeof(unsigned));
    name = prefix() + "mutex";
    hMutex = CreateMutexA(NULL, FALSE, name.toLatin1());
    name = prefix() + "in";
    hEventIn = CreateEventA(NULL, TRUE, FALSE, name.toLatin1());
    name = prefix() + "out";
    hEventOut = CreateEventA(NULL, TRUE, FALSE, name.toLatin1());
    bExit = false;
    start();
}

IPC::~IPC()
{
    bExit = true;
    SetEvent(hEventIn);
    if (s)
        UnmapViewOfFile(s);
    if (hMem)
        CloseHandle(hMem);
    if (hMutex)
        CloseHandle(hMutex);
    if (hEventIn)
        CloseHandle(hEventIn);
    if (hEventOut)
        CloseHandle(hEventOut);
    wait(2000);
}

void IPC::run()
{
    for (;;){
        ResetEvent(hEventIn);
        WaitForSingleObject(hEventIn, INFINITE);
        if (bExit)
            break;
        QTimer::singleShot(0, remote, SLOT(command()));
    }
}

void IPC::process()
{
    IPCLock(this);
    for (unsigned i = 0; i < N_SLOTS; i++){
        if (s[i] != SLOT_IN)
            continue;
        QString in;
        QString out;
        QString name = prefix() + QString::number(i);
        Qt::HANDLE hMem = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.toLatin1());
        if (hMem == NULL){
            s[i] = SLOT_NONE;
            PulseEvent(hEventOut);
            continue;
        }
        unsigned short *mem = (unsigned short*)MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (mem == NULL){
            log(L_WARN, "Map error");
            s[i] = SLOT_NONE;
            PulseEvent(hEventOut);
            continue;
        }
        unsigned short *p;
        for (p = mem; *p; p++)
            in += QChar(*p);

        bool bError = false;
        bool bRes = remote->command(in, out, bError);
        p = mem;
        unsigned size = 0;
        if (!bError){
            if (bRes){
                *(p++) = QChar('>').unicode();
            }else{
                *(p++) = QChar('?').unicode();
            }
            size = out.length();
            if (size > 0x3F00)
                size = 0x3F00;
            memcpy(p, out.unicode(), size * sizeof(unsigned short));
            size++;
        }
        p[size] = 0;
        UnmapViewOfFile(mem);
        CloseHandle(hMem);
        s[i] = SLOT_OUT;
        PulseEvent(hEventOut);
    }
}

#ifndef SM_REMOTECONTROL
#define SM_REMOTECONTROL	0x2001
#endif
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION	0x1000
#endif

QString IPC::prefix()
{
    QString res;
    if (GetSystemMetrics(SM_REMOTECONTROL) || GetSystemMetrics(SM_REMOTESESSION))
        res = "Global/";
    res += SIM_SHARED;
    return res;
}

IPCLock::IPCLock(IPC *ipc)
{
    m_ipc = ipc;
    WaitForSingleObject(m_ipc->hMutex, INFINITE);
}

IPCLock::~IPCLock()
{
    ReleaseMutex(m_ipc->hMutex);
}

#endif

RemotePlugin::RemotePlugin()
    : Plugin(NULL)
{
}

RemotePlugin::RemotePlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
{
    m_propertyHub = SIM::PropertyHub::create("remote");
    bind();
#ifdef WIN32
    remote = this;
    ipc = new IPC;
#endif
}

RemotePlugin::~RemotePlugin()
{
#ifdef WIN32
    delete ipc;
#endif
    while (!m_sockets.empty())
        delete m_sockets.front();
}

QByteArray RemotePlugin::getConfig()
{
    return QByteArray(); //Fixmeee
}

QWidget *RemotePlugin::createConfigWindow(QWidget *parent)
{
    return new RemoteConfig(parent, this);
}

bool RemotePlugin::processEvent(Event* e)
{
	if(e->type() == eEventPluginLoadConfig)
	{
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("remote");
        if(!hub.isNull())
            setPropertyHub(hub);
	}
    return false;
}

static char TCP[] = "tcp:";

void RemotePlugin::bind()
{
    QString path = value("Path").toString();
    if (path.startsWith(TCP)){
        unsigned short port = path.mid(strlen(TCP)).toUShort();
        ServerSocketNotify::bind(port, port, NULL);
// ToDo: Restore this
//#ifndef WIN32
//    }else{
//        ServerSocketNotify::bind(path.toLatin1());
//#endif
    }
}

bool RemotePlugin::accept(Socket *s, unsigned long)
{
    log(L_DEBUG, "Accept remote control");
    new ControlSocket(this, s);
    return false;
}

void RemotePlugin::bind_ready(unsigned short)
{
}

bool RemotePlugin::error(const QString &err)
{
    if (!err.isEmpty())
        log(L_DEBUG, "Remote: %s", qPrintable(err));
    return true;
}

void RemotePlugin::command()
{
#ifdef WIN32
    ipc->process();
#endif
}

const unsigned CMD_STATUS		= 0;
const unsigned CMD_INVISIBLE	= 1;
const unsigned CMD_MAINWND		= 2;
const unsigned CMD_SEARCHWND	= 3;
const unsigned CMD_QUIT			= 4;
const unsigned CMD_CLOSE		= 5;
const unsigned CMD_HELP			= 6;
const unsigned CMD_ADD			= 7;
const unsigned CMD_DELETE		= 8;
const unsigned CMD_OPEN			= 9;
const unsigned CMD_FILE			= 10;
const unsigned CMD_CONTACTS		= 11;
const unsigned CMD_SENDFILE		= 12;
const unsigned CMD_GROUP		= 13;
const unsigned CMD_SHOW			= 14;
const unsigned CMD_SMS			= 15;
#ifdef WIN32
const unsigned CMD_ICON			= 16;
#endif

struct cmdDef
{
    const char *cmd;
    const char *shortDescr;
    const char *longDescr;
    unsigned minArgs;
    unsigned maxArgs;
};

static cmdDef cmds[] =
    {
        { "STATUS", "set status", "STATUS [status]", 0, 1 },
        { "INVISIBLE", "set invisible mode", "INVISIBLE [on|off]", 0, 1 },
        { "MAINWINDOW", "show/hide main window", "MAINWINDOW [on|off|toggle]", 0, 1 },
        { "SEARCHWINDOW", "show/hide search window", "SEARCHWINDOW [on|off]", 0, 1 },
        { "QUIT", "quit SIM", "QUIT", 0, 0 },
        { "CLOSE", "close session", "CLOSE", 0, 0 },
        { "HELP", "command help information", "HELP [<cmd>]", 0, 1 },
        { "ADD", "add contact", "ADD <protocol> <address> [<nick>] [<group>]", 2, 4 },
        { "DELETE", "delete contact", "DELETE [<address> | <nick>]", 1, 1 },
        { "OPEN", "open contact", "OPEN <protocol> <address> [<nick>] [<group>]", 2, 4 },
        { "FILE", "process UIN file", "FILE <file>", 1, 1 },
        { "CONTACTS", "print contact list", "CONTACTS [<message_type>]", 0, 1 },
        { "SENDFILE", "send file", "SENDFILE <file> <contact>", 2, 2 },
        { "GROUP", "get group name", "GROUP id", 1, 1 },
        { "SHOW", "open unread message", "SHOW", 0, 0 },
        { "SMS", "send SMS", "SMS <phone> <message>", 2, 2 },
#ifdef WIN32
        { "ICON", "get used icon", "ICON name", 1, 1 },
#endif
        { NULL, NULL, NULL, 0, 0 }
    };

#if 0
{ "MESSAGE", "send message", "MESSAGE <UIN|Name> <message>", 2, 2 },
{ "SMS", "send SMS", "SMS <phone> <message>", 2, 2 },
{ "DOCK", "show/hide dock", "DOCK [on|off]", 0, 1 },
{ "NOTIFY", "set notify mode", "NOTIFY [on|off]", 0, 1 },
{ "ICON", "get icon in xpm format", "ICON nIcon", 1, 1 },
{ "POPUP", "show popup", "POPUP x y", 2, 2 },
#endif

static bool isOn(const QString &s)
{
    return (s == "1") || (s == "on") || (s == "ON");
}

static bool cmpStatus(const QString &s1, const QString &s2)
{
    QString ss1 = s1;
    QString ss2 = s2;
    ss1 = ss1.remove('&');
    ss2 = ss2.remove('&');
    return ss1.toLower() == ss2.toLower();
}

static QWidget *findWidget(const char *className)
{
    QWidgetList list = QApplication::topLevelWidgets();
	QWidget* w;
    foreach (w,list)
	{
		if (w->inherits(className))
			return w;
	}
	return NULL;
}

struct ContactInfo
{
    QString		name;
    unsigned	id;
    unsigned	group;
    QString		key;
    QString		icon;
};

static bool cmp_info(const ContactInfo &p1, const ContactInfo &p2)
{
    return p1.key < p2.key;
}

#ifdef WIN32

void packData(QString &out, void *data, unsigned size)
{
    unsigned char *p = (unsigned char*)data;
    for (unsigned i = 0; i < size; i++, p++){
        char b[3];
        sprintf(b, "%02X", *p);
        out += b;
    }
}

void packBitmap(QString &out, HBITMAP hBmp)
{
    BITMAP		bmp;
    BITMAPINFO	*pbmi;
    WORD    cClrBits;
    if (!GetObjectA(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        return;

    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    unsigned size = sizeof(BITMAPINFOHEADER);
    if (cClrBits != 24)
        size += sizeof(RGBQUAD) * (1<< cClrBits);
    pbmi = (BITMAPINFO*)malloc(size);

    pbmi->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth  = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits);

    pbmi->bmiHeader.biCompression = BI_RGB;
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight;

    HDC hDC = CreateCompatibleDC(NULL);
    void *bits = malloc(pbmi->bmiHeader.biSizeImage);
    GetDIBits(hDC, hBmp, 0, (WORD) pbmi->bmiHeader.biHeight, bits, pbmi, DIB_RGB_COLORS);

    packData(out, pbmi, size);
    out += "\n";
    packData(out, bits, pbmi->bmiHeader.biSizeImage);

    DeleteDC(hDC);
    free(bits);
    free(pbmi);
}

class IconWidget : public QWidget
{
public:
    IconWidget(const QPixmap &p);
    HICON icon();
};

IconWidget::IconWidget(const QPixmap &p)
{
    setWindowIcon(p);
}

HICON IconWidget::icon()
{
	HWND hWnd = (HWND)effectiveWinId();
	if(NULL == hWnd)
		return NULL;
    return (HICON)::SendMessage( hWnd, WM_GETICON, ICON_SMALL, 0 );
}

#endif

bool RemotePlugin::command(const QString &in, QString &out, bool &bError)
{
    QString cmd;
    vector<QString> args;
    int i = 0;
    for (; i < (int)(in.length()); i++)
        if (in[i] != ' ')
            break;
    for (; i < (int)(in.length()); i++){
        if (in[i] == ' ')
            break;
        cmd += in[i].toUpper();
    }
    for (; i < (int)(in.length()); ){
        for (; i < (int)(in.length()); i++)
            if (in[i] != ' ')
                break;
        if (i >= (int)(in.length()))
            break;
        QString arg;
        if ((in[i] == '\'') || (in[i] == '\"')){
            QChar c = in[i];
            for (i++; i < (int)(in.length()); i++){
                if (in[i] == c){
                    i++;
                    break;
                }
                arg += in[i];
            }
        }else{
            for (; i < (int)(in.length()); i++){
                if (in[i] == '\\'){
                    i++;
                    if (i >= (int)(in.length()))
                        break;
                    arg += in[i];
                    continue;
                }
                if (in[i] == ' ')
                    break;
                arg += in[i];
            }
        }
        args.push_back(arg);
    }
    unsigned nCmd = 0;
    const cmdDef *c;
	for (c = cmds; c->cmd; c++, nCmd++)
		if (QString(cmd) == QString(c->cmd))
            break;
	
    if (c->cmd == NULL){
        out = "Unknown command ";
        out += cmd;
        return false;
    }
    if ((args.size() < c->minArgs) || (args.size() > c->maxArgs)){
        out = "Bad arguments number. Try help ";
        out += cmd;
        return false;
    }
    QWidget *w;
    unsigned n;
    CorePlugin *core = GET_CorePlugin();
    switch (nCmd){
#ifdef WIN32
    case CMD_ICON:{
            IconWidget w(Pict(args[0].toUtf8()));
            HICON icon = w.icon();
            ICONINFO info;
            if (!GetIconInfo(icon, &info))
                return false;
            packBitmap(out, info.hbmMask);
            out += "\n";
            packBitmap(out, info.hbmColor);
            return true;
        }
#endif
    case CMD_SENDFILE:{   //fix me, concepted only for ICQ-Proto
            FileMessage *msg = new FileMessage;
            msg->setContact(args[1].toUInt());   //anyhow
            msg->setFile(args[0]);               //this should
            EventOpenMessage(msg).process();     //be tested, or be rewritten
            delete msg;
            return true;
        }
    case CMD_GROUP:{
            Group *grp = getContacts()->group(args[0].toUInt());
            if (grp == NULL)
                return false;
            if (grp->id() == 0){
                out += i18n("Not in list");
            }else{
                out += grp->getName();
            }
            return true;
        }
    case CMD_CONTACTS:{
#ifdef WIN32
            if (value("EnableMenu").toBool()){
#endif
                unsigned type = 0;
                if (args.size())
                    type = args[0].toUInt();
                ContactList::ContactIterator it;
                Contact *contact;
                vector<ContactInfo> contacts;
                list<unsigned> groups;
                while ((contact = ++it) != NULL){
                    if ((contact->getFlags() & CONTACT_TEMPORARY) || contact->getIgnore())
                        continue;
                    if (type){
                        Command cmd;
                        cmd->id      = type;
                        cmd->menu_id = MenuMessage;
                        cmd->param   = (void*)(contact->id());
                        if (!EventCheckCommandState(cmd).process())
                            continue;
                    }
                    unsigned style = 0;
                    QString statusIcon;
                    unsigned status = contact->contactInfo(style, statusIcon);
                    if ((status == STATUS_OFFLINE) && core->value("ShowOnLine").toBool())
                        continue;
                    unsigned mode = core->value("SortMode").toUInt();
                    ContactInfo info;
                    QString active;
                    active.sprintf("%08lX", (long unsigned int)(0xFFFFFFFF - contact->getLastActive()));
                    if (core->value("GroupMode").toUInt()){
                        unsigned index = 0xFFFFFFFF;
                        if (contact->getGroup()){
                            Group *grp = getContacts()->group(contact->getGroup());
                            if (grp)
                                index = getContacts()->groupIndex(grp->id());
                        }
                        QString grpIndex;
                        grpIndex.sprintf("%08X", index);
                        info.key += grpIndex;
                    }
                    for (;;){
                        if ((mode & 0xFF) == 0)
                            break;
                        switch (mode & 0xFF){
                        case SORT_STATUS:
                            info.key += QString::number(9 - status);
                            break;
                        case SORT_ACTIVE:
                            info.key += active.toLower();
                            break;
                        case SORT_NAME:
                            info.key += contact->getName().toLower();
                            break;
                        }
                        mode = mode >> 8;
                    }
                    info.name  = contact->getName();
                    info.id    = contact->id();
                    info.icon  = statusIcon;
                    info.group = contact->getGroup();
                    if (core->value("GroupMode").toUInt()){
                        info.group = contact->getGroup();
                        list<unsigned>::iterator it;
                        for (it = groups.begin(); it != groups.end(); ++it)
                            if ((*it) == (unsigned)contact->getGroup())
                                break;
                        groups.push_back(contact->getGroup());
                    }
                    contacts.push_back(info);
                }
                sort(contacts.begin(), contacts.end(), cmp_info);
                out += QString::number(contacts.size());
                out += " ";
                out += QString::number(groups.size());
                out += "\n";
                for (vector<ContactInfo>::iterator itl = contacts.begin(); itl != contacts.end(); ++itl){
                    out += "\n";
                    out += QString::number((*itl).id);
                    out += " ";
                    out += QString::number((*itl).group);
                    out += " ";
                    out += (*itl).icon;
                    out += " ";
                    out += (*itl).name;
                }
#ifdef WIN32
            }
#endif
            return true;
        }
    case CMD_FILE:{
            QFile f(args[0]);
            if (!f.open(QIODevice::ReadOnly)){
                out = "Can't open ";
                out += args[0];
                return false;
            }
            bool bOpen = false;
            unsigned uin = 0;
            Buffer sf;
            sf = f.readAll();
            while (sf.readPos() < (unsigned)sf.size()){
                QByteArray line;
                sf.scan("\n", line);
                if (!line.isEmpty() && (line[(int)line.length() - 1] == '\r'))
                    line = line.left(line.length() - 1);
                if (line == "[ICQ Message User]")
                    bOpen = true;
                if (line.left(4) == "UIN=")
                    uin = line.mid(4).toUInt();
            }
            if (uin == 0){
                out = "Bad file ";
                out += args[0];
                return false;
            }
            EventAddContact::AddContact ac;
            ac.proto = "ICQ";
            ac.addr  = QString::number(uin);
            ac.nick  = QString::null;
            ac.group = 0;
            EventAddContact e(&ac);
            e.process();
            Contact *contact = e.contact();
            if (contact == NULL){
                out = "Can't add user";
                return false;
            }
            if (bOpen){
                Message *m = new Message(MessageGeneric);
                m->setContact(contact->id());
                EventOpenMessage(m).process();
                delete m;
            }
            return true;
        }
    case CMD_STATUS:
        if (args.size()){
            unsigned status = STATUS_UNKNOWN;
            for (n = 0; n < getContacts()->nClients(); n++){
                Client *client = getContacts()->getClient(n);
                for (const CommandDef *d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
                    if (cmpStatus(d->text, args[0])){
                        status = d->id;
                        break;
                    }
                }
                if (status != STATUS_UNKNOWN)
                    break;
            }
            if (status == STATUS_UNKNOWN){
                out = "Unknown status ";
                out += args[0];
                return false;
            }
            for (n = 0; n < getContacts()->nClients(); n++){
                Client *client = getContacts()->getClient(n);
                if (client->getCommonStatus())
                    client->setStatus(status, true);
            }
            if (core->getManualStatus() == status)
                return true;
            //core->data.ManualStatus.asULong()  = status;
            core->setValue("ManualStatus", (unsigned int)status);
            core->setValue("StatusTime", (unsigned int)time(NULL));
            EventClientStatus().process();
            return true;
        }
        for (n = 0; n < getContacts()->nClients(); n++){
            Client *client = getContacts()->getClient(n);
            if (client->getCommonStatus()){
                const CommandDef *d = NULL;
                for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
                    if (d->id == core->getManualStatus())
                        break;
                }
                if (d){
                    out = "STATUS ";
                    QString tmp = d->text;
                    tmp = tmp.remove('&');
                    out += tmp;
                    break;
                }
            }
        }
        return true;

    case CMD_INVISIBLE:
        if (args.size()){
            bool bInvisible = isOn(args[0]);
            if (core->value("Invisible").toBool() != bInvisible){
                core->setValue("Invisible", bInvisible);
                for (unsigned i = 0; i < getContacts()->nClients(); i++)
                    getContacts()->getClient(i)->setInvisible(bInvisible);
            }
        }else{
            out  = "INVISIBLE ";
            out += core->value("Invisible").toBool() ? "on" : "off";
        }
        return true;
    case CMD_MAINWND:
        w = findWidget("MainWindow");
        if (args.size()){
            if (args[0].toLower() == "toggle"){
                if (w){
                    if (w->isVisible()){
                        w->hide();
                    }else{
                        w->show();
                    }
                }
            }else if (isOn(args[0])){
                if (w)
                    raiseWindow(w);
            }else{
                if (w)
                    w->hide();
            }
        }else{
            out += "MAINWINDOW ";
            out += (w ? "on" : "off");
        }
        return true;
    case CMD_SEARCHWND:
        w = findWidget("SearchDialog");
        if (args.size()){
            if (isOn(args[0])){
                if (w){
                    raiseWindow(w);
                }else{
                    Command cc;
                    cc->id = CmdSearch;
                    EventCommandExec(cc).process();
                }
            }else{
                if (w)
                    w->close();
            }
        }else{
            out = "SEARCHWINDOW ";
            out += (w ? "on" : "off");
        }
        return true;
    case CMD_QUIT:{
            Command cc;
            cc->id = CmdQuit;
            EventCommandExec(cc).process();
            break;
        }
    case CMD_CLOSE:
        bError = true;
        return false;
    case CMD_OPEN:
    case CMD_ADD:{
            Group *grp = NULL;
            bool  bNewGrp = false;
            if (args.size() > 3){
                ContactList::GroupIterator it;
                while ((grp = ++it) != NULL){
                    if (grp->getName() == args[3])
                        break;
                }
                if (grp == NULL){
                    grp = getContacts()->group(0, true);
                    grp->setName(args[3]);
                    bNewGrp = true;
                }
            }
            EventAddContact::AddContact ac;
            ac.proto = args[0];
            ac.addr  = args[1];
            if (args.size() > 2)
                ac.nick = args[2];
            ac.group = grp ? grp->id() : 0;
            EventAddContact e(&ac);
            e.process();
            Contact *contact = e.contact();
            if (contact){
                if (bNewGrp){
                    EventGroup e(grp, EventGroup::eChanged);
                    e.process();
                }
                if (nCmd == CMD_OPEN){
                    Message *m = new Message(MessageGeneric);
                    m->setContact(contact->id());
                    EventOpenMessage(m).process();
                    delete m;
                }
                return true;
            }
            if (bNewGrp)
                delete grp;
            out += "Can't create ";
            out += args[1];
            return false;
        }
    case CMD_DELETE:{
            ContactList::ContactIterator it;
            Contact *contact;
            while ((contact = ++it) != NULL){
                if (contact->getName() == args[0]){
                    delete contact;
                    return true;
                }
            }
            EventDeleteContact e(args[0]);
            if (e.process())
                return true;
            out = "Contact ";
            out += args[0];
            out += " not found";
            return false;
        }
    case CMD_SHOW:{
            Command cmd;
            if (core->unread.size())
                cmd->id = CmdUnread;
            else
                return false;
            EventCommandExec(cmd).process();
            return true;
        }
    case CMD_SMS:{
            SMSMessage *m = new SMSMessage;
            m->setPhone(args[0]);
            m->setText(args[1]);
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (client->send(m, NULL))
                    return true;
            }
            return false;
        }
    case CMD_HELP:
        if (args.size() == 0){
            for (c = cmds; c->cmd; c++){
                out += c->cmd;
                out += "\t";
                out += c->shortDescr;
                out += "\r\n";       
            }
        }else{
            args[0] = args[0].toUpper();
            for (c = cmds; c->cmd; c++)
                if (args[0] == c->cmd)
                    break;
            if (c->cmd == NULL){
                out = "Unknown command ";
                out += args[0];
                return false;
            }
            out = c->cmd;
            out += "\t";
            out += c->shortDescr;
            out += "\r\n";          //Fixme WIN32
            out += c->longDescr;
        }
        return true;
    }
    return false;
}

void RemotePlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr RemotePlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant RemotePlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void RemotePlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

static char Prompt[] = "\r\n>"; 

ControlSocket::ControlSocket(RemotePlugin *plugin, Socket *socket)
{
    m_plugin = plugin;
    m_plugin->m_sockets.push_back(this);
    m_socket = new ClientSocket(this);
    m_socket->setSocket(socket);
    m_socket->setRaw(true);
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
    write(Prompt);
}

ControlSocket::~ControlSocket()
{
    for (list<ControlSocket*>::iterator it = m_plugin->m_sockets.begin(); it != m_plugin->m_sockets.end(); ++it){
        if ((*it) == this){
            m_plugin->m_sockets.erase(it);
            break;
        }
    }
    delete m_socket;
}

void ControlSocket::write(const char *msg)
{
    log(L_DEBUG, "Remote write %s", msg);
    m_socket->writeBuffer().packetStart();
    m_socket->writeBuffer().pack(msg, strlen(msg));
    m_socket->write();
}

bool ControlSocket::error_state(const QString &err, unsigned)
{
    if (!err.isEmpty())
        log(L_WARN, "ControlSocket error %s", qPrintable(err));
    return true;
}

void ControlSocket::connect_ready()
{
}

void ControlSocket::packet_ready()
{
    QByteArray line;
    if (!m_socket->readBuffer().scan("\n", line))
        return;
    if (line.isEmpty())
        return;
	QString strLine=QString(line.data()).trimmed();
    /*if (line[(int)line.length() - 1] == '\r')
        line = line.left(line.size() - 1);*/
    log(L_DEBUG, "Remote read: %s", qPrintable(strLine));
    QString out;
    bool bError = false;
        bool bRes = m_plugin->command(strLine.toLatin1(), out, bError);
    if (bError){
        m_socket->error_state("");
        return;
    }
    if (!bRes)
        write("? ");
    QByteArray s;
    if (!out.isEmpty())
        s = out.toLocal8Bit();
    QByteArray res;
	strLine=QString(s).trimmed();
	
	//if (!strLine.contains('\n'))
	strLine += "\r\n";
	if (strLine.trimmed().isEmpty()) return;
        res=strLine.toLocal8Bit();
	
    /*for (const char *p = s.data(); *p ; p++){
        if (*p == '\r')
            continue;
        if (*p == '\n')
            res += '\r';
        res += *p;
    }*/
    write(res);
    write(Prompt);
}

