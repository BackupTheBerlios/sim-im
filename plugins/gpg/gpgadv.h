/***************************************************************************
                          gpgadv.h  -  description
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

#ifndef _GPGADV_H
#define _GPGADV_H

#include "ui_gpgadvbase.h"

class GpgPlugin;

class GpgAdvanced : public QWidget, public Ui::GpgAdvanced
{
    Q_OBJECT
public:
    GpgAdvanced(QWidget *parent, GpgPlugin *plugin);
    ~GpgAdvanced();
public slots:
    void apply();
protected:
    GpgPlugin *m_plugin;
};

#endif

