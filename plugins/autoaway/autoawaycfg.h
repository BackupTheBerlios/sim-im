/***************************************************************************
                          autoawaycfg.h  -  description
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

#ifndef _AUTOAWAYCFG_H
#define _AUTOAWAYCFG_H

#include "ui_autoawaycfgbase.h"

class AutoAwayPlugin;

class AutoAwayConfig : public QWidget, public Ui::AutoAwayConfigBase
{
    Q_OBJECT
public:
    AutoAwayConfig(QWidget *parent, AutoAwayPlugin*);
public slots:
    void apply();
    void toggledAway(bool);
    void toggledNA(bool);
    void toggledOff(bool);
protected:
    AutoAwayPlugin *m_plugin;
};

#endif

