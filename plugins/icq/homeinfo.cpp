/***************************************************************************
                          homeinfo.cpp  -  description
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

#include "homeinfo.h"
#include "icqclient.h"
#include "log.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

using namespace SIM;

HomeInfo::HomeInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_data    = data;
    m_client  = client;
    m_contact = contact;
    if (m_data){
        edtAddress->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        disableWidget(cmbCountry);
        disableWidget(cmbZone);
    }
    fill();
    btnWebLocation->setText(i18n("map"));
    connect(btnWebLocation, SIGNAL(clicked()), this, SLOT(goUrl()));
}

void HomeInfo::apply()
{
}

void HomeInfo::updateData(ICQUserData* data)
{
    data->setAddress(edtAddress->toPlainText());
    data->setCity(edtCity->text());
    data->setState(edtState->text());
    data->setZip(edtZip->text());
    data->setCountry(getComboValue(cmbCountry, getCountries()));
}

void HomeInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if (client != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

void HomeInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
    updateData(data);
}

bool HomeInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
            fill();
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

static QString formatTime(char n)
{
    QString res;
    res.sprintf("%+i:%02u", -n/2, (n & 1) * 30);
    return res;
}

static void initTZCombo(QComboBox *cmb, char tz)
{
    if (tz < -24)
        tz = 0;
    if (tz > 24)
        tz = 0;
    if (cmb->isEnabled()){
        unsigned nSel = 12;
        unsigned n = 0;
        for (char i = 24; i >= -24; i--, n++){
            cmb->addItem(formatTime(i));
            if (i == tz) nSel = n;
        }
        cmb->setCurrentIndex(nSel);
    }else{
        cmb->addItem(formatTime(tz));
    }
}

void HomeInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    edtAddress->setPlainText(data->getAddress());
    edtCity->setText(data->getCity());
    edtState->setText(data->getState());
    edtZip->setText(data->getZip());
    initCombo(cmbCountry, data->getCountry(), getCountries());
    initTZCombo(cmbZone, data->getTimeZone());
}

void HomeInfo::goUrl()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    QString url = QString("http://www.mapquest.com/maps/map.adp?city=%1&state=%2&country=%3&zip=%4")
                    .arg(edtCity->text())
                    .arg(edtState->text())
                    .arg(cmbCountry->currentText())
                    .arg(edtZip->text());
    EventGoURL e(url);
    e.process();
}

