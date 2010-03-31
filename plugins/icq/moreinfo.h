/***************************************************************************
                          moreinfo.h  -  description
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

#ifndef _MOREINFO_H
#define _MOREINFO_H

#include "ui_moreinfobase.h"
#include "contacts/client.h"

#include "event.h"

class ICQClient;
class ICQUserData;

class MoreInfo : public QWidget, public Ui::MoreInfoBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MoreInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
    void goUrl();
    void urlChanged(const QString&);
    void setLang(int);
    void birthDayChanged();
protected:
    void updateData(ICQUserData* data);
    virtual bool processEvent(SIM::Event *e);
    void fill();
    ICQUserData *m_data;
    unsigned  m_contact;
    ICQClient *m_client;
};

#endif

