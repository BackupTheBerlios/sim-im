/***************************************************************************
                          dockwnd.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "simapi.h"

#include "icons.h"
#include "log.h"
#include "dockwnd.h"
#include "dock.h"
#include "core.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QMouseEvent>

using namespace SIM;

void DockWnd::trayAction(QSystemTrayIcon::ActivationReason reason)
{
    log( L_DEBUG, "Tray activation reason: %d", (int)reason );
    switch (reason){
    case QSystemTrayIcon::Context:
        showPopup();
        return;
    case QSystemTrayIcon::DoubleClick:
        bNoToggle = true;
        QTimer::singleShot(0, this, SLOT(dbl_click()));
        return;
    case QSystemTrayIcon::Trigger:
#ifdef Q_OS_MAC
        showPopup();
#else
        if (bNoToggle)
            bNoToggle = false;
        else
            emit toggleWin();
#endif
        return;
    default:
        return;
    }
}

void DockWnd::messageClicked() {
    if (m_queue.isEmpty())
        return;
    unsigned id = m_queue.front().id;
    SIM::Client	*client = m_queue.front().client;
    m_queue.erase(m_queue.begin());
    if (!m_queue.empty())
        showBalloon();
    Command cmd;
    cmd->id    = id;
    cmd->param = client;
    EventCommandExec(cmd).process();
}

void DockWnd::showPopup()
{
    m_menu = m_plugin->createMenu();
#ifdef Q_OS_MAC
    m_TrayIcon.setContextMenu( m_menu );
#else
    m_menu->exec( QCursor::pos() );
#endif
}

DockWnd::DockWnd(DockPlugin *plugin, const char *icon, const char *text)
  : QWidget(NULL, Qt::Window
                  | Qt::FramelessWindowHint
                  | Qt::WindowStaysOnTopHint)
  , EventReceiver(LowPriority)
  , m_state(icon)
  , m_plugin(plugin)
{
    setObjectName("dock");
    setMouseTracking(true);
    bNoToggle = false;
    bBlink = false;
    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));

    m_bBalloon = QSystemTrayIcon::supportsMessages();
    setIcon(icon);
    QWidget::hide();
    connect(
        &m_TrayIcon,
        SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        SLOT(trayAction(QSystemTrayIcon::ActivationReason))
    );
    connect(
        &m_TrayIcon,
        SIGNAL(messageClicked()),
        SLOT(messageClicked())
    );
    m_TrayIcon.show();
    setTip(text);
    reset();
}

DockWnd::~DockWnd()
{
    quit();
}

void DockWnd::quit()
{
    m_TrayIcon.hide();
}

void DockWnd::dbl_click()
{
    emit doubleClicked();
}

bool DockWnd::processEvent(Event *e)
{
    switch (e->type()){
    case eEventMessageReceived:
    case eEventMessageRead:
    case eEventMessageDeleted:
        reset();
        break;
    case eEventSetMainIcon: {
        EventSetMainIcon *smi = static_cast<EventSetMainIcon*>(e);
        m_state = smi->icon();
        if (bBlink)
            break;
        setIcon(m_state);
        break;
    }
    case eEventSetMainText: {
        EventSetMainText *smt = static_cast<EventSetMainText*>(e);
        setTip(smt->text());
        break;
    }
    case eEventIconChanged:
        setIcon((bBlink && !m_unread.isEmpty()) ? m_unread : m_state);
        break;
    case eEventLanguageChanged:
        setTip(m_tip);
        break;
    case eEventQuit:
        quit();
        break;
    case eEventShowNotification:{
        if (!m_bBalloon)
            return false;
        EventShowNotification *ee = static_cast<EventShowNotification*>(e);
        const EventNotification::ClientNotificationData &data = ee->data();
        if (data.id == 0)
            return false;
        foreach(BalloonItem item, m_queue ) {
            if (item.id == data.id)
                return true;
        }
        QString arg = data.args;

        BalloonItem item;
        item.id   = data.id;
        item.client = data.client;
        item.flags  = (data.flags & EventNotification::ClientNotificationData::E_INFO) ? EventNotification::ClientNotificationData::E_INFO : EventNotification::ClientNotificationData::E_INFO;
        item.text = i18n(data.text);
        if (item.text.indexOf("%1") >= 0)
            item.text = item.text.arg(arg);
        if (!m_queue.empty()){
            m_queue.push_back(item);
            return true;
        }
        item.title = "SIM";
        if (getContacts()->nClients() > 1){
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                if (getContacts()->getClient(i) == data.client){
                    item.title = getContacts()->getClient(i)->name();
                    int n = item.title.indexOf(".");
                    if (n > 0)
                        item.title = item.title.left(n) + " " + item.title.mid(n + 1);
                }
            }
        }
        m_queue.push_back(item);
        if (showBalloon())
            return true;
        return false;
    }
    default:
        break;
    }
    return false;
}

bool DockWnd::showBalloon()
{
    if (m_queue.empty())
        return false;
    BalloonItem &item = m_queue.front();

    m_TrayIcon.showMessage(
        item.title,
        item.text,
        item.flags & EventNotification::ClientNotificationData::E_INFO ? QSystemTrayIcon::Information : QSystemTrayIcon::Critical,
        20000
    );

    return true;
}

void DockWnd::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.drawPixmap((width() - drawIcon.width())/2, (height() - drawIcon.height())/2, drawIcon);
}

void DockWnd::setIcon(const QString &icon)
{
    if(m_curIcon == icon)
        return;
    m_curIcon = icon;
    drawIcon = Pict(icon);
    QWidget::setWindowIcon(drawIcon);
    m_TrayIcon.setIcon(drawIcon);
}

void DockWnd::setTip(const QString &text)
{
    m_tip = text;
    QString tip = m_unreadText;
    if (tip.isEmpty()){
        tip = i18n(text);
        tip = tip.remove('&');
    }
    if(tip == m_curTipText)
        return;
    m_curTipText = tip;
    m_TrayIcon.setToolTip(m_curTipText);
}

void DockWnd::mouseEvent( QMouseEvent *e)
{
    switch(e->button()){
    case Qt::LeftButton:
        if (bNoToggle)
            bNoToggle = false;
        else
            emit toggleWin();
        break;
    case Qt::RightButton:
        emit showPopup(e->globalPos());
        break;
    case Qt::MidButton:
        emit doubleClicked();
        break;
    default:
        break;
    }
}

void DockWnd::mousePressEvent( QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
}

void DockWnd::mouseReleaseEvent( QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    mouseEvent(e);
}

void DockWnd::mouseMoveEvent( QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
}

void DockWnd::mouseDoubleClickEvent( QMouseEvent*)
{
    bNoToggle = true;
    emit doubleClicked();
}

void DockWnd::enterEvent( QEvent* )
{
}

void DockWnd::blink()
{
    if (m_unread.isEmpty()){
        bBlink = false;
        blinkTimer->stop();
        setIcon(m_state);
        return;
    }
    bBlink = !bBlink;
    setIcon(bBlink ? m_unread : m_state);
}

struct msgIndex
{
    unsigned	contact;
    unsigned	type;
};

bool operator < (const msgIndex &a, const msgIndex &b)
{
    if (a.contact < b.contact)
        return true;
    if (a.contact > b.contact)
        return false;
    return a.type < b.type;
}

typedef QMap<msgIndex, unsigned> MAP_COUNT;

void DockWnd::reset()
{
    m_unread = QString::null;
    QString oldUnreadText = m_unreadText;
    m_unreadText = QString::null;
    MAP_COUNT count;
    MAP_COUNT::iterator itc;
    CorePlugin *core = GET_CorePlugin();
    for (std::list<msg_id>::iterator it = core->unread.begin(); it != core->unread.end(); ++it){
        if (m_unread.isEmpty()){
            CommandDef *def =core->messageTypes.find(it->type);
            if (def)
                m_unread = def->icon;
        }
        msgIndex m;
        m.contact = it->contact;
        m.type    = it->type;
        itc = count.find(m);
        if (itc == count.end()){
            count.insert(m, 1);
        }else{
            itc.value()++;
        }
    }
    if (!count.empty()){
        for (itc = count.begin(); itc != count.end(); ++itc){
            CommandDef *def = core->messageTypes.find(itc.key().type);
            if (def == NULL)
                continue;
            MessageDef *mdef = (MessageDef*)(def->param);
            QString msg = i18n(mdef->singular, mdef->plural, itc.value());

            Contact *contact = getContacts()->contact(itc.key().contact);
            if (contact == NULL)
                continue;
            msg = i18n("%1 from %2")
                  .arg(msg)
                  .arg(contact->getName());
            if (m_unreadText.length() + 2 + msg.length() >= 64){
                m_unreadText += "...";
                break;
            }

            if (!m_unreadText.isEmpty())
                m_unreadText += "\n";
            m_unreadText += msg;
        }
    }
    if (!m_unread.isEmpty() && !blinkTimer->isActive())
        blinkTimer->start(1500);
    if (m_unreadText != oldUnreadText)
        setTip(m_tip);
}

