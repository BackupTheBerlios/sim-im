/***************************************************************************
                          icqconfig.h  -  description
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

#ifndef _ICQCONFIG_H
#define _ICQCONFIG_H

#include "ui_icqconfigbase.h"
#include "icqclient.h"

class ICQConfig : public QWidget, public Ui::ICQConfigBase
{
    Q_OBJECT
public:
    ICQConfig(QWidget *parent, ICQClient *client, bool bConfig);
    CorePlugin *core;
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected slots:
    void changed();
    void changed(const QString&);
    void newToggled(bool);
    void autoToggled(bool);
    void invisibleToggled(bool);
protected:
    bool m_bConfig;
    ICQClient *m_client;
};

#endif

