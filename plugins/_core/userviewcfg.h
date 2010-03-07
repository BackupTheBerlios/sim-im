/***************************************************************************
                          userviewcfg.h  -  description
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

#ifndef _USERVIEWCFG_H
#define _USERVIEWCFG_H

#include "simgui/qcolorbutton.h"
#include "ui_userviewcfgbase.h"

class CorePlugin;

class UserViewConfig : public QWidget, public Ui::ContactList
{
    Q_OBJECT
public:
    UserViewConfig(QWidget *parent);
    ~UserViewConfig();
public slots:
    void apply();
    void colorsToggled(bool);
    void sortChanged(int);
    void setFonts(bool);
protected:
    void fillBox(QComboBox*);
    void setSortMode(unsigned);
    unsigned getSortMode();
};

#endif

