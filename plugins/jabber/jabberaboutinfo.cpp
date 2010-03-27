/***************************************************************************
                          jabberaboutinfo.cpp  -  description
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
#include "jabberaboutinfo.h"
#include "jabber.h"
#include "contacts/contact.h"

using namespace SIM;

JabberAboutInfo::JabberAboutInfo(QWidget *parent, JabberUserData *data, JabberClient *client) : QWidget(parent)
        //: JabberAboutInfoBase(parent)
{
	setupUi(this);
    m_client  = client;
    m_data    = data;
    if (m_data)
        edtAbout->setReadOnly(true);
    fill(m_data);
}

void JabberAboutInfo::apply()
{
}

bool JabberAboutInfo::processEvent(Event *e)
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

void JabberAboutInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtAbout->setPlainText(data->getDesc());
}

void JabberAboutInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = m_client->toJabberUserData((SIM::IMContact*)_data); // FIXME unsafe type conversion
    data->setDesc(edtAbout->toPlainText());
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabberaboutinfo.moc"
#endif
*/

