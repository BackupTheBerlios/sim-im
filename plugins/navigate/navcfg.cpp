/***************************************************************************
                          navcfg.cpp  -  description
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

#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>

#include "navcfg.h"
#include "navigate.h"

NavCfg::NavCfg(QWidget *parent, NavigatePlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
#ifdef WIN32
    chkNew->setChecked(plugin->value("NewWindow").toBool());
    edtBrowser->hide();
    edtMailer->hide();
    lblBrowser->hide();
    lblMailer->hide();
#else
    edtBrowser->setText(plugin->value("Browser").toString());
    edtMailer->setText(plugin->value("Mailer").toString());
    chkNew->hide();
#endif
#ifdef USE_KDE
    connect(chkKDE, SIGNAL(toggled(bool)), SLOT(useKDEtoggled(bool)));
    chkKDE->setChecked(plugin->getUseKDE());
#else
    chkKDE->hide();
#endif
}

void NavCfg::apply()
{
#ifdef WIN32
    m_plugin->setValue("NewWindow", chkNew->isChecked());
#else
    m_plugin->setValue("Browser", edtBrowser->text());
    m_plugin->setValue("Mailer", edtMailer->text());
#endif
#ifdef USE_KDE
    m_plugin->setUseKDE(chkKDE->isChecked());
#endif
}

void NavCfg::useKDEtoggled(bool on)
{
    bool off = !on;
#ifdef USE_KDE
    edtBrowser->setEnabled(off);
    edtMailer->setEnabled(off);
    lblBrowser->setEnabled(off);
    lblMailer->setEnabled(off);
#endif
    off=false;
}

