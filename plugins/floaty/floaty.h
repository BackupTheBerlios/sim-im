/***************************************************************************
                          floaty.h  -  description
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

#ifndef _FLOATY_H
#define _FLOATY_H

#include <QObject>
#include <QPoint>
#include <QHash>

#include "cfg.h"
#include "event.h"
#include "plugins.h"

class QTimer;

struct FloatyUserData
{
    SIM::Data	X;
    SIM::Data	Y;
};

class FloatyWnd;
class QTimer;

class FloatyPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    FloatyPlugin(unsigned);
    virtual ~FloatyPlugin();
    bool m_bBlink;
    void startBlink();
protected slots:
    void showPopup();
    void unreadBlink();
protected:
    virtual bool processEvent(SIM::Event *e);
    unsigned long CmdFloaty;
    unsigned long user_data_id;
    QPoint	 popupPos;
    QTimer	 *unreadTimer;
    unsigned long popupId;
    QHash<unsigned long, FloatyWnd*> m_floaties;
    friend class FloatyWnd;
};

#endif

