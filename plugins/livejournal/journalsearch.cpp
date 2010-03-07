/***************************************************************************
                          msnsearch.cpp  -  description
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

#include "journalsearch.h"
#include "livejournal.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QShowEvent>

using namespace SIM;

JournalSearch::JournalSearch(LiveJournalClient *client, QWidget *parent) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
}

void JournalSearch::showEvent(QShowEvent *e)
{
	showEvent(e);
    emit setAdd(true);
}

void JournalSearch::createContact(unsigned tmpFlags, Contact *&contact)
{
    if (edtCommunity->text().isEmpty())
        return;
    if (m_client->findContact(edtCommunity->text(), contact, false))
        return;
    m_client->findContact(edtCommunity->text(), contact, true, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}

