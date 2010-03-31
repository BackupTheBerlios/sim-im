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

void AIMInfo::updateData(ICQUserData* data)
{
    data->setFirstName(edtFirst->text());
    data->setLastName(edtLast->text());
    data->setMiddleName(edtMiddle->text());
    data->setMaiden(edtMaiden->text());
    data->setNick(edtNick->text());
    data->setAddress(edtStreet->text());
    data->setCity(edtCity->text());
    data->setState(edtState->text());
    data->setZip(edtZip->text());
    data->setCountry(getComboValue(cmbCountry, getCountries()));
}

void AIMInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if(client.data() != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

void AIMInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
    updateData(data);
}

bool AIMInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
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

    edtScreen->setText(data->getScreen());
    edtFirst->setText(data->getFirstName());
    edtLast->setText(data->getLastName());
	edtMiddle->setText(data->getMiddleName());
    edtMaiden->setText(data->getMaiden());
    edtNick->setText(data->getNick());
    edtStreet->setText(data->getAddress());
    edtCity->setText(data->getCity());
    edtState->setText(data->getState());
    edtZip->setText(data->getZip());
    initCombo(cmbCountry, data->getCountry(), getCountries());

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
        switch (m_data->getStatus()){
        case STATUS_ONLINE:
        case STATUS_OFFLINE:
            status = m_data->getStatus();
            break;
        default:
            status = STATUS_AWAY;
        }
    }else{
        status = m_client->getStatus();
    }
    if (m_data && !m_data->getAutoReply().isEmpty()){
        edtAutoReply->setPlainText(m_data->getAutoReply());
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
        edtOnline->setText(formatDateTime(QDateTime::fromTime_t(data->getStatusTime())));
        lblNA->hide();
        edtNA->hide();
    }else{
		if (data->getOnlineTime()){
			edtOnline->setText(formatDateTime(QDateTime::fromTime_t(data->getOnlineTime())));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || text.isEmpty()){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(QDateTime::fromTime_t(data->getStatusTime())));
        }
    }
    if (data->getIP()){
        edtExtIP->setText(formatAddr(data->getIP(), data->getPort()));
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if (data->getRealIP() && ((data->getIP() == 0) || ((data->getIP()) != (data->getRealIP())))){
        edtIntIP->setText(formatAddr(data->getRealIP(), data->getPort()));
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

