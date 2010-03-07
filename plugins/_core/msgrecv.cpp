/***************************************************************************
                          msgrecv.cpp  -  description
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

#include "msgrecv.h"
#include "msgedit.h"
#include "msgview.h"
#include "history.h"
#include "core.h"

#include "simgui/toolbtn.h"
#include "simgui/textshow.h"

#include <QToolTip>
#include <QTimer>
#include <QLayout>

using namespace std;
using namespace SIM;

MsgReceived::MsgReceived(MsgEdit *parent, Message *msg, bool bOpen)
        : QObject(parent), EventReceiver(HighPriority - 1)
{
    m_id	  = msg->id();
    m_contact = msg->contact();
    m_client  = msg->client();
    m_edit    = parent;
    m_bOpen	  = bOpen;
    m_msg	  = msg;
    m_type	  = msg->baseType();

    if (m_bOpen){
        m_edit->m_edit->setReadOnly(true);
        QString p = msg->presentation();
        if (p.isEmpty())
            p = msg->getRichText();
        EventAddHyperlinks e(p);
        e.process();
        p = MsgViewBase::parseText(e.text(), CorePlugin::instance()->value("OwnColors").toBool(), CorePlugin::instance()->value("UseSmiles").toBool());
        m_edit->m_edit->setText(p);
        if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::instance()->value("OwnColors").toBool()){
            m_edit->m_edit->setBackground(msg->getBackground());
            m_edit->m_edit->setForeground(msg->getForeground(), true);
        }
        for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
            if ((it->id == msg->id()) &&
                    (it->contact == msg->contact()) &&
                    (it->client == msg->client())){
                CorePlugin::instance()->unread.erase(it);
                EventMessageRead(msg).process();
                break;
            }
        }
        m_edit->setupNext();
    }else{
        connect(m_edit->m_edit, SIGNAL(textChanged()), m_edit, SLOT(setInput()));
    }
}

bool MsgReceived::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        unsigned id = cmd->bar_grp;
        if (cmd->param == m_edit){
            MessageDef *mdef = NULL;
            CommandDef *msgCmd = CorePlugin::instance()->messageTypes.find(m_type);
            if (msgCmd)
                mdef = (MessageDef*)(msgCmd->param);
            if (mdef && mdef->cmdReceived){
                for (const CommandDef *d = mdef->cmdReceived; !d->text.isEmpty(); d++){
                    if (d->popup_id && (d->popup_id == cmd->menu_id)){
                        Message *msg = History::load(m_id, m_client, m_contact);
                        if (msg){
                            CommandDef c = *cmd;
                            c.param = msg;
                            m_edit->execCommand(&c);
                        }
                        return true;
                    }
                }
            }

            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                Message *msg = History::load(m_id, m_client, m_contact);
                if (msg){
                    CommandDef c = *cmd;
                    c.id   -= CmdReceived;
                    c.param = msg;
                    m_edit->execCommand(&c);
                }
                return true;
            }
        }
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= 0x1000) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                switch (cmd->id - CmdReceived){
                case CmdMsgQuote:
                case CmdMsgForward:{
                        CommandDef c = *cmd;
                        Message *msg = m_msg;
                        if (msg == NULL)
                            msg = History::load(m_id, m_client, m_contact);
                        if (msg){
                            c.id   -= CmdReceived;
                            c.param = msg;
                            if (EventCheckCommandState(&c).process())
                                cmd->flags &= ~BTN_HIDE;
                            if (m_msg == NULL)
                                delete msg;
                        }
                        return true;
                    }
                }
                MessageDef *mdef = NULL;
                CommandDef *msgCmd = CorePlugin::instance()->messageTypes.find(m_type);
                if (msgCmd)
                    mdef = (MessageDef*)(msgCmd->param);
                if (mdef && mdef->cmdReceived){
                    for (const CommandDef *d = mdef->cmdReceived; !d->text.isEmpty(); d++){
                        if (d->id + CmdReceived == cmd->id){
                            if (d->flags & COMMAND_CHECK_STATE){
                                Message *msg = m_msg;
                                if (msg == NULL)
                                    msg = History::load(m_id, m_client, m_contact);
                                if (msg){
                                    CommandDef c = *d;
                                    c.param = msg;
                                    if (EventCheckCommandState(&c).process())
                                        cmd->flags &= ~BTN_HIDE;
                                    if (m_msg == NULL)
                                        delete msg;
                                }
                            }else{
                                cmd->flags &= ~BTN_HIDE;
                            }
                            return true;
                        }
                    }
                }
                return true;
            }
            if (cmd->id == CmdMsgAnswer){
                e->process(this);
                cmd->flags |= BTN_HIDE;
                if (CorePlugin::instance()->getContainerMode() == 0)
                    cmd->flags &= ~BTN_HIDE;
                return true;
            }

            if (m_bOpen){
                switch (cmd->id){
                case CmdTranslit:
                case CmdSmile:
                case CmdSend:
                case CmdSendClose:
                    e->process(this);
                    cmd->flags |= BTN_HIDE;
                    return true;
                case CmdNextMessage:
                    e->process(this);
                    cmd->flags |= BTN_HIDE;
                    if (CorePlugin::instance()->getContainerMode() == 0)
                        cmd->flags &= ~BTN_HIDE;
                    return true;
                }
            }
        }
    } else
    if (e->type() == eEventMessageDeleted){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->id() == m_id)
            QTimer::singleShot(0, m_edit, SLOT(goNext()));
    }
    return false;
}

void MsgReceived::init()
{
    m_msg = NULL;
}

/*
#ifndef NO_MOC_INCLUDES
#include "msgrecv.moc"
#endif
*/

