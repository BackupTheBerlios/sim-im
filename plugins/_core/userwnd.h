/***************************************************************************
                          userwnd.h  -  description
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

#ifndef _USERWND_H
#define _USERWND_H

#include "cfg.h"
#include "message.h"

#include <QSplitter>
#include <QByteArray>
#include <QCloseEvent>

class MsgEdit;
class MsgView;
class CorePlugin;
class QToolBar;
class UserList;

struct UserWndData
{
    SIM::Data	EditHeight;
    SIM::Data	editBar[7];
    SIM::Data	MessageType;
};

class UserWnd : public QSplitter
{
    Q_OBJECT
public:
    UserWnd(unsigned long id, Buffer *cfg, bool bReceived, bool bAdjust);
    ~UserWnd();
    QByteArray getConfig();
    unsigned long id() const { return m_id; }
    QString getName();
    QString getLongName();
    QString getIcon();
    unsigned type();
    void setMessage(SIM::Message*);
    void setStatus(const QString&);
    void showListView(bool bShow);
    QString status() { return m_status; }
    UserList	*m_list;
    void markAsRead();
    bool isClosed() { return m_bClosed; }
    bool m_bTyping;
    PROP_ULONG(MessageType);
signals:
    void closed(UserWnd*);
    void statusChanged(UserWnd*);
    void multiplyChanged();
protected slots:
    void modeChanged();
    void editHeightChanged(int);
    void toolbarChanged(bool);
    void selectChanged();
protected:
    PROP_ULONG(EditHeight);
    void closeEvent(QCloseEvent*);
    MsgView		*m_view;
    MsgEdit		*m_edit;
    QSplitter           *m_splitter;
    QString		m_status;
    bool		m_bResize;
    bool		m_bClosed;
    bool		m_bBarChanged;
    unsigned long	m_id;
    UserWndData	data;
    friend class MsgEdit;
};

#endif

