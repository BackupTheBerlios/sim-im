/***************************************************************************
                          msgcontacts.cpp  -  description
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

#include "msgcontacts.h"
#include "msgedit.h"

#include "userwnd.h"
#include "userlist.h"
#include "core.h"
#include "contacts/contact.h"
#include "simgui/textshow.h"
#include "simgui/toolbtn.h"

#include <QLayout>

using namespace std;
using namespace SIM;

MsgContacts::MsgContacts(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit = parent;
    m_list = new UserList(m_edit);
    m_edit->m_layout->addWidget(m_list);
    connect(m_list, SIGNAL(selectChanged()), this, SLOT(changed()));
    ContactsMessage *m = static_cast<ContactsMessage*>(msg);
    QString contacts = m->getContacts();
    while (contacts.length()){
        QString item = getToken(contacts, ';');
        QString url = getToken(item, ',');
        QString proto = getToken(url, ':');
        if (proto == "sim"){
            unsigned contact_id = url.toULong();
            if (getContacts()->contact(contact_id))
                m_list->select( contact_id );
        }
    }
    changed();
    connect(m_edit, SIGNAL(finished()), this, SLOT(editFinished()));
    connect(m_list, SIGNAL(finished()), this, SLOT(listFinished()));
}

MsgContacts::~MsgContacts()
{
    if (m_edit && m_edit->m_edit)
        m_edit->m_edit->show();
    if (m_list)
        delete m_list;
}

void MsgContacts::editFinished()
{
    m_edit = NULL;
}

void MsgContacts::listFinished()
{
    m_list = NULL;
}

void MsgContacts::changed()
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = m_list->isHaveSelected() ? 0 : COMMAND_DISABLED;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();
}

void MsgContacts::init()
{
    m_edit->m_edit->hide();
    m_list->show();
    m_list->setFocus();
}

bool MsgContacts::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                return true;
            }
            switch (cmd->id){
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return true;
            case CmdTranslit:
            case CmdSmile:
            case CmdNextMessage:
            case CmdMsgAnswer:
                e->process(this);
                cmd->flags |= BTN_HIDE;
                return true;
            }
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->toPlainText();
            QString contacts;
            QList< unsigned int > listSelected = m_list->selected();
            foreach( unsigned int id, listSelected ) {
                Contact *contact = getContacts()->contact( id );
                if (contact){
                    if (!contacts.isEmpty())
                        contacts += ';';
                    contacts += QString("sim:%1,%2") .arg( id ) .arg( contact->getName() );
                }
            }
            if (!contacts.isEmpty()){
                ContactsMessage *msg = new ContactsMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setContacts(contacts);
                msg->setClient(m_client);
                m_edit->sendMessage(msg);
            }
            return true;
        }
    }
    return false;
}


/*
#ifndef NO_MOC_INCLUDES
#include "msgcontacts.moc"
#endif
*/

