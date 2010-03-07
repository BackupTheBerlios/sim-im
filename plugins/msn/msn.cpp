/***************************************************************************
                          msn.cpp  -  description
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
#include "misc.h"

#include "msn.h"
#include "msnclient.h"


using namespace SIM;

Plugin *createMSNPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new MSNPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createMSNPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

MSNProtocol::MSNProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

MSNProtocol::~MSNProtocol()
{
}

ClientPtr MSNProtocol::createClient(Buffer *cfg)
{
	ClientPtr msn = ClientPtr(new MSNClient(this, cfg));
	getClientManager()->addClient(msn);
    return msn;
}

static CommandDef msn_descr =
    CommandDef (
        0,
        I18N_NOOP("MSN"),
        "MSN_online",
        "MSN_invisible",
        I18N_NOOP("https://accountservices.passport.net/uiresetpw.srf?lc=1033"),
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INVISIBLE,
        NULL,
        QString::null
    );

const CommandDef *MSNProtocol::description()
{
    return &msn_descr;
}

static CommandDef msn_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "MSN_online",
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
            "MSN_away",
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
            "MSN_na",
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
            "MSN_dnd",
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
            STATUS_BRB,
            I18N_NOOP("Be right back"),
            "MSN_onback",
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
            STATUS_PHONE,
            I18N_NOOP("On the phone"),
            "MSN_onphone",
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
            STATUS_LUNCH,
            I18N_NOOP("On the lunch"),
            "MSN_lunch",
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
            "MSN_offline",
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

const CommandDef *MSNProtocol::statusList()
{
    return msn_status_list;
}

MSNPlugin::MSNPlugin(unsigned base)
        : Plugin(base)
{
    MSNPacket = registerType();
    EventAddOk   = registerType();
    EventAddFail = registerType();
    MSNInitMail  = registerType();
    MSNNewMail   = registerType();

    getContacts()->addPacketType(MSNPacket, msn_descr.text, true);

    m_protocol = new MSNProtocol(this);
}

MSNPlugin::~MSNPlugin()
{
    getContacts()->removePacketType(MSNPacket);
    delete m_protocol;
}
