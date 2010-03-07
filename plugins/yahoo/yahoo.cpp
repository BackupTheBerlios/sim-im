/***************************************************************************
                          yahoo.cpp  -  description
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

#include "clientmanager.h"
#include "yahoo.h"
#include "yahooclient.h"
#include "core.h"

using namespace SIM;

Plugin *createYahooPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new YahooPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createYahooPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

unsigned YahooPlugin::YahooPacket = 0;

YahooPlugin::YahooPlugin(unsigned base)
        : Plugin(base)
{
    YahooPacket = registerType();
    getContacts()->addPacketType(YahooPacket, "Yahoo!");
    registerMessages();
    m_protocol = new YahooProtocol(this);
}

YahooPlugin::~YahooPlugin()
{
    delete m_protocol;
    unregisterMessages();
    getContacts()->removePacketType(YahooPacket);
}

YahooProtocol::YahooProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

YahooProtocol::~YahooProtocol()
{
}

ClientPtr YahooProtocol::createClient(Buffer *cfg)
{
	ClientPtr yahoo = ClientPtr(new YahooClient(this, cfg));
	getClientManager()->addClient(yahoo);
    return yahoo;
}

static CommandDef yahoo_descr =
    CommandDef (
        0,
        I18N_NOOP("Yahoo!"),
        "Yahoo!_online",
        "Yahoo!_invisible",
        "http://edit.yahoo.com/config/eval_forgot_pw?.src=pg&.done=http://messenger.yahoo.com/&.redir_from=MESSENGER",
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INVISIBLE,
        NULL,
        QString::null
    );

const CommandDef *YahooProtocol::description()
{
    return &yahoo_descr;
}

static CommandDef yahoo_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
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
            STATUS_AWAY,
            I18N_NOOP("Away"),
            "Yahoo!_away",
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
            STATUS_NA,
            I18N_NOOP("N/A"),
            "Yahoo!_na",
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
            STATUS_DND,
            I18N_NOOP("Busy"),
            "Yahoo!_dnd",
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
            "Yahoo!_offline",
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

const CommandDef *YahooProtocol::statusList()
{
    return yahoo_status_list;
}
