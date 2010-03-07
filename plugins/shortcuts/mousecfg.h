/***************************************************************************
                          mousecfg.h  -  description
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

#ifndef _MOUSECFG_H
#define _MOUSECFG_H

#include "ui_mousecfgbase.h"

class ShortcutsPlugin;

class MouseConfig : public QWidget, public Ui::MouseCfg
{
    Q_OBJECT
public:
    MouseConfig(QWidget *parent, ShortcutsPlugin *plugin);
    ~MouseConfig();
public slots:
    void apply();
protected slots:
    void selectionChanged();
    void buttonChanged(int);
    void changed(bool);
protected:
    ShortcutsPlugin *m_plugin;
    void resizeEvent(QResizeEvent *e);
    void adjustColumns();
    void loadMenu(unsigned long id);
};

#endif

