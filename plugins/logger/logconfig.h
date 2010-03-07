/***************************************************************************
                          logconfig.h  -  description
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

#ifndef _LOGCONFIG_H
#define _LOGCONFIG_H

#include "event.h"

#include "ui_logconfigbase.h"

class LoggerPlugin;

class LogConfig : public QWidget, public Ui::LogConfigBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    LogConfig(QWidget *parent, LoggerPlugin *plugin);
public slots:
    void apply();
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    LoggerPlugin	*m_plugin;
};

#endif

