/***************************************************************************
                          plugincfg.h  -  description
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

#ifndef _PLUGINCFG_H
#define _PLUGINCFG_H

#include "plugins.h"
#include "ui_plugincfgbase.h"

class PluginCfg : public QWidget, public Ui::PluginCfgBase
{
    Q_OBJECT
public:
    PluginCfg(QWidget *parent, const QString& pluginname);
public slots:
    void apply();
protected:
    SIM::PluginInfo *m_info;
	QString m_pluginName;
};

#endif

