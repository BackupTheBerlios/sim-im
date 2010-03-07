/***************************************************************************
                          msggen.h  -  description
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

#ifndef _MSGGEN_H
#define _MSGGEN_H

#include "event.h"

#include <QObject>
#include <QString>

class MsgEdit;

class MsgGen : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgGen(MsgEdit *parent, SIM::Message *msg);
protected slots:
    void init();
    void emptyChanged(bool bEmpty);
protected:
    virtual bool processEvent(SIM::Event*);
    QString     m_client;
    bool        m_bCanSend;
    MsgEdit    *m_edit;
};

#endif

