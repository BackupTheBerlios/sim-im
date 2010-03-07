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

#include "msggen.h"

#include "simapi.h"
#include "log.h"

#include "msgedit.h"
#include "userwnd.h"

using namespace SIM;

MsgGen::MsgGen(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText(QString::null);
        m_edit->m_edit->setReadOnly(false);
    }
    if (msg->getFlags() & MESSAGE_INSERT){
        QString text = msg->getPlainText();
        //m_edit->m_edit->insert(text, false, true, true); //FIXME
        m_edit->m_edit->insertHtml(text);
    }else{
        QString text = msg->getRichText();
        if (!text.isEmpty()){
            m_edit->m_edit->setText(text);
            m_edit->m_edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::instance()->value("OwnColors").toBool()){
                m_edit->m_edit->setBackground(msg->getBackground());
                m_edit->m_edit->setForeground(msg->getForeground(), true);
            }
        }
    }
    connect(m_edit->m_edit, SIGNAL(emptyChanged(bool)), this, SLOT(emptyChanged(bool)));
    emptyChanged(m_edit->m_edit->isEmpty());
}

void MsgGen::init()
{
    if (m_edit->topLevelWidget()->isActiveWindow() && !m_edit->topLevelWidget()->isMinimized())
        m_edit->m_edit->setFocus();
}

void MsgGen::emptyChanged(bool bEmpty)
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = bEmpty ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();
}

bool MsgGen::processEvent(Event *e)
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
            QString msgText = m_edit->m_edit->toHtml();
            if (!msgText.isEmpty())
			{
				msgText.remove(QRegExp("^<!DOCTYPE[^>]*>"));
				msgText.remove(QRegExp("<style[^>]*>[^<]*</style>"));
                log(L_DEBUG, "Send: %s", qPrintable(msgText));
                Message *msg = new Message;
                msg->setText(msgText);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client);
                msg->setFlags(MESSAGE_RICHTEXT);
                msg->setForeground(m_edit->m_edit->foreground().rgb() & 0xFFFFFF);
                msg->setBackground(m_edit->m_edit->background().rgb() & 0xFFFFFF);
                msg->setFont(CorePlugin::instance()->value("EditFont").toString());
                m_edit->sendMessage(msg);
            }
            return true;
        }
    }
    return false;
}

// vim: set expandtab:


