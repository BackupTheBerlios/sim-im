/***************************************************************************
                          icqinfo.cpp  -  description
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
#include "icqinfo.h"
#include "icqclient.h"
#include "simgui/ballonmsg.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QStringList>
#include <QComboBox>
#include <QPixmap>
#include <QLabel>
#include <QTabWidget>

using namespace SIM;

const ext_info chat_groups[] =
    {
        { I18N_NOOP("General chat"), 1 },
        { I18N_NOOP("Romance"), 2 },
        { I18N_NOOP("Games"), 3 },
        { I18N_NOOP("Students"), 4 },
        { I18N_NOOP("20 Something"), 5 },
        { I18N_NOOP("30 Something"), 6 },
        { I18N_NOOP("40 Something"), 7 },
        { I18N_NOOP("50 Plus"), 8 },
        { I18N_NOOP("Seeking Women"), 9 },
        { I18N_NOOP("Seeking Men"), 10 },
        { "", 0 }
    };

const ext_info *p_chat_groups = chat_groups;

ICQInfo::ICQInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_client	= client;
    m_data		= data;
    m_contact	= contact;
    edtUin->setReadOnly(true);
    if (m_data){
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
        edtNick->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
        lblRandom->hide();
        cmbRandom->hide();
        tabWnd->removeTab(tabWnd->indexOf(password));
    }else{
        edtAutoReply->hide();
        connect(this, SIGNAL(raise(QWidget*)), topLevelWidget(), SLOT(raisePage(QWidget*)));
    }
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtExtIP->setReadOnly(true);
    edtIntIP->setReadOnly(true);
    edtClient->setReadOnly(true);
    fill();
}

void ICQInfo::apply()
{
    ICQUserData *data = m_data;
    if (data == NULL){
        if (m_client->getState() == Client::Connected){
            QString errMsg;
            QWidget *errWidget = edtCurrent;
            if (!edtPswd1->text().isEmpty() || !edtPswd2->text().isEmpty()){
                if (edtCurrent->text().isEmpty()){
                    errMsg = i18n("Input current password");
                }else{
                    if (edtPswd1->text() != edtPswd2->text()){
                        errMsg = i18n("Confirm password does not match");
                        errWidget = edtPswd2;
                    }else if (edtCurrent->text() != m_client->getPassword()){
                        errMsg = i18n("Invalid password");
                    }
                }
            }
            if (!errMsg.isEmpty()){
                for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
                    if (p->inherits("QTabWidget")){
                        static_cast<QTabWidget*>(p)->setCurrentWidget(this);
                        break;
                    }
                }
                emit raise(this);
                BalloonMsg::message(errMsg, errWidget);
                return;
            }
            if (!edtPswd1->text().isEmpty())
                m_client->changePassword(edtPswd1->text());
            // clear Textboxes
            edtCurrent->clear();
            edtPswd1->clear();
            edtPswd2->clear();
        }
        m_data = &m_client->data.owner;
        m_client->setRandomChatGroup(getComboValue(cmbRandom, chat_groups));
    }
}

void ICQInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::clientData*)_data);  // FIXME unsafe type conversion
    data->FirstName.str() = edtFirst->text();
    data->LastName.str()  = edtLast->text();
    data->Nick.str()      = edtNick->text();
}

bool ICQInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    } else
    if ((e->type() == eEventMessageReceived) && m_data){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->type() == MessageStatus){
            if (m_client->dataName(m_data) == msg->client())
                fill();
        }
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

void ICQInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;

    edtUin->setText(QString::number(data->Uin.toULong()));
    edtFirst->setText(data->FirstName.str());
    edtLast->setText(data->LastName.str());
    edtNick->setText(data->Nick.str());

    if (m_data == NULL){
        if (edtFirst->text().isEmpty()) {
            QString firstName = getContacts()->owner()->getFirstName();
            firstName = getToken(firstName, '/');
            edtFirst->setText(firstName);
        }
        if (edtLast->text().isEmpty()) {
            QString lastName = getContacts()->owner()->getLastName();
            lastName = getToken(lastName, '/');
            edtLast->setText(lastName);
        }
        password->setEnabled(m_client->getState() == Client::Connected);
    }

    cmbStatus->clear();
    unsigned status = STATUS_ONLINE;
    if (m_data){
        unsigned s = m_data->Status.toULong();
        if (s == ICQ_STATUS_OFFLINE){
            status = STATUS_OFFLINE;
        }else if (s & ICQ_STATUS_DND){
            status = STATUS_DND;
        }else if (s & ICQ_STATUS_OCCUPIED){
            status = STATUS_OCCUPIED;
        }else if (s & ICQ_STATUS_NA){
            status = STATUS_NA;
        }else if (s & ICQ_STATUS_AWAY){
            status = STATUS_AWAY;
        }else if (s & ICQ_STATUS_FFC){
            status = STATUS_FFC;
        }
    }else{
        status = m_client->getStatus();
        initCombo(cmbRandom, m_client->getRandomChatGroup(), chat_groups);
    }
    if ((status != STATUS_ONLINE) && (status != STATUS_OFFLINE) && m_data){
        edtAutoReply->setPlainText(m_data->AutoReply.str());
    }else{
        edtAutoReply->hide();
    }

    int current = 0;
    QString text;
    if (m_data && (status == STATUS_OFFLINE) && m_data->bInvisible.toBool()){
        cmbStatus->addItem(Pict("ICQ_invisible"), i18n("Possibly invisible"));
    }else{
		ProtocolPtr proto = ICQPlugin::icq_plugin->m_icq;
		ICQProtocol* icq = static_cast<ICQProtocol*>(proto.data());
        for (const CommandDef *cmd = icq->statusList(); cmd->id; cmd++){
            if (cmd->flags & COMMAND_CHECK_STATE)
                continue;
            if (status == cmd->id){
                current = cmbStatus->count();
                text = cmd->text;
            }
            cmbStatus->addItem(Pict(cmd->icon), i18n(cmd->text));
        }
    }
    cmbStatus->setCurrentIndex(current);
    disableWidget(cmbStatus);
    if (status == STATUS_OFFLINE){
        lblOnline->setText(i18n("Last online") + ':');
        edtOnline->setText(formatDateTime(data->StatusTime.toULong()));
        lblNA->hide();
        edtNA->hide();
    }else{
        if (data->OnlineTime.toULong()){
            edtOnline->setText(formatDateTime(data->OnlineTime.toULong()));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || text.isEmpty()){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(data->StatusTime.toULong()));
        }
    }
    if (data->IP.ip()){
        edtExtIP->setText(formatAddr(data->IP, data->Port.toULong()));
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if ((data->RealIP.ip()) && ((data->IP.ip() == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        edtIntIP->setText(formatAddr(data->RealIP, data->Port.toULong()));
    }else{
        lblIntIP->hide();
        edtIntIP->hide();
    }
    if (m_data){
        QString client_name = m_client->clientName(data);
        if (client_name.length()){
            edtClient->setText(client_name);
        }else{
            lblClient->hide();
            edtClient->hide();
        }
    }else{
        QString name = PACKAGE;
        name += ' ';
        name += VERSION;
#ifdef WIN32
        name += "/win32";
#endif
        edtClient->setText(name);
    }
}

