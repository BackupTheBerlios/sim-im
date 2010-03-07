/***************************************************************************
                          icqsecure.cpp  -  description
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
#include "icqsecure.h"
#include "icqclient.h"
#include "simgui/ballonmsg.h"
#include "contacts/contact.h"

#include <QCheckBox>
#include <QTabWidget>
#include <QLineEdit>

using namespace SIM;

ICQSecure::ICQSecure(QWidget *parent, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    connect(chkHideIP, SIGNAL(toggled(bool)), this, SLOT(hideIpToggled(bool)));
    setListView(lstVisible);
    setListView(lstInvisible);
    fill();
    connect(lstVisible, SIGNAL(deleteItem(ListViewItem*)), this, SLOT(deleteVisibleItem(ListViewItem*)));
    connect(lstInvisible, SIGNAL(deleteItem(ListViewItem*)), this, SLOT(deleteInvisibleItem(ListViewItem*)));
}

void ICQSecure::deleteVisibleItem(ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = m_client->toICQUserData(++it)) != NULL){
            data->VisibleId.asULong() = 0;
            EventContact eContact(contact, EventContact::eChanged);
            eContact.process();
        }
    }
}

void ICQSecure::deleteInvisibleItem(ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = m_client->toICQUserData(++it)) != NULL){
            data->InvisibleId.asULong() = 0;
            EventContact eContact(contact, EventContact::eChanged);
            eContact.process();
        }
    }
}

void ICQSecure::apply()
{
    bool bStatusChanged = false;
    if (chkHideIP->isChecked() != m_client->getHideIP()){
        bStatusChanged = true;
        m_client->setHideIP(chkHideIP->isChecked());
    }
    unsigned mode = 0;
	if(btnDirectAllow->isChecked())
		mode = 1;
	else if(btnDirectAuth->isChecked())
		mode = 2;
    if (mode != m_client->getDirectMode()){
        bStatusChanged = true;
        m_client->setDirectMode(mode);
    }
    if (bStatusChanged && (m_client->getState() == Client::Connected))
        m_client->snacService()->sendStatus();
    m_client->setIgnoreAuth(chkIgnoreAuth->isChecked());
    m_client->setUseMD5(chkUseMD5->isChecked());
}

void ICQSecure::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->WaitAuth.asBool() = chkAuth->isChecked();
    data->WebAware.asBool() = chkWeb->isChecked();
}

void ICQSecure::fill()
{
    chkAuth->setChecked(m_client->data.owner.WaitAuth.toBool());
    chkWeb->setChecked(m_client->data.owner.WebAware.toBool());
    chkHideIP->setChecked(m_client->getHideIP());
    chkIgnoreAuth->setChecked(m_client->getIgnoreAuth());
    chkUseMD5->setChecked(m_client->getUseMD5());

    switch(m_client->getDirectMode())
	{
		case 0:
			btnDirectContact->setChecked(true);
			break;
		case 1:
			btnDirectAllow->setChecked(true);
			break;
		case 2:
			btnDirectAuth->setChecked(true);
			break;
		default:
			break;

	}
    fillListView(lstVisible, &ICQUserData::VisibleId);
    fillListView(lstInvisible, &ICQUserData::InvisibleId);
    hideIpToggled(m_client->getHideIP());
}

bool ICQSecure::processEvent(Event *e)
{
    if (e->type() == eEventClientChanged){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    } else
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        fillListView(lstVisible, &ICQUserData::VisibleId);
        fillListView(lstInvisible, &ICQUserData::InvisibleId);
    }
    return false;
}

void ICQSecure::setListView(ListView *lst)
{
    //lst->setSorting(0);
    lst->addColumn(i18n("UIN"));
    lst->addColumn(i18n("Nick"));
    lst->addColumn(i18n("Name"));
    lst->addColumn(i18n("EMail"));
    //lst->setColumnAlignment(0, Qt::AlignRight);
    lst->setExpandingColumn(3);
}

void ICQSecure::fillListView(ListView *lst, SIM::Data ICQUserData::* field)
{
    lst->clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, m_client);
        while ((data = m_client->toICQUserData(++it)) != NULL){
            if ((data->*field).toULong()){
                QString firstName = contact->getFirstName();
                QString lastName  = contact->getLastName();
                firstName = getToken(firstName, '/');
                lastName = getToken(lastName, '/');
                if (!lastName.isEmpty()){
                    if (!firstName.isEmpty())
                        firstName += ' ';
                    firstName += lastName;
                }
                QString mails;
                QString emails = contact->getEMails();
                while (emails.length()){
                    QString mailItem = getToken(emails, ';', false);
                    mailItem = getToken(mailItem, '/');
                    if (!mails.isEmpty())
                        mails += ", ";
                    mails += mailItem;
                }
                ListViewItem *item = new ListViewItem(lst);
                item->setText(0,QString::number(data->Uin.toULong()));
                item->setText(1,contact->getName());
                item->setText(2,firstName);
                item->setText(3,mails);
                item->setText(4,QString::number(contact->id()));
                unsigned long status = STATUS_UNKNOWN;
                unsigned style  = 0;
                QString statusIcon;
                ((Client*)m_client)->contactInfo(data, status, style, statusIcon);
                item->setPixmap(0, Pict(statusIcon));
            }
        }
    }
}

void ICQSecure::hideIpToggled(bool bOn)
{
    if (bOn) {
        btnDirectAuth->setChecked(true);
        grpDirect->setEnabled(false);
    } else {
        switch(m_client->getDirectMode())
        {
            case 0:
                btnDirectContact->setChecked(true);
                break;
            case 1:
                btnDirectAllow->setChecked(true);
                break;
            case 2:
                btnDirectAuth->setChecked(true);
                break;
            default:
                break;

        }
        grpDirect->setEnabled(true);
    }
}


// vim: set expandtab:

