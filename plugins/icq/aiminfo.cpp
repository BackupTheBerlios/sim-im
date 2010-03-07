/***************************************************************************
                          aiminfo.cpp  -  description
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
#include "aiminfo.h"
#include "icqclient.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QStringList>
#include <QComboBox>
#include <QPixmap>
#include <QLabel>

using namespace SIM;

AIMInfo::AIMInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_client  = client;
    m_data    = data;
    m_contact = contact;
    edtScreen->setReadOnly(true);
    if (m_data){
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
        edtMiddle->setReadOnly(true);
        edtMaiden->setReadOnly(true);
        edtNick->setReadOnly(true);
        edtStreet->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
        disableWidget(cmbCountry);
    }else{
        edtAutoReply->hide();
    }
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtExtIP->setReadOnly(true);
    edtIntIP->setReadOnly(true);
    edtClient->setReadOnly(true);
    fill();
}

void AIMInfo::apply()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
}

void AIMInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::clientData*)_data);  // FIXME unsafe type conversion
    data->FirstName.str()   = edtFirst->text();
    data->LastName.str()    = edtLast->text();
    data->MiddleName.str()  = edtMiddle->text();
    data->Maiden.str()      = edtMaiden->text();
    data->Nick.str()        = edtNick->text();
    data->Address.str()     = edtStreet->text();
    data->City.str()        = edtCity->text();
    data->State.str()       = edtState->text();
    data->Zip.str()         = edtZip->text();
    data->Country.asULong() = getComboValue(cmbCountry, getCountries());
}

bool AIMInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    } else
    if ((e->type() == eEventMessageReceived) && m_data){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->type() == MessageStatus){
            if (m_client->dataName(m_data) == msg->client())
                fill();
        }
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

void AIMInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;

    edtScreen->setText(data->Screen.str());
    edtFirst->setText(data->FirstName.str());
    edtLast->setText(data->LastName.str());
    edtMiddle->setText(data->MiddleName.str());
    edtMaiden->setText(data->Maiden.str());
    edtNick->setText(data->Nick.str());
    edtStreet->setText(data->Address.str());
    edtCity->setText(data->City.str());
    edtState->setText(data->State.str());
    edtZip->setText(data->Zip.str());
    initCombo(cmbCountry, data->Country.toULong(), getCountries());

    if (m_data == NULL){
        if (edtFirst->text().isEmpty()) {
            QString firstName = getContacts()->owner()->getFirstName();
            firstName = getToken(firstName, '/');
            edtFirst->setText(firstName);
        }
        if (edtLast->text().isEmpty()) {
            QString lastName = getContacts()->owner()->getLastName();
            lastName = getToken(lastName, '/');
            edtLast->setText(lastName);
        }
    }

    cmbStatus->clear();
    unsigned status = STATUS_ONLINE;
    if (m_data){
        switch (m_data->Status.toULong()){
        case STATUS_ONLINE:
        case STATUS_OFFLINE:
            status = m_data->Status.toULong();
            break;
        default:
            status = STATUS_AWAY;
        }
    }else{
        status = m_client->getStatus();
    }
    if (m_data && !m_data->AutoReply.str().isEmpty()){
        edtAutoReply->setPlainText(m_data->AutoReply.str());
    }else{
        edtAutoReply->hide();
    }

    int current = 0;
    QString text;
	ProtocolPtr proto = ICQPlugin::icq_plugin->m_aim;
	AIMProtocol* aim = static_cast<AIMProtocol*>(proto.data());
    for (const CommandDef *cmd = aim->statusList(); cmd->id; cmd++){
        if (cmd->flags & COMMAND_CHECK_STATE)
            continue;
        if (status == cmd->id){
            current = cmbStatus->count();
            text = cmd->text;
        }
        cmbStatus->addItem(Pict(cmd->icon), i18n(cmd->text));
    }

    cmbStatus->setCurrentIndex(current);
    disableWidget(cmbStatus);
    if (status == STATUS_OFFLINE){
        lblOnline->setText(i18n("Last online") + ':');
        edtOnline->setText(formatDateTime(QDateTime::fromTime_t(data->StatusTime.toULong())));
        lblNA->hide();
        edtNA->hide();
    }else{
        if (data->OnlineTime.toULong()){
            edtOnline->setText(formatDateTime(QDateTime::fromTime_t(data->OnlineTime.toULong())));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || text.isEmpty()){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(QDateTime::fromTime_t(data->StatusTime.toULong())));
        }
    }
    if (data->IP.ip()){
        edtExtIP->setText(formatAddr(data->IP, data->Port.toULong()));
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if (data->RealIP.ip() && ((data->IP.ip() == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        edtIntIP->setText(formatAddr(data->RealIP, data->Port.toULong()));
    }else{
        lblIntIP->hide();
        edtIntIP->hide();
    }
    if (m_data){
        QString client_name = m_client->clientName(data);
        if (client_name.length()){
            edtClient->setText(client_name);
        }else{
            lblClient->hide();
            edtClient->hide();
        }
    }else{
        QString name = PACKAGE;
        name += ' ';
        name += VERSION;
#ifdef WIN32
        name += "/win32";
#endif
        edtClient->setText(name);
    }
}

