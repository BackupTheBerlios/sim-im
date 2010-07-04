/***************************************************************************
                          msgurl.cpp  -  description
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

#include "msgurl.h"
#include "msgedit.h"
#include "userwnd.h"
#include "core.h"

#include "simgui/toolbtn.h"
#include "simgui/textshow.h"


using namespace SIM;

MsgUrl::MsgUrl(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText(QString::null);
        m_edit->m_edit->setReadOnly(false);
    }
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    Command cmd;
    cmd->id    = CmdUrlInput;
    cmd->param = m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolEdit *edtUrl = qobject_cast<CToolEdit*>(eWidget.widget());
    if (edtUrl){
        connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
        edtUrl->setText(static_cast<UrlMessage*>(msg)->getUrl());
        if (edtUrl->text().isEmpty()){
            QString url;
            EventGetURL e;
            e.process();
            url = e.url();
            if (!url.isEmpty()){
                url = url.mid(1);
                int n = url.indexOf('\"');
                if (n > 0){
                    QString u = url.left(n);
                    edtUrl->setText(u);
                    url = url.mid(n + 1);
                    n = url.indexOf('\"');
                    if (n > 0)
                        url = url.mid(n + 1);
                }
                n = url.indexOf('\"');
                if (n > 0){
                    url = url.left(n);
                    m_edit->m_edit->setText(url);
                }
            }
        }
        urlChanged(edtUrl->text());
    }
}

void MsgUrl::init()
{
    if (!m_edit->topLevelWidget()->isActiveWindow() || m_edit->topLevelWidget()->isMinimized())
        return;
    Command cmd;
    cmd->id    = CmdUrlInput;
    cmd->param = m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolEdit *edtUrl = qobject_cast<CToolEdit*>(eWidget.widget());
    if (edtUrl && edtUrl->text().isEmpty()){
        edtUrl->setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgUrl::urlChanged(const QString &str)
{
    Command cmd;
    cmd->id = CmdSend;
    cmd->flags = str.isEmpty() ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();
}

bool MsgUrl::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if (cmd->id == CmdUrlInput)
                    cmd->flags &= ~BTN_HIDE;
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
            QString urlText;
            Command cmd;
            cmd->id    = CmdUrlInput;
            cmd->param = m_edit;
            EventCommandWidget eWidget(cmd);
            eWidget.process();
            CToolEdit *edtUrl = qobject_cast<CToolEdit*>(eWidget.widget());
            if (edtUrl)
                urlText = edtUrl->text();
            if (!urlText.isEmpty()){
                UrlMessage *msg = new UrlMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setText(msgText);
                msg->setUrl(urlText);
                msg->setClient(m_client);
                m_edit->sendMessage(msg);
            }
            return true;
        }
    }
    return false;
}
