/***************************************************************************
                          action.h  -  description
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

#ifndef _ACTION_H
#define _ACTION_H

#include <QObject>
#include <QList>
#include <QProcess>

#include "cfg.h"
#include "event.h"
#include "plugins.h"

class QProcess;

struct ActionUserData
{
    SIM::Data	OnLine;
    SIM::Data	Status;
    SIM::Data	Message;
    SIM::Data	Menu;
    SIM::Data	NMenu;
};

class ActionPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ActionPlugin(unsigned);
    virtual ~ActionPlugin();
    unsigned long action_data_id;
protected slots:
    void ready(int, QProcess::ExitStatus);
    void msg_ready(int, QProcess::ExitStatus);
protected:
    unsigned long CmdAction;
    virtual bool processEvent(SIM::Event*);
    QWidget *createConfigWindow(QWidget *parent);
};

#endif

