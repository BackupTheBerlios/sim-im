/***************************************************************************
                          remotecfg.h  -  description
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

#ifndef _REMOTECFG_H
#define _REMOTECFG_H

#include "ui_remotecfgbase.h"

class RemotePlugin;

class RemoteConfig : public QWidget, public Ui::RemoteCfg
{
    Q_OBJECT
public:
    RemoteConfig(QWidget *parent, RemotePlugin*);
public slots:
    void apply();
    void selected(int);
    void toggled(bool);
protected:
    RemotePlugin *m_plugin;
};

#endif

