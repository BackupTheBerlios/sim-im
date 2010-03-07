/***************************************************************************
                          autoreply.cpp  -  description
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

#include "autoreply.h"
#include "core.h"
#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"
#include "contacts/client.h"

#include <QPixmap>
#include <QCheckBox>
#include <QTimer>
#include <QLabel>

using namespace SIM;

AutoReplyDialog::AutoReplyDialog(unsigned status)
    : QDialog(NULL)
    , m_status(status)
    , m_time(15)
    , m_timer(new QTimer(this))
{
	setupUi(this);
    SET_WNDPROC("mainwnd");
    QString text, icon;
    if (!loadIconAndIconText(status, text, icon))
        return;
    setWindowTitle(i18n("Autoreply message") + ' ' + i18n(text));
    setWindowIcon(Icon(icon));
    lblTime->setText(i18n("Close after %n second", "Close after %n seconds", m_time));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(1000);
	SIM::PropertyHubPtr ar = getContacts()->getUserData("AR");
    text = ar->stringMapValue("AutoReply", m_status);
    edtAutoResponse->setText(text);
    connect(edtAutoResponse, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(chkNoShow, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
    EventTmplHelpList e;
    e.process();
    edtAutoResponse->setHelpList(e.helpList());
}

AutoReplyDialog::AutoReplyDialog(const SIM::IMStatusPtr& status)
    : QDialog(NULL)
    , m_time(15)
    , m_timer(new QTimer(this))
    , m_imstatus(status)
{
    setupUi(this);
    initIconMap();
    QString text = status->text();
    QIcon icon = Icon(statusIcon(status));
    setWindowTitle(i18n("Autoreply message") + ' ' + i18n(text));
    setWindowIcon(icon);
    lblTime->setText(i18n("Close after %n second", "Close after %n seconds", m_time));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(1000);
    SIM::PropertyHubPtr ar = getContacts()->getUserData("AR");
    text = ar->value("AutoReply" + m_imstatus->id()).toString();
    edtAutoResponse->setText(text);
    connect(edtAutoResponse, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(chkNoShow, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
    EventTmplHelpList e;
    e.process();
    edtAutoResponse->setHelpList(e.helpList());
}

AutoReplyDialog::~AutoReplyDialog()
{
}

void AutoReplyDialog::initIconMap()
{
    m_iconmap.insert("online", "SIM_online");
    m_iconmap.insert("away", "SIM_away");
    m_iconmap.insert("n/a", "SIM_na");
    m_iconmap.insert("dnd", "SIM_dnd");
    m_iconmap.insert("occupied", "SIM_occupied");
    m_iconmap.insert("free_for_chat", "SIM_ffc");
    m_iconmap.insert("offline", "SIM_offline");
}

QString AutoReplyDialog::statusIcon(const SIM::IMStatusPtr& status)
{
    return m_iconmap.value(status->id());
}

bool AutoReplyDialog::loadIconAndIconText(unsigned status, QString &text, QString &icon)
{
    bool btextFound = true;
    for (unsigned i = 0; i < getContacts()->nClients(); i++)
    {
        for (const CommandDef *d = getContacts()->getClient(i)->protocol()->statusList(); !d->text.isEmpty(); d++)
        {
            if (d->id == status)
            {
                text = d->text;
                switch (d->id){
                        case STATUS_ONLINE:
                            icon="SIM_online";
                            break;
                        case STATUS_AWAY:
                            icon="SIM_away";
                            break;
                        case STATUS_NA:
                            icon="SIM_na";
                            break;
                        case STATUS_DND:
                            icon="SIM_dnd";
                            break;
                        case STATUS_OCCUPIED:
                            icon="SIM_occupied";
                            break;
                        case STATUS_FFC:
                            icon="SIM_ffc";
                            break;
                        case STATUS_OFFLINE:
                            icon="SIM_offline";
                            break;
                        default:
                            icon=d->icon;
                            break;
                }
                break;
            }
        }
        if (!text.isEmpty())
            break;
    }
    if (text.isEmpty())
        return !btextFound;
    return btextFound;
}

void AutoReplyDialog::textChanged()
{
    stopTimer();
}

void AutoReplyDialog::toggled(bool)
{
    stopTimer();
}

void AutoReplyDialog::stopTimer()
{
    if (m_timer == NULL)
        return;
    delete m_timer;
    m_timer = NULL;
    lblTime->hide();
}

void AutoReplyDialog::timeout()
{
    if (--m_time <= 0)
    {
        accept();
        return;
    }
    lblTime->setText(i18n("Close after %n second", "Close after %n seconds", m_time));
}

void AutoReplyDialog::accept()
{
	SIM::PropertyHubPtr core = CorePlugin::instance()->propertyHub();
	core->setStringMapValue("NoShowAutoReply", m_status, chkNoShow->isChecked() ? "1" : "");
    core->setValue("NoShowAutoReply" + m_imstatus->id(), chkNoShow->isChecked());

	SIM::PropertyHubPtr ar = getContacts()->getUserData("AR");
	ar->setStringMapValue("AutoReply", m_status, edtAutoResponse->toPlainText());
    core->setValue("AutoReply" + m_imstatus->id(), edtAutoResponse->toPlainText());
    m_imstatus->setText(edtAutoResponse->toPlainText());
	QDialog::accept();
}

void AutoReplyDialog::help()
{
    stopTimer();
    QString helpString = i18n("In text you can use:") + '\n';
    EventTmplHelp e(helpString);
    e.process();
    BalloonMsg::message(e.help(), btnHelp, false, 400);
}


