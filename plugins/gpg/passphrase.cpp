/***************************************************************************
                          passphrase.cpp  -  description
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

#include "icons.h"
#include "misc.h"

#include "passphrase.h"
#include "gpg.h"
#include "simgui/ballonmsg.h"

#include <QPixmap>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

using namespace SIM;

PassphraseDlg::PassphraseDlg(GpgPlugin *plugin, const QString &key) : QDialog(NULL)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setObjectName("passphrase");
    m_plugin = plugin;
    SET_WNDPROC("passphrase")
    setWindowIcon(Icon("encrypted"));
    setButtonsPict(this);
    m_key = key;
    lblTitle->setText(i18n("Input passphrase for key %1") .arg(key));
    connect(edtPass, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    btnOk->setEnabled(false);
    chkSave->setChecked(m_plugin->value("SavePassphrase").toBool());
}

PassphraseDlg::~PassphraseDlg()
{
    emit finished();
}

void PassphraseDlg::accept()
{
    m_plugin->setValue("SavePassphrase", chkSave->isChecked());
    emit apply(edtPass->text());
}

void PassphraseDlg::textChanged(const QString &text)
{
    btnOk->setEnabled(!text.isEmpty());
}

void PassphraseDlg::error()
{
    raiseWindow(this);
    BalloonMsg::message(i18n("Bad passphrase"), btnOk);
}

