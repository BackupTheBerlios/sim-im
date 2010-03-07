/***************************************************************************
                          msgurl.h  -  description
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

#ifndef _MSGURL_H
#define _MSGURL_H

#include <QObject>
#include <QString>

#include "event.h"

class MsgEdit;

class MsgUrl : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgUrl(MsgEdit *parent, SIM::Message *msg);
protected slots:
    void init();
    void urlChanged(const QString&);
protected:
    virtual bool processEvent(SIM::Event*);
    QString     m_client;
    MsgEdit    *m_edit;
};

#endif

