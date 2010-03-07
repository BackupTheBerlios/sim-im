/***************************************************************************
                          msggen.cpp  -  description
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

#include "msgauth.h"
#include "msgedit.h"
#include "simgui/textshow.h"
#include "userwnd.h"
#include "core.h"

#include "simgui/toolbtn.h"

#include <QTimer>
#include <QToolButton>

using namespace SIM;

MsgAuth::MsgAuth(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_type   = msg->type();
    m_edit   = parent;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText(QString::null);
        m_edit->m_edit->setReadOnly(false);
    }
    QString text = msg->getPlainText();
    if (!text.isEmpty())
        parent->m_edit->setText(text);
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = 0;
    cmd->param = parent;
    EventCommandChecked(cmd).process();
}

void MsgAuth::init()
{
    m_edit->m_edit->setFocus();
}

bool MsgAuth::processEvent(Event *e)
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
            case CmdTranslit:
            case CmdSmile:
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return true;
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
            AuthMessage *msg = new AuthMessage(m_type);
            msg->setText(msgText);
            msg->setContact(m_edit->m_userWnd->id());
            msg->setClient(m_client);
            m_edit->sendMessage(msg);
            return true;
        }
    }
    return false;
}

/*
#ifndef NO_MOC_INCLUDES
#include "msgauth.moc"
#endif
*/

