/***************************************************************************
                          icqconfig.cpp  -  description
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

#include "icqconfig.h"
#include "icq.h"

#include <QTimer>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QValidator>
#include <QTabWidget>
#include <QComboBox>

using namespace SIM;

ICQConfig::ICQConfig(QWidget *parent, ICQClient *client, bool bConfig)
    : QWidget(parent)
{
    setupUi(this);
    m_client = client;
    m_bConfig = bConfig;

    if (m_bConfig){
        QTimer::singleShot(0, this, SLOT(changed()));
        connect(chkNew, SIGNAL(toggled(bool)), this, SLOT(newToggled(bool)));
        if (m_client->data.owner.Uin.toULong()){
            edtUin->setText(QString::number(m_client->data.owner.Uin.toULong()));
            chkNew->setChecked(false);
            edtPasswd->setText(m_client->getPassword());
/*         }else if(core->getRegNew()) {
 *             edtUin->setText(core->getICQUIN());
 *             edtPasswd->setText(core->getICQPassword());
 */
        }else{
            chkNew->setChecked(true);
        }
    //FIXME: we will have to do something when UIN is greater than signed int
        edtUin->setValidator(new QIntValidator(1000, 0x7FFFFFFF, edtUin));  
        connect(edtUin, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    }else{
        tabConfig->removeTab(tabConfig->indexOf(tabICQ));
    }
    edtServer->setText(m_client->getServer());
    edtPort->setValue(m_client->getPort());
    edtMinPort->setValue(m_client->getMinPort());
    edtMaxPort->setValue(m_client->getMaxPort());
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    cmbFormat->addItem(i18n("RTF"));
    cmbFormat->addItem(i18n("UTF"));
    cmbFormat->addItem(i18n("Plain text"));
    cmbFormat->setCurrentIndex(client->getSendFormat());
    chkPlugins->setChecked(client->getDisablePlugins());
    chkUpdate->setChecked(client->getDisableAutoUpdate());
    chkAutoReply->setChecked(client->getDisableAutoReplyUpdate());
    chkTyping->setChecked(client->getDisableTypingNotification());
    chkInvisible->hide();
    edtInvisible->hide();
    lblInvisible->hide();
    lblInvisible2->hide();
    chkDND->setChecked(client->getAcceptInDND());
    chkOccupied->setChecked(client->getAcceptInOccupied());
    chkHTTP->setChecked(client->getUseHTTP());
    connect(chkAuto, SIGNAL(toggled(bool)), this, SLOT(autoToggled(bool)));
    connect(chkInvisible, SIGNAL(toggled(bool)), this, SLOT(invisibleToggled(bool)));
    chkAuto->setChecked(client->getAutoHTTP());
    chkKeepAlive->setChecked(client->getKeepAlive());
	chkMediaSense->setChecked(client->getMediaSense());
    cmbAck->setCurrentIndex(client->getAckMode());
}

void ICQConfig::autoToggled(bool bState)
{
    chkHTTP->setEnabled(!bState);
}

void ICQConfig::invisibleToggled(bool bState)
{
    lblInvisible->setEnabled(bState);
    lblInvisible2->setEnabled(bState);
    edtInvisible->setEnabled(bState);
}

void ICQConfig::apply(Client*, void*)
{
}

void ICQConfig::apply()
{
    if (m_bConfig){
        m_client->setUin(edtUin->text().toULong());
        m_client->setPassword(edtPasswd->text());
    }
    m_client->setServer(edtServer->text());
    m_client->setPort(edtPort->text().toUShort());
    m_client->setMinPort(edtMinPort->text().toUShort());
    m_client->setMaxPort(edtMaxPort->text().toUShort());
    m_client->setSendFormat(cmbFormat->currentIndex());
    m_client->setDisablePlugins(chkPlugins->isChecked());
    m_client->setDisableAutoUpdate(chkUpdate->isChecked());
    m_client->setDisableAutoReplyUpdate(chkAutoReply->isChecked());
    m_client->setDisableTypingNotification(chkTyping->isChecked());
    m_client->setAcceptInDND(chkDND->isChecked());
    m_client->setAcceptInOccupied(chkOccupied->isChecked());
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setAutoHTTP(chkAuto->isChecked());
    m_client->setKeepAlive(chkKeepAlive->isChecked());
    m_client->setAckMode(cmbAck->currentIndex());
    m_client->setMediaSense(chkMediaSense->isChecked());
}

void ICQConfig::changed(const QString&)
{
    changed();
}

void ICQConfig::newToggled(bool bNew)
{
    if (bNew)
        edtUin->setText(QString::null);
    lblUin->setEnabled(!bNew);
    edtUin->setEnabled(!bNew);
}

void ICQConfig::changed()
{
    bool bOK = true;
    if (!chkNew->isChecked())
        bOK = (edtUin->text().toLong() > 1000);
    bOK =  bOK && !edtPasswd->text().isEmpty() &&
           !edtServer->text().isEmpty() &&
           edtPort->text().toUShort();
    emit okEnabled(bOK);
}

// vim: set expandtab:


