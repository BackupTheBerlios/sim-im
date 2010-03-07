/***************************************************************************
                          msgsms.cpp  -  description
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

#include "msgsms.h"
#include "msgedit.h"
#include "userwnd.h"
#include "core.h"
#include "contacts/contact.h"
#include "simgui/toolbtn.h"
#include "simgui/textshow.h"

#include <QCheckBox>

using namespace SIM;

const unsigned MAX_SMS_LEN_LATIN1	= 160;
const unsigned MAX_SMS_LEN_UNICODE	= 70;

MsgSMS::MsgSMS(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_edit     = parent;
    m_bExpand  = false;
    m_bCanSend = false;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText(QString::null);
        m_edit->m_edit->setReadOnly(false);
    }
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    m_panel	= NULL;
    Command cmd;
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolCombo *cmbPhone = qobject_cast<CToolCombo*>(eWidget.widget());
    if (cmbPhone)
        connect(cmbPhone->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact == NULL)
        return;
    if (cmbPhone){
        QString phones = contact->getPhones();
        while (phones.length()){
            QString phoneItem = getToken(phones, ';', false);
            phoneItem = getToken(phoneItem, '/', false);
            QString phone = getToken(phoneItem, ',');
            getToken(phoneItem, ',');
            if (phoneItem.toUInt() == CELLULAR)
                cmbPhone->addItem(phone);
        }
        t = static_cast<SMSMessage*>(msg)->getPhone();
        if (!t.isEmpty())
            cmbPhone->setText(t);
    }
    textChanged();
	SIM::PropertyHubPtr data = contact->getUserData("SMS");
    if (contact->getFlags() & CONTACT_TEMP){
        m_panel = new SMSPanel(m_edit);
        m_edit->m_layout->insertWidget(0, m_panel);
        connect(m_panel, SIGNAL(destroyed()), this, SLOT(panelDestroyed()));
        m_panel->show();
    }
    if (m_edit->m_edit->toPlainText().isEmpty()){
        EventTemplate::TemplateExpand t;
        if (!data->value("SMSSignatureBefore").toString().isEmpty()){
            t.tmpl = data->value("SMSSignatureBefore").toString();
            t.contact  = contact;
            t.receiver = this;
            t.param    = NULL;
            EventTemplateExpand(&t).process();
        }else{
            m_bExpand = true;
            if (!data->value("SMSSignatureAfter").toString().isEmpty()){
                t.tmpl = data->value("SMSSignatureAfter").toString();
                t.contact = contact;
                t.receiver = this;
                t.param = NULL;
                EventTemplateExpand(&t).process();
            }
        }
    }
}

MsgSMS::~MsgSMS()
{
    m_edit->m_userWnd->setStatus(QString::null); // Clear "Size: %1 / Max. size: %2" from status line
    if (m_panel)
        delete m_panel;
}

void MsgSMS::panelDestroyed()
{
    m_panel = NULL;
}

void MsgSMS::init()
{
    if (!m_edit->topLevelWidget()->isActiveWindow() || m_edit->topLevelWidget()->isMinimized())
        return;
    Command cmd;
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolCombo *cmbPhone = qobject_cast<CToolCombo*>(eWidget.widget());
    if (cmbPhone && cmbPhone->lineEdit()->text().isEmpty()){
        cmbPhone->setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgSMS::textChanged(const QString&)
{
    textChanged();
}

void MsgSMS::textChanged()
{
    QString phone;
    QString msgText = m_edit->m_edit->toPlainText();
    Command cmd;
    cmd->id    = CmdTranslit;
    cmd->param = m_edit;
    EventCommandWidget eWidget1(cmd);
    eWidget1.process();
    CToolButton *btnTranslit = qobject_cast<CToolButton*>(eWidget1.widget());
    if (btnTranslit && btnTranslit->isChecked())
        msgText = toTranslit(msgText);
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    EventCommandWidget eWidget2(cmd);
    eWidget2.process();
    CToolCombo *cmbPhone = qobject_cast<CToolCombo*>(eWidget2.widget());
    if (cmbPhone)
        phone = cmbPhone->lineEdit()->text();
    bool bCanSend = !phone.isEmpty() || !msgText.isEmpty();
    if (bCanSend != m_bCanSend){
        m_bCanSend = bCanSend;
        cmd->id    = CmdSend;
        cmd->flags = m_bCanSend ? 0 : COMMAND_DISABLED;
        EventCommandDisabled(cmd).process();
    }
    unsigned size = msgText.length();
    unsigned max_size = MAX_SMS_LEN_UNICODE;
    if (isLatin(msgText))
        max_size = MAX_SMS_LEN_LATIN1;
    QString status = i18n("Size: %1 / Max. size: %2")
                     .arg(size) .arg(max_size);
    if (size > max_size){
        status += " ! ";
        status += i18n("Message will be split");
    }
    m_edit->m_userWnd->setStatus(status);
}

bool MsgSMS::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if (cmd->id == CmdPhoneNumber)
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
    if (e->type() == eEventTemplateExpanded){
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        if (m_bExpand){
            m_edit->m_edit->append(t->tmpl);
        }else{
            m_edit->m_edit->setText(t->tmpl);
            m_edit->m_edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            m_bExpand = true;
            Contact *contact = getContacts()->contact(m_id);
            if (contact){
				SIM::PropertyHubPtr data = contact->getUserData("SMS");
                if (!data->value("SMSSignatureAfter").toString().isEmpty()){
                    t->tmpl = data->value("SMSSignatureAfter").toString();
                    EventTemplateExpand(t).process();
                }
            }
        }
        return true;
    }
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            unsigned flags = 0;
            QString msgText = m_edit->m_edit->toPlainText();
            QString phone;
            Command c;
            c->id    = CmdPhoneNumber;
            c->param = m_edit;
            EventCommandWidget eWidget(c);
            eWidget.process();
            CToolCombo *cmbPhone = qobject_cast<CToolCombo*>(eWidget.widget());
            if (cmbPhone)
                phone = cmbPhone->lineEdit()->text();

            if (!msgText.isEmpty() && !phone.isEmpty()){
                SMSMessage *msg = new SMSMessage;
                msg->setText(msgText);
                msg->setFlags(flags);
                msg->setPhone(phone);
                msg->setContact(m_edit->m_userWnd->id());
                if (m_edit->sendMessage(msg)){
                    Contact *contact = getContacts()->contact(m_edit->m_userWnd->id());
                    if (contact){
                        if (contact->getFlags() & CONTACT_TEMP){
                            contact->setName(phone);
                            if (m_panel && m_panel->chkSave->isChecked()){
                                contact->setFlags(contact->getFlags() & ~CONTACT_TEMP);
                                delete m_panel;
                            }
                            EventContact(contact, EventContact::eChanged).process();
                        }
                        QString newPhones;
                        QString phones = contact->getPhones();
                        QString type = "Private Cellular";
                        QString src  = "-";
                        while (phones.length()){
                            QString phoneItem = getToken(phones, ';', false);
                            QString item = phoneItem;
                            QString phoneStr  = getToken(phoneItem, '/', false);
                            QString phone     = getToken(phoneStr, ',');
                            QString phoneType = getToken(phoneStr, ',');
                            if ((phone != msg->getPhone()) || (phoneStr.toUInt() != CELLULAR)){
                                if (!newPhones.isEmpty())
                                    newPhones += ';';
                                newPhones += item;
                                continue;
                            }
                            type = phoneType;
                            src  = phoneItem;
                        }
                        phone += ',';
                        phone += type;
                        phone += ',';
                        phone += QString::number(CELLULAR);
                        phone += '/';
                        phone += src;
                        if (!newPhones.isEmpty())
                            phone += ';';
                        newPhones = phone + newPhones;
						QString oldPhones = contact->getPhones();
						contact->setPhones(newPhones);
                        if (oldPhones != newPhones){
                            EventContact(contact, EventContact::eChanged).process();
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

SMSPanel::SMSPanel(QWidget *parent)
        : QFrame(parent)
		, lay(new QHBoxLayout(this))
{
    chkSave = new QCheckBox(i18n("Save phone in contact list"), this);
    lay->addSpacing(7);
    lay->addWidget(chkSave);
}

SMSPanel::~SMSPanel()
{}
