/***************************************************************************
                          jabberconfig.cpp  -  description
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

#include "simapi.h"

#include <QTimer>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>

#include "simgui/linklabel.h"
#include "misc.h"

#include "jabberclient.h"
#include "jabberconfig.h"
#include "jabber.h"

using namespace SIM;

JabberConfig::JabberConfig(QWidget *parent, JabberClient *client, bool bConfig) : QWidget(parent)
        //: JabberConfigBase(parent)
{
	setupUi(this);
    m_client = client;
    m_bConfig = bConfig;
    QTimer::singleShot(0, this, SLOT(changed()));
    edtID->setText(m_client->getID());
    edtPasswd->setText(m_client->getPassword());
    edtServer->setText(m_client->getServer());
    edtPort->setValue(m_client->getPort());
    edtPriority->setValue(m_client->getPriority());
    edtResource->setText(m_client->data.owner.getResource());
    edtVHost->setText(m_client->data.getVHost());
    if (m_bConfig){
        tabCfg->removeTab(tabCfg->indexOf(tabJabber));
    }else{
        lblServer->hide();
        edtServer->hide();
        lblPort->hide();
        edtPort->hide();
        chkSSL1->hide();
        edtServer1->setText(i18n("jabber.org"));
        edtPort1->setValue(m_client->getPort());
    }
    chkSSL->setChecked(m_client->getUseSSL());
    chkSSL1->setChecked(m_client->getUseSSL());
    chkPlain->setChecked(m_client->getUsePlain());
    edtMinPort->setValue(m_client->getMinPort());
    edtMaxPort->setValue(m_client->getMaxPort());
    chkVHost->setChecked(m_client->getUseVHost());
    chkTyping->setChecked(m_client->getTyping());
    chkRichText->setChecked(m_client->getRichText());
    chkIcons->setChecked(m_client->getProtocolIcons());
    chkSubscribe->setChecked(m_client->getAutoSubscribe());
    chkAccept->setChecked(m_client->getAutoAccept());
    chkVersion->setChecked(m_client->getUseVersion());
    lnkPublic->setText(i18n("List of public servers"));
    lnkPublic->setUrl("http://www.xmpp.net/servers");
    connect(edtID, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(chkSSL, SIGNAL(toggled(bool)), this, SLOT(toggledSSL(bool)));
    connect(chkSSL1, SIGNAL(toggled(bool)), this, SLOT(toggledSSL(bool)));
    connect(chkVHost, SIGNAL(toggled(bool)), this, SLOT(toggledVHost(bool)));
    chkHTTP->setChecked(m_client->getUseHTTP());
    edtUrl->setText(m_client->getURL());
    edtVHost->setEnabled(m_client->getUseVHost());
}

void JabberConfig::apply(Client*, void*)
{
}

void JabberConfig::apply()
{
    if (m_bConfig){
        m_client->setServer(edtServer->text());
        m_client->setPort(edtPort->text().toUShort());
    }else{
        m_client->setServer(edtServer1->text());
        m_client->setPort(edtPort1->text().toUShort());
    }
    m_client->setUseVHost(false);
    if (chkVHost->isChecked()){
        m_client->data.getVHost() = edtVHost->text();
        if (!edtVHost->text().isEmpty())
            m_client->setUseVHost(true);
    }
    QString jid = edtID->text();
    int n = jid.indexOf('@');
    if (n >= 0){
        QString host = jid.mid(n + 1);
        m_client->data.setVHost(host);
        m_client->setUseVHost(true);
    } else if (chkVHost->isChecked()){
        jid += '@';
        jid += edtVHost->text();
    } else {
        jid += '@';
        jid += edtServer1->text();
    }
    if (!m_bConfig){
        m_client->setID(jid);
        m_client->setPassword(edtPasswd->text());
        m_client->setRegister(chkRegister->isChecked());
    }
    if (m_bConfig){
        m_client->setUseSSL(chkSSL1->isChecked());
    }else{
        m_client->setUseSSL(chkSSL->isChecked());
    }
    m_client->setUsePlain(chkPlain->isChecked());
    m_client->setMinPort(edtMinPort->text().toUShort());
    m_client->setMaxPort(edtMaxPort->text().toUShort());
    m_client->setTyping(chkTyping->isChecked());
    m_client->setRichText(chkRichText->isChecked());
    m_client->setUseVersion(chkVersion->isChecked());
    m_client->setAutoSubscribe(chkSubscribe->isChecked());
    m_client->setAutoAccept(chkAccept->isChecked());
    if (m_client->getProtocolIcons() != chkIcons->isChecked()){
        m_client->setProtocolIcons(chkIcons->isChecked());
        EventRepaintView e;
        e.process();
    }
    m_client->data.owner.getResource() = edtResource->text();
    m_client->setPriority(edtPriority->text().toLong());
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setURL(edtUrl->text());
}

void JabberConfig::toggledSSL(bool bState)
{
    unsigned port = edtPort1->text().toUShort();
    if (m_bConfig)
        port = edtPort->text().toUShort();
    if (port == 0)
        port = 5222;
    if (bState){
        port++;
    }else{
        port--;
    }
    edtPort->setValue(port);
    edtPort1->setValue(port);
}

void JabberConfig::toggledVHost(bool bState)
{
    edtVHost->setEnabled(bState);
}

void JabberConfig::changed(const QString&)
{
    changed();
}

void JabberConfig::changed()
{
    bool bOK =  !edtID->text().isEmpty() &&
                !edtPasswd->text().isEmpty();
    if (bOK){
        if (m_bConfig){
            bOK = !edtServer->text().isEmpty() &&
                  edtPort->text().toUShort();
        }else{
            bOK = !edtServer1->text().isEmpty() &&
                  edtPort1->text().toUShort();
        }
    }
    emit okEnabled(bOK);
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabberconfig.moc"
#endif
*/

