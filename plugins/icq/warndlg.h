/***************************************************************************
                          warndlg.h  -  description
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

#ifndef _WARNDLG_H
#define _WARNDLG_H

#include "ui_warndlgbase.h"
#include "icqclient.h"

class WarningMessage;

class WarnDlg : public QDialog, public Ui::WarnDlgBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    WarnDlg(QWidget *parent, ICQUserData *data, ICQClient *client);
    ~WarnDlg();
protected:
    void accept();
    virtual bool processEvent(SIM::Event *e);
    void showError(const char*);
    ICQClient		*m_client;
    ICQUserData		*m_data;
    unsigned		m_contact;
    WarningMessage	*m_msg;
};

#endif

