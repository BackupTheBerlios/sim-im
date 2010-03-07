/***************************************************************************
                          ignorelist.cpp  -  description
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
#include "log.h"
#include "misc.h"

#include "ignorelist.h"
#include "contacts/contact.h"

#include <QPixmap>

using namespace SIM;

unsigned CmdListUnignore = 0x130001;

IgnoreList::IgnoreList(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
    Command cmd;
    cmd->id          = CmdListUnignore;
    cmd->text        = I18N_NOOP("Unignore");
    cmd->icon        = QString::null;
    cmd->accel       = QString::null;
    cmd->bar_id      = 0;
    cmd->menu_id     = MenuListView;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    lstIgnore->addColumn(i18n("Contact"));
    lstIgnore->addColumn(i18n("Name"));
    lstIgnore->addColumn(i18n("EMail"));
    connect(lstIgnore, SIGNAL(deleteItem(ListViewItem*)), this, SLOT(deleteItem(ListViewItem*)));
    connect(lstIgnore, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(lstIgnore, SIGNAL(dragEnter(QMimeSource*)), this, SLOT(dragEnter(QMimeSource*)));
    connect(lstIgnore, SIGNAL(drop(QMimeSource*)), this, SLOT(drop(QMimeSource*)));
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        if (!contact->getIgnore())
            continue;
        ListViewItem *item = new ListViewItem(lstIgnore);
        updateItem(item, contact);
    }
}

bool IgnoreList::processEvent(Event *e)
{
    switch (e->type()){
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                removeItem(findItem(contact));
                break;
            }
            case EventContact::eCreated: {
                ListViewItem *item = findItem(contact);
                if (contact->getIgnore()){
                    if (item == NULL)
                        item = new ListViewItem(lstIgnore);
                    updateItem(item, contact);
                }else{
                    removeItem(item);
                }
                break;
            }
            case EventContact::eChanged: {
                if(contact->getIgnore()) {
                    ListViewItem *item = findItem(contact);
                    if(!item) {
                        if (item == NULL)
                            item = new ListViewItem(lstIgnore);
                        updateItem(item, contact);
                    }
                } else {
                    ListViewItem *item = findItem(contact);
                    removeItem(item);
                }
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdListUnignore) && (cmd->menu_id == MenuListView)){
            ListViewItem *item = (ListViewItem*)(cmd->param);
            if (item->listView() == lstIgnore){
                unignoreItem(item);
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void IgnoreList::updateItem(ListViewItem *item, Contact *contact)
{
    QString name = contact->getName();
    QString firstName = contact->getFirstName();
    QString lastName = contact->getLastName();
    firstName = getToken(firstName, '/');
    lastName = getToken(lastName, '/');
    if (!lastName.isEmpty()){
        if (!firstName.isEmpty())
            firstName += ' ';
        firstName += lastName;
    }
    QString mail;
    QString mails = contact->getEMails();
    while (mails.length()){
        QString mailItem = getToken(mails, ';');
        if (mail.length())
            mail += ',';
        mail += getToken(mailItem, '/');
    }
    unsigned style;
    QString statusIcon;
    contact->contactInfo(style, statusIcon);
    item->setText(0, name);
    item->setText(1, firstName);
    item->setText(2, mail);
    item->setText(3, QString::number(contact->id()));
    item->setPixmap(0, Pict(statusIcon));
}

ListViewItem *IgnoreList::findItem(Contact *contact)
{
	for(int c = 0; c < lstIgnore->topLevelItemCount(); c++)
	{
		ListViewItem *item = static_cast<ListViewItem*>(lstIgnore->topLevelItem(c));
		if (item->text(3).toUInt() == contact->id())
			return item;
	}
	return NULL;
}

void IgnoreList::unignoreItem(ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact) {
        contact->setIgnore(false);
        EventContact(contact, EventContact::eChanged).process();
    }
}

void IgnoreList::deleteItem(ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact) {
        EventContact(contact,EventContact::eDeleted).process();
    }
}

void IgnoreList::dragStart()
{
    ListViewItem *item = lstIgnore->currentItem();
    if (item == NULL)
        return;
//    Contact *contact = getContacts()->contact(item->text(3).toUInt());
//    if (contact)
//        lstIgnore->startDrag(new ContactDragObject(lstIgnore, contact));
}

void IgnoreList::dragEnter(QMimeSource *s)
{
//    if (ContactDragObject::canDecode(s)){
//        Contact *contact = ContactDragObject::decode(s);
//        if (contact){
//            if (!contact->getIgnore()){
//                log(L_DEBUG, "Set true");
//                lstIgnore->acceptDrop(true);
//                return;
//            }
//        }
//    }
//    log(L_DEBUG, "Set false");
//    lstIgnore->acceptDrop(false);
}

void IgnoreList::drop(QMimeSource *s)
{
    if (ContactDragObject::canDecode(s)){
        Contact *contact = ContactDragObject::decode(s);
        if (contact){
            if (!contact->getIgnore()){
                contact->setIgnore(true);
                EventContact(contact, EventContact::eChanged).process();
                return;
            }
        }
    }
}

void IgnoreList::removeItem(ListViewItem *item)
{
    if (item == NULL)
        return;
	/*
    ListViewItem *nextItem = NULL;
    if (item == lstIgnore->currentItem()){
        nextItem = item->nextSibling();
        if (nextItem == NULL){
            for (nextItem = lstIgnore->firstChild(); nextItem; nextItem = nextItem->nextSibling())
                if (nextItem->nextSibling() == item)
                    break;
        }
    }
	*/
    delete item;
    //if (nextItem)
     //   lstIgnore->setCurrentItem(nextItem);
}

