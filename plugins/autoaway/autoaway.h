/***************************************************************************
                          autoaway.h  -  description
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

#ifndef _AUTOAWAY_H
#define _AUTOAWAY_H

#include <QObject>

#include "cfg.h"
#include "event.h"
#include "plugins.h"

class QTimer;

struct AutoAwayData
{
    SIM::Data	AwayTime;
    SIM::Data	EnableAway;
    SIM::Data	NATime;
    SIM::Data	EnableNA;
    SIM::Data	OffTime;
    SIM::Data	EnableOff;
    SIM::Data	DisableAlert;
    SIM::Data	RealManualStatus;
};

class AutoAwayPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    AutoAwayPlugin(unsigned, Buffer*);
    ~AutoAwayPlugin();
    PROP_ULONG(AwayTime);
    PROP_BOOL(EnableAway);
    PROP_ULONG(NATime);
    PROP_BOOL(EnableNA);
    PROP_ULONG(OffTime);
    PROP_BOOL(EnableOff);
    PROP_BOOL(DisableAlert);
    PROP_ULONG(RealManualStatus);
protected slots:
    void timeout();
protected:
    virtual bool processEvent(SIM::Event*);
    virtual QByteArray getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    unsigned getIdleTime();
    bool bAway;
    bool bNA;
    bool bOff;
    QTimer *m_timer;
    AutoAwayData data;
};

#endif

