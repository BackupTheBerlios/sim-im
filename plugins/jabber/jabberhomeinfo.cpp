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
        if (contact->clientData.have(m_data))
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
        if (m_data->ID.str() == data->ID.str() && m_data->Node.str() == data->Node.str())
            fill(data);
    }
    return false;
}

void JabberHomeInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtStreet->setPlainText(data->Street.str());
    edtExt->setPlainText(data->ExtAddr.str());
    edtCity->setText(data->City.str());
    edtState->setText(data->Region.str());
    edtZip->setText(data->PCode.str());
    edtCountry->setText(data->Country.str());
}

void JabberHomeInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = m_client->toJabberUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->Street.str() = edtStreet->toPlainText();
    data->ExtAddr.str() = edtExt->toPlainText();
    data->City.str()    = edtCity->text();
    data->Region.str()  = edtState->text();
    data->PCode.str()   = edtZip->text();
    data->Country.str() = edtCountry->text();
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabberhomeinfo.moc"
#endif
*/

