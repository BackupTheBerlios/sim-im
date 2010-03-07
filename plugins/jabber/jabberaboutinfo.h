/***************************************************************************
                          jabberaboutinfo.h  -  description
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

#ifndef _JABBERABOUTINFO_H
#define _JABBERABOUTINFO_H

#include "ui_jabberaboutinfobase.h"
#include "event.h"


struct JabberUserData;
class JabberClient;

class JabberAboutInfo : public QWidget, public Ui::JabberAboutInfo, public SIM::EventReceiver
{
    Q_OBJECT
public:
    JabberAboutInfo(QWidget *parent, JabberUserData *data, JabberClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill(JabberUserData *data);
    JabberUserData *m_data;
    JabberClient *m_client;
};

#endif

