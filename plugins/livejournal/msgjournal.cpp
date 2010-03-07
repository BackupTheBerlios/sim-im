/***************************************************************************
                          msgjournal.cpp  -  description
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

#include "msgjournal.h"
#include "livejournal.h"
#include "msgedit.h"
#include "userwnd.h"

#include "simgui/ballonmsg.h"
#include "simgui/toolbtn.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"

#include <QLayout>
#include <QLineEdit>
#include <QComboBox>

using namespace SIM;

MsgJournal::MsgJournal(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit  = parent;
    m_wnd   = new MsgJournalWnd(m_edit);
    connect(m_wnd, SIGNAL(finished()), this, SLOT(frameDestroyed()));
    m_edit->m_layout->insertWidget(0, m_wnd);
    m_wnd->show();
    JournalMessage	*m = static_cast<JournalMessage*>(msg);
    m_ID	= m->getID();
    m_oldID = m->id();
    m_time  = m->getTime();
    m_wnd->edtSubj->setText(m->getSubject());
    m_wnd->cmbSecurity->setCurrentIndex(m->getPrivate());
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact){
        clientData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = ++it) != NULL){
            if ((m_client.isEmpty() && (data->Sign.toULong() == LIVEJOURNAL_SIGN)) ||
                (m_client == it.client()->dataName(data))){
                LiveJournalClient *client = static_cast<LiveJournalClient*>(it.client());
                for (unsigned i = 1; i < client->getMoods(); i++){
                    const QString mood = client->getMood(i);
                    if (mood.isEmpty())
                        continue;
                    QString s = mood;
                    QString ts = i18n(mood);
                    if (s != ts){
                        s += " (";
                        s += ts;
                        s += ")";
                    }
                    m_wnd->cmbMood->insertItem(INT_MAX,s);
                }
                m_wnd->cmbMood->setCurrentIndex(static_cast<JournalMessage*>(msg)->getMood());
                m_wnd->cmbMood->setMinimumSize(m_wnd->cmbMood->sizeHint());
                break;
            }
        }
    }
    m_wnd->cmbComment->setCurrentIndex(m->getComments());
    QString text = msg->getRichText();
    if (!text.isEmpty()){
        m_edit->m_edit->setText(text);
        m_edit->m_edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        CorePlugin *core = GET_CorePlugin();
        if ((msg->getBackground() != msg->getForeground()) && !core->property("OwnColors").toBool()){
            m_edit->m_edit->setBackground(msg->getBackground());
            m_edit->m_edit->setForeground(msg->getForeground(), true);
        }
    }
    connect(m_edit->m_edit, SIGNAL(emptyChanged(bool)), this, SLOT(emptyChanged(bool)));
    emptyChanged(m_edit->m_edit->isEmpty());
    m_edit->m_edit->setParam(m_edit);
}

MsgJournal::~MsgJournal()
{
    if (m_wnd)
        delete m_wnd;
}

void MsgJournal::init()
{
    m_wnd->edtSubj->setFocus();
}

void MsgJournal::emptyChanged(bool bEmpty)
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = bEmpty ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();
}

bool MsgJournal::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if ((cmd->id == CmdDeleteJournalMessage + CmdReceived) && m_ID)
                    cmd->flags &= ~BTN_HIDE;
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
        if (cmd->param == m_edit){
            if (cmd->id == CmdSend){
                QString msgText = m_edit->m_edit->toHtml();
                if (!msgText.isEmpty())
                    send(msgText);
                return true;
            }
            if (cmd->id == CmdDeleteJournalMessage + CmdReceived){
                QWidget *w = m_edit->m_bar;
                Command cmd;
                cmd->id		= CmdDeleteJournalMessage + CmdReceived;
                cmd->param	= m_edit;
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                QWidget *btnRemove = eWidget.widget();
                if (btnRemove)
                    w = btnRemove;
                BalloonMsg::ask(NULL, i18n("Remove record from journal?"), w, SLOT(removeRecord(void*)), NULL, NULL, this);
                return true;
            }
            return false;
        }
    }
    return false;
}

void MsgJournal::removeRecord(void*)
{
    send(QString::null);
}

void MsgJournal::send(const QString& msgText)
{
    JournalMessage *msg = new JournalMessage;
    msg->setText(msgText);
    msg->setContact(m_edit->m_userWnd->id());
    msg->setClient(m_client);
    msg->setFlags(MESSAGE_RICHTEXT);
    msg->setID(m_ID);
    msg->setOldID(m_oldID);
    msg->setTime(m_time);
    msg->setForeground(m_edit->m_edit->foreground().rgb() & 0xFFFFFF);
    msg->setBackground(m_edit->m_edit->background().rgb() & 0xFFFFFF);
    CorePlugin *core = GET_CorePlugin();
    msg->setFont(core->property("EditFont").toString());
    msg->setSubject(m_wnd->edtSubj->text());
    msg->setPrivate(m_wnd->cmbSecurity->currentIndex());
    msg->setMood(m_wnd->cmbMood->currentIndex());
    msg->setComments(m_wnd->cmbComment->currentIndex());

    EventRealSendMessage(msg, m_edit).process();
}

void MsgJournal::frameDestroyed()
{
    m_wnd = NULL;
}

MsgJournalWnd::MsgJournalWnd(QWidget *parent)
        : QWidget(parent)
{
	setupUi(this);
}

MsgJournalWnd::~MsgJournalWnd()
{
    finished();
}

