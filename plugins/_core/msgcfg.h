/***************************************************************************
                          msgcfg.h  -  description
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

#ifndef _MSGCFG_H
#define _MSGCFG_H

#include "propertyhub.h"
#include "ui_msgcfgbase.h"

class FileConfig;

class MessageConfig : public QWidget, public Ui::MessageConfigBase
{
    Q_OBJECT
public:
    MessageConfig(QWidget *parent, SIM::PropertyHubPtr data);
public slots:
    void apply(SIM::PropertyHubPtr);
protected:
    FileConfig *m_file;
};

#endif

