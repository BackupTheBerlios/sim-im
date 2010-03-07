/***************************************************************************
                          ontopcfg.h  -  description
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

#ifndef _ONTOPCFG_H
#define _ONTOPCFG_H

#include "ui_ontopcfgbase.h"

class OnTopPlugin;

class OnTopCfg : public QWidget, public Ui::OnTopCfgBase
{
    Q_OBJECT
public:
    OnTopCfg(QWidget *w, OnTopPlugin *plugin);
public slots:
    void apply();
protected:
    OnTopPlugin *m_plugin;
};

#endif

