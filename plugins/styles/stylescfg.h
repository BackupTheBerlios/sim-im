/***************************************************************************
                          stylescfg.h  -  description
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

#ifndef _STYLESCFG_H
#define _STYLESCFG_H

#include "ui_stylescfgbase.h"

class StylesPlugin;

class FontConfig;

class StylesConfig : public QWidget, public Ui::StylesConfigBase
{
    Q_OBJECT
public:
    StylesConfig(QWidget *parent, StylesPlugin*);
    ~StylesConfig();
public slots:
    void apply();
protected:
    FontConfig   *font_cfg;
    StylesPlugin *m_plugin;
};

#endif

