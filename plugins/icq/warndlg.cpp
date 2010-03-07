/***************************************************************************
                          warndlg.cpp  -  description
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
#include "warndlg.h"
#include "simgui/ballonmsg.h"
#include "icqclient.h"
#include "icqmessage.h"
#include "contacts/contact.h"

#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <QRegExp>
#include <QCheckBox>
#include <QTimer>

using namespace SIM;

WarnDlg::WarnDlg(QWidget *parent, ICQUserData *data, ICQClient *client)
  : QDialog(parent)
{
    setupUi(this);
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose);
    SET_WNDPROC("warn")
    setWindowIcon(Icon("error"));
    setButtonsPict(this);
    setWindowTitle(windowTitle());
    m_client  = client;
    m_data    = data;
    m_msg     = NULL;
    m_contact = 0;
    Contact *contact;
    if (m_client->findContact(client->screen(data), NULL, false, contact))
        m_contact = contact->id();
    lblInfo->setText(lblInfo->text().replace(QRegExp("\\%1"), m_client->screen(data)));
    chkAnon->setChecked(m_client->getWarnAnonimously());
}

WarnDlg::~WarnDlg()
{
    if (m_msg)
        EventMessageCancel(m_msg).process();
}

void WarnDlg::accept()
{
    m_msg = new WarningMessage;
    m_msg->setClient(m_client->dataName(m_data));
    m_msg->setContact(m_contact);
    m_msg->setAnonymous(chkAnon->isChecked());
    m_client->setWarnAnonimously(chkAnon->isChecked());
    if (!((Client*)m_client)->send(m_msg, m_data)){
        delete m_msg;
        m_msg = NULL;
        showError(I18N_NOOP("Send failed"));
        return;
    }
    buttonOk->setEnabled(false);
}

void WarnDlg::showError(const char *error)
{
    buttonOk->setEnabled(true);
    BalloonMsg::message(i18n(error), buttonOk);
}

bool WarnDlg::processEvent(Event *e)
{
    if (e->type() == eEventMessageSent){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg == m_msg){
            m_msg = false;
            QString err = msg->getError();
            if (!err.isEmpty()){
                showError(qPrintable(err));
            }else{
                QTimer::singleShot(0, this, SLOT(close()));
            }
        }
    }
    return false;
}
