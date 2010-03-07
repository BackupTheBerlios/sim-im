/***************************************************************************
  netmonitor.cpp  -  description
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
#include <QWidget>

#include "misc.h"
#include "core_consts.h"

#include "profile.h"
#include "profilemanager.h"

#include "netmonitor.h"
#include "monitor.h"

using namespace std;
using namespace SIM;

Plugin *createNetmonitorPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new NetmonitorPlugin(base, config);
    return plugin;
}

static PluginInfo info =
{
    I18N_NOOP("Network monitor"),
    I18N_NOOP("Plugin provides monitoring of net and messages\n"
            "For show monitor on start run sim -m"),
    VERSION,
    createNetmonitorPlugin,
    PLUGIN_DEFAULT
};

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

//static DataDef monitorData[] =
//{
    //        { "LogLevel", DATA_ULONG, 1, DATA(7) },
    //        { "LogPackets", DATA_STRING, 1, 0 },
//    { "Geometry", DATA_LONG, 5, DATA(-1) },
//    { "Show", DATA_BOOL, 1, 0 },
//    { NULL, DATA_UNKNOWN, 0, 0 }
//};

NetmonitorPlugin::NetmonitorPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
    , monitor(NULL)
{
    m_propertyHub = SIM::PropertyHub::create("netmonitor");

    const QStringList packets = value("LogPackets").toString().split(',');
    Q_FOREACH( const QString &v, packets)
        setLogType(v.toULong(), true);
    
    CmdNetMonitor = registerType();

    Command cmd;
    cmd->id          = CmdNetMonitor;
    cmd->text        = I18N_NOOP("Network monitor");
    cmd->icon        = "network";
    cmd->bar_id      = ToolBarMain;
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x8000;
    cmd->flags		= COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    EventArg e("-m", I18N_NOOP("Show network monitor"));
    if (e.process() || value("Show").toBool())
        showMonitor();
}

NetmonitorPlugin::~NetmonitorPlugin()
{
    saveState();
    setValue("Show", monitor != NULL);
    QString packets;
    QSetIterator<unsigned> it(m_packets);
    while (it.hasNext()) {
        if (packets.length())
            packets += ',';
        packets += QString::number(it.next());
    }
    setValue("LogPackets", packets);
    EventCommandRemove(CmdNetMonitor).process();

    delete monitor;

}

QByteArray NetmonitorPlugin::getConfig()
{
    return QByteArray();
}

bool NetmonitorPlugin::isLogType(unsigned id)
{
    return ( m_packets.contains( id ) );
}

void NetmonitorPlugin::setLogType(unsigned id, bool bLog)
{
    if (bLog){
        m_packets.insert(id);
    }else{
        m_packets.remove(id);
    }
}

const unsigned NO_DATA = (unsigned)(-1);

void NetmonitorPlugin::showMonitor()
{
    if (monitor == NULL)
    {
        monitor = new MonitorWindow(this);
        //bool bPos = (data.geometry[LEFT].toLong() != NO_DATA) && (data.geometry[TOP].toLong() != NO_DATA);
        //bool bSize = (data.geometry[WIDTH].toLong() != NO_DATA) && (data.geometry[HEIGHT].toLong() != NO_DATA);
        //restoreGeometry(monitor, data.geometry, bPos, bSize);
        connect(monitor, SIGNAL(finished()), this, SLOT(finished()));
    }
    raiseWindow(monitor);
}

bool NetmonitorPlugin::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdNetMonitor){
            showMonitor();
            return true;
        }
    }
    else if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("netmonitor");
        if(!hub.isNull())
            setPropertyHub(hub);
    }
    return false;
}

void NetmonitorPlugin::finished()
{
    saveState();
    QTimer::singleShot(0, this, SLOT(realFinished()));
}

void NetmonitorPlugin::realFinished()
{
    delete monitor; //Fixme: Crash
    monitor = NULL;
}

void NetmonitorPlugin::saveState()
{
    if (monitor == NULL)
        return;
    //saveGeometry(monitor, data.geometry);
}

void NetmonitorPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr NetmonitorPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant NetmonitorPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void NetmonitorPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}
