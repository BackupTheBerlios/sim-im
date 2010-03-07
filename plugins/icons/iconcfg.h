/***************************************************************************
                          iconcfg.h  -  description
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

#ifndef _ICONCFG_H
#define _ICONCFG_H


#include "ui_iconcfgbase.h"

class IconsPlugin;

class IconCfg : public QWidget, public Ui::IconCfgBase
{
    Q_OBJECT
public:
    IconCfg(QWidget *parent, IconsPlugin *plugin);
public slots:
    void apply();
    void up();
    void down();
    void add();
    void remove();
    void itemSelectionChanged();
protected:
    IconsPlugin *m_plugin;
};

#endif

