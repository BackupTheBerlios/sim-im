/***************************************************************************
                          about.h  -  description
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

#ifndef _ABOUT_H
#define _ABOUT_H

#include <QObject>

#include "event.h"
#include "plugins.h"

class QWidget;

class AboutPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    AboutPlugin(unsigned);
    virtual ~AboutPlugin();
protected slots:
    void aboutDestroyed();
    void realDestroy();
protected:
    virtual bool processEvent(SIM::Event*);
    unsigned long CmdBugReport;
    unsigned long CmdAbout;
    QWidget *about;
#ifdef USE_KDE
    unsigned CmdAboutKDE;
    QWidget *about_kde;
#endif
};

#endif

