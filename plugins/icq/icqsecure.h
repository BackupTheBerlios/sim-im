/***************************************************************************
                          icqsecure.h  -  description
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

#ifndef _ICQSECURE_H
#define _ICQSECURE_H

#include "cfg.h"
#include "contacts/client.h"

#include "simgui/listview.h"
#include "ui_icqsecurebase.h"

class ICQClient;
class ICQUserData;

class ICQSecure : public QWidget, public Ui::Secure, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ICQSecure(QWidget *parent, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
    void hideIpToggled(bool);
    void deleteVisibleItem(QTreeWidgetItem *item);
    void deleteInvisibleItem(QTreeWidgetItem *item);
protected:
    void updateData(ICQUserData* data);
    virtual bool processEvent(SIM::Event *e);
    void fill();
    void setListView(QTreeWidget*);
    void fillListView(QTreeWidget *lst, SIM::Data ICQUserData::* field);
    void fillListView(QTreeWidget *lst, int v);
    ICQClient	*m_client;
};

#endif

