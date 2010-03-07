/**************************************************************************
                          smssetup.cpp  -  description
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

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimer>
#include <QLabel>
#include <QTabWidget>

#include "misc.h"

#include "sms.h"
#include "smssetup.h"
#include "serial.h"

using namespace SIM;

SMSSetup::SMSSetup(QWidget *parent, SMSClient *client) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    QStringList res = SerialPort::devices();
    unsigned n = 0;
    unsigned cur = 0;
    if (m_client->getState() == Client::Connected){
        cmbPort->insertItem(INT_MAX,m_client->getDevice());
        cur = 0;
        n++;
    }
    for (QStringList::Iterator it = res.begin(); it != res.end(); ++it, n++){
        if ((*it) == m_client->getDevice())
            cur = cmbPort->count();
        cmbPort->insertItem(INT_MAX,*it);
    }
    cmbPort->setCurrentIndex(cur);
    for (unsigned i = 0; i < (unsigned)(cmbBaud->count()); i++){
        if (cmbBaud->itemText(i).toULong() == m_client->getBaudRate()){
            cmbBaud->setCurrentIndex(i);
        }
    }
    chkXonXoff->setChecked(m_client->getXonXoff());
    if (client->getState() == Client::Connected){
        if (client->getCharging()){
            lblCharge->setText(i18n("Charging:"));
        }else{
            lblCharge->setText(i18n("Battery:"));
        }
        barCharge->setValue(client->getCharge());
        barQuality->setValue(client->getQuality());
        edtModel->setReadOnly(true);
        edtModel->setText(client->model());
        edtOper->setText(client->oper());
    }else{
        tabSMS->removeTab(tabSMS->indexOf(tabPhone));
    }
    QTimer::singleShot(0, this, SLOT(init()));
}

void SMSSetup::apply()
{
    m_client->setDevice(cmbPort->currentText());
    m_client->setBaudRate(cmbBaud->currentText().toULong());
    m_client->setXonXoff(chkXonXoff->isChecked());
}

void SMSSetup::apply(Client*, void*)
{
}

void SMSSetup::init()
{
    emit okEnabled(true);
}

