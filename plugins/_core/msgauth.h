/***************************************************************************
                          msgauth.h  -  description
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

#ifndef _MSGAUTH_H
#define _MSGAUTH_H

#include <QObject>
#include <QString>

#include "event.h"

class MsgEdit;

class MsgAuth : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgAuth(MsgEdit *parent, SIM::Message *msg);
protected slots:
    void init();
protected:
    virtual bool processEvent(SIM::Event*);
    QString     m_client;
    unsigned    m_type;
    MsgEdit    *m_edit;
};

#endif

