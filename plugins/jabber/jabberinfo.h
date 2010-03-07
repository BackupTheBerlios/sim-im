/***************************************************************************
                          jabberinfo.h  -  description
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

#ifndef _JABBERINFO_H
#define _JABBERINFO_H

#include "ui_jabberinfobase.h"
#include "event.h"

struct JabberUserData;
class JabberClient;

class JabberInfo : public QWidget, public Ui::JabberInfo, public SIM::EventReceiver
{
    Q_OBJECT
public:
    JabberInfo(QWidget *parent, JabberUserData *data, JabberClient *client);
signals:
    void raise(QWidget*);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void goUrl();
    void urlChanged(const QString&);
    void resourceActivated(int);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    JabberUserData *m_data;
    JabberClient *m_client;
};

#endif

