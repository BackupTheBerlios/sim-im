/***************************************************************************
                          workinfo.h  -  description
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

#ifndef _WORKINFO_H
#define _WORKINFO_H

#include "ui_workinfobase.h"
#include "event.h"

class ICQClient;
struct ICQUserData;

class WorkInfo : public QWidget, public Ui::WorkInfo, public SIM::EventReceiver
{
    Q_OBJECT
public:
    WorkInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void goUrl();
    void urlChanged(const QString&);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    ICQUserData *m_data;
    unsigned  m_contact;
    ICQClient *m_client;
};

#endif

