/***************************************************************************
                          forwardcfg.h  -  description
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

#ifndef _FORWARDCFG_H
#define _FORWARDCFG_H

#include "ui_forwardcfgbase.h"
#include "propertyhub.h"

using namespace SIM;
class ForwardPlugin;

class ForwardConfig : public QWidget, public Ui::ForwardConfigBase
{
    Q_OBJECT
public:
    ForwardConfig(QWidget *parent, PropertyHubPtr data, ForwardPlugin *plugin);
public slots:
    void apply(PropertyHubPtr data);
    void apply();
protected:
    ForwardPlugin	*m_plugin;
};

#endif

