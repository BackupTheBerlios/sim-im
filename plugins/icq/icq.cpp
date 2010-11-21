/***************************************************************************
                          icq.cpp  -  description
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

#include "icq.h"
#include "core.h"
#include "log.h"
#include "clientmanager.h"
#include "imagestorage/imagestorage.h"
#include "contacts/protocolmanager.h"
#include "icqclient.h"

using namespace SIM;

Plugin *createICQPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new ICQPlugin();
    return plugin;
}

Plugin *createICQPluginObject()
{
    return new ICQPlugin();
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        PLUGIN_PROTOCOL,
        createICQPluginObject
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

ICQProtocol::ICQProtocol(Plugin *plugin)
        : Protocol(plugin)
{
    initStatuses();
}

ICQProtocol::~ICQProtocol()
{
}

QString ICQProtocol::name()
{
    return "ICQ";
}

QString ICQProtocol::iconId()
{
    return "ICQ";
}

SIM::ClientPtr ICQProtocol::createClient(const QString& name)
{
    ClientPtr icq = ClientPtr(new ICQClient(this, name, false));
    return icq;
}

void ICQProtocol::initStatuses()
{
//    m_states.clear();
//    addStatus(ICQStatusPtr(new ICQStatus("online", "Online", true, "", getImageStorage()->icon("ICQ_online"))));
//    addStatus(ICQStatusPtr(new ICQStatus("away", "Away", true, "", getImageStorage()->icon("ICQ_away"))));
//    addStatus(ICQStatusPtr(new ICQStatus("n/a", "N/A", true, "", getImageStorage()->icon("ICQ_na"))));
//    addStatus(ICQStatusPtr(new ICQStatus("dnd", "Do not disturb", true, "", getImageStorage()->icon("ICQ_dnd"))));
//    addStatus(ICQStatusPtr(new ICQStatus("occupied", "Occupied", true, "", getImageStorage()->icon("ICQ_occupied"))));
//    addStatus(ICQStatusPtr(new ICQStatus("free_for_chat", "Free for chat", true, "", getImageStorage()->icon("ICQ_ffc"))));
//    addStatus(ICQStatusPtr(new ICQStatus("offline", "Offline", true, "", getImageStorage()->icon("ICQ_offline"))));
}

void ICQProtocol::addStatus(ICQStatusPtr status)
{
    m_states.append(status);
}

//AIMProtocol::AIMProtocol(Plugin *plugin)
//        : Protocol(plugin)
//{
//}

//AIMProtocol::~AIMProtocol()
//{
//}

//QStringList AIMProtocol::states()
//{
//	// TODO
//	return QStringList();
//}

//SIM::IMStatusPtr AIMProtocol::status(const QString& /*id*/)
//{
//	// TODO
//	return SIM::IMStatusPtr();
//}

//IMContact* AIMProtocol::createIMContact(const QSharedPointer<Client>& client)
//{
//    return new ICQUserData(client);
//}

//ClientPtr AIMProtocol::createClient(Buffer *cfg)
//{
//	ClientPtr aim = ClientPtr(new ICQClient(this, cfg, true));
//	getClientManager()->addClient(aim);
//	return aim;
//}

//SIM::ClientPtr AIMProtocol::createClient(const QString& name)
//{
//    ClientPtr aim = ClientPtr(new ICQClient(this, 0, true));
//    getClientManager()->addClient(aim);
//    return aim;
//}

//Protocol *ICQPlugin::m_icq = NULL;
//Protocol *ICQPlugin::m_aim = NULL;

ICQPlugin *ICQPlugin::icq_plugin = NULL;

ICQPlugin::ICQPlugin()
        : Plugin()
{
    icq_plugin = this;

    m_icq = ProtocolPtr(new ICQProtocol(this));
	getProtocolManager()->addProtocol(m_icq);
//    m_aim = ProtocolPtr(new AIMProtocol(this));
//    getProtocolManager()->addProtocol(m_aim);

//    EventMenu(MenuSearchResult, EventMenu::eAdd).process();
//    EventMenu(MenuIcqGroups, EventMenu::eAdd).process();

//    Command cmd;
//    cmd->id          = CmdVisibleList;
//    cmd->text        = I18N_NOOP("Visible list");
//    cmd->menu_id     = MenuContactGroup;
//    cmd->menu_grp    = 0x8010;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdInvisibleList;
//    cmd->text		 = I18N_NOOP("Invisible list");
//    cmd->menu_grp	 = 0x8011;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdIcqSendMessage;
//    cmd->text		 = I18N_NOOP("&Message");
//    cmd->icon		 = "message";
//    cmd->menu_id	 = MenuSearchResult;
//    cmd->menu_grp	 = 0x1000;
//    cmd->bar_id		 = 0;
//    cmd->popup_id	 = 0;
//    cmd->flags		 = COMMAND_DEFAULT;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdInfo;
//    cmd->text		 = I18N_NOOP("User &info");
//    cmd->icon		 = "info";
//    cmd->menu_grp	 = 0x1001;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdGroups;
//    cmd->text		 = I18N_NOOP("&Add to group");
//    cmd->icon		 = QString::null;
//    cmd->menu_grp	 = 0x1002;
//    cmd->popup_id	 = MenuIcqGroups;
//    EventCommandCreate(cmd).process();

//    cmd->id			 = CmdGroups;
//    cmd->text		 = "_";
//    cmd->menu_id	 = MenuIcqGroups;
//    cmd->flags		 = COMMAND_CHECK_STATE;
//    EventCommandCreate(cmd).process();

//    registerMessages();

//    RetrySendDND = registerType();
//    RetrySendOccupied = registerType();
}

ICQPlugin::~ICQPlugin()
{
    //unregisterMessages();
    getProtocolManager()->removeProtocol(m_aim);
    getProtocolManager()->removeProtocol(m_icq);

//    getContacts()->removePacketType(OscarPacket);
//    getContacts()->removePacketType(ICQDirectPacket);
//    getContacts()->removePacketType(AIMDirectPacket);

//    EventCommandRemove(CmdVisibleList).process();
//    EventCommandRemove(CmdInvisibleList).process();

//    EventMenu(MenuSearchResult, EventMenu::eRemove).process();
//    EventMenu(MenuIcqGroups, EventMenu::eRemove).process();
}

