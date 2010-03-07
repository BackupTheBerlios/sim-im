/***************************************************************************
                          securedlg.h  -  description
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

#ifndef _SECUREDLG_H
#define _SECUREDLG_H

#include "ui_securedlgbase.h"
#include "event.h"

class ICQClient;
struct ICQUserData;

class SecureDlg : public QDialog, public Ui::SecureDlgBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    SecureDlg(ICQClient *client, unsigned contact, ICQUserData *data);
    ~SecureDlg();
    ICQClient	*m_client;
    unsigned	m_contact;
    ICQUserData *m_data;
protected slots:
    void start();
protected:
    virtual bool processEvent(SIM::Event *e);
    void error(const QString &err);
    SIM::Message *m_msg;
};

#endif

