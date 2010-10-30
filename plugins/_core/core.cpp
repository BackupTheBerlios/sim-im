/***************************************************************************
  core.cpp  -  description
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

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <QSettings>
#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QTextCodec>
#include <QProcess>
#include <QInputDialog>
#include <QMessageBox>
#include <QTranslator>
#include <QByteArray>
#include <QDateTime>

// simlib

#include "simgui/ballonmsg.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"
#include "contacts/protocolmanager.h"
#include "contacts/imcontact.h"
#include "events/eventhub.h"
#include "profilemanager.h"
#include "clientmanager.h"
#include "contacts/contactlist.h"
#include "commands/commandhub.h"

// _core
#include "core.h"
#include "roster/userview.h"
#include "profileselectdialog.h"
#include "commonstatus.h"

using namespace std;
using namespace SIM;

//#ifdef WIN32

//class LockThread : public QThread
//{
//	public:
//		LockThread(Qt::HANDLE hEvent);
//		Qt::HANDLE hEvent;
//	protected:
//		void run();
//};

//#endif

//class FileLock : public QFile
//{
//	public:
//		FileLock(const QString &name);
//		~FileLock();
//		bool lock(bool bSend);
//	protected:
//#ifdef WIN32
//		LockThread	*m_thread;
//#else
//		bool m_bLock;
//#endif
//};

Plugin *createCorePlugin(unsigned /*base*/, bool, Buffer */*config*/)
{
    Plugin *plugin = new CorePlugin();
    return plugin;
}

Plugin *createCorePluginObject()
{
    Plugin *plugin = new CorePlugin();
    return plugin;
}


static PluginInfo info =
{
    I18N_NOOP("Core"),
    I18N_NOOP("Core plugin"),
    VERSION,
    PLUGIN_DEFAULT | PLUGIN_NODISABLE | PLUGIN_RELOAD,
    createCorePluginObject
};

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

//#if !defined(WIN32) && !defined(USE_KDE)

//struct loaded_domain;

//struct loaded_l10nfile
//{
//	const char *filename;
//	int decided;
//	const void *data;
//	loaded_l10nfile() : filename(0), decided(0), data(0) {}
//};


//void k_nl_load_domain (loaded_l10nfile *domain_file);
//void k_nl_unload_domain (loaded_domain *domain);
//char *k_nl_find_msg (loaded_l10nfile *domain_file, const char *msgid);

//#endif

//static CorePlugin* g_plugin = 0;

//CorePlugin* getCorePlugin()
//{
//    return g_plugin;
//}

//static QWidget *getInterfaceSetup(QWidget *parent, SIM::PropertyHubPtr data)
//{
//	return new MessageConfig(parent, data);
//}

//static QWidget *getSMSSetup(QWidget *parent, SIM::PropertyHubPtr data)
//{
//	return new SMSConfig(parent, data);
//}

//static QWidget *getHistorySetup(QWidget *parent, SIM::PropertyHubPtr data)
//{
//	return new UserHistoryCfg(parent, data);
//}

//struct autoReply
//{
//	unsigned	status;
//	const char	*text;
//};

//static autoReply autoReplies[] =
//{
//	{ STATUS_AWAY, I18N_NOOP(
//			"I am currently away from the computer,\n"
//			"please leave your message and I will get back to you as soon as I return!"
//			) },
//	{ STATUS_NA, I18N_NOOP(
//			"I am out'a here.\n"
//			"See you tomorrow!"
//			) },
//	{ STATUS_DND, I18N_NOOP(
//			"Please do not disturb me now. Disturb me later."
//			) },
//	{ STATUS_OCCUPIED, I18N_NOOP(
//			"I'm occupied at the moment. Please only urgent messages."
//			) },
//	{ STATUS_FFC, I18N_NOOP(
//			"We'd love to hear what you have to say. Join our chat."
//			) },
//	{ STATUS_ONLINE, I18N_NOOP(
//			"I'm here."
//			) },
//	{ STATUS_OFFLINE, I18N_NOOP(
//			"I'm offline."
//			) },
//	{ 0, NULL }
//};

CorePlugin::CorePlugin() : QObject()
    , Plugin            ()
//    , historyXSL        (NULL)
//    , m_bInit           (false)
//    , m_cfg             (NULL)
//    , m_focus           (NULL)
//    , m_search          (NULL)
//    , m_translator      (NULL)
//    , m_manager         (NULL)
//    , m_status          (NULL)
//    , m_statusWnd       (NULL)
//    , m_nClients        (0)
//    , m_nClientsMenu    (0)
//    , m_nResourceMenu   (0)
//    , m_alert           (NULL)
//    , m_lock            (NULL)
//    , m_RegNew          (false)
//    , m_tmpl            (new Tmpl(this))
//    , m_cmds            (new Commands())
//    , m_HistoryThread   (NULL)
//    , m_bIgnoreEvents   (false)
    , m_propertyHub(SIM::PropertyHub::create("_core"))
{
//    g_plugin = this;
//	setValue("StatusTime", QDateTime::currentDateTime().toTime_t());
//    m_containerManager = new ContainerManager(this);

//	boundTypes();

//	EventMenu(MenuFileDecline,      EventMenu::eAdd).process();
//	EventMenu(MenuMailList,         EventMenu::eAdd).process();
//	EventMenu(MenuPhoneList,        EventMenu::eAdd).process();
//	EventMenu(MenuStatusWnd,        EventMenu::eAdd).process();
//	EventMenu(MenuEncoding,         EventMenu::eAdd).process();
//	EventMenu(MenuSearchItem,       EventMenu::eAdd).process();
//	EventMenu(MenuSearchGroups,     EventMenu::eAdd).process();
//	EventMenu(MenuSearchOptions,    EventMenu::eAdd).process();

//	createMainToolbar();
//	createHistoryToolbar();
//	createContainerToolbar();
//	createMsgEditToolbar();
//	createTextEditToolbar();
//	createMenuMsgView();
//	createMenuTextEdit();

//	MsgEdit::setupMessages(); // Make sure this function is called after createContainerToolbar and createMsgEditToolbar
//    // because setupMessages() adds items to MenuMessage and to ToolBatMsgEdit, which are
//	// created by createContainerToolbar and createMsgEditToolbar
//	// If menu or toolbar were not created, items can't be added, and will be just missing
//	Command cmd;

//	EventMenu(MenuGroup, EventMenu::eAdd).process();
//	EventMenu(MenuContact, EventMenu::eAdd).process();
//	EventMenu(MenuContactGroup, EventMenu::eAdd).process();
//	EventMenu(MenuMsgCommand, EventMenu::eAdd).process();

//	createEventCmds();
    subscribeToEvents();

    m_commonStatus = new CommonStatus(getClientManager());
    m_main = new MainWindow(this);
}

void CorePlugin::subscribeToEvents()
{
    getEventHub()->getEvent("init")->connectTo(this, SLOT(eventInit()));
    getEventHub()->getEvent("quit")->connectTo(this, SLOT(eventQuit()));
}

void CorePlugin::eventQuit()
{
    //destroy();
}

void CorePlugin::createCommands()
{
//    log(L_DEBUG, "createMainToolbar()");
    UiCommandPtr showOffline = UiCommand::create(I18N_NOOP("Show &offline"), "online_on", "show_offline", QStringList("main"));
    showOffline->setWidgetType(UiCommand::wtButton);
    showOffline->setCheckable(true);
    showOffline->setChecked(m_main->userview()->isShowOffline());
    getCommandHub()->registerCommand(showOffline);

    UiCommandPtr dontshow = UiCommand::create(I18N_NOOP("Do&n't show groups"), "grp_off", "groupmode_dontshow", QStringList("groupmode_menu"));
    dontshow->setCheckable(true);
    dontshow->setChecked(true);
    dontshow->setAutoExclusive(true);
    UiCommandPtr mode1 = UiCommand::create(I18N_NOOP("Group mode 1"), "grp_on", "groupmode_mode1", QStringList("groupmode_menu"));
    mode1->setCheckable(true);
    mode1->setAutoExclusive(true);
    UiCommandPtr mode2 = UiCommand::create(I18N_NOOP("Group mode 2"), "grp_on", "groupmode_mode2", QStringList("groupmode_menu"));
    mode2->setCheckable(true);
    mode2->setAutoExclusive(true);

    UiCommandPtr showEmptyGroups = UiCommand::create(I18N_NOOP("Show &empty groups"), QString(), "show_empty_groups", QStringList("groupmode_menu"));

    UiCommandPtr createGroup = UiCommand::create(I18N_NOOP("&Create group"), "grp_create", "create_group", QStringList("groupmode_menu"));

    CommandSetPtr groupsset = getCommandHub()->createCommandSet("groups");
    groupsset->appendCommand(dontshow);
    groupsset->appendCommand(mode1);
    groupsset->appendCommand(mode2);
    groupsset->appendSeparator();
    groupsset->appendCommand(SIM::getCommandHub()->command("show_offline"));
    groupsset->appendCommand(showEmptyGroups);
    groupsset->appendSeparator();
    groupsset->appendCommand(createGroup);

    UiCommandPtr groups = UiCommand::create(I18N_NOOP("&Groups"), (m_main->userview()->groupMode() > 0) ? "grp_on" : "grp_off",
                                            "groupmode_menu", QStringList("main"));
    groups->setSubcommands(groupsset);
    groups->setWidgetType(UiCommand::wtButton);
    getCommandHub()->registerCommand(groups);

    createMainMenuCommand();

}

void CorePlugin::createMainMenuCommand()
{
    CommandSetPtr mainmenu = getCommandHub()->createCommandSet("main_menu");
    mainmenu->appendCommand(getCommandHub()->command("common_status"));
    mainmenu->appendCommand(getCommandHub()->command("groupmode_menu"));

    UiCommandPtr mainmenucmd = UiCommand::create("Main menu", "1downarrow", "main_menu");
    mainmenucmd->setWidgetType(UiCommand::wtButton);
    mainmenucmd->setSubcommands(mainmenu);
    getCommandHub()->registerCommand(mainmenucmd);
}

//void CorePlugin::createCommand(int id, const QString& text, const QString& icon, int menu_id,
//        int menu_grp, int bar_id, int bar_grp, int flags, const QString& accel)
//{
//    Command cmd;
//    cmd->id = id;
//    cmd->text = text;
//    cmd->icon = icon;
//    cmd->menu_id = menu_id;
//    cmd->menu_grp = menu_grp;
//    cmd->bar_id = bar_id;
//    cmd->bar_grp = bar_grp;
//    cmd->flags = flags;
//    cmd->accel = accel;
//    EventCommandCreate(cmd).process();
//}

//ContainerManager* CorePlugin::containerManager() const
//{
//    return m_containerManager;
//}

//void CorePlugin::createEventCmds()
//{
//    Command cmd;
//    createCommand(CmdMsgQuote, I18N_NOOP("&Quote"), QString::null, MenuMsgCommand, 0x1002,
//            0, 0, COMMAND_CHECK_STATE, QString::null);

//    createCommand(CmdMsgQuote + CmdReceived, I18N_NOOP("&Quote"), QString::null, MenuMsgCommand, 0x1002,
//            ToolBarMsgEdit, 0x1041, COMMAND_CHECK_STATE | BTN_PICT, QString::null);

//    createCommand(CmdMsgForward, I18N_NOOP("&Forward"), QString::null, MenuMsgCommand, 0x1003,
//            0, 0, COMMAND_CHECK_STATE, QString::null);

//    createCommand(CmdMsgForward + CmdReceived, I18N_NOOP("&Forward"), QString::null, MenuMsgCommand, 0x1003,
//            ToolBarMsgEdit, 0x1042, COMMAND_CHECK_STATE | BTN_PICT, QString::null);

//    createCommand(CmdMsgAnswer, I18N_NOOP("&Answer"), "mail_generic", MenuMsgCommand, 0x1003,
//            ToolBarMsgEdit, 0x8000, COMMAND_CHECK_STATE | BTN_PICT, QString::null);


//    EventMenu(MenuContainer, EventMenu::eAdd).process();

//    createCommand(0, I18N_NOOP("&Messages"), "message", MenuMsgCommand, 0x1003, 0, 0x8000, COMMAND_CHECK_STATE | BTN_PICT, "_core");

//    cmd->id			= 0;
//    cmd->text		= I18N_NOOP("SMS");
//    cmd->icon		= "cell";
//    cmd->icon_on	= QString::null;
//    cmd->param		= (void*)getSMSSetup;
//    cmd->accel      = "SMS";
//    EventAddPreferences(cmd).process();

//    cmd->id			= 0;
//    cmd->text		= I18N_NOOP("&History setup");
//    cmd->icon		= "history";
//    cmd->icon_on	= QString::null;
//    cmd->param		= (void*)getHistorySetup;
//    cmd->accel      = "history";
//    EventAddPreferences(cmd).process();

//    cmd->id			= CmdGrpCreate;
//    cmd->text		= I18N_NOOP("&Create group");
//    cmd->icon		= "grp_create";
//    cmd->icon_on	= QString::null;
//    cmd->menu_id	= MenuGroup;
//    cmd->menu_grp	= 0x4000;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdGrpRename;
//    cmd->text		= I18N_NOOP("&Rename group");
//    cmd->icon		= "grp_rename";
//    cmd->accel		= "F2";
//    cmd->menu_grp	= 0x4001;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdGrpDelete;
//    cmd->text		= I18N_NOOP("&Delete group");
//    cmd->icon		= "remove";
//    cmd->accel		= "Del";
//    cmd->menu_grp	= 0x4002;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdGrpUp;
//    cmd->text		= I18N_NOOP("Up");
//    cmd->icon		= "1uparrow";
//    cmd->accel		= "Ctrl+Up";
//    cmd->menu_grp	= 0x6000;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdGrpDown;
//    cmd->text		= I18N_NOOP("Down");
//    cmd->icon		= "1downarrow";
//    cmd->accel		= "Ctrl+Down";
//    cmd->menu_grp	= 0x6001;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdGrpTitle;
//    cmd->text		= "_";
//    cmd->icon		= "grp_on";
//    cmd->accel		= QString::null;
//    cmd->menu_grp	= 0x1000;
//    cmd->flags		= COMMAND_CHECK_STATE | COMMAND_TITLE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdConfigure;
//    cmd->text		= I18N_NOOP("Setup");
//    cmd->icon		= "configure";
//    cmd->menu_grp	= 0xB000;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContactTitle;
//    cmd->text		= "_";
//    cmd->icon		= QString::null;
//    cmd->accel		= QString::null;
//    cmd->menu_id	= MenuContact;
//    cmd->menu_grp	= 0x1000;
//    cmd->popup_id	= 0;
//    cmd->flags		= COMMAND_CHECK_STATE | COMMAND_TITLE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdUnread;
//    cmd->menu_grp	= 0x1000;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdSendMessage;
//    cmd->menu_grp	= 0x2000;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdClose;
//    cmd->text		= "&Close";
//    cmd->icon		= "exit";
//    cmd->menu_id	= MenuContact;
//    cmd->menu_grp	= 0xF000;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContactGroup;
//    cmd->text		= I18N_NOOP("Group");
//    cmd->icon		= "grp_on";
//    cmd->menu_grp	= 0x8000;
//    cmd->popup_id	= MenuContactGroup;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContactRename;
//    cmd->text		= I18N_NOOP("&Rename");
//    cmd->icon		= QString::null;
//    cmd->menu_grp	= 0x8001;
//    cmd->popup_id	= 0;
//    cmd->accel		= "F2";
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContactDelete;
//    cmd->text		= I18N_NOOP("&Delete");
//    cmd->icon		= "remove";
//    cmd->menu_grp	= 0x8002;
//    cmd->accel		= "Del";
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdShowAlways;
//    cmd->text		= I18N_NOOP("Show &always");
//    cmd->icon		= QString::null;
//    cmd->menu_grp	= 0x8003;
//    cmd->accel		= QString::null;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id         = CmdFetchAway;
//    cmd->text       = I18N_NOOP("&Fetch away message");
//    cmd->icon       = "message";
//    cmd->menu_grp   = 0x8020;
//    cmd->flags      = COMMAND_CHECK_STATE | BTN_HIDE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdInfo;
//    cmd->text		= I18N_NOOP("User &info");
//    cmd->icon		= "info";
//    cmd->menu_grp	= 0x7010;
//    cmd->accel		= QString::null;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdHistory;
//    cmd->text		= I18N_NOOP("&History");
//    cmd->icon		= "history";
//    cmd->menu_grp	= 0x7020;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdConfigure;
//    cmd->text		= I18N_NOOP("Setup");
//    cmd->icon		= "configure";
//    cmd->menu_grp	= 0x7020;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContainer;
//    cmd->text		= I18N_NOOP("To container");
//    cmd->icon		= QString::null;
//    cmd->popup_id	= MenuContainer;
//    cmd->menu_grp	= 0x8010;
//    cmd->accel		= QString::null;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContainer;
//    cmd->text		= "_";
//    cmd->popup_id	= 0;
//    cmd->menu_id	= MenuContainer;
//    cmd->menu_grp	= 0x1000;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdContactGroup;
//    cmd->text		= "_";
//    cmd->icon		= QString::null;
//    cmd->accel		= QString::null;
//    cmd->menu_id	= MenuContactGroup;
//    cmd->menu_grp	= 0x2000;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdDeclineWithoutReason;
//    cmd->text		= I18N_NOOP("Decline file without reason");
//    cmd->icon		= QString::null;
//    cmd->menu_id	= MenuFileDecline;
//    cmd->menu_grp   = 0x1000;
//    cmd->flags		= COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdDeclineReasonBusy;
//    cmd->text		= I18N_NOOP("Sorry, I'm busy right now, and can not respond to your request");
//    cmd->menu_grp   = 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdDeclineReasonLater;
//    cmd->text		= I18N_NOOP("Sorry, I'm busy right now, but I'll be able to respond to you later");
//    cmd->menu_grp   = 0x1002;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdDeclineReasonInput;
//    cmd->text		= I18N_NOOP("Enter a decline reason");
//    cmd->menu_grp   = 0x1004;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdEditList;
//    cmd->text		= I18N_NOOP("&Edit");
//    cmd->icon		= "mail_generic";
//    cmd->menu_id	= MenuMailList;
//    cmd->menu_grp	= 0x1000;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdRemoveList;
//    cmd->text		= I18N_NOOP("&Delete");
//    cmd->icon		= "remove";
//    cmd->menu_grp	= 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdEditList;
//    cmd->text		= I18N_NOOP("&Edit");
//    cmd->icon		= "phone";
//    cmd->menu_id	= MenuPhoneList;
//    cmd->menu_grp	= 0x1000;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdRemoveList;
//    cmd->text		= I18N_NOOP("&Delete");
//    cmd->icon		= "remove";
//    cmd->menu_grp	= 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdStatusWnd;
//    cmd->text		= "_";
//    cmd->icon		= QString::null;
//    cmd->menu_id	= MenuStatusWnd;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdChangeEncoding;
//    cmd->text		 = "_";
//    cmd->menu_id	 = MenuEncoding;
//    cmd->menu_grp	 = 0x1000;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdAllEncodings;
//    cmd->text		 = I18N_NOOP("&Show all encodings");
//    cmd->menu_id	 = MenuEncoding;
//    cmd->menu_grp	 = 0x8000;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdContactGroup;
//    cmd->text		 = I18N_NOOP("Add to &group");
//    cmd->icon		 = QString::null;
//    cmd->menu_id	 = MenuSearchItem;
//    cmd->menu_grp	 = 0x2000;
//    cmd->bar_id		 = 0;
//    cmd->bar_grp	 = 0;
//    cmd->popup_id	 = MenuSearchGroups;
//    cmd->flags		 = COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdSearchOptions;
//    cmd->text		 = "_";
//    cmd->icon		 = QString::null;
//    cmd->menu_id	 = MenuSearchItem;
//    cmd->menu_grp	 = 0x3000;
//    cmd->popup_id	 = 0;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdSearchInfo;
//    cmd->text		 = I18N_NOOP("&Info");
//    cmd->icon		 = "info";
//    cmd->menu_id	 = MenuSearchOptions;
//    cmd->menu_grp	 = 0x3000;
//    cmd->popup_id	 = 0;
//    cmd->flags		 = COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdSearchMsg;
//    cmd->text		 = I18N_NOOP("Send &message");
//    cmd->icon		 = "message";
//    cmd->menu_id	 = MenuSearchOptions;
//    cmd->menu_grp	 = 0x3001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdContactGroup;
//    cmd->text		 = "_";
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    cmd->menu_id	 = MenuSearchGroups;
//    cmd->menu_grp	 = 0x1000;
//    EventCommandCreate(cmd).process();
//}

//void CorePlugin::initData()
//{
//    delete historyXSL;

//    historyXSL = new XSL(value("HistoryStyle").toString());
//    if (value("EditBackground").toUInt() == 0 && value("EditForeground").toUInt() == 0)
//    {
//        QPalette pal = QApplication::palette();
//        setValue("EditBackground", pal.color(QPalette::Base).rgb() & 0xFFFFFF);
//        setValue("EditForeground", pal.color(QPalette::Text).rgb() & 0xFFFFFF);
//    }
//    editFont = FontEdit::str2font(value("EditFont").toString(), QApplication::font());
//    setAutoReplies();
//}

//void CorePlugin::setPropertyHub(SIM::PropertyHubPtr hub)
//{
//    m_propertyHub = hub;
//}

SIM::PropertyHubPtr CorePlugin::propertyHub()
{
    return m_propertyHub;
}

//QVariant CorePlugin::value(const QString& key)
//{
//    return m_propertyHub->value(key);
//}

//void CorePlugin::setValue(const QString& key, const QVariant& v)
//{
//    m_propertyHub->setValue(key, v);
//}

//void CorePlugin::setAutoReplies()
//{
//    SIM::PropertyHubPtr data = getContacts()->getUserData("AR");
//	for (autoReply *a = autoReplies; a->text; a++)
//    {
//		const QString &t = data->stringMapValue("AutoReply", a->status);
//        if (!t.isEmpty())
//            continue;

//        data->setStringMapValue("AutoReply", a->status, i18n(a->text));
//	}
//}

CorePlugin::~CorePlugin()
{
//    prepareConfig();
//    //PropertyHub::save();
//    destroy();
//    if (m_containerManager)
//    {
//        delete m_containerManager;
//        m_containerManager = NULL;
//    }
//    delete m_lock;
//    delete m_cmds;
//    delete m_tmpl;
//    delete m_status;
//    delete historyXSL;
//    delete m_HistoryThread;
//    delete m_containerManager;
//    delete m_main;

//    removeTranslator();
}

//QString CorePlugin::tsFile(const QString &lang)
//{
//#if defined( WIN32 ) || defined( __OS2__ )
//	// lang is ascii, so this works fine
//	QString s = "ts\\" + lang.toLower() + ".qm";
//	QFile f(app_file(s));
//	if (!f.exists())
//		return QString();
//#else
//	QString s = PREFIX "/share/locale/";
//	QString l = lang;
//	int idx = l.indexOf('.');
//	if(idx != -1)
//		l = l.left(idx);
//	s += l;
//	s += "/LC_MESSAGES/sim.mo";
//	QFile f(s);
//	if (!f.exists())
//    {
//		QString l = lang;
//		int idx = l.indexOf('_');
//		if(idx != -1)
//			l = l.left(idx);

//		s  = PREFIX "/share/locale/";
//		s += l;
//		s += "/LC_MESSAGES/sim.mo";
//        f.setFileName(s);
//		if (!f.exists())
//			return QString();
//	}
//#endif
//	return f.fileName();
//}

//void CorePlugin::installTranslator()
//{
//	m_translator = NULL;
//	QString lang = value("Lang").toString();
//    if (lang == "-")
//		return;
//	if (lang.isEmpty())
//    {
//#ifdef WIN32
//		char buff[256];
//		int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, buff, sizeof(buff));
//		if (res){
//			lang += (char)tolower(buff[0]);
//			lang += (char)tolower(buff[1]);
//		}
//#else
//    #ifdef USE_KDE
//		    return;
//    #else
//		    char *p = getenv("LANG");
//		    if (p)
//            {
//			    for (; *p; p++)
//                {
//				    if (*p == '.') break;
//				    lang += *p;
//			    }
//		    }
//    #endif
//#endif
//	}
//	QString ts = tsFile(lang);
//	if (ts.isEmpty())
//		return;
//	/*
//#if !defined(WIN32) && !defined(USE_KDE)
//m_translator = new SIMTranslator(NULL, po);
//#else
//*/
//	m_translator = new QTranslator(NULL);
//	m_translator->load(ts);
//	//#endif
//	qApp->installTranslator(m_translator);
//#if !defined(WIN32) && !defined(USE_KDE)
//	resetPlural();
//#endif
//	EventLanguageChanged e(m_translator);
//	e.process();
//}

//void CorePlugin::removeTranslator()
//{
//	if (m_translator)
//    {
//		qApp->removeTranslator(m_translator);
//		delete m_translator;
//		m_translator = NULL;
//#if !defined(WIN32) && !defined(USE_KDE)
//		resetPlural();
//#endif
//		EventLanguageChanged e(NULL);
//		e.process();
//	}
//}

//struct msgIndex
//{
//	unsigned	contact;
//	unsigned	type;
//};

//struct msgCount
//{
//	unsigned	count;
//	unsigned	index;
//};

//bool operator < (const msgIndex &a, const msgIndex &b)
//{
//	if (a.contact < b.contact)
//		return true;
//	if (a.contact > b.contact)
//		return false;
//	return a.type < b.type;
//}


//Client* CorePlugin::getClient(unsigned i)
//{
//    return getContacts()->getClient(i);
//}

//typedef map<msgIndex, msgCount> MAP_COUNT;

//void CorePlugin::getWays(vector<clientContact> &ways, Contact *contact)
//{
//    IMContact *data;
//    ClientDataIterator it = contact->clientDataIterator();
//	while ((data = ++it) != NULL)
//    {
//        IMContact *data1;
//        ClientDataIterator it1 = contact->clientDataIterator();
//		bool bOK = true;
//		while ((data1 = ++it1) != NULL)
//        {
//			if (data1 == data)
//				break;
//            if (data->getSign() != data1->getSign())
//				continue;
//			if (it.client()->compareData(data, data1))
//            {
//				bOK = false;
//				break;
//			}
//		}
//		if (!bOK)
//			continue;
//		clientContact c;
//		c.client = it.client();
//		c.data   = data;
//		c.bNew   = false;
//		ways.push_back(c);
//		for (unsigned i = 0; i < getContacts()->nClients(); i++)
//        {
//            Contact *clContact;
//            IMContact *data2 = data;
//            if (getClient(i) == it.client() || !getClient(i)->isMyData(data2, clContact) || clContact != contact)
//                continue;

//            clientContact c;
//            c.client = getClient(i);
//            c.data   = data2;
//            c.bNew   = false;
//            ways.push_back(c);
//		}
//	}
//}

//void CorePlugin::changeClientStatus(SIM::Client* client, const SIM::IMStatusPtr& status)
//{
//    if (status->hasText()) {
//        bool noShow = propertyHub()->value("NoShowAutoReply" + status->id()).toBool();
//        if (!noShow) {
//            AutoReplyDialog dlg(status);
//            if (!dlg.exec())
//                return;
//        }
//    }
//    client->changeStatus(status);
//}

//static const char *helpList[] =
//{
//	"&IP;",
//	I18N_NOOP("ip-address"),
//	"&Mail;",
//	I18N_NOOP("e-mail"),
//	"&Phone;",
//	I18N_NOOP("phone"),
//	"&Nick;",
//	I18N_NOOP("contact nick"),
//	"&Unread;",
//	I18N_NOOP("number of unread messages from this contact"),
//	"&Status;",
//	I18N_NOOP("contact status"),
//	"&TimeStatus;",
//	I18N_NOOP("time of set status"),
//	"&IntervalStatus;",
//	I18N_NOOP("time from set status"),
//	NULL,
//};

//#if 0
//I18N_NOOP("male", "%1 wrote:" )
//I18N_NOOP("female", "%1 wrote:" )
//#endif

//bool CorePlugin::processEventIconChanged()
//{
//    QStringList smiles;
//    getIcons()->getSmiles(smiles);
//    unsigned flags = 0;
//    QString smile_icon;
//    if (smiles.empty())
//        flags = BTN_HIDE;
//    else
//        smile_icon = smiles.front();
//    Command cmd;
//    cmd->id			= CmdSmile;
//    cmd->text		= I18N_NOOP("I&nsert smile");
//    cmd->icon		= smile_icon;
//    cmd->bar_id		= ToolBarMsgEdit;
//    cmd->bar_grp	= 0x7000;
//    cmd->flags		= COMMAND_CHECK_STATE | flags;
//    EventCommandChange(cmd).process();
//    return false;
//}

//bool CorePlugin::processEventJoinAlert()
//{
//    if (!value("NoJoinAlert").toBool() && (m_alert == NULL))
//    {
//        Command cmd;
//        cmd->id = CmdStatusBar;
//        EventCommandWidget eWidget(cmd);
//        eWidget.process();
//        QWidget *widget = eWidget.widget();
//        if (widget == NULL)
//            return true;
//        raiseWindow(widget->topLevelWidget());
//        QStringList l;
//        l.append(i18n("OK"));
//        m_alert = new BalloonMsg(NULL,
//                quoteString(
//                    i18n("At loading contact list contacts with identical names were automatically joined.\n"
//                        "If it is wrong, you can separate them. "
//                        "For this purpose in contact menu choose the necessary name and choose a command \"Separate\".")),
//                l, widget, NULL, false, true, 150, i18n("Don't show this message in next time"));
//        connect(m_alert, SIGNAL(finished()), this, SLOT(alertFinished()));
//    }
//    return true;
//}

//bool CorePlugin::processEventGroup(Event* e)
//{
//    EventGroup *ev = static_cast<EventGroup*>(e);
//    if (ev->action() != EventGroup::eChanged)
//        return false;
//    if (m_bIgnoreEvents)
//        return true;
//    return false;
//}

//bool CorePlugin::processEventDeleteMessage(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    History::del(em->msg());
//    return true;
//}

//bool CorePlugin::processEventRewriteMessage(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    History::rewrite(em->msg());
//    return false;
//}

//bool CorePlugin::processEventTmplHelp(SIM::Event* e)
//{
//    EventTmplHelp *eth = static_cast<EventTmplHelp*>(e);
//    QString str = eth->help();
//    for (const char **p = helpList; *p;)
//    {
//        str += *(p++);
//        str += " - ";
//        str += i18n(*(p++));
//        str += '\n';
//    }
//    str += '\n';
//    str += i18n("`<command>` - call <command> and substitute command output\n");
//    eth->setHelp(str);
//    return true;
//}

//bool CorePlugin::processEventTmplHelpList(SIM::Event* e)
//{
//    EventTmplHelpList *ethl = static_cast<EventTmplHelpList*>(e);
//    ethl->setHelpList(helpList);
//    return true;
//}

//bool CorePlugin::processEventARRequest(SIM::Event* e)
//{
//    EventARRequest *ear = static_cast<EventARRequest*>(e);
//    ARRequest *r = ear->request();
//    SIM::PropertyHubPtr ar;
//    QString tmpl;
//    if (r->contact) {
//        ar = r->contact->getUserData()->getUserData("AR");
//        if (ar)
//            tmpl = ar->stringMapValue("AutoReply", r->status);
//        if (tmpl.isEmpty()) {
//            ar.clear();
//            Group *grp = getContacts()->group(r->contact->getGroup());
//            if (grp)
//                ar = r->contact->getUserData()->getUserData("AR");
//            if (ar)
//                tmpl = ar->stringMapValue("AutoReply", r->status);
//        }
//    }
//    if (tmpl.isEmpty()) {
//        ar = getContacts()->getUserData("AR");
//        tmpl = ar->stringMapValue("AutoReply", r->status);
//        if (tmpl.isEmpty())
//            tmpl = ar->stringMapValue("AutoReply", STATUS_AWAY);
//    }
//    EventTemplate::TemplateExpand t;
//    t.contact	= r->contact;
//    t.param		= r->param;
//    t.receiver	= r->receiver;
//    t.tmpl		= tmpl;
//    EventTemplateExpand(&t).process();
//    return true;
//}

//bool CorePlugin::processEventSaveState(SIM::Event* e)
//{
//    SIM::PropertyHubPtr ar = getContacts()->getUserData("AR");
//    for (autoReply *a = autoReplies; a->text; a++)
//    {
//        QString t = ar->stringMapValue("AutoReply", a->status);
//        if (t == i18n(a->text))
//            ar->setStringMapValue("AutoReply", a->status, QString::null);
//    }
//    e->process(this);
//    setAutoReplies();
//    return true;
//}

//bool CorePlugin::processEventPluginChanged(SIM::Event* e)
//{
//    EventPluginChanged *p = static_cast<EventPluginChanged*>(e);
//    if (p->pluginName() == "_core")
//    {
//        QString profile = ProfileManager::instance()->currentProfileName();
//        setValue("StatusTime", (unsigned int)QDateTime::currentDateTime().toTime_t());
//        removeTranslator();
//        installTranslator();
//        initData();
//        EventUpdateCommandState(CmdOnline).process();
//    }
//    return false;
//}

void CorePlugin::eventInit()
{
    log(L_DEBUG, "CorePlugin::eventInit");
    if(!init()) {
        getEventHub()->triggerEvent("init_abort");
        return;
    }
//    QTimer::singleShot(0, this, SLOT(checkHistory()));
//    QTimer::singleShot(0, this, SLOT(postInit()));
}

//bool CorePlugin::processEventHomeDir(SIM::Event* e)
//{
//    EventHomeDir *homedir = static_cast<EventHomeDir*>(e);
//    QString fname = homedir->homeDir();
//    QString profile;
//    if(QDir(fname).isRelative())
//        profile = ProfileManager::instance()->currentProfileName();
//    if (profile.length())
//        profile += '/';
//    profile += fname;
//    homedir->setHomeDir(profile);
//    // dunno know if this is correct... :(
//    EventHomeDir eProfile(homedir->homeDir());
//    if (!eProfile.process(this))
//        homedir->setHomeDir(app_file(homedir->homeDir()));
//    else
//        homedir->setHomeDir(eProfile.homeDir());
//    makedir(homedir->homeDir());
//    return true;
//}

//bool CorePlugin::processEventGetProfile(SIM::Event* e)
//{
//    EventGetProfile *e_get_profile = static_cast<EventGetProfile*>(e);
//    e_get_profile->setProfileValue(ProfileManager::instance()->currentProfileName());
//    return true;
//}

//bool CorePlugin::processEventAddPreferences(SIM::Event* e)
//{
//    EventAddPreferences *ap = static_cast<EventAddPreferences*>(e);
//    CommandDef *cmd = ap->def();
//    cmd->menu_id = MenuGroup;
//    EventCommandCreate(cmd).process();
//    cmd->menu_id = MenuContact;
//    EventCommandCreate(cmd).process();
//    preferences.add(cmd);
//    return true;
//}

//bool CorePlugin::processEventRemovePreferences(SIM::Event* e)
//{
//    EventRemovePreferences *rm = static_cast<EventRemovePreferences*>(e);
//    unsigned long id = rm->id();
//    EventCommandRemove(id).process();
//    preferences.erase(id);
//    return true;
//}

//bool CorePlugin::processEventClientChanged(SIM::Event* e)
//{
//    if(e->type() == eEventClientsChanged)
//    {
//        if (m_bInit)
//            loadMenu();
//    }
//    if (getContacts()->nClients()){
//        unsigned i;
//        for (i = 0; i < getContacts()->nClients(); i++)
//            if (getContacts()->getClient(i)->getCommonStatus())
//                break;
//        if (i >= getContacts()->nClients()){
//            Client *client = getContacts()->getClient(0);
//            //setManualStatus(client->getManualStatus());
//            client->setCommonStatus(true);
//            EventClientChanged(client).process();
//        }
//    }
//    return false;
//}

//bool CorePlugin::processEventCreateMessageType(SIM::Event* e)
//{
//    EventCreateMessageType *ecmt = static_cast<EventCreateMessageType*>(e);
//    CommandDef *cmd = ecmt->def();
//    if (cmd->menu_grp){
//        cmd->menu_id = MenuMessage;
//        cmd->flags   = COMMAND_CHECK_STATE;
//        EventCommandCreate(cmd).process();
//    }
//    if (cmd->param){
//        MessageDef *mdef = (MessageDef*)(cmd->param);
//        if (mdef->cmdReceived){
//            for (const CommandDef *c = mdef->cmdReceived; !c->text.isEmpty(); c++){
//                CommandDef cmd = *c;
//                if(cmd.icon.isEmpty()){
//                    cmd.icon   = "empty";
//                    cmd.flags |= BTN_PICT;
//                }
//                cmd.id += CmdReceived;
//                cmd.menu_id  = 0;
//                cmd.menu_grp = 0;
//                cmd.flags	|= COMMAND_CHECK_STATE;
//                EventCommandCreate(&cmd).process();
//            }
//        }
//        if (mdef->cmdSent){
//            for (const CommandDef *c = mdef->cmdSent; !c->text.isEmpty(); c++){
//                CommandDef cmd = *c;
//                if(cmd.icon.isEmpty()){
//                    cmd.icon = "empty";
//                    cmd.flags |= BTN_PICT;
//                }
//                cmd.id += CmdReceived;
//                cmd.menu_id  = 0;
//                cmd.menu_grp = 0;
//                cmd.flags	|= COMMAND_CHECK_STATE;
//                EventCommandCreate(&cmd).process();
//            }
//        }
//    }
//    messageTypes.add(cmd);
//    QString name = typeName(cmd->text);
//    MAP_TYPES::iterator itt = types.find(name);
//    if (itt == types.end()){
//        types.insert(MAP_TYPES::value_type(name, cmd->id));
//    }else{
//        (*itt).second = cmd->id;
//    }
//    return true;
//}

//bool CorePlugin::processEventRemoveMessageType(SIM::Event* e)
//{
//    EventRemoveMessageType *ermt = static_cast<EventRemoveMessageType*>(e);
//    unsigned long id = ermt->id();
//    CommandDef *def;
//    def = CorePlugin::instance()->messageTypes.find(id);
//    if (def){
//        MessageDef *mdef = (MessageDef*)(def->param);
//        if (mdef->cmdReceived){
//            for (const CommandDef *c = mdef->cmdReceived; !c->text.isEmpty(); c++){
//                EventCommandRemove(c->id + CmdReceived).process();
//            }
//        }
//        if (mdef->cmdSent){
//            for (const CommandDef *c = mdef->cmdSent; !c->text.isEmpty(); c++){
//                EventCommandRemove(c->id + CmdReceived).process();
//            }
//        }
//    }
//    for (MAP_TYPES::iterator itt = types.begin(); itt != types.end(); ++itt){
//        if ((*itt).second == id){
//            types.erase(itt);
//            break;
//        }
//    }
//    EventCommandRemove(id).process();
//    messageTypes.erase(id);
//    return true;
//}

//bool CorePlugin::processEventContact(SIM::Event* e)
//{
//    EventContact *ec = static_cast<EventContact*>(e);
//    Contact *contact = ec->contact();
//    switch(ec->action()) {
//        case EventContact::eDeleted:
//            clearUnread(contact->id());
//            History::remove(contact);
//            break;
//        case EventContact::eChanged:
//            if (m_bIgnoreEvents)
//                return true;
//            if (contact->getIgnore())
//                clearUnread(contact->id());
//            break;
//        case EventContact::eOnline:
//            {
//                SIM::PropertyHubPtr data = contact->getUserData("_core");
//                if (!data.isNull() && data->value("OpenOnOnline").toBool()){
//                    Message *msg = new Message(MessageGeneric);
//                    msg->setContact(contact->id());
//                    EventOpenMessage(msg).process();
//                    delete msg; // wasn't here before event changes...
//                }
//                break;
//            }
//        default:
//            break;
//    }
//    return false;
//}

//bool CorePlugin::processEventMessageAcked(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    Message *msg = em->msg();
//    if (msg->baseType() == MessageFile){
//        QWidget *w = new FileTransferDlg(static_cast<FileMessage*>(msg));
//        raiseWindow(w);
//    }
//    return false;
//}

//bool CorePlugin::processEventMessageDeleted(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    Message *msg = em->msg();
//    History::del(msg->id());
//    for (list<msg_id>::iterator it = unread.begin(); it != unread.end(); ++it){
//        msg_id &m = *it;
//        if (m.id == msg->id()){
//            unread.erase(it);
//            break;
//        }
//    }
//    return false;
//}

//bool CorePlugin::processEventMessageReceived(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    Message *msg = em->msg();
//    Contact *contact = getContacts()->contact(msg->contact());
//    if (contact){
//        if (msg->getTime() == 0){
//            msg->setTime(QDateTime::currentDateTime().toTime_t());
//        }
//        unsigned type = msg->baseType();
//        if (type == MessageStatus){
//            SIM::PropertyHubPtr data = contact->getUserData("_core");
//            if ((data.isNull()) || !data->value("LogStatus").toBool())
//                return false;
//        }else if (type == MessageFile){
//            SIM::PropertyHubPtr data = contact->getUserData("_core");
//            if(!data.isNull()){
//                if (data->value("AcceptMode").toUInt() == 1){
//                    QString dir = data->value("IncomingPath").toString();
//                    if (!dir.isEmpty() && !dir.endsWith("/") && !dir.endsWith("\\"))
//                        dir += '/';
//                    dir = user_file(dir);
//                    EventMessageAccept(msg, dir,
//                            data->value("OverwriteFiles").toBool() ?
//                            Replace : Ask).process();
//                    return msg;
//                }
//                if (data->value("AcceptMode").toUInt() == 2){
//                    EventMessageDecline(msg, data->value("DeclineMessage").toString()).process();
//                    return msg;
//                }
//            }
//        }else{
//            contact->setLastActive(QDateTime::currentDateTime().toTime_t());
//            EventContact(contact, EventContact::eStatus).process();
//        }
//    }
//    return processEventSent(e);
//}

//bool CorePlugin::processEventSent(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    Message *msg = em->msg();
//    CommandDef *def = messageTypes.find(msg->type());
//    if (def){
//        History::add(msg, typeName(def->text));
//        if ((e->type() == eEventMessageReceived) && (msg->type() != MessageStatus)){
//            msg_id m;
//            m.id = msg->id();
//            m.contact = msg->contact();
//            m.client = msg->client();
//            m.type = msg->baseType();
//            unread.push_back(m);
//            if (msg->getFlags() & MESSAGE_NOVIEW)
//                return false;
//            Contact *contact = getContacts()->contact(msg->contact());
//            if (contact && (contact->getFlags() & CONTACT_TEMPORARY)){
//                contact->setFlags(contact->getFlags() & ~CONTACT_TEMPORARY);
//                EventContact(contact, EventContact::eChanged).process();
//            }
//            if (contact){
//                SIM::PropertyHubPtr data = contact->getUserData("_core");
//                if (!data.isNull() && data->value("OpenNewMessage").toUInt()){
//                    if (data->value("OpenNewMessage").toUInt() == NEW_MSG_MINIMIZE)
//                        msg->setFlags(msg->getFlags() | MESSAGE_NORAISE);
//                    EventOpenMessage(msg).process();
//                }
//            }
//        }
//    }
//    else
//    {
//        log(L_WARN,"No CommandDef for message %u found!",msg->type());
//    }
//    return false;
//}

//bool CorePlugin::processEventDefaultAction(SIM::Event* e)
//{
//    EventDefaultAction *eda = static_cast<EventDefaultAction*>(e);
//    unsigned long contact_id = eda->id();
//    unsigned index = 0;
//    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it, index++){
//        if (it->contact != contact_id)
//            continue;
//        Command cmd;
//        cmd->id = CmdUnread + index;
//        cmd->menu_id = MenuMain;
//        return EventCommandExec(cmd).process();
//    }
//    EventMenuGetDef eMenu(MenuMessage);
//    eMenu.process();
//    CommandsDef *cmdsMsg = eMenu.defs();
//    CommandsList itc(*cmdsMsg, true);
//    CommandDef *c;
//    while ((c = ++itc) != NULL){
//        c->param = (void*)(contact_id);
//        if(EventCheckCommandState(c).process()) {
//            return EventCommandExec(c).process();
//        }
//    }
//    return false;
//}

//bool CorePlugin::processEventLoadMessage(SIM::Event* e)
//{
//    EventLoadMessage *elm = static_cast<EventLoadMessage*>(e);
//    Message *msg = History::load(elm->id(), elm->client(), elm->contact());
//    elm->setMessage(msg);
//    return true;
//}

//bool CorePlugin::processEventOpenMessage(SIM::Event* e)
//{
//    EventMessage *em = static_cast<EventMessage*>(e);
//    Message *msg = em->msg();
//    if (msg->getFlags() & MESSAGE_NOVIEW)
//        return false;
//    Contact *contact = getContacts()->contact(msg->contact());
//    m_focus = qApp->focusWidget();
//    if (m_focus)
//        connect(m_focus, SIGNAL(destroyed()), this, SLOT(focusDestroyed()));
//    if (contact == NULL)
//        return false;
//    UserWnd		*userWnd	= NULL;
//    ContainerPtr container;
//    bool bNew = false;
//    for(int i = 0; i < containerManager()->containerCount(); i++)
//    {
//        container = containerManager()->container(i);
//        if(containerManager()->containerMode() == ContainerManager::cmSimpleMode)
//        {
//            if(container->isReceived() != ((msg->getFlags() & MESSAGE_RECEIVED) != 0))
//            {
//                container.clear();
//                continue;
//            }
//        }
//        userWnd = container->wnd(contact->id());
//        if (userWnd)
//            break;
//        container.clear();
//    }

//    if(userWnd == NULL)
//    {
//        if (contact->getFlags() & CONTACT_TEMP)
//        {
//            contact->setFlags(contact->getFlags() & ~CONTACT_TEMP);
//            EventContact(contact, EventContact::eChanged).process();
//        }
//        userWnd = new UserWnd(contact->id(), NULL, msg->getFlags() & MESSAGE_RECEIVED, msg->getFlags() & MESSAGE_RECEIVED);
//        if(containerManager()->containerMode() == ContainerManager::cmOneContainer)
//        {
//            if(containerManager()->containerCount() == 0)
//            {
//                container = containerManager()->makeContainer(1);
//                containerManager()->addContainer(container);
//                bNew = true;
//            }
//            else
//                container = containerManager()->container(0);
//        }
//        else if (containerManager()->containerMode() == ContainerManager::cmGroupContainers)
//        {
//            unsigned id = contact->getGroup() + CONTAINER_GRP;
//            container = containerManager()->containerById(id);
//            if(!container)
//            {
//                container = containerManager()->makeContainer(id);
//                containerManager()->addContainer(container);
//                bNew = true;
//            }
//        }
//        else
//        {
//            container = containerManager()->makeContainer(contact->id());
//            containerManager()->addContainer(container);
//            bNew = true;
//            if (containerManager()->containerMode() == ContainerManager::cmSimpleMode)
//                container->setReceived(msg->getFlags() & MESSAGE_RECEIVED);
//        }
//        container->addUserWnd(userWnd, (msg->getFlags() & MESSAGE_NORAISE) == 0);
//    }
//    else
//    {
//        if ((msg->getFlags() & MESSAGE_NORAISE) == 0)
//            container->raiseUserWnd(userWnd->id());
//    }
//    container->setNoSwitch(true);
//    userWnd->setMessage(msg);
//    if (msg->getFlags() & MESSAGE_NORAISE){
//        if (bNew){
//            container->m_bNoRead = true;
//#ifdef WIN32
//            ShowWindow(container->winId(), SW_SHOWMINNOACTIVE);
//#else
//            container->init();
//            container->showMinimized();
//#endif
//        }
//        if (m_focus)
//            m_focus->setFocus();
//    }else{
//        container->init();
//        container->show();
//        raiseWindow(container.data());
//    }
//    container->setNoSwitch(false);
//    if (m_focus)
//        disconnect(m_focus, SIGNAL(destroyed()), this, SLOT(focusDestroyed()));
//    m_focus = NULL;
//    return true;
//}

//bool CorePlugin::processCheckCmdChangeEncoding(SIM::CommandDef* cmd)
//{
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact == NULL)
//        return false;
//    QTextCodec *codec = getContacts()->getCodec(contact);
//    unsigned nEncoding = 3;
//    QStringList main;
//    QStringList nomain;
//    QStringList::Iterator it;
//    const ENCODING *enc;
//    for (enc = getContacts()->getEncodings(); enc->language; enc++){
//        if (enc->bMain){
//            main.append(i18n(enc->language) + " (" + enc->codec + ')');
//            nEncoding++;
//            continue;
//        }
//        if (!value("ShowAllEncodings").toBool())
//            continue;
//        nomain.append(i18n(enc->language) + " (" + enc->codec + ')');
//        nEncoding++;
//    }
//    CommandDef *cmds = new CommandDef[nEncoding];
//    cmd->param = cmds;
//    cmd->flags |= COMMAND_RECURSIVE;
//    nEncoding = 0;
//    cmds[nEncoding].id = 1;
//    cmds[nEncoding].text = I18N_NOOP("System");
//    if (!strcmp(codec->name(), "System"))
//        cmds[nEncoding].flags = COMMAND_CHECKED;
//    nEncoding++;
//    main.sort();
//    for (it = main.begin(); it != main.end(); ++it){
//        QString str = *it;
//        int n = str.indexOf('(');
//        str = str.mid(n + 1);
//        n = str.indexOf(')');
//        str = str.left(n);
//        if (str == codec->name())
//            cmds[nEncoding].flags = COMMAND_CHECKED;
//        cmds[nEncoding].id = nEncoding + 1;
//        cmds[nEncoding].text = "_";
//        cmds[nEncoding].text_wrk = (*it);
//        nEncoding++;
//    }
//    if (!value("ShowAllEncodings").toBool())
//        return true;
//    cmds[nEncoding++].text = "_";
//    nomain.sort();
//    for (it = nomain.begin(); it != nomain.end(); ++it){
//        QString str = *it;
//        int n = str.indexOf('(');
//        str = str.mid(n + 1);
//        n = str.indexOf(')');
//        str = str.left(n);
//        if (str == codec->name())
//            cmds[nEncoding].flags = COMMAND_CHECKED;
//        cmds[nEncoding].id = nEncoding;
//        cmds[nEncoding].text = "_";
//        cmds[nEncoding].text_wrk = (*it);
//        nEncoding++;
//    }
//    return true;
//}

//bool CorePlugin::processCheckCmdAllEncodings(SIM::CommandDef* cmd)
//{
//    cmd->flags &= ~COMMAND_CHECKED;
//    if (value("ShowAllEncodings").toBool())
//        cmd->flags |= COMMAND_CHECKED;
//    return true;
//}

//bool CorePlugin::processCheckCmdEnableSpell(SIM::CommandDef* cmd)
//{
//    cmd->flags &= ~COMMAND_CHECKED;
//    if (value("EnableSpell").toBool())
//        cmd->flags |= COMMAND_CHECKED;
//    return true;
//}

//bool CorePlugin::processCheckCmdSendClose(SIM::CommandDef* cmd)
//{
//    cmd->flags &= ~COMMAND_CHECKED;
//    if (value("CloseSend").toBool())
//        cmd->flags |= COMMAND_CHECKED;
//    return false;
//}

//bool CorePlugin::processCheckCmdContactClients(SIM::CommandDef* cmd)
//{
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact == NULL)
//        return false;
//    vector<clientContact> ways;
//    getWays(ways, contact);
//    if (cmd->menu_id == MenuMessage){
//        unsigned n = ways.size();
//        if (n < 1)
//            return false;
//        if (n == 1){
//            QString resources = ways[0].client->resources(ways[0].data);
//            if (resources.isEmpty())
//                return false;
//            QString wrk = resources;
//            unsigned n = 0;
//            while (!wrk.isEmpty()){
//                getToken(wrk, ';');
//                n++;
//            }
//            CommandDef *cmds = new CommandDef[n + 2];
//            cmds[0].text = "_";
//            n = 1;
//            while (!resources.isEmpty()){
//                unsigned long id = CmdContactResource + n;
//                if (n > m_nResourceMenu){
//                    m_nResourceMenu = n;
//                    EventMenu(id, EventMenu::eAdd).process();
//                    Command cmd;
//                    cmd->id			= CmdContactClients;
//                    cmd->text		= "_";
//                    cmd->menu_id	= id;
//                    cmd->menu_grp	= 0x1000;
//                    cmd->flags		= COMMAND_CHECK_STATE;
//                    EventCommandCreate(cmd).process();
//                }
//                cmds[n].id		 = id;
//                cmds[n].text	 = "_";
//                cmds[n].popup_id = id;
//                QString res = getToken(resources, ';');
//                cmds[n].icon     = (const char*)(getToken(res, ',').toULong());
//                QString t = ways[0].client->contactName(ways[0].data);
//                t += '/' + res;
//                cmds[n].text_wrk = t;
//                n++;
//            }
//            cmd->param = cmds;
//            cmd->flags |= COMMAND_RECURSIVE;
//            return true;
//        }
//        CommandDef *cmds = new CommandDef[n + 2];
//        cmds[0].text = "_";
//        n = 1;
//        for (vector<clientContact>::iterator itw = ways.begin(); itw != ways.end(); ++itw, n++){
//            unsigned long id  = CmdContactClients + n;
//            if (n > m_nClientsMenu){
//                m_nClientsMenu = n;
//                EventMenu(id, EventMenu::eAdd).process();

//                Command cmd;
//                cmd->id			= CmdContactClients;
//                cmd->text		= "_";
//                cmd->menu_id	= id;
//                cmd->menu_grp	= 0x1000;
//                cmd->flags		= COMMAND_CHECK_STATE;
//                EventCommandCreate(cmd).process();

//                cmd->id			= CmdSeparate;
//                cmd->text		= I18N_NOOP("&Separate");
//                cmd->menu_grp	= 0x2000;
//                cmd->flags		= COMMAND_DEFAULT;
//                EventCommandCreate(cmd).process();
//            }
//            cmds[n].id		 = id;
//            cmds[n].text	 = "_";
//            cmds[n].popup_id = id;
//            unsigned long status = STATUS_UNKNOWN;
//            unsigned style = 0;
//            QString statusIcon;
//            if (itw->bNew){
//                void *data = itw->data;
//                Client *client = contact->activeClient(data, itw->client);
//                if (client == NULL){
//                    client = itw->client;
//                    data   = itw->data;
//                }
//                client->contactInfo(data, status, style, statusIcon);
//            }else{
//                itw->client->contactInfo(itw->data, status, style, statusIcon);
//            }
//            cmds[n].icon = statusIcon;
//            QString t = itw->client->contactName(itw->data);
//            bool bFrom = false;
//            for (unsigned i = 0; i < getContacts()->nClients(); i++){
//                Client *client = getContacts()->getClient(i);
//                if (client == itw->client)
//                    continue;
//                Contact *contact;
//                IMContact *data = itw->data;
//                if (client->isMyData(data, contact)){
//                    bFrom = true;
//                    break;
//                }
//            }
//            if (bFrom){
//                t += ' ';
//                t += i18n("from %1") .arg(itw->client->name());
//            }
//            cmds[n].text_wrk = t;
//        }
//        cmd->param = cmds;
//        cmd->flags |= COMMAND_RECURSIVE;
//        return true;
//    }
//    if (cmd->menu_id > CmdContactResource){
//        unsigned nRes = cmd->menu_id - CmdContactResource - 1;
//        unsigned n;
//        for (n = 0; n < ways.size(); n++){
//            QString resources = ways[n].client->resources(ways[n].data);
//            while (!resources.isEmpty()){
//                getToken(resources, ';');
//                if (nRes-- == 0){
//                    clientContact &cc = ways[n];
//                    EventMenuGetDef eMenu(MenuMessage);
//                    eMenu.process();
//                    CommandsDef *cmdsMsg = eMenu.defs();
//                    unsigned nCmds = 0;
//                    {
//                        CommandsList it(*cmdsMsg, true);
//                        while (++it)
//                            nCmds++;
//                    }
//                    CommandDef *cmds = new CommandDef[nCmds];
//                    nCmds = 0;

//                    CommandsList it(*cmdsMsg, true);
//                    CommandDef *c;
//                    while ((c = ++it) != NULL){
//                        if ((c->id == MessageSMS) && (cc.client->protocol()->description()->flags & PROTOCOL_NOSMS))
//                            continue;
//                        if (!cc.client->canSend(c->id, cc.data)){
//                            EventCheckSend e(c->id, cc.client, cc.data);
//                            if (!e.process())
//                                continue;
//                        }
//                        cmds[nCmds] = *c;
//                        cmds[nCmds].id      = c->id;
//                        cmds[nCmds].flags	= COMMAND_DEFAULT;
//                        cmds[nCmds].menu_id = cmd->menu_id;
//                        nCmds++;
//                    }
//                    cmd->param = cmds;
//                    cmd->flags |= COMMAND_RECURSIVE;
//                    return true;
//                }
//            }
//        }
//        return false;
//    }
//    unsigned n = cmd->menu_id - CmdContactClients - 1;
//    if (n >= ways.size())
//        return false;
//    clientContact &cc = ways[n];

//    EventMenuGetDef eMenu(MenuMessage);
//    eMenu.process();
//    CommandsDef *cmdsMsg = eMenu.defs();
//    unsigned nCmds = 0;
//    {
//        CommandsList it(*cmdsMsg, true);
//        while (++it)
//            nCmds++;
//    }
//    QString resources = cc.client->resources(cc.data);
//    if (!resources.isEmpty()){
//        nCmds++;
//        while (!resources.isEmpty()){
//            getToken(resources, ';');
//            nCmds++;
//        }
//    }

//    CommandDef *cmds = new CommandDef[nCmds];
//    nCmds = 0;

//    CommandsList it(*cmdsMsg, true);
//    CommandDef *c;
//    while ((c = ++it) != NULL){
//        if ((c->id == MessageSMS) && (cc.client->protocol()->description()->flags & PROTOCOL_NOSMS))
//            continue;
//        if (!cc.client->canSend(c->id, cc.data)){
//            EventCheckSend e(c->id, cc.client, cc.data);
//            if (!e.process())
//                continue;
//        }
//        cmds[nCmds] = *c;
//        cmds[nCmds].id      = c->id;
//        cmds[nCmds].flags	= COMMAND_DEFAULT;
//        cmds[nCmds].menu_id = cmd->menu_id;
//        nCmds++;
//    }
//    resources = cc.client->resources(cc.data);
//    if (!resources.isEmpty()){
//        cmds[nCmds++].text = "_";
//        unsigned nRes = 1;
//        for (unsigned i = 0; i < n; i++){
//            QString resources = ways[i].client->resources(ways[i].data);
//            while (!resources.isEmpty()){
//                getToken(resources, ';');
//                unsigned long id = CmdContactResource + nRes;
//                if (nRes > m_nResourceMenu){
//                    m_nResourceMenu = nRes;
//                    EventMenu(id, EventMenu::eAdd).process();
//                    Command cmd;
//                    cmd->id			= CmdContactClients;
//                    cmd->text		= "_";
//                    cmd->menu_id	= id;
//                    cmd->menu_grp	= 0x1000;
//                    cmd->flags		= COMMAND_CHECK_STATE;
//                    EventCommandCreate(cmd).process();
//                }
//                nRes++;
//            }
//        }
//        QString resources = cc.client->resources(cc.data);
//        while (!resources.isEmpty()){
//            unsigned long id = CmdContactResource + nRes;
//            if (nRes > m_nResourceMenu){
//                m_nResourceMenu = nRes;
//                EventMenu(id, EventMenu::eAdd).process();
//                Command cmd;
//                cmd->id			= CmdContactClients;
//                cmd->text		= "_";
//                cmd->menu_id	= id;
//                cmd->menu_grp	= 0x1000;
//                cmd->flags		= COMMAND_CHECK_STATE;
//                EventCommandCreate(cmd).process();
//            }
//            cmds[nCmds].id		 = id;
//            cmds[nCmds].text	 = "_";
//            cmds[nCmds].popup_id = id;
//            QString res = getToken(resources, ';');
//            cmds[nCmds].icon     = (const char*)getToken(res, ',').toULong();
//            QString t = cc.client->contactName(ways[0].data);
//            t += '/' + res;
//            cmds[nCmds++].text_wrk = t;
//            nRes++;
//        }
//    }
//    cmd->param = cmds;
//    cmd->flags |= COMMAND_RECURSIVE;

//    return true;
//}

//bool CorePlugin::processCheckMenuContainer(SIM::CommandDef* cmd)
//{
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact){
//        unsigned nContainers = 1;
//        QWidgetList list = QApplication::topLevelWidgets();
//        QWidget * w;
//        foreach(w,list)
//        {
//            if (w->inherits("Container"))
//                nContainers++;
//        }
//        CommandDef *cmds = new CommandDef[nContainers + 1];
//        unsigned n = 0;
//        foreach(w,list)
//        {
//            if (w->inherits("Container")){
//                Container *c = static_cast<Container*>(w);
//                cmds[n] = *cmd;
//                cmds[n].icon = QString::null;
//                cmds[n].id = c->getId();
//                cmds[n].flags = COMMAND_DEFAULT;
//                cmds[n].text_wrk = c->name();
//                if (c->wnd(contact->id()))
//                    cmds[n].flags |= COMMAND_CHECKED;
//                n++;
//            }
//        }
//        cmds[n].icon = QString::null;
//        cmds[n].id = NEW_CONTAINER;
//        cmds[n].flags = COMMAND_DEFAULT;
//        cmds[n].text = I18N_NOOP("&New");
//        cmd->param = cmds;
//        cmd->flags |= COMMAND_RECURSIVE;
//        return true;
//    }
//    return false;
//}

//bool CorePlugin::processCheckMenuMessage(SIM::CommandDef* cmd)
//{
//    cmd->flags &= ~COMMAND_CHECKED;
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact){
//        vector<clientContact> ways;
//        getWays(ways, contact);
//        for (vector<clientContact>::iterator it = ways.begin(); it != ways.end(); ++it){
//            if ((cmd->id == MessageSMS) && (it->client->protocol()->description()->flags & PROTOCOL_NOSMS))
//                return false;
//            if (it->client->canSend(cmd->id, it->data)){
//                return true;
//            }
//        }
//        if ((cmd->id == MessageSMS) && !ways.empty()){
//            vector<clientContact>::iterator it;
//            for (it = ways.begin(); it != ways.end(); ++it){
//                if ((it->client->protocol()->description()->flags & PROTOCOL_NOSMS) == 0)
//                    break;
//            }
//            if (it == ways.end())
//                return false;
//        }
//    }
//    for (unsigned i = 0; i < getContacts()->nClients(); i++){
//        if (getContacts()->getClient(i)->canSend(cmd->id, NULL))
//            return true;
//    }
//    return false;
//}

//bool CorePlugin::processCheckMenuMsgCommand(SIM::CommandDef* cmd)
//{
//    Message *msg = (Message*)(cmd->param);
//    switch (cmd->id){
//        case CmdMsgQuote:
//        case CmdMsgForward:
//            if ((msg->getFlags() & MESSAGE_RECEIVED) == 0)
//                return false;
//            QString p = msg->presentation();
//            if (!p.isEmpty()){
//                unsigned type = msg->baseType();
//                switch (type){
//                    case MessageFile:
//                        return false;
//                }
//                cmd->flags &= ~COMMAND_CHECKED;
//                return true;
//            }
//            break;
//    }
//    return false;
//}

//bool CorePlugin::processCheckCmdPhoneLocation(SIM::CommandDef* cmd)
//{
//    unsigned n = 2;
//    QString phones = getContacts()->owner()->getPhones();
//    while (!phones.isEmpty()){
//        getToken(phones, ';');
//        n++;
//    }
//    CommandDef *cmds = new CommandDef[n];
//    n = 0;
//    cmds[n].id      = CmdPhoneLocation;
//    cmds[n].text    = I18N_NOOP("Not available");
//    cmds[n].menu_id = MenuPhoneLocation;
//    phones = getContacts()->owner()->getPhones();
//    bool bActive = false;
//    while (!phones.isEmpty()){
//        n++;
//        QString item = getToken(phones, ';', false);
//        item = getToken(item, '/', false);
//        QString number = getToken(item, ',');
//        getToken(item, ',');
//        unsigned long icon = getToken(item, ',').toULong();
//        cmds[n].id   = CmdPhoneLocation + n;
//        cmds[n].text = "_";
//        cmds[n].menu_id  = MenuPhoneLocation;
//        cmds[n].text_wrk = number;
//        if (!item.isEmpty()){
//            cmds[n].flags = COMMAND_CHECKED;
//            bActive = true;
//        }
//        switch (icon){
//            case PHONE:
//                cmds[n].icon = "phone";
//                break;
//            case FAX:
//                cmds[n].icon = "fax";
//                break;
//            case CELLULAR:
//                cmds[n].icon = "cell";
//                break;
//            case PAGER:
//                cmds[n].icon = "pager";
//                break;
//        }
//    }
//    if (!bActive)
//        cmds[0].flags = COMMAND_CHECKED;
//    cmd->param = cmds;
//    cmd->flags |= COMMAND_RECURSIVE;
//    return true;
//}

//bool CorePlugin::processCheckCmdUnread(SIM::CommandDef* cmd)
//{
//    unsigned long contact_id = 0;
//    if (cmd->menu_id == MenuContact)
//        contact_id = (unsigned long)cmd->param;
//    MAP_COUNT count;
//    MAP_COUNT::iterator itc;
//    CommandDef *def;
//    unsigned n = 0;
//    for (list<msg_id>::iterator it = unread.begin(); it != unread.end(); ++it, n++){
//        if (contact_id && (it->contact != contact_id))
//            continue;
//        msgIndex m;
//        m.contact = it->contact;
//        m.type    = it->type;
//        itc = count.find(m);
//        if (itc == count.end()){
//            msgCount c;
//            c.index = n;
//            c.count = 1;
//            count.insert(MAP_COUNT::value_type(m, c));
//        }else{
//            msgCount &c = (*itc).second;
//            c.index = n;
//            c.count++;
//        }
//    }
//    if (count.empty())
//        return false;
//    CommandDef *cmds = new CommandDef[count.size() + 1];
//    n = 0;
//    for (itc = count.begin(); itc != count.end(); ++itc, n++){
//        cmds[n].id = CmdUnread + (*itc).second.index;
//        def = messageTypes.find((*itc).first.type);
//        if (def == NULL)
//            continue;
//        MessageDef *mdef = (MessageDef*)(def->param);
//        cmds[n].icon = def->icon;
//        QString msg = i18n(mdef->singular, mdef->plural, (*itc).second.count);
//        if(msg.isEmpty())
//        {
//            log(L_ERROR, "Message is missing some definitions! Text: %s, ID: %lu",
//                    qPrintable(def->text), def->id);
//            int cnt = (*itc).second.count;
//            msg = QString("%1").arg(cnt);
//        }
//        if ((*itc).second.count == 1){
//            int n = msg.indexOf("1 ");
//            if (n == 0){
//                msg = msg.left(1).toUpper() + msg.mid(1);
//            }else{
//                msg = msg.left(n - 1);
//            }
//        }
//        if (contact_id == 0){
//            Contact *contact = getContacts()->contact((*itc).first.contact);
//            if (contact == NULL)
//                continue;
//            msg = i18n("%1 from %2")
//                .arg(msg)
//                .arg(contact->getName());
//        }
//        cmds[n].text_wrk = msg;
//        cmds[n].text = "_";
//    }
//    cmd->param = cmds;
//    cmd->flags |= COMMAND_RECURSIVE;
//    return true;
//}

//bool CorePlugin::processCheckCmdSendSMS(SIM::CommandDef* cmd)
//{
//    cmd->flags &= COMMAND_CHECKED;
//    for (unsigned i = 0; i < getContacts()->nClients(); i++){
//        Client *client = getContacts()->getClient(i);
//        if (client->canSend(MessageSMS, NULL))
//            return true;
//    }
//    return false;
//}

//bool CorePlugin::processCheckCmdShowPanel(SIM::CommandDef* cmd)
//{
//    cmd->flags &= ~COMMAND_CHECKED;
//    if (m_statusWnd)
//        cmd->flags |= COMMAND_CHECKED;
//    return true;
//}

//bool CorePlugin::processCheckCmdCommonStatus(SIM::CommandDef* cmd)
//{
//    unsigned n = cmd->menu_id - CmdClient;
//    if (n >= getContacts()->nClients())
//        return false;
//    Client *client = getContacts()->getClient(n);
//    cmd->flags &= ~COMMAND_CHECKED;
//    if (client->getCommonStatus())
//        cmd->flags |= COMMAND_CHECKED;
//    return true;
//}

//bool CorePlugin::processEventCheckCommandState(SIM::Event* e)
//{
//    EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
//    CommandDef *cmd = ecs->cmd();
//    if (cmd->menu_id == MenuEncoding){
//        if (cmd->id == CmdChangeEncoding) {
//            return processCheckCmdChangeEncoding(cmd);
//        }
//        if (cmd->id == CmdAllEncodings) {
//            return processCheckCmdAllEncodings(cmd);
//        }
//    }
//    if (cmd->id == CmdEnableSpell) {
//        return processCheckCmdEnableSpell(cmd);
//    }
//    if (cmd->id == CmdSendClose) {
//        return processCheckCmdSendClose(cmd);
//    }
//    if ((cmd->id == CmdFileAccept) || (cmd->id == CmdFileDecline)) {
//        Message *msg = (Message*)(cmd->param);
//        if (msg->getFlags() & MESSAGE_TEMP)
//            return true;
//        return false;
//    }
//    if (cmd->id == CmdContactClients) {
//        return processCheckCmdContactClients(cmd);
//    }
//    if (cmd->menu_id == MenuContainer) {
//        return processCheckMenuContainer(cmd);
//    }
//    if (cmd->menu_id == MenuMessage) {
//        return processCheckMenuMessage(cmd);
//    }
//    if (cmd->menu_id == MenuMsgCommand){
//        return processCheckMenuMsgCommand(cmd);
//    }
//    if (cmd->menu_id == MenuPhoneState){
//        cmd->flags &= ~COMMAND_CHECKED;
//        if (cmd->id == CmdPhoneNoShow + getContacts()->owner()->getPhoneStatus())
//            cmd->flags |= COMMAND_CHECKED;
//        return true;
//    }
//    if ((cmd->menu_id == MenuPhoneLocation) && (cmd->id == CmdPhoneLocation)){
//        return processCheckCmdPhoneLocation(cmd);
//    }
//    if (cmd->id == CmdUnread){
//        return processCheckCmdUnread(cmd);
//    }
//    if (cmd->id == CmdSendSMS){
//        return processCheckCmdSendSMS(cmd);
//    }
//    if (cmd->id == CmdShowPanel){
//        return processCheckCmdShowPanel(cmd);
//    }
//    if ((cmd->id == CmdContainer) && (cmd->menu_id == MenuContact)){
//        if (getContainerMode() && getContainerMode() != 3)
//            return true;
//        return false;
//    }
//    if (cmd->id == CmdCommonStatus){
//        return processCheckCmdCommonStatus(cmd);
//    }
//    if (cmd->id == CmdTitle) {
//        if (cmd->param && adjustClientItem(cmd->menu_id, cmd))
//            return true;
//        return false;
//    }
//    if (adjustClientItem(cmd->id, cmd))
//        return true;
//    unsigned n = cmd->menu_id - CmdClient;
//    if (n > getContacts()->nClients())
//        return false;
//    Client *client = getContacts()->getClient(n);
//    if (cmd->id == CmdInvisible){
//        if (client->getInvisible()){
//            cmd->flags |= COMMAND_CHECKED;
//        }else{
//            cmd->flags &= ~COMMAND_CHECKED;
//        }
//        return true;
//    }
//    const CommandDef *curStatus = NULL;
//    const CommandDef *d;
//    for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
//        if (d->id == cmd->id)
//            curStatus = d;
//    }
//    if (curStatus == NULL)
//        return 0;
//    bool bChecked = false;
//    unsigned status = client->getManualStatus();
//    bChecked = (status == curStatus->id);
//    if (bChecked){
//        cmd->flags |= COMMAND_CHECKED;
//    }else{
//        cmd->flags &= ~COMMAND_CHECKED;
//    }
//    return true;
//}

//bool CorePlugin::processExecMenuEncoding(SIM::CommandDef* cmd)
//{
//    if (cmd->id == CmdAllEncodings)
//    {
//        Command c;
//        c->id     = CmdChangeEncoding;
//        c->param  = cmd->param;
//        EventCommandWidget eWidget(cmd);
//        eWidget.process();
//        QToolButton *btn = qobject_cast<QToolButton*>(eWidget.widget());
//        if (btn)
//            QTimer::singleShot(0, btn, SLOT(animateClick()));
//        setValue("ShowAllEncodings", !value("ShowAllEncodings").toBool());
//        return true;
//    }
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact == NULL)
//        return false;
//    QByteArray codecStr;
//    const char *codec = NULL;
//    if (cmd->id == 1)
//    {
//        codec = "-";
//    }
//    else
//    {
//        QStringList main;
//        QStringList nomain;
//        QStringList::Iterator it;
//        const ENCODING *enc;
//        for (enc = getContacts()->getEncodings(); enc->language; enc++){
//            if (enc->bMain){
//                main.append(i18n(enc->language) + " (" + enc->codec + ')');
//                continue;
//            }
//            if (!value("ShowAllEncodings").toBool())
//                continue;
//            nomain.append(i18n(enc->language) + " (" + enc->codec + ')');
//        }
//        QString str;
//        main.sort();
//        int n = cmd->id - 1;
//        for (it = main.begin(); it != main.end(); ++it){
//            if (--n == 0){
//                str = *it;
//                break;
//            }
//        }
//        if (n >= 0){
//            nomain.sort();
//            for (it = nomain.begin(); it != nomain.end(); ++it){
//                if (--n == 0){
//                    str = *it;
//                    break;
//                }
//            }
//        }
//        if (!str.isEmpty()){
//            int n = str.indexOf('(');
//            str = str.mid(n + 1);
//            n = str.indexOf(')');
//            codecStr = str.left(n).toLatin1();
//            codec = codecStr;
//        }
//    }
//    if (codec == NULL)
//        return false;
//    QString oldCodec = contact->getEncoding();
//    if (oldCodec != codec){
//        contact->setEncoding(codec);
//        EventContact(contact, EventContact::eChanged).process();
//        EventHistoryConfig(contact->id()).process();
//    }
//    return false;
//}

//bool CorePlugin::processExecMenuMessage(SIM::CommandDef* cmd)
//{
//    Message *msg;
//    CommandDef *def = messageTypes.find(cmd->id);
//    if (def == NULL)
//        return false;
//    MessageDef *mdef = (MessageDef*)(def->param);
//    if (mdef->create == NULL)
//        return false;
//    msg = mdef->create(NULL);
//    msg->setContact((unsigned long)(cmd->param));
//    if (mdef->flags & MESSAGE_SILENT){
//        Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//        if (contact){
//            ClientDataIterator it = contact->clientDataIterator();
//            void *data;
//            while ((data = ++it) != NULL){
//                Client *client = it.client();
//                if (client->canSend(msg->type(), data) && client->send(msg, data))
//                    break;
//            }
//        }
//        return true;
//    }
//    EventOpenMessage(msg).process();
//    delete msg;
//    return true;
//}

//bool CorePlugin::processExecMenuMsgCommand(SIM::CommandDef* cmd)
//{
//    Message *msg = (Message*)(cmd->param);
//    QString p;
//    switch (cmd->id){
//        case CmdMsgQuote:
//        case CmdMsgForward:
//            p = msg->presentation();
//            if (p.isEmpty())
//                return false;
//            p = unquoteText(p);
//            QStringList l = p.split('\n');
//            QStringList::Iterator it;
//            if (l.count() && l.last().isEmpty()){
//                it = l.end();
//                --it;
//                l.removeLast();
//            }
//            for (it = l.begin(); it != l.end(); ++it)
//                (*it) = QLatin1String(">") + (*it);
//            p = l.join("\n");
//            Message *m = new Message(MessageGeneric);
//            m->setContact(msg->contact());
//            m->setClient(msg->client());
//            if (cmd->id == CmdMsgForward){
//                QString name;
//                Contact *contact = getContacts()->contact(msg->contact());
//                if (contact)
//                    name = contact->getName();
//                p = g_i18n("%1 wrote:", contact) .arg(name) + '\n' + p;
//                m->setFlags(MESSAGE_FORWARD);
//            }else{
//                m->setFlags(MESSAGE_INSERT);
//            }
//            m->setText(p);
//            EventOpenMessage(m).process();
//            delete m;
//            return true;
//    }
//    return false;
//}

//bool CorePlugin::processExecCmdGrantAuth(SIM::CommandDef* cmd)
//{
//    Message *from = (Message*)(cmd->param);
//    Message *msg = new AuthMessage(MessageAuthGranted);
//    msg->setContact(from->contact());
//    msg->setClient(from->client());
//    Contact *contact = getContacts()->contact(msg->contact());
//    if (contact){
//        void *data;
//        ClientDataIterator it = contact->clientDataIterator();
//        while ((data = ++it) != NULL){
//            Client *client = it.client();
//            if (!from->client().isEmpty()){
//                if ((client->dataName(data) == from->client()) && client->send(msg, data))
//                    return true;
//            }else{
//                if (client->canSend(MessageAuthGranted, data) && client->send(msg, data))
//                    return true;
//            }
//        }
//    }
//    delete msg;
//    return true;
//}

//bool CorePlugin::processExecCmdRefuseAuth(SIM::CommandDef* cmd)
//{
//    Message *from = (Message*)(cmd->param);
//    Message *msg = new AuthMessage(MessageAuthRefused);
//    msg->setContact(from->contact());
//    msg->setClient(from->client());
//    EventOpenMessage(msg).process();
//    delete msg;
//    return true;
//}

//bool CorePlugin::processExecCmdSeparate(SIM::CommandDef* cmd)
//{
//    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//    if (contact == NULL)
//        return false;
//    unsigned n = cmd->menu_id - CmdContactClients - 1;
//    vector<clientContact> ways;
//    getWays(ways, contact);
//    if (n >= ways.size())
//        return false;
//    clientContact &cc = ways[n];
//    IMContact *data;
//    ClientDataIterator it = contact->clientDataIterator(cc.client);
//    while ((data = ++it) != NULL){
//        if (data == cc.data)
//            break;
//    }
//    if (data == NULL){
//        data = cc.data;
//        cc.client->createData(data, contact);
//    }
//    Contact *newContact = getContacts()->contact(0, true);
//    newContact->setGroup(contact->getGroup());
//    newContact->join(data, contact);
//    contact->setup();
//    newContact->setup();
//    EventContact e1(contact, EventContact::eChanged);
//    e1.process();
//    EventContact e2(newContact, EventContact::eChanged);
//    e2.process();
//    return true;
//}

//bool CorePlugin::processExecCmdSendSMS(SIM::CommandDef* /*cmd*/)
//{
//    Contact *contact = getContacts()->contact(0, true);
//    contact->setFlags(CONTACT_TEMP);
//    contact->setName(i18n("Send SMS"));
//    EventContact eChanged(contact, EventContact::eChanged);
//    eChanged.process();
//    Command com;
//    com->id      = MessageSMS;
//    com->menu_id = MenuMessage;
//    com->param   = (void*)(contact->id());
//    EventCommandExec(com).process();
//    return true;
//}

//bool CorePlugin::processExecCmdHistory(SIM::CommandDef* cmd)
//{
//    unsigned long id = (unsigned long)(cmd->param);
//    if (!value("UseExtViewer").toBool()){
//        HistoryWindow *wnd = NULL;
//        QWidgetList list = QApplication::topLevelWidgets();
//        QWidget * w;
//        foreach(w,list)
//        {
//            if(w->inherits("HistoryWindow"))
//            {
//                wnd =  static_cast<HistoryWindow*>(w);
//                if (wnd->id() == id)
//                    break;
//                wnd = NULL;
//            }
//        }
//        if (wnd == NULL){
//            wnd = new HistoryWindow(id);
//            unsigned int historySizeX = value("HistorySizeX").toUInt();
//            unsigned int historySizeY = value("HistorySizeY").toUInt();
//            if(historySizeX && historySizeY)
//                wnd->resize(historySizeX, historySizeY);
//        }
//        raiseWindow(wnd);
//    }
//    else
//    {
//        if (!m_HistoryThread)
//            m_HistoryThread = new HistoryThread();
//        m_HistoryThread->set_id(id);
//        m_HistoryThread->set_Viewer(value("ExtViewer").toString());
//        m_HistoryThread->start();
//    }
//    return true;
//}

//bool CorePlugin::processExecCmdConfigure(SIM::CommandDef* cmd)
//{
//    if ((cmd->menu_id == MenuContact) || (cmd->menu_id == MenuGroup)){
//        showInfo(cmd);
//        return true;
//    }
//    if (m_cfg == NULL){
//        m_cfg = new ConfigDlg::ConfigureDialog();
//        connect(m_cfg, SIGNAL(finished()), this, SLOT(dialogFinished()));

//        unsigned int cfgGeometryWidth = value("CfgGeometryWidth").toUInt();
//        unsigned int cfgGeometryHeight = value("CfgGeometryHeight").toUInt();
//        if(cfgGeometryWidth == 0 || cfgGeometryHeight == 0)
//        {
//            cfgGeometryWidth = 500;
//            cfgGeometryHeight = 380;
//        }
//        m_cfg->resize(cfgGeometryWidth, cfgGeometryHeight);
//    }
//    raiseWindow(m_cfg);
//    return true;
//}

//bool CorePlugin::processExecCmdSearch(SIM::CommandDef* /*cmd*/)
//{
//    if (m_search == NULL){
//        m_search = new SearchDialog;
//        connect(m_search, SIGNAL(finished()), this, SLOT(dialogFinished()));
//        unsigned int searchGeometryWidth = value("SearchGeometryWidth").toUInt();
//        unsigned int searchGeometryHeight = value("SearchGeometryHeight").toUInt();
//        if(searchGeometryWidth == 0 || searchGeometryHeight == 0)
//        {
//            searchGeometryWidth = 500;
//            searchGeometryHeight = 380;
//        }
//        m_search->resize(searchGeometryWidth, searchGeometryHeight);
//    }
//    raiseWindow(m_search);
//    return false;
//}

//bool CorePlugin::processExecMenuPhoneState(SIM::CommandDef* cmd)
//{
//    Contact *owner = getContacts()->owner();
//    if ((unsigned long)owner->getPhoneStatus() != cmd->id - CmdPhoneNoShow){
//        owner->setPhoneStatus(cmd->id - CmdPhoneNoShow);
//        EventContact(owner, EventContact::eChanged).process();
//    }
//    return true;
//}

//bool CorePlugin::processExecMenuPhoneLocation(SIM::CommandDef* cmd)
//{
//    Contact *owner = getContacts()->owner();
//    unsigned n = cmd->id - CmdPhoneLocation;
//    QString res;
//    QString phones = owner->getPhones();
//    while (!phones.isEmpty()){
//        QString item = getToken(phones, ';', false);
//        QString v = getToken(item, '/', false);
//        QString number = getToken(v, ',', false);
//        QString type = getToken(v, ',', false);
//        QString icon = getToken(v, ',', false);
//        v = number + ',' + type + ',' + icon;
//        if (--n == 0)
//            v += ",1";
//        if (!res.isEmpty())
//            res += ';';
//        res += v;
//    }
//    if (res != owner->getPhones()){
//        owner->setPhones(res);
//        EventContact(owner, EventContact::eChanged).process();
//    }
//    return true;
//}

//bool CorePlugin::processExecCmdSetup(SIM::CommandDef* cmd)
//{
//    unsigned n = cmd->menu_id - CmdClient;
//    if (n >= getContacts()->nClients())
//        return false;
//    Client *client = getContacts()->getClient(n);
//    if (m_cfg == NULL){
//        m_cfg = new ConfigDlg::ConfigureDialog();
//        connect(m_cfg, SIGNAL(finished()), this, SLOT(dialogFinished()));
//    }
//    static_cast<ConfigDlg::ConfigureDialog*>(m_cfg)->raisePage(client);
//    raiseWindow(m_cfg);
//    return true;
//}

//bool CorePlugin::processExecCmdPhoneBook(SIM::CommandDef* /*cmd*/)
//{
//    if (m_cfg == NULL){
//        m_cfg = new ConfigDlg::ConfigureDialog;
//        connect(m_cfg, SIGNAL(finished()), this, SLOT(dialogFinished()));
//    }
//    static_cast<ConfigDlg::ConfigureDialog*>(m_cfg)->raisePhoneBook();
//    raiseWindow(m_cfg);
//    return true;
//}

//bool CorePlugin::processExecCmdCommonStatus(SIM::CommandDef* cmd)
//{
//    unsigned n = cmd->menu_id - CmdClient;
//    if (n >= getContacts()->nClients())
//        return false;
//    Client *client = getContacts()->getClient(n);
//    if (cmd->flags & COMMAND_CHECKED){
//        client->setStatus(getManualStatus(), true);
//    }else{
//        client->setStatus(client->getManualStatus(), false);
//    }
//    for (unsigned i = 0; i < getContacts()->nClients(); i++){
//        if (getContacts()->getClient(i)->getCommonStatus())
//            return true;
//    }
//    client = getContacts()->getClient(0);
//    if (client){
//        client->setCommonStatus(true);
//        EventClientChanged(client).process();
//    }
//    return true;
//}

//bool CorePlugin::processStatusChange(int clientnum, SIM::CommandDef* cmd)
//{
//    Client *client = getContacts()->getClient(clientnum);
//    if (cmd->id == CmdInvisible){
//        client->setInvisible(!client->getInvisible());
//        return true;
//    }
//    const CommandDef *d;
//    const CommandDef *curStatus = NULL;
//    for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
//        if (d->id == cmd->id)
//            curStatus = d;
//    }
//    if (curStatus == NULL)
//        return false;
//    if ((((cmd->id != STATUS_ONLINE) && (cmd->id != STATUS_OFFLINE)) ||
//                (client->protocol()->description()->flags & PROTOCOL_AR_OFFLINE))&&
//            (client->protocol()->description()->flags & (PROTOCOL_AR | PROTOCOL_AR_USER))){
//        QString noShow = propertyHub()->stringMapValue("NoShowAutoReply", cmd->id);
//        if (noShow.isEmpty()){
//            AutoReplyDialog dlg(cmd->id);
//            if (!dlg.exec())
//                return true;
//        }
//    }
//    client->setStatus(cmd->id, false);
//    return true;
//}

//bool CorePlugin::processEventCommandExec(SIM::Event* e)
//{
//    EventCommandExec *ece = static_cast<EventCommandExec*>(e);
//    CommandDef *cmd = ece->cmd();
//    if (cmd->menu_id == MenuEncoding) {
//        return processExecMenuEncoding(cmd);
//    }
//    if (cmd->id == CmdEnableSpell) {
//        setValue("EnableSpell", cmd->flags & COMMAND_CHECKED);
//        return false;
//    }
//    if (cmd->menu_id == MenuMessage){
//        return processExecMenuMessage(cmd);
//    }
//    if (cmd->menu_id == MenuMsgCommand){
//        return processExecMenuMsgCommand(cmd);
//    }
//    if (cmd->id == CmdGrantAuth){
//        return processExecCmdGrantAuth(cmd);
//    }
//    if (cmd->id == CmdRefuseAuth){
//        return processExecCmdRefuseAuth(cmd);
//    }

//    if (cmd->id == CmdSeparate){
//        return processExecCmdSeparate(cmd);
//    }
//    if (cmd->id == CmdSendClose){
//        setValue("CloseSend", (cmd->flags & COMMAND_CHECKED) != 0);
//        return true;
//    }
//    if (cmd->id == CmdSendSMS){
//        return processExecCmdSendSMS(cmd);
//    }
//    if (cmd->id == CmdHistory){
//        return processExecCmdHistory(cmd);
//    }
//    if (cmd->id == CmdConfigure){
//        return processExecCmdConfigure(cmd);
//    }
//    if (cmd->id == CmdSearch){
//        return processExecCmdSearch(cmd);
//    }
//    if ((cmd->menu_id == MenuContact) || (cmd->menu_id == MenuGroup)){
//        if (cmd->id == CmdInfo){
//            showInfo(cmd);
//            return true;
//        }
//        CommandDef *def = preferences.find(cmd->id);
//        if (def){
//            showInfo(cmd);
//            return true;
//        }
//    }
//    if (cmd->menu_id == MenuPhoneState){
//        return processExecMenuPhoneState(cmd);
//    }
//    if (cmd->menu_id == MenuPhoneLocation){
//        return processExecMenuPhoneLocation(cmd);
//    }
//    if (cmd->id == CmdSetup){
//        return processExecCmdSetup(cmd);
//    }
//    if (cmd->id == CmdPhoneBook){
//        return processExecCmdPhoneBook(cmd);
//    }
//    if (cmd->id == CmdCommonStatus){
//        return processExecCmdCommonStatus(cmd);
//    }
//    if (cmd->id == CmdProfileChange){
//        QTimer::singleShot(0, this, SLOT(selectProfile()));
//        return true;
//    }
//    unsigned n = cmd->menu_id - CmdClient;
//    if (n < getContacts()->nClients()){
//        return processStatusChange(n, cmd);
//    }
//    if ((cmd->id == CmdCM) || (cmd->id == CmdConnections)){
//        if (m_manager == NULL){
//            m_manager = new ConnectionManager(false);
//            connect(m_manager, SIGNAL(finished()), this, SLOT(managerFinished()));
//        }
//        raiseWindow(m_manager);
//        return true;
//    }
//    Message *msg = (Message*)(cmd->param);
//    if (cmd->id == CmdFileAccept){
//        Contact *contact = getContacts()->contact(msg->contact());
//        SIM::PropertyHubPtr data = contact->getUserData("_core");
//        QString dir;
//        if(!data.isNull())
//            dir = data->value("IncomingPath").toString();
//        if (!dir.isEmpty() && (!dir.endsWith("/")) && (!dir.endsWith("\\")))
//            dir += '/';
//        dir = user_file(dir);
//        EventMessageAccept(msg, dir, Ask).process();
//    }
//    if (cmd->id == CmdDeclineWithoutReason){
//        EventMessageDecline(msg).process();
//    }
//    if (cmd->id == CmdDeclineReasonBusy){
//        QString reason = i18n("Sorry, I'm busy right now, and can not respond to your request");
//        EventMessageDecline(msg, reason).process();
//    }
//    if (cmd->id == CmdDeclineReasonLater){
//        QString reason = i18n("Sorry, I'm busy right now, but I'll be able to respond to you later");
//        EventMessageDecline(msg, reason).process();
//    }
//    if(cmd->id == CmdDeclineReasonInput)
//    {
//        Message *msg = (Message*)(cmd->param);
//        QWidgetList list = QApplication::topLevelWidgets();
//        DeclineDlg *dlg = NULL;
//        QWidget *w;
//        foreach(w,list)
//        {
//            if(w->inherits("DeclineDlg"))
//            {
//                dlg = static_cast<DeclineDlg*>(w);
//                if (dlg->message()->id() == msg->id())
//                    break;
//                dlg = NULL;
//            }
//        }
//        if (dlg == NULL)
//            dlg = new DeclineDlg(msg);
//        raiseWindow(dlg);
//    }
//    if((cmd->id >= CmdUnread) && (cmd->id < CmdUnread + unread.size()))
//    {
//        unsigned n = cmd->id - CmdUnread;
//        for (list<msg_id>::iterator it = unread.begin(); it != unread.end(); ++it){
//            if (n-- == 0){
//                Message *msg = History::load(it->id, it->client, it->contact);
//                if (msg){
//                    msg->setFlags(msg->getFlags() & ~MESSAGE_NORAISE);
//                    EventOpenMessage(msg).process();
//                    delete msg;
//                    break;
//                }
//            }
//        }
//        return true;
//    }
//    if ((cmd->menu_id > CmdContactResource) && (cmd->menu_id <= CmdContactResource + 0x100)){
//        Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//        CommandDef *def = messageTypes.find(cmd->id);
//        if (def && contact){
//            unsigned nRes = cmd->menu_id - CmdContactResource - 1;
//            vector<clientContact> ways;
//            getWays(ways, contact);
//            for (unsigned n = 0; n < ways.size(); n++){
//                QString resources = ways[n].client->resources(ways[n].data);
//                while (!resources.isEmpty()){
//                    QString res = getToken(resources, ';');
//                    if (nRes-- == 0){
//                        clientContact &cc = ways[n];
//                        IMContact *data;
//                        ClientDataIterator it = contact->clientDataIterator(cc.client);
//                        while ((data = ++it) != NULL){
//                            if (data == cc.data)
//                                break;
//                        }
//                        if (data == NULL){
//                            data = cc.data;
//                            cc.client->createData(data, contact);
//                            EventContact(contact, EventContact::eChanged).process();
//                        }
//                        getToken(res, ',');
//                        MessageDef *mdef = (MessageDef*)(def->param);
//                        Message *msg = mdef->create(NULL);
//                        msg->setContact((unsigned long)(cmd->param));
//                        msg->setClient(cc.client->dataName(data));
//                        msg->setResource(res);
//                        EventOpenMessage(msg).process();
//                        delete msg;
//                        return true;
//                    }
//                }
//            }
//        }
//        return false;
//    }
//    if ((cmd->menu_id > CmdContactClients) && (cmd->menu_id <= CmdContactClients + 0x100)){
//        Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//        CommandDef *def = messageTypes.find(cmd->id);
//        if (def && contact){
//            unsigned n = cmd->menu_id - CmdContactClients - 1;
//            vector<clientContact> ways;
//            getWays(ways, contact);
//            if (n < ways.size()){
//                clientContact &cc = ways[n];

//                IMContact *data;
//                ClientDataIterator it = contact->clientDataIterator(cc.client);
//                while ((data = ++it) != NULL){
//                    if (data == cc.data)
//                        break;
//                }
//                if (data == NULL){
//                    data = cc.data;
//                    cc.client->createData(data, contact);
//                    EventContact(contact, EventContact::eChanged).process();
//                }

//                MessageDef *mdef = (MessageDef*)(def->param);
//                Message *msg = mdef->create(NULL);
//                msg->setContact((unsigned long)(cmd->param));
//                msg->setClient(cc.client->dataName(data));
//                EventOpenMessage(msg).process();
//                delete msg;
//                return true;
//            }
//        }
//    }
//    if (cmd->id == CmdShowPanel)
//    {
//        setValue("ShowPanel", ((cmd->flags & COMMAND_CHECKED) != 0));
//        //setShowPanel((cmd->flags & COMMAND_CHECKED) != 0);
//        showPanel();
//    }
//    return false;
//}

//bool CorePlugin::processEventGoURL(SIM::Event* e)
//{
//    EventGoURL *u = static_cast<EventGoURL*>(e);
//    QString url = u->url();
//    QString proto;
//    int n = url.indexOf(':');
//    if (n < 0)
//        return false;
//    proto = url.left(n);
//    url = url.mid(n + 1 );
//    if (proto == "sms"){
//        while (url[0] == '/')
//            url = url.mid(1);
//        Contact *contact = getContacts()->contactByPhone(url);
//        if (contact){
//            Command cmd;
//            cmd->id		 = MessageSMS;
//            cmd->menu_id = MenuMessage;
//            cmd->param	 = (void*)(contact->id());
//            EventCommandExec(cmd).process();
//        }
//        return true;
//    }
//    if (proto != "sim")
//        return false;
//    unsigned long contact_id = url.toULong();
//    Contact *contact = getContacts()->contact(contact_id);
//    if (contact){
//        Command cmd;
//        cmd->id		 = MessageGeneric;
//        cmd->menu_id = MenuMessage;
//        cmd->param	 = (void*)contact_id;
//        EventCommandExec(cmd).process();
//    }
//    return false;
//}

//bool CorePlugin::processEvent(Event *e)
//{
//	switch (e->type())
//    {
//		case eEventIconChanged:
//                return processEventIconChanged();

//		case eEventJoinAlert:
//                return processEventJoinAlert();

//		case eEventGroup:
//                return processEventGroup(e);

//		case eEventDeleteMessage:
//                return processEventDeleteMessage(e);

//		case eEventRewriteMessage:
//                return processEventRewriteMessage(e);

//		case eEventTmplHelp:
//                return processEventTmplHelp(e);

//		case eEventTmplHelpList:
//                return processEventTmplHelpList(e);

//		case eEventARRequest:
//                return processEventARRequest(e);

//		case eEventSaveState:
//                return processEventSaveState(e);

//		case eEventPluginChanged:
//                return processEventPluginChanged(e);

//		case eEventHomeDir:
//            return processEventHomeDir(e);

//		case eEventGetProfile:
//                return processEventGetProfile(e);

//		case eEventAddPreferences:
//                return processEventAddPreferences(e);

//		case eEventRemovePreferences:
//                return processEventRemovePreferences(e);

//		case eEventClientsChanged:
//		case eEventClientChanged:
//                return processEventClientChanged(e);

//		case eEventCreateMessageType:
//                return processEventCreateMessageType(e);

//		case eEventRemoveMessageType:
//                return processEventRemoveMessageType(e);

//		case eEventContact:
//                return processEventContact(e);

//		case eEventMessageAcked:
//                return processEventMessageAcked(e);

//		case eEventMessageDeleted:
//                return processEventMessageDeleted(e);

//		case eEventMessageReceived:
//                return processEventMessageReceived(e);

//		case eEventSent:
//                return processEventSent(e);

//		case eEventDefaultAction:
//                return processEventDefaultAction(e);

//		case eEventLoadMessage:
//                return processEventLoadMessage(e);

//		case eEventOpenMessage:
//                return processEventOpenMessage(e);

//		case eEventCheckCommandState:
//                return processEventCheckCommandState(e);

//		case eEventUpdateCommandState:
//			{
//				EventUpdateCommandState *eucs = static_cast<EventUpdateCommandState*>(e);
//				return updateMainToolbar(eucs->commandID());
//			}
//		case eEventCommandExec:
//            return processEventCommandExec(e);

//		case eEventGoURL:
//			return processEventGoURL(e);
//		default:
//			break;
//	}
//	return false;
//}

//void CorePlugin::showInfo(CommandDef *cmd)
//{
//	UserConfig *cfg = NULL;
//	Contact *contact = NULL;
//	Group   *group   = NULL;
//	unsigned long id = (unsigned long)(cmd->param);
//	if (cmd->menu_id == MenuContact)
//	{
//		contact = getContacts()->contact(id);
//		if (contact == NULL)
//			return;
//	}
//	if (cmd->menu_id == MenuGroup){
//		group = getContacts()->group(id);
//		if (group == NULL)
//			return;
//	}
//	if ((contact == NULL) && (group == NULL))
//		return;
//	QWidgetList list = QApplication::topLevelWidgets();
//    QWidget *w;
//    foreach(w,list)
//	{
//		if (w->inherits("UserConfig"))
//		{
//			cfg = static_cast<UserConfig*>(w);
//			if ((contact && (cfg->m_contact == contact)) ||
//					(group && (cfg->m_group == group)))
//				break;
//			cfg = NULL;
//		}
//	}
//	if (cfg == NULL){
//		cfg = new UserConfig(contact, group);
//        unsigned int cfgGeometryWidth = value("CfgGeometryWidth").toUInt();
//        unsigned int cfgGeometryHeight = value("CfgGeometryHeight").toUInt();
//        if(cfgGeometryWidth == 0 || cfgGeometryHeight == 0)
//        {
//            cfgGeometryWidth = 500;
//            cfgGeometryHeight = 380;
//        }
//        cfg->resize(cfgGeometryWidth, cfgGeometryHeight);
//	}
//	raiseWindow(cfg);
//	if (!cfg->raisePage(cmd->id))
//		cfg->raiseDefaultPage();
//}

//void CorePlugin::dialogFinished()
//{
//	QTimer::singleShot(0, this, SLOT(dialogDestroy()));
//}

//void CorePlugin::dialogDestroy()
//{
//	if (m_cfg && !m_cfg->isVisible()){
//		delete m_cfg;
//		m_cfg = NULL;
//	}
//	if (m_search && !m_search->isVisible()){
//		delete m_search;
//		m_search = NULL;
//	}
//}

//QWidget *CorePlugin::createConfigWindow(QWidget *parent)
//{
//	return new InterfaceConfig(parent);
//}

//void CorePlugin::hideWindows()
//{
//	QWidgetList list = QApplication::topLevelWidgets();
//	QWidget * w;
//    foreach(w,list)
//	{
//		w->hide();
//	}
//}

//void CorePlugin::ignoreEvents(bool i)
//{
//    m_bIgnoreEvents = i;
//}

//void CorePlugin::changeProfile(const QString& profilename)
//{
//	log(L_DEBUG, "CorePlugin::changeProfile()");
//	destroy();
//	getContacts()->clearClients();
//	getContacts()->clear();
//	setValue("StatusTime", (unsigned int)QDateTime::currentDateTime().toTime_t());
//	ProfileManager::instance()->selectProfile(profilename);
//	removeTranslator();
//	installTranslator();
//	initData();
//}

//void CorePlugin::selectProfile()
//{
//	log(L_DEBUG, "CorePlugin::selectProfile()");
//	EventSaveState e;
//	e.process();
//	bool changed = init(false);
////	if (changed){
////		EventInit e2;
////		e2.process();
////	}
//}

bool CorePlugin::init()
{
    log(L_DEBUG, "CorePlugin::init");
    QSettings settings;

    // FIXME:
    /*
    EventArg e1("-profile:", I18N_NOOP("Use specified profile"));
    e1.process();
    QString cmd_line_profile = e1.value();
    if (!cmd_line_profile.isEmpty()){
        bCmdLineProfile = true;
        setProfile(QString::null);
        QString profileDir = user_file(cmd_line_profile);
        QDir d(profileDir);
        if (d.exists()) {
            bCmdLineProfile = false;
            setProfile(cmd_line_profile);
        }
    }
    */

    QString profile = settings.value("Profile").toString();
    bool noshow = settings.value("NoShow", false).toBool();

    if(profile.isEmpty() && noshow)
    {
        settings.setValue("NoShow", false);
        noshow = false;
    }

    if(!noshow)
    {
        ProfileSelectDialog dlg;
        dlg.setModal(true);
        if(dlg.exec() != QDialog::Accepted)
            return false;

        profile = dlg.profile();
    }

    log(L_DEBUG, "Profile selected: %s", qPrintable(profile));

    m_propertyHub = ProfileManager::instance()->getPropertyHub("_core");
    if(!m_propertyHub)
        return false;

    createCommands();

    getEventHub()->triggerEvent("load_config");

    QStringList clients = getClientManager()->clientList();
    foreach(const QString& clname, clients)
    {
        ClientPtr client = getClientManager()->client(clname);
        client->changeStatus(client->savedStatus());
    }

    getContactList()->load();
    log(L_DEBUG, "Contact list loaded");

    m_main->init();
    m_main->show();

    return true;

//    if (getRegNew()&&!bCmdLineProfile){
//        hideWindows();
//        NewProtocol pDlg(NULL,1,true);
//        pDlg.exec();
//    }

//    loadUnread();
//    containerManager()->init();

//    log(L_DEBUG, "geometry: %s", value("geometry").toByteArray().toHex().data());
//    m_main->restoreGeometry(value("geometry").toByteArray());

//    m_main->show();

//    EventLoginStart e;
//    e.process();

//    if (!bNew)
//    {
//        QString containers = value("Containers").toString();
//        QVariantMap containerMap = value("Container").toMap();
//        while (!containers.isEmpty())
//        {
//            Container *c = new Container(0, containerMap.value(getToken(containers, ',')).toString().toUtf8().constData());
//            c->init();
//        }
//    }
//    //clearContainer();
//    setValue("Containers", QString());
//    setValue("Container", QVariantMap());

//    m_bInit = true;
//    loadMenu();
//    if (!bRes)
//    {
//        EventSaveState eSave;
//        eSave.process();
//        return true;
//    }
//    return bRes || bNew;
}

//void CorePlugin::startLogin()
//{
//    ClientList clients;
//    SIM::getClientManager()->load();
//    foreach(const QString& clname, SIM::getClientManager()->clientList()) {
//        clients.push_back(SIM::getClientManager()->client(clname));
//    }
//    clients.addToContacts();
//    getContacts()->load();

//    for (unsigned i = 0; i < clients.size(); i++)
//    {
//        Client *client = getContacts()->getClient(i);
//        unsigned status = client->getStatus();
//        if (status == STATUS_OFFLINE)
//            status = STATUS_ONLINE;
//        client->setStatus(status, client->getCommonStatus());
//    }
//}

//void CorePlugin::destroy()
//{
//    QWidgetList l = QApplication::topLevelWidgets();
//    QWidget *w;
//    list<QWidget*> forRemove;
//    foreach(w,l)
//    {
//        if (w->inherits("Container") ||
//            w->inherits("HistoryWindow") ||
//            w->inherits("UserConfig"))
//            forRemove.push_back(w);
//    }
//    for(list<QWidget*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
//        delete *itr;

//    if (m_statusWnd)
//    {
//        delete m_statusWnd;
//        m_statusWnd = NULL;
//    }
//    if (m_cfg)
//    {
//        delete m_cfg;
//        m_cfg = NULL;
//    }
//    if (m_search)
//    {
//        delete m_search;
//        m_search = NULL;
//    }
//    if (m_manager)
//    {
//        delete m_manager;
//        m_manager = NULL;
//    }
//}

//static char CLIENTS_CONF[] = "clients.conf";

//void CorePlugin::loadDir()
//{
//	//QString saveProfile = getProfile();
//	//setProfile(QString::null);
//	//bool bOK = false;
//	QString baseName = user_file(QString::null);
//	QDir dir(baseName);
//	dir.setFilter(QDir::Dirs);
//	QStringList list = dir.entryList();
//	for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
//    {
//		QString entry = *it;
//		if (entry[0] == '.')
//			continue;
//		QString fname = QString(baseName).append("/").append(entry).append("/").append(CLIENTS_CONF);
//		QFile f(fname);
//		if (f.exists()){
//			m_profiles.append(entry);
//			/*
//			if (entry == saveProfile)
//				bOK = true;
//				*/
//		}
//	}
//	/*
//	if (bOK)
//		setProfile(saveProfile);
//		*/
//}

//void CorePlugin::prepareConfig()
//{
//	QString unread_str;
//	for(list<msg_id>::iterator itUnread = unread.begin(); itUnread != unread.end(); ++itUnread)
//	{
//		msg_id &m = (*itUnread);
//		if (!unread_str.isEmpty())
//			unread_str += ';';
//		unread_str += QString::number(m.contact);
//		unread_str += ',';
//		unread_str += QString::number(m.id);
//		unread_str += ',';
//		unread_str += m.client;
//	}
//	setValue("Unread", unread_str);

////	unsigned editBgColor = value("EditBackground").toUInt();
////	unsigned editFgColor = value("EditForeground").toUInt();

//	QPalette pal = QApplication::palette();
//        if (((pal.color(QPalette::Base).rgb() & 0xFFFFFF) == value("EditBackground").toUInt()) &&
//            ((pal.color(QPalette::Text).rgb() & 0xFFFFFF) == value("EditForeground").toUInt()))
//	{
//		setValue("EditBackground", 0xffffff);
//		setValue("EditForeground", 0xffffff);
//	}

//	QString ef     = FontEdit::font2str(editFont, false);
//	QString def_ef = FontEdit::font2str(QApplication::font(), false);
//	setValue("EditFont", ef);
//	if ((ef == def_ef) || !value("EditSaveFont").toBool())
//		setValue("EditFont", QString());

//	//clearContainer();
//	QString containers;

//	QWidgetList list = QApplication::topLevelWidgets();
//	QWidget* w;
//	QVariantMap containerMap;
//    foreach(w,list)
//	{
//		if (w->inherits("Container"))
//		{
//			Container *c = static_cast<Container*>(w);
//            if (c->isReceived())
//                continue;
//			if (!containers.isEmpty())
//				containers += ',';
//			containers += QString::number(c->getId());
//			containerMap.insert(QString::number(c->getId()), c->getState());
//		}
//	}
//	setValue("Containers", containers);
//	if (m_main)
//	{
//		log(L_DEBUG, "Saving geometry");
//        setValue("geometry", m_main->saveGeometry());
//        setValue("toolbar_state", m_main->saveState());
//	}

//}

//static char BACKUP_SUFFIX[] = "~";
//QByteArray CorePlugin::getConfig()
//{
//	QString unread_str;
//	for(list<msg_id>::iterator itUnread = unread.begin(); itUnread != unread.end(); ++itUnread)
//	{
//		msg_id &m = (*itUnread);
//		if (!unread_str.isEmpty())
//			unread_str += ';';
//		unread_str += QString::number(m.contact);
//		unread_str += ',';
//		unread_str += QString::number(m.id);
//		unread_str += ',';
//		unread_str += m.client;
//	}
//	setValue("Unread", unread_str);

//	unsigned editBgColor = value("EditBackground").toUInt();
//	unsigned editFgColor = value("EditForeground").toUInt();

//	QPalette pal = QApplication::palette();
//        if (((pal.color(QPalette::Base).rgb() & 0xFFFFFF) == value("EditBackground").toUInt()) &&
//            ((pal.color(QPalette::Text).rgb() & 0xFFFFFF) == value("EditForeground").toUInt()))
//	{
//		setValue("EditBackground", 0);
//		setValue("EditForeground", 0);
//	}

//	QString ef     = FontEdit::font2str(editFont, false);
//	QString def_ef = FontEdit::font2str(QApplication::font(), false);
//	setValue("EditFont", ef);
//	if ((ef == def_ef) || !value("EditSaveFont").toBool())
//		setValue("EditFont", QString());

//	//clearContainer();
//	QString containers;

//	QWidgetList list = QApplication::topLevelWidgets();
//	QWidget* w;
//	QVariantMap containerMap;// = value("Container").toMap();
//    foreach(w,list)
//	{
//		if (w->inherits("Container"))
//		{
//			Container *c = static_cast<Container*>(w);
//            if (c->isReceived())
//                continue;
//			if (!containers.isEmpty())
//				containers += ',';
//			containers += QString::number(c->getId());
//			containerMap.insert(QString::number(c->getId()), c->getState());
//		}
//	}
//	setValue("Containers", containers);
//	if (m_main)
//	{
//		log(L_DEBUG, "Saving geometry");
//        setValue("geometry", m_main->saveGeometry());
//        setValue("toolbar_state", m_main->saveState());
//        /*
//		if (m_main->m_bar)
//		{
//			// Should update main toolbar pos only when toolbar is really exist...
//			saveToolbar(m_main->m_bar, data.toolBarState);
//		}
//        */
//	}

//	// We should save profile and noshow values in profile-independent _core config, and
//	// all other values in profile-dependent config.
//	// FIXME: This is a nasty hack, profile management should be rewritten

//	//Saving profile-independent config:
//	static DataDef generalCoreDataDef[] =
//	{
//		{ "Profile", DATA_STRING,  1, 0 },
//		{ NULL,      DATA_UNKNOWN, 0, 0 }
//	};
//	struct TGeneralCoreData
//	{
//		SIM::Data	Profile;
//	} GeneralCoreData;


//	QString saveProfile = ProfileManager::instance()->currentProfileName();
//	//setProfile(QString::null);

//	load_data(generalCoreDataDef, &GeneralCoreData, NULL);  // This will just init data
//	GeneralCoreData.Profile.str() = saveProfile;
//	//GeneralCoreData.NoShow.asBool() = getNoShow();

//	QByteArray cfg = save_data(generalCoreDataDef, &GeneralCoreData);

//	QString cfgName = user_file("plugins.conf");
//	QFile fCFG(QString(cfgName).append(BACKUP_SUFFIX)); // use backup file for this ...
//	if(!fCFG.open(QIODevice::WriteOnly | QIODevice::Truncate))
//	{
//        log(L_ERROR, "Can't create %s", qPrintable(cfgName));
//	}
//	else
//	{
//		QByteArray write = "[_core]\n";
//		write += "enable,";
//		write += QByteArray::number(m_base);
//		write += '\n';
//		write += cfg;
//		fCFG.write(write);

//		fCFG.flush();  // Make sure that file is fully written and we will not get "Disk Full" error on fCFG.close
//        const QFile::FileError status = fCFG.error();
//		const QString errorMessage = fCFG.errorString();
//		fCFG.close();
//        if (status != QFile::NoError) {
//            log(L_ERROR, "IO error writing to file %s : %s", qPrintable(fCFG.fileName()), qPrintable(errorMessage));
//		}
//		else
//		{
//			// rename to normal file
//			QFileInfo fileInfo(fCFG.fileName());
//			QString desiredFileName = fileInfo.fileName();
//			desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
////#if defined( WIN32 ) || defined( __OS2__ )
//			fileInfo.dir().remove(desiredFileName);
////#endif
//			if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName))
//			{
//				log(L_ERROR, "Can't rename file %s to %s (%s)", (const char*)fileInfo.fileName().toUtf8().data(), (const char*)desiredFileName.toUtf8().data(), fileInfo.dir().path().toUtf8().data());
//			}
//		}
//	}

//	// Saving profile-dependent config:
//	//setProfile(saveProfile);
//	cfgName = user_file(CLIENTS_CONF);
//	QFile f(QString(cfgName).append(BACKUP_SUFFIX)); // use backup file for this ...
//	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
//		log(L_ERROR, "Can't create %s", qPrintable(cfgName));
//	}else{
//		for (unsigned i = 0; i < getContacts()->nClients(); i++){
//			Client *client = getContacts()->getClient(i);
//			Protocol *protocol = client->protocol();
//			QByteArray line = "[";
//			line += QFile::encodeName(protocol->plugin()->name()).data();
//			line += '/';
//			line += protocol->description()->text.toUtf8();
//			line += "]\n";
//			f.write(line);
//			line = client->getConfig();
//			if (line.length()){
//				line += '\n';
//				f.write(line);
//			}
//		}
//		f.flush();  // Make shure that file is fully written and we will not get "Disk Full" error on f.close
//        const QFile::FileError status = f.error();
//		const QString errorMessage = f.errorString();
//		f.close();
//        if (status != QFile::NoError) {
//            log(L_ERROR, "IO error writing to file %s : %s", qPrintable(f.fileName()), qPrintable(errorMessage));
//		} else {
//			// rename to normal file
//			QFileInfo fileInfo(f.fileName());
//			QString desiredFileName = fileInfo.fileName();
//			desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
////#if defined( WIN32 ) || defined( __OS2__ )
//			fileInfo.dir().remove(desiredFileName);
////#endif
//			if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName))
//			{
//				log(L_ERROR, "Can't rename file %s to %s (%s)", (const char*)fileInfo.fileName().toUtf8().data(), (const char*)desiredFileName.toUtf8().data(), fileInfo.dir().path().toUtf8().data());
//			}
//		}
//	}

//#ifndef WIN32
//	QString dir = user_file("");
//	chmod(QFile::encodeName(dir),S_IRUSR | S_IWUSR | S_IXUSR);
//#endif
//	setValue("EditBackground", editBgColor);
//	setValue("EditForeground", editFgColor);
//	return QByteArray();
//}

//void CorePlugin::loadUnread()
//{
//	unread.clear();
//	QString unread_str = value("Unread").toString();
//	while (!unread_str.isEmpty()){
//		QString item = getToken(unread_str, ';');
//		unsigned long contact = getToken(item, ',').toULong();
//		unsigned long id = getToken(item, ',').toULong();
//		Message *msg = History::load(id, item, contact);
//		if (msg == NULL)
//			continue;
//		msg_id m;
//		m.id = id;
//		m.contact = contact;
//		m.client  = item;
//		m.type    = msg->baseType();
//		unread.push_back(m);
//	}
//	setValue("Unread", QString());
//}

//void CorePlugin::clearUnread(unsigned contact_id)
//{
//	for (list<msg_id>::iterator it = unread.begin(); it != unread.end();)
//    {
//		if (it->contact != contact_id)
//        {
//			++it;
//			continue;
//		}
//		unread.erase(it);
//		it = unread.begin();
//	}
//}

//Message *CorePlugin::createMessage(const char *type, Buffer *cfg)
//{
//	MAP_TYPES::iterator itt = types.find(type);
//    if (itt == types.end())
//        return new Message(MessageGeneric, cfg);

//    CommandDef *def = messageTypes.find(itt->second);
//    if (!def)
//        return new Message(MessageGeneric, cfg);

//    MessageDef *mdef = (MessageDef*)(def->param);
//    if (!mdef->create)
//        return new Message(MessageGeneric, cfg);

//    Message *msg = (mdef->create)(cfg);
//    if (msg)
//        return msg;
//	return new Message(MessageGeneric, cfg);
//}

//void CorePlugin::loadClients(const QString& profilename, SIM::ClientList& clients)
//{
//	QString cfgName = ProfileManager::instance()->rootPath() + QDir::separator() + profilename + QDir::separator() + "clients.conf";
//	QFile f(cfgName);
//	if (!f.open(QIODevice::ReadOnly))
//    {
//        log(L_ERROR, "[1]Can't open %s", qPrintable(cfgName));
//		return;
//	}
//	Buffer cfg = f.readAll();
//	for (;;)
//    {
//		QByteArray section = cfg.getSection();
//		if (section.isEmpty())
//			break;
//		QString s = section;	// ?
//		ClientPtr client = loadClient(s, &cfg);
//		if (client)
//			clients.push_back(client);
//	}
//}

//void CorePlugin::loadClients(SIM::ClientList &clients)
//{
//	loadClients(ProfileManager::instance()->currentProfileName(), clients);
//}

//ClientPtr CorePlugin::loadClient(const QString &name, Buffer *cfg)
//{
//	if (name.isEmpty())
//		return ClientPtr();
//	QString clientName = name;
//	QString pluginName = getToken(clientName, '/');
//    if (pluginName.isEmpty() || clientName.length() == 0)
//		return ClientPtr();
//	if(!getPluginManager()->isPluginProtocol(pluginName))
//    {
//        log(L_DEBUG, "Plugin %s is not a protocol plugin", qPrintable(pluginName));
//		return ClientPtr();
//	}
//	PluginPtr plugin = getPluginManager()->plugin(pluginName);
//	if(plugin.isNull())
//    {
//        log(L_WARN, "Plugin %s not found", qPrintable(pluginName));
//		return ClientPtr();
//	}
//	ProfileManager::instance()->currentProfile()->enablePlugin(pluginName);
//	ProtocolPtr protocol;
//	ProtocolIterator it;
//    while ((protocol = ++it) != NULL)
//        if (protocol->description()->text == clientName)
//            return protocol->createClient(cfg);
//    log(L_DEBUG, "Protocol %s not found", qPrintable(clientName));
//	return ClientPtr();
//}

//bool CorePlugin::adjustClientItem(unsigned id, CommandDef *cmd)
//{
//	unsigned n = id - CmdClient;
//	if (n >= getContacts()->nClients())
//		return false;
//	Client *client = getContacts()->getClient(n);
//	Protocol *protocol = client->protocol();
//	const CommandDef *descr = protocol->description();
//	cmd->icon		= descr->icon;
//	cmd->text_wrk   = clientName(client);
//	return true;
//}

//void CorePlugin::managerFinished()
//{
//	QTimer::singleShot(0, this, SLOT(destroyManager()));
//}

//void CorePlugin::destroyManager()
//{
//	if (m_manager){
//		delete m_manager;
//		m_manager = NULL;
//	}
//}

//QString CorePlugin::typeName(const QString &name)
//{
//	QString text = name;

//	text.remove('&');
//	if (!text.length())
//		log(L_DEBUG,"defText is empty!");
//	return text;
//}

//void CorePlugin::postInit()
//{
//    m_main->restoreGeometry(value("geometry").toByteArray());
//    m_main->restoreState(value("toolbar_state").toByteArray());
//}

//void CorePlugin::loadMenu()
//{
//    EventMenu(MenuConnections, EventMenu::eRemove).process();

//    unsigned nClients = getContacts()->nClients();

//    EventMenu(MenuConnections, EventMenu::eAdd).process();

//    Command cmd;
//    cmd->id          = CmdCM;
//    cmd->text        = I18N_NOOP("Connection manager");
//    cmd->menu_id     = MenuConnections;
//    cmd->menu_grp    = 0x8000;

//    EventCommandCreate(cmd).process();

//    cmd->id			= CmdShowPanel;
//    cmd->text		= I18N_NOOP("Show status panel");
//    cmd->menu_grp	= 0x8001;
//    cmd->flags		= COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    if (nClients >= 2)
//    {
//        cmd->id          = CmdConnections;
//        cmd->text        = I18N_NOOP("Connections");
//        cmd->menu_id     = MenuMain;
//        cmd->menu_grp    = 0x8040;
//        cmd->popup_id    = MenuConnections;
//        cmd->flags		 = COMMAND_DEFAULT;
//    }
//    else
//    {
//        cmd->id			= CmdConnections;
//        cmd->text		= I18N_NOOP("Connection manager");
//        cmd->menu_grp	= 0x8040;
//        cmd->menu_id	= MenuMain;
//        cmd->flags		= COMMAND_DEFAULT;
//    }
//    EventCommandCreate(cmd).process();

//    if (m_status == NULL)
//	    m_status = new CommonStatus;

//    for (unsigned i = 0; i < m_nClients; i++)
//        EventMenu(CmdClient + i, EventMenu::eRemove).process();

//    for (m_nClients = 0; m_nClients < getContacts()->nClients(); m_nClients++){
//        unsigned long menu_id = CmdClient + m_nClients;
//        EventMenu(menu_id, EventMenu::eAdd).process();
//        Client *client = getContacts()->getClient(m_nClients);
//        Protocol *protocol = client->protocol(); // FIXME there is no protocol, nirvana pointer :(
        
//        CommandDef *cmd = const_cast<CommandDef*>(protocol->statusList());
//        if (cmd){
//	        Command c;
//	        c->id       = CmdTitle;
//	        c->text     = "_";
//	        c->menu_id  = menu_id;
//	        c->menu_grp = 0x0001;
//	        c->flags    = COMMAND_CHECK_STATE | COMMAND_TITLE;
//	        EventCommandCreate(c).process();
//	        c->id		= CmdCommonStatus;
//	        c->text		= I18N_NOOP("Common status");
//	        c->menu_id	= menu_id;
//	        c->menu_grp = 0x3000;
//	        c->flags	= COMMAND_CHECK_STATE;
//	        EventCommandCreate(c).process();
//	        c->id		= CmdSetup;
//	        c->text		= I18N_NOOP("Configure client");
//	        c->icon		= "configure";
//	        c->menu_id	= menu_id;
//	        c->menu_grp = 0x3001;
//	        c->flags	= COMMAND_DEFAULT;
//	        EventCommandCreate(c).process();
//	        c->id		= menu_id;
//	        c->text		= "_";
//	        c->icon		= QString::null;
//	        c->menu_id	= MenuConnections;
//	        c->menu_grp	= 0x1000 + menu_id;
//	        c->popup_id	= menu_id;
//	        c->flags	= COMMAND_CHECK_STATE;
//	        EventCommandCreate(c).process();
//	        unsigned id = 0x100;
//	        //            for (; cmd->id; cmd++){
//	        for (; !cmd->text.isEmpty(); cmd++){
//		        c = *cmd;
//		        c->menu_id = menu_id;
//		        c->menu_grp = id++;
//		        c->flags = COMMAND_CHECK_STATE;
//		        EventCommandCreate(c).process();
//	        }
//	        if (protocol->description()->flags & PROTOCOL_INVISIBLE){
//		        c->id		= CmdInvisible;
//		        c->text		= I18N_NOOP("&Invisible");
//		        c->icon		= protocol->description()->icon_on;
//		        c->menu_grp	= 0x1000;
//		        c->flags	= COMMAND_CHECK_STATE;
//		        EventCommandCreate(c).process();
//	        }
//        }
//    }
//    showPanel();
//}

//void CorePlugin::showPanel()
//{
//    if (m_main == NULL)
//        return;
//    bool bShow = value("ShowPanel").toBool();
//    if (bShow){
//        if (getContacts()->nClients() < 2)
//            bShow = false;
//    }
//    if (bShow){
//        if (m_statusWnd == NULL)
//            m_statusWnd = new StatusWnd;
//        m_statusWnd->show();
//        return;
//    }
//    if (m_statusWnd){
//        delete m_statusWnd;
//        m_statusWnd = NULL;
//    }
//}


//unsigned CorePlugin::getContainerMode()
//{
//    return m_containerManager->containerMode();
//}

//void CorePlugin::setContainerMode(unsigned value)
//{
//    m_containerManager->setContainerMode((ContainerManager::ContainerMode)value);
//    emit modeChanged(value);
//}

//QString CorePlugin::clientName(Client *client)
//{
//    QString s = client->name();
//    QString res = i18n(getToken(s, '.'));
//    res += ' ';
//    return res + s;
//}

//void CorePlugin::checkHistory()
//{
//    Contact *contact;
//    ContactList::ContactIterator it;
//    while ((contact = ++it) != NULL)
//    {
//        SIM::PropertyHubPtr data = contact->getUserData("history");
//        if (data.isNull() || !data->value("CutDays").toBool())
//            continue;
//        QDateTime now(QDateTime::currentDateTime());
//        now = now.addSecs(-data->value("Days").toUInt() * 24 * 60 * 60);
//        History::cut(NULL, contact->id(), now.toTime_t());
//    }
//    QTimer::singleShot(24 * 60 * 60 * 1000, this, SLOT(checkHistory()));
//}

//void CorePlugin::setManualStatus(unsigned long status)
//{
//    if (status == getManualStatus())
//        return;
//    setValue("StatusTime", (unsigned int)QDateTime::currentDateTime().toTime_t());
//    //data.ManualStatus.asULong() = status;
//    setValue("ManualStatus", (uint)status);
//}

//void CorePlugin::alertFinished()
//{
//    if (m_alert)
//        setValue("NoJoinAlert", m_alert->isChecked());
//    m_alert = NULL;
//}

//void CorePlugin::focusDestroyed()
//{
//    m_focus = NULL;
//}

//bool CorePlugin::lockProfile(const QString &profile, bool bSend)
//{
//    if (profile.isEmpty())
//    {
//        if (!m_lock)
//            return true;

//        delete m_lock;
//        m_lock = NULL;
//        return true;
//    }
//    FileLock *lock = new FileLock(user_file(".lock"));
//    if (!lock->lock(bSend))
//    {
//        delete lock;
//        return false;
//    }
//    if (m_lock)
//        delete m_lock;
//    m_lock = lock;
//    return true;
//}

//void CorePlugin::showMain()
//{
//    if (m_main)
//    {
//        m_main->show();
//        raiseWindow(m_main);
//    }
//}

//#ifdef WIN32

//LockThread::LockThread(Qt::HANDLE _hEvent)
//{
//    hEvent = _hEvent;
//}

//void LockThread::run()
//{
//    for (;;)
//    {
//        DWORD res = WaitForSingleObject(hEvent, INFINITE);
//        if (res == WAIT_ABANDONED)
//            break;
//        QTimer::singleShot(0, CorePlugin::instance(), SLOT(showMain()));
//    }
//}

//// From zlib
//// Copyright (C) 1995-2002 Mark Adler

//#define BASE 65521L
//#define NMAX 5552

//#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
//#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
//#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
//#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
//#define DO16(buf)   DO8(buf,0); DO8(buf,8);

//unsigned adler32(const char *buf, unsigned len)
//{
//    unsigned long s1 = 0;
//    unsigned long s2 = 0;
//    int k;
//    while (len > 0) {
//        k = len < NMAX ? len : NMAX;
//        len -= k;
//        while (k >= 16) {
//            DO16(buf);
//            buf += 16;
//            k -= 16;
//        }
//        if (k != 0) do {
//            s1 += *buf++;
//            s2 += s1;
//        } while (--k);
//        s1 %= BASE;
//        s2 %= BASE;
//    }
//    return (s2 << 16) | s1;
//}

//#endif

//    FileLock::FileLock(const QString &name)
//: QFile(name)
//{
//#ifdef WIN32
//    m_thread = NULL;
//#else
//    m_bLock  = false;
//#endif
//}

//FileLock::~FileLock()
//{
//#ifdef WIN32
//    if (m_thread)
//    {
//        CloseHandle(m_thread->hEvent);
//        m_thread->wait(1000);
//        m_thread->terminate();
//        delete m_thread;
//    }
//#else
//    close();
//    if (m_bLock)
//        QFile::remove(fileName());
//#endif
//}

//#ifdef WIN32
//bool FileLock::lock(bool bSend)
//{
//    QString event = "SIM.";
//    const QByteArray s = fileName().toLocal8Bit();
//    event += QString::number(adler32(s.data(), s.length()));
//    Qt::HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event.toLatin1());
//    if (hEvent)
//    {
//        if (bSend)
//            SetEvent(hEvent);
//        CloseHandle(hEvent);
//        return false;
//    }
//    hEvent = CreateEventA(NULL, false, false, event.toLatin1());
//    if (hEvent == NULL)
//        return false;
//    m_thread = new LockThread(hEvent);
//    m_thread->start();
//#else
//    bool FileLock::lock(bool)
//    {
//        if (!open(QIODevice::ReadWrite | QIODevice::Truncate))
//        {
//            log(L_WARN, "Can't create %s", qPrintable(fileName()));
//            return false;
//        }
//        struct flock fl;
//        fl.l_type   = F_WRLCK;
//        fl.l_whence = SEEK_SET;
//        fl.l_start  = 0;
//        fl.l_len    = 1;
//        if (fcntl(handle(), F_SETLK, &fl) == -1)
//            //QFile::remove(name());
//            return false;
//        m_bLock = true;
//#endif
//        return true;
//    }

//void HistoryThread::run()
//{
//    QString str = user_file(".history_file");
//    History::save(m_id, str);
//    QProcess *m_ex;
//    m_ex = new QProcess();
//    m_ex->start(m_Viewer, QStringList(str));
//}

//CorePlugin* CorePlugin::instance()
//{
//    return g_plugin;
//}

//unsigned long CorePlugin::getManualStatus()
//{
//    return value("ManualStatus").toUInt();
//}

// vim: set expandtab:

