/***************************************************************************
                          netmonitor.h  -  description
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

#ifndef _NETMONITOR_H
#define _NETMONITOR_H

#include <QObject>
#include <QSet>

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

struct NetMonitorData
{
//    SIM::Data	LogLevel;
//    SIM::Data	LogPackets;
//    SIM::Data	geometry[5];
//    SIM::Data	Show;
};

class MonitorWindow;

class NetmonitorPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    NetmonitorPlugin(unsigned, Buffer *name);
    virtual ~NetmonitorPlugin();
//    PROP_ULONG(LogLevel);
//    PROP_STR(LogPackets);
//    PROP_BOOL(Show);
    bool isLogType(unsigned id);
    void setLogType(unsigned id, bool bLog);
        
    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

protected slots:
    void finished();
    void realFinished();

protected:
    virtual bool processEvent(SIM::Event *e);
    virtual QByteArray getConfig();
    void showMonitor();
    void saveState();
    unsigned long CmdNetMonitor;
    QSet<unsigned> m_packets;
    MonitorWindow *monitor;

private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif

