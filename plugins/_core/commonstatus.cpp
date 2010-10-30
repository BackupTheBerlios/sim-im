/***************************************************************************
                          status.cpp  -  description
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
#include "commonstatus.h"

#include <map>
#include <list>

#include <QTimer>
#include <QApplication>
#include <QWidget>

#include "core.h"
#include "events/eventhub.h"

#include "contacts/client.h"
#include "clientmanager.h"
#include "commands/commandhub.h"
#include "commands/uicommand.h"
#include "contacts/protocol.h"

using namespace SIM;

CommonStatus::CommonStatus(SIM::ClientManager* manager) : QObject(), m_clientManager(manager)
{
    m_statusCmd = UiCommand::create(I18N_NOOP("Status"), "SIM_inactive", "common_status", QStringList("main_toolbar"));
    m_statusCmd->setWidgetType(UiCommand::wtCombobox);
    rebuildStatusList();
    getCommandHub()->registerCommand(m_statusCmd);

    getEventHub()->getEvent("init")->connectTo(this, SLOT(eventInit()));
}

CommonStatus::~CommonStatus()
{

}

void CommonStatus::rebuildStatusList()
{
    getCommandHub()->deleteCommandSet("common_states");
    CommandSetPtr states = getCommandHub()->createCommandSet("common_states");

    UiCommandPtr online = UiCommand::create("Online", "SIM_online", "common_status_online");
    online->setAutoExclusive(true);
    online->setCheckable(true);
    online->subscribeTo(this, SLOT(statusOnline()));
    states->appendCommand(online);

    UiCommandPtr freeForChat = UiCommand::create("Free for chat", "SIM_ffc", "common_status_ffc");
    freeForChat->setAutoExclusive(true);
    freeForChat->setCheckable(true);
    freeForChat->subscribeTo(this, SLOT(statusFreeForChat()));
    states->appendCommand(freeForChat);

    UiCommandPtr away = UiCommand::create("Away", "SIM_away", "common_status_away");
    away->setAutoExclusive(true);
    away->setCheckable(true);
    away->subscribeTo(this, SLOT(statusAway()));
    states->appendCommand(away);

    UiCommandPtr na = UiCommand::create("N/A", "SIM_na", "common_status_na");
    na->setAutoExclusive(true);
    na->setCheckable(true);
    na->subscribeTo(this, SLOT(statusNa()));
    states->appendCommand(na);

    UiCommandPtr dnd = UiCommand::create("Do not disturb", "SIM_dnd", "common_status_dnd");
    dnd->setAutoExclusive(true);
    dnd->setCheckable(true);
    dnd->subscribeTo(this, SLOT(statusDnd()));
    states->appendCommand(dnd);

    UiCommandPtr offline = UiCommand::create("Offline", "SIM_online", "common_status_offline");
    offline->setAutoExclusive(true);
    offline->setCheckable(true);
    offline->subscribeTo(this, SLOT(statusOffline()));
    states->appendCommand(offline);

    m_statusCmd->setSubcommands(states);
}

void CommonStatus::statusOnline()
{
    setCommonStatus("online");
}

void CommonStatus::statusFreeForChat()
{
    setCommonStatus("free_for_chat");
}

void CommonStatus::statusAway()
{
    setCommonStatus("away");
}

void CommonStatus::statusNa()
{
    setCommonStatus("na");
}

void CommonStatus::statusDnd()
{
    setCommonStatus("dnd");
}

void CommonStatus::statusOffline()
{
    setCommonStatus("offline");
}

void CommonStatus::setCommonStatus(const QString& id)
{
//    QList<ClientPtr> clients = m_clientManager->allClients();
//    foreach(const ClientPtr& client, clients)
//    {
//        Protocol* proto = client->protocol();
//        if(proto->states(SIM::Protocol::DefaultGroup).contains(id))
//        {
//            client->changeStatus(proto->status(id, SIM::Protocol::DefaultGroup), SIM::Protocol::DefaultGroup);
//        }
//    }
}

void CommonStatus::eventInit()
{
}
