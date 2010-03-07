/***************************************************************************
                          yahooinfo.h  -  description
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

#ifndef _YAHOOINFO_H
#define _YAHOOINFO_H

#include "ui_yahooinfobase.h"
#include "event.h"

class YahooClient;
struct YahooUserData;

class YahooInfo : public QWidget, public Ui::YahooInfoBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    YahooInfo(QWidget *parent, YahooUserData *data, YahooClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    YahooUserData *m_data;
    YahooClient *m_client;
};


#endif

