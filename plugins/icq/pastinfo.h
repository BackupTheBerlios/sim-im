/***************************************************************************
                          pastinfo.h  -  description
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

#ifndef _PASTINFO_H
#define _PASTINFO_H

#include "country.h"
#include "event.h"

#include "ui_pastinfobase.h"

class ICQClient;
struct ICQUserData;

class PastInfo : public QWidget, public Ui::PastInfoBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    PastInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void cmbAfChanged(int);
    void cmbBgChanged(int);
protected:
    virtual bool processEvent(SIM::Event *e);
    QString getInfo(QComboBox *cmb, QLineEdit *edt, const SIM::ext_info*);
    void fill();
    ICQUserData *m_data;
    unsigned  m_contact;
    ICQClient *m_client;
};

#endif

