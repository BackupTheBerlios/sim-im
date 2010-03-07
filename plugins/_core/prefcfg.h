/***************************************************************************
                          prefcfg.h  -  description
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

#ifndef _PREFCFG_H
#define _PREFCFG_H

#include "cmddef.h"
#include "contacts.h"
#include "ui_prefcfgbase.h"

class PrefConfig : public QWidget, public Ui::PrefConfigBase
{
    Q_OBJECT
public:
    PrefConfig(QWidget *parent, SIM::CommandDef *cmd, SIM::Contact *contact, SIM::Group *group);

signals:
    void apply(SIM::PropertyHubPtr);
    void apply(SIM::PropertyHubPtr, bool);

public slots:
    void apply();

protected slots:
    void overrideToggled(bool);

protected:
    SIM::CommandDef	*m_cmd;
    SIM::Contact	*m_contact;
    SIM::Group		*m_group;
};

#endif

