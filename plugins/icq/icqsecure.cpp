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
    connect(lstVisible, SIGNAL(deleteItem(QTreeWidgetItem*)), this, SLOT(deleteVisibleItem(QTreeWidgetItem*)));
    connect(lstInvisible, SIGNAL(deleteItem(QTreeWidgetItem*)), this, SLOT(deleteInvisibleItem(QTreeWidgetItem*)));
}

void ICQSecure::deleteVisibleItem(QTreeWidgetItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it = contact->clientDataIterator();
        while ((data = m_client->toICQUserData(++it)) != NULL){
            data->setVisibleId(0);
            EventContact eContact(contact, EventContact::eChanged);
            eContact.process();
        }
    }
}

void ICQSecure::deleteInvisibleItem(QTreeWidgetItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it = contact->clientDataIterator();
        while ((data = m_client->toICQUserData(++it)) != NULL){
            data->setInvisibleId(0);
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

void ICQSecure::updateData(ICQUserData* data)
{
    data->setWaitAuth(chkAuth->isChecked());
    data->setWebAware(chkWeb->isChecked());
}

void ICQSecure::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if (client != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

void ICQSecure::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
    updateData(data);
}

void ICQSecure::fill()
{
    chkAuth->setChecked(m_client->data.owner.getWaitAuth());
    chkWeb->setChecked(m_client->data.owner.getWebAware());
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
    fillListView(lstVisible, 1);
    fillListView(lstInvisible, 2);
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
        fillListView(lstVisible, 1);
        fillListView(lstInvisible, 2);
    }
    return false;
}

void ICQSecure::setListView(QTreeWidget *lst)
{
    lst->setColumnCount(4);
    QStringList columns;

    columns.append(i18n("UIN"));
    columns.append(i18n("Nick"));
    columns.append(i18n("Name"));
    columns.append(i18n("EMail"));
    lst->setHeaderLabels(columns);
}

void ICQSecure::fillListView(QTreeWidget *lst, SIM::Data ICQUserData::* field)
{
    lst->clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ICQUserData *data;
        ClientDataIterator it = contact->clientDataIterator(m_client);
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
                QTreeWidgetItem *item = new QTreeWidgetItem(lst);
                item->setText(0,QString::number(data->getUin()));
                item->setText(1,contact->getName());
                item->setText(2,firstName);
                item->setText(3,mails);
                item->setText(4,QString::number(contact->id()));
                unsigned long status = STATUS_UNKNOWN;
                unsigned style  = 0;
                QString statusIcon;
                ((Client*)m_client)->contactInfo(data, status, style, statusIcon);
                item->setIcon(0, Pict(statusIcon));
            }
        }
    }
}

void ICQSecure::fillListView(QTreeWidget *lst, int v)
{
    lst->clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ICQUserData *data;
        ClientDataIterator it = contact->clientDataIterator(m_client);
        while ((data = m_client->toICQUserData(++it)) != NULL){
            unsigned long val = 0;
            switch(v)
            {
            case 0:
                val = data->getIgnoreId();
                break;
            case 1:
                val = data->getVisibleId();
                break;
            case 2:
                val = data->getInvisibleId();
                break;
            }

            if (val){
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
                QTreeWidgetItem *item = new QTreeWidgetItem(lst);
                item->setText(0,QString::number(data->getUin()));
                item->setText(1,contact->getName());
                item->setText(2,firstName);
                item->setText(3,mails);
                item->setText(4,QString::number(contact->id()));
                unsigned long status = STATUS_UNKNOWN;
                unsigned style  = 0;
                QString statusIcon;
                ((Client*)m_client)->contactInfo(data, status, style, statusIcon);
                item->setIcon(0, Pict(statusIcon));
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

