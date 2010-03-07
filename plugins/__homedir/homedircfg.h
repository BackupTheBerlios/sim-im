/***************************************************************************
                          homedircfg.h  -  description
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

#ifndef _HOMEDIRCFG_H
#define _HOMEDIRCFG_H

#include "ui_homedircfgbase.h"

class HomeDirPlugin;

class HomeDirConfig : public QWidget, public Ui::HomeDirConfigBase
{
    Q_OBJECT
public:
    HomeDirConfig(QWidget *parent, HomeDirPlugin *plugin);
public slots:
    void apply();
protected slots:
    void defaultToggled(bool);
protected:
    HomeDirPlugin *m_plugin;
};

#endif

