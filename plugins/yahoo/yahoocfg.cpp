/***************************************************************************
                          yahoocfg.cpp  -  description
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

#include <QTimer>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

#include "simgui/linklabel.h"
#include "misc.h"

#include "yahoocfg.h"
#include "yahooclient.h"

YahooConfig::YahooConfig(QWidget *parent, YahooClient *client, bool bConfig) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig)
        tabCfg->removeTab(tabCfg->indexOf(tabYahoo));
    QTimer::singleShot(0, this, SLOT(changed()));
    edtLogin->setText(m_client->getLogin());
    edtPassword->setText(m_client->getPassword());
    edtServer->setText(m_client->getServer());
    edtPort->setValue(m_client->getPort());
    edtMinPort->setValue(m_client->getMinPort());
    edtMaxPort->setValue(m_client->getMaxPort());
    connect(edtLogin, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPassword, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    lnkReg->setText(i18n("Get a Yahoo! ID"));
    lnkReg->setUrl("http://edit.yahoo.com/config/eval_register");
    chkHTTP->setChecked(m_client->getUseHTTP());
    chkAuto->setChecked(m_client->getAutoHTTP());
    connect(chkAuto, SIGNAL(toggled(bool)), this, SLOT(autoToggled(bool)));
    autoToggled(m_client->getAutoHTTP());
}

void YahooConfig::apply(SIM::Client*, void*)
{
}

void YahooConfig::apply()
{
    if (!m_bConfig){
        m_client->setLogin(edtLogin->text());
        m_client->setPassword(edtPassword->text());
    }
    m_client->setServer(edtServer->text());
    m_client->setPort(edtPort->text().toUShort());
    m_client->setMinPort(edtMinPort->text().toUShort());
    m_client->setMaxPort(edtMaxPort->text().toUShort());
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setAutoHTTP(chkAuto->isChecked());
}

void YahooConfig::autoToggled(bool bState)
{
    chkHTTP->setEnabled(!bState);
}

void YahooConfig::changed(const QString&)
{
    changed();
}

void YahooConfig::changed()
{
    emit okEnabled(!edtLogin->text().isEmpty() &&
                   !edtPassword->text().isEmpty() &&
                   !edtServer->text().isEmpty() &&
                   edtPort->text().toUShort());
}

