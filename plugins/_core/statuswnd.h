/***************************************************************************
                          statuswnd.h  -  description
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

#ifndef _STATUSWND_H
#define _STATUSWND_H


#include "event.h"
#include <QFrame>
#include <QLabel>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

class CorePlugin;

class QHBoxLayout;
class QTimer;
class QToolButton;
class BalloonMsg;

class StatusLabel : public QLabel
{
    Q_OBJECT
public:
    StatusLabel(QWidget *parent, SIM::Client *client, unsigned id);

protected slots:
    void timeout();

protected:
    void mousePressEvent(QMouseEvent *e);

private:
    void setPict();
    void fillStatusMenu(QMenu& menu);
    void startBlinkTimer();
    void stopBlinkTimer();
    QTimer			*m_blinkTimer;
    SIM::Client		*m_client;
    unsigned		m_id;
    bool			m_bBlink;
    friend class StatusFrame;

};

class StatusFrame : public QFrame, public SIM::EventReceiver
{
    Q_OBJECT
public:
    StatusFrame(QWidget *parent);
    void adjustPos();
    StatusLabel *findLabel(SIM::Client*);
signals:
    void showButton(bool);
protected slots:
    void addClients();
protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual bool processEvent(SIM::Event*);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QFrame		*m_frame;
    QHBoxLayout	*m_lay;
};

class StatusWnd : public QFrame
{
    Q_OBJECT
public:
    StatusWnd();
    BalloonMsg *showError(const QString &text, QStringList &buttons, SIM::Client *client);
protected slots:
    void showButton(bool);
    void clicked();
protected:
    StatusFrame *m_frame;
    QHBoxLayout	*m_lay;
    QToolButton	*m_btn;
};

#endif

