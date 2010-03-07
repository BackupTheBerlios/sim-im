/***************************************************************************
                          declinedlg.h  -  description
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

#ifndef _DECLINEDLG_H
#define _DECLINEDLG_H

#include "ui_declinedlgbase.h"
#include "event.h"

class DeclineDlg : public QDialog, public Ui::DeclineDlgBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    DeclineDlg(SIM::Message *msg);
    ~DeclineDlg();
    SIM::Message *message() { return m_msg; }
protected:
    virtual bool processEvent(SIM::Event*);
    void accept();
    SIM::Message *m_msg;
};

#endif

