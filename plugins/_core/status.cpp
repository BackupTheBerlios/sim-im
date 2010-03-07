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

#include <map>
#include <list>

#include <QTimer>
#include <QApplication>
#include <QWidget>

#include "unquot.h"
#include "status.h"
#include "core.h"
#include "statuswnd.h"
#include "logindlg.h"
#include "autoreply.h"

#include "contacts/client.h"
#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "clientmanager.h"
#include "simgui/ballonmsg.h"

using namespace std;
using namespace SIM;

typedef map<unsigned, unsigned> MAP_STATUS;

CommonStatus::CommonStatus()
        : EventReceiver(LowPriority + 2)
{
    m_bBlink  = false;
    m_timer   = NULL;
    m_balloon = NULL;

    EventMenu(MenuStatus, EventMenu::eAdd).process();

    Command cmd;
    cmd->id          = CmdStatusMenu;
    cmd->text        = I18N_NOOP("Status");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x6000;
    cmd->popup_id    = MenuStatus;
    cmd->flags		 = COMMAND_IMPORTANT;

    EventCommandCreate(cmd).process();

    m_bInitialized = false;
    rebuildStatus();
    QTimer::singleShot(500, this, SLOT(setBarStatus()));
}

CommonStatus::~CommonStatus()
{
    EventCommandRemove(CmdStatusBar).process();
    EventMenu(MenuStatus, EventMenu::eRemove).process();
}

void CommonStatus::setBarStatus()
{
    QString text = I18N_NOOP("Inactive");
    QString icon = "SIM_inactive";

    m_bConnected = false;
    bool bActive = getSocketFactory()->isActive();
	if (!bActive)
		for (unsigned i = 0; i < getContacts()->nClients(); i++)
		{
			Client *client = getContacts()->getClient(i);
			if (client->getState() != Client::Connected)
				continue;

			bActive = true;
			break;
		}

    if (bActive)
	{
        m_bConnected = false;
        for (unsigned i = 0; i < getContacts()->nClients(); i++)
		{
            Client *client = getContacts()->getClient(i);
            if (!client->getCommonStatus() || (client->getState() != Client::Connecting))
				continue;

			m_bConnected = true;
			break;
        }
        if (m_bConnected)
		{
            text = I18N_NOOP("Connecting");
            Client *client = getContacts()->getClient(0);
            Protocol *protocol = NULL;
            if (client)
                protocol = client->protocol();
            if (m_timer == NULL){
                m_timer = new QTimer(this);
                connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
                m_timer->start(1000);
                m_bBlink = false;
            }
            unsigned status;
            if (m_bBlink)
			{
                icon = "SIM_online";
                status = CorePlugin::instance()->getManualStatus();
            }
			else
			{
                icon = "SIM_offline";
                status = STATUS_OFFLINE;
            }
        }
		else
		{
            if (m_timer)
			{
                delete m_timer;
                m_timer = NULL;
            }
            unsigned status = CorePlugin::instance()->getManualStatus();
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (!client->getCommonStatus())
                    continue;
                if (client->getState() == Client::Error){
                    icon = "SIM_error";
                    text = I18N_NOOP("Error");
                    break;
                }
            }
            if (i >= getContacts()->nClients()){
                Client *client = getContacts()->getClient(0);
                if (client)
				{
                    const CommandDef *d;
                    unsigned i = getContacts()->nClients();
					if ((status == STATUS_ONLINE) && CorePlugin::instance()->value("Invisible").toBool())
						for (i = 0; i < getContacts()->nClients(); i++)
						{
							Client *client = getContacts()->getClient(i);
							if (!(client->protocol()->description()->flags & PROTOCOL_INVISIBLE))
								continue;

							icon = "SIM_invisible";
							text = I18N_NOOP("&Invisible");
							break;
						}
					if (i >= getContacts()->nClients())
					{
						for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++)
						{
							if (d->id != status)
								continue;

							if (status == STATUS_ONLINE)
								icon = "SIM_online";
							else if (status == STATUS_AWAY)
								icon = "SIM_away";
							else if (status == STATUS_NA)
								icon = "SIM_na";
							else if (status == STATUS_DND)
								icon = "SIM_dnd";
							else if (status == STATUS_OCCUPIED)
								icon = "SIM_occupied";
							else if (status == STATUS_FFC)
								icon = "SIM_ffc";
							else if (status == STATUS_OFFLINE)
								icon = "SIM_offline";
							text = d->text;
							break;
						}
					}
                }
            }
        }
    }

    Command cmd;
    cmd->id          = CmdStatusBar;
    cmd->text        = text;
    cmd->icon        = icon;
    cmd->bar_id      = ToolBarMain;
    cmd->bar_grp     = 0x6000;
    cmd->popup_id    = MenuStatus;
    cmd->flags		 = BTN_PICT;

	if (m_bInitialized)
		EventCommandChange(cmd).process();
	else
		EventCommandCreate(cmd).process();

    m_bInitialized = true;

    EventSetMainIcon(icon).process();
    EventSetMainText(text).process();
}

void CommonStatus::timeout()
{
    m_bBlink = !m_bBlink;
    setBarStatus();
}

void CommonStatus::rebuildStatus()
{
    MAP_STATUS status;
    unsigned nClients = getContacts()->nClients();
    if (nClients == 0)
        return;
    int nInvisible = -1;
    for (unsigned i = 0; i < nClients; i++){
        Client *client = getContacts()->getClient(i);
        for (const CommandDef *cmd = client->protocol()->statusList(); !cmd->text.isEmpty(); cmd++)
		{
            MAP_STATUS::iterator it = status.find(cmd->id);
			if (it == status.end())
				status.insert(MAP_STATUS::value_type(cmd->id, 1));
			else
				it->second++;
        }
		if (!(nInvisible == -1 && client->protocol()->description()->flags & PROTOCOL_INVISIBLE))
			continue;

		nInvisible = i;
    }
    if (nInvisible != -1)
	{
        Command cmd;
        cmd->id			= CmdInvisible;
        cmd->text		= I18N_NOOP("&Invisible");
        cmd->icon		= "SIM_invisible";
        cmd->menu_id	= MenuStatus;
        cmd->menu_grp	= 0x2000;
        cmd->flags		= COMMAND_CHECK_STATE;
        EventCommandCreate(cmd).process();
    }
    Client *client = getContacts()->getClient(0);
    unsigned id = 0x1000;
    unsigned long FirstStatus = 0;
    unsigned long ManualStatus = 0;
    for (const CommandDef *cmd = client->protocol()->statusList(); !cmd->text.isEmpty(); cmd++)
	{
        MAP_STATUS::iterator it = status.find(cmd->id);
        if (it == status.end())
            continue;
        if (it->second != nClients)
            continue;
        CommandDef c = *cmd;
        if (FirstStatus == 0)
            FirstStatus = cmd->id;
        if ((ManualStatus == 0) && (CorePlugin::instance()->getManualStatus() == cmd->id))
            ManualStatus = cmd->id;
		if (c.id == STATUS_ONLINE)
			c.icon = "SIM_online";
		else if (c.id == STATUS_AWAY)
			c.icon = "SIM_away";
		else if (c.id == STATUS_NA)
			c.icon = "SIM_na";
		else if (c.id == STATUS_DND)
			c.icon = "SIM_dnd";
		else if (c.id == STATUS_OCCUPIED)
			c.icon = "SIM_occupied";
		else if (c.id == STATUS_FFC)
			c.icon = "SIM_ffc";
		else if (c.id == STATUS_OFFLINE)
			c.icon = "SIM_offline";
        c.menu_id  = MenuStatus;
        c.menu_grp = id++;
        c.flags = COMMAND_CHECK_STATE;
        EventCommandCreate(&c).process();
    }
    if (ManualStatus == 0)
        ManualStatus = FirstStatus;
    CorePlugin::instance()->setManualStatus(ManualStatus);
    setBarStatus();
}

void CommonStatus::checkInvisible()
{
    bool bAllInvisible    = true;
    bool bAllNotInvisible = true;
    for (unsigned i = 0; i < getContacts()->nClients(); i++)
    {
        Client *client = getContacts()->getClient(i);
        if (!(client->protocol()->description()->flags & PROTOCOL_INVISIBLE))
            continue;

        if (client->getInvisible())
            bAllNotInvisible = false;
        else
            bAllInvisible = false;
    }

    if (bAllNotInvisible && bAllInvisible)
        return;

    if (bAllInvisible)
        CorePlugin::instance()->setValue("Invisible", true);
    if (bAllNotInvisible)
        CorePlugin::instance()->setValue("Invisible", false);
}

bool CommonStatus::processEvent(Event *e)
{
    switch (e->type())
	{
    case eEventClientChanged:
        checkInvisible();
        setBarStatus();
        break;
    case eEventShowNotification:
		{
            EventShowNotification *ee = static_cast<EventShowNotification*>(e);
            const EventNotification::ClientNotificationData &data = ee->data();
			for (list<BalloonItem>::iterator it = m_queue.begin(); it != m_queue.end(); ++it)
				if (it->id == data.id)
					return true;
            BalloonItem item;
            item.id     = data.id;
            item.client = data.client;
            item.text   = i18n(data.text);
			if (!data.args.isEmpty())
				if (item.text.indexOf("%1") >= 0)
					item.text = item.text.arg(data.args);
            QString title = "SIM";
			if (getContacts()->nClients() > 1)
				for (unsigned i = 0; i < getContacts()->nClients(); i++)
					if (getContacts()->getClient(i) == data.client)
					{
						title = data.client->name();
						int n = title.indexOf(".");
						if (n > 0)
							title = title.left(n) + ' ' + title.mid(n + 1);
						break;
					}
            item.text	= QString("<img src=\"sim:icons/%1\">&nbsp;<b><nobr>%2</nobr></b><br><center>")
                .arg((data.flags & EventNotification::ClientNotificationData::E_INFO) ? "info" : "error")
                        .arg(title) + quoteString(item.text) + "</center>";
			if (data.options.isEmpty())
				item.buttons.append(i18n("OK"));
			else {
				QStringList sl = data.options.split(QLatin1Char('\0'));
                                Q_FOREACH(const QString &s, sl)
					item.buttons.append(i18n(s));
                        }
            m_queue.push_back(item);
            if (m_balloon == NULL)
                showBalloon();
            break;
        }
    case eEventClientNotification:
		{
            EventClientNotification *ee = static_cast<EventClientNotification*>(e);
            const EventNotification::ClientNotificationData &data = ee->data();
            if (data.code == AuthError)
			{
                QString msg;
                if (!data.text.isEmpty())
                    msg = i18n(data.text).arg(data.args);
				ClientPtr client = SIM::getClientManager()->client(data.client->name());
                LoginDialog *loginDlg = new LoginDialog(false, client, msg, NULL);
                raiseWindow(loginDlg);
            }
			else
			{
                EventShowNotification eShow(data);
                eShow.process();
            }
            return true;
        }
    case eEventClientStatus:
    case eEventSocketActive:
    case eEventInit:
        setBarStatus();
        break;
    case eEventClientsChanged:
		{
            bool bCommon = false;
            for (unsigned i = 0; i < getContacts()->nClients(); i++)
			{
                Client *client = getContacts()->getClient(i);
                if (client->getCommonStatus())
                    bCommon = true;
            }
            if (!bCommon)
			{
                Client *client = getContacts()->getClient(0);
                if (client)
				{
                    client->setCommonStatus(true);
                    EventClientChanged(client).process();
                }
            }
            checkInvisible();
            rebuildStatus();
            break;
        }
    case eEventCheckCommandState:
		{
            EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
            CommandDef *def = ecs->cmd();
            if (def->menu_id == MenuStatus)
			{
                if (def->id == CmdInvisible)
				{
					if (CorePlugin::instance()->value("Invisible").toBool())
						def->flags |= COMMAND_CHECKED;
					else
						def->flags &= ~COMMAND_CHECKED;
                    return true;
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
				for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++)
					if (d->id == def->id)
						curStatus = d;
                if (curStatus == NULL)
                    return 0;
                bool bChecked = false;
                unsigned status = CorePlugin::instance()->getManualStatus();
                bChecked = (status == curStatus->id);

				if (bChecked)
					def->flags |= COMMAND_CHECKED;
				else
					def->flags &= ~COMMAND_CHECKED;
                return true;
            }
            return 0;
        }
    case eEventCommandExec:{
            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
            CommandDef *def = ece->cmd();
            if (def->menu_id == MenuStatus)
			{
                if (def->id == CmdInvisible)
				{
                    CorePlugin::instance()->setValue("Invisible", !CorePlugin::instance()->value("Invisible").toBool());
					for (unsigned i = 0; i < getContacts()->nClients(); i++)
						getContacts()->getClient(i)->setInvisible(CorePlugin::instance()->value("Invisible").toBool());
                    return true;
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
				for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++)
					if (d->id == def->id)
						curStatus = d;
                if (curStatus == NULL)
                    return false;
                bool bOfflineStatus = false;
                for (unsigned i = 0; i < getContacts()->nClients(); i++)
				{
                    Client *client = getContacts()->getClient(i);
					if (!(client->getCommonStatus() && client->protocol()->description()->flags & PROTOCOL_AR_OFFLINE))
						continue;

					bOfflineStatus = true;
					break;
                }

				if (bOfflineStatus || 
					(def->id != STATUS_ONLINE && def->id != STATUS_OFFLINE)){
                    QString noShow = CorePlugin::instance()->propertyHub()->stringMapValue("NoShowAutoReply", def->id);
                    if (noShow.isEmpty())
					{
                        AutoReplyDialog dlg(def->id);
                        if (!dlg.exec())
                            return true;
                    }
                }
                CorePlugin::instance()->setManualStatus(def->id);
                for (unsigned i = 0; i < getContacts()->nClients(); i++)
				{
                    Client *client = getContacts()->getClient(i);
                    if (client->getCommonStatus())
                        client->setStatus(def->id, true);
                }
                return true;
            }
            break;
        }
    default:
        break;
    }
    return false;
}

void CommonStatus::showBalloon()
{
    if (m_balloon || m_queue.empty())
        return;
    Command cmd;
    cmd->id = CmdStatusBar;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    QWidget *widget = eWidget.widget();
    if (widget == NULL){
        m_queue.erase(m_queue.begin());
        return;
    }
    BalloonItem &item = m_queue.front();
    if (CorePlugin::instance()->m_statusWnd)
        m_balloon = CorePlugin::instance()->m_statusWnd->showError(item.text, item.buttons, item.client);
    if (m_balloon == NULL)
        m_balloon = new BalloonMsg(NULL, item.text, item.buttons, widget);
    connect(m_balloon, SIGNAL(yes_action(void*)), this, SLOT(yes_action(void*)));
    connect(m_balloon, SIGNAL(finished()), this, SLOT(finished()));
    raiseWindow(widget->topLevelWidget());
    m_balloon->show();
}

void CommonStatus::yes_action(void*)
{
    if (!m_queue.empty() && m_balloon)
	{
        m_balloon->hide();
        BalloonItem &item = m_queue.front();
        Command cmd;
        cmd->id    = item.id;
        cmd->param = item.client;
        EventCommandExec(cmd).process();
    }
}

void CommonStatus::finished()
{
    m_balloon = NULL;
    if (!m_queue.empty())
        m_queue.erase(m_queue.begin());
    QTimer::singleShot(1000, this, SLOT(showBalloon()));
}

