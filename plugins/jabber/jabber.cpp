/***************************************************************************
                          jabber.cpp  -  description
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

#include "jabberclient.h"
#include "jabber.h"
#include "misc.h"
#include "contacts/protocolmanager.h"
#include "clientmanager.h"

#include <QByteArray>

using namespace SIM;

Plugin *createJabberPluginObject()
{
    Plugin *plugin = new JabberPlugin();
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        PLUGIN_PROTOCOL,
        createJabberPluginObject
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

JabberProtocol::JabberProtocol(Plugin *plugin)
        : Protocol(plugin)
{
    //initStatuses();
}

JabberProtocol::~JabberProtocol()
{
}

//void JabberProtocol::initStatuses()
//{
//    addStatus(JabberStatusPtr(new JabberStatus("online", "Online", true, "", Icon("Jabber_online"), QString::null, QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("away", "Away", true, "", Icon("Jabber_away"), "away", QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("n/a", "N/A", true, "", Icon("Jabber_na"), "xa", QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("dnd", "Do not disturb", true, "", Icon("Jabber_dnd"), "dnd", QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("occupied", "Occupied", true, "", Icon("Jabber_occupied"), "occupied", QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("free_for_chat", "Free for chat", true, "", Icon("Jabber_ffc"), "chat", QString::null)));
//    addStatus(JabberStatusPtr(new JabberStatus("offline", "Offline", true, "", Icon("Jabber_offline"), QString::null, "unavailable")));
//}

//void JabberProtocol::addStatus(JabberStatusPtr status)
//{
//    m_states.append(status);
//}

QString JabberProtocol::name()
{
    return "Jabber";
}

QString JabberProtocol::iconId()
{
    return "Jabber";
}

SIM::ClientPtr JabberProtocol::createClient(const QString& name)
{
    ClientPtr jabber = ClientPtr(new JabberClient(this, name));
    return jabber;
}

//static CommandDef jabber_descr =
//    CommandDef (
//        0,
//        I18N_NOOP("Jabber"),
//        "Jabber_online",
//        "Jabber_invisible",
//        QString::null,
//        0,
//        0,
//        0,
//        0,
//        0,
//        PROTOCOL_INFO | PROTOCOL_AR | PROTOCOL_INVISIBLE | PROTOCOL_SEARCH | PROTOCOL_AR_OFFLINE,
//        NULL,
//        QString::null
//    );

//const CommandDef *JabberProtocol::description()
//{
//    return &jabber_descr;
//}

//static CommandDef jabber_status_list[] =
//    {
//        CommandDef (
//            STATUS_ONLINE,
//            I18N_NOOP("Online"),
//            "Jabber_online",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            STATUS_AWAY,
//            I18N_NOOP("Away"),
//            "Jabber_away",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            STATUS_NA,
//            I18N_NOOP("N/A"),
//            "Jabber_na",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            STATUS_DND,
//            I18N_NOOP("Do not Disturb"),
//            "Jabber_dnd",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//		CommandDef (
//            STATUS_OCCUPIED,
//            I18N_NOOP("Occupied"),
//            "Jabber_occupied",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            STATUS_FFC,
//            I18N_NOOP("Free for chat"),
//            "Jabber_ffc",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef (
//            STATUS_OFFLINE,
//            I18N_NOOP("Offline"),
//            "Jabber_offline",
//            QString::null,
//            QString::null,
//            0,
//            0,
//            0,
//            0,
//            0,
//            0,
//            NULL,
//            QString::null
//        ),
//        CommandDef ()
//    };

//const CommandDef *JabberProtocol::statusList()
//{
//    return jabber_status_list;
//}

//DataDef jabberData[] =
//    {
//        { "BrowserBar", DATA_LONG, 7, 0 },
//        { "BrowserHistory", DATA_UTF, 1, 0 },
//        { "AllLevels", DATA_BOOL, 1, 0 },
//        { "BrowseType", DATA_ULONG, 1, DATA(BROWSE_DISCO | BROWSE_BROWSE | BROWSE_AGENTS) },
//        { NULL, DATA_UNKNOWN, 0, 0 }
//    };

JabberPlugin *JabberPlugin::plugin = NULL;

JabberPlugin::JabberPlugin() : Plugin()
{
    plugin = this;
//    load_data(jabberData, &data, cfg);
//    JabberPacket = registerType();
//    getContacts()->addPacketType(JabberPacket, jabber_descr.text, true);

//    EventMenu(MenuSearchResult, EventMenu::eAdd).process();
//    EventMenu(MenuJabberGroups, EventMenu::eAdd).process();
//    EventMenu(MenuBrowser, EventMenu::eAdd).process();

//    EventToolbar(BarBrowser, EventToolbar::eAdd).process();

//    Command	cmd;
//    cmd->id			 = CmdJabberMessage;
//    cmd->text		 = I18N_NOOP("&Message");
//    cmd->icon		 = "message";
//    cmd->menu_id	 = MenuSearchResult;
//    cmd->menu_grp	 = 0x1000;
//    cmd->flags		 = COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBrowseInfo;
//    cmd->text		 = I18N_NOOP("User &info");
//    cmd->icon		 = "info";
//    cmd->menu_grp	 = 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdGroups;
//    cmd->text		 = I18N_NOOP("&Add to group");
//    cmd->icon		 = QString::null;
//    cmd->menu_grp	 = 0x1002;
//    cmd->popup_id	 = MenuJabberGroups;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdGroups;
//    cmd->text		 = "_";
//    cmd->menu_id	 = MenuJabberGroups;
//    cmd->popup_id	 = 0;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBack;
//    cmd->text		 = I18N_NOOP("&Back");
//    cmd->icon		 = "1leftarrow";
//    cmd->bar_grp	 = 0x1000;
//    cmd->bar_id		 = BarBrowser;
//    cmd->menu_id	 = 0;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdForward;
//    cmd->text		 = I18N_NOOP("&Next");
//    cmd->icon		 = "1rightarrow";
//    cmd->bar_grp	 = 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdUrl;
//    cmd->text		 = I18N_NOOP("JID");
//    cmd->icon		 = "run";
//    cmd->bar_grp	 = 0x2000;
//    cmd->flags		 = BTN_COMBO_CHECK;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdNode;
//    cmd->text		 = I18N_NOOP("Node");
//    cmd->bar_grp	 = 0x2001;
//    cmd->flags		 = BTN_COMBO | BTN_NO_BUTTON;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBrowseSearch;
//    cmd->text		 = I18N_NOOP("&Search");
//    cmd->icon		 = "find";
//    cmd->bar_grp	 = 0x3000;
//    cmd->menu_id	 = MenuSearchOptions;
//    cmd->menu_grp	 = 0x2000;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdRegister;
//    cmd->text		 = I18N_NOOP("&Register");
//    cmd->icon		 = "reg";
//    cmd->bar_grp	 = 0x3001;
//    cmd->menu_grp	 = 0x2001;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBrowseInfo;
//    cmd->text		 = I18N_NOOP("Info");
//    cmd->icon		 = "info";
//    cmd->menu_id	 = 0;
//    cmd->menu_grp	 = 0;
//    cmd->bar_grp	 = 0x3010;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBrowseConfigure;
//    cmd->text		 = I18N_NOOP("Configure");
//    cmd->icon		 = "configure";
//    cmd->bar_grp	 = 0x3020;
//    cmd->menu_id	 = MenuSearchOptions;
//    cmd->menu_grp	 = 0x2002;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdBrowseMode;
//    cmd->text		 = I18N_NOOP("Browser mode");
//    cmd->icon		 = "configure";
//    cmd->bar_grp	 = 0x5000;
//    cmd->menu_id	 = 0;
//    cmd->menu_grp	 = 0;
//    cmd->flags		 = COMMAND_DEFAULT;
//    cmd->popup_id	 = MenuBrowser;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdOneLevel;
//    cmd->text		 = I18N_NOOP("Load one level");
//    cmd->icon		 = QString::null;
//    cmd->bar_id		 = 0;
//    cmd->bar_grp	 = 0;
//    cmd->menu_id	 = MenuBrowser;
//    cmd->menu_grp	 = 0x1000;
//    cmd->popup_id	 = 0;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdAllLevels;
//    cmd->text		 = I18N_NOOP("Load all levels");
//    cmd->menu_grp	 = 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdModeDisco;
//    cmd->text		 = "Discovery";
//    cmd->menu_grp	 = 0x2000;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdModeBrowse;
//    cmd->text		 = "Browse";
//    cmd->menu_grp	 = 0x2001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdModeAgents;
//    cmd->text		 = "Agents";
//    cmd->menu_grp	 = 0x2002;
//    EventCommandCreate(cmd).process();

    m_protocol = ProtocolPtr(new JabberProtocol(this));
	getProtocolManager()->addProtocol(m_protocol);
}

JabberPlugin::~JabberPlugin()
{
    getProtocolManager()->removeProtocol(m_protocol);
}

// vim: set expandtab:

