/***************************************************************************
                          nonim.cpp  -  description
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

#include "contacts.h"

#include "nonim.h"
#include "contacts/contact.h"
#include "simgui/intedit.h"
#include <QShowEvent>

using namespace SIM;

NonIM::NonIM(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtMail->setValidator(new EMailValidator(edtMail));
    edtPhone->setValidator(new PhoneValidator(edtPhone));
}

void NonIM::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit setAdd(true);
}

void NonIM::add(Contact *&contact)
{
    contact = getContacts()->contact(0, true);
    contact->setFirstName(edtFirst->text());
    contact->setLastName(edtLast->text());
    if (!edtMail->text().isEmpty())
        contact->setEMails(edtMail->text() + "/-");
    if (!edtPhone->text().isEmpty())
        contact->setPhones(edtPhone->text() + "/-");
    QString nick = edtNick->text();
    if (nick.isEmpty()){
        nick = edtFirst->text();
        if (!nick.isEmpty() && !edtLast->text().isEmpty())
            nick += ' ';
        nick += edtLast->text();
    }
    if (nick.isEmpty())
        nick = edtMail->text();
    if (nick.isEmpty())
        nick = edtPhone->text();
    contact->setName(nick);
}

void NonIM::createContact(unsigned tmpFlags, Contact *&contact)
{
       add(contact);
       contact->setFlags(contact->getFlags() | tmpFlags);
}


/*
#ifndef NO_MOC_INCLUDES
#include "nonim.moc"
#endif
*/

