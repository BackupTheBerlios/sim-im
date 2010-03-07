/***************************************************************************
                          statuswnd.cpp  -  description
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
#include "statuswnd.h"
#include "core.h"
#include "log.h"

#include "contacts/client.h"
#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "simgui/ballonmsg.h"
#include "simgui/toolbtn.h"


#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QToolTip>
#include <QTimer>
#include <QFrame>
#include <QToolButton>
#include <QImage>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QMenu>

using namespace std;
using namespace SIM;

StatusLabel::StatusLabel(QWidget *parent, Client *client, unsigned id)
        : QLabel(parent)
{
    m_client = client;
    m_bBlink = false;
    m_id = id;
    m_blinkTimer = NULL;
    setPict();
}

void StatusLabel::startBlinkTimer()
{
    if (m_blinkTimer == NULL) {
        m_blinkTimer = new QTimer(this);
        connect(m_blinkTimer, SIGNAL(timeout()), this, SLOT(timeout()));
        m_blinkTimer->start(1000);
        m_bBlink = false;
    }
}

void StatusLabel::stopBlinkTimer()
{
    if (m_blinkTimer) {
        delete m_blinkTimer;
        m_blinkTimer = NULL;
    }
}

void StatusLabel::setPict()
{
    QIcon icon;
    QString text;
    if (m_client->getState() == Client::Connecting) {
        if (getSocketFactory()->isActive()) {
            IMStatusPtr status;
            startBlinkTimer();
            text = I18N_NOOP("Connecting");
            if(m_client->protocol()) {
                if (m_bBlink) {
                    status = m_client->currentStatus();
                } else {
                    status = m_client->protocol()->status("offline");
                }
                icon = status->icon();
            } else {
                icon = m_bBlink ? Icon("online") : Icon("offline");
            }
        } else {
            stopBlinkTimer();
            // TODO retreive appropriate icon
            icon = Icon("inactive");
            text = I18N_NOOP("Inactive");
        }
    }
    else
    {
        stopBlinkTimer();
        if (m_client->getState() == Client::Error) {
            icon = Icon("error");
            text = I18N_NOOP("Error");
        } else {
            icon = m_client->currentStatus()->icon();
            text = m_client->currentStatus()->name();
        }
    }
    QPixmap p = icon.pixmap(size());
    setPixmap(p);
    QString tip = CorePlugin::instance()->clientName(m_client);
    tip += '\n';
    tip += i18n(text);
    setToolTip(tip);
    resize(p.width(), p.height());
    setFixedSize(p.width(), p.height());
}

void StatusLabel::timeout()
{
    m_bBlink = !m_bBlink;
    setPict();
}

void StatusLabel::fillStatusMenu(QMenu& menu)
{
    menu.clear();
    menu.setTitle(m_client->name());
    QStringList statusNames = m_client->protocol()->statuses();
    foreach(const QString& statusId, statusNames) {
        IMStatusPtr status = m_client->protocol()->status(statusId);
        QAction* action = menu.addAction(status->icon(), status->name());
        action->setProperty("status_id", status->id());
    }
}

void StatusLabel::mousePressEvent(QMouseEvent *me)
{
    if(me->button() == Qt::RightButton)
    {
        QMenu statusMenu;
        fillStatusMenu(statusMenu);
        if(!statusMenu.isEmpty()) {
            QAction* action = statusMenu.exec(CToolButton::popupPos(this, &statusMenu));
            if(action) {
                IMStatusPtr status = m_client->protocol()->status(action->property("status_id").toString());
                if(!status)
                {
                    log(L_WARN, "Invalid status requested: %s (%s)", qPrintable(action->property("status_id").toString()), qPrintable(m_client->name()));
                    return;
                }
                CorePlugin::instance()->changeClientStatus(m_client, status);
            }
        }
    }
}

StatusFrame::StatusFrame(QWidget *parent) : QFrame(parent), EventReceiver(LowPriority + 1)
{
    log(L_DEBUG, "StatusFrame::StatusFrame()");
    setFrameStyle(NoFrame);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_frame = new QFrame(this);
    m_frame->show();
    m_lay = new QHBoxLayout(m_frame);
    m_lay->setMargin(1);
    m_lay->setSpacing(2);
    m_lay->addStretch();
    addClients();
}

void StatusFrame::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::RightButton){
        Command cmd;
        cmd->id = MenuConnections;
        EventMenuGet e(cmd);
        e.process();
        QMenu *popup = e.menu();
        if (popup)
            popup->popup(me->globalPos());
    }
}

bool StatusFrame::processEvent(Event *e)
{
    switch (e->type()){
    case eEventSocketActive: {
        const QList<StatusLabel*> list = findChildren<StatusLabel*>();
        Q_FOREACH(StatusLabel *lbl, list) {
            lbl->setPict();
        }
        break;
    }
    case eEventCheckCommandState:
        {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->menu_id == MenuStatusWnd) && (cmd->id == CmdStatusWnd)){
            unsigned n = 0;
            const QList<StatusLabel*> list = findChildren<StatusLabel*>();
            Q_FOREACH(StatusLabel *lbl, list) {
                if (lbl->x() + lbl->width() > width())
                    n++;
            }
            CommandDef *cmds = new CommandDef[n + 1];
            n = 0;
            Q_FOREACH(StatusLabel *lbl, list) {
                if (lbl->x() + lbl->width() > width()) {
                    cmds[n].id = 1;
                    cmds[n].text = "_";
                    cmds[n].text_wrk = CorePlugin::instance()->clientName(lbl->m_client);
                    cmds[n].popup_id = lbl->m_id;
                    if (lbl->m_client->getState() == Client::Error) {
                        cmds[n].icon = "error";
                    } else {
                        Protocol *protocol = lbl->m_client->protocol();
                        const CommandDef *cmd = protocol->description();
                        cmds[n].icon = cmd->icon;
                        for (cmd = protocol->statusList(); !cmd->text.isEmpty(); cmd++) {
                            if (cmd->id == lbl->m_client->getStatus()) {
                                cmds[n].icon = cmd->icon;
                                break;
                            }
                        }
                    }
                    n++;
                }
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return true;
        }
        break;
    }
    case eEventClientsChanged:
        addClients();
        break;
    case eEventClientChanged:{
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        StatusLabel *lbl = findLabel(ecc->client());
        if (lbl)
            lbl->setPict();
        break;
    }
    case eEventIconChanged:{
        const QList<StatusLabel*> list = findChildren<StatusLabel*>();
        Q_FOREACH(StatusLabel *lbl, list) {
            lbl->setPict();
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void StatusFrame::addClients()
{
    const QList<StatusLabel*> list = findChildren<StatusLabel*>();
    Q_FOREACH(StatusLabel *lbl, list) {
        delete lbl;
    }
    for (unsigned i = 0; i < getContacts()->nClients(); i++) {
        Client *client = getContacts()->getClient(i);
        QWidget *w = new StatusLabel(m_frame, client, CmdClient + i);
        m_lay->addWidget(w);
        w->show();
    }
    adjustPos();
    repaint();
}

StatusLabel *StatusFrame::findLabel(Client *client)
{
    const QList<StatusLabel*> list = findChildren<StatusLabel*>();
    Q_FOREACH(StatusLabel *lbl, list) {
        if (lbl->m_client == client)
            return lbl;
    }
    return NULL;
}

QSize StatusFrame::sizeHint() const
{
    QSize res = m_frame->sizeHint();
    res.setWidth(20);
    return res;
}

QSize StatusFrame::minimumSizeHint() const
{
    QSize res = m_frame->minimumSizeHint();
    res.setWidth(20);
    return res;
}

void StatusFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    adjustPos();
}

void StatusFrame::adjustPos()
{
    QSize s = m_frame->minimumSizeHint();
    m_frame->resize(s);
    m_frame->move(width() > s.width() ? width() - s.width() : 0, 0);
    emit showButton(width() < s.width());
    repaint();
    m_frame->repaint();
    const QList<StatusLabel*> list = findChildren<StatusLabel*>();
    Q_FOREACH(StatusLabel *lbl, list) {
	lbl->repaint();
    }
}

static const char * const arrow_h_xpm[] = {
            "9 7 3 1",
            "	    c None",
            ".	    c #000000",
            "+	    c none",
            "..++..+++",
            "+..++..++",
            "++..++..+",
            "+++..++..",
            "++..++..+",
            "+..++..++",
            "..++..+++"};

StatusWnd::StatusWnd() : QFrame(NULL)
{
    log(L_DEBUG, "StatusWnd::StatusWnd()");
    setFrameStyle(NoFrame);
    m_lay = new QHBoxLayout(this);
    m_lay->setMargin(0);
    m_frame = new StatusFrame(this);
    m_btn = new QToolButton(this);
    m_btn->setAutoRaise(true);
    QIcon icon( QPixmap((const char **)arrow_h_xpm) );
    m_btn->setIcon( icon );
    m_btn->setMinimumSize(QSize(10, 10));
    m_lay->addWidget(m_frame);
    m_lay->addWidget(m_btn);
    connect(m_frame, SIGNAL(showButton(bool)), this, SLOT(showButton(bool)));
    connect(m_btn, SIGNAL(clicked()), this, SLOT(clicked()));
    EventAddWidget(this, true, EventAddWidget::eStatusWindow).process();
}

void StatusWnd::showButton(bool bState)
{
    if (bState){
        m_btn->show();
    }else{
        m_btn->hide();
    }
}

void StatusWnd::clicked()
{
    Command cmd;
    cmd->popup_id = MenuStatusWnd;
    cmd->flags    = COMMAND_NEW_POPUP;
    EventMenuGet e(cmd);
    e.process();
    QMenu *popup = e.menu();
    if (popup){
        QPoint pos = CToolButton::popupPos(m_btn, popup);
        popup->popup(pos);
    }
}

BalloonMsg *StatusWnd::showError(const QString &text, QStringList &buttons, Client *client)
{
    if (!isVisible())
        return NULL;
    StatusLabel *lbl = m_frame->findLabel(client);
    if (lbl == NULL)
        return NULL;
    if (lbl->x() + lbl->width() > width())
        return NULL;
    return new BalloonMsg(NULL, text, buttons, lbl);
}

