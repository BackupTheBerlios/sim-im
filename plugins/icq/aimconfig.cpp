/***************************************************************************
                          aimconfig.cpp  -  description
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

#include "aimconfig.h"
#include "icq.h"
#include "simgui/linklabel.h"

#include <QTimer>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QValidator>
#include <QTabWidget>
#include <QComboBox>

using namespace SIM;

AIMConfig::AIMConfig(QWidget *parent, ICQClient *client, bool bConfig) : QDialog(parent)
{
	setupUi(this);
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig){
        QTimer::singleShot(0, this, SLOT(changed()));
        edtScreen->setText(m_client->data.owner.getScreen());
        edtPasswd->setText(m_client->getPassword());
        connect(edtScreen, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        lnkReg->setText(i18n("Register new ScreenName"));
        lnkReg->setUrl("http://my.screenname.aol.com/_cqr/login/login.psp?siteId=aimregistrationPROD&authLev=1&mcState=initialized&createSn=1&triedAimAuth=y");
    }else{
        tabConfig->removeTab(tabConfig->indexOf(tabAIM));
    }
    edtServer->setText(m_client->getServer());
    edtPort->setValue(m_client->getPort());
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    chkHTTP->setChecked(client->getUseHTTP());
    connect(chkAuto, SIGNAL(toggled(bool)), this, SLOT(autoToggled(bool)));
    chkAuto->setChecked(client->getAutoHTTP());
    chkKeepAlive->setChecked(client->getKeepAlive());
}

void AIMConfig::autoToggled(bool bState)
{
    chkHTTP->setEnabled(!bState);
}

void AIMConfig::apply(Client*, void*)
{
}

void AIMConfig::applyContact(const SIM::ClientPtr&, SIM::IMContact*)
{

}

void AIMConfig::apply()
{
    if (m_bConfig){
        m_client->setScreen(edtScreen->text().toLower());
        m_client->setPassword(edtPasswd->text());
    }
    m_client->setServer(edtServer->text());
    m_client->setPort(edtPort->text().toUShort());
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setAutoHTTP(chkAuto->isChecked());
    m_client->setKeepAlive(chkKeepAlive->isChecked());
}

void AIMConfig::changed(const QString&)
{
    changed();
}

void AIMConfig::changed()
{
    bool bOK = true;
    bOK =  !edtScreen->text().isEmpty() &&
           !edtPasswd->text().isEmpty() &&
           !edtServer->text().isEmpty() &&
           edtPort->text().toUShort();
    emit okEnabled(bOK);
}

