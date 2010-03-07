/***************************************************************************
                          msgcontacts.h  -  description
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

#ifndef _MSGCONTACTS_H
#define _MSGCONTACTS_H

#include <QObject>
#include <QString>

#include "event.h"

class MsgEdit;
class UserList;

class MsgContacts : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgContacts(MsgEdit *btn, SIM::Message *msg);
    ~MsgContacts();
protected slots:
    void init();
    void changed();
    void editFinished();
    void listFinished();
protected:
    virtual bool processEvent(SIM::Event*);
    UserList   *m_list;
    MsgEdit    *m_edit;
    QString     m_client;
};

#endif

