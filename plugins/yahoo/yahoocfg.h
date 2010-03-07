/***************************************************************************
                          yahoocfg.h  -  description
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

#ifndef _YAHOOCFG_H
#define _YAHOOCFG_H

#include "contacts.h"

#include "ui_yahoocfgbase.h"

class YahooClient;

class YahooConfig : public QWidget, public Ui::YahooConfigBase
{
    Q_OBJECT
public:
    YahooConfig(QWidget *parent, YahooClient *client, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected slots:
    void changed();
    void changed(const QString&);
    void autoToggled(bool);
protected:
    bool m_bConfig;
    YahooClient *m_client;
};

#endif

