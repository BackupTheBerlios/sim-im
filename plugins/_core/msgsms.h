/***************************************************************************
                          msgsms.h  -  description
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

#ifndef _MSGSMS_H
#define _MSGSMS_H

#include <QFrame>
#include <QHBoxLayout>
#include "event.h"

class QCheckBox;
class MsgEdit;

class SMSPanel : public QFrame
{
    Q_OBJECT
public:
    SMSPanel(QWidget *parent);
    ~SMSPanel();
    QCheckBox	*chkSave;
private:
	QHBoxLayout *lay;
};

class MsgSMS : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgSMS(MsgEdit *parent, SIM::Message *msg);
    ~MsgSMS();
protected slots:
    void init();
    void textChanged(const QString&);
    void textChanged();
    void panelDestroyed();
protected:
    virtual bool processEvent(SIM::Event*);
    MsgEdit     *m_edit;
    SMSPanel    *m_panel;
    unsigned    m_id;
    bool        m_bExpand;
    bool        m_bCanSend;
};

#endif

