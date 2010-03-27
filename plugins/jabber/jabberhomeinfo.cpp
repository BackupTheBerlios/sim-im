/***************************************************************************
                          jabberhomeinfo.cpp  -  description
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

#include "jabberclient.h"
#include "jabberhomeinfo.h"
#include "jabber.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QStringList>

using namespace SIM;

JabberHomeInfo::JabberHomeInfo(QWidget *parent, JabberUserData *data, JabberClient *client) : QWidget(parent)
        //: JabberHomeInfoBase(parent)
{
	setupUi(this);
    m_client  = client;
    m_data    = data;
    if (m_data){
        edtStreet->setReadOnly(true);
        edtExt->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        edtCountry->setReadOnly(true);
    }
    fill(m_data);
}

void JabberHomeInfo::apply()
{
}

bool JabberHomeInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
            fill(m_data);
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill(m_data);
    } else
    if (m_data && (e->type() == eEventVCard)){
        EventVCard *evc = static_cast<EventVCard*>(e);
        JabberUserData *data = evc->data();
        if (m_data->getId() == data->getId() && m_data->getNode() == data->getNode())
            fill(data);
    }
    return false;
}

void JabberHomeInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtStreet->setPlainText(data->getStreet());
    edtExt->setPlainText(data->getExtAddr());
    edtCity->setText(data->getCity());
    edtState->setText(data->getRegion());
    edtZip->setText(data->getPCode());
    edtCountry->setText(data->getCountry());
}

void JabberHomeInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = m_client->toJabberUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
    data->setStreet(edtStreet->toPlainText());
    data->setExtAddr(edtExt->toPlainText());
    data->setCity(edtCity->text());
    data->setRegion(edtState->text());
    data->setPCode(edtZip->text());
    data->setCountry(edtCountry->text());
}

