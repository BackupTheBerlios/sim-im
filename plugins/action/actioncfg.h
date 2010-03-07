/***************************************************************************
                          actioncfg.h  -  description
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

#ifndef _ACTIONCFG_H
#define _ACTIONCFG_H

#include "ui_actioncfgbase.h"
#include "propertyhub.h"

using namespace SIM;

struct ActionUserData;
class ActionPlugin;
class MenuConfig;

class ActionConfig : public QWidget, public Ui::ActionConfigBase
{
    Q_OBJECT
public:
    ActionConfig(QWidget *parent, SIM::PropertyHubPtr data, ActionPlugin *plugin);
    virtual ~ActionConfig();
public slots:
    void apply();
    void apply(PropertyHubPtr);
    void help();
protected:
    MenuConfig		*m_menu;
    void setEnabled (bool);
    SIM::PropertyHubPtr m_data;
    ActionPlugin *m_plugin;
};

#endif

