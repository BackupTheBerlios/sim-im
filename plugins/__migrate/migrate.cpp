/***************************************************************************
                          migrate.cpp  -  description
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

#include <QDir>

#include "misc.h"

#include "migrate.h"
#include "migratedlg.h"

using namespace SIM;

Plugin *createMigratePlugin(unsigned base, bool, Buffer*)
{
    MigratePlugin *plugin = new MigratePlugin(base);
    if (!plugin->init()){
        delete plugin;
        return NULL;
    }
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Migrate"),
        I18N_NOOP("Plugin provides convert configuration and history from SIM 0.8"),
        VERSION,
        createMigratePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

MigratePlugin::MigratePlugin(unsigned base)
        : Plugin(base)
{
}

MigratePlugin::~MigratePlugin()
{
}

bool MigratePlugin::init()
{
    QString dir = user_file(QString::null);
    QDir d(dir);
    if (!d.exists())
        return false;
    QStringList cnvDirs;
    QStringList dirs = d.entryList(QDir::Dirs);
    QStringList::Iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it){
        if ((*it)[0] == '.')
            continue;
        QString p = dir + (*it);
        p += '/';
        QFile icqConf(p + "icq.conf");
        QFile clientsConf(p + "clients.conf");
        if (icqConf.exists() && !clientsConf.exists()){
            cnvDirs.append(*it);
        }
    }
    if (cnvDirs.count() == 0)
        return false;
    MigrateDialog dlg(dir, cnvDirs);
    dlg.exec();
    return true;
}

