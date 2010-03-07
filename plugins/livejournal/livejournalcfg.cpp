/***************************************************************************
                          livejournalcfg.cpp  -  description
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

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>

#include "simgui/linklabel.h"
#include "misc.h"

#include "livejournalcfg.h"
#include "livejournal.h"

using namespace SIM;

LiveJournalCfg::LiveJournalCfg(QWidget *parent, LiveJournalClient *client, bool bConfig)
        : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    m_bConfig = bConfig;
    edtName->setText(client->data.owner.User.str());
    if (bConfig){
        edtPassword->setText(client->getPassword());
        lblLnk->setText(i18n("Register new user"));
        lblLnk->setUrl("http://www.livejournal.com/create.bml");
    }else{
        edtName->setReadOnly(true);
        edtPassword->hide();
        lblPassword->hide();
    }
    edtServer->setText(client->getServer());
    edtPath->setText(client->getURL());
    edtPort->setValue(client->getPort());
    edtInterval->setValue(client->getInterval());
    chkFastServer->setChecked(client->getFastServer());
    chkUseFormatting->setChecked(client->getUseFormatting());
    chkUseSignature->setChecked(client->getUseSignature());
    edtSignature->setPlainText(client->getSignatureText());
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPassword, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(chkUseSignature, SIGNAL(toggled(bool)), this, SLOT(useSigToggled(bool)));
    useSigToggled(chkUseSignature->isChecked());
    changed("");
    QTimer::singleShot(0, this, SLOT(changed()));
}

void LiveJournalCfg::changed(const QString&)
{
    changed();
}

void LiveJournalCfg::changed()
{
    emit okEnabled(!edtName->text().isEmpty() && !edtPassword->text().isEmpty());
}

void LiveJournalCfg::apply()
{
    if (m_bConfig){
        m_client->data.owner.User.str() = edtName->text();
        m_client->setPassword(edtPassword->text());
    }
    m_client->setServer(edtServer->text());
    m_client->setURL(edtPath->text());
    m_client->setPort(edtPort->text().toUShort());
    m_client->setInterval(edtInterval->text().toULong());
    m_client->setFastServer(chkFastServer->isChecked());
    m_client->setUseFormatting(chkUseFormatting->isChecked());
    m_client->setUseSignature(chkUseSignature->isChecked());
    if (edtSignature->toPlainText() != m_client->getSignatureText())
        m_client->setSignature(edtSignature->toPlainText());
}

void LiveJournalCfg::apply(Client*, void*)
{
}

void LiveJournalCfg::useSigToggled(bool value)
{
    edtSignature->setEnabled(value);
}


