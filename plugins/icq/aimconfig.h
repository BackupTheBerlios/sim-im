/***************************************************************************
                          aimconfig.h  -  description
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

#ifndef _AIMCONFIG_H
#define _AIMCONFIG_H

#include "ui_aimconfigbase.h"
#include "icqclient.h"

class AIMConfig : public QDialog, public Ui::AIMConfigBase
{
    Q_OBJECT
public:
    AIMConfig(QWidget *parent, ICQClient *client, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
protected slots:
    void changed();
    void changed(const QString&);
    void autoToggled(bool);
protected:
    bool m_bConfig;
    ICQClient *m_client;
};

#endif

