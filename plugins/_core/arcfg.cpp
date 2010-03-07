/***************************************************************************
                          arcfg.cpp  -  description
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

#include "arcfg.h"
#include "core.h"
#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include <QTabWidget>
#include <QCheckBox>

using namespace SIM;

ARConfig::ARConfig(QWidget *p, unsigned status, const QString &name, Contact *contact) 
    : QWidget(p)
    , m_status(status)
    , m_contact(contact)
{
	setupUi(this);
    setButtonsPict(this);
    tabAR->setTabText(tabAR->indexOf(tab), name);
    SIM::PropertyHubPtr ar;
    QString text;
	SIM::PropertyHubPtr core = CorePlugin::instance()->propertyHub();
    QString noShow = core->stringMapValue("NoShowAutoReply", m_status);
    if (m_contact)
    {
        chkNoShow->hide();
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        ar = m_contact->getUserData()->getUserData("AR");
        if (ar)
            text = ar->stringMapValue("AutoReply", m_status);
        if (!text.isEmpty())
            chkOverride->setChecked(true);
        else
        {
            ar.clear();
            Group *grp = getContacts()->group(m_contact->getGroup());
            if (grp)
                ar = grp->getUserData()->getUserData("AR");
            if (!ar.isNull())
                text = ar->stringMapValue("AutoReply", m_status);
        }
        toggled(chkOverride->isChecked());
    }
    else
        chkOverride->hide();
    if (text.isEmpty())
    {
        ar = getContacts()->getUserData("AR");
        if (!noShow.isEmpty())
            chkNoShow->setChecked(true);
        text = ar->stringMapValue("AutoReply", m_status);
        if (text.isEmpty())
            text = ar->stringMapValue("AutoReply", STATUS_AWAY);
    }
    edtAutoReply->setText(text);
    EventTmplHelpList e;
    e.process();
    edtAutoReply->setHelpList(e.helpList());
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
}

void ARConfig::apply()
{
    if (m_contact) 
        applyForSpecialUser();
    else
        applyGlobal();
}

void ARConfig::applyForSpecialUser()
{
    SIM::PropertyHubPtr ar = m_contact->getUserData()->getUserData("AR");
    if (chkOverride->isChecked())
    {
        if(ar.isNull())
            ar = m_contact->getUserData()->createUserData("AR");
        ar->setStringMapValue("AutoReply", m_status, edtAutoReply->toPlainText());
    }
    else if (!ar.isNull())
        ar->setStringMapValue("AutoReply", m_status, QString::null);
}
void ARConfig::applyGlobal()
{
    SIM::PropertyHubPtr ar = getContacts()->getUserData("AR");
    ar->setStringMapValue("AutoReply", m_status, edtAutoReply->toPlainText());
    SIM::PropertyHubPtr core = CorePlugin::instance()->propertyHub();
    core->setStringMapValue("NoShowAutoReply", m_status, chkNoShow->isChecked() ? "1" : "");
}
void ARConfig::toggled(bool bState)
{
    edtAutoReply->setEnabled(bState);
}

void ARConfig::help()
{
    QString helpString = i18n("In text you can use:") + '\n';
    EventTmplHelp e(helpString);
    e.process();
    BalloonMsg::message(e.help(), btnHelp, false, 400);
}

// vim: set expandtab:

