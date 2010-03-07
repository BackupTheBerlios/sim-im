/***************************************************************************
                          msninfo.cpp  -  description
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

#include <QLineEdit>
#include <QStringList>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>

#include "icons.h"
#include "misc.h"

#include "msninfo.h"
#include "msnclient.h"
#include "contacts/contact.h"

using namespace SIM;

MSNInfo::MSNInfo(QWidget *parent, MSNUserData *data, MSNClient *client) : QWidget(parent)
{
    setupUi(this);
    m_client  = client;
    m_data    = data;
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtEMail->setReadOnly(true);
    if (m_data){
        edtNick->setReadOnly(true);
    }
    fill();
}

void MSNInfo::apply()
{
}

bool MSNInfo::processEvent(Event *e)
{
    if ((e->type() == eEventMessageReceived) && m_data){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if ((msg->type() == MessageStatus) && (m_client->dataName(m_data) == msg->client()))
            fill();
    } else
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

void MSNInfo::fill()
{
    MSNUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtEMail->setText(data->EMail.str());
    edtNick->setText(!data->ScreenName.str().isEmpty() ? data->ScreenName.str() : data->EMail.str());
    int current = 0;
    QString text;
    unsigned status = m_data ? m_data->Status.toULong() : m_client->getStatus();
    for (const CommandDef *cmd = m_client->protocol()->statusList(); cmd->id; cmd++){
        if (cmd->flags & COMMAND_CHECK_STATE)
            continue;
        if (status == cmd->id){
            current = cmbStatus->count();
            text = cmd->text;
        }
        cmbStatus->addItem(Icon(cmd->icon), i18n(cmd->text));
    }
    cmbStatus->setCurrentIndex(current);
    disableWidget(cmbStatus);
    if (status == STATUS_OFFLINE){
        lblOnline->setText(i18n("Last online") + ":");
        edtOnline->setText(formatDateTime(data->StatusTime.toULong()));
        lblNA->hide();
        edtNA->hide();
    }else{
        if (data->OnlineTime.toULong()){
            edtOnline->setText(formatDateTime(data->OnlineTime.toULong()));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || (text.isEmpty())){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(data->StatusTime.toULong()));
        }
    }
}

void MSNInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    QString nick = edtNick->text();
    if (nick == edtEMail->text())
        nick = QString::null;
    MSNUserData *data = m_client->toMSNUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->ScreenName.str() = nick;
}

