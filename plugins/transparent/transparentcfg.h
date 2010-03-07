/***************************************************************************
                          transparentcfg.h  -  description
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

#ifndef _TRANSPARENTCFG_H
#define _TRANSPARENTCFG_H

#include "ui_transparentcfgbase.h"

class TransparentPlugin;

class TransparentCfg : public QWidget, public Ui::TransparentCfgBase
{
    Q_OBJECT
public:
    TransparentCfg(QWidget *parent, TransparentPlugin *plugin);
public slots:
    void apply();
protected:
    TransparentPlugin *m_plugin;
};

#endif

