/***************************************************************************
                          yahooinfo.cpp  -  description
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
#include "yahoo.h"
#include "yahooinfo.h"
#include "yahooclient.h"
#include "core.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QStringList>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>

using namespace SIM;

YahooInfo::YahooInfo(QWidget *parent, YahooUserData *data, YahooClient *client) : QWidget(parent)
{
	setupUi(this);
    m_client  = client;
    m_data    = data;
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtLogin->setReadOnly(true);
    if (m_data){
        edtNick->setReadOnly(true);
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
    }
    fill();
}

void YahooInfo::apply()
{
    YahooUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
}

bool YahooInfo::processEvent(Event *e)
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

void YahooInfo::fill()
{
    YahooUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    edtLogin->setText(data->Login.str());
    edtNick->setText(data->Nick.str());
    edtFirst->setText(data->First.str());
    edtLast->setText(data->Last.str());
    int current = 0;
    QString text;
    unsigned long status = STATUS_OFFLINE;
    if (m_data == NULL){
        if (m_client->getState() == Client::Connected){
            QString statusIcon;
            unsigned style  = 0;
            m_client->contactInfo(&m_client->data.owner, status, style, statusIcon);
        }
    }else{
        QString statusIcon;
        unsigned style  = 0;
        m_client->contactInfo(data, status, style, statusIcon);
    }
    for (const CommandDef *cmd = m_client->protocol()->statusList(); cmd->id; cmd++){
        if (cmd->flags & COMMAND_CHECK_STATE)
            continue;
        if (status == cmd->id){
            current = cmbStatus->count();
            text = cmd->text;
        }
        cmbStatus->insertItem(INT_MAX, Icon(cmd->icon), i18n(cmd->text));
    }
    cmbStatus->setCurrentIndex(current);
    disableWidget(cmbStatus);
    if (status == STATUS_OFFLINE){
        if (data->StatusTime.toULong()){
            lblOnline->setText(i18n("Last online") + ":");
            edtOnline->setText(formatDateTime(data->StatusTime.toULong()));
            lblOnline->show();
            edtOnline->show();
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
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

void YahooInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    YahooUserData *data = m_client->toYahooUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->Nick.str()  = edtNick->text();
    data->First.str() = edtFirst->text();
    data->Last.str()  = edtLast->text();
}

