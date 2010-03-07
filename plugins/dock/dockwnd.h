/***************************************************************************
                          dockwnd.h  -  description
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

#ifndef _DOCKWND_H
#define _DOCKWND_H

#include <QWidget>
#include <QPixmap>
#include <QSystemTrayIcon>
#include <QMenu>

#include "event.h"

class WharfIcon;
class DockPlugin;

struct BalloonItem
{
    QString     text;
    QString     title;
    unsigned	id;
    unsigned	flags;
    SIM::Client	*client;
};

class DockWnd : public QWidget, public SIM::EventReceiver
{
    Q_OBJECT
public:
    DockWnd(DockPlugin *plugin, const char *icon, const char *text);
    ~DockWnd();
    void setIcon(const QString &icon);
    void setTip(const QString &text);
    void mouseEvent( QMouseEvent *e);
    virtual void mouseDoubleClickEvent( QMouseEvent *e);
signals:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
protected slots:
    void blink();
    void dbl_click();
    void showPopup();
    void trayAction(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();
protected:
    virtual bool processEvent(SIM::Event *e);
    void  reset();
    bool  bNoToggle;
    QString m_tip;
    QString m_curTipText;
    QString m_curIcon;
    QString m_state;
    QString m_unread;
    QString m_unreadText;
    QPixmap drawIcon;
    virtual void enterEvent( QEvent *e);
    virtual void paintEvent( QPaintEvent *e);
    virtual void mousePressEvent( QMouseEvent *e);
    virtual void mouseReleaseEvent( QMouseEvent *e);
    virtual void mouseMoveEvent( QMouseEvent *e);
    void quit();
    bool bBlink;
    QTimer *blinkTimer;

    bool                m_bBalloon;
    QList<BalloonItem>  m_queue;
    bool                showBalloon();

    DockPlugin *m_plugin;
    QSystemTrayIcon m_TrayIcon;
    QMenu *m_menu;
};

#endif

