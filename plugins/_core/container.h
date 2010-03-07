/***************************************************************************
                          container.h  -  description
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

#ifndef _CONTAINER_H
#define _CONTAINER_H

#include "cfg.h"
#include "event.h"
#include "message.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QPixmap>
#include <QLabel>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QByteArray>
#include <QEvent>
#include <QMoveEvent>
#include <QMouseEvent>
#include <QTabBar>
#include <QVBoxLayout>
#include <QList>
#include <QShortcut>
#include <list>

const unsigned NEW_CONTAINER	= (unsigned)(-1);
const unsigned GRP_CONTAINER	= 0x80000000;

class UserWnd;
class UserTabBar;
class QSplitter;
class CToolBar;
class QWidgetStack;
class CorePlugin;
class Container;

struct ContainerData
{
    SIM::Data	Id;
    SIM::Data	Windows;
    SIM::Data	ActiveWindow;
    SIM::Data	geometry[5];
    SIM::Data	barState[7];
    SIM::Data	StatusSize;
    SIM::Data	WndConfig;
};

class ContainerStatus : public QStatusBar
{
    Q_OBJECT
public:
    ContainerStatus(QWidget *parent);
signals:
    void sizeChanged(int);
protected:
    void resizeEvent(QResizeEvent*);
};

class UserTabBar : public QTabBar
{
    Q_OBJECT
public:
    UserTabBar(QWidget *parent);
    void raiseTab(unsigned id);
    UserWnd *wnd(unsigned id);
    int tab(unsigned contactid);
    UserWnd *currentWnd();
    std::list<UserWnd*> windows();
    void removeTab(unsigned id);
    void changeTab(unsigned id);
    void setHighlighted(unsigned id, bool bState);
    void setCurrent(unsigned i);
    unsigned current();
    bool isHighlighted(int  /*UserWnd *wnd*/);
public slots:
    void slotRepaint();
protected:
    virtual void layoutTabs();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    UserWnd* wndForTab(int tab);
};

class Container : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
	static const int WndType = QVariant::UserType + 1;

    Container(unsigned id, const char *cfg = NULL);
    ~Container();
    QString name();
    UserWnd *wnd(unsigned id);
    UserWnd *wnd();
    std::list<UserWnd*> windows();
    QByteArray getState();
    bool isReceived() { return m_bReceived; }
    void setReceived(bool bReceived) { m_bReceived = bReceived; }
    void setNoSwitch(bool bState);
    void setMessageType(unsigned id);
    void contactChanged(SIM::Contact *contact);
    PROP_ULONG(Id);
    PROP_STR(Windows);
    PROP_ULONG(ActiveWindow);
    PROP_ULONG(StatusSize);
    PROP_STRLIST(WndConfig);
    bool m_bNoRead;
    void init();
	void setContainerGeometry();
public slots:
    void addUserWnd(UserWnd*, bool bRaise);
    void removeUserWnd(UserWnd*);
    void raiseUserWnd(int /*UserWnd**/);
    void contactSelected(int);
    void toolbarChanged(QToolBar*);
    void statusChanged(int);
    void accelActivated();
    void statusChanged(UserWnd*);
    void modeChanged();
    void wndClosed();
    void flash();
    void setReadMode();
protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);
    virtual bool event(QEvent*);
    virtual bool processEvent(SIM::Event*);
    void showBar();
    void setupAccel();
    QShortcut* makeShortcut(unsigned int key, unsigned int id);
    ContainerData       data;
    bool                m_bInit;
    bool                m_bInSize;
    bool                m_bStatusSize;
    bool                m_bBarChanged;
    bool                m_bReceived;
    bool                m_bNoSwitch;
    CToolBar            *m_bar;
    QDockWidget         m_avatar_window;
    QLabel              m_avatar_label;
    QSplitter           *m_tabSplitter;
    UserTabBar          *m_tabBar;
    ContainerStatus     *m_status;
    QStackedWidget      *m_wnds;
    QList<QShortcut*>   m_shortcuts;
    std::list<UserWnd*> m_childs;
    QFrame *frm;
    QVBoxLayout *lay;
};

#endif

