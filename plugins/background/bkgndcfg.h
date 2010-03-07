/***************************************************************************
                          bkgndcfg.h  -  description
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

#ifndef _BKGNDCFG_H
#define _BKGNDCFG_H

#include "ui_bkgndcfgbase.h"

class BackgroundPlugin;

class BkgndCfg : public QWidget, public Ui::BkgndCfgBase
{
    Q_OBJECT
public:
    BkgndCfg(QWidget *parent, BackgroundPlugin *plugin);
public slots:
    void apply();
protected:
    BackgroundPlugin *m_plugin;
};

#endif

