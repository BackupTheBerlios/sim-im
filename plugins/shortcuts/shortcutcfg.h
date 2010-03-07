/***************************************************************************
                          shortcutcfg.h  -  description
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

#ifndef _SHORTCUTCFG_H
#define _SHORTCUTCFG_H

#include "ui_shortcutcfgbase.h"

class MouseConfig;
class ShortcutsPlugin;

class ShortcutsConfig : public QWidget, public Ui::ShortcutCfg
{
    Q_OBJECT
public:
    ShortcutsConfig(QWidget *parent, ShortcutsPlugin *plugin);
    ~ShortcutsConfig();
public slots:
    void apply();
protected slots:
    void selectionChanged();
    void keyChanged();
    void keyClear();
    void globalChanged(bool);
protected:
    virtual void resizeEvent(QResizeEvent*);
    void adjustColumns();
    void loadMenu(unsigned long id, bool bCanGlobal);
    void saveMenu(unsigned long id);
    ShortcutsPlugin	*m_plugin;
    MouseConfig		*mouse_cfg;
};

#endif

