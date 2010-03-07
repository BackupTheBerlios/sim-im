/***************************************************************************
                          filecfg.h  -  description
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

#ifndef _FILECFG_H
#define _FILECFG_H

#include "propertyhub.h"
#include "ui_filecfgbase.h"

class QCheckBox;

class FileConfig : public QWidget, public Ui::FileConfigBase
{
    Q_OBJECT
public:
    FileConfig(QWidget *parent, SIM::PropertyHubPtr data);
public slots:
    void apply(SIM::PropertyHubPtr);
};

#endif

