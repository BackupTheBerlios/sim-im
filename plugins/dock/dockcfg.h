/***************************************************************************
                          dockcfg.h  -  description
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

#ifndef _DOCKCFG_H
#define _DOCKCFG_H

#include "ui_dockcfgbase.h"

class DockPlugin;

class DockCfg :  public QWidget, public Ui::DockCfgBase
{
    Q_OBJECT
public:
    DockCfg(QWidget *w, DockPlugin *plugin);
public slots:
    void apply();
    void autoHideToggled(bool);
    void customize();
protected:
    DockPlugin *m_plugin;
};

#endif

