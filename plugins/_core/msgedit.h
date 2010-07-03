/***************************************************************************
                          msgedit.h  -  description
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

#ifndef _MSGEDIT_H
#define _MSGEDIT_H

#include "core.h"
#include "simgui/textshow.h"
#include "event.h"

#include <QMenu>

class CorePlugin;
class UserWnd;
class CToolBar;
class QVBoxLayout;
class QFrame;
class TextEdit;

struct ClientStatus
{
    unsigned long	status;
    unsigned		client;
    SIM::IMContact	*data;
};

class MsgTextEdit : public TextEdit
{
    Q_OBJECT
public:
    MsgTextEdit(MsgEdit *edit, QWidget *parent);
protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void contentsDropEvent(QDropEvent*);
    virtual void contentsDragEnterEvent(QDragEnterEvent*);
    virtual void contentsDragMoveEvent(QDragMoveEvent*);
    SIM::Message *createMessage(QMimeSource*);
    MsgEdit *m_edit;
};

class MsgEdit : public QFrame, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgEdit(QWidget *parent, UserWnd *userWnd);
    ~MsgEdit();
    CToolBar    *m_bar;
    bool        setMessage(SIM::Message *msg, bool bSetFocus);
    UserWnd     *m_userWnd;
    TextEdit	*m_edit;
    QVBoxLayout	*m_layout;
    bool        sendMessage(SIM::Message *msg);
    static void setupMessages();
    void        getWays(std::vector<ClientStatus> &cs, SIM::Contact *contact);
    SIM::Client *client(void *&data, bool bCreate, bool bSendTyping, unsigned contact_id, bool bUseClient=true);
    bool        m_bReceived;
    unsigned    m_flags;
    void        execCommand(SIM::CommandDef *cmd);
    unsigned	type() { return m_type; }
    bool        adjustType();
    QString     m_resource;
signals:
    void heightChanged(int);
    void init();
    void finished();
public slots:
    void insertSmile(const QString &id);
    void modeChanged();
    void editLostFocus();
    void editTextChanged();
    void editEnterPressed();
    void setInput();
    void goNext();
    void setupNext();
    void colorsChanged();
    void execCommand();
    void editFinished();
    void editFontChanged(const QFont&);
protected:
    QObject     *m_processor;
    QObject     *m_recvProcessor;
    unsigned    m_type;
    virtual bool    processEvent(SIM::Event*);
    void resizeEvent(QResizeEvent*);
    void stopSend(bool bCheck=true);
    void showCloseSend(bool bShow);
    void typingStart();
    void typingStop();
    void changeTyping(SIM::Client *client, void *data);
    void setEmptyMessage();
    bool setType(unsigned type);
    bool        m_bTyping;
    QString     m_typingClient;
    bool send();
    QList<int> m_multiply;
    SIM::CommandDef m_cmd;
    SIM::Message   *m_msg;
    EventMessageRetry::MsgSend m_retry;
    QString         m_client;
};

class SmileLabel : public QLabel
{
    Q_OBJECT
public:
    SmileLabel(const QString &id, QWidget *parent);
signals:
    void clicked(const QString &id);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    QString id;
};

class SmilePopup : public QFrame
{
    Q_OBJECT
public:
    SmilePopup(QWidget *parent);
signals:
    void insert(const QString &id);
protected slots:
    void labelClicked(const QString &id);
};

#endif

